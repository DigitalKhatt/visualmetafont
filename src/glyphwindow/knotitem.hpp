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
#ifndef KNOTITEM_H
#define KNOTITEM_H
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>


class KnotItem : public QGraphicsObject {
	Q_OBJECT

public:
	enum KnotType {
		InCurve,
		LeftControl,
		RightControl,
	};

	enum { Type = UserType + 3 };

	int type() const
	{
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}

	QRectF boundingRect() const Q_DECL_OVERRIDE;	
	QPainterPath shape() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

public:	
	KnotItem(KnotType knottype, QGraphicsItem * parent = Q_NULLPTR);
	~KnotItem();

private:
	KnotType knottype;
	double radius;
	QBrush brush;
	QPen pen;	
};
#endif // KNOTITEM_H