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

#include <qstring.h>
#include <QByteArray>
#include <QMap>
#include <QSet>
#include <QVector>
#include <qpoint.h>
#include <optional>
#include <unordered_map>
#include <set>
#include <QDataStream>
#include "to_opentype.h"
#include "FSMDriver.h"
#include "JustificationContext.h"
#include "qobject.h"
#include "commontypes.h"
#include <stdexcept>
#include <iostream>



class Lookup;
class QJsonObject;
class Font;
struct hb_font_t;
struct hb_face_t;
class Automedina;
class GlyphVis;
struct Subtable;
class MarkBaseSubtable;

struct hb_buffer_t;

typedef struct MP_instance* MP;

struct GlyphParameters {
  double lefttatweel;
  double righttatweel;
  std::optional<double> leftextratio;
  std::optional<double> rightextratio;
  std::optional<double> left_tatweeltension;
  std::optional<double> right_tatweeltension;
  std::optional<int> which_in_baseline;

  bool operator==(const GlyphParameters& r) const {
    return r.lefttatweel == lefttatweel && r.righttatweel == righttatweel && r.leftextratio == leftextratio && r.rightextratio == rightextratio
      && r.left_tatweeltension == left_tatweeltension && r.right_tatweeltension == right_tatweeltension && r.which_in_baseline == which_in_baseline;
  }
};

namespace std {
  template<>
  struct hash<GlyphParameters> {
    size_t operator()(const GlyphParameters& r) const
    {
      return hash<double>{}(r.lefttatweel)
        ^ hash<double>{}(r.righttatweel)
        ^ hash<std::optional<double>>{}(r.leftextratio)
        ^ hash<std::optional<double>>{}(r.rightextratio)
        ^ hash<std::optional<double>>{}(r.left_tatweeltension)
        ^ hash<std::optional<double>>{}(r.right_tatweeltension)
        ^ hash<std::optional<double>>{}(r.which_in_baseline);
    }
  };

  template<>
  struct equal_to<GlyphParameters> {
    bool operator()(const GlyphParameters& r, const GlyphParameters& r2) const
    {
      return r == r2;
    }
  };
}



struct GlyphLayoutInfo {
  int advance;
  int x_offset;
  int y_offset;
  int x_advance;
  int y_advance;
  int codepoint;
  int cluster;
  unsigned int lookup_index;
  unsigned int subtable_index;
  double lefttatweel = 0;
  double righttatweel = 0;
  uint32_t base_codepoint;
  bool beginsajda;
  bool endsajda;
  uint32_t color = 0;
};

enum class  LineType {
  Line = 0,
  Sura = 1,
  Bism = 2
};

struct LineLayoutInfo {
  std::vector<GlyphLayoutInfo> glyphs;
  int xstartposition;
  int ystartposition;
  LineType type = LineType::Line;
  float underfull;
};

struct LayoutPages {
  QList<QList<LineLayoutInfo>> pages;
  QList<QStringList> originalPages;
  QList<QString> suraNamebyPage;
};

struct SuraLocation {
  QString name;
  int pageNumber;
  int x;
  int y;
};

QDataStream& operator<<(QDataStream& stream, const SuraLocation& location);
QDataStream& operator>>(QDataStream& stream, SuraLocation& location);

enum class LineJustification {
  Center,
  Distribute
};

using CalcAnchor = std::function<QPoint(QString, QString, QPoint, double, double)>;

class AnchorCalc {
public:
  virtual QPoint operator()(QString glyphName, QString className, QPoint adjust, double lefttatweel = 0.0, double righttatweel = 0.0) { return QPoint(0, 0); };
  QPoint getAdjustment(Automedina& y, MarkBaseSubtable& subtable, GlyphVis* curr, QString className, QPoint adjust, double lefttatweel, double righttatweel, GlyphVis** poriginalglyph);

};

struct Just {

  Just(OtLayout* layout) : layout{ layout } {}

  struct JustStep {
    bool gsub = false;
    std::vector<Lookup*> lookups;
  };

  std::vector<JustStep> stretchSteps;
  std::vector<JustStep> shrinkSteps;
  QByteArray getOpenTypeTable();

private:
  OtLayout* layout;

};

#ifdef DIGITALKHATT_WEBLIB
class OtLayout {
#else
class OtLayout : public QObject {
  Q_OBJECT
#endif
    friend class Automedia;
  friend class GlyphVis;
  friend class LayoutWindow;
  friend class ToOpenType;
public:

  constexpr static int FrameHeight = 27400;
  constexpr static int FrameWidth = 17000;
  constexpr static int InterLineSpacing = 1800; // 1785;
  constexpr static int TopSpace = 1450; //1600
  constexpr static int Margin = 400;


  enum GDEFClasses {
    BaseGlyph = 1,
    LigatureGlyph = 2,
    MarkGlyph = 3,
    ComponentGlyph = 4
  };

#if DIGITALKHATT_WEBLIB
  OtLayout(MP mp, bool extended);
#else
  OtLayout(MP mp, bool extended, QObject* parent = Q_NULLPTR);
#endif
  ~OtLayout();

  void loadLookupFile(std::string fileName);

  void readJson(const QJsonObject& json);
  hb_font_t* createFont(int scale, bool newFace = true);
  QSet<quint16> classtoUnicode(QString className);
  QSet<quint16> regexptoUnicode(QString regexp);

  QSet<QString> classtoGlyphName(QString className);
  void saveParameters(QJsonObject& json) const;
  void readParameters(const QJsonObject& json);

  void addClass(QString name, QSet<QString> set);

  QByteArray getGSUB();
  QByteArray getGPOS();
  QByteArray getGDEF();

public:

  static int SCALEBY; // = 8;
  static double EMSCALE;
  static int MINSPACEWIDTH; // = 8;
  static int SPACEWIDTH;
  static int MAXSPACEWIDTH;

  QString import;

  QVector<Lookup*> gsublookups;
  QVector<Lookup*> gposlookups;
  QVector<Lookup* > lookups;
  QMap<QString, QSet<Lookup*> > allFeatures;


  QMap<QString, int> gsublookupsIndexByName;
  QMap<QString, int> gposlookupsIndexByName;
  QMap<QString, int> lookupsIndexByName;

  QByteArray scriptList;
  Font* m_font;
  hb_face_t* face;

  bool dirty;

  QByteArray gsub_array;
  QByteArray gpos_array;
  QByteArray gdef_array;

  //QJSEngine myEngine;

  QMap<QString, quint16> glyphCodePerName;
  QMap<quint16, QString> glyphNamePerCode;
  QMap<quint16, quint16> unicodeToGlyphCode;

  QMap<quint16, GDEFClasses> glyphGlobalClasses;

  //QMap<QString, AnchorCalc*> anchorCalcFunctions;
  CalcAnchor getanchorCalcFunctions(QString functionName, Subtable* subtable);
  void setParameter(quint16 glyphCode, quint32 lookup, quint32 subtable, quint16 markCode, quint16 baseCode, QPoint displacement, Qt::KeyboardModifiers modifiers);

  QHash<QString, GlyphVis> glyphs;

  QVector<QList<quint16>> markGlyphSets;

  quint16 addMarkSet(QList<quint16> list);
  quint16 addMarkSet(QVector<QString> list);

  void addLookup(Lookup* lookup);
  void addTable(Lookup* lookup) {
    this->addLookup(lookup);
    this->tables.push_back(lookup);
  };

  MP mp;

  double nuqta();
  double getNumericVariable(QString name);

  GlyphVis* getGlyph(int code);
  GlyphVis* getGlyph(QString name, double lefttatweel, double righttatweel);
  GlyphVis* getGlyph(int code, double lefttatweel, double righttatweel);

  int tajweedcolorindex = 0xFFFF;

  QList<LineLayoutInfo> justifyPage(int emScale, int lineWidth, int pageWidth, QStringList lines, LineJustification justification, bool newFace, bool tajweedColor);
  LayoutPages pageBreak(int emScale, int lineWidth, bool pageFinishbyaVerse);

  bool applyJustification = true;

  GlyphVis* getAlternate(int glyphCode, GlyphParameters parameters, bool generateNewGlyph = false);

  void clearAlternates();

  void parseCppJsonLookup(QString lookupName, const QJsonObject& json);

  QByteArray getCmap();

  ToOpenType* toOpenType = nullptr;

  void setDisabled(Lookup* lookup) {
    disabledLookups.insert(lookup);
  }

  void executeFSM(FSMSubtable& subtable, OT::hb_ot_apply_context_t* c) {
    fsmDriver.executeFSM(subtable, c);
  }

  JustificationContext justificationContext;

  bool isOTVar = true;

  bool useNormAxisValues = true;

  enum class WhichJust {
    First,
    Second,
    HarfBuzz
  };

  WhichJust whichJust = WhichJust::HarfBuzz;

  std::unordered_map<QString, ValueLimits> expandableGlyphs;

  std::pair<int,int> getDeltaSetEntry(DefaultDelta delta, const ValueLimits& limits) {
    return toOpenType->getDeltaSetEntry(delta, limits);
    /*
    const auto& it = defaultDeltaSets.find(delta);
    if (it != defaultDeltaSets.end()) {
      return { 0,it->second };
    }
    else {
      int val = defaultDeltaSets.size();
      defaultDeltaSets.insert({ delta ,val });
      return { 0,val };
    }*/
    
  } 
  
  QByteArray JTST();

  Just justTable;  

  double normalToParameter(unsigned int code, float tatweel, bool left) {

    if (!useNormAxisValues || tatweel == 0.0)
      return tatweel;

    if (tatweel < -1) {
      //throw new std::runtime_error("tatweel error for glyph " + code);
      auto& name = glyphNamePerCode.value(code);
      std::cout.precision(17);
      std::cout << "min tatweel " << std::fixed << tatweel << " error for glyph " << name.toStdString() << '\n';
      tatweel = -1;
    }

    if (tatweel > 1) {
      //throw new std::runtime_error("tatweel error for glyph " + code);
      auto& name = glyphNamePerCode.value(code);
      std::cout.precision(17);
      std::cout << "max tatweel " << std::fixed << tatweel << " error for glyph " << name.toStdString() << '\n';
      tatweel = 1;
    }

    ValueLimits limits;

    auto& name = glyphNamePerCode.value(code);

    auto& find = expandableGlyphs.find(name);

    if (find == expandableGlyphs.end()) {
      //throw new std::runtime_error("tatweel error for glyph " + name.toStdString());
      std::cout << "No expandable glyph " + name.toStdString();
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
    }
    else {
      return (tatweel * max);
    }

  }

#ifndef DIGITALKHATT_WEBLIB
signals:
  void parameterChanged();
#endif



private:
  //void evaluateImport();
  //void prepareJSENgine();

  Automedina* automedina;

  QMap<QString, QSet<quint16> > allGposFeatures;
  QMap<QString, QSet<quint16> > allGsubFeatures;



  QByteArray getGSUBorGPOS(bool isgsub, QVector<Lookup*>& lookups,
    QMap<QString, QSet<quint16>>& allFeatures, QMap<QString, int>& lookupsIndexByName);
  QByteArray getFeatureList(QMap<QString, QSet<quint16>> allFeatures);
  QByteArray getScriptList(int featureCount);


  double _nuqta = -1;

  QSet<Lookup*> disabledLookups;

  static int AlternatelastCode;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> alternatePaths;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> nojustalternatePaths;

  bool JustificationInProgress = false;

  void applyJustFeature(hb_buffer_t* buffer, bool& needgpos, double& diff, QString feature, hb_font_t* shapefont, double nuqta, int emScale);
  void applyJustFeature_old(hb_buffer_t* buffer, bool& needgpos, double& diff, QString feature, hb_font_t* shapefont, double nuqta, int emScale);

  void jutifyLine(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, int emScale, bool tajweedColor);
  void jutifyLine_old(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, int emScale, bool tajweedColor);

  bool extended = true;

  std::vector<Lookup*> tables;

  FSMDriver fsmDriver;


  //std::unordered_map<DefaultDelta, int> defaultDeltaSets;



};

#endif // OTLAYOUT_H
