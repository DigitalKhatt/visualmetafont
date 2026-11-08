#include <QtWidgets>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "font.hpp"
#include "glyph.hpp"
#include "glyphwindow.h"
#include "glyphview.hpp"
#include "knotitem.hpp"
#include "metaposteditor.hpp"
#include "sourcelinks.hpp"

static void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
static void requireTitleState(GlyphWindow* window,bool modified) {
  require(window->isWindowModified()==modified,"Window modified state follows its saved source");
  require(window->windowTitle().contains("[*]"),"Window keeps Qt's native modified-state placeholder");
  if (QGuiApplication::platformName()=="cocoa")
    require(window->windowTitle().contains(" *[*]")==modified,"macOS window title shows an explicit asterisk only while modified");
}
static void write(const QString& path,const QByteArray& bytes) {
  QFile f(path); require(f.open(QIODevice::WriteOnly) && f.write(bytes)==bytes.size(),"Write fixture");
}
static QGraphicsItem* point(GlyphScene* scene,int path,int knot) {
  const auto key=SourceLinks::pointKey(path,knot);
  for (auto* item:scene->items()) if (item->data(SourceLinks::KeysRole).toStringList().contains(key)) return item;
  return nullptr;
}
static int curves(mp_edge_object* picture) {
  int count=0;
  for (auto* object=picture ? picture->body : nullptr;object;object=object->next)
    if (object->type==mp_fill_code || object->type==mp_stroked_code) ++count;
  return count;
}
static void drag(GlyphView* view,QGraphicsItem* item,const QPointF& delta,bool keepSelection=false) {
  require(item && item->flags().testFlag(QGraphicsItem::ItemIsMovable),"Drag an editable source point");
  view->setFocus(); if (!keepSelection) view->scene()->clearSelection();
  const auto start=item->mapToScene(QPointF());
  const auto end=item->mapToScene(delta);
  QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
  press.setScenePos(start); press.setButton(Qt::LeftButton); press.setButtons(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&press);
  QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
  move.setScenePos(end); move.setLastScenePos(start); move.setButtons(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&move);
  QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
  release.setScenePos(end); release.setButton(Qt::LeftButton);
  QApplication::sendEvent(view->scene(),&release);
}
static QTreeWidgetItem* layer(QTreeWidget* layers,const QString& name) {
  for (int i=0;i<layers->topLevelItemCount();++i) {
    auto* item=layers->topLevelItem(i);
    if (item->data(1,Qt::UserRole).toString()==name) return item;
  }
  throw std::runtime_error("Layer row not found");
}
static void selectLayer(QTreeWidget* layers,const QString& name) {
  layers->setCurrentItem(layer(layers,name),1);
  layers->window()->findChild<GlyphView*>()->setPictureName(name);
}
static QGraphicsPathItem* referenceItem(GlyphScene* scene,const QString& name) {
  for (auto* item:scene->items()) if (item->data(0)=="layerReference" && item->data(1).toString()==name)
    return dynamic_cast<QGraphicsPathItem*>(item);
  return nullptr;
}
static QPoint togglePoint(QTreeWidget* layers,const QString& name,int column,bool indicator=false) {
  auto* item=layer(layers,name);
  layers->scrollToItem(item);
  auto rect=layers->visualItemRect(item);
  const auto left=layers->header()->sectionViewportPosition(column);
  rect.setLeft(left); rect.setWidth(layers->columnWidth(column));
  if (column==1) return QPoint(rect.left()+14,rect.center().y());
  if (!indicator) return rect.center();
  struct Delegate : QStyledItemDelegate { using QStyledItemDelegate::initStyleOption; } delegate;
  QStyleOptionViewItem option; option.initFrom(layers);
  const auto index=layers->model()->index(layers->indexOfTopLevelItem(item),column);
  delegate.initStyleOption(&option,index); option.rect=rect;
  const auto check=layers->style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator,&option,layers);
  require(check.isValid(),"Checkbox indicator has a hit rectangle");
  return check.center();
}
static void clickToggle(QTreeWidget* layers,const QString& name,int column,bool indicator=false) {
  QTest::mouseClick(layers->viewport(),Qt::LeftButton,Qt::NoModifier,togglePoint(layers,name,column,indicator));
}

static void chooseDestinationFromMenu(QTreeWidget* layers,const QString& name) {
  bool found=false;
  QTimer::singleShot(0,[&] {
    auto* menu=qobject_cast<QMenu*>(QApplication::activePopupWidget());
    if (!menu) return;
    for (auto* action:menu->actions()) if (action->text()=="Use for new paths") {
      found=true; menu->setActiveAction(action); QTest::keyClick(menu,Qt::Key_Return); return;
    }
    menu->close();
  });
  const auto position=togglePoint(layers,name,1);
  QContextMenuEvent event(QContextMenuEvent::Mouse,position,layers->viewport()->mapToGlobal(position));
  QApplication::sendEvent(layers->viewport(),&event);
  require(found,"Layer context menu exposes an explicit new-path destination action");
}
static void selectionAndSavedState(const QString& project,const QString& scanPath) {
  Font font; require(font.loadFile(project),"Load selection/undo fixture"); QApplication::restoreOverrideCursor();
  auto* glyph=font.glyphs.first();
  std::unique_ptr<GlyphWindow> window(new GlyphWindow(glyph));
  window->resize(1450,950); window->show(); window->activateWindow(); QApplication::processEvents();
  auto* view=window->findChild<GlyphView*>(); auto* scene=view->getScene();
  auto* editor=window->findChild<MetaPostEditor*>();
  scene->setPictureName("construction"); scene->setLayerLocked(QString(),true);
  Glyph::ImageInfo info; info.path=QFileInfo(scanPath).fileName();
  glyph->setPropertyWithUndo("image",QVariant::fromValue(info));
  requireTitleState(window.get(),true);
  scene->setLayerLocked(GlyphScene::imageLayerName(),true);
  ImageItem* image=nullptr;
  for (auto* item:scene->items()) if (auto* candidate=dynamic_cast<ImageItem*>(item)) image=candidate;
  const auto a=point(scene,0,0)->scenePos(), b=point(scene,0,2)->scenePos();
  QRectF bounds(a,b); bounds=bounds.normalized().adjusted(-30,-110,30,30);
  image->setPos(bounds.topLeft()-QPointF(20,20)); image->setScale(10);
  view->centerOn(bounds.center()); QApplication::processEvents();
  const auto start=view->mapFromScene(bounds.topLeft()), end=view->mapFromScene(bounds.bottomRight());
  require(image->contains(image->mapFromScene(view->mapToScene(start))),"Selection starts over the locked image");
  scene->clearSelection();
  QTest::mousePress(view->viewport(),Qt::LeftButton,Qt::NoModifier,start);
  QMouseEvent move(QEvent::MouseMove,end,view->viewport()->mapToGlobal(end),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QApplication::sendEvent(view->viewport(),&move); QApplication::processEvents();
  const bool rectangleVisible=!view->rubberBandRect().isEmpty();
  QTest::mouseRelease(view->viewport(),Qt::LeftButton,Qt::NoModifier,end);
  require(rectangleVisible,"Locked picture/image allows the default rubber-band rectangle");
  require(point(scene,0,0)->isSelected() && point(scene,0,2)->isSelected() && !image->isSelected(),
      "Rubber band selects multiple editable points through locked layers");
  glyph->undoStack()->undo();
  require(!window->isWindowModified(),"Undo to loaded source clears modified state");
  requireTitleState(window.get(),false);
  glyph->undoStack()->redo(); require(window->isWindowModified(),"Redo restores modified state");
  requireTitleState(window.get(),true);
  require(font.saveFile(),"Save edited fixture");
  require(!window->isWindowModified(),"Saving the font updates the glyph window");
  requireTitleState(window.get(),false);
  glyph->undoStack()->undo(); require(window->isWindowModified(),"Undo away from saved source is modified");
  glyph->undoStack()->redo(); require(!window->isWindowModified(),"Redo to saved source clears modified state");
  drag(view,point(scene,0,1),QPointF(12,6));
  require(window->isWindowModified(),"Graphical point edit is modified");
  requireTitleState(window.get(),true);
  glyph->undoStack()->undo(); require(!window->isWindowModified(),"Undo graphical edit to saved source clears modified state");
  glyph->undoStack()->redo(); require(window->isWindowModified(),"Redo graphical edit is modified");
  glyph->undoStack()->undo();
  std::unique_ptr<GlyphWindow> second(new GlyphWindow(glyph));
  glyph->setPropertyWithUndo("width",glyph->width()+10);
  require(window->isWindowModified() && second->isWindowModified(),"All windows track model edits");
  require(font.saveFile(),"Save model with multiple windows");
  require(!window->isWindowModified() && !second->isWindowModified(),"Font save clears all saved windows");
  auto* code=editor->textEdit(); code->moveCursor(QTextCursor::End); code->insertPlainText("\n% pending draft\n");
  require(window->isWindowModified(),"Unexecuted draft is modified");
  requireTitleState(window.get(),true);
  code->undo(); require(!window->isWindowModified(),"Undo unexecuted draft clears modified state");
  code->redo(); glyph->undoStack()->undo(); glyph->undoStack()->redo();
  require(window->isWindowModified(),"Undo/redo model to saved state still protects pending draft");
  editor->useDrawing(); require(!window->isWindowModified(),"Discarding the remaining draft restores clean state");
  auto body=code->document()->find("%%beginbody");
  require(!body.isNull(),"Executed draft fixture has a body");
  body.movePosition(QTextCursor::EndOfBlock); body.insertText("\n% executed draft");
  window->executeCommands();
  require(window->isWindowModified() && !editor->hasPendingChanges(),"Executing source marks the model modified");
  glyph->undoStack()->undo();
  require(!window->isWindowModified(),"Undo executed source restores clean state");
}
int main(int argc,char** argv) {
  QApplication app(argc,argv);
  try {
    QTemporaryDir temporary; QDir dir(temporary.path());
    write(dir.filePath("test.mp"),"lineheight:=88;penwidth:=88;nuqta:=123.2;showAnchors:=0;\n");
    write(dir.filePath("font-project.json"),R"({"schemaVersion":2,"groups":[],"glyphGroups":{},"glyphFiles":["glyphs.mp"],"internalParameters":[{"key":"scaleX","label":"Horizontal scale","metaPostIndex":100,"metaPostMapping":"absolute","min":50,"default":100,"max":200}],"openTypeAxes":[]})");
    write(dir.filePath("glyphs.mp"),R"(defchar(capture.test,123,800,-1,-1);
%%beginparams
z.reference = (50,150);
%%beginpaths
controlledImage(construction)(
draw (0,0) .. tension 1.2 and 1.4 .. (90+lefttatweel*20,60) .. (180,0)
);
controlledImage(other)(
draw (-400,-50) -- (-300,50)
);
fill (300,0)--(350,0)--(350,100)--(300,100)--cycle;
%%endpaths
%%beginbody
draw construction shifted (500,300);
other := other shifted (50,0);
enddefchar;
)");
    Font font;
    require(font.loadFile(dir.filePath("test.mp")),"Load captured-picture fixture"); QApplication::restoreOverrideCursor();
    auto* glyph=font.glyphs.first();
    if (!glyph->getEdge()) { std::cerr << glyph->getLog().toStdString() << '\n'; require(false,"Render captured-picture fixture"); }
    const auto originalSource=glyph->source();
    const auto originalPath=Glyph::getPath(glyph->getEdge());
    const auto width=glyph->getEdge()->width;
    auto info=font.getMPGlyphInfo(glyph->getEdge()->charcode);
    require(info.controlledPictures.size()==2 && info.controlledPictureSources.size()==3,"Capture requested pictures and their source snapshots");
    require(curves(info.controlledPictures.value("construction"))==1,"Capture construction centerline once");
    // The legacy helper must produce the same final geometry and metrics.
    Font reference; require(reference.loadFile(dir.filePath("test.mp")),"Load reference fixture"); QApplication::restoreOverrideCursor();
    reference.executeMetaPost(R"(def controlledImage(text name,content)=
if not picture name: picture name; name:=nullpicture; fi
begingroup save currentpicture; picture currentpicture;
currentpicture:=name; content; name:=currentpicture; endgroup enddef;)");
    auto* old=reference.glyphs.first();
    require(old->getEdge() && Glyph::getPath(old->getEdge())==originalPath && old->getEdge()->width==width,
        "Capturing metadata does not change legacy final output");

    std::unique_ptr<GlyphWindow> window(new GlyphWindow(glyph));
    window->resize(1450,950); window->show(); window->activateWindow(); app.processEvents();
    auto* layers=window->findChild<QTreeWidget*>("layersView");
    auto* view=window->findChild<GlyphView*>(); auto* scene=view->getScene();
    auto* editor=window->findChild<MetaPostEditor*>();
    auto* pathDock=window->findChild<QDockWidget*>("Path");
    auto* layersDock=window->findChild<QDockWidget*>("layersDock");
    require(pathDock && layersDock && pathDock->widget()->height()>=160 && pathDock->height()>layersDock->height(),
        "Path preview gets a usable size and more space than the compact Layers dock");
    require(!window->findChild<QSlider*>("referenceOpacity") && layersDock->findChildren<QLabel*>().isEmpty(),
        "Layers dock has no opacity slider or explanatory text");
    require(!window->findChild<QComboBox*>("pictureSelector"),"Toolbar picture selector replaced by Layers dock");
    require(layers && layers->topLevelItemCount()==3 && layers->currentItem()->data(1,Qt::UserRole).toString().isEmpty(),"Final glyph is default, captured pictures selectable");
    for (const auto& name:scene->pictureNames())
      require(scene->layerVisible(name) && !scene->layerLocked(name) && layer(layers,name)->checkState(0)==Qt::Checked,
          "All named picture layers open visible and unlocked");
    require(point(scene,2,0) && point(scene,2,0)->isVisible() && point(scene,0,0)->isVisible() && point(scene,0,0)->isEnabled(),
        "Final and named picture controls are editable from first opening");
    chooseDestinationFromMenu(layers,"construction");
    require(scene->pictureName()=="construction" && scene->layerVisible("construction"),"Context-menu action selects and reveals the new-path destination");
    selectLayer(layers,QString());
    view->zoomOut(); view->zoomOut(); const auto finalZoom=view->transform();
    for (int i=0;i<8;++i) {
      selectLayer(layers,"construction");
      require(scene->pictureName()=="construction" && scene->pictureHasEditablePaths(),"Captured picture is editable through its source paths");
      require(point(scene,0,0) && point(scene,0,2),"Open construction curve exposes first and last authored points");
      require(point(scene,2,0)->isVisible() && point(scene,2,0)->isEnabled(),"Visible unlocked final layer keeps its controls while a construction layer is selected");
      selectLayer(layers,QString());
    }
    require(view->transform()==finalZoom && glyph->source()==originalSource && glyph->undoStack()->count()==0 && !window->isWindowModified(),
        "Switching views preserves final camera, source, undo and dirty state");
    require(Glyph::getPath(glyph->getEdge())==originalPath && glyph->getEdge()->width==width,"Switching pictures preserves final output");

    // Real users click the small checkbox itself as well as its surrounding cell.
    // Both must toggle exactly once; Qt's release delegate must not toggle a
    // second time after our custom press handler.
    for (int column:{0,1,2}) for (bool indicator:{true,false}) for (int i=0;i<4;++i) {
      const int stateColumn=column==2 ? 2 : 0;
      const auto previous=layer(layers,"other")->checkState(stateColumn);
      QSignalSpy changes(scene,&GlyphScene::picturesChanged);
      clickToggle(layers,"other",column,indicator);
      require(layer(layers,"other")->checkState(stateColumn)!=previous && changes.count()==1,
          "One checkbox click produces exactly one layer change, both on the indicator and the cell");
      require(scene->pictureName().isEmpty(),"Checkbox clicks keep the active layer unchanged");
    }
    for (int column:{0,1,2}) {
      const int stateColumn=column==2 ? 2 : 0;
      const auto previous=layer(layers,"other")->checkState(stateColumn);
      QSignalSpy changes(scene,&GlyphScene::picturesChanged);
      const auto position=togglePoint(layers,"other",column,true);
      QTest::mouseDClick(layers->viewport(),Qt::LeftButton,Qt::NoModifier,position);
      QTest::mouseRelease(layers->viewport(),Qt::LeftButton,Qt::NoModifier,position);
      require(layer(layers,"other")->checkState(stateColumn)!=previous && changes.count()==1,
          "The second click in a rapid pair must toggle, rather than being swallowed as a double-click");
      clickToggle(layers,"other",column,true);
    }
    for (int column:{0,1,2}) {
      const int stateColumn=column==2 ? 2 : 0;
      const auto previous=layer(layers,"other")->checkState(stateColumn);
      QSignalSpy changes(scene,&GlyphScene::picturesChanged);
      const auto position=togglePoint(layers,"other",column,true);
      QTest::mousePress(layers->viewport(),Qt::LeftButton,Qt::NoModifier,position);
      require(layer(layers,"other")->checkState(stateColumn)==previous,"Checkbox waits for release before committing");
      const auto outside=layers->visualItemRect(layer(layers,QString())).center();
      QTest::mouseMove(layers->viewport(),outside);
      QTest::mouseRelease(layers->viewport(),Qt::LeftButton,Qt::NoModifier,outside);
      require(layer(layers,"other")->checkState(stateColumn)==previous && changes.count()==0,
          "Releasing outside the pressed toggle cancels the click without selecting another layer");
    }
    // Visibility is independent of selection; all reference copies are inert.
    require(scene->layerVisible(QString()) && scene->layerVisible("other"),"Layer toggles preserve the initially visible layers");
    auto* constructionReference=referenceItem(scene,"construction");
    require(constructionReference && constructionReference->isVisible() && !constructionReference->flags().testFlag(QGraphicsItem::ItemIsSelectable) &&
        constructionReference->acceptedMouseButtons()==Qt::NoButton && constructionReference->childItems().isEmpty(),
        "Reference contours remain inert while their separate source controls are available");
    require(constructionReference->path()==Glyph::getPath(info.controlledPictures.value("construction")) &&
        constructionReference->transform()==QTransform::fromScale(1,-1),"Construction reference uses original source coordinates");
    clickToggle(layers,"construction",0);
    require(scene->pictureName().isEmpty() && !scene->layerVisible("construction") && !constructionReference->isVisible(),"Eye toggles only reference visibility without changing active layer");
    selectLayer(layers,"construction");
    require(scene->layerVisible("construction") && referenceItem(scene,QString())->isVisible() && !constructionReference->isVisible(),
        "Selecting a hidden layer reveals it, final glyph becomes a reference and active curve is not drawn twice");
    clickToggle(layers,QString(),0);
    require(scene->pictureName()=="construction" && !referenceItem(scene,QString())->isVisible(),"Hide final overlay independently");
    clickToggle(layers,QString(),0);
    clickToggle(layers,"other",2);
    require(scene->pictureName()=="construction" && scene->layerLocked("other"),"Locking a reference does not select it");
    clickToggle(layers,"construction",2);
    auto* lockedPoint=point(scene,0,2);
    require(scene->selectedItems().isEmpty() && lockedPoint && !lockedPoint->isEnabled(),"Active lock clears selection and disables graphical controls");
    drag(view,lockedPoint,QPointF(12,8));
    QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::ShiftModifier,view->mapFromScene(QPointF(250,50)));
    require(glyph->source()==originalSource && glyph->undoStack()->count()==0,"Locked layer blocks both point drags and shift-click path creation");
    const bool wasModified=window->isWindowModified();
    glyph->setWidth(glyph->width());
    require(scene->layerLocked("construction") && !point(scene,0,2)->isEnabled() && scene->layerLocked("other"),"Locks survive regeneration");
    window->setWindowModified(wasModified); // The explicit model edit above marks the window dirty.
    clickToggle(layers,"construction",2);
    require(point(scene,0,2)->isEnabled(),"Unlock restores editing");
    clickToggle(layers,"construction",0);
    require(!point(scene,0,2)->isVisible(),"Hiding active layer hides its controls");
    drag(view,point(scene,0,2),QPointF(12,8));
    QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::ShiftModifier,view->mapFromScene(QPointF(250,50)));
    require(glyph->source()==originalSource && glyph->undoStack()->count()==0,"Hidden active layer cannot be modified graphically");
    clickToggle(layers,"construction",0);
    require(qAbs(referenceItem(scene,QString())->opacity()-0.25)<1e-9 && scene->pictureName()=="construction", "Layer references retain their default opacity");
    require(glyph->source()==originalSource && Glyph::getPath(glyph->getEdge())==originalPath && glyph->undoStack()->count()==0 && !window->isWindowModified(),
        "All layer controls preserve font output, source, undo stack and dirty state");

    scene->setFillEnable(true);
    selectLayer(layers,QString()); selectLayer(layers,"construction");
    require(referenceItem(scene,QString())->brush().style()!=Qt::NoBrush,"Filled references follow the Fill setting");
    bool filled=false;
    for (auto* item:scene->items()) if (auto* contour=dynamic_cast<ContourItem*>(item)) {
      for (auto* child:contour->childItems()) if (auto* path=dynamic_cast<QGraphicsPathItem*>(child)) {
        if (!path->path().isEmpty() && path->brush().style()!=Qt::NoBrush) filled=true;
      }
    }
    require(filled,"Active layer preserves the Fill setting across selection");
    scene->setFillEnable(false);

    // Edit a different visible layer without selecting it in the dock.
    selectLayer(layers,QString());
    require(point(scene,0,2)->isVisible() && point(scene,0,2)->isEnabled(),"Visible unlocked source layer exposes controls while Final glyph is selected");
    drag(view,point(scene,0,2),QPointF(12,8));
    require(glyph->source().contains("(192,8)") && scene->pictureName().isEmpty(),"Nonselected source-layer drag edits the correct source without changing the selected layer");
    glyph->undoStack()->undo();
    require(Glyph::getPath(glyph->getEdge())==originalPath,"Undo a nonselected-layer drag");
    selectLayer(layers,"construction");
    clickToggle(layers,"construction",2);
    drag(view,point(scene,2,0),QPointF(10,5));
    require(glyph->source().contains("(310,5)") && scene->layerLocked("construction"),"A locked selected layer does not block editing another unlocked layer");
    glyph->undoStack()->undo();
    clickToggle(layers,"construction",2);
    scene->clearSelection();
    point(scene,0,0)->setSelected(true); point(scene,2,0)->setSelected(true);
    clickToggle(layers,"construction",2);
    require(!point(scene,0,0)->isSelected() && point(scene,2,0)->isSelected(),"Lock deselects only the affected layer");
    clickToggle(layers,"construction",2); scene->clearSelection();

    point(scene,0,2)->setSelected(true); point(scene,2,0)->setSelected(true);
    drag(view,point(scene,0,2),QPointF(6,4),true);
    require(glyph->source().contains("(186,4)") && glyph->source().contains("(306,4)"),"Grouped movement edits source points in both visible layers");
    glyph->undoStack()->undo();
    require(Glyph::getPath(glyph->getEdge())==originalPath,"One Undo restores a multi-layer drag");
    selectLayer(layers,QString());
    scene->clearSelection(); auto* nonselectedStart=point(scene,0,0); nonselectedStart->setSelected(true);
    const auto insertion=nonselectedStart->parentItem()->mapToScene(QPointF(30,15));
    QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::ShiftModifier,view->mapFromScene(insertion));
    require(glyph->controlledPaths[0].size()==4 && glyph->controlledPaths[2].size()==5,
        "Insertion in a nonselected layer modifies its own source path");
    glyph->undoStack()->undo();
    require(Glyph::getPath(glyph->getEdge())==originalPath,"Undo nonselected-layer insertion");

    // Keep a draft while editing a captured point, and exercise graphical undo/redo.
    auto cursor=editor->textEdit()->document()->find("draw construction shifted (500,300);");
    cursor.movePosition(QTextCursor::EndOfBlock); cursor.insertText("\n% draft survives picture edits");
    selectLayer(layers,"construction");
    auto* last=point(scene,0,2);
    drag(view,last,QPointF(12,8));
    require(glyph->source().contains("(192,8)") && editor->toPlainText().contains("draft survives picture edits"),"Captured-point drag changes its authored path and keeps pending code");
    require(Glyph::getPath(glyph->getEdge())!=originalPath,"Captured construction edit regenerates final glyph");
    require(referenceItem(scene,QString())->path()==Glyph::getPath(glyph->getEdge()),"Final reference updates after graphical source edits");
    require(scene->pictureName()=="construction" && curves(font.getMPGlyphInfo(glyph->getEdge()->charcode).controlledPictures.value("construction"))==1,
        "Regeneration retains view and never accumulates old construction paths");
    glyph->undoStack()->undo();
    require(Glyph::getPath(glyph->getEdge())==originalPath && editor->toPlainText().contains("draft survives picture edits"),"Captured edit undo restores final output and retains draft");
    require(referenceItem(scene,QString())->path()==originalPath,"Final reference updates after undo");
    glyph->undoStack()->redo(); glyph->undoStack()->undo();
    require(scene->pictureName()=="construction" && point(scene,0,2),"Capture bindings survive repeated undo/redo");

    // Point insertion must use the selected picture's source frame and global path ID.
    auto* startPoint=point(scene,0,0); scene->clearSelection(); startPoint->setSelected(true);
    const auto insertedPosition=startPoint->parentItem()->mapToScene(QPointF(30,15));
    QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::ShiftModifier,view->mapFromScene(insertedPosition));
    require(glyph->controlledPaths[0].size()==4,"Insert point in captured path");
    const auto inserted=glyph->controlledPaths[0][1]->expr->constantValue(0).toPointF();
    require(QLineF(inserted,QPointF(30,15)).length()<2,"Inserted point uses captured source coordinates");
    glyph->undoStack()->undo();
    require(glyph->controlledPaths[0].size()==3 && Glyph::getPath(glyph->getEdge())==originalPath,"Undo captured point insertion");

    // Code and graphical selection continue to use global source path identities.
    auto* first=point(scene,0,0); scene->clearSelection(); first->setSelected(true);
    bool linked=false;
    for (const auto& selection:editor->textEdit()->extraSelections())
      if (selection.format.property(QTextFormat::UserProperty)=="sourceDeclaration" && selection.cursor.selectedText()=="(0,0)") linked=true;
    require(linked,"Captured point navigates to its source declaration");
    if (argc>1) require(window->grab().save(QString::fromLocal8Bit(argv[1])),"Save Layers dock preview");

    selectLayer(layers,"other");
    require(!scene->pictureHasEditablePaths(),"Picture altered after capture is displayed read-only instead of guessing a transform");
    for (auto* item:scene->items()) if (item->type()==KnotItem::Type) {
      for (auto* parent=item->parentItem();parent;parent=parent->parentItem())
        if (auto* layer=dynamic_cast<ContourItem*>(parent)) {
          if (layer->pictureName()=="other")
            require(!item->flags().testFlag(QGraphicsItem::ItemIsMovable),"Unverified picture cannot expose unrelated source controls");
          break;
        }
    }

    selectLayer(layers,QString());
    drag(view,point(scene,2,0),QPointF(10,5));
    require(glyph->source().contains("(310,5)") && scene->pictureName().isEmpty(),"Final glyph retains direct graphical editing with captured pictures present");
    glyph->undoStack()->undo(); require(Glyph::getPath(glyph->getEdge())==originalPath,"Final-view undo restores geometry");
    for (int i=0;i<20;++i) {
      glyph->setWidth(glyph->width());
      require(Glyph::getPath(glyph->getEdge())==originalPath &&
          curves(font.getMPGlyphInfo(glyph->getEdge()->charcode).controlledPictures.value("construction"))==1,
          "Repeated capture replaces snapshots without changing final output or adding contours");
    }
    // Changing a source dependency updates captures while their source IDs remain stable.
    selectLayer(layers,"construction");
    const auto source=editor->toPlainText();
    require(!source.isEmpty(),"Draft retained");
    for (double left: {0.0,1.0,4.0}) for (double scale: {100.0,125.0,150.0}) {
      glyph->setProperty("lefttatweel",QVariant::fromValue(Glyph::AxisType{left}));
      glyph->setProperty("scaleX",QVariant::fromValue(Glyph::AxisType{scale}));
      old->setProperty("lefttatweel",QVariant::fromValue(Glyph::AxisType{left}));
      old->setProperty("scaleX",QVariant::fromValue(Glyph::AxisType{scale}));
      require(Glyph::getPath(glyph->getEdge())==Glyph::getPath(old->getEdge()) && glyph->getEdge()->width==old->getEdge()->width,
          "Parameter variations preserve legacy final output");
      require(scene->pictureName()=="construction" && point(scene,0,2),"Construction bindings survive extension and horizontal-scale variations");
    }
    selectLayer(layers,QString());
    const auto scaled=Glyph::getPath(glyph->getEdge());
    drag(view,point(scene,2,0),QPointF(8,4));
    require(glyph->source().contains("(308,4)"),"Final-picture dragging inverts the verified horizontal-scale frame");
    glyph->undoStack()->undo(); require(Glyph::getPath(glyph->getEdge())==scaled,"Scaled final edit Undo");
    // Source structure changes refresh the list and discard removed captures.
    selectLayer(layers,"construction");
    auto withoutPictures=glyph->source();
    withoutPictures.replace("controlledImage(construction)","controlledImage(replacement)");
    withoutPictures.replace("draw construction shifted", "draw replacement shifted");
    glyph->setSource(withoutPictures);
    require(scene->pictureName().isEmpty() && layers->currentItem()->data(1,Qt::UserRole).toString().isEmpty(),
        "Removing the active picture safely returns to final glyph");
    require(!scene->pictureNames().contains("construction") && !referenceItem(scene,"construction") && !scene->layerLocked("construction"),
        "Removed picture drops its reference and view state");
    selectLayer(layers,"replacement");
    require(scene->pictureHasEditablePaths() && !scene->layerLocked("replacement"),"Newly named picture is editable and unlocked");
    // Image is a separate layer, independent of the selected or Final layer.
    const auto beforeImage=Glyph::getPath(glyph->getEdge()); const auto beforeImageWidth=glyph->getEdge()->width;
    QImage scan(64,64,QImage::Format_RGB32); scan.fill(QColor(230,235,240));
    const auto scanPath=dir.filePath("reference.png"); require(scan.save(scanPath),"Create image-layer fixture");
    const int rowCount=layers->topLevelItemCount();
    Glyph::ImageInfo imageInfo; imageInfo.path=scanPath; imageInfo.pos=QPointF(450,-140);
    glyph->setPropertyWithUndo("image",QVariant::fromValue(imageInfo));
    require(scene->hasImage() && layers->topLevelItemCount()==rowCount+1 && layer(layers,GlyphScene::imageLayerName())->text(1)=="Image",
        "Importing an image adds its independent layer row");
    ImageItem* image=nullptr;
    for (auto* item:scene->items()) if (auto* candidate=dynamic_cast<ImageItem*>(item)) image=candidate;
    require(image && !image->pixmap().isNull(),"Imported image is loaded");
    scene->setLayerVisible(QString(),false); scene->setLayerLocked(QString(),true);
    require(image->isVisible() && image->isEnabled(),"Hiding and locking Final glyph leaves the image visible and editable");
    const auto imageSource=glyph->source(); const auto destination=scene->pictureName();
    clickToggle(layers,GlyphScene::imageLayerName(),1);
    require(!image->isVisible() && scene->pictureName()==destination && glyph->source()==imageSource,"Image name toggles visibility only");
    clickToggle(layers,GlyphScene::imageLayerName(),0);
    require(image->isVisible(),"Image Show checkbox has the same effect as its name");
    clickToggle(layers,GlyphScene::imageLayerName(),2);
    require(image->isVisible() && !image->isEnabled(),"Image lock prevents editing without hiding the scan");
    drag(view,image,QPointF(10,5));
    require(glyph->image().pos==imageInfo.pos && glyph->source()==imageSource,"Locked image drag does not change its placement or source");
    clickToggle(layers,GlyphScene::imageLayerName(),2);
    scene->clearSelection(); point(scene,0,0)->setSelected(true); image->setSelected(true);
    clickToggle(layers,GlyphScene::imageLayerName(),2);
    require(!image->isSelected() && point(scene,0,0)->isSelected(),"Image lock clears only the image selection");
    clickToggle(layers,GlyphScene::imageLayerName(),2);
    drag(view,image,QPointF(10,5));
    require(glyph->image().pos==imageInfo.pos+QPointF(10,5),"Unlocked image moves even with Final glyph hidden and locked");
    glyph->undoStack()->undo(); require(glyph->image().pos==imageInfo.pos,"Undo restores image placement");
    for (auto* action:window->findChildren<QAction*>()) {
      auto text=action->text(); text.remove('&');
      require(text!="Enable Image" && text!="Visible Image","Legacy image menu controls are removed");
    }
    glyph->setPropertyWithUndo("image",QVariant::fromValue(Glyph::ImageInfo{}));
    require(!scene->hasImage() && layers->topLevelItemCount()==rowCount && !image->isVisible(),"Removing an image removes its layer row");
    glyph->undoStack()->undo(); require(scene->hasImage() && image->isVisible(),"Undo image removal restores the layer");
    require(Glyph::getPath(glyph->getEdge())==beforeImage && glyph->getEdge()->width==beforeImageWidth,
        "Image import, layer controls, movement and undo preserve glyph output");
    require(editor->toPlainText().contains("draft survives picture edits"),"Image editing preserves the pending source draft");
    if (argc>1) require(window->grab().save(QString::fromLocal8Bit(argv[1])),"Save image-layer preview");
    selectionAndSavedState(dir.filePath("test.mp"),scanPath);
    std::cout << "Layers editor checks passed: name/eye toggle parity, rapid clicks, independent image layer and drag/undo, menu cleanup, simultaneous controls, source bindings and geometry parity\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
