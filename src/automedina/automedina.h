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
#include <string>
#include <unordered_set>
#include <vector>

#include "OtLayout.h"
#include "digitalkhatt/layout/ClassMap.h"
#include "qhash.h"
#include "qmap.h"
#include "qpoint.h"
#include "qset.h"

class LayoutWindow;

class Automedina {
  friend class OtLayout;
  friend class AnchorCalc;
  friend class LayoutWindow;
  friend class GlyphVis;
  friend class ToOpenType;
  friend class GenerateLayout;

 public:
  // static const uint16_t AyaNumberCode = 65200;
  static const uint16_t AyaNumberCode = 0xE000;
  const int markheigh = 500;
  const int markdepth = 80;
  const int spacemkmk = 100;
  const int spacebasetotopmark = 100;
  const int shaddamarkheight = markheigh - 250;
  const int spacebasetobottommark = 50;
  const int minwaqfhigh = 900;

 public:
  Automedina(OtLayout* layout, Font* font, bool extended) : glyphs{layout->glyphs}, m_layout{layout}, font{font}, extended{extended} {}

  std::unordered_set<std::uint16_t> classtoUnicode(const std::string& exprName, bool includeExpandables);
  std::unordered_set<std::uint16_t> classtoUnicode(const std::string& exprName) {
    return classtoUnicode(exprName, true);
  };
  std::unordered_set<std::uint16_t> regexptoUnicode(const std::string& regexp);
  QSet<QString> classtoGlyphName(QString className);
  std::unordered_map<std::string, GlyphVis>& glyphs;
  virtual ~Automedina();

  virtual Lookup* getLookup(std::string lookupName) = 0;
  virtual CalcAnchor getanchorCalcFunctions(std::string functionName, Subtable* subtable) = 0;
  virtual CursiveAnchorFunc getCursiveFunctions(std::string functionName, Subtable* subtable) {
    CursiveAnchorFunc func;
    return func;
  }
  virtual PairAdjustFunc getPairAdjustFunction(std::string functionName, Subtable* subtable) {
    PairAdjustFunc func;
    return func;
  }
  virtual void generateSubstEquivGlyphs() {}

  std::map<std::string, std::string> addedGlyphs;

 protected:
  OtLayout* m_layout;

  digitalkhatt::layout::ClassMap classes;

  std::map<std::string, std::unordered_set<std::uint16_t>> cachedClasstoUnicode;
  // QMap<QString, AnchorCalc*> anchorCalcFunctions;

  std::unordered_set<std::string> initchar;
  std::unordered_set<std::string> medichar;

  Font* font;

  std::map<std::string, std::map<uint16_t, QPoint>> markAnchors;
  std::map<std::string, std::map<uint16_t, QPoint>> entryAnchors;
  std::map<std::string, std::map<uint16_t, QPoint>> exitAnchors;
  std::map<std::string, std::map<uint16_t, QPoint>> entryAnchorsRTL;
  std::map<std::string, std::map<uint16_t, QPoint>> exitAnchorsRTL;

  bool extended;

  std::vector<std::map<uint16_t, std::vector<ExtendedGlyph>>> cvxxfeatures;

  void generateAyas(QString ayaName, bool colored);
};
