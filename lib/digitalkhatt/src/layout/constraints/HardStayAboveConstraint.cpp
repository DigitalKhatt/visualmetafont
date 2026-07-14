#include "digitalkhatt/layout/constraints/HardStayAboveConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void HardStayAboveConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  // The two candidate lower bounds (mark bottom must stay >= yMin).
  const double baseTopBound = boxTopY(*base) + minGapToBase;
  const double baselineBound = base->lineY + minDistFromBaseline;

  double yMin;
  if (testBothReferences) {
    // OR semantics: satisfied if the mark clears EITHER bound, i.e. the
    // loosest (lowest) lower bound wins.
    yMin = std::min(baseTopBound, baselineBound);
  } else if (useBaseTop) {
    yMin = baseTopBound;
  } else {
    yMin = baselineBound;
  }

  // Constraint: yMin - markBottom <= 0
  const double C = yMin - boxBottomY(mark);
  if (C <= 0.0) return;  // hard feasibility: inactive => do nothing

  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-12) return;

  double deltaLambda = -(C + alpha * lambda) / denom;

  // inequality with feasible C <= 0 => lambda <= 0
  double lambdaNew = std::min(0.0, lambda + deltaLambda);
  double applied = lambdaNew - lambda;
  lambda = lambdaNew;

  // grad dC/dy = -1
  mark.dy += (-1.0) * w * applied;
  buildWorldPolys(mark);
}

void HardStayAboveConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  const double baseTopBound = boxTopY(*base) + minGapToBase;
  const double baselineBound = base->lineY + minDistFromBaseline;
  const double yMin = testBothReferences ? std::min(baseTopBound, baselineBound)
                      : useBaseTop       ? baseTopBound
                                         : baselineBound;

  const double C = yMin - boxBottomY(mark);  // violation when C > 0
  if (C <= 0.0) return;

  const auto mb = mark.worldPolys.boundingAABB();
  ConstraintViolation v;
  v.type = ViolationType::HardStayAbove;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = C;
  v.glyphA = mark.globalIndex;
  v.glyphB = base->globalIndex;
  v.markerCount = 2;  // the floor rail as a segment at y = yMin under the mark
  v.marker[0] = {mb.minx, yMin};
  v.marker[1] = {mb.maxx, yMin};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
