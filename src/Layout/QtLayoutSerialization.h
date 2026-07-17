#pragma once

#include <QDataStream>
#include <QString>

#include "OtLayout.h"

inline QDataStream& operator<<(QDataStream& stream,
                               const SuraLocation& location) {
  return stream << QString::fromStdU16String(location.name)
                << location.pageNumber << location.x << location.y;
}

inline QDataStream& operator>>(QDataStream& stream, SuraLocation& location) {
  QString name;
  stream >> name >> location.pageNumber >> location.x >> location.y;
  location.name = name.toStdU16String();
  return stream;
}

inline QDataStream& operator<<(QDataStream& stream,
                               const digitalkhatt::ByteBuffer& buffer) {
  stream.writeRawData(reinterpret_cast<const char*>(buffer.data()),
                      static_cast<int>(buffer.size()));
  return stream;
}
