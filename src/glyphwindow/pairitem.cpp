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

#include "pairitem.hpp"
#include "glyphwindow.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "sourcelinks.hpp"

PairItem::PairItem(Glyph::Param& param, Glyph* glyph, QGraphicsItem* parent) :QGraphicsItem(parent), param{ param } {
  this->glyph = glyph;  
  setFlags(ItemIsMovable | ItemIsSelectable);
  setCacheMode(DeviceCoordinateCache);

  setToolTip(param.name);
  setData(SourceLinks::KeysRole, QStringList{SourceLinks::parameterKey(param.name)});


  QColor col = QColor(4, 100, 166);
  this->pen = QPen(col);
  this->pen.setCosmetic(true);
  this->pen.setWidthF(1.5);

  this->brush = Qt::white;
  radius = 4;

}

QRectF PairItem::boundingRect() const
{
  QRectF rect = QRectF(-radius, -radius, 2 * radius, 2 * radius);

  /*
  if (scene()) {
    if (!scene()->views().isEmpty()) {
      QGraphicsView* view = scene()->views().first();
      double radiusScene = view->mapToScene(QRect(-radius, -radius, 2 * radius, 2 * radius)).boundingRect().width() / 2;
      rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
    }

  }*/

  if (scene()) {
    if (!scene()->views().isEmpty()) {
      QGraphicsView* view = scene()->views().first();
      double ratio = view->physicalDpiX() / 96.0;
      double radiusScene = view->mapToScene(QRect(-radius * ratio, -radius * ratio, 2 * radius * ratio, 2 * radius * ratio)).boundingRect().width() / 2;
      rect = QRectF(-radiusScene, -radiusScene, 2 * radiusScene, 2 * radiusScene);
    }
  }

  return rect;

}

QPainterPath PairItem::shape() const
{
  QPainterPath path;
  const auto rect = boundingRect();
  if (anchorParameter) {
    path.moveTo(rect.center().x(), rect.top());
    path.lineTo(rect.right(), rect.center().y());
    path.lineTo(rect.center().x(), rect.bottom());
    path.lineTo(rect.left(), rect.center().y());
    path.closeSubpath();
  } else {
    path.addEllipse(rect);
  }

  return path;
}
void PairItem::setAnchorParameter(bool anchor) {
  if (anchorParameter == anchor) return;
  anchorParameter = anchor;
  pen.setColor(anchor ? QColor(155, 55, 145) : QColor(4, 100, 166));
  brush = anchor ? QBrush(QColor(248, 225, 245)) : QBrush(Qt::white);
  setToolTip(anchor ? QObject::tr("Anchor parameter: %1").arg(param.name) : param.name);
  update();
}
void PairItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QPointF diff = event->pos() - event->lastPos();  

  
  auto scene = (GlyphScene*)this->scene();

  int position = scene->getControlledPosition(event);

  auto expr = param.expr.get();
  if (expr->isConstant(position)) {
    QPoint pair = expr->constantValue(position).toPoint() + diff.toPoint();
    expr->setConstantValue(position, pair);
    glyph->setProperty(param.name.toLatin1(), pair);
  }


  
  /*
  QPointF pair = glyph->property(param.name.toLatin1()).toPointF() + diff;


  auto exp = glyph->expressions.value(param.name);
  exp->setConstantValue(0, pair);


  glyph->setProperty(param.name.toLatin1(), pair);*/
}
void PairItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{

  if (data(SourceLinks::HighlightRole).toBool()) {
    painter->setBrush(QColor(255,193,70));
    auto linkedPen = this->pen;
    linkedPen.setColor(QColor(170,85,0));
    linkedPen.setWidthF(2.5);
    painter->setPen(linkedPen);
  }
  else if (isSelected()) {
    painter->setBrush(this->pen.color());
    painter->setPen(this->pen);
  }
  else {
    if (flags() & ItemIsMovable) {
      painter->setBrush(this->brush);
      painter->setPen(this->pen);
    }
    else {
      painter->setBrush(Qt::gray);
      painter->setPen(Qt::gray);
    }

  }

  painter->drawPath(shape());

  /*
  if (!isSelected()) {

    QColor selectionColor(Qt::black);
    selectionColor.setAlphaF(0.2);

    QPen pen(selectionColor);
    pen.setWidth(2);
    pen.setCosmetic(true);

    painter->setPen(pen);
    painter->drawEllipse(boundingRect());

    QColor color(Qt::black);
    color.setAlphaF(0.2);

    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawEllipse(boundingRect());
  }
  else {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(boundingRect());
  }*/
}

PairItem::~PairItem() {

}
