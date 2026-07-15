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

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "JustificationContext.h"
#include "OtLayout.h"
// #include "hb-font.hh"

template <typename Map>
std::vector<typename Map::key_type> stdMapKeys(const Map& map) {
  std::vector<typename Map::key_type> keys;
  keys.reserve(map.size());
  for (const auto& [key, value] : map) keys.push_back(key);
  return keys;
}

template <typename String>
std::string asStdString(const String& value) {
  if constexpr (std::is_convertible_v<String, std::string>) {
    return value;
  } else {
    return value.toStdString();
  }
}

template <typename Range>
std::unordered_set<std::string> toStdStringSet(const Range& range) {
  std::unordered_set<std::string> result;
  result.reserve(range.size());
  for (const auto& value : range) {
    result.insert(asStdString(value));
  }
  return result;
}

struct Lookup;
class QJsonObject;
class AnchorCalc;
class Font;

using AddedGlyphSet = std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>>;

struct Subtable {
  friend class OtLayout;

  Subtable(Lookup* lookup);
  virtual ~Subtable() {};

  virtual void readJson(const QJsonObject& json) {};
  virtual QByteArray getOptOpenTypeTable(bool extended) {
    if (isDirty) {
      return getOpenTypeTable(extended);
    } else {
      return openTypeSubTable;
    }
  };
  virtual QByteArray getConvertedOpenTypeTable() {
    return getOpenTypeTable(false);
  }

  virtual void generateSubstEquivGlyphs() {
  }

  virtual QByteArray getOpenTypeTable(bool extended) {
    return QByteArray();
  };
  virtual std::uint16_t getCodeFromName(std::string name);
  virtual std::string getNameFromCode(std::uint16_t code);

  virtual void saveParameters(QJsonObject& json) const {}
  virtual void readParameters(const QJsonObject& json) {}

  virtual bool isExtended() { return false; }

  virtual bool isConvertible() { return false; }

  Lookup* getLookup() {
    return m_lookup;
  }

  std::string name;

 protected:
  Lookup* m_lookup;
  Font* metafont;
  OtLayout* m_layout;
  bool isDirty = true;
  QByteArray openTypeSubTable;
  void setVariationIndexOffset(
      QByteArray& anchorTables,
      quint32 anchorOffset,
      std::map<int, std::pair<int, std::pair<int, int>>>& posToVar);
};

struct SingleSubtable : Subtable {
  SingleSubtable(Lookup* lookup, std::uint16_t format = 2);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;

  std::map<std::uint16_t, std::uint16_t> subst;

  bool isExtended() override;

  std::uint16_t format;
};

struct SingleSubtableWithExpansion : SingleSubtable {
  SingleSubtableWithExpansion(Lookup* lookup);
  QByteArray getOpenTypeTable(bool extended) override;
  // void readJson(const QJsonObject &json) override;

  std::map<std::uint16_t, GlyphExpansion> expansion;

  bool isConvertible() override { return false; }
};

struct SingleSubtableWithTatweel : SingleSubtable {
  SingleSubtableWithTatweel(Lookup* lookup);
  // QByteArray getOpenTypeTable() override;
  // void readJson(const QJsonObject &json) override;

  std::map<std::uint16_t, GlyphExpansion> expansion;

  QByteArray getOpenTypeTable(bool extended) override;

  bool isConvertible() override { return true; }

  QByteArray getConvertedOpenTypeTable() override;

  virtual void generateSubstEquivGlyphs() override;
};

struct MultipleSubtable : Subtable {
  MultipleSubtable(Lookup* lookup);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;

  std::map<std::uint16_t, std::vector<std::uint16_t>> subst;

  std::uint16_t format = 1;
};

struct AlternateSubtable : Subtable {
  AlternateSubtable(Lookup* lookup, std::uint16_t format = 1);
  QByteArray getOpenTypeTable(bool extended) override;

  std::map<std::uint16_t, std::vector<ExtendedGlyph>> alternates;

  virtual void generateSubstEquivGlyphs() override;

  std::uint16_t format = 1;
};

struct AlternateSubtableWithTatweel : AlternateSubtable {
  AlternateSubtableWithTatweel(Lookup* lookup);

  QByteArray getOpenTypeTable(bool extended) override;

  bool isConvertible() override { return true; }

  QByteArray getConvertedOpenTypeTable() override;

  virtual void generateSubstEquivGlyphs() override;
};

struct LigatureSubtable : Subtable {
  LigatureSubtable(Lookup* lookup);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;

  struct Ligature {
    std::uint16_t ligatureGlyph;
    std::vector<std::uint16_t> componentGlyphIDs;
  };

  std::vector<Ligature> ligatures;

  std::uint16_t format = 1;
};

struct SingleAdjustmentSubtable : Subtable {
  SingleAdjustmentSubtable(Lookup* lookup, std::uint16_t format = 2);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;
  void saveParameters(QJsonObject& json) const override;
  void readParameters(const QJsonObject& json) override;

  std::map<std::uint16_t, ValueRecord> singlePos;
  std::map<std::uint16_t, ValueRecord> parameters;

  bool isExtended() override;

  std::uint16_t format;
};
struct hb_cursive_anchor_context_t;

struct PairAdjustmentSubtable : Subtable {
  struct PairValue {
    std::variant<ValueRecord, PairAdjustFunc> valueRecord1;
    std::variant<ValueRecord, PairAdjustFunc> valueRecord2;
  };
  struct PairValueFinal {
    ValueRecord valueRecord1;
    ValueRecord valueRecord2;
  };
  PairAdjustmentSubtable(Lookup* lookup, std::uint16_t format = 1);
  QByteArray getOpenTypeTable(bool extended) override;
  // void saveParameters(QJsonObject& json) const override;
  // void readParameters(const QJsonObject& json) override;

  std::map<std::uint16_t, std::map<std::uint16_t, PairValue>> pairPos;
  std::map<std::uint16_t, std::map<std::uint16_t, PairValue>> parameters;

  std::uint16_t format;
  void getPairValue(hb_cursive_anchor_context_t* context);

 private:
  std::uint16_t valueFormat1 = 0;
  std::uint16_t valueFormat2 = 0;
};

struct CursiveSubtable : Subtable {
  struct EntryExit {
    std::optional<QPoint> entry;
    CursiveAnchorFunc entryFunction;
    std::string entryName;
    std::optional<QPoint> exit;
    CursiveAnchorFunc exitFunction;
    std::string exitName;
  };
  CursiveSubtable(Lookup* lookup) : Subtable{lookup} {}
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;
  void readParameters(const QJsonObject& json) override;
  void saveParameters(QJsonObject& json) const override;

  std::map<std::uint16_t, EntryExit> anchors;

  std::map<std::uint16_t, QPoint> exitParameters;
  std::map<std::uint16_t, QPoint> entryParameters;

  virtual std::optional<QPoint> getEntry(std::uint16_t glyph_id, GlyphParameters parameters);

  virtual QPoint calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, QPoint entry);

  virtual std::optional<QPoint> getExit(std::uint16_t glyph_id, GlyphParameters parameters);

 private:
  void setAnchorTable(std::uint16_t glyphCode,
                      QByteArray& entryExitRecords,
                      QByteArray& anchorTables,
                      quint32& anchorOffset,
                      std::map<int, std::pair<int, std::pair<int, int>>>& posToVar,
                      bool extended,
                      bool isEntry);
};

struct MarkBaseSubtable : Subtable {
  struct MarkClass {
    std::unordered_set<std::string> mark;
    std::unordered_set<std::uint16_t> markCodes;
    CalcAnchor basefunction;
    CalcAnchor markfunction;
    std::map<std::string, QPoint> baseparameters;
    std::map<std::string, QPoint> markparameters;
    std::map<std::string, QPoint> baseanchors;
    std::map<std::string, QPoint> markanchors;
  };
  MarkBaseSubtable(Lookup* lookup);

  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;
  void saveParameters(QJsonObject& json) const override;
  void readParameters(const QJsonObject& json) override;

  std::vector<std::string> base;
  std::map<std::string, MarkClass> classes;

  std::vector<std::uint16_t> sortedBaseCodes;

  std::vector<std::uint16_t> baseCoverage;
  std::map<std::uint16_t, std::uint16_t> markCoverage;

  //  Computed during getOpenTypeTable
  std::map<std::uint16_t, std::uint16_t> markCodes;
  std::map<std::uint16_t, std::string> classNamebyIndex;

  virtual std::optional<QPoint> getBaseAnchor(std::uint16_t mark_id, std::uint16_t base_id, GlyphParameters parameters);
  virtual QPoint getBaseAnchor(std::string baseGlyphName, std::string className, GlyphParameters parameters);
  virtual std::optional<QPoint> getMarkAnchor(std::uint16_t mark_id, std::uint16_t base_id, GlyphParameters parameters);
  QPoint getMarkAnchor(std::string markGlyphName, std::string className, GlyphParameters parameters);

 private:
  void setAnchorTable(std::string className,
                      std::uint16_t glyphCode,
                      QByteArray& anchorTables,
                      quint32& anchorOffset,
                      std::map<int, std::pair<int, std::pair<int, int>>>& posToVar,
                      bool extended,
                      bool isBase);
};

struct ChainingSubtable : Subtable {
  struct LookupRecord {
    std::uint16_t position;
    std::string lookupName;
  };
  struct Rule {
    std::vector<std::unordered_set<std::string>> backtrack;
    std::vector<std::unordered_set<std::string>> lookahead;
    std::vector<std::unordered_set<std::string>> input;
    std::vector<LookupRecord> lookupRecords;
  };

  struct CompiledRule {
    std::vector<std::unordered_set<std::uint16_t>> backtrack;
    std::vector<std::unordered_set<std::uint16_t>> lookahead;
    std::vector<std::unordered_set<std::uint16_t>> input;
    std::vector<LookupRecord> lookupRecords;
  };

  ChainingSubtable(Lookup* lookup);
  QByteArray getOpenTypeTable(bool extended) override;
  void readJson(const QJsonObject& json) override;

  Rule rule;

  // Compiled
  CompiledRule compiledRule;
};

enum class DFAActionType {
  LOOKUP,
  ACTION,
  STARTNEWMATCH
};

struct DFAAction {
  DFAActionType type;
  std::string name;
  int idRule;
};

struct DFABackTrackInfo {
  int prevTransIndex;
  std::vector<DFAAction> actions;
};

inline bool operator==(const DFAAction& lhs, const DFAAction& rhs) {
  return lhs.type == rhs.type && lhs.name == rhs.name;  //&& lhs.idRule == rhs.idRule;
}

inline bool operator!=(const DFAAction& lhs, const DFAAction& rhs) {
  return lhs.type != rhs.type || lhs.name != rhs.name;  //&& lhs.idRule == rhs.idRule;
}

inline bool operator==(const DFABackTrackInfo& lhs, const DFABackTrackInfo& rhs) {
  return lhs.prevTransIndex == rhs.prevTransIndex && lhs.actions == rhs.actions;
}

inline bool operator!=(const DFABackTrackInfo& lhs, const DFABackTrackInfo& rhs) {
  return lhs.prevTransIndex != rhs.prevTransIndex || lhs.actions != rhs.actions;
}

inline bool operator<(const DFAAction& lhs, const DFAAction& rhs) {
  /*if (lhs.idRule != rhs.idRule)
    return lhs.idRule < rhs.idRule;
  else*/
  if (lhs.type != rhs.type)
    return lhs.type < rhs.type;
  else
    return lhs.name < rhs.name;
}

inline bool operator<(const DFABackTrackInfo& lhs, const DFABackTrackInfo& rhs) {
  if (lhs.prevTransIndex != rhs.prevTransIndex)
    return lhs.prevTransIndex < rhs.prevTransIndex;
  else
    return lhs.actions < rhs.actions;
}

struct DFATransOut {
  int state;
  std::vector<DFABackTrackInfo> backtracks;
};

struct DFASTate {
  std::map<int, DFATransOut> transtitions;
  int final = 0;
  DFABackTrackInfo backtrackfinal;
};

class DFA {
 public:
  int minBackup = 0;
  int maxBackup = 0;
  int maxLoop = 10;
  std::vector<int> backupStates;
  std::vector<DFASTate> states;
  std::vector<std::unordered_set<std::uint16_t>> eqClasses;
  std::map<std::uint16_t, std::uint16_t> glyphToClass;
};

struct FSMSubtable : Subtable {
 public:
  FSMSubtable(Lookup* lookup) : Subtable{lookup} {}

  QByteArray getOpenTypeTable(bool extended) override;

  bool isExtended() override { return false; }

  DFA dfa;
};
