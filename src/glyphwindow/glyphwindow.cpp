/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include "glyphwindow.h"

#include <QtWidgets>

#include "commands.h"
#include "font.hpp"
#include "glyphparametercontroller.hpp"
#include "pathview.h"
#include "nibenvelope.h"
#include "metaposteditor.hpp"

namespace {
class LayersView : public QTreeWidget {
 public:
  using QTreeWidget::QTreeWidget;
 protected:
  void mousePressEvent(QMouseEvent* event) override {
    const auto index=indexAt(event->pos());
    if (event->button()==Qt::LeftButton && index.isValid() && (index.column()>=0 && index.column()<=2)) {
      togglePress_=true;
      pressedToggle_=index;
      event->accept();
      return; // Name/eye/lock controls do not change the new-path destination.
    }
    QTreeWidget::mousePressEvent(event);
  }
  void mouseReleaseEvent(QMouseEvent* event) override {
    if (togglePress_ && event->button()==Qt::LeftButton) {
      const auto pressed=pressedToggle_;
      togglePress_=false;
      pressedToggle_=QPersistentModelIndex();
      if (pressed.isValid() && indexAt(event->pos())==pressed) {
        auto* item=itemFromIndex(pressed);
        const int column=pressed.column()==2 ? 2 : 0;
        item->setCheckState(column,item->checkState(column)==Qt::Checked ? Qt::Unchecked : Qt::Checked);
      }
      event->accept();
      return; // Consume the release too; Qt's checkbox delegate must not toggle again.
    }
    QTreeWidget::mouseReleaseEvent(event);
  }
  void mouseMoveEvent(QMouseEvent* event) override {
    if (togglePress_) { event->accept(); return; }
    QTreeWidget::mouseMoveEvent(event);
  }
  void keyPressEvent(QKeyEvent* event) override {
    if (event->key()==Qt::Key_Space && currentItem()) {
      if (!event->isAutoRepeat()) {
        const int column=currentColumn()==2 ? 2 : 0;
        auto* item=currentItem();
        item->setCheckState(column,item->checkState(column)==Qt::Checked ? Qt::Unchecked : Qt::Checked);
      }
      event->accept(); return;
    }
    QTreeWidget::keyPressEvent(event);
  }
  void mouseDoubleClickEvent(QMouseEvent* event) override {
    const auto index=indexAt(event->pos());
    if (event->button()==Qt::LeftButton && index.isValid() && (index.column()>=0 && index.column()<=2)) {
      mousePressEvent(event); // The second press arrives as a double-click event.
      return;
    }
    QTreeWidget::mouseDoubleClickEvent(event);
  }
 private:
  bool togglePress_=false;
  QPersistentModelIndex pressedToggle_;
};
}

GlyphWindow::GlyphWindow(Glyph* glyph) {
  setAttribute(Qt::WA_DeleteOnClose);

  undoStack = glyph->undoStack();
  // glyph->setSource(glyph->source());

  this->glyph = glyph;
  updateWindowTitle();

  connect(glyph, &Glyph::valueChanged, this, &GlyphWindow::glyphChanged);

  glyphView = new GlyphView(glyph);

  scene = glyphView->getScene();

  createActions();
  createStatusBar();
  createDockWindows();
  connect(glyph->font,&Font::sourcesSaved,this,&GlyphWindow::updateModifiedState);
  updateModifiedState();
  connect(scene,&GlyphScene::picturesChanged,this,&GlyphWindow::refreshLayersView);
  refreshLayersView();
  connect(scene, &GlyphScene::anchorsChanged, this, &GlyphWindow::refreshAnchorsView);
  connect(scene, &GlyphScene::anchorSelectionChanged, this, &GlyphWindow::syncAnchorSelection);
  refreshAnchorsView();
  connect(scene,&QGraphicsScene::selectionChanged,this,&GlyphWindow::drawingSelectionChanged);
  connect(mpostEdit,&MetaPostEditor::declarationSelected,this,[this](const QStringList& keys) {
    scene->highlightSourceKeys(keys);
    bool hidden=false;
    if (!keys.isEmpty()) for (auto* item:scene->items()) {
      if (item->data(SourceLinks::HighlightRole).toBool()) {
        if (item->isVisible()) { glyphView->ensureVisible(item,30,30); return; }
        hidden=true;
      }
    }
    if (hidden) statusBar()->showMessage(tr("Matching controls are hidden. Check Show and Lock in Layers, and the Anchors button."),5000);
  });

  this->setCentralWidget(glyphView);

  updateWindowTitle();

  glyphView->setFocus();

  statusBar()->addPermanentWidget(scene->pointerPosition);
}
void GlyphWindow::createLayersView() {
  auto* dock=new QDockWidget(tr("Layers"),this);
  dock->setObjectName("layersDock");
  auto* page=new QWidget(dock);
  auto* layout=new QVBoxLayout(page);
  layout->setContentsMargins(6,6,6,6);
  finalPositionsControl=new QCheckBox(tr("Final positions"),page);
  finalPositionsControl->setObjectName("finalPositions");
  finalPositionsControl->setChecked(scene->finalPositions());
  finalPositionsControl->setToolTip(tr("Apply the final glyph origin and scale to named pictures. Uncheck to edit source coordinates."));
  connect(finalPositionsControl,&QCheckBox::toggled,scene,&GlyphScene::setFinalPositions);
  layout->addWidget(finalPositionsControl);
  layersView=new LayersView(page);
  layersView->setObjectName("layersView");
  layersView->setColumnCount(3);
  layersView->setHeaderLabels({tr("Show"),tr("Layer"),tr("Lock")});
  layersView->setRootIsDecorated(false);
  layersView->setUniformRowHeights(true);
  layersView->setSelectionMode(QAbstractItemView::SingleSelection);
  layersView->setAllColumnsShowFocus(true);
  layersView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  layersView->setMinimumHeight(layersView->header()->sizeHint().height()+layersView->fontMetrics().height()+8);
  layersView->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
  layersView->header()->setSectionResizeMode(1,QHeaderView::Stretch);
  layersView->header()->setSectionResizeMode(2,QHeaderView::ResizeToContents);
  layersView->header()->setStretchLastSection(false);
  layout->addWidget(layersView);
  layersView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(layersView,&QTreeWidget::customContextMenuRequested,this,[this](const QPoint& position) {
    auto* item=layersView->itemAt(position);
    if (!item) return;
    const auto name=item->data(1,Qt::UserRole).toString();
    if (name==GlyphScene::imageLayerName()) return;
    QMenu menu(this);
    auto* destination=menu.addAction(tr("Use for new paths"));
    destination->setCheckable(true); destination->setChecked(name==scene->pictureName());
    destination->setEnabled(!scene->layerLocked(name));
    if (menu.exec(layersView->viewport()->mapToGlobal(position))==destination) {
      glyphView->setPictureName(name);
      layersView->setCurrentItem(item,1);
      statusBar()->showMessage(tr("New paths will be added to %1.").arg(name.isEmpty() ? tr("Final glyph") : name),5000);
    }
  });
  connect(layersView,&QTreeWidget::itemChanged,this,[this](QTreeWidgetItem* item,int column) {
    const auto name=item->data(1,Qt::UserRole).toString();
    // A scene change refreshes the row, so read its state before calling it.
    const bool checked=item->checkState(column)==Qt::Checked;
    if (column==0) scene->setLayerVisible(name,checked);
    else if (column==2) scene->setLayerLocked(name,checked);
  });
  dock->setWidget(page);
  addDockWidget(Qt::RightDockWidgetArea,dock);
  viewMenu->addAction(dock->toggleViewAction());
}
void GlyphWindow::refreshLayersView() {
  if (!layersView) return;
  if (finalPositionsControl) {
    const QSignalBlocker blocker(finalPositionsControl);
    finalPositionsControl->setEnabled(!scene->pictureNames().isEmpty());
    finalPositionsControl->setChecked(scene->finalPositions());
  }
  const QSignalBlocker blocker(layersView);
  auto names=scene->pictureNames(); names.prepend(QString());
  if (scene->hasImage()) names.append(GlyphScene::imageLayerName());
  // Reuse rows on ordinary renders: changing an eye or lock must not destroy
  // the current item while Qt is processing its click.
  QStringList existing;
  for (int i=0;i<layersView->topLevelItemCount();++i)
    existing.append(layersView->topLevelItem(i)->data(1,Qt::UserRole).toString());
  if (existing!=names) {
    layersView->clear();
    for (const auto& name:names) {
      auto* item=new QTreeWidgetItem(layersView);
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
      item->setData(1,Qt::UserRole,name);
      item->setText(1,name.isEmpty() ? tr("Final glyph") : name==GlyphScene::imageLayerName() ? tr("Image") : name);
      item->setToolTip(0,tr("Visibility in this editor; font output is unchanged"));
      item->setToolTip(2,tr("Prevent graphical edits to this layer"));
      if (name==scene->pictureName()) layersView->setCurrentItem(item,1);
    }
  }
  for (int i=0;i<names.size();++i) {
    auto* item=layersView->topLevelItem(i);
    const auto& name=names[i];
    item->setCheckState(0,scene->layerVisible(name) ? Qt::Checked : Qt::Unchecked);
    item->setCheckState(2,scene->layerLocked(name) ? Qt::Checked : Qt::Unchecked);
    auto font=item->font(1); font.setBold(name==scene->pictureName()); item->setFont(1,font);
    item->setToolTip(1,name==GlyphScene::imageLayerName()
        ? tr("Click to show/hide the image. Lock prevents moving or scaling it.\n%1").arg(glyph->image().path)
        : tr("Click to show/hide. Right-click to use this layer for new paths."));
  }
  // Leave dock sizing to the user; row updates must not reset its height.
}

namespace {
QString anchorTypeName(int type) {
  switch (type) {
    case 1:
      return QObject::tr("Mark");
    case 2:
      return QObject::tr("Entry LTR");
    case 3:
      return QObject::tr("Exit LTR");
    case 4:
      return QObject::tr("Entry RTL");
    case 5:
      return QObject::tr("Exit RTL");
    case 6:
      return QObject::tr("General");
    default:
      return QString::number(type);
  }
}
}  // namespace
void GlyphWindow::createAnchorsView() {
  auto* dock = new QDockWidget(tr("Anchors"), this);
  dock->setObjectName("anchorsDock");
  auto* page = new QWidget(dock);
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(6, 6, 6, 6);
  auto* options = new QHBoxLayout;
  anchorMaster = new QCheckBox(tr("Show anchors"));
  anchorMaster->setObjectName("anchorMasterVisible");
  anchorMaster->setChecked(glyph->font->anchorsVisible());
  options->addWidget(anchorMaster);
  anchorLabelMode = new QComboBox;
  anchorLabelMode->setObjectName("anchorLabelMode");
  anchorLabelMode->addItems({tr("All labels"), tr("Selected label"), tr("No labels")});
  options->addWidget(anchorLabelMode, 1);
  layout->addLayout(options);
  anchorFilter = new QLineEdit;
  anchorFilter->setObjectName("anchorFilter");
  anchorFilter->setPlaceholderText(tr("Filter anchors by name or type"));
  anchorFilter->setClearButtonEnabled(true);
  layout->addWidget(anchorFilter);
  anchorsView = new QTreeWidget;
  anchorsView->setObjectName("anchorsView");
  anchorsView->setColumnCount(2);
  anchorsView->setHeaderLabels({tr("Show"), tr("Anchor")});
  anchorsView->setRootIsDecorated(false);
  anchorsView->setUniformRowHeights(true);
  anchorsView->setSelectionMode(QAbstractItemView::SingleSelection);
  anchorsView->setAllColumnsShowFocus(true);
  anchorsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  anchorsView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  anchorsView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  anchorsView->header()->setStretchLastSection(false);
  layout->addWidget(anchorsView);
  auto* buttons = new QHBoxLayout;
  auto* showAll = new QPushButton(tr("Show all"));
  showAll->setObjectName("showAllAnchors");
  auto* hideAll = new QPushButton(tr("Hide all"));
  hideAll->setObjectName("hideAllAnchors");
  onlySelectedAnchor = new QPushButton(tr("Only selected"));
  onlySelectedAnchor->setObjectName("onlySelectedAnchor");
  onlySelectedAnchor->setEnabled(false);
  for (auto* button : {showAll, hideAll, onlySelectedAnchor}) buttons->addWidget(button);
  layout->addLayout(buttons);
  connect(anchorMaster, &QCheckBox::toggled, glyph->font, &Font::setAnchorsVisible);
  connect(anchorLabelMode, qOverload<int>(&QComboBox::currentIndexChanged), scene,
          &GlyphScene::setAnchorLabelMode);
  connect(anchorFilter, &QLineEdit::textChanged, this, [this] { refreshAnchorsView(); });
  connect(showAll, &QPushButton::clicked, this, [this] {
    glyph->font->setAnchorsVisible(true);
    scene->showAllAnchors(true);
  });
  connect(hideAll, &QPushButton::clicked, this, [this] { scene->showAllAnchors(false); });
  connect(onlySelectedAnchor, &QPushButton::clicked, this, [this] {
    if (auto* row = anchorsView->currentItem()) {
      const auto key = row->data(1, Qt::UserRole).toString();
      glyph->font->setAnchorsVisible(true);
      scene->showOnlyAnchor(key);
    }
  });
  connect(anchorsView, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* row, int column) {
    if (column != 0) return;
    const auto key = row->data(1, Qt::UserRole).toString();
    const bool visible = row->checkState(0) == Qt::Checked;
    if (visible) glyph->font->setAnchorsVisible(true);
    scene->setAnchorVisible(key, visible);
  });
  connect(anchorsView, &QTreeWidget::currentItemChanged, this,
          [this](QTreeWidgetItem* row, QTreeWidgetItem*) {
            onlySelectedAnchor->setEnabled(row != nullptr);
            if (!row) return;
            const auto key = row->data(1, Qt::UserRole).toString();
            scene->focusAnchor(key);
            mpostEdit->highlightDeclarations({key});
            for (auto* item : scene->items())
              if (item->data(0).toString() == "anchorMarker" && item->isVisible() &&
                  item->data(SourceLinks::KeysRole).toStringList().contains(key)) {
                glyphView->ensureVisible(item, 30, 30);
                break;
              }
          });
  dock->setWidget(page);
  addDockWidget(Qt::RightDockWidgetArea, dock);
  viewMenu->addAction(dock->toggleViewAction());
  if (auto* layers = findChild<QDockWidget*>("layersDock")) tabifyDockWidget(layers, dock);
  dock->raise();
}
void GlyphWindow::syncAnchorSelection(const QString& key) {
  if (!anchorsView) return;
  const QSignalBlocker blocker(anchorsView);
  QTreeWidgetItem* selected = nullptr;
  for (int i = 0; i < anchorsView->topLevelItemCount(); ++i) {
    auto* row = anchorsView->topLevelItem(i);
    if (row->data(1, Qt::UserRole).toString() == key) {
      selected = row;
      break;
    }
  }
  anchorsView->setCurrentItem(selected);
  onlySelectedAnchor->setEnabled(selected != nullptr);
  if (selected && !selected->isHidden()) anchorsView->scrollToItem(selected);
}
void GlyphWindow::refreshAnchorsView() {
  if (!anchorsView) return;
  {
    QSignalBlocker block(anchorMaster);
    anchorMaster->setChecked(glyph->font->anchorsVisible());
  }
  {
    QSignalBlocker block(anchorLabelMode);
    anchorLabelMode->setCurrentIndex(scene->anchorLabelMode());
  }
  const auto infos = scene->anchorInfos();
  QStringList keys, existing;
  for (const auto& info : infos) keys.append(info.key);
  for (int i = 0; i < anchorsView->topLevelItemCount(); ++i)
    existing.append(anchorsView->topLevelItem(i)->data(1, Qt::UserRole).toString());
  const QSignalBlocker blocker(anchorsView);
  if (keys != existing) {
    anchorsView->clear();
    for (const auto& info : infos) {
      auto* row = new QTreeWidgetItem(anchorsView);
      row->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
      row->setData(1, Qt::UserRole, info.key);
    }
  }
  for (int i = 0; i < infos.size(); ++i) {
    const auto& info = infos[i];
    auto* row = anchorsView->topLevelItem(i);
    row->setText(1, info.name);
    row->setCheckState(0, scene->anchorVisible(info.key) ? Qt::Checked : Qt::Unchecked);
    QStringList layers;
    for (const auto& layer : info.layers)
      layers.append(layer.isEmpty() ? tr("Final glyph") : layer);
    const auto position = QString("%1, %2").arg(info.point.x(), 0, 'g', 7).arg(info.point.y(), 0, 'g', 7);
    const auto tooltip = info.name + "\n" + anchorTypeName(info.type) + " · " + position +
                         "\n" + tr("Layers: %1").arg(layers.join(", "));
    for (int col = 0; col < anchorsView->columnCount(); ++col) row->setToolTip(col, tooltip);
    row->setHidden(
        !(info.name + " " + anchorTypeName(info.type)).contains(anchorFilter->text(), Qt::CaseInsensitive));
  }
  const int height = qMax(anchorsView->sizeHintForRow(0), anchorsView->fontMetrics().height() + 6);
  anchorsView->setMinimumHeight(anchorsView->header()->sizeHint().height() + 3 * height +
                                2 * anchorsView->frameWidth());
  syncAnchorSelection(scene->highlightedAnchor());
}

void GlyphWindow::executeCommands(void) {
  if (mpostEdit->hasConflict()) {
    statusBar()->showMessage(tr("Resolve the code/drawing conflict before executing."));
    return;
  }
  const auto source = mpostEdit->toPlainText();
  if (source != glyph->source()) {
    mpostEdit->beginExecution();
    glyph->undoStack()->push(new GlyphSourceChangeCommand(glyph, "Source Changed", glyph->source(), source, true));
  }

  glyph->getEdge();

  logOutput->setPlainText(glyph->getLog());
  infoOutput->setPlainText(glyph->getInfo());

  // glyphView->setGlyph(glyph);
}

void GlyphWindow::glyphChanged(QString name) {
  if (mpostEdit) {
    updateSourceTopology();
    mpostEdit->synchronizeSource(glyph->source());
  }
  updateModifiedState();
}
void GlyphWindow::updateModifiedState() {
  setWindowModified(glyph->font->glyphHasSourceChanges(glyph) ||
      (mpostEdit && (mpostEdit->hasPendingChanges() || mpostEdit->hasConflict())));
  updateWindowTitle();
}
void GlyphWindow::updateWindowTitle() {
  auto title=glyph->name()+" -  "+glyph->font->fontName();
  // Cocoa uses its native edited indicator rather than a visible asterisk.
  // Retain Qt's placeholder for native modified-state handling on all platforms.
  if (QGuiApplication::platformName()=="cocoa" && isWindowModified()) title+=" *";
  setWindowTitle(title+"[*]");
}
void GlyphWindow::changeEvent(QEvent* event) {
  QMainWindow::changeEvent(event);
  if (event->type()==QEvent::ModifiedChange) updateWindowTitle();
}
void GlyphWindow::updateSourceTopology() {
  QMap<int,QSet<int>> topology;
  for (auto path=glyph->controlledPaths.cbegin();path!=glyph->controlledPaths.cend();++path) {
    for (const auto point:glyph->authoredPointIds(path.key())) topology[path.key()].insert(point);
  }
  mpostEdit->setPointTopology(topology);
}
void GlyphWindow::drawingSelectionChanged() {
  if (scene->isRebuilding()) return;
  QStringList keys;
  for (auto* item:scene->selectedItems()) keys+=item->data(SourceLinks::KeysRole).toStringList();
  const bool found=mpostEdit->highlightDeclarations(keys);
  scene->highlightSourceKeys(keys);
  if (found) {
    metapostCode->show(); tabWidget->setCurrentIndex(0);
  } else if (!keys.isEmpty()) statusBar()->showMessage(tr("No matching declaration in this glyph's current code; it may be inherited, generated, or changed in the draft."),5000);
}
void GlyphWindow::createDockWindows() {
  metapostCode = new QDockWidget(tr("MetaFont Code"), this);
  metapostCode->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);

  QFont textEditFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  mpostEdit = new MetaPostEditor(metapostCode);

  executeButton = new QPushButton("&Execute", metapostCode);
  executeButton->setObjectName("executeCode");
  connect(executeButton, SIGNAL(clicked()), this, SLOT(executeCommands()));

  QVBoxLayout* mpostLayout = new QVBoxLayout;
  mpostLayout->addWidget(mpostEdit);
  mpostLayout->addWidget(executeButton);

  QWidget* mpostPage = new QWidget(metapostCode);
  mpostPage->setLayout(mpostLayout);

#define CREATE(x)         \
  x = new QPlainTextEdit; \
  x->setReadOnly(true);   \
  x->setFont(textEditFont);

  CREATE(logOutput)
  CREATE(infoOutput)

#undef CREATE

  tabWidget = new QTabWidget(metapostCode);
  tabWidget->addTab(mpostPage, "Input");
  tabWidget->addTab(logOutput, "Log");
  tabWidget->addTab(infoOutput, "Info");

  mpostEdit->initializeSource(glyph->source());
  updateSourceTopology();
  connect(mpostEdit, &MetaPostEditor::draftStateChanged, this, [this] {
    updateModifiedState();
    executeButton->setEnabled(!mpostEdit->hasConflict());
    updateUndoActions();
  });
  connect(mpostEdit->textEdit(), &QPlainTextEdit::undoAvailable, this, &GlyphWindow::updateUndoActions);
  connect(mpostEdit->textEdit(), &QPlainTextEdit::redoAvailable, this, &GlyphWindow::updateUndoActions);
  for (auto* input : mpostEdit->findChildren<QLineEdit*>())
    connect(input, &QLineEdit::textChanged, this, &GlyphWindow::updateUndoActions);

  metapostCode->setWidget(tabWidget);
  addDockWidget(Qt::LeftDockWidgetArea, metapostCode);
  viewMenu->addAction(metapostCode->toggleViewAction());

  createUndoView();
  generatePropertiesView();

  tabifyDockWidget(dockUndo, dockProprties);

  createPathView();
  createLayersView();
  createAnchorsView();
}

void GlyphWindow::createPathView() {
  dockPath = new QDockWidget(this);
  dockPath->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  dockPath->setObjectName(QStringLiteral("Path"));

  auto pathView = new PathView(glyph, this);

  dockPath->setWidget(pathView);
  addDockWidget(Qt::RightDockWidgetArea, dockPath);
  dockPath->setWindowTitle("Path");

  viewMenu->addAction(dockPath->toggleViewAction());
}
void GlyphWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  if (!dockLayoutInitialized_) {
    dockLayoutInitialized_=true;
    resizeDocks({dockUndo, dockPath, findChild<QDockWidget*>("anchorsDock")}, {140, 400, 220},
                Qt::Vertical);
  }
}
void GlyphWindow::resizeEvent(QResizeEvent* event) {
  QMainWindow::resizeEvent(event);
  if (tabWidget != NULL) {
    this->resizeDocks({metapostCode, dockUndo}, {this->width() / 4, this->width() / 4}, Qt::Horizontal);
    // tabWidget->setMinimumWidth(this->width() / 3);
    // tabWidget->resize(this->width() / 3, tabWidget->height());
  }
}
void GlyphWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

void GlyphWindow::createActions() {
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileToolBar = addToolBar(tr("File"));

  const QIcon importIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
  QAction* importImage = new QAction(importIcon, tr("&Import Image"), this);
  importImage->setShortcuts(QKeySequence::Open);
  importImage->setStatusTip(tr("Import image as background"));
  connect(importImage, &QAction::triggered, this, &GlyphWindow::importImage);
  fileMenu->addAction(importImage);
  fileToolBar->addAction(importImage);

  const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
  QAction* saveAct = new QAction(saveIcon, tr("&Save..."), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the current form letter"));
  connect(saveAct, &QAction::triggered, this, &GlyphWindow::save);
  fileMenu->addAction(saveAct);
  fileToolBar->addAction(saveAct);

  const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(":/images/print.png"));
  QAction* printAct = new QAction(printIcon, tr("&Print..."), this);
  printAct->setShortcuts(QKeySequence::Print);
  printAct->setStatusTip(tr("Print the current form letter"));
  // connect(printAct, &QAction::triggered, this, &MainWindow::print);
  fileMenu->addAction(printAct);
  fileToolBar->addAction(printAct);

  fileMenu->addSeparator();

  QAction* quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
  quitAct->setShortcuts(QKeySequence::Quit);
  quitAct->setStatusTip(tr("Quit the application"));

  QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
  editToolBar = addToolBar(tr("Edit"));

  undoAction = new QAction(tr("Undo"), this);
  redoAction = new QAction(tr("Redo"), this);
  undoAction->setShortcuts(QKeySequence::Undo);
  redoAction->setShortcuts(QKeySequence::Redo);
  const auto performUndo = [this](bool redo) {
    auto* focus = QApplication::focusWidget();
    if (mpostEdit && focus && (focus == mpostEdit->textEdit() || mpostEdit->isAncestorOf(focus))) {
      if (auto* line = qobject_cast<QLineEdit*>(focus)) { if (redo) line->redo(); else line->undo(); }
      else { if (redo) mpostEdit->textEdit()->redo(); else mpostEdit->textEdit()->undo(); }
    } else { if (redo) undoStack->redo(); else undoStack->undo(); }
  };
  connect(undoAction, &QAction::triggered, this, [performUndo] { performUndo(false); });
  connect(redoAction, &QAction::triggered, this, [performUndo] { performUndo(true); });
  connect(undoStack, &QUndoStack::canUndoChanged, this, &GlyphWindow::updateUndoActions);
  connect(undoStack, &QUndoStack::canRedoChanged, this, &GlyphWindow::updateUndoActions);
  connect(qApp, &QApplication::focusChanged, this, &GlyphWindow::updateUndoActions);
  updateUndoActions();

  undoAction->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/images/undo.png")));
  redoAction->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/images/redo.png")));
  editMenu->addAction(undoAction);
  editToolBar->addAction(undoAction);
  editMenu->addAction(redoAction);
  editToolBar->addAction(redoAction);
  editToolBar->addSeparator();

  editMenu->addSeparator();
  auto* findAction = editMenu->addAction(tr("Find…"), this, [this] {
    metapostCode->show(); tabWidget->setCurrentIndex(0); mpostEdit->showFind();
  });
  findAction->setShortcuts(QKeySequence::Find);
  auto* replaceAction = editMenu->addAction(tr("Replace…"), this, [this] {
    metapostCode->show(); tabWidget->setCurrentIndex(0); mpostEdit->showReplace();
  });
  replaceAction->setShortcuts(QKeySequence::Replace);
  editMenu->addAction(tr("Find Next"), this, [this] { mpostEdit->findNext(); }, QKeySequence::FindNext);
  editMenu->addAction(tr("Find Previous"), this, [this] { mpostEdit->findPrevious(); }, QKeySequence::FindPrevious);

  anchorsButton = new QPushButton(tr("&Anchors"));
  anchorsButton->setObjectName("anchorsButton");
  anchorsButton->setCheckable(true);
  anchorsButton->setChecked(glyph->font->anchorsVisible());
  editToolBar->addWidget(anchorsButton);
  connect(anchorsButton, &QPushButton::toggled, this, &GlyphWindow::showAnchors);
  connect(glyph->font, &Font::anchorsVisibilityChanged, this, [this](bool visible) {
    const QSignalBlocker blocker(anchorsButton);
    anchorsButton->setChecked(visible);
  });

  viewMenu = menuBar()->addMenu(tr("&View"));

  menuBar()->addSeparator();

  itemMenu = menuBar()->addMenu(tr("&Item"));

  QAction* enableFill = itemMenu->addAction(tr("&Enable Fill"), this, &GlyphWindow::enableFill);
  enableFill->setCheckable(true);
  enableFill->setChecked(false);

  menuBar()->addSeparator();

  QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));

  QAction* aboutAct = helpMenu->addAction(tr("&About"), this, &GlyphWindow::about);
  aboutAct->setStatusTip(tr("Show the application's About box"));

  QAction* aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

  QToolButton* pointerButton = new QToolButton;
  pointerButton->setCheckable(true);
  pointerButton->setChecked(true);
  pointerButton->setIcon(QIcon(":/images/pointer.png"));
  QToolButton* linePointerButton = new QToolButton;
  linePointerButton->setCheckable(true);
  linePointerButton->setIcon(QIcon(":/images/linepointer.png"));

  auto tt = QImageReader::supportedImageFormats();

  // QToolButton *addPointButton = new QToolButton;
  // addPointButton->setCheckable(true);
  // addPointButton->setIcon(QIcon(":/images/curve.svg"));

  pointerTypeGroup = new QButtonGroup(this);
  pointerTypeGroup->addButton(pointerButton, int(GlyphScene::MoveItem));
  pointerTypeGroup->addButton(linePointerButton, int(GlyphScene::Ruler));
  // pointerTypeGroup->addButton(addPointButton, int(GlyphScene::AddPoint));
  connect(pointerTypeGroup, SIGNAL(buttonClicked(int)),
          this, SLOT(pointerGroupClicked(int)));

  pointerToolbar = addToolBar(tr("Pointer type"));
  pointerToolbar->addWidget(pointerButton);
  pointerToolbar->addWidget(linePointerButton);
  // pointerToolbar->addWidget(addPointButton);

  auto* penStrokeButton = new QPushButton("Pen Stroke Editor");
  pointerToolbar->addWidget(penStrokeButton);
  connect(penStrokeButton, &QPushButton::clicked, this, [this]() {
    QDialog dlg(this);
    auto* layout = new QVBoxLayout(&dlg);
    auto* editor = new nibenvelope::PenStrokeEditor(glyph->font, this);
    layout->addWidget(editor);
    dlg.resize(1000, 700);
    dlg.exec();
  });

  windowMenu = menuBar()->addMenu(tr("&Window"));
  connect(windowMenu, &QMenu::aboutToShow, this, &GlyphWindow::updateWindowMenu);

  updateWindowMenu();
}
void GlyphWindow::updateWindowMenu() {
  windowMenu->clear();

  for (QWidget* w : QApplication::topLevelWidgets()) {
    if (!w->isWindow() || !w->isVisible()) continue;

    auto title = w->windowTitle();

    if (w->isWindowModified()) {
      const bool explicitMarker=QGuiApplication::platformName()=="cocoa" && qobject_cast<GlyphWindow*>(w);
      title.replace("[*]", explicitMarker ? "" : "*");
    } else {
      title.replace("[*]", "");
    }

    QAction* act = windowMenu->addAction(title);
    act->setCheckable(true);
    act->setChecked(w->isActiveWindow());

    QObject::connect(act, &QAction::triggered, w, [w]() {
      w->raise();
      w->activateWindow();
    });
  }
}
void GlyphWindow::showAnchors(bool checked) {
  glyph->font->setAnchorsVisible(checked);
}
void GlyphWindow::pointerGroupClicked(int) {
  scene->setMode(GlyphScene::Mode(pointerTypeGroup->checkedId()));
}
void GlyphWindow::enableFill(bool enable) {
  scene->setFillEnable(enable);
}

void GlyphWindow::fillContour(bool fill) {
}
void GlyphWindow::about() {
  QMessageBox::about(this, tr("About Dock Widgets"),
                     tr("The <b>Dock Widgets</b> example demonstrates how to "
                        "use Qt's dock widgets. You can enter your own text, "
                        "click a customer to add a customer name and "
                        "address, and click standard paragraphs to add them."));
}
void GlyphWindow::importImage() {
  QFileDialog dialog(this, tr("Import Image"));
  initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {
  }
}
void GlyphWindow::initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode) {
  static bool firstDialog = true;

  if (firstDialog) {
    firstDialog = false;
    QSettings settings;
    QString lastPictureLocation = settings.value("LastPictureLocation").value<QString>();
    if (lastPictureLocation == "") {
      const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
      dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    } else {
      dialog.setDirectory(lastPictureLocation);
    }
  }

  QStringList mimeTypeFilters;
  const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
                                                ? QImageReader::supportedMimeTypes()
                                                : QImageWriter::supportedMimeTypes();
  foreach (const QByteArray& mimeTypeName, supportedMimeTypes)
    mimeTypeFilters.append(mimeTypeName);
  mimeTypeFilters.sort();
  dialog.setMimeTypeFilters(mimeTypeFilters);
  dialog.selectMimeTypeFilter("image/jpeg");
  if (acceptMode == QFileDialog::AcceptSave)
    dialog.setDefaultSuffix("jpg");
}
bool GlyphWindow::loadFile(const QString& fileName) {
  QImageReader reader(fileName);
  reader.setAutoTransform(true);
  const QImage newImage = reader.read();
  if (newImage.isNull()) {
    QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                             tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
    return false;
  }

  QFileInfo imageFileInfo{fileName};  // QFileInfo(fileName).absolutePath()
  Glyph::ImageInfo imageInfo;
  if (!imageFileInfo.isRelative()) {
    QDir d = QFileInfo(glyph->font->path()).absoluteDir();
    QString relPath = d.relativeFilePath(imageFileInfo.absoluteFilePath());
    if (!relPath.startsWith("..")) {
      imageInfo.path = relPath;
    }
  } else {
    imageInfo.path = fileName;
  }

  QSettings settings;

  settings.setValue("LastPictureLocation", imageFileInfo.absolutePath());

  QVariant var;
  var.setValue(imageInfo);
  glyph->setPropertyWithUndo("image", var);

  // const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
  //	.arg(QDir::toNativeSeparators(fileName)).arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
  // statusBar()->showMessage(message);

  return true;
}
bool GlyphWindow::save() {
  if (mpostEdit->hasPendingChanges() || mpostEdit->hasConflict()) {
    metapostCode->show(); tabWidget->setCurrentIndex(0);
    statusBar()->showMessage(tr("Code has unexecuted changes. Execute it before saving; your draft has been kept."));
    return false;
  }
  QApplication::setOverrideCursor(Qt::WaitCursor);
  bool ret = glyph->font->saveFile();
  QApplication::restoreOverrideCursor();
  if (!ret) QMessageBox::warning(this, tr("Cannot save glyph sources"), glyph->font->projectError);
  return ret;
}
void GlyphWindow::updateUndoActions() {
  if (!undoAction || !redoAction) return;
  auto* focus = QApplication::focusWidget();
  const bool editing = mpostEdit && focus && (focus == mpostEdit->textEdit() || mpostEdit->isAncestorOf(focus));
  if (editing) {
    const auto* line = qobject_cast<QLineEdit*>(focus);
    undoAction->setEnabled(line ? line->isUndoAvailable() : mpostEdit->textEdit()->document()->isUndoAvailable());
    redoAction->setEnabled(line ? line->isRedoAvailable() : mpostEdit->textEdit()->document()->isRedoAvailable());
    undoAction->setText(tr("Undo Code Edit")); redoAction->setText(tr("Redo Code Edit"));
  } else {
    undoAction->setEnabled(undoStack->canUndo()); redoAction->setEnabled(undoStack->canRedo());
    undoAction->setText(tr("Undo Drawing Edit")); redoAction->setText(tr("Redo Drawing Edit"));
  }
}
void GlyphWindow::closeEvent(QCloseEvent* event) {
  if (maybeSave()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}
bool GlyphWindow::maybeSave() {
  if (!isWindowModified())
    return true;
  const QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Application"),
                                                               tr("The document has been modified.\n"
                                                                  "Do you want to save your changes?"),
                                                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  switch (ret) {
    case QMessageBox::Save:
      return save();
    case QMessageBox::Cancel:
      return false;
    default:
      break;
  }
  return true;
}
void GlyphWindow::writeSettings() {
  QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
  settings.setValue("geometry", saveGeometry());
}
void GlyphWindow::createUndoView() {
  dockUndo = new QDockWidget(this);
  dockUndo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
  dockUndo->setObjectName(QStringLiteral("dockWidget"));
  QWidget* dockWidgetContents = new QWidget();
  dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
  QVBoxLayout* vboxLayout1 = new QVBoxLayout(dockWidgetContents);
  vboxLayout1->setSpacing(4);
  vboxLayout1->setContentsMargins(0, 0, 0, 0);
  vboxLayout1->setObjectName(QStringLiteral("vboxLayout1"));
  QHBoxLayout* hboxLayout = new QHBoxLayout();

  hboxLayout->setSpacing(6);

  hboxLayout->setContentsMargins(0, 0, 0, 0);

  hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
  QSpacerItem* spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

  hboxLayout->addItem(spacerItem);

  QLabel* label = new QLabel(dockWidgetContents);
  label->setObjectName(QStringLiteral("label"));
  label->setText("Undo limit");

  hboxLayout->addWidget(label);

  QSpinBox* undoLimit = new QSpinBox(dockWidgetContents);
  undoLimit->setObjectName(QStringLiteral("undoLimit"));

  hboxLayout->addWidget(undoLimit);

  vboxLayout1->addLayout(hboxLayout);

  undoView = new QUndoView(dockWidgetContents);
  undoView->setObjectName(QStringLiteral("undoView"));
  undoView->setCleanIcon(QIcon(":/images/ok.png"));
  undoView->setStack(undoStack);

  vboxLayout1->addWidget(undoView);

  dockUndo->setWidget(dockWidgetContents);
  addDockWidget(Qt::RightDockWidgetArea, dockUndo);
  dockUndo->setWindowTitle("Undo Stack");

  viewMenu->addAction(dockUndo->toggleViewAction());
}
void GlyphWindow::generatePropertiesView() {
  dockProprties = new QDockWidget(tr("Proprties"), this);
  dockProprties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

  addDockWidget(Qt::RightDockWidgetArea, dockProprties);
  dockProprties->setWindowTitle("Proprties");

  viewMenu->addAction(dockProprties->toggleViewAction());

  GlyphParameterController* controller = new GlyphParameterController(glyph, this);

  QScrollArea* scroll3 = new QScrollArea();
  scroll3->setWidgetResizable(true);
  scroll3->setWidget(controller);

  dockProprties->setWidget(scroll3);
}
