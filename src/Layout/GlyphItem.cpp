/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#include "GlyphItem.h"
#include "GlyphRenderingQt.h"
#include "GlyphVis.h"
#include "QPainter"
#include "LayoutWindow.h"
#include <QGraphicsSceneMouseEvent>
#include "qdebug.h"

GlyphItem::GlyphItem(double xscale, double yscale, GlyphVis* glyph,
                     LayoutWindow* layoutWindow, GlyphParameters parameters,
                     std::uint32_t lookup, std::uint32_t subtable,
                     std::uint16_t baseChar, QGraphicsItem* parent)
    : QGraphicsPathItem(parent)
{

  m_glyph = glyph;
  m_layoutWindow = layoutWindow;
  m_lookup = lookup;
  m_subtable = subtable;
  m_parameters = parameters;
  m_baseChar = baseChar;

  auto path = digitalkhatt::qt::pathForGlyph(
      *glyph->getAlternate(parameters));


  path.setFillRule(Qt::WindingFill);

  if (m_glyph->name.find("aya") != std::string::npos) {
    path.setFillRule(Qt::OddEvenFill);
  }


  setPath(path);

  if (const auto* coloredGlyph = m_glyph->getColoredGlyph()) {
    m_picture = digitalkhatt::qt::pictureForGlyph(*coloredGlyph);
  }


  setBrush(Qt::black);
  setPen(Qt::NoPen);
  m_scale = xscale;



  QTransform m;
  m.scale(xscale, yscale);

  setTransform(m);


}
void GlyphItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  lastPos = event->scenePos();
  lastdiff = QPoint(0, 0);
}

void GlyphItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{

  Qt::KeyboardModifiers modifiers = event->modifiers();

  QPointF diff = event->scenePos() - lastPos;

  diff = QPoint(diff.x() / m_scale, diff.y() / m_scale);

  QPoint newdiff(diff.x(), -diff.y());

  //qDebug() << "newdiff" << newdiff << "lastdiff : " << lastdiff;

  QPoint disp = newdiff - lastdiff;

  lastdiff = newdiff;

  m_layoutWindow->setParameter(m_glyph->charcode, m_lookup, m_subtable,
                               m_glyph->charcode, m_baseChar, disp,
                               modifiers);



  //glyph->setProperty(param.name.toLatin1(), pair);
}

GlyphItem::~GlyphItem()
{
}
void GlyphItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
  auto coloredGlyph = m_glyph->getColoredGlyph();
  if (coloredGlyph) {
    painter->drawPicture(0, 0, m_picture);
  }
  else {
    QGraphicsPathItem::paint(painter, option, widget);
  }
}
