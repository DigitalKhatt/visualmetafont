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

#include <cstdint>
#include <QGraphicsPathItem>
#include <QPicture>
#include "commontypes.h"

class GlyphVis;
class LayoutWindow;

class GlyphItem : public QGraphicsPathItem
{
  friend class GraphicsViewAdjustment;
public:
  GlyphItem(double xscale, double yscale, GlyphVis* glyph,
            LayoutWindow* layoutWindow, GlyphParameters parameters,
            std::uint32_t lookup = 0, std::uint32_t subtable = 0,
            std::uint16_t baseChar = 0,
            QGraphicsItem* parent = Q_NULLPTR);
  ~GlyphItem();
  //QRectF boundingRect() const Q_DECL_OVERRIDE;
  //void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*) override;


private:
  GlyphVis* m_glyph;
  LayoutWindow* m_layoutWindow;
  std::uint32_t m_lookup;
  std::uint32_t m_subtable;
  std::uint16_t m_baseChar;
  QPointF lastPos;
  QPoint lastdiff;
  double m_scale;
  GlyphParameters m_parameters;
  QPicture m_picture;
};
