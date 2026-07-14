#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

void buildWorldPolys(GlyphInstance& g) {
  const double ox = g.baseX + g.dx;
  const double oy = g.baseY + g.dy;

  if (g.geom) {
    g.worldPolys = g.geom->translate(ox, oy);
  } else {
    g.worldPolys = g.geomScaled.translate(ox, oy);
  }
}

}  // namespace digitalkhatt::layout
