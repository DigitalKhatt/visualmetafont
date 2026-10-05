#include "digitalkhatt/layout/GlyphInstance.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace digitalkhatt::layout {

void buildWorldPolys(GlyphInstance& g) {
  const double deltaX = g.dx - g.lastBuiltDx;
  const double deltaY = g.dy - g.lastBuiltDy;
  g.iterationMaxMovementSquared = std::max(g.iterationMaxMovementSquared,
      deltaX * deltaX + deltaY * deltaY);
  g.lastBuiltDx = g.dx;
  g.lastBuiltDy = g.dy;
  const double ox = g.baseX + g.dx;
  const double oy = g.baseY + g.dy;

  const auto& local = g.geom ? *g.geom : g.geomScaled;
  // Cache local bounds once; translated geometry can then reuse those bounds.
  (void)local.boundingAABB();
  g.worldPolys = local.translate(ox, oy);
}

void applySolvedGlyphOffsets(GlyphLayoutInfo& layout, const GlyphInstance& g, double horizontalScale) {
  if (!(horizontalScale > 0.0) || !std::isfinite(horizontalScale) ||
      !std::isfinite(g.dx) || !std::isfinite(g.dy)) return;
  const double x = std::round(layout.x_offset + g.dx / horizontalScale);
  const double y = std::round(layout.y_offset + g.dy);
  const auto fits = [](double value) {
    return std::isfinite(value) && value >= std::numeric_limits<int>::min() &&
        value <= std::numeric_limits<int>::max();
  };
  if (!fits(x) || !fits(y)) return;
  layout.x_offset = static_cast<int>(x);
  layout.y_offset = static_cast<int>(y);
}

}  // namespace digitalkhatt::layout
