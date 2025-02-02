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
#include "font.hpp"
#include "glyph.hpp"
#ifndef DIGITALKHATT_WEBLIB
#include "qpainterpath.h"
#include "qpainter.h"
#include "qcolor.h"
#endif

#include "automedina/automedina.h"
#include <cmath>
#include "metafont.h";


GlyphVis::GlyphVis() {
  m_edge = nullptr;
  m_otLayout = nullptr;
  copiedPath = nullptr;
  isCopiedPath = false;
}

GlyphVis::~GlyphVis()
{
  if (copiedPath && isCopiedPath) {
    mp_graphic_object* p, * q;

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
  width = other.width;
  height = other.height;
  depth = other.depth;
  charlt = other.charlt;
  charrt = other.charrt;
  bbox = other.bbox;
  leftAnchor = other.leftAnchor;
  rightAnchor = other.rightAnchor;
#ifndef DIGITALKHATT_WEBLIB
  path = other.path;
  picture = other.picture;
#endif
  anchors = other.anchors;
  matrix = other.matrix;


  isdirty = other.isdirty;
  m_edge = other.m_edge;
  m_otLayout = other.m_otLayout;

  copiedPath = other.copiedPath;
  isCopiedPath = other.isCopiedPath;

  if (other.copiedPath && isCopiedPath) {
    copiedPath = m_otLayout->font->copyEdgeBody(other.copiedPath);
  }

  expanded = other.expanded;

  isAlternate = other.isAlternate;
}

bool GlyphVis::isColored() {
  return glyphtype == GlyphType::GlyphTypeColored;
}

GlyphVis* GlyphVis::getColoredGlyph() {

  GlyphVis* coloredGlyph = nullptr;

  if (!coloredglyph.isEmpty()) {
    if (m_otLayout->glyphs.contains(coloredglyph)) {
      coloredGlyph = &m_otLayout->glyphs[coloredglyph];
    }
  }

  return coloredGlyph;
}

GlyphType GlyphVis::getGlypfType() {
  if (m_edge != nullptr) {
    return (GlyphType)m_edge->glyphtype;
  }
  return GlyphType::Unknown;
}

GlyphVis* GlyphVis::getAlternate(GlyphParameters parameters) {
  if (parameters.lefttatweel != 0.0 || parameters.righttatweel != 0.0) {
    return m_otLayout->getAlternate(charcode, parameters);
  }
  else {
    return this;
  }
}

GlyphVis::GlyphVis(GlyphVis&& other)
{

  name = other.name;
  originalglyph = other.originalglyph;
  coloredglyph = other.coloredglyph;
  glyphtype = other.glyphtype;
  charcode = other.charcode;
  width = other.width;
  height = other.height;
  depth = other.depth;
  charlt = other.charlt;
  charrt = other.charrt;
  bbox = other.bbox;
  leftAnchor = other.leftAnchor;
  rightAnchor = other.rightAnchor;
#ifndef DIGITALKHATT_WEBLIB
  path = other.path;
  picture = other.picture;
#endif
  anchors = other.anchors;
  matrix = other.matrix;

  copiedPath = other.copiedPath;
  isCopiedPath = other.isCopiedPath;

  isdirty = other.isdirty;
  m_edge = other.m_edge;
  m_otLayout = other.m_otLayout;

  other.copiedPath = nullptr;
#ifndef DIGITALKHATT_WEBLIB
  other.path = {};
#endif
  expanded = other.expanded;
  isAlternate = other.isAlternate;
}

GlyphVis& GlyphVis::operator=(const GlyphVis& other) {
  if (this == &other) return *this;

  name = other.name;
  originalglyph = other.originalglyph;
  coloredglyph = other.coloredglyph;
  glyphtype = other.glyphtype;
  charcode = other.charcode;
  width = other.width;
  height = other.height;
  depth = other.depth;
  charlt = other.charlt;
  charrt = other.charrt;
  bbox = other.bbox;
  leftAnchor = other.leftAnchor;
  rightAnchor = other.rightAnchor;
#ifndef DIGITALKHATT_WEBLIB
  path = other.path;
  picture = other.picture;
#endif
  anchors = other.anchors;
  matrix = other.matrix;


  isdirty = other.isdirty;
  m_edge = other.m_edge;
  m_otLayout = other.m_otLayout;

  copiedPath = other.copiedPath;
  isCopiedPath = other.isCopiedPath;

  if (other.copiedPath && isCopiedPath) {
    copiedPath = m_otLayout->font->copyEdgeBody(other.copiedPath);
  }

  expanded = other.expanded;
  isAlternate = other.isAlternate;

  return *this;
}

bool GlyphVis::isAyaNumber()
{
  return (charcode >= Automedina::AyaNumberCode && charcode <= Automedina::AyaNumberCode + 286);
}
GlyphVis::GlyphVis(OtLayout* otLayout, mp_edge_object* edge, bool copyPath) {
  m_edge = edge;
  m_otLayout = otLayout;

  this->name = m_edge->charname;
  if (m_edge->originalglyph != "" && this->name != m_edge->originalglyph)
    originalglyph = m_edge->originalglyph;

  charcode = m_edge->charcode;
  width = m_edge->width;
  height = m_edge->height;
  depth = m_edge->depth;
  charlt = m_edge->charlt;
  charrt = m_edge->charrt;
  if (m_edge->coloredglyph) {
    coloredglyph = QString(m_edge->coloredglyph);
  }
  glyphtype = (GlyphType)m_edge->glyphtype;

  if (edge->body == nullptr) {
    bbox.llx = 0;
    bbox.lly = 0;
    bbox.urx = 0;
    bbox.ury = 0;
  }
  else {
    bbox.llx = m_edge->minx;
    bbox.lly = m_edge->miny;
    bbox.urx = m_edge->maxx;
    bbox.ury = m_edge->maxy;
  }
  double intpart;
  if (!std::isnan(m_edge->xleftanchor)) {
    if (std::modf(m_edge->xleftanchor, &intpart) != 0.0 || std::modf(m_edge->yleftanchor, &intpart) != 0.0) {
      int stop = 5;
    }
    leftAnchor = QPoint(round(m_edge->xleftanchor), round(m_edge->yleftanchor));
  }
  if (!std::isnan(m_edge->xrightanchor)) {
    //if (!isdigit(m_edge->xrightanchor) || !isdigit(m_edge->xrightanchor)) {
    if (std::modf(m_edge->xrightanchor, &intpart) != 0.0 || std::modf(m_edge->yrightanchor, &intpart) != 0.0) {
      int stop = 5;
    }
    rightAnchor = QPoint(round(m_edge->xrightanchor), round(m_edge->yrightanchor));
  }

  //matrix = getMatrix(m_otLayout->mp, charcode);
  matrix = { m_edge->xpart,m_edge->ypart };
#ifndef DIGITALKHATT_WEBLIB
  if (!isColored()) {
    path = Glyph::getPath(m_edge);
  }
  else {
    picture = Glyph::getPicture(m_edge);
    path = Glyph::getPath(m_edge);
  }
#endif
  auto body = m_edge != nullptr ? m_edge->body : nullptr;
  if (copyPath) {
    isCopiedPath = true;
    copiedPath = m_otLayout->font->copyEdgeBody(body);
  }
  else {
    isCopiedPath = false;
    this->copiedPath = body;
  }


  for (int i = 0; i < m_edge->numAnchors; i++) {
    AnchorPoint anchor = m_edge->anchors[i];
    //auto type = anchor.type == (int)AnchorType::EntryAnchorRTL ? AnchorType::EntryAnchor : (anchor.type == (int)AnchorType::ExitAnchorRTL ? AnchorType::ExitAnchor : (AnchorType)anchor.type);
    anchors.insert({ anchor.anchorName,(AnchorType)anchor.type }, { QPoint(anchor.x, anchor.y), anchor.type });
  }


}

bool GlyphVis::conatinsAnchor(QString name, AnchorType type) {
  return anchors.contains({ name,type });
}

QPoint GlyphVis::getAnchor(QString name, AnchorType type) {
  return anchors.value({ name,type }).anchor;
}
