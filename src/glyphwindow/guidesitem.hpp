/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef GUIDESITEM_H
#define GUIDESITEM_H
#include <QGraphicsObject>

#include "glyph.hpp"

class GuidesItem : public QGraphicsObject {
	Q_OBJECT

public:
	GuidesItem(Glyph* glyph, QGraphicsItem * parent = Q_NULLPTR);
	~GuidesItem();
	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
	Glyph* glyph;
	double lineHeight;
	
};
#endif // GUIDESITEM_H
