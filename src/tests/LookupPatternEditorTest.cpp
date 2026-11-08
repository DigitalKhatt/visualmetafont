#include <QtTest>
#include <QtWidgets>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "LookupCaseGallery.h"
#include "LookupEditorWindow.h"
#include "LookupPatternEditor.h"
#include "LookupPatterns.h"
#include "MPFont.h"
#include "OtLayout.h"
#include "digitalkhatt/project/GlyphSources.h"
namespace fs = std::filesystem;
static std::string read(const fs::path& p) {
  std::ifstream f(p);
  return {std::istreambuf_iterator<char>(f), {}};
}
static void require(bool b, const char* m) {
  if (!b) throw std::runtime_error(m);
}
int main(int argc, char** argv) {
  QApplication app(argc, argv);
  if (argc < 2) return 2;
  try {
    {
      LookupCaseGallery virtualGallery;
      virtualGallery.resize(800, 560);
      int rendered = 0;
      virtualGallery.setCases(std::numeric_limits<quint64>::max(), [&](quint64 index) {
        ++rendered;
        LookupCaseThumbnail thumbnail;
        thumbnail.label = QString::number(index);
        thumbnail.glyphDependencies.insert(index == 0 ? "case0" : "other");
        thumbnail.path.addRect(0, 0, 200, 500);
        return thumbnail;
      });
      virtualGallery.show();
      QElapsedTimer ready;
      ready.start();
      while (virtualGallery.cachedPreviewCount() < 2 && ready.elapsed() < 2000) {
        QApplication::processEvents();
        QTest::qWait(1);
      }
      require(virtualGallery.cachedPreviewCount() > 1,
              "The gallery must show multiple rendered cases together");
      require(rendered < 50,
              "All must render visible tiles rather than eagerly enumerate combinations");
      QElapsedTimer settled;
      settled.start();
      while (!virtualGallery.visiblePreviewsReady() && settled.elapsed() < 2000) {
        QApplication::processEvents();
        QTest::qWait(1);
      }
      require(virtualGallery.visiblePreviewsReady(),
              "Visible tiles must settle before an anchor edit");
      const int renderedBefore = rendered, cachedBefore = virtualGallery.cachedPreviewCount();
      virtualGallery.invalidateGlyph("case0");
      require(virtualGallery.cachedPreviewCount() == cachedBefore,
              "Editing an anchor must retain existing tile images");
      settled.restart();
      while (!virtualGallery.visiblePreviewsReady() && settled.elapsed() < 2000) {
        QApplication::processEvents();
        QTest::qWait(1);
      }
      require(virtualGallery.visiblePreviewsReady() && rendered == renderedBefore + 1,
              "An anchor change must rerender only its dependent tile");
      QTest::mouseClick(virtualGallery.viewport(), Qt::LeftButton, Qt::NoModifier, QPoint(300, 80));
      require(virtualGallery.currentIndex() == 1, "Clicking a gallery tile must select its case");
      virtualGallery.setCurrentIndex(std::numeric_limits<quint64>::max() - 2);
      QApplication::processEvents();
      require(virtualGallery.currentIndex() == std::numeric_limits<quint64>::max() - 2,
              "Virtual scrolling must reach the final cases without allocating the full list");
      require(virtualGallery.cachedPreviewCount() < 50,
              "The preview cache must stay bounded after a large scroll");
    }
    const auto project = fs::absolute(argv[1]);
    const fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    MPFont font;
    font.initialize("MPGUI:=1;" + read(resources / "mfplain.mp") + read(resources / "mpost.mp") +
                        read(resources / "vmf.mp") + read(project),
                    project);
    digitalkhatt::project::ProjectGlyphSources::load(project).registerWith(font);
    OtLayout layout(&font, true, true);
    layout.useNormAxisValues = false;
    layout.loadLookupFile("features.fea");
    const std::string markLookup = layout.lookupsIndexByName.contains("marks")
        ? "marks" : "defaultmarkposition";
    QUndoStack undo;
    {
      // The model header is intentionally included after Qt: Qt's `slots`
      // macro must not change the shared value type's layout across modules.
      const auto patterns = digitalkhatt::layout::lookupPatterns(layout, "defaultdotmarks");
      const QStringList dotNames{"onedotup", "twodotsup", "three_dots", "onedotdown",
                                 "twodotsdown"};
      require(patterns.size() == std::size_t(dotNames.size()),
              "The Qt consumer must see all five dot patterns");
      for (std::size_t i = 0; i < patterns.size(); ++i) {
        require(QString::fromStdString(patterns[i].subtable) == dotNames[int(i)],
                "Dot pattern names must survive the shared-library boundary");
        require(patterns[i].glyphSets.size() == 2 && !patterns[i].glyphSets[0].empty() &&
                    !patterns[i].glyphSets[1].empty(),
                "Dot patterns must retain their base and mark sets in Qt");
      }
      QElapsedTimer elapsed;
      elapsed.start();
      LookupEditorWindow host("defaultdotmarks",
                              new LookupPatternEditor(layout, "defaultdotmarks", &undo));
      auto& dots = *static_cast<LookupPatternEditor*>(host.centralWidget());
      require(bool(host.windowFlags() & Qt::WindowMaximizeButtonHint),
              "Lookup windows must expose a native maximize control");
      host.show();
      QApplication::processEvents();
      auto* maximize = dots.findChild<QPushButton*>("maximizeLookupEditor");
      maximize->click();
      QApplication::processEvents();
      require(host.isMaximized(),
              "The editor maximize button must maximize its independent window");
      maximize->click();
      QApplication::processEvents();
      require(!host.isMaximized(), "Restore must leave maximized mode");
      auto* dotRules = dots.findChild<QComboBox*>("rulePatternSelector");
      auto* dotCases = dots.findChild<LookupCaseGallery*>("patternCaseGallery");
      require(dotRules->count() == dotNames.size() + 1,
              "Dots must expose all groups together and five individual groups");
      for (int i = 0; i < dotRules->count(); ++i) {
        dotRules->setCurrentIndex(i);
        require(dotCases->count() > 0 && dotCases->count() <= 1000,
                "Each dot subtable must have a bounded page of cases");
        require(dots.findChild<QLabel*>("patternAppliedStatus")
                    ->text()
                    .startsWith("Selected rule applied"),
                "Each default dot subtable must preview an applied attachment");
      }
      require(elapsed.elapsed() < 5000,
              "Opening and switching all dot groups must remain responsive");
      std::cout << "All five dot groups opened and previewed in " << elapsed.elapsed() << " ms\n";
    }
    LookupPatternEditor editor(layout, QString::fromStdString(markLookup), &undo);
    editor.resize(1500, 900);
    editor.show();
    editor.findChild<QToolButton*>("editSelectedPattern")->setChecked(true);
    QApplication::processEvents();
    auto* rules = editor.findChild<QComboBox*>("rulePatternSelector");
    auto* cases = editor.findChild<LookupCaseGallery*>("patternCaseGallery");
    auto* limit = editor.findChild<QComboBox*>("patternCaseLimit");
    require(limit && limit->currentData().toULongLong() == 1000,
            "Gallery must default to 1000 cases");
    auto* targets = editor.findChild<QComboBox*>("patternAttachment");
    auto* status = editor.findChild<QLabel*>("patternAppliedStatus");
    require(!editor.findChild<QDoubleSpinBox*>("patternScale"),
            "The mark widget must not expose XScale");
    require(!editor.findChild<QComboBox*>("anchorRole"),
            "Both anchors must be editable without a role selector");
    auto* markX = editor.findChild<QSpinBox*>("markAnchorOffsetX");
    auto* markY = editor.findChild<QSpinBox*>("markAnchorOffsetY");
    require(markX && markY, "Base and mark offsets must both be visible");
    auto* x = editor.findChild<QSpinBox*>("anchorOffsetX");
    auto* y = editor.findChild<QSpinBox*>("anchorOffsetY");
    require(rules && cases && targets && x && y, "Editor controls must exist");
    rules->setCurrentIndex(rules->findText("topmarks / topmarks"));
    QApplication::processEvents();
    require(cases->count() > 0 && cases->count() <= 1000,
            "Rules must page their glyph combinations");
    for (quint64 i = 0; i < cases->count(); ++i)
      if (editor.caseText(i).startsWith("alef.isol · fatha")) {
        cases->setCurrentIndex(i);
        break;
      }
    QApplication::processEvents();
    const auto galleryTotal = digitalkhatt::layout::lookupPatterns(layout, markLookup);
    quint64 total = 0;
    for (const auto& p : galleryTotal)
      if (p.subtable == "topmarks" && p.markClass == "topmarks") total = p.combinations();
    require(cases->count() == std::min<quint64>(1000, total),
            "The default page must expose up to 1000 renderings");
    limit->setCurrentIndex(1);
    require(cases->count() == std::min<quint64>(10000, total),
            "10000 must expose a larger gallery page");
    limit->setCurrentIndex(2);
    require(cases->count() == total, "All must expose every combination in the selected group");
    require(!editor.findChild<QPushButton*>("nextPatternPage")->isEnabled(),
            "All must not hide another page");
    limit->setCurrentIndex(0);
    const auto firstCase = editor.caseText(0);
    if (total > 1000) {
      editor.findChild<QPushButton*>("nextPatternPage")->click();
      require(cases->count() == std::min<quint64>(1000, total - 1000) &&
                  editor.caseText(0) != firstCase,
              "Next must advance to the next block of rendered cases");
      editor.findChild<QPushButton*>("previousPatternPage")->click();
      require(editor.caseText(0) == firstCase, "Previous must restore the first block");
    }
    for (quint64 i = 0; i < cases->count(); ++i)
      if (editor.caseText(i).startsWith("alef.isol · fatha")) {
        cases->setCurrentIndex(i);
        break;
      }
    QApplication::processEvents();
    require(targets->count() > 0, "The rule must expose an actual mark attachment");
    require(status->text().startsWith("Selected rule applied"), qPrintable(status->text()));
    require(x->isEnabled(), "Applied attachments must be editable");
    const int oldX = x->value(), oldY = y->value();
    x->setValue(oldX + 15);
    QTest::keyClick(x, Qt::Key_Return);
    QApplication::processEvents();
    require(undo.count() == 1 && x->value() == oldX + 15,
            "Numeric adjustment must create one undo command and refresh");
    undo.undo();
    QApplication::processEvents();
    require(x->value() == oldX && y->value() == oldY, "Undo must restore the original adjustment");
    undo.redo();
    QApplication::processEvents();
    require(x->value() == oldX + 15, "Redo must restore the edited value");
    auto* reset = editor.findChild<QPushButton*>("resetAnchorAdjustment");
    reset->click();
    QApplication::processEvents();
    require(x->value() == 0 && y->value() == 0, "Reset must remove the adjustment");
    undo.undo();
    QApplication::processEvents();
    require(x->value() == oldX + 15, "Reset must be undoable");
    undo.undo();
    QApplication::processEvents();
    auto* scene = editor.findChild<QGraphicsScene*>("lookupPatternScene");
    auto* view = editor.findChild<QGraphicsView*>("lookupPatternView");
    using namespace digitalkhatt::layout;
    const std::vector<std::uint16_t> selectedSequence{
        std::uint16_t(layout.glyphCodePerName.at("alef.isol")),
        std::uint16_t(layout.glyphCodePerName.at("fatha"))};
    const auto shaped = previewLookupPattern(layout, markLookup, selectedSequence,
                                             std::vector<GlyphParameters>(2), true);
    const auto attached = shaped.attachments.front();
    const AnchorTarget baseTarget{attached.lookup, attached.markClass, "alef.isol",
                                  attached.subtable, AnchorRole::Base};
    const AnchorTarget markTarget{attached.lookup, attached.markClass, "fatha", attached.subtable,
                                  AnchorRole::Mark};
    const auto savedBase = anchorAdjustment(layout, baseTarget),
               savedMark = anchorAdjustment(layout, markTarget);
    auto itemWithTag = [&](QString tag) -> QGraphicsItem* {
      for (auto* item : scene->items())
        if (item->data(0).toString() == tag) return item;
      return nullptr;
    };
    auto moveTo = [&](QPoint point) {
      QMouseEvent move(QEvent::MouseMove, QPointF(point), Qt::NoButton, Qt::LeftButton,
                       Qt::NoModifier);
      QApplication::sendEvent(view->viewport(), &move);
    };
    auto* body = itemWithTag("lookupMarkBody");
    require(body, "The selected mark must be directly draggable");
    auto* pathBody = static_cast<QGraphicsPathItem*>(body);
    auto local = pathBody->path().boundingRect().center();
    if (!pathBody->path().contains(local)) {
      const auto rect = pathBody->path().boundingRect();
      for (int a = 1; a < 10; ++a)
        for (int b = 1; b < 10; ++b) {
          const auto p = rect.topLeft() + QPointF(a * rect.width() / 10, b * rect.height() / 10);
          if (pathBody->path().contains(p)) local = p;
        }
    }
    const auto bodyStart = view->mapFromScene(body->mapToScene(local));
    const auto bodyPosition = body->scenePos();
    const int beforeGesture = undo.index();
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, bodyStart);
    moveTo(bodyStart + QPoint(20, -12));
    require(scene->mouseGrabberItem() == body,
            "Live updates must preserve the item under the mouse");
    require(anchorAdjustment(layout, baseTarget) != savedBase &&
                anchorAdjustment(layout, markTarget) == savedMark,
            "Dragging the mark must change only its base anchor before release");
    require(body->scenePos() != bodyPosition, "The rendered mark must move during mouse movement");
    const auto firstMove = body->scenePos();
    moveTo(bodyStart + QPoint(40, -24));
    require(body->scenePos() != firstMove,
            "Further pointer movement must continue updating the rendering");
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier,
                        bodyStart + QPoint(40, -24));
    QApplication::processEvents();
    require(undo.index() == beforeGesture + 1, "A continuous mark drag must be one undo step");
    undo.undo();
    QApplication::processEvents();
    require(anchorAdjustment(layout, baseTarget) == savedBase &&
                anchorAdjustment(layout, markTarget) == savedMark,
            "Undo must restore the full mark drag");
    auto* handle = itemWithTag("lookupAnchorHandle");
    body = itemWithTag("lookupMarkBody");
    require(handle && body, "Both direct editing targets must remain available");
    const auto anchorStart = view->mapFromScene(handle->scenePos());
    const auto oldConnection = handle->scenePos(), oldMarkOrigin = body->scenePos();
    const auto oldMarkPoint = describeAnchor(layout, markTarget, {}).resolved;
    const int beforeAnchorGesture = undo.index();
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, anchorStart);
    moveTo(anchorStart + QPoint(20, -12));
    require(scene->mouseGrabberItem() == handle,
            "The anchor dot must remain grabbed during live reshaping");
    require(anchorAdjustment(layout, markTarget) != savedMark &&
                anchorAdjustment(layout, baseTarget) == savedBase,
            "Dragging the dot must change only the mark anchor before release");
    require(body->scenePos() != oldMarkOrigin,
            "Changing the mark anchor must immediately reposition its outline");
    const auto newMarkPoint = describeAnchor(layout, markTarget, {}).resolved;
    const auto fixedConnection = body->scenePos() + QPointF(newMarkPoint.x(), newMarkPoint.y());
    require(QLineF(fixedConnection, oldConnection).length() < 2,
            "The mark must remain attached to the fixed base anchor");
    require((body->scenePos() - oldMarkOrigin +
             QPointF(newMarkPoint.x() - oldMarkPoint.x(), newMarkPoint.y() - oldMarkPoint.y()))
                    .manhattanLength() < 2,
            "Mark origin movement must compensate for the changed local mark anchor");
    moveTo(anchorStart + QPoint(30, -18));
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier,
                        anchorStart + QPoint(30, -18));
    QApplication::processEvents();
    require(undo.index() == beforeAnchorGesture + 1,
            "A continuous anchor-dot drag must be one undo step");
    undo.undo();
    QApplication::processEvents();
    require(anchorAdjustment(layout, baseTarget) == savedBase &&
                anchorAdjustment(layout, markTarget) == savedMark,
            "Undo must restore both anchors after a dot drag");
    require(x->value() == oldX && y->value() == oldY,
            "Dragging must preserve the original adjustments after undo");
    auto* compare = editor.findChild<QCheckBox*>("comparePatternVariations");
    compare->setChecked(true);
    QApplication::processEvents();
    require(status->text().startsWith("Selected rule applied"),
            "Stretch comparisons must preserve the selected rule");
    bool requested = false;
    QObject::connect(&editor, &LookupPatternEditor::saveRequested, [&] { requested = true; });
    editor.findChild<QPushButton*>("saveLookupAdjustments")->click();
    require(requested, "Save must reach the parent document");
    LookupPatternEditor context(layout, "lowmarkafterwawandreh.mark", &undo);
    context.resize(1100, 760);
    context.show();
    QApplication::processEvents();
    require(context.findChild<LookupCaseGallery*>("patternCaseGallery")->count() > 0,
            "Contextual rules must expose paged witnesses");
    if (argc > 2) {
      compare->setChecked(false);
      const auto waitForTiles = [&] {
        QElapsedTimer ready;
        ready.start();
        while (!cases->visiblePreviewsReady() && ready.elapsed() < 5000) {
          QApplication::processEvents();
          QTest::qWait(1);
        }
        require(cases->visiblePreviewsReady(), "Visible gallery tiles must finish rendering");
      };
      waitForTiles();
      const QFileInfo output(QString::fromLocal8Bit(argv[2]));
      require(editor.grab().save(output.dir().filePath(output.completeBaseName() + "-edit.png")),
              "Save editing preview");
      editor.findChild<QToolButton*>("editSelectedPattern")->setChecked(false);
      rules->setCurrentIndex(0);
      cases->setCurrentIndex(0);
      QApplication::processEvents();
      waitForTiles();
      require(editor.grab().save(QString::fromLocal8Bit(argv[2])), "Save gallery preview");
    }
    std::cout << "Rule editor: bounded cases, actual attachments, numeric edits, drag, undo/redo, "
                 "reset and variations passed\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
