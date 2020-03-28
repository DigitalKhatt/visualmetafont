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