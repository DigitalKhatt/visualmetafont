#pragma once

#include "digitalkhatt/geometry/geometry.h"

namespace digitalkhatt::layout {

inline geometry::GeometrySet buildGlyphCollisionGeometry(const geometry::GlyphCubic& outline,
                                                         bool isMark) {
  // Keep each mark component unsplit. GJK's farthest-vertex support mapping
  // implicitly tests its hull, giving the mark one coherent collision proxy.
  // Bases retain decomposed parts so marks can occupy their concave regions.
  return isMark ? geometry::buildPolyFromCubics(outline, geometry::CUBIC_FLATNESS_TOLERANCE)
                : geometry::buildConvexPartsFromCubics(outline, geometry::CUBIC_FLATNESS_TOLERANCE);
}

}  // namespace digitalkhatt::layout
