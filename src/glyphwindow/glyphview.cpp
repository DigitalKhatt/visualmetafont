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

#include "glyphview.hpp"
#include "contouritem.hpp"
#include "pairitem.hpp"
#include "glyphscene.hpp"
#include "glyph.hpp"
#include "glyphcellitem.hpp"
//#include <QGLWidget>




#include <QKeyEvent>

GlyphView::GlyphView(Glyph *glyph) {
	this->setRenderHint(QPainter::Antialiasing);
	//this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	this->setWindowTitle("Glyph View");

	//this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));

	setResizeAnchor(QGraphicsView::ViewportAnchor::AnchorViewCenter);

	glyphscene = new GlyphScene(glyph);
	setScene(glyphscene);



	setDragMode(QGraphicsView::RubberBandDrag);

	setGlyph(glyph);

	setMouseTracking(true);
}

GlyphView::~GlyphView() {
	//disconnect();	
}

GlyphScene * GlyphView::getScene() {
	return glyphscene;
}

void GlyphView::setGlyph(Glyph * glyph) {
	m_glyph = glyph;
	glyphscene->setGlyph(glyph);
}

void GlyphView::resizeEvent(QResizeEvent *event) {
	//centerOn(contour);
	QGraphicsView::resizeEvent(event);
}

/*
void GlyphView::drawBackground(QPainter *painter, const QRectF &rect)
{

	QPen pen = QPen(Qt::blue);
	pen.setWidth(2);
	pen.setCosmetic(true);

	{ // X- and Y-Axis drawing
		painter->setPen(pen);
		//painter->drawLine(sceneRect.left(), h_2, sceneRect.right(), h_2);     // X-Axis
		painter->drawLine(rect.left(), 0, rect.right(), 0);     // X-Axis
		//painter->drawLine(w_2, sceneRect.top(), w_2, sceneRect.bottom());  // Y-Axis
		painter->drawLine(0, rect.top(), 0, rect.bottom());  // Y-Axis
	}
}*/

void GlyphView::scaleView(qreal scaleFactor)
{
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100)
		return;

	scale(scaleFactor, scaleFactor);

}


void GlyphView::zoomIn()
{
	scaleView(qreal(1.2));
}

void GlyphView::zoomOut()
{
	scaleView(1 / qreal(1.2));
}
void GlyphView::keyPressEvent(QKeyEvent *event)
{
	//centerOn(contour);
	switch (event->key()) {
	case Qt::Key_Plus:
		zoomIn();
		break;
	case Qt::Key_Minus:
		zoomOut();
		break;
	case Qt::Key_Space:
	default:
		QGraphicsView::keyPressEvent(event);
	}
}
