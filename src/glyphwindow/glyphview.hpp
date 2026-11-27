/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#include <QGraphicsView>
#include "glyph.hpp"
#include "contouritem.hpp"
#include "glyphscene.hpp"
#include "imageitem.hpp"
#include "guidesitem.hpp"
#include "componentitem.hpp"

class GlyphView : public QGraphicsView {
	Q_OBJECT

public:
	GlyphView(Glyph *glyph);
	~GlyphView();

	//void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
	void setGlyph(Glyph * glyph);	
	GlyphScene * getScene();
  void setPictureName(const QString& name);
	
	

public slots:	
	void zoomIn();
	void zoomOut();



protected:
	void scaleView(qreal scaleFactor);
	void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
	
	GlyphScene * glyphscene = nullptr;
	Glyph *m_glyph;
  struct Camera { QTransform transform; QPointF center; };
  QMap<QString,Camera> pictureCameras_;


	
	
};
