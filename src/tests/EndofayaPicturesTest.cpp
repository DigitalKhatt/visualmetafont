#include <QtWidgets>
#include <QtTest/QTest>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "font.hpp"
#include "glyph.hpp"
#include "glyphwindow.h"
#include "glyphview.hpp"
#include "sourcelinks.hpp"

static void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
static QByteArray read(const QString& path) {
  QFile f(path); require(f.open(QIODevice::ReadOnly),"Read source fixture"); return f.readAll();
}
static void load(Font& font,const QString& path) {
  require(font.loadFile(path),"Load Madina source"); QApplication::restoreOverrideCursor();
}
static QByteArray snapshot(Glyph* glyph) {
  auto* edge=glyph->getEdge();
  if (!edge) throw std::runtime_error(glyph->getLog().toStdString());
  QByteArray result; QDataStream out(&result,QIODevice::WriteOnly);
  out << edge->width << edge->height << edge->depth << edge->xpart << edge->ypart;
  out << edge->numAnchors;
  for (int i=0;i<edge->numAnchors;++i) {
    const auto& a=edge->anchors[i]; out << QString::fromUtf8(a.anchorName) << a.type << a.x << a.y;
  }
  for (auto* object=edge->body;object;object=object->next) {
    out << int(object->type);
    if (object->type!=mp_fill_code && object->type!=mp_stroked_code) continue;
    const auto* fill=reinterpret_cast<mp_fill_object*>(object);
    out << int(fill->color_model) << fill->color.a_val << fill->color.b_val << fill->color.c_val << fill->color.d_val;
    auto* first=fill->path_p; auto* p=first;
    if (!first) continue;
    do {
      out << p->x_coord << p->y_coord << p->left_x << p->left_y << p->right_x << p->right_y;
      out << int(p->data.types.left_type) << int(p->data.types.right_type);
      p=p->next;
    } while (p && p!=first);
  }
  return result;
}
static QString legacySource(QString source) {
  const int begin=source.indexOf("% Editable source pictures;");
  const int end=source.indexOf("color bluecolor[];",begin);
  require(begin>=0 && end>begin,"Named-picture initialization is present");
  source.remove(begin,end-begin);
  const QStringList parts{"first","second","third","fourth","fifth"};
  for (int i=0;i<parts.size();++i) {
    const auto picture="aya."+parts[i];
    const auto header=QString("controlledImage(%1)(\nfill\n").arg(picture);
    require(source.contains(header),"Each source picture contains a single editable contour");
    source.replace(header,QString("controlledPath (%1,0)(_%2)(\n").arg(i).arg(parts[i]));
    source.replace("pathpart "+picture,"_"+parts[i]);
  }
  return source;
}
static QGraphicsItem* point(GlyphScene* scene,int path,int index) {
  for (auto* item:scene->items()) if (item->data(SourceLinks::KeysRole).toStringList().contains(SourceLinks::pointKey(path,index))) return item;
  return nullptr;
}
static void drag(GlyphView* view,QGraphicsItem* item,const QPointF& delta) {
  require(item && item->flags().testFlag(QGraphicsItem::ItemIsMovable),"Named-picture point is editable");
  view->setFocus(); view->scene()->clearSelection();
  const auto start=item->mapToScene(QPointF()), end=item->mapToScene(delta);
  QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
  press.setScenePos(start); press.setButton(Qt::LeftButton); press.setButtons(Qt::LeftButton); QApplication::sendEvent(view->scene(),&press);
  QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
  move.setScenePos(end); move.setLastScenePos(start); move.setButtons(Qt::LeftButton); QApplication::sendEvent(view->scene(),&move);
  QGraphicsSceneMouseEvent release(QEvent::GraphicsSceneMouseRelease);
  release.setScenePos(end); release.setButton(Qt::LeftButton); QApplication::sendEvent(view->scene(),&release);
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
int main(int argc,char** argv) {
  QApplication app(argc,argv);
  try {
    require(argc>1,"Expected Madina project file");
    const auto project=QString::fromLocal8Bit(argv[1]);
    Font current,legacy; load(current,project); load(legacy,project);
    QByteArray scaleKey;
    for (const auto& axis:current.project.internalParameters()) if (axis.metaPostIndex==100) scaleKey=axis.key.toUtf8();
    require(!scaleKey.isEmpty(),"Project declares its horizontal scale parameter");
    auto* glyph=current.glyphperName.value("endofaya"); auto* reference=legacy.glyphperName.value("endofaya");
    require(glyph && reference,"Aya source glyphs exist");
    reference->setSource(legacySource(glyph->source()));
    std::unique_ptr<Font> frozen;
    if (argc>3) {
      frozen=std::make_unique<Font>(); load(*frozen,project);
      frozen->glyphperName.value("endofaya")->setSource(QString::fromUtf8(read(QString::fromLocal8Bit(argv[3]))));
    }
    struct Variation { double left,right,scale; };
    const QVector<Variation> variations{{0,0,100},{1,0,100},{0,2,100},{4,3,100},{0,0,75},{0,0,125},{2,1,150}};
    int comparisons=0;
    for (bool colored:{false,true}) for (const auto& v:variations) {
      QVector<Font*> fonts{&current,&legacy}; if (frozen) fonts.append(frozen.get());
      for (auto* font:fonts) font->executeMetaPost(colored ? "useColoredAya:=true;" : "useColoredAya:=false;");
      for (const auto& name:QStringList{"endofaya","endofaya.colored"}) {
        for (auto* font:fonts) {
          auto* g=font->glyphperName.value(name); require(g,"Coloured aya source exists");
          g->setProperty("lefttatweel",QVariant::fromValue(Glyph::AxisType{v.left}));
          g->setProperty("righttatweel",QVariant::fromValue(Glyph::AxisType{v.right}));
          g->setProperty(scaleKey.constData(),QVariant::fromValue(Glyph::AxisType{v.scale}));
        }
        const auto actual=snapshot(current.glyphperName.value(name));
        require(actual==snapshot(legacy.glyphperName.value(name)),"Named pictures preserve every curve/control coordinate, colour, anchor and metric across parameter variations");
        if (frozen) require(actual==snapshot(frozen->glyphperName.value(name)),"Frozen pre-migration source produces exactly the same result");
        ++comparisons;
      }
    }
    current.executeMetaPost("useColoredAya:=false;");
    glyph->setProperty("lefttatweel",QVariant::fromValue(Glyph::AxisType{0}));
    glyph->setProperty("righttatweel",QVariant::fromValue(Glyph::AxisType{0}));
    glyph->setProperty(scaleKey.constData(),QVariant::fromValue(Glyph::AxisType{100}));
    const auto neutral=snapshot(glyph), source=glyph->source().toUtf8();
    require(glyph->pictureNames.size()==5,"Five source pictures registered");
    std::unique_ptr<GlyphWindow> window(new GlyphWindow(glyph));
    window->resize(1500,950); window->show(); app.processEvents();
    auto* layers=window->findChild<QTreeWidget*>("layersView"); auto* view=window->findChild<GlyphView*>();
    require(layers && layers->topLevelItemCount()==6+(view->getScene()->hasImage() ? 1 : 0) && layers->currentItem()->data(1,Qt::UserRole).toString().isEmpty(),"Final view plus all five named aya pictures");
    for (double scale:{100.,75.,125.,150.}) {
      glyph->setProperty(scaleKey.constData(),QVariant::fromValue(Glyph::AxisType{scale}));
      auto* edge=glyph->getEdge();
      const auto info=current.getMPGlyphInfo(edge->charcode);
      std::cout << "Aya display scale " << scale << ": final normalization " << edge->xpart << ", " << edge->ypart << '\n';
      const auto raw=Glyph::getPath(info.controlledPictures.value("aya.first")).elementAt(0);
      const auto final=Glyph::getPath(edge).elementAt(0);
      require(QLineF(point(view->getScene(),0,0)->scenePos(),QPointF(final.x,-final.y)).length()<1e-5,
          "Named aya controls align with the final contour after glyph normalization and horizontal scaling");
      auto* mode=window->findChild<QCheckBox*>("finalPositions");
      require(mode && mode->isEnabled(),"Source/final coordinates can be chosen for ordinary named pictures");
      mode->setChecked(false);
      require(QLineF(point(view->getScene(),0,0)->scenePos(),QPointF(raw.x,-raw.y)).length()<1e-5,
          "Source-coordinate mode retains the original authored picture position");
      mode->setChecked(true);
      auto* reference=static_cast<QGraphicsPathItem*>(nullptr);
      for (auto* item:view->scene()->items()) if (item->data(0)=="layerReference" && item->data(1)=="aya.first")
        reference=dynamic_cast<QGraphicsPathItem*>(item);
      require(reference && QLineF(reference->mapToScene(QPointF(raw.x,raw.y)),QPointF(final.x,-final.y)).length()<1e-5,
          "Named picture reference overlay uses the same final coordinate frame as its controls");
    }
    glyph->setProperty(scaleKey.constData(),QVariant::fromValue(Glyph::AxisType{100}));
    const QStringList parts{"first","second","third","fourth","fifth"};
    for (int i=0;i<parts.size();++i) {
      selectLayer(layers,"aya."+parts[i]);
      require(view->getScene()->pictureHasEditablePaths() && point(view->getScene(),i,0),"Each aya picture has verified editable source points");
    }
    selectLayer(layers,QString());
    for (int i=0;i<parts.size();++i) {
      auto* control=point(view->getScene(),i,0);
      require(control && control->isVisible() && control->isEnabled(),"All five visible unlocked aya layers expose editable controls together");
    }
    require(snapshot(glyph)==neutral && glyph->source().toUtf8()==source,"Picture selection does not change the aya glyph");
    selectLayer(layers,"aya.first");
    const auto oldPoint=glyph->property("z1").toPointF();
    drag(view,point(view->getScene(),0,0),QPointF(4,2));
    require(glyph->property("z1").toPointF()==QPointF((oldPoint+QPointF(4,2)).toPoint()) && snapshot(glyph)!=neutral,"Editing a source picture regenerates all dependent aya construction");
    glyph->undoStack()->undo();
    require(snapshot(glyph)==neutral,"Undo named-picture edit restores the exact aya output");
    if (argc>2) {
      QRectF bounds;
      for (auto* item:view->getScene()->items()) {
        if (item->data(0)=="layerReference" && item->isVisible()) bounds=bounds.united(item->sceneBoundingRect());
      }
      view->fitInView(bounds.adjusted(-80,-80,80,80),Qt::KeepAspectRatio);
    }
    if (argc>2) require(window->grab().save(QString::fromLocal8Bit(argv[2])),"Save named-picture editor preview");
    // Sequential evaluation must not accumulate contours inside the pictures.
    for (int i=0;i<8;++i) { glyph->setWidth(glyph->width()); require(snapshot(glyph)==neutral,"Repeated evaluation remains identical"); }

    QTemporaryDir temporary; const QDir target(temporary.path());
    const QDir origin(QFileInfo(project).absolutePath());
    for (const auto& file:origin.entryInfoList({"*.mp","*.json","*.fea"},QDir::Files))
      require(QFile::copy(file.absoluteFilePath(),target.filePath(file.fileName())),"Copy source-only save/reload fixture");
    const auto copiedProject=target.filePath(QFileInfo(project).fileName());
    Font saved; load(saved,copiedProject);
    auto* edited=saved.glyphperName.value("endofaya");
    edited->setPropertyWithUndo("z1",oldPoint+QPointF(4,2));
    const auto editedOutput=snapshot(edited);
    require(saved.saveFile(),"Save named pictures to temporary source project");
    Font reloaded; load(reloaded,copiedProject);
    require(reloaded.glyphperName.value("endofaya")->pictureNames.size()==5 && snapshot(reloaded.glyphperName.value("endofaya"))==editedOutput,
        "Named picture declarations and graphical edits survive save/reload");
    std::cout << "Endofaya pictures: " << comparisons << " exact plain/coloured parameter comparisons; five editable layers; drag/undo; repeated rendering; save/reload passed\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
