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

#include "GlyphVis.h"
#include "OtLayout.h"


#include <cmath>
#include <utility>

#include "automedina/automedina.h"
#include "metafont.h"


GlyphVis::GlyphVis() {
  m_otLayout = nullptr;
  copiedPath = nullptr;
}

GlyphVis::~GlyphVis() {
  if (copiedPath) {
    mp_graphic_object *p, *q;

    p = copiedPath;
    while (p != NULL) {
      q = p->next;
      mp_gr_toss_object(p);
      p = q;
    }
  }
}
GlyphVis::GlyphVis(const GlyphVis& other) {
  name = other.name;
  originalglyph = other.originalglyph;
  coloredglyph = other.coloredglyph;
  glyphtype = other.glyphtype;
  charcode = other.charcode;
  unicode = other.unicode;
  width = other.width;
  height = other.height;
  depth = other.depth;
  charlt = other.charlt;
  charrt = other.charrt;
  bbox = other.bbox;
  leftAnchor = other.leftAnchor;
  rightAnchor = other.rightAnchor;
  anchors = other.anchors;
  matrix = other.matrix;

  m_otLayout = other.m_otLayout;
  if (other.copiedPath) {
    copiedPath = m_otLayout->copyEdgeBody(other.copiedPath);
  } else {
    copiedPath = nullptr;
  }

  expanded = other.expanded;

  isAlternate = other.isAlternate;
}

bool GlyphVis::isColored() {
  return glyphtype == GlyphType::GlyphTypeColored;
}

GlyphVis* GlyphVis::getColoredGlyph() {
  GlyphVis* coloredGlyph = nullptr;

  if (!coloredglyph.empty()) {
    if (m_otLayout->glyphs.contains(coloredglyph)) {
      coloredGlyph = &m_otLayout->glyphs[coloredglyph];
    }
  }

  return coloredGlyph;
}

GlyphType GlyphVis::getGlypfType() {
  return glyphtype;
}

GlyphVis* GlyphVis::getAlternate(GlyphParameters parameters) {
  if (m_otLayout != nullptr && (parameters.lefttatweel != 0.0 || parameters.righttatweel != 0.0 || parameters.scalex != 0)) {
    return m_otLayout->getAlternate(charcode, parameters);
  } else {
    return this;
  }
}

GlyphVis::GlyphVis(GlyphVis&& other) noexcept : GlyphVis() {
  swap(other);
}

GlyphVis& GlyphVis::operator=(GlyphVis other) {
  swap(other);
  return *this;
}

void GlyphVis::swap(GlyphVis& other) noexcept {
  using std::swap;
  swap(name, other.name);
  swap(originalglyph, other.originalglyph);
  swap(coloredglyph, other.coloredglyph);
  swap(glyphtype, other.glyphtype);
  swap(charcode, other.charcode);
  swap(unicode, other.unicode);
  swap(width, other.width);
  swap(height, other.height);
  swap(depth, other.depth);
  swap(charlt, other.charlt);
  swap(charrt, other.charrt);
  swap(bbox, other.bbox);
  swap(leftAnchor, other.leftAnchor);
  swap(rightAnchor, other.rightAnchor);
  swap(copiedPath, other.copiedPath);
  swap(anchors, other.anchors);
  swap(matrix, other.matrix);
  swap(expanded, other.expanded);
  swap(isAlternate, other.isAlternate);
  swap(m_otLayout, other.m_otLayout);
}

bool GlyphVis::isAyaNumber() {
  return (charcode >= Automedina::AyaNumberCode && charcode <= Automedina::AyaNumberCode + 286);
}
GlyphVis::GlyphVis(OtLayout* otLayout, const mp_edge_object* edge) {
  m_otLayout = otLayout;

  this->name = edge->charname;
  if (edge->originalglyph != "" && this->name != edge->originalglyph)
    originalglyph = edge->originalglyph;

  charcode = edge->charcode;
  unicode = edge->unicode;
  width = edge->width;
  height = edge->height;
  depth = edge->depth;
  charlt = edge->charlt;
  charrt = edge->charrt;
  if (edge->coloredglyph) {
    coloredglyph = edge->coloredglyph;
  }
  glyphtype = (GlyphType)edge->glyphtype;

  if (edge->body == nullptr) {
    bbox.llx = 0;
    bbox.lly = 0;
    bbox.urx = 0;
    bbox.ury = 0;
  } else {
    bbox.llx = edge->minx;
    bbox.lly = edge->miny;
    bbox.urx = edge->maxx;
    bbox.ury = edge->maxy;
  }
  double intpart;
  if (!std::isnan(edge->xleftanchor)) {
    if (std::modf(edge->xleftanchor, &intpart) != 0.0 || std::modf(edge->yleftanchor, &intpart) != 0.0) {
      int stop = 5;
    }
    leftAnchor = Point(round(edge->xleftanchor), round(edge->yleftanchor));
  }
  if (!std::isnan(edge->xrightanchor)) {
    if (std::modf(edge->xrightanchor, &intpart) != 0.0 || std::modf(edge->yrightanchor, &intpart) != 0.0) {
      int stop = 5;
    }
    rightAnchor = Point(round(edge->xrightanchor), round(edge->yrightanchor));
  }

  // matrix = getMatrix(m_otLayout->mp, charcode);
  matrix = {edge->xpart, edge->ypart};
  copiedPath = m_otLayout->copyEdgeBody(edge->body);

  for (int i = 0; i < edge->numAnchors; i++) {
    AnchorPoint anchor = edge->anchors[i];
    // auto type = anchor.type == (int)AnchorType::EntryAnchorRTL ? AnchorType::EntryAnchor : (anchor.type == (int)AnchorType::ExitAnchorRTL ? AnchorType::ExitAnchor : (AnchorType)anchor.type);
    anchors.insert_or_assign({anchor.anchorName, (AnchorType)anchor.type},
                             GlyphVisAnchor{Point(anchor.x, anchor.y), anchor.type});
  }
}

bool GlyphVis::conatinsAnchor(const std::string& name, AnchorType type) {
  return anchors.contains({name, type});
}

Point GlyphVis::getAnchor(const std::string& name, AnchorType type) {
  const auto anchor = anchors.find({name, type});
  return anchor != anchors.end() ? anchor->second.anchor : Point{};
}
