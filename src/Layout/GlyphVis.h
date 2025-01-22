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

#include "font.hpp"
#include "qstring.h"
#ifndef DIGITALKHATT_WEBLIB
#include "qpicture.h"
#include "qpainterpath.h"
#include "qpoint.h"
#endif
#include "qmap.h"
#include <unordered_map>
#include "OtLayout.h"

extern "C"
{
#include "AddedFiles/newmp.h"
}


class OtLayout;
struct mp_edge_object;
typedef struct mp_gr_knot_data* mp_gr_knot;
class QPainterPath;
struct mp_graphic_object;

struct GlyphVisAnchor {
  QPoint anchor;
  int type;
};

enum class  GlyphType {
  Unknown = 0,
  GlyphTypeBase = 1,
  GlyphTypeLigature = 2,
  GlyphTypeMark = 3,
  GlyphTypeComponent = 4,
  GlyphTypeColored = 5,
  GlyphTypeTemp = 6,
};

class  GlyphVis {
  friend class MyQPdfEnginePrivate;
  friend class ExportToHTML;
public:
  struct BBox {
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;
  };


  GlyphVis(OtLayout* otLayout, mp_edge_object* edge, bool copyPath = false);
  GlyphVis();


  bool isAyaNumber();

  ~GlyphVis();
  GlyphVis(GlyphVis&& other);
  GlyphVis(const GlyphVis& other);
  GlyphVis& operator=(const GlyphVis& other);

  QString name;
  QString originalglyph;
  QString coloredglyph;
  GlyphType glyphtype;
  int charcode = 0;
  double width = 0;
  double height = 0;
  double depth = 0;
  double charlt = 0;
  double charrt = 0;
  BBox bbox;
  std::optional<QPoint> leftAnchor;
  std::optional<QPoint> rightAnchor;
  mp_graphic_object* copiedPath = nullptr;

#ifndef DIGITALKHATT_WEBLIB
  QPainterPath path;
  QPicture picture;  
#endif

  enum class  AnchorType {
    MarkAnchor = 1,
    EntryAnchor = 2,
    ExitAnchor = 3,
    EntryAnchorRTL = 4,
    ExitAnchorRTL = 5,
    Anchor = 6,
  };
  struct AnchorKey {
    QString name;
    AnchorType type;
    bool operator<(const AnchorKey& other) const {
      return std::tie(name, type) < std::tie(other.name, other.type);
    }
  };
  QMap<AnchorKey, GlyphVisAnchor> anchors;
  Transform matrix = {};

  mp_edge_object* edge() {
    return m_edge;
  }

  GlyphType getGlypfType();

  bool isColored();

  GlyphVis* getColoredGlyph();

  GlyphVis* getAlternate(GlyphParameters parameters);

  QPoint getAnchor(QString name, AnchorType type);

  bool conatinsAnchor(QString name, AnchorType type);
  bool expanded = false;
  bool isAlternate = false;
  

private:

  bool isdirty = true;
  mp_edge_object* m_edge = nullptr;
  OtLayout* m_otLayout = nullptr;
  bool isCopiedPath = false;






};

