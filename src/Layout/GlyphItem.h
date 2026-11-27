/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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
