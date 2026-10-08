#pragma once

#include <string>
#include <optional>

#include "digitalkhatt/core/digitalkahtt_types.h"
#include "digitalkhatt/geometry/geometry.h"

namespace digitalkhatt::layout {

// The subset of GlyphVis (width/height/bbox) the solver actually reads.
struct GlyphMetrics {
  double width = 0.0;
  double height = 0.0;
  double bboxLlx = 0.0;
  double bboxUrx = 0.0;
};

struct GlyphInstance {
  double baseX = 0.0;
  double baseY = 0.0;
  double lineY = 0.0;
  double dx = 0.0;
  double dy = 0.0;

  // Observe every applied projection, including corrections that cancel later
  // in the same iteration. A net displacement alone can hide conflicting forces.
  double lastBuiltDx = 0.0;
  double lastBuiltDy = 0.0;
  double iterationMaxMovementSquared = 0.0;

  const geometry::GeometrySet* geom = nullptr;

  geometry::GeometrySet geomScaled;

  // Optional unsplit outline, scaled exactly like geomScaled. NFP uses this
  // independently of the hull/decomposed geometry used by other constraints.
  std::optional<geometry::GeometrySet> noFitGeometry;
  const geometry::GeometrySet& noFitLocalGeometry() const {
    return noFitGeometry ? *noFitGeometry : (geom ? *geom : geomScaled);
  }

  std::size_t noFitShape = 0; // registered scaled-local shape for the optional oracle
  bool noFitConvexContact = false;

  bool isMark = false;

  bool isTopMark = false;

  geometry::GeometrySet worldPolys;  // rebuilt after an offset changes

  GlyphLayoutInfo* glyphLayout = nullptr;
  GlyphMetrics metrics;
  std::string glyphName;
  int lineIndex = 0;
  int glyphIndex = 0;
  int globalIndex = 0;
  GlyphInstance* prevBase = nullptr;
  GlyphInstance* nextBase = nullptr;
  double mobility = 0.0;  // 0 = rigid, 1 = very movable
  double attachCompliance = 1.0;

  // XPBD state (persistent across iterations!)
  double attachLambda = 0.0;
  double ownLambdaLeft = 0.0;
  double ownLambdaRight = 0.0;
  double ownLambdaVert = 0.0;
};

void buildWorldPolys(GlyphInstance& g);
// During constraint projection, base position and local geometry stay fixed.
// Initial preparation still calls buildWorldPolys unconditionally.
inline void updateWorldPolys(GlyphInstance& g) {
  if (g.dx != g.lastBuiltDx || g.dy != g.lastBuiltDy) buildWorldPolys(g);
}
// Solver deltas are in world space; rendered x offsets are scaled by line.xscale.
void applySolvedGlyphOffsets(GlyphLayoutInfo& layout, const GlyphInstance& g, double horizontalScale);

}  // namespace digitalkhatt::layout
