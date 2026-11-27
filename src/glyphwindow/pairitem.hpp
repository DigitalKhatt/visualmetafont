/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef PAIRITEM_H
#define PAIRITEM_H

#include <QGraphicsItem>
#include "glyph.hpp"
#include <QPainter>

class MdiGlyph;

class PairItem : public QGraphicsItem {

public:
	PairItem(Glyph::Param& param, Glyph* glyph, QGraphicsItem *parent);
	~PairItem();

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	QPainterPath shape() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
  void setAnchorParameter(bool anchor);
  bool isAnchorParameter() const { return anchorParameter; }
  QString parameterName() const { return param.name; }

protected:	
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
	Glyph::Param& param;
	Glyph* glyph;
	double radius;
	QBrush brush;
	QPen pen;
  bool anchorParameter = false;
	
};
#endif // PAIRITEM_H
