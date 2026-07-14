#include "digitalkhatt/layout/constraints/HardStayBelowConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void HardStayBelowConstraint::project(SolverContext& solverContext,
                                      double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  // Two references, each measured against a different point on the mark:
  // base-bottom arm uses the mark's TOP (whole mark must clear it), baseline
  // arm uses the point belowBaselineFraction up from the mark's BOTTOM (only
  // that fraction of the bbox need clear it) -- so each arm's residual is
  // computed separately rather than reduced to a single yMax.
  const double markTop = boxTopY(mark);
  const double baselineRefPoint = boxBottomY(mark) + belowBaselineFraction * boxHeight(mark);

  const double C_baseBottom = markTop - (boxBottomY(*base) - maxGapToBase);
  const double C_baseline = baselineRefPoint - (base->lineY + maxDistToBaseline);

  double C;
  if (testBothReferences) {
    // OR semantics: satisfied if the mark clears EITHER bound, i.e. the
    // loosest (smallest) residual wins. Both arms share grad dC/dy = +1, so
    // taking the min is valid here.
    C = std::min(C_baseBottom, C_baseline);
  } else if (useBaseBottom) {
    C = C_baseBottom;
  } else {
    C = C_baseline;
  }

  if (C <= 0.0) return;  // already satisfied

  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-12) return;

  double deltaLambda = -(C + alpha * lambda) / denom;

  // inequality with feasible C <= 0 => lambda <= 0
  double lambdaNew = std::min(0.0, lambda + deltaLambda);
  double applied = lambdaNew - lambda;
  lambda = lambdaNew;

  // grad dC/dy = +1 → move downward
  mark.dy += w * applied;
  buildWorldPolys(mark);
}

void HardStayBelowConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  // Mirrors project(): each arm has its own residual (different mark point)
  // and its own bound (for the marker line), selected the same way.
  const double markTop = boxTopY(mark);
  const double baselineRefPoint = boxBottomY(mark) + belowBaselineFraction * boxHeight(mark);

  const double baseBottomBound = boxBottomY(*base) - maxGapToBase;
  const double baselineBound = base->lineY + maxDistToBaseline;

  const double C_baseBottom = markTop - baseBottomBound;
  const double C_baseline = baselineRefPoint - baselineBound;

  double C;
  double activeBound;
  if (testBothReferences) {
    // OR semantics: report whichever arm is least violated (the one that
    // would resolve first), matching project()'s min-residual selection.
    if (C_baseBottom <= C_baseline) {
      C = C_baseBottom;
      activeBound = baseBottomBound;
    } else {
      C = C_baseline;
      activeBound = baselineBound;
    }
  } else if (useBaseBottom) {
    C = C_baseBottom;
    activeBound = baseBottomBound;
  } else {
    C = C_baseline;
    activeBound = baselineBound;
  }

  if (C <= 0.0) return;  // violation when C > 0

  const auto mb = mark.worldPolys.boundingAABB();
  ConstraintViolation v;
  v.type = ViolationType::HardStayBelow;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = C;
  v.glyphA = mark.globalIndex;
  v.glyphB = base->globalIndex;
  v.markerCount = 2;  // the ceiling rail as a segment at y = activeBound
  v.marker[0] = {mb.minx, activeBound};
  v.marker[1] = {mb.maxx, activeBound};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
