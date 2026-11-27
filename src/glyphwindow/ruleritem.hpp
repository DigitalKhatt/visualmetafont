/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef RULERITEM_HPP
#define RULERITEM_HPP
#include <QGraphicsItem>

class RulerItem : public QGraphicsItem {	

public:
	RulerItem(const QLineF &line, QGraphicsItem * parent = Q_NULLPTR);
	~RulerItem();

	QLineF line() const;
	void setLine(const QLineF &line);

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

private:
	QGraphicsLineItem* firstline;
	QGraphicsLineItem* secondline;
	QGraphicsLineItem* thirdline;

	QGraphicsSimpleTextItem* firstdistText;
	QGraphicsSimpleTextItem* seconddistText;
	QGraphicsSimpleTextItem* thirddistText;

	QGraphicsSimpleTextItem* firstangleText;
	QGraphicsSimpleTextItem* secondangleText;
};

#endif // RULERITEM_HPP