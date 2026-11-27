/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once

#include <QGraphicsView>

class GraphicsViewAdjustment : public QGraphicsView {
  Q_OBJECT

 public:
  GraphicsViewAdjustment(bool isDark, QWidget* parent = Q_NULLPTR);
  GraphicsViewAdjustment(bool isDark, QGraphicsScene* scene, QWidget* parent = Q_NULLPTR);

  ~GraphicsViewAdjustment();

 public slots:
  void zoomIn();
  void zoomOut();

 protected:
  void scaleView(qreal scaleFactor);
  void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void contextMenuEvent(QContextMenuEvent* event) override;
  bool isDark;
};
