#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QVector>

// Coordinates are viewport pixels, so labels stay separated at every zoom.
inline QVector<QRectF> placeAnchorLabels(const QVector<QPointF>& points,
                                        const QVector<QSizeF>& sizes,
                                        QVector<QRectF> occupied = {}) {
  for (const auto& point : points)
    occupied.append(QRectF(point - QPointF(6, 6), QSizeF(12, 12)));
  QVector<QRectF> result;
  for (int i = 0; i < points.size(); ++i) {
    QRectF candidate(points[i] + QPointF(9, -sizes[i].height() / 2), sizes[i]);
    bool overlap;
    do {
      overlap = false;
      for (const auto& previous : occupied) {
        if (candidate.adjusted(-3, -2, 3, 2).intersects(previous)) {
          candidate.moveTop(previous.bottom() + 5);
          overlap = true;
        }
      }
    } while (overlap);
    result.append(candidate);
    occupied.append(candidate);
  }
  return result;
}
