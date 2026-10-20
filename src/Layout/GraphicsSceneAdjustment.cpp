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

#include "GraphicsSceneAdjustment.h"

#include <QPainter>

#include "LayoutWindow.h"
#include "OtLayout.h"

GraphicsSceneAdjustment::GraphicsSceneAdjustment(bool isDrawBackground, bool isDark, QObject* parent)
    : isDrawBackground(isDrawBackground), isDark(isDark), QGraphicsScene(parent) {
}

GraphicsSceneAdjustment::~GraphicsSceneAdjustment() {
}

QRectF GraphicsSceneAdjustment::pageRect() const {
  const double scale = 1 << OtLayout::SCALEBY;
  return QRectF((-layout_.pageWidth + layout_.rightMargin) * scale,
      -layout_.topSpace * scale, layout_.pageWidth * scale, layout_.pageHeight * scale);
}

void GraphicsSceneAdjustment::setPageLayout(const digitalkhatt::PageLayout& layout) {
  layout_ = layout;
  const double border = 20 * (1 << OtLayout::SCALEBY);
  setSceneRect(pageRect().united(itemsBoundingRect()).adjusted(-border, -border, border, border));
  invalidate(sceneRect(), QGraphicsScene::BackgroundLayer);
  update();
}

void GraphicsSceneAdjustment::drawBackground(QPainter* painter, const QRectF& rect) {
  // painter->setBrush(Qt::black);
  if (isDrawBackground) {
    painter->save();
    if (!isDark) {
      painter->setPen(QPen(Qt::black, digitalkhatt::PageLayout::borderStrokeWidth << OtLayout::SCALEBY));
    } else {
      painter->setPen(QPen(Qt::white, digitalkhatt::PageLayout::borderStrokeWidth << OtLayout::SCALEBY));
    }

    if (!isDark) {
      painter->setBrush(Qt::white);
    } else {
      painter->setBrush(Qt::black);
    }

    const double scale = 1 << OtLayout::SCALEBY;
    const QRectF page = pageRect();
    painter->drawRect(page);

    if (!isDark) {
      painter->setPen(QPen(Qt::darkGray, 20 << OtLayout::SCALEBY));
    } else {
      painter->setPen(QPen(Qt::white, 20 << OtLayout::SCALEBY));
    }

    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QPointF(-layout_.textWidth() * scale, page.top()),
                      QPointF(-layout_.textWidth() * scale, page.bottom()));
    painter->drawLine(QPointF(0, page.top()), QPointF(0, page.bottom()));
    painter->restore();
  }
}
