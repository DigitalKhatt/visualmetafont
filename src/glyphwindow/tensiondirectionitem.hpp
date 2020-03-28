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
#ifndef TENSIONDIRECTIONITEM_H
#define TENSIONDIRECTIONITEM_H
#include <QGraphicsItem>
#include <QtCore>
#include "glyph.hpp"

class QGraphicsView;

class TensionDirectionItem : public QGraphicsObject {
	Q_OBJECT
public:
	TensionDirectionItem(Glyph::Param dirparam, Glyph::Param tensparam, Glyph* glyph, QGraphicsItem *parent = Q_NULLPTR);
	~TensionDirectionItem();

	enum { Type = UserType + 2 };

	int type() const;



	QRectF boundingRect() const Q_DECL_OVERRIDE;
	QPainterPath shape() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
	void calculatePosition(double pangle);

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:

	const double MAXTENSION = 10000;
	const double MINTENSION = 0.75;
	const double MINDISTANCE = 50;
	const double MAXDISTANCE = 250;
	const double STEP = 1 / ((MAXDISTANCE - MINDISTANCE) * MINTENSION);


	Glyph::Param dirparam;
	Glyph::Param tensparam;
	Glyph* glyph;
	double radius;
	QGraphicsView* view;

	double getDistanceFromTension(double& tension) {
		return MINDISTANCE + 1 / tension / STEP;
	}
	double getTensionFromDistance(double& distance) {
		double tension;
		if (distance < MINDISTANCE) {
			distance = MINDISTANCE;
			tension = MAXTENSION;
		}
		else if (distance > MAXDISTANCE) {
			distance = MAXDISTANCE;
			tension = MINTENSION;
		}
		else {
			//tension = qExp(10000/distance) / 100000;
			tension = 1 / ((distance - MINDISTANCE) * STEP);
		}
		return tension;
	}



};
#endif // TENSIONDIRECTIONITEM_H