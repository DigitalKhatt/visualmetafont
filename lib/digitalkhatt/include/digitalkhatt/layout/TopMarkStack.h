#pragma once

#include <optional>
#include <vector>
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

struct TopMarkStackLink {
  GlyphInstance* inner;
  GlyphInstance* outer;
  double offsetX;
  double minGap;
  bool outlineClearance = false;
  std::optional<double> preferredGap;
  bool compactAttachment = false;
  bool preserveAuthoredGap = false;
};

// Apply authored rules within a base's group, optionally including that base,
// to consecutive original glyph indices on the same line. Every intervening glyph blocks the match,
// regardless of class membership. Positions never determine semantic adjacency.
std::vector<TopMarkStackLink> buildTopMarkStackLinks(
    const std::vector<GlyphInstance*>& group);

}  // namespace digitalkhatt::layout
