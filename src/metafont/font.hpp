/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef FONT_H
#define FONT_H
#include <QHash>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

#include <string>
#include <string_view>

#include "MPFont.h"
#include "projectmetadata.hpp"
#include "glyphsourcedocument.hpp"

struct MPGlyphInfo {
  mp_edge_object* currentPicture = nullptr;
  QMap<QString, mp_edge_object*> controlledPictures;
  QMap<QString, mp_edge_object*> controlledPictureSources;
};

class GlyphVis;
class Glyph;
typedef struct MP_instance* MP;
struct mp_edge_object;
typedef struct mp_graphic_object mp_graphic_object;
class Font : public QObject {
  Q_OBJECT

 public:
  friend GlyphVis;
  Font(QObject* parent = Q_NULLPTR);
  ~Font();
  bool loadFile(const QString& fileName);
  bool saveFile();
  bool hasSourceChanges();
  bool glyphHasSourceChanges(Glyph* glyph) const;
  bool saveUnicodes();
  ProjectMetadata project;
  QString projectError;
  bool saveProjectMetadata();
  QStringList glyphSourceFiles() const;
  QString glyphSourceFile(const Glyph* glyph) const;
  bool createGlyphFile(const QString& relativePath);
  Glyph* createGlyph(const QString& name, int unicode, const QString& sourceFile);
  bool removeGlyph(Glyph* glyph);
  bool moveGlyphs(const QVector<Glyph*>& selected, const QString& sourceFile);
  QVector<Glyph*> glyphs;
  QHash<QString, Glyph*> glyphperName;
  QString filePath();
  std::string filePathStd() const;
  QString fontName();
  double lineHeight();
  double getNumericVariable(QString name);
  double getInternalNumericVariable(QString name);
  bool anchorsVisible() const { return anchorsVisible_; }
  void setAnchorsVisible(bool visible);
  bool getPairVariable(QString name, QPointF& point);
  bool getBoolVariable(QString name);
  Glyph* getGlyph(uint code);
  QString path() {
    return m_path;
  }
  QString currentDir() {
    return m_currentDir;
  }
  std::string executeMetaPost(std::string command);
  std::vector<mp_edge_object*> getEdges() const;
  mp_edge_object* getEdge(int charCode);
  MPGlyphInfo getMPGlyphInfo(int charCode);
  bool hasGlyph(std::string_view glyphName) const;
  mp_graphic_object* copyEdgeBody(mp_graphic_object* source);
  QString getLog();
  MPFont& mpFont();
  QString familyName();
  QString copyright();
  std::string familyNameStd();
  std::string copyrightStd();

  QVector<QString> pictureNames;

 signals:
  void anchorsVisibilityChanged(bool visible);
  void sourcesSaved();

 private:
  void synchronizeAnchorVisibility();
  bool anchorsVisible_ = false;
  struct SourceFile { QString path; GlyphSourceDocument document; };
  struct GlyphOrigin { int file = -1, block = -1; QString canonical, target; };
  QVector<SourceFile> sourceFiles_;
  QHash<const Glyph*, GlyphOrigin> glyphOrigins_;
  void orderGlyphsBySource();
  void synchronizeGlyphSources();
  QString m_path;
  QString m_fontName;
  QString m_currentDir;

  MPFont m_mpFont;
};
#endif  // FONT_H
