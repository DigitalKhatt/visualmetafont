#pragma once

#include <vector>
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

struct TopMarkStackLink {
  GlyphInstance* inner;
  GlyphInstance* outer;
  double offsetX;
  double minGap;
};

// Apply authored rules only to marks of the same base with consecutive original
// glyph indices on the same line. Every intervening glyph blocks the match,
// regardless of class membership. Positions never determine semantic adjacency.
std::vector<TopMarkStackLink> buildTopMarkStackLinks(
    const std::vector<GlyphInstance*>& group);

}  // namespace digitalkhatt::layout
