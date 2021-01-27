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
	void testKasheda();
	void serializeTexPages();
	void serializeMedinaPages();

private:
	
	void createActions();
	void createDockWindows();
	void loadLookupFile(QString fileName);
	bool save();
	bool generateOpenType();
  bool generateOpenTypeCff2Standard();
  bool generateOpenTypeCff2Extended();
  bool generateOpenTypeCff2(bool extended);
	bool exportpdf();
	bool generateAllQuranTexBreaking();
	bool generateAllQuranTexMedina();
	LayoutPages shapeMedina(int scale, int lineWidth);
	void testQuarn();
	void simpleAdjustPage(hb_buffer_t *buffer);
	void adjustPage(QString text, hb_font_t* shapeFont, hb_buffer_t *buffer);	
	void adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, int beginPage, int nbPages, QVector<int>&, int emScale);
	void adjustOverlapping(QList<QList<LineLayoutInfo>>& pages, int lineWidth, QList<QStringList> originalPages, int emScale);

	//QList<LineLayoutInfo> justifyPage(int emScale, int lineWidth, QStringList lines, LineJustification justification, bool newFace = true);
	//QList<LineLayoutInfo> justifyPage_old(int emScale, int lineWidth, QStringList lines, LineJustification justification, bool newFace = true);

	//LayoutPages pageBreak(int emScale, int lineWidth, bool pageFinishbyaVerse);

	void setQutranText(int type);

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

	bool applyJustification = true;
	bool applyCollisionDetection = false;
};
