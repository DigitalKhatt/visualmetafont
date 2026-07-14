#pragma once

#include <algorithm>
#include <cmath>

#include "digitalkhatt/geometry/geometry.h"
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

inline double boxTopY(const GlyphInstance& g) {
  return g.worldPolys.boundingAABB().maxy;
}

inline double boxBottomY(const GlyphInstance& g) {
  return g.worldPolys.boundingAABB().miny;
}

inline double boxCenterX(const GlyphInstance& g) {
  auto box = g.worldPolys.boundingAABB();
  return 0.5 * (box.minx + box.maxx);
}

inline double boxCenterY(const GlyphInstance& g) {
  auto box = g.worldPolys.boundingAABB();
  return 0.5 * (box.miny + box.maxy);
}

inline double boxHeight(const GlyphInstance& g) {
  auto box = g.worldPolys.boundingAABB();
  return box.maxy - box.miny;
}

// True when `mark`'s world-bbox center lies inside `base`'s world bbox, i.e.
// the base encloses the mark (as a bowl-shaped final/isolated Jeem/Hah/Khah
// encloses a below-mark). A normal below-mark hangs *below* the base ink, so
// its center is under base.miny and this returns false -- which is exactly how
// bowl-enclosed marks are distinguished from ordinary below-marks.
inline bool markEnclosedByBase(const GlyphInstance& mark, const GlyphInstance& base) {
  const auto bb = base.worldPolys.boundingAABB();
  const double cx = boxCenterX(mark);
  const double cy = boxCenterY(mark);
  return cx > bb.minx && cx < bb.maxx && cy > bb.miny && cy < bb.maxy;
}

inline geometry::Vec2 normalizeSafe(const geometry::Vec2& v,
                                    const geometry::Vec2& fallback = {1.0, 0.0}) {
  double L = std::sqrt(v.x * v.x + v.y * v.y);
  if (L < 1e-12) return fallback;
  return {v.x / L, v.y / L};
}

inline geometry::Vec2 applyDirectionalBias(const geometry::Vec2& rawN,
                                           const geometry::Vec2& preferredDir,
                                           double beta) {
  beta = std::clamp(beta, 0.0, 1.0);
  geometry::Vec2 nb{
      (1.0 - beta) * rawN.x + beta * preferredDir.x,
      (1.0 - beta) * rawN.y + beta * preferredDir.y};
  return normalizeSafe(nb, rawN);
}

}  // namespace digitalkhatt::layout
