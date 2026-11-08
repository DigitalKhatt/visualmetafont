#pragma once

#include <QMap>
#include <QSet>
#include <QStringList>
#include <QVector>

namespace SourceLinks {
// Item data is presentation state only; it never enters glyph source or undo.
constexpr int KeysRole = 61;
constexpr int HighlightRole = 62;
inline QString parameterKey(const QString& name) { return "parameter:" + name; }
inline QString anchorKey(const QString& name, int type) { return QString("anchor:%1:%2").arg(type).arg(name); }
inline QString pointKey(int path, int point, const QString& role = "point") {
  return QString("point:%1:%2:%3").arg(path).arg(point).arg(role);
}
QStringList expressionKeys(const QString& expression);
struct Declaration {
  QString key;
  int begin = 0;
  int end = 0;
  QStringList related;
};
// Read-only source indexing. Incomplete/structurally changed paths are excluded
// rather than guessing a point identity from its coordinates.
QVector<Declaration> declarations(const QString& source, const QMap<int,QSet<int>>& topology);
}
