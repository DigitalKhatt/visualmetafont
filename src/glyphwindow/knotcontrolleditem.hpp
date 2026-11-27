/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;
	bool sceneEventFilter(QGraphicsItem* watched, QEvent* event) Q_DECL_OVERRIDE;


public:
	KnotControlledItem(int m_numsubpath, int m_numpoint, mp_gr_knot knot, int numpointpath, Glyph* glyph, QGraphicsItem* parent = Q_NULLPTR);
	~KnotControlledItem();

	void setPositions(mp_gr_knot knot);
	enum { Type = UserType + 4 };

	int type() const
	{
		// Enable the use of qgraphicsitem_cast with this item.
		return Type;
	}

private:
	bool moveMinMaxDeltas(QGraphicsItem* watched, QGraphicsSceneMouseEvent* event);
	bool addKashida(bool left, int nbKnots);
	bool AddLerp(int valueType, int lerpType, int axis);
	bool RemoveLerp(int valueType);


	int m_numsubpath;
	int m_numpoint;
	int m_numpointpath;
	bool m_isInControlledPath;
	mp_gr_knot m_knot;
	Glyph* m_glyph;
	Glyph::Knot* m_glyphknot;
	KnotItem* incurve;
	KnotItem* left;
	KnotItem* right;
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
	bool updateControlPoint(QGraphicsSceneMouseEvent* event, bool leftControl, QPointF diff, bool shift, bool ctrl);


	bool updateControlledPoint(MFExpr* expr, int position, QPointF diff, bool rountmodif);
};
#endif // KNOTCONTROLLEDITEM_H
