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

#include "ruleritem.hpp"
#include <QPen>

RulerItem::RulerItem(const QLineF &line, QGraphicsItem * parent) : QGraphicsItem(parent) {

	QPen defaultpen = QPen();
	defaultpen.setWidth(2);
	defaultpen.setCosmetic(true);

	firstline = new QGraphicsLineItem(this);
	firstline->setPen(defaultpen);
	secondline = new QGraphicsLineItem(this);
	secondline->setPen(defaultpen);
	thirdline = new QGraphicsLineItem(this);
	thirdline->setPen(defaultpen);

	firstdistText = new QGraphicsSimpleTextItem(this);
	firstdistText->setFlags(ItemIgnoresTransformations);
	seconddistText = new QGraphicsSimpleTextItem(this);
	seconddistText->setFlags(ItemIgnoresTransformations);
	thirddistText = new QGraphicsSimpleTextItem(this);
	thirddistText->setFlags(ItemIgnoresTransformations);

	firstangleText = new QGraphicsSimpleTextItem(this);
	firstangleText->setFlags(ItemIgnoresTransformations);
	secondangleText = new QGraphicsSimpleTextItem(this);
	secondangleText->setFlags(ItemIgnoresTransformations);

	setLine(line);
}

RulerItem::~RulerItem() {
	
}

void RulerItem::setLine(const QLineF &line) {

	
	firstline->setLine(line);
	
	firstdistText->setText(QString::number(line.length(), 'f', 2));	
	firstdistText->setPos(line.pointAt(0.5));

	QPointF third = QPointF(line.p1().x(), line.p2().y());

	QLineF line2 = QLineF(line.p1(), third);

	qreal angle = line2.angleTo(line);
	firstangleText->setText(QString::number(angle, 'f', 2));
	firstangleText->setPos(line.p1());

	secondline->setLine(line2);
	seconddistText->setText(QString::number(line2.length(), 'f', 2));
	seconddistText->setPos(line2.pointAt(0.5));

	line2 = QLineF(line.p2(), third);
	thirdline->setLine(line2);
	thirddistText->setText(QString::number(line2.length(), 'f', 2));
	thirddistText->setPos(line2.pointAt(0.5));

	
}
QLineF RulerItem::line() const
{	
	return firstline->line();
}

QRectF RulerItem::boundingRect() const
{
	return QRectF();
}

void RulerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{	
}

