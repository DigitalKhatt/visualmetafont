#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

using namespace geometry;

namespace digitalkhatt::layout {

// Signed anti-misassociation residual, measured on the ink box.
//
// markB is the left mark (next glyph), markA the right mark (earlier base), so the
// overlap of markB's right ink edge past markA's left ink edge is O = B_R - A_L.
// Each mark must keep an exposed protrusion on its own-base side that clears both a
// self-floor (recognizability) and a cross-floor (not swamped by the neighbor):
//   exposed(A) = wA - O >= max(selfKeep*wA, crossKeep*wB)
//   exposed(B) = wB - O >= max(selfKeep*wB, crossKeep*wA)
// Collapsing to a single allowed overlap T (may be negative -> forces a gap):
//   C = O - T,  want C <= 0.
// The bbox terms are constant w.r.t. dx, so the gradient is unchanged.
static double horizontalOrderResidual(const GlyphInstance& markA,
                                      const GlyphInstance& markB,
                                      double selfKeep, double crossKeep) {
  const double wA = markA.metrics.bboxUrx - markA.metrics.bboxLlx;
  const double wB = markB.metrics.bboxUrx - markB.metrics.bboxLlx;

  const double B_R = markB.baseX + markB.dx + markB.metrics.bboxUrx;
  const double A_L = markA.baseX + markA.dx + markA.metrics.bboxLlx;
  const double O = B_R - A_L;

  const double TA = wA - std::max(selfKeep * wA, crossKeep * wB);
  const double TB = wB - std::max(selfKeep * wB, crossKeep * wA);
  const double T = std::min(TA, TB);

  return O - std::max(T, 0.0);
}

void HorizontalOrderConstraint::project(SolverContext& solverContext, double dt) {
  const double wAinv = markA.mobility;
  const double wBinv = markB.mobility;
  if (wAinv + wBinv == 0.0) return;

  double C = horizontalOrderResidual(markA, markB, selfKeep, crossKeep);

  // Gradients
  Vec2 nA = {-1.0, 0.0};
  Vec2 nB = {1.0, 0.0};

  // Inactive case → decay λ (soft preference)
  if (C <= 0.0) {
    C = 0.0;
    nA = {0.0, 0.0};
    nB = {0.0, 0.0};
  }

  const double alpha = compliance / (dt * dt);
  const double denom = wAinv + wBinv + alpha;
  if (denom < 1e-12) return;

  double deltaLambda = -(C + alpha * lambda) / denom;

  // One-sided clamp: λ ≤ 0
  double newLambda = std::min(0.0, lambda + deltaLambda);
  double applied = newLambda - lambda;
  lambda = newLambda;

  // Apply corrections
  markA.dx += nA.x * (wAinv * applied);
  buildWorldPolys(markA);

  markB.dx += nB.x * (wBinv * applied);
  buildWorldPolys(markB);
}

void HorizontalOrderConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  const double C = horizontalOrderResidual(markA, markB, selfKeep, crossKeep);
  if (C <= 0.0) return;  // correctly ordered

  ConstraintViolation v;
  v.type = ViolationType::HorizontalOrder;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = C;
  v.glyphA = markA.globalIndex;
  v.glyphB = markB.globalIndex;
  // Marker: a segment connecting the two out-of-order marks' box centers.
  v.markerCount = 2;
  v.marker[0] = {boxCenterX(markA), boxCenterY(markA)};
  v.marker[1] = {boxCenterX(markB), boxCenterY(markB)};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
