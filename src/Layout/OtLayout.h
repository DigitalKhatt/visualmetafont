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

#ifndef OTLAYOUT_H
#define OTLAYOUT_H

#include <qpoint.h>
#include <qstring.h>

#include <QDataStream>
#include "digitalkhatt/core/ByteBuffer.h"
#include <QMap>
#include <QSet>
#include <QVector>
#include <iostream>
#include <concepts>
#include <cstdint>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "FSMDriver.h"
#include "JustificationContext.h"
#include "commontypes.h"
#include "global.h"
#include "hb.h"
#include "qobject.h"
#include "to_opentype.h"
#include <digitalkhatt/core/digitalkahtt_types.h>

struct Lookup;
class QJsonObject;
class Font;
struct hb_font_t;
struct hb_face_t;
class Automedina;
class GlyphVis;
struct Subtable;
struct MarkBaseSubtable;

struct hb_buffer_t;

typedef struct MP_instance* MP;

struct ExtendedGlyph {
  int code;
  double lefttatweel;
  double righttatweel;
};

using GlyphLayoutInfo = digitalkhatt::GlyphLayoutInfo;
using LineType = digitalkhatt::LineType;
using LineLayoutInfo = digitalkhatt::LineLayoutInfo;
using LineJustification = digitalkhatt::LineJustification;
using LineToJustify = digitalkhatt::LineToJustify;
using JustType = digitalkhatt::JustType;
using JustStyle = digitalkhatt::JustStyle;
using ShrinkType = digitalkhatt::ShrinkType;
using JustOption = digitalkhatt::JustOption;

using LayoutPage = std::vector<LineLayoutInfo>;
using LayoutPageList = std::vector<LayoutPage>;
using OriginalPage = std::vector<digitalkhatt::TextString>;
using OriginalPageList = std::vector<OriginalPage>;

struct LayoutPages {
  LayoutPageList pages;
  OriginalPageList originalPages;
  std::vector<digitalkhatt::TextString> suraNamebyPage;
};

struct SuraLocation {
  digitalkhatt::TextString name;
  int pageNumber;
  int x;
  int y;
};

inline digitalkhatt::TextString makeSuraLocationName(digitalkhatt::TextView name, int suraNumber) {
  digitalkhatt::TextString result{name};
  result += u" ( ";
  const auto number = std::to_string(suraNumber);
  result.append(number.begin(), number.end());
  result += u" )";
  return result;
}

QDataStream& operator<<(QDataStream& stream, const SuraLocation& location);
QDataStream& operator>>(QDataStream& stream, SuraLocation& location);
inline QDataStream& operator<<(QDataStream& stream,
                               const digitalkhatt::ByteBuffer& buffer) {
  stream.writeRawData(reinterpret_cast<const char*>(buffer.data()),
                      static_cast<int>(buffer.size()));
  return stream;
}

struct ValueRecord {
  std::int16_t xPlacement;
  std::int16_t yPlacement;
  std::int16_t xAdvance;
  std::int16_t yAdvance;

  bool operator==(const ValueRecord& rhs) const {
    return xPlacement == rhs.xPlacement && yPlacement == rhs.yPlacement &&
           xAdvance == rhs.xAdvance && yAdvance == rhs.yAdvance;
  }

  bool isEmpty() const {
    ValueRecord empty{};
    return *this == empty;
  }

  std::uint8_t format() const {
    std::uint8_t f = xPlacement == 0 ? 0 : 1;
    if (yPlacement != 0) {
      f = f | 0x02;
    }
    if (xAdvance != 0) {
      f = f | 0x04;
    }
    if (yAdvance != 0) {
      f = f | 0x08;
    }
    return f;
  }
};

struct Point {
  constexpr Point() = default;
  constexpr Point(int x, int y) : x_{x}, y_{y} {}

  template <typename T>
    requires requires(const T& point) {
      { point.x() } -> std::convertible_to<int>;
      { point.y() } -> std::convertible_to<int>;
    }
  constexpr Point(const T& point) : x_{point.x()}, y_{point.y()} {}

  template <typename T>
    requires std::constructible_from<T, int, int>
  constexpr operator T() const {
    return T{x_, y_};
  }

  constexpr int x() const { return x_; }
  constexpr int y() const { return y_; }
  constexpr void setX(int x) { x_ = x; }
  constexpr void setY(int y) { y_ = y; }
  constexpr bool isNull() const { return x_ == 0 && y_ == 0; }

  constexpr Point& operator+=(Point rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  constexpr Point& operator-=(Point rhs) {
    x_ -= rhs.x_;
    y_ -= rhs.y_;
    return *this;
  }

  friend constexpr Point operator+(Point lhs, Point rhs) { return lhs += rhs; }
  friend constexpr Point operator-(Point lhs, Point rhs) { return lhs -= rhs; }
  friend constexpr bool operator==(Point, Point) = default;

 private:
  int x_{};
  int y_{};
};

using CalcAnchor = std::function<Point(std::string, std::string, Point, GlyphParameters)>;
using CursiveAnchorFunc = std::function<Point(bool, GlyphVis*, GlyphVis*)>;
using PairAdjustFunc = std::function<ValueRecord(GlyphVis*, GlyphVis*)>;

class AnchorCalc {
 public:
  virtual Point operator()(std::string glyphName, std::string className,
                           Point adjust, GlyphParameters parameters) {
    return {};
  };
  Point getAdjustment(Automedina& y, MarkBaseSubtable& subtable, GlyphVis* curr,
                      const std::string& className, Point adjust,
                      GlyphParameters parameters, GlyphVis** poriginalglyph);
};

struct Just {
  Just(OtLayout* layout) : layout{layout} {}

  struct JustStep {
    bool gsub = false;
    std::vector<Lookup*> lookups;
  };

  std::vector<JustStep> stretchSteps;
  std::vector<JustStep> shrinkSteps;
  std::vector<Lookup*> lastGsubLookups;
  digitalkhatt::ByteBuffer getOpenTypeTable();

 private:
  OtLayout* layout;
};

Q_DECLARE_METATYPE(JustType)
Q_DECLARE_METATYPE(JustStyle)
Q_DECLARE_METATYPE(ShrinkType)
#ifdef DIGITALKHATT_WEBLIB
class OtLayout {
#else
class OtLayout : public QObject {
  Q_OBJECT
#endif

  friend class Automedina;
  friend class GlyphVis;
  friend class LayoutWindow;
  friend class ToOpenType;
  friend class GenerateLayout;

 public:
  constexpr static int FrameHeight = 27400;
  constexpr static int FrameWidth = 17000;
  constexpr static int InterLineSpacing = 1800;  // (1.5969)
  // constexpr static int InterLineSpacing = 1690; //(1.5)
  constexpr static int TopSpace = 1450;  // 1600
  constexpr static int Margin = 300;
  // 15500 for oldMadinah
  constexpr static int TextWidth = FrameWidth - (2 * Margin);

  enum GDEFClasses {
    BaseGlyph = 1,
    LigatureGlyph = 2,
    MarkGlyph = 3,
    ComponentGlyph = 4
  };

#if defined DIGITALKHATT_WEBLIB
  OtLayout(Font* font, bool extended);
#else
  OtLayout(Font* font, bool extended, bool generateVariableOpenType, QObject* parent = Q_NULLPTR);
#endif
  ~OtLayout();

  void loadLookupFile(std::string fileName);

  void parseFeatureFile(std::string fileName);
  hb_font_t* createFont(double scale, bool newFace = true);
  std::unordered_set<std::uint16_t> classtoUnicode(const std::string& className);
  std::unordered_set<std::uint16_t> regexptoUnicode(const std::string& regexp);

  void saveParameters(QJsonObject& json) const;
  void readParameters(const QJsonObject& json);

  void addClass(QString name, QSet<QString> set);

  digitalkhatt::ByteBuffer getGSUB();
  digitalkhatt::ByteBuffer getGPOS();
  digitalkhatt::ByteBuffer getGDEF();

 public:
  static int SCALEBY;  // = 8;
  static double EMSCALE;
  static int MINSPACEWIDTH;  // = 8;
  static int SPACEWIDTH;
  static int MAXSPACEWIDTH;

  QString import;

  QVector<Lookup*> gsublookups;
  QVector<Lookup*> gposlookups;
  QVector<Lookup*> lookups;
  QMap<QString, QSet<Lookup*>> allFeatures;

  QMap<QString, int> gsublookupsIndexByName;
  QMap<QString, int> gposlookupsIndexByName;
  QMap<QString, int> lookupsIndexByName;

  digitalkhatt::ByteBuffer scriptList;

  hb_face_t* face;

  bool dirty;

  digitalkhatt::ByteBuffer gsub_array;
  digitalkhatt::ByteBuffer gpos_array;
  digitalkhatt::ByteBuffer gdef_array;

  std::unordered_map<std::string, std::uint16_t> glyphCodePerName;
  std::map<std::uint16_t, std::string> glyphNamePerCode;
  std::map<std::uint16_t, std::uint16_t> unicodeToGlyphCode;

  QMap<quint16, GDEFClasses> glyphGlobalClasses;

  // QMap<QString, AnchorCalc*> anchorCalcFunctions;
  CalcAnchor getanchorCalcFunctions(const std::string& functionName, Subtable* subtable);
  CursiveAnchorFunc getCursiveFunctions(const std::string& functionName, Subtable* subtable);
  PairAdjustFunc getPairAdjustFunction(std::string functionName, Subtable* subtable);
  void setParameter(quint16 glyphCode, quint32 lookup, quint32 subtable, quint16 markCode, quint16 baseCode, QPoint displacement, Qt::KeyboardModifiers modifiers);

  std::unordered_map<std::string, GlyphVis> glyphs;

  QVector<QList<quint16>> markGlyphSets;

  quint16 addMarkSet(QList<quint16> list);
  quint16 addMarkSet(QVector<QString> list);

  void generateSubstEquivGlyphs();

  void addLookup(Lookup* lookup);
  void addTable(Lookup* lookup) {
    this->addLookup(lookup);
    this->tables.push_back(lookup);
  };

  Font* font;

  double nuqta();

  GlyphVis* getGlyph(int code);
  GlyphVis* getGlyph(const std::string& name, GlyphParameters parameters);
  GlyphVis* getGlyph(int code, GlyphParameters parameters);

  int tajweedcolorindex = 0xFFFF;

  QList<LineLayoutInfo> justifyPage(double emScale, int lineWidth, int pageWidth, QStringList lines, LineJustification justification, bool newFace, bool tajweedColor, QString mushafLayoutType) {
    return justifyPage(emScale, lineWidth, pageWidth, lines, justification, newFace, tajweedColor, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, {JustType::HarfBuzz, JustStyle::None, ShrinkType::None}, mushafLayoutType);
  }

  QList<LineLayoutInfo> justifyPage(double emScale, int lineWidth, int pageWidth, QStringList lines, LineJustification justification, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level, JustOption justOption, QString mushafLayoutType);
  QList<LineLayoutInfo> justifyPage(double emScale, int pageWidth, const QVector<LineToJustify>& lines, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level, JustOption justOption, QString mushafLayoutType);

  QList<LineLayoutInfo> justifyPageUsingFeatures(double emScale, int pageWidth, const QVector<LineToJustify>& lines, bool newFace, bool tajweedColor,
                                                 hb_buffer_cluster_level_t cluster_level, JustOption justOption, QString mushafLayout);
  LayoutPages pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, int lastPage, hb_buffer_cluster_level_t cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
  QList<QStringList> pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, QString text, QSet<int> forcedBreaks, int nbPages);
  QList<QStringList> pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, QString text, int nbPages);

  bool applyJustification = true;

  GlyphVis* getAlternate(int glyphCode, GlyphParameters parameters, bool generateNewGlyph = false, bool addToEquivSubst = false);
  std::unordered_map<GlyphParameters, GlyphVis*>& getSubstEquivGlyphs(int glyphCode);
  hb_position_t gethHorizontalAdvance(hb_font_t* hbFont, hb_codepoint_t glyph, GlyphParameters parameters, void* userData);

  void clearAlternates();

  bool parseCppLookup(QString lookupName);

  digitalkhatt::ByteBuffer getCmap();

  ToOpenType* toOpenType = nullptr;

  void setDisabled(Lookup* lookup) {
    disabledLookups.insert(lookup);
  }

  void executeFSM(FSMSubtable& subtable, OT::hb_ot_apply_context_t* c) {
    fsmDriver.executeFSM(subtable, c);
  }

  JustificationContext justificationContext;

  bool isOTVar = false;

  bool useNormAxisValues = true;

  std::unordered_map<std::string, ValueLimits> expandableGlyphs;

  std::pair<int, int> getDeltaSetEntry(DefaultDelta delta, const int subregionIndex) {
    return toOpenType->getDeltaSetEntry(delta, subregionIndex);
  }

  digitalkhatt::ByteBuffer JTST();

  Just justTable;

  float normalToParameter(unsigned int code, float tatweel, bool left) {
    if (!useNormAxisValues || tatweel == 0.0)
      return tatweel;

    if (tatweel < -1) {
      // throw new std::runtime_error("tatweel error for glyph " + code);
      const auto& name = glyphNamePerCode.at(code);
      std::cout.precision(17);
      std::cout << "min tatweel " << std::fixed << tatweel << " error for glyph " << name << '\n';
      tatweel = -1;
    }

    if (tatweel > 1) {
      // throw new std::runtime_error("tatweel error for glyph " + code);
      const auto& name = glyphNamePerCode.at(code);
      std::cout.precision(17);
      std::cout << "max tatweel " << std::fixed << tatweel << " error for glyph " << name << '\n';
      tatweel = 1;
    }

    ValueLimits limits;

    const auto& name = glyphNamePerCode.at(code);

    const auto& find = expandableGlyphs.find(name);

    if (find == expandableGlyphs.end()) {
      // throw new std::runtime_error("tatweel error for glyph " + name.toStdString());
      std::cout << "No expandable glyph " + name + "\n";
      return tatweel;
    }

    limits = find->second;

    double min = left ? limits.minLeft : limits.minRight;
    double max = left ? limits.maxLeft : limits.maxRight;

    if (toOpenType->isUniformAxis()) {
      min = left ? toOpenType->axisLimits.minLeft : toOpenType->axisLimits.minRight;
      max = left ? toOpenType->axisLimits.maxLeft : toOpenType->axisLimits.maxRight;
    }

    if (tatweel < 0) {
      return (-tatweel * min);
    } else {
      return (tatweel * max);
    }
  }
  static int AlternatelastCode;
  bool isExtended() { return extended; }

  QSet<quint16> getSubsts(int charCode);

  void saveFontInfo();

#ifndef DIGITALKHATT_WEBLIB
 signals:
  void parameterChanged();
#endif

 private:
  // void evaluateImport();
  // void prepareJSENgine();

  Automedina* automedina;

  QMap<QString, QSet<quint16>> allGposFeatures;
  QMap<QString, QSet<quint16>> allGsubFeatures;

  digitalkhatt::ByteBuffer getGSUBorGPOS(bool isgsub, QVector<Lookup*>& lookups, QMap<QString, QSet<quint16>>& allFeatures, QMap<QString, int>& lookupsIndexByName);
  digitalkhatt::ByteBuffer getFeatureList(QMap<QString, QSet<quint16>> allFeatures);
  digitalkhatt::ByteBuffer getScriptList(int featureCount);

  double _nuqta = -1;

  QSet<Lookup*> disabledLookups;

  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> tempGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> addedGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> substEquivGlyphs;

  bool JustificationInProgress = false;

  void applyJustFeature(hb_buffer_t* buffer, bool& needgpos, double& diff, QString feature, hb_font_t* shapefont, double nuqta, double emScale);
  void applyJustFeature_old(hb_buffer_t* buffer, bool& needgpos, double& diff, QString feature, hb_font_t* shapefont, double nuqta, double emScale);

  void jutifyLine(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, bool tajweedColor);
  void jutifyLine_old(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, double emScale, bool tajweedColor);

  bool extended = true;

  std::vector<Lookup*> tables;

  FSMDriver fsmDriver;

  // std::unordered_map<DefaultDelta, int> defaultDeltaSets;
};

#endif  // OTLAYOUT_H
