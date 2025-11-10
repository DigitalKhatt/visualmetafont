#pragma once
#include <QString>
#include <QRegularExpression>
#include <QVector>
#include <QMap>
#include <functional>
#include "OtLayout.h"


class TajweedService {
public:
  TajweedService() = default;

  // Equivalent to your TS method. Works on QString (UTF-16).
  void applyTajweedForText(
      const QString& text,
      const std::function<void(int /*pos*/, const QString& /*style*/)> &setTajweed,
      const std::function<void()> &resetIndex,
      bool isIndopak) const;

  // Page-level version (keeps your line mapping behavior).
  QVector<QMap<int, QString>>
  applyTajweedByPage(const QVector<LineToJustify>& lines, bool isIndopak) const;
};
