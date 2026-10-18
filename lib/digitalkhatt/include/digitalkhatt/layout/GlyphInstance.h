#pragma once

#include <optional>
#include <string>

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
  enum class InitialPlacement { None, CenteredGap, LeftOfRehWaw, BelowRehWaw };
  double baseX = 0.0;
  double baseY = 0.0;
  double lineY = 0.0;
  double dx = 0.0;
  double dy = 0.0;
  // The initial positioning contribution is included in dx/dy. Ordinary
  // constraints and audits use this adjusted input as the authored baseline.
  double initialPositioningDx = 0.0;
  double initialPositioningDy = 0.0;
  InitialPlacement initialPlacement = InitialPlacement::None;

  // Observe every applied projection, including corrections that cancel later
  // in the same iteration. A net displacement alone can hide conflicting forces.
  double lastBuiltDx = 0.0;
  double lastBuiltDy = 0.0;
  double iterationMaxMovementSquared = 0.0;

  const geometry::GeometrySet* geom = nullptr;

  geometry::GeometrySet geomScaled;

  bool isMark = false;

  bool isTopMark = false;
  // Authored horizontal GPOS placement for a deformed mark is a stack target,
  // not a request to recenter its enlarged bounding box over a dot.
  bool preserveGposHorizontal = false;
  // Font-authored lowest-ink floor, scaled to world units by the placement adapter.
  std::optional<double> baselineClearance;
  double justificationAddedLeftTatweel = 0;

  geometry::GeometrySet worldPolys;  // rebuilt after an offset changes

  GlyphLayoutInfo* glyphLayout = nullptr;
  GlyphMetrics metrics;
  std::string glyphName;
  int lineIndex = 0;
  int glyphIndex = 0;
  int globalIndex = 0;
  // Page-local semantic word identity, assigned from spaces and line boundaries
  // before solving. -1 denotes whitespace or unknown ownership.
  int wordIndex = -1;
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
// Convert page-space solver deltas back through the complete line transform.
void applySolvedGlyphOffsets(GlyphLayoutInfo& layout, const GlyphInstance& g, double horizontalScale, double verticalScale = 1.0);

}  // namespace digitalkhatt::layout
