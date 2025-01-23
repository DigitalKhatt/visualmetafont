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

#pragma once

//#include <QtWidgets>
#include "qmainwindow.h"
#include "OtLayout.h"
#include <qcombobox.h>
#include "qsqldatabase.h"

class Font;
class GlyphVis;
class OtLayout;
class GraphicsViewAdjustment;
class GraphicsSceneAdjustment;
struct hb_buffer_t;
struct hb_font_t;
class QPlainTextEdit;
class QPushButton;
class QVBoxLayout;

class QSpinBox;
class QLabel;
class QSpinBox;
class QTreeWidget;

struct OverlapResult {
  int pageIndex;
  int lineIndex; 
  int nextGlyph;
  int prevGlyph;
};

class LayoutWindow : public QMainWindow
{
	Q_OBJECT


public:

	LayoutWindow(Font *font, QWidget *parent = Q_NULLPTR, Qt::WindowFlags flags = Qt::WindowFlags());
	~LayoutWindow();

public slots:
	void layoutParameterChanged();
	void executeRunText(bool newFace, int refresh = 2);
	

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots :
	void calculateMinimumSize();
  void findOverflows(bool overfull);  
	void testKasheda();
	void serializeTexPages();
	void serializeMedinaPages();
  void compareIndopakFonts();
  void compareFonts(QString layoutName, QString textCol);
  void compareWaqfs(QString layoutName, QString textCol);
  void createDataBase();

private:
	
	void createActions();
	void createDockWindows();
	void loadLookupFile(QString fileName);
	bool save();
	bool generateOpenType();
  bool generateOpenTypeCff2Standard();
  bool generateOpenTypeCff2StandardWithoutVar();
  bool generateOpenTypeCff2Extended();
  bool generateOpenTypeCff2(bool extended, bool generateVariableOpenType);
	bool exportpdf();
	bool generateAllQuranTexBreaking();
	bool generateMushaf(bool isHTML);
  bool generateMadinaVARHTML();
  bool generateLayoutInfo();
	LayoutPages shapeMedina(double scale, int lineWidth, OtLayout* layout, hb_buffer_cluster_level_t  cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
  LayoutPages shapeMushaf(double scale, int lineWidth, OtLayout* layout, hb_buffer_cluster_level_t  cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
	void testQuarn();
	void simpleAdjustPage(hb_buffer_t *buffer);
	void adjustPage(QString text, hb_font_t* shapeFont, hb_buffer_t *buffer);	
	void adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, int beginPage, int nbPages, QVector<int>&, double emScale, QVector<OverlapResult>& result, bool onlySameLine);
  void adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, QList<QStringList> originalPages, double emScale, bool onlySameLine);
  void applyDirectedForceLayout(QList<QList<LineLayoutInfo>>& pages, QList<QStringList> originalPages, int lineWidth, int beginPage, int nbPages, double emScale);
  void generateOverlapLookups(const QList<QList<LineLayoutInfo>>& pages,const QList<QStringList>& originalPages,const QVector<OverlapResult>& result);
  void editLookup(QString lookupName);
  void saveCollision();
  void layoutDatabase();
  void loadMushafLayout(QString layoutName);
  void generateTestFile();
  void checkOffMarks();

	void setQuranText(int type);
  QComboBox* justCombo;
	QDockWidget* textRun;
	QDockWidget* lookupTree;
	QPlainTextEdit * textEdit;
	QPushButton * executeRunButton;
	QVBoxLayout * textRunLayout;

	QMenu *viewMenu;
	//toolbar
	QToolBar *fileToolBar;
	QToolBar *editToolBar;
	QToolBar *pointerToolbar;

	QMenu *otherMenu;


	Font* m_font;

	OtLayout * m_otlayout;

	GraphicsViewAdjustment* m_graphicsView;
	GraphicsSceneAdjustment* m_graphicsScene;	
	QSpinBox *integerSpinBox;
	QLabel *suraName;
	QSpinBox *fontSizeSpinBox;
	QTreeWidget * lokkupTreeWidget;

	

	QList<QString> currentQuranText;
	QList<QString> suraNameByPage;

	bool applyJustification;
	bool applyCollisionDetection = false;
  bool applyFontSize = false;
  bool applyForce = false;
  bool applyTeXAlgo = false;  

  QComboBox* mushafLayouts;
};
