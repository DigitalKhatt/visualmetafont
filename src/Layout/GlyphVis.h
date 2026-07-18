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

#include <optional>
#include <map>
#include <string>

#include <unordered_map>

#include "commontypes.h"
#include "metafont.h"

class OtLayout;
// struct mp_edge_object;
// typedef struct mp_gr_knot_data* mp_gr_knot;
// struct mp_graphic_object;

struct GlyphVisAnchor {
  Point anchor;
  int type;
};

enum class GlyphType {
  Unknown = 0,
  GlyphTypeBase = 1,
  GlyphTypeLigature = 2,
  GlyphTypeMark = 3,
  GlyphTypeComponent = 4,
  GlyphTypeColored = 5,
  GlyphTypeTemp = 6,
};

class GlyphVis {
 public:
  struct BBox {
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;
  };

  GlyphVis(OtLayout* otLayout, const mp_edge_object* edge);
  GlyphVis();

  bool isAyaNumber();

  ~GlyphVis();
  GlyphVis(GlyphVis&& other) noexcept;
  GlyphVis(const GlyphVis& other);
  GlyphVis& operator=(GlyphVis other);
  void swap(GlyphVis& other) noexcept;

  std::string name;
  std::string originalglyph;
  std::string coloredglyph;
  GlyphType glyphtype = GlyphType::Unknown;
  int charcode = 0;
  int unicode = -1;
  double width = 0;
  double height = 0;
  double depth = 0;
  double charlt = 0;
  double charrt = 0;
  BBox bbox;
  std::optional<Point> leftAnchor;
  std::optional<Point> rightAnchor;
  mp_graphic_object* copiedPath = nullptr;

  mp_graphic_object* mpPath() const {
    return copiedPath;
  }

  enum class AnchorType {
    MarkAnchor = 1,
    EntryAnchor = 2,
    ExitAnchor = 3,
    EntryAnchorRTL = 4,
    ExitAnchorRTL = 5,
    Anchor = 6,
  };
  struct AnchorKey {
    std::string name;
    AnchorType type;
    bool operator<(const AnchorKey& other) const {
      return std::tie(name, type) < std::tie(other.name, other.type);
    }
  };
  std::map<AnchorKey, GlyphVisAnchor> anchors;
  Transform matrix = {};

  GlyphType getGlypfType();

  bool isColored();

  GlyphVis* getColoredGlyph();

  GlyphVis* getAlternate(GlyphParameters parameters);

  Point getAnchor(const std::string& name, AnchorType type);

  bool conatinsAnchor(const std::string& name, AnchorType type);
  bool expanded = false;
  bool isAlternate = false;

 private:
  OtLayout* m_otLayout = nullptr;
};

inline void swap(GlyphVis& lhs, GlyphVis& rhs) noexcept {
  lhs.swap(rhs);
}
