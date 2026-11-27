/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once

#include <QGraphicsScene>
#include <digitalkhatt/core/PageLayout.h>

class GraphicsSceneAdjustment : public QGraphicsScene {
  Q_OBJECT

 public:
  GraphicsSceneAdjustment(bool isDrawBackground, bool isDark, QObject* parent);
  ~GraphicsSceneAdjustment();
  void drawBackground(QPainter* painter, const QRectF& rect) override;
  void setPageLayout(const digitalkhatt::PageLayout& layout);
  QRectF pageRect() const;

 private:
  bool isDrawBackground;
  bool isDark;
  digitalkhatt::PageLayout layout_;
};
