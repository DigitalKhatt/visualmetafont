#pragma once

#include <QPainterPath>
#include <QPicture>

#include "GlyphVis.h"
#include "glyph.hpp"

namespace digitalkhatt::qt {

inline QPainterPath pathForGlyph(const GlyphVis& glyph) {
  mp_edge_object edge{};
  edge.body = glyph.mpPath();
  return Glyph::getPath(&edge);
}

inline QPicture pictureForGlyph(const GlyphVis& glyph) {
  mp_edge_object edge{};
  edge.body = glyph.mpPath();
  return Glyph::getPicture(&edge);
}

}  // namespace digitalkhatt::qt
