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
#ifndef KNOTCONTROLLEDITEM_H
#define KNOTCONTROLLEDITEM_H
#include <QGraphicsItem>
#include <QPainter>
#include <QRectF>
# include "mplibps.h"
#include "knotitem.hpp"
#include "glyph.hpp"




class TensionDirectionItem;


class KnotControlledItem : public QGraphicsObject {
	Q_OBJECT

	friend class GlyphScene;
public:	

	QRectF boundingRect() const Q_DECL_OVERRIDE;	
	//QPainterPath shape() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
	bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) Q_DECL_OVERRIDE;

public:	
	KnotControlledItem(int m_numsubpath, int m_numpoint, mp_gr_knot knot, Glyph* glyph, QGraphicsItem * parent = Q_NULLPTR);
	~KnotControlledItem();

	void setPositions(mp_gr_knot knot);	
	enum { Type = UserType + 4 };

	int type() const
	{
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}

private:
	int m_numsubpath;
	int m_numpoint;
	mp_gr_knot m_knot;
	Glyph* m_glyph;
	Glyph::Knot * m_glyphknot;
	KnotItem* incurve;
	KnotItem* left;
	KnotItem* right;
	TensionDirectionItem* lefttd;
	TensionDirectionItem* righttd;
	QGraphicsLineItem* leftline;
	QGraphicsLineItem* rightline;

	const double MAXTENSION = 10000;
	const double MINTENSION = 0.75;
	const double MINDISTANCE = 50;
	const double MAXDISTANCE = 250;
	const double STEP = 1 / ((MAXDISTANCE - MINDISTANCE) * MINTENSION);

	double getDistanceFromTension(double& tension) {
		return MINDISTANCE + 1 / tension / STEP;
	}
	double getTensionFromDistance(double distance) {
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
	bool updateControlPoint(bool leftControl, QPointF diff, bool shift, bool ctrl, QLineF line);
};
#endif // KNOTCONTROLLEDITEM_H