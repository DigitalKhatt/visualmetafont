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

#include "digitalkhatt/core/ByteBuffer.h"
#include <filesystem>
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
#include "ParameterJson.h"
#include "commontypes.h"
#include "global.h"
#include "hb.h"
#include <digitalkhatt/core/digitalkahtt_types.h>
#include <digitalkhatt/layout/ClassMap.h>

struct Lookup;
class MPFont;
struct hb_font_t;
struct hb_face_t;
class Automedina;
class GlyphVis;
class ToOpenType;
struct Subtable;
struct MarkBaseSubtable;
struct mp_graphic_object;

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

class OtLayout {

  friend class Automedina;
  friend class GlyphVis;
  friend class ToOpenType;

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

  OtLayout(MPFont* font, bool extended,
           bool generateVariableOpenType = false);
  ~OtLayout();

  void loadLookupFile(std::string fileName);

  void parseFeatureFile(std::string fileName);
  hb_font_t* createFont(double scale, bool newFace = true);
  std::unordered_set<std::uint16_t> classtoUnicode(const std::string& className);
  std::unordered_set<std::uint16_t> regexptoUnicode(const std::string& regexp);

  void saveParameters(ParameterJsonObject& json) const;
  void readParameters(const ParameterJsonObject& json);

  void addClass(std::string name, std::unordered_set<std::string> set);

  digitalkhatt::ByteBuffer getGSUB();
  digitalkhatt::ByteBuffer getGPOS();
  digitalkhatt::ByteBuffer getGDEF();

 public:
  static int SCALEBY;  // = 8;
  static double EMSCALE;
  static int MINSPACEWIDTH;  // = 8;
  static int SPACEWIDTH;
  static int MAXSPACEWIDTH;

  std::string import;

  std::vector<Lookup*> gsublookups;
  std::vector<Lookup*> gposlookups;
  std::vector<Lookup*> lookups;
  std::map<std::string, std::set<Lookup*>> allFeatures;

  std::map<std::string, int> gsublookupsIndexByName;
  std::map<std::string, int> gposlookupsIndexByName;
  std::map<std::string, int> lookupsIndexByName;

  digitalkhatt::ByteBuffer scriptList;

  hb_face_t* face;

  bool dirty;

  digitalkhatt::ByteBuffer gsub_array;
  digitalkhatt::ByteBuffer gpos_array;
  digitalkhatt::ByteBuffer gdef_array;

  std::unordered_map<std::string, std::uint16_t> glyphCodePerName;
  std::map<std::uint16_t, std::string> glyphNamePerCode;
  std::map<std::uint16_t, std::uint16_t> unicodeToGlyphCode;

  std::map<std::uint16_t, GDEFClasses> glyphGlobalClasses;

  CalcAnchor getanchorCalcFunctions(const std::string& functionName, Subtable* subtable);
  CursiveAnchorFunc getCursiveFunctions(const std::string& functionName, Subtable* subtable);
  PairAdjustFunc getPairAdjustFunction(std::string functionName, Subtable* subtable);

  std::unordered_map<std::string, GlyphVis> glyphs;

  std::vector<std::vector<std::uint16_t>> markGlyphSets;

  std::uint16_t addMarkSet(std::vector<std::uint16_t> list);
  std::uint16_t addMarkSet(const std::vector<std::string>& list);

  void generateSubstEquivGlyphs();
  void generateSubstEquivGlyphsLegacy();

  void addLookup(Lookup* lookup);
  void addTable(Lookup* lookup) {
    this->addLookup(lookup);
    this->tables.push_back(lookup);
  };

  MPFont* font;

  double nuqta();

  GlyphVis* getGlyph(int code);
  GlyphVis* getGlyph(const std::string& name, GlyphParameters parameters);
  GlyphVis* getGlyph(int code, GlyphParameters parameters);

  int tajweedcolorindex = 0xFFFF;

  std::vector<LineLayoutInfo> justifyPage(double emScale, int lineWidth, int pageWidth, std::vector<std::string> lines, LineJustification justification, bool newFace, bool tajweedColor, std::string mushafLayoutType = {}) {
    return justifyPage(emScale, lineWidth, pageWidth, lines, justification, newFace, tajweedColor, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, {JustType::HarfBuzz, JustStyle::None, ShrinkType::None}, mushafLayoutType);
  }

  std::vector<LineLayoutInfo> justifyPage(double emScale, int lineWidth, int pageWidth, std::vector<std::string> lines, LineJustification justification, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level, JustOption justOption, std::string mushafLayoutType);
  std::vector<LineLayoutInfo> justifyPage(double emScale, int pageWidth, const std::vector<LineToJustify>& lines, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level, JustOption justOption, std::string mushafLayoutType);

  std::vector<LineLayoutInfo> justifyPageUsingFeatures(double emScale, int pageWidth, const std::vector<LineToJustify>& lines, bool newFace, bool tajweedColor,
                                                       hb_buffer_cluster_level_t cluster_level, JustOption justOption, std::string mushafLayout);
  LayoutPages pageBreak(std::vector<digitalkhatt::TextString> pages, double emScale, int lineWidth, bool pageFinishbyaVerse, int lastPage, hb_buffer_cluster_level_t cluster_level = HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
  OriginalPageList pageBreak(double emScale, int lineWidth,
                             bool pageFinishbyaVerse,
                             digitalkhatt::TextString text,
                             std::unordered_set<int> forcedBreaks, int nbPages);
  OriginalPageList pageBreak(double emScale, int lineWidth,
                             bool pageFinishbyaVerse,
                             digitalkhatt::TextString text, int nbPages);

  bool applyJustification = true;

  GlyphVis* getAlternate(int glyphCode, GlyphParameters parameters, bool generateNewGlyph = false, bool addToEquivSubst = false);
  std::unordered_map<GlyphParameters, GlyphVis*>& getSubstEquivGlyphs(int glyphCode);
  hb_position_t gethHorizontalAdvance(hb_font_t* hbFont, hb_codepoint_t glyph, GlyphParameters parameters, void* userData);

  void clearAlternates();

  bool parseCppLookup(const std::string& lookupName);

  digitalkhatt::ByteBuffer getCmap();
  mp_graphic_object* copyEdgeBody(mp_graphic_object* source) const;

  ToOpenType* toOpenType = nullptr;

  void setDisabled(Lookup* lookup);
  void setLookupDisabled(Lookup* lookup, bool disabled);
  void setLookupDisabled(std::string lookupName, bool disabled);

  void executeFSM(FSMSubtable& subtable, OT::hb_ot_apply_context_t* c) {
    fsmDriver.executeFSM(subtable, c);
  }

  JustificationContext justificationContext;

  bool isOTVar = false;

  bool useNormAxisValues = true;
  // Make the live MetaPost font use the integer advances written to hmtx.
  // This is useful when comparing live shaping with a generated OpenType font.
  bool quantizeGlyphAdvances = false;

  std::unordered_map<std::string, ValueLimits> expandableGlyphs;

  std::pair<int, int> getDeltaSetEntry(DefaultDelta delta, int subregionIndex);

  digitalkhatt::ByteBuffer JTST();

  Just justTable;

  float normalToParameter(unsigned int code, float tatweel, bool left);
  static int AlternatelastCode;
  bool isExtended() const { return extended; }
  void setExtended(bool value) { extended = value; }

  std::unordered_set<std::uint16_t> getSubsts(int charCode);

  const digitalkhatt::layout::ClassMap& glyphClasses() const;
  std::unordered_set<std::uint16_t> classToUnicode(const std::string& className);
  std::map<std::uint16_t, std::vector<ExtendedGlyph>>& resetCvxxFeatures();

 private:
  // void evaluateImport();
  // void prepareJSENgine();

  Automedina* automedina;

  std::map<std::string, std::set<std::uint16_t>> allGposFeatures;
  std::map<std::string, std::set<std::uint16_t>> allGsubFeatures;

  digitalkhatt::ByteBuffer getGSUBorGPOS(bool isgsub, std::vector<Lookup*>& lookups, std::map<std::string, std::set<std::uint16_t>>& allFeatures, std::map<std::string, int>& lookupsIndexByName);
  digitalkhatt::ByteBuffer getFeatureList(const std::map<std::string, std::set<std::uint16_t>>& allFeatures);
  digitalkhatt::ByteBuffer getScriptList(int featureCount);

  double _nuqta = -1;

  // Lookup objects are recreated whenever a feature file is parsed. Keep the
  // disabled state by name so it survives those reparses during font export.
  std::unordered_set<std::string> disabledLookups;

  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> tempGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> addedGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> substEquivGlyphs;

  bool JustificationInProgress = false;

  void applyJustFeature(hb_buffer_t* buffer, bool& needgpos, double& diff, const std::string& feature, hb_font_t* shapefont, double nuqta, double emScale);
  void applyJustFeature_old(hb_buffer_t* buffer, bool& needgpos, double& diff, const std::string& feature, hb_font_t* shapefont, double nuqta, double emScale);

  void jutifyLine(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, bool tajweedColor);
  void jutifyLine_old(hb_font_t* shapefont, hb_buffer_t* buffer, int lineWidth, double emScale, bool tajweedColor);

  bool extended = true;

  std::vector<Lookup*> tables;

  FSMDriver fsmDriver;

  // std::unordered_map<DefaultDelta, int> defaultDeltaSets;
};

#endif  // OTLAYOUT_H
