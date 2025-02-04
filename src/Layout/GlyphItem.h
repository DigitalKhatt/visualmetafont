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

#include <QGraphicsPathItem>
#include "commontypes.h"

class GlyphVis;
class OtLayout;

class GlyphItem : public QGraphicsPathItem
{
  friend class GraphicsViewAdjustment;
public:
  GlyphItem(double xscale, double yscale, GlyphVis* glyph, OtLayout* layout, GlyphParameters parameters, quint32 lookup = 0, quint32 subtable = 0, quint16 baseChar = 0, QGraphicsItem* parent = Q_NULLPTR);
  ~GlyphItem();
  //QRectF boundingRect() const Q_DECL_OVERRIDE;
  //void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) override;


private:
  GlyphVis* m_glyph;
  OtLayout* m_layout;
  quint32 m_lookup;
  quint32 m_subtable;
  quint16 m_baseChar;
  QPointF lastPos;
  QPoint lastdiff;
  double m_scale;
  GlyphParameters m_parameters;
  //QPainterPath path;
};
