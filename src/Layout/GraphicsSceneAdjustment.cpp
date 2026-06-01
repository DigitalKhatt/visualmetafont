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

void GraphicsSceneAdjustment::drawBackground(QPainter* painter, const QRectF& rect) {
  // painter->setBrush(Qt::black);
  if (isDrawBackground) {
    if (!isDark) {
      painter->setPen(QPen(Qt::black, 20 << OtLayout::SCALEBY));
    } else {
      painter->setPen(QPen(Qt::white, 20 << OtLayout::SCALEBY));
    }

    if (!isDark) {
      painter->setBrush(Qt::white);
    } else {
      painter->setBrush(Qt::black);
    }

    painter->drawRect(-OtLayout::FrameWidth + OtLayout::Margin << OtLayout::SCALEBY, -OtLayout::TopSpace << OtLayout::SCALEBY, OtLayout::FrameWidth << OtLayout::SCALEBY, OtLayout::FrameHeight << OtLayout::SCALEBY);

    if (!isDark) {
      painter->setPen(QPen(Qt::darkGray, 20 << OtLayout::SCALEBY));
    } else {
      painter->setPen(QPen(Qt::white, 20 << OtLayout::SCALEBY));
    }

    painter->drawLine(-OtLayout::FrameWidth + 2 * OtLayout::Margin << OtLayout::SCALEBY, -OtLayout::TopSpace << OtLayout::SCALEBY, -OtLayout::FrameWidth + 2 * OtLayout::Margin << OtLayout::SCALEBY, OtLayout::FrameHeight - OtLayout::TopSpace << OtLayout::SCALEBY);
  }
}
