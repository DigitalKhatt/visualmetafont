#pragma once

#include <string_view>
#include <vector>

#include "digitalkhatt/geometry/geometry.h"

namespace digitalkhatt::layout {

// World geometry and source order used by both Save Collision and the native
// report. These outlines are decomposed even for marks; XPBD keeps its hulls.
struct CollisionGlyph {
  const geometry::GeometrySet* geometry;
  std::string_view name;
  int line;
  int word;
  bool isMark;
  double fontSize;
};

struct CollisionContact {
  std::size_t first;
  std::size_t second;
  geometry::Contact contact;
  double minimumGap;
};

bool isCollisionSpace(std::string_view name);
std::vector<CollisionContact> findCollisionContacts(
    const std::vector<CollisionGlyph>& glyphs, double minimumGap);

} // namespace digitalkhatt::layout
