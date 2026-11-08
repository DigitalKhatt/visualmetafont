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

#ifndef MAINWINDOWS_H
#define MAINWINDOWS_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "contouritem.hpp"
#include "glyph.hpp"
#include "glyphscene.hpp"
#include "glyphview.hpp"
#include "pairitem.hpp"

class MetaPostEditor;

class GlyphWindow : public QMainWindow {
  Q_OBJECT

 public:
  GlyphWindow(Glyph* glyph);

 public slots:
  void executeCommands();
  void glyphChanged(QString name);
  bool loadFile(const QString&);
  void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void changeEvent(QEvent* event) override;

 private slots:
  void about();
  void importImage();
  void enableFill(bool enable);
  void fillContour(bool fill);
  void pointerGroupClicked(int id);
  void showAnchors(bool checked);

 private:
  MP_options* _mp_options;
  MP _mp_instance;

  // QSvgWidget * svgWidget ;
  QScrollArea* scrollArea;

  MetaPostEditor* mpostEdit = nullptr;
  QAction* undoAction = nullptr;
  QAction* redoAction = nullptr;
  void updateUndoActions();
  void updateModifiedState();
  void updateWindowTitle();
  void updateSourceTopology();
  void drawingSelectionChanged();

  QPlainTextEdit *logOutput, *infoOutput;

  QTabWidget* tabWidget;
  QPushButton* executeButton;
  QVBoxLayout* mainLayout;

  GlyphView* glyphView;
  GlyphScene* scene;

  Glyph* glyph;

  void createActions();
  void createStatusBar();
  void createDockWindows();
  void initializeImageFileDialog(QFileDialog& dialog, QFileDialog::AcceptMode acceptMode);
  bool save();
  void closeEvent(QCloseEvent* event);
  bool maybeSave();
  void writeSettings();
  void createUndoView();
  void createPathView();
  void generatePropertiesView();
  void updateWindowMenu();

  QMenu* viewMenu;
  QMenu* itemMenu;
  QMenu* windowMenu;

  QUndoStack* undoStack;
  QUndoView* undoView;
  QDockWidget* dockProprties;
  QDockWidget* dockUndo;
  QDockWidget* dockPath;
  QDockWidget* metapostCode;

  // toolbar
  QToolBar* fileToolBar;
  QToolBar* editToolBar;
  QToolBar* pointerToolbar;
  QPushButton* anchorsButton = nullptr;
  QTreeWidget* layersView = nullptr;
  QCheckBox* finalPositionsControl = nullptr;
  bool dockLayoutInitialized_ = false;
  void createLayersView();
  void refreshLayersView();
  void createAnchorsView();
  void refreshAnchorsView();
  void syncAnchorSelection(const QString& key);
  QTreeWidget* anchorsView = nullptr;
  QLineEdit* anchorFilter = nullptr;
  QComboBox* anchorLabelMode = nullptr;
  QCheckBox* anchorMaster = nullptr;
  QPushButton* onlySelectedAnchor = nullptr;

  QButtonGroup* pointerTypeGroup;
};

#endif  // MAINWINDOWS_H
