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

#include "knotitem.hpp"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsView>
#include <QtMath>

static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
	// We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
	// if we pass a value of 0.0 to QPainterPathStroker::setWidth()
	const qreal penWidthZero = qreal(0.00000001);

	if (path == QPainterPath() || pen == Qt::NoPen)
		return path;
	QPainterPathStroker ps;
	ps.setCapStyle(pen.capStyle());
	if (pen.widthF() <= 0.0)
		ps.setWidth(penWidthZero);
	else
		ps.setWidth(pen.widthF());
	ps.setJoinStyle(pen.joinStyle());
	ps.setMiterLimit(pen.miterLimit());
	QPainterPath p = ps.createStroke(path);
	p.addPath(path);
	return p;
}

KnotItem::KnotItem(KnotType knottype, QGraphicsItem * parent)
	: QGraphicsObject(parent){

	//setFlag(ItemIsMovable);
	setFlag(ItemSendsGeometryChanges);
	setCacheMode(DeviceCoordinateCache);
	//setZValue(-1);

	this->knottype = knottype;

	float penwidth = 1.5;

	switch (knottype) {
	case KnotItem::InCurve:{
		QColor col = QColor(4, 100, 166);
		this->pen = QPen(col);		
		this->pen.setCosmetic(true);
		this->pen.setWidthF(penwidth);
		
		this->brush = Qt::white;
		radius = 4;
		break;
	}
	case KnotItem::LeftControl:
		this->pen = QPen(Qt::cyan);		
		this->pen.setCosmetic(true);
		this->pen.setWidthF(penwidth);
		this->brush = Qt::white;
		radius = 3;
		break;
	case KnotItem::RightControl:
		this->pen = QPen(Qt::magenta);		
		this->pen.setCosmetic(true);
		this->pen.setWidthF(penwidth);
		this->brush = Qt::white;
		radius = 3;
		break;
	}
	
}

KnotItem::~KnotItem() {
	
}
QRectF KnotItem::boundingRect() const
{	
	return shape().controlPointRect();
}
QPainterPath KnotItem::shape() const
{
	QRectF  rect  = QRectF(-radius, -radius, 2 * radius, 2 * radius);

	if (scene()) {
		if (!scene()->views().isEmpty()) {
			QGraphicsView* view = scene()->views().first();
			double ratio = view->physicalDpiX() / 96.0;
			double radiusScene = view->mapToScene(QRect(-radius*ratio, -radius*ratio, 2 * radius*ratio, 2 * radius*ratio)).boundingRect().width() / 2;
			rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
		}
	}

	QPainterPath path;

	path.addEllipse(rect);

	return path; // qt_graphicsItem_shapeFromPath(path, pen);
}
void KnotItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{	
	
	painter->setRenderHint(QPainter::Antialiasing, true);

	

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
	

	QRectF  rect = QRectF(-radius, -radius, 2 * radius, 2 * radius);

	if (scene()) {
		if (!scene()->views().isEmpty()) {
			QGraphicsView* view = scene()->views().first();
			double ratio = view->physicalDpiX() / 96.0;
			double radiusScene = view->mapToScene(QRect(-radius*ratio, -radius*ratio, 2 * radius*ratio, 2 * radius*ratio)).boundingRect().width() / 2;
			rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
		}
	}

	painter->drawEllipse(rect);
}