/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef KNOTITEM_H
#define KNOTITEM_H
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>

class KnotControlledItem;


class KnotItem : public QGraphicsObject {
	Q_OBJECT
		friend KnotControlledItem;
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
	QPen disabledPen;
};
#endif // KNOTITEM_H
