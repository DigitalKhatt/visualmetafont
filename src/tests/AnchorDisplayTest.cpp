#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QtTest>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "font.hpp"
#include "glyph.hpp"
#include "glyphview.hpp"
#include "glyphwindow.h"
#include "knotitem.hpp"
#include "pairitem.hpp"
#include "sourcelinks.hpp"

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
static void write(const QString& path, const QByteArray& data) {
  QFile file(path);
  require(file.open(QIODevice::WriteOnly) && file.write(data) == data.size(), "Write fixture");
}
static void labelsDoNotOverlap(GlyphView* view, int minimum = 10) {
  QVector<QRectF> rectangles;
  for (auto* item : view->scene()->items()) {
    if (item->data(0).toString() != "anchorLabel" || !item->isVisible()) continue;
    const auto bounds = item->deviceTransform(view->viewportTransform()).mapRect(item->boundingRect());
    for (const auto& previous : rectangles)
      require(!previous.intersects(bounds), "Anchor text must not overlap in viewport coordinates");
    rectangles.append(bounds);
    for (auto* point : view->scene()->items()) if (auto* parameter = dynamic_cast<PairItem*>(point)) {
      if (!parameter->isVisible()) continue;
      const auto marker = parameter->deviceTransform(view->viewportTransform()).mapRect(parameter->boundingRect());
      require(!marker.intersects(bounds), "Anchor labels must leave editable markers uncovered");
    }
    for (auto* point : view->scene()->items()) if (point->type() == KnotItem::Type && point->isVisible()) {
      const auto marker = point->deviceTransform(view->viewportTransform()).mapRect(point->boundingRect());
      require(!marker.intersects(bounds), "Anchor labels must leave outline controls uncovered");
    }
  }
  require(rectangles.size() >= minimum, "Computed anchors visible even from an edge cached while hidden");
}

int main(int argc, char** argv) {
  QApplication application(argc, argv);
  try {
    QTemporaryDir temporary;
    const QDir dir(temporary.path());
    write(dir.filePath("test.mp"), "lineheight:=88;penwidth:=88;nuqta:=123.2;showAnchors:=1;\n");
    QByteArray source = R"(defchar(anchor.test,123,300,-1,-1);
%%beginparams
z.p = (40,130);
z.mark.offset = (0,0);
z.ordinary = (230,20);
%%beginpaths
fill (0,0)--(180,0)--(180,100)--(0,100)--cycle;
%%endpaths
%%beginbody
pair indirectpoint;
indirectpoint := z.p;
% setMarkAnchor(notReal,z.ordinary);
)";
    for (int i = 0; i < 12; ++i)
      source += "setMarkAnchor(overlapping" + QByteArray::number(i) + ",indirectpoint);\n";
    source += "setMarkAnchor(offsetpoint,(0,130)+z.mark.offset);\nenddefchar;\n";
    write(dir.filePath("glyphs.mp"), source);
    Font font;
    require(font.loadFile(dir.filePath("test.mp")), "Load anchor fixture");
    QApplication::restoreOverrideCursor();
    require(font.anchorsVisible(), "Read initial showAnchors from the internal-variable namespace");
    auto* glyph = font.glyphs.first();
    {
      std::unique_ptr<GlyphWindow> initiallyVisible(new GlyphWindow(glyph));
      require(initiallyVisible->findChild<QPushButton*>("anchorsButton")->isChecked(),
          "Button is initially checked when the loaded MetaPost setting is enabled");
    }
    const auto anchorParams = glyph->anchorParameterNames();
    require(anchorParams.contains("z.p") && anchorParams.contains("z.mark.offset") &&
        !anchorParams.contains("z.ordinary"), "Detect indirect anchor controls and preserve unrelated controls");
    font.setAnchorsVisible(false);
    glyph->isDirty = true;
    require(glyph->getEdge(), "Cache a glyph while anchors are hidden");
    const auto originalSource = glyph->source();
    const auto originalWidth = glyph->width();
    const auto originalUndoIndex = glyph->undoStack()->index();
    std::unique_ptr<GlyphWindow> first(new GlyphWindow(glyph)), second(new GlyphWindow(glyph));
    first->setAnimated(false);
    first->resize(1250, 850);
    first->show();
    application.processEvents();
    // Both pages of the tabbed dock must allow the user to allocate more
    // vertical space, and refreshing their contents must keep that allocation.
    for (const auto& name : {QString("anchorsDock"), QString("layersDock")}) {
      auto* dock = first->findChild<QDockWidget*>(name);
      auto* tree = dock->findChild<QTreeWidget*>();
      dock->raise();
      first->resizeDocks({dock}, {230}, Qt::Vertical);
      application.processEvents();
      const int compactHeight = tree->viewport()->height();
      first->resizeDocks({dock}, {480}, Qt::Vertical);
      application.processEvents();
      const int expandedHeight = tree->viewport()->height();
      require(expandedHeight > compactHeight + 20,
              "Anchors and Layers lists must expand when their dock height is increased");
      emit glyph->valueChanged("dock-resize-test", false);
      application.processEvents();
      require(tree->viewport()->height() == expandedHeight,
              "Refreshing anchors or layers must preserve the user-selected dock height");
    }
    first->findChild<QDockWidget*>("anchorsDock")->raise();
    first->resizeDocks({first->findChild<QDockWidget*>("anchorsDock")}, {220}, Qt::Vertical);
    application.processEvents();
    auto* button = first->findChild<QPushButton*>("anchorsButton");
    auto* otherButton = second->findChild<QPushButton*>("anchorsButton");
    auto* view = first->findChild<GlyphView*>();
    require(button && otherButton && view && !button->isChecked(), "Initial button matches hidden state");
    int ordinary = 0, anchor = 0;
    PairItem* editable = nullptr;
    for (auto* item : view->scene()->items()) if (auto* pair = dynamic_cast<PairItem*>(item)) {
      if (pair->isAnchorParameter()) { ++anchor; require(!pair->isVisible(), "Hide anchor controls"); }
      else { ++ordinary; require(pair->isVisible(), "Keep ordinary controls visible"); }
      if (pair->toolTip().contains("z.p")) editable = pair;
    }
    require(anchor == 2 && ordinary == 1 && editable, "Distinct anchor and ordinary point categories");
    otherButton->click();
    application.processEvents();
    require(button->isChecked() && otherButton->isChecked() && font.anchorsVisible(), "Synchronize windows");
    for (auto* item : view->scene()->items()) if (auto* pair = dynamic_cast<PairItem*>(item))
      require(pair->isVisible(), "Show anchor controls again");
    require(editable->flags().testFlag(QGraphicsItem::ItemIsMovable), "Anchor controls remain draggable");
    labelsDoNotOverlap(view);
    if (argc > 2) {
      Font actual;
      require(actual.loadFile(QString::fromLocal8Bit(argv[2])), "Load real-font preview");
      QApplication::restoreOverrideCursor();
      actual.setAnchorsVisible(true);
      for (const auto& name : {QString("behshape.medi.basmala"), QString("hah.init.basmala"), QString("behshape.init")}) {
        auto* realGlyph = actual.glyphperName.value(name);
        require(realGlyph && realGlyph->getEdge(), "Render real basmala glyph");
        const auto unchangedSource = realGlyph->source();
        const auto expectedLabels = realGlyph->getEdge()->numAnchors;
        std::unique_ptr<GlyphWindow> window(new GlyphWindow(realGlyph));
        window->resize(1400, 950); window->show();
        auto* realView = window->findChild<GlyphView*>();
        for (int i = 0; i < 5; ++i) realView->zoomOut();
        application.processEvents();
        labelsDoNotOverlap(realView, expectedLabels);
        require(!window->isWindowModified() && realGlyph->source() == unchangedSource,
            "Inspecting actual anchors must leave real font source unchanged");
        window->grab().save(QString::fromLocal8Bit(argv[1]) + "/" + name + ".png");
        auto* realTree = window->findChild<QTreeWidget*>("anchorsView");
        for (int row = 0; row < realTree->topLevelItemCount(); ++row) {
          auto* anchor = realTree->topLevelItem(row);
          if (anchor->text(1) != "marks_topmarks") continue;
          realTree->setCurrentItem(anchor, 1);
          window->findChild<QComboBox*>("anchorLabelMode")->setCurrentIndex(1);
          application.processEvents();
          labelsDoNotOverlap(realView, 1);
          window->grab().save(QString::fromLocal8Bit(argv[1]) + "/" + name + "-selected.png");
          break;
        }
      }
    }
    // Individual anchor state is per editor, and points/labels stay linked by
    // stable name+type keys rather than their (possibly identical) positions.
    auto* anchorTree = first->findChild<QTreeWidget*>("anchorsView");
    auto* labelMode = first->findChild<QComboBox*>("anchorLabelMode");
    auto* filter = first->findChild<QLineEdit*>("anchorFilter");
    require(anchorTree && labelMode && filter,
            "The anchor dock must expose its inventory and display controls");
    require(anchorTree->topLevelItemCount() == 13, "The dock must include every evaluated anchor");
    auto rowFor = [&](const QString& name) -> QTreeWidgetItem* {
      for (int i = 0; i < anchorTree->topLevelItemCount(); ++i)
        if (anchorTree->topLevelItem(i)->text(1) == name) return anchorTree->topLevelItem(i);
      return nullptr;
    };
    auto visibleCount = [&](const char* kind) {
      int count = 0;
      for (auto* item : view->scene()->items())
        if (item->data(0).toString() == kind && item->isVisible()) ++count;
      return count;
    };
    auto highlightedCount = [&](const char* kind, const QString& key) {
      int count = 0;
      for (auto* item : view->scene()->items())
        if (item->data(0).toString() == kind && item->isVisible() &&
            item->data(SourceLinks::HighlightRole).toBool() &&
            item->data(SourceLinks::KeysRole).toStringList().contains(key))
          ++count;
      return count;
    };
    auto* row = rowFor("overlapping3");
    require(row, "Find a computed anchor even through an indirect point expression");
    const auto key = row->data(1, Qt::UserRole).toString();
    anchorTree->scrollToItem(row);
    application.processEvents();
    const auto rowRect = anchorTree->visualItemRect(row);
    const int nameX = anchorTree->header()->sectionViewportPosition(1) + 35;
    QTest::mouseClick(anchorTree->viewport(), Qt::LeftButton, Qt::NoModifier,
                      QPoint(nameX, rowRect.center().y()));
    application.processEvents();
    require(highlightedCount("anchorMarker", key) == 1 && highlightedCount("anchorLabel", key) == 1,
            "Selecting a row must highlight its exact point and label");
    require(highlightedCount("anchorLeader", key) <= 1,
            "The matching leader belongs to the same anchor identity");
    QTest::mouseClick(anchorTree->viewport(), Qt::LeftButton, Qt::NoModifier,
                      QPoint(12, rowRect.center().y()));
    application.processEvents();
    require(visibleCount("anchorMarker") == 12 && visibleCount("anchorLabel") == 12,
            "Hiding one anchor must hide its point and label only");
    int otherMarkers = 0;
    for (auto* item : second->findChild<GlyphView*>()->scene()->items())
      if (item->data(0).toString() == "anchorMarker" && item->isVisible()) ++otherMarkers;
    require(otherMarkers == 13, "Individual anchor visibility must remain local to its editor");
    labelMode->setCurrentIndex(1);
    anchorTree->setCurrentItem(rowFor("overlapping5"), 1);
    application.processEvents();
    require(visibleCount("anchorMarker") == 12 && visibleCount("anchorLabel") == 1,
            "Selected-label mode must keep other points while showing one matching label");
    labelMode->setCurrentIndex(2);
    require(visibleCount("anchorLabel") == 0 && visibleCount("anchorMarker") == 12,
            "No-label mode must retain anchor points");
    labelMode->setCurrentIndex(0);
    require(visibleCount("anchorLabel") == 12,
            "All-label mode must respect individually hidden anchors");
    filter->setText("offsetpoint");
    require(!rowFor("offsetpoint")->isHidden() && rowFor("overlapping5")->isHidden(),
            "Name filtering must affect the dock list without changing the drawing");
    filter->clear();
    first->findChild<QPushButton*>("hideAllAnchors")->click();
    require(visibleCount("anchorMarker") == 0 && visibleCount("anchorLabel") == 0,
            "Hide all must remove anchor decorations");
    for (auto* item : view->scene()->items())
      if (auto* point = dynamic_cast<PairItem*>(item))
        require(point->isVisible() != point->isAnchorParameter(),
                "Hide all must also hide anchor-only parameter controls");
    anchorTree->setCurrentItem(rowFor("offsetpoint"), 1);
    first->findChild<QPushButton*>("onlySelectedAnchor")->click();
    require(visibleCount("anchorMarker") == 1 && visibleCount("anchorLabel") == 1,
            "Only selected must isolate the anchor and its label");
    for (auto* item : view->scene()->items())
      if (auto* point = dynamic_cast<PairItem*>(item)) {
        if (point->parameterName() == "z.p")
          require(!point->isVisible(), "Controls used only by hidden anchors must be hidden");
        if (point->parameterName() == "z.mark.offset")
          require(point->isVisible(), "The isolated anchor's editable offset must remain visible");
      }
    // Reevaluate the same glyph: the per-anchor choice and selected row survive.
    glyph->isDirty = true;
    require(glyph->getEdge(), "Reevaluate anchor fixture");
    emit glyph->valueChanged("anchors-view-test", false);
    application.processEvents();
    require(
        visibleCount("anchorMarker") == 1 && anchorTree->currentItem()->text(1) == "offsetpoint",
        "Visibility and selection must survive redraw");
    font.setAnchorsVisible(false);
    font.setAnchorsVisible(true);
    require(visibleCount("anchorMarker") == 1,
            "The master switch must preserve individual visibility choices");
    first->findChild<QPushButton*>("showAllAnchors")->click();
    require(visibleCount("anchorMarker") == 13, "Show all must restore every anchor");
    auto* exactRow = rowFor("overlapping9");
    const auto exactKey = exactRow->data(1, Qt::UserRole).toString();
    // Selecting the drawing's computed marker selects its matching dock row.
    for (auto* item : view->scene()->items())
      if (item->data(0).toString() == "anchorMarker" &&
          item->data(SourceLinks::KeysRole).toStringList().contains(exactKey)) {
        view->scene()->clearSelection();
        item->setSelected(true);
        break;
      }
    application.processEvents();
    require(anchorTree->currentItem() == exactRow,
            "Drawing selection must identify the same anchor in the dock");
    labelsDoNotOverlap(view);
    view->zoomIn(); view->zoomIn();
    labelsDoNotOverlap(view);
    view->zoomOut(); view->zoomOut(); view->zoomOut();
    first->resize(1150, 780);
    application.processEvents();
    labelsDoNotOverlap(view);
    require(glyph->source() == originalSource && glyph->width() == originalWidth &&
        glyph->undoStack()->index() == originalUndoIndex && !first->isWindowModified() && !second->isWindowModified(),
        "Visibility changes must not modify source, dimensions, undo history or dirty state");
    if (argc > 1) {
      QDir().mkpath(argv[1]);
      first->grab().save(QString::fromLocal8Bit(argv[1]) + "/anchors-visible.png");
    }
    // MetaPost command edits also update the button and both scenes.
    font.executeMetaPost("showAnchors:=0;");
    require(!button->isChecked() && !otherButton->isChecked(), "Synchronize command-driven visibility");
    for (auto* item : view->scene()->items()) {
      if (item->data(0).toString() == "anchorLabel") require(!item->isVisible(), "Hide computed labels");
    }
    if (argc > 1) first->grab().save(QString::fromLocal8Bit(argv[1]) + "/anchors-hidden.png");
    font.setAnchorsVisible(true);
    QGraphicsSceneMouseEvent move(QEvent::GraphicsSceneMouseMove);
    move.setPos(QPointF(12, 6)); move.setLastPos(QPointF());
    move.setButtons(Qt::LeftButton);
    const auto oldPoint = glyph->params.at("z.p").expr->constantValue(0).toPointF();
    view->scene()->sendEvent(editable, &move);
    require(glyph->params.at("z.p").expr->constantValue(0).toPointF() == oldPoint + QPointF(12, 6),
        "Special marker still edits the anchor's parameter");
    labelsDoNotOverlap(view);
    std::cout
        << "PASS anchor display: master and per-anchor visibility, dock selection, labels, "
           "isolation, dependency controls, redraw persistence, source preservation and editing\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
