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

#include "GraphicsViewAdjustment.h"
#include <QtGui>


GraphicsViewAdjustment::GraphicsViewAdjustment(QWidget *parent)
	: GraphicsViewAdjustment(NULL, parent)
{
}
GraphicsViewAdjustment::GraphicsViewAdjustment(QGraphicsScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent)
{
	setRenderHints(QPainter::HighQualityAntialiasing);
	//setRenderHint(QPainter::Antialiasing, true);
	//setDragMode(QGraphicsView::RubberBandDrag);
	setOptimizationFlags(QGraphicsView::DontSavePainterState);
	//setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	//setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	
}

GraphicsViewAdjustment::~GraphicsViewAdjustment()
{
}

void GraphicsViewAdjustment::resizeEvent(QResizeEvent *event) {	
	QGraphicsView::resizeEvent(event);
}

void GraphicsViewAdjustment::scaleView(qreal scaleFactor)
{
	qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	//if (factor < 0.07 || factor > 100)
		//return;

	scale(scaleFactor, scaleFactor);
}


void GraphicsViewAdjustment::zoomIn()
{
	scaleView(qreal(1.2));
}

void GraphicsViewAdjustment::zoomOut()
{
	scaleView(1 / qreal(1.2));
}
void GraphicsViewAdjustment::keyPressEvent(QKeyEvent *event)
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
