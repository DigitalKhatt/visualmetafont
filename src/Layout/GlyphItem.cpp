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
