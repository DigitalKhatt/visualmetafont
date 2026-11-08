#pragma once
#include <QByteArray>
#include <QMap>
#include <QStringList>
#include <QVector>
#include "digitalkhatt/project/GlyphSources.h"

struct GlyphSourceBlock {
  int start = 0, length = 0; // UTF-16 positions for existing Qt editor clients
  QString name, source;
};

// Qt adapter only; parsing and lossless rendering live in the shared library.
class GlyphSourceDocument {
 public:
  GlyphSourceDocument() = default;
  explicit GlyphSourceDocument(const digitalkhatt::project::GlyphSourceDocument& document);
  bool read(const QString& path, QString* error = nullptr);
  bool parse(const QByteArray& bytes, QString* error = nullptr);
  QByteArray render(const QMap<int, QString>& replacements, const QStringList& appended = {}) const;
  QByteArray original;
  QVector<GlyphSourceBlock> blocks;
 private:
  void assign(const digitalkhatt::project::GlyphSourceDocument& document);
  digitalkhatt::project::GlyphSourceDocument document_;
};
