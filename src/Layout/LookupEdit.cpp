#include "LayoutWindow.h"
#include "LookupPatternEditor.h"
#include "LookupEditorWindow.h"
#include "font.hpp"
#include "glyphwindow.h"
#include "GraphicsSceneAdjustment.h"
#include <memory>
#include <QtWidgets>

void LayoutWindow::editLookup(QString lookupName) {
  const auto objectName = QStringLiteral("lookupEditor.") + lookupName;
  if (auto* existing = findChild<QMainWindow*>(objectName)) {
    existing->show();
    existing->raise();
    return;
  }
  if (!lookupUndo_) {
    lookupUndo_ = new QUndoStack(this);
    lookupUndo_->setObjectName("lookupAdjustmentUndo");
    auto* menu = menuBar()->addMenu(tr("Adjustments"));
    auto* undo = lookupUndo_->createUndoAction(this, tr("Undo adjustment"));
    undo->setShortcuts(QKeySequence::Undo);
    menu->addAction(undo);
    auto* redo = lookupUndo_->createRedoAction(this, tr("Redo adjustment"));
    redo->setShortcuts(QKeySequence::Redo);
    menu->addAction(redo);
    connect(lookupUndo_, &QUndoStack::indexChanged, this, [this] {
      setWindowModified(positioningExternalDirty_ || !lookupUndo_->isClean());
      if (!m_graphicsScene->items().isEmpty()) executeRunText(true, 0);
    });
  }
  try {
    auto* editor = new LookupPatternEditor(*m_otlayout, lookupName, lookupUndo_);
    auto windowOwner = std::make_unique<LookupEditorWindow>(lookupName, editor, this);
    auto* window = windowOwner.get();
    window->setObjectName(objectName);
    connect(lookupUndo_, &QUndoStack::cleanChanged, window,
            [this, window] { window->setWindowModified(!lookupUndo_->isClean()); });
    window->setWindowModified(!lookupUndo_->isClean());
    connect(editor, &LookupPatternEditor::draggingChanged, window, [this, window](bool active) {
      window->setWindowModified(active || !lookupUndo_->isClean());
      setWindowModified(active || positioningExternalDirty_ || !lookupUndo_->isClean());
    });
    connect(editor, &LookupPatternEditor::saveRequested, this, [this] {
      if (!save()) statusBar()->showMessage(tr("Could not save positioning adjustments"), 5000);
    });
    connect(editor, &LookupPatternEditor::glyphEditRequested, this, [this](const QString& name) {
      auto found = m_font->glyphperName.find(name);
      if (found == m_font->glyphperName.end()) return;
      auto* window = new GlyphWindow(found.value());
      window->showMaximized();
    });
    connect(this, &LayoutWindow::positioningReloading, window, &QWidget::close);
    connect(this, &LayoutWindow::positioningParametersChanged, editor,
            &LookupPatternEditor::refreshPreview);
    auto* action = new QAction(lookupName, window);
    viewMenu->addAction(action);
    connect(action, &QAction::triggered, window, [window] {
      window->show();
      window->raise();
      window->activateWindow();
    });
    window->show();
    window->raise();
    windowOwner.release();
  } catch (const std::exception& error) {
    statusBar()->showMessage(QString::fromUtf8(error.what()), 7000);
  }
}
