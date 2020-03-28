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

#include "tensiondirectionitem.hpp"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>




TensionDirectionItem::TensionDirectionItem(Glyph::Param dirparam, Glyph::Param tensparam, Glyph* glyph, QGraphicsItem *parent) : QGraphicsObject(parent) {
	this->glyph = glyph;
	this->dirparam = dirparam;
	this->tensparam = tensparam;
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges | ItemSendsScenePositionChanges);
	setCacheMode(DeviceCoordinateCache);
	//setZValue(-1);	
	radius = 5;

	//view = scene()->views().first();

	//QTransform m;
	//m.scale(1, -1);

	//setTransform(m);


	//calculatePosition();


}

TensionDirectionItem::~TensionDirectionItem() {

}
int TensionDirectionItem::type() const
{
	// Enable the use of qgraphicsitem_cast with this item.
	return Type;
}
void TensionDirectionItem::calculatePosition(double pangle) {

	if (scene()) {

		double length = MINDISTANCE;
		double angle = pangle;
		if (tensparam.type == Glyph::tension) {
			double tension = glyph->property(tensparam.name.toLatin1()).toDouble();
			length = getDistanceFromTension(tension);
		}
		if (dirparam.type == Glyph::direction) {
			//double tensionradiusScene = length; // view->mapToScene(QRect(0, 0, length, length)).boundingRect().width();
			//QLineF line(0, 0, tensionradiusScene, 0);
			angle = glyph->property(dirparam.name.toLatin1()).toDouble();
			//line.setAngle(-angle - 180);
			//QPointF pair = line.p2();
			//setPos(pair);
		}
		QLineF line(0, 0, length, 0);
		//line.setLength(length);
		line.setAngle(-angle - 180);		
		setPos(line.p2());
	}
}

QRectF TensionDirectionItem::boundingRect() const
{

	QRectF rect = QRectF(-radius, -radius, 2 * radius, 2 * radius);

	return rect;

	if (scene()) {
		QGraphicsView* view = scene()->views().first();
		double radiusScene = view->mapToScene(QRect(-radius, -radius, 2 * radius, 2 * radius)).boundingRect().width() / 2;
		rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);

	}

	return rect;

}

QPainterPath TensionDirectionItem::shape() const
{
	QPainterPath path;

	path.addEllipse(boundingRect());

	return path;
}
void TensionDirectionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	Qt::KeyboardModifiers modifiers = event->modifiers();

	bool shift = false;
	bool ctrl = false;

	if (modifiers & Qt::ShiftModifier) {
		shift = true;
	}
	if (modifiers & Qt::ControlModifier) {
		ctrl = true;
	}

	QLineF currentline(0, 0, pos().x(), pos().y());

	double angle = currentline.angle();
	double length = currentline.length();

	QPointF pair = mapToParent(QPointF(event->pos().x(), event->pos().y()));
	QLineF line(0, 0, pair.x(), pair.y());
	if (dirparam.type == Glyph::direction && !shift) {
		angle = line.angle();
		glyph->setProperty(dirparam.name.toLatin1(), -angle + 180);
	}
	if (tensparam.type == Glyph::tension && !ctrl) {
		length = line.length();
		double tension = getTensionFromDistance(length);

		glyph->setProperty(tensparam.name.toLatin1(), tension);
	}

	line.setLength(length);
	line.setAngle(angle);
	//setPos(line.p2()); 

	//update();
	//QGraphicsItem::mouseMoveEvent(event);
}

QVariant TensionDirectionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	QPointF pair = this->pos();
	switch (change) {
	case ItemPositionChange:
		//ensureVisible();
		break;
	case ItemPositionHasChanged:
		/*
		if (dirparam.type == Glyph::direction) {
			QLineF line(0, 0, pos().x(), pos().y());
			double angle = line.angle();
			glyph->setProperty(dirparam.name.toLatin1(), -angle);
		}*/
	case ItemScenePositionHasChanged:
		//calculatePosition();
	default:
		break;
	};

	return QGraphicsItem::itemChange(change, value);
}

void TensionDirectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	QTransform t = painter->transform();
	qreal m11 = t.m11(), m22 = t.m22();
	painter->save(); // save painter state
	painter->translate(QPoint(0, 0));

	//painter->save();

	QGraphicsView* view = scene()->views().first();
	double width = view->mapToScene(QRect(0, 0, 100, 100)).boundingRect().width();
	//painter->translate(QPointF(-100, -100));

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
	}

	painter->restore();
}
