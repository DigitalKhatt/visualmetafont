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

#include "pairitem.hpp"
#include "glyphwindow.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

PairItem::PairItem(Glyph::Param param, Glyph* glyph, QGraphicsItem *parent):QGraphicsItem(parent){
	this->glyph = glyph;
	this->param = param;
	setFlags(ItemIsMovable | ItemIsSelectable);	
	setCacheMode(DeviceCoordinateCache);	
	
	setToolTip(param.name);

	

	if (param.type == Glyph::direction) {
		setPos(100, 100);
	}

	QColor col = QColor(4, 100, 166);
	this->pen = QPen(col);
	this->pen.setCosmetic(true);
	this->pen.setWidthF(1.5);

	this->brush = Qt::white;
	radius = 4;
	
}

QRectF PairItem::boundingRect() const
{	
	QRectF rect = QRectF(-radius, -radius, 2 * radius, 2 * radius);

	/*
	if (scene()) {
		if (!scene()->views().isEmpty()) {
			QGraphicsView* view = scene()->views().first();
			double radiusScene = view->mapToScene(QRect(-radius, -radius, 2 * radius, 2 * radius)).boundingRect().width() / 2;
			rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
		}

	}*/

	if (scene()) {
		if (!scene()->views().isEmpty()) {
			QGraphicsView* view = scene()->views().first();
			double ratio = view->physicalDpiX() / 96.0;
			double radiusScene = view->mapToScene(QRect(-radius * ratio, -radius * ratio, 2 * radius*ratio, 2 * radius*ratio)).boundingRect().width() / 2;
			rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
		}
	}

	return rect;
	
}

QPainterPath PairItem::shape() const
{
	QPainterPath path;

	path.addEllipse(boundingRect());

	return path;
}
void PairItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	QPointF diff = event->pos() - event->lastPos();

	QPointF pair = glyph->property(param.name.toLatin1()).toPointF() +  diff;

	glyph->setProperty(param.name.toLatin1(), pair);
}
void PairItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{

	if (isSelected()) {
		painter->setBrush(this->pen.color());
		painter->setPen(this->pen);
	}
	else {
		if (flags() & ItemIsMovable) {
			painter->setBrush(this->brush);
			painter->setPen(this->pen);
		}
		else {
			painter->setBrush(Qt::gray);
			painter->setPen(Qt::gray);
		}

	}

	painter->drawEllipse(boundingRect());
	
	/*
	if (!isSelected()) {

		QColor selectionColor(Qt::black);
		selectionColor.setAlphaF(0.2);

		QPen pen(selectionColor);
		pen.setWidth(2);
		pen.setCosmetic(true);

		painter->setPen(pen);
		painter->drawEllipse(boundingRect());

		QColor color(Qt::black);
		color.setAlphaF(0.2);

		painter->setPen(Qt::NoPen);
		painter->setBrush(color);
		painter->drawEllipse(boundingRect());
	}
	else {
		painter->setPen(Qt::NoPen);
		painter->setBrush(Qt::darkGray);
		painter->drawEllipse(boundingRect());
	}*/
}

PairItem::~PairItem() {
	
}
