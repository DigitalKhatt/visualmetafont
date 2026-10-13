#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

using namespace geometry;

namespace digitalkhatt::layout {
namespace {

constexpr double kStretchedFathaJoinedGap = 100.0;

enum class FixedFatha { None,
                        A,
                        B };

FixedFatha fixedFathaForJoinedPair(const GlyphInstance& a, const GlyphInstance& b) {
  const auto stretchedFatha = [](const GlyphInstance& glyph) {
    return glyph.preserveGposHorizontal && glyph.glyphName == "fatha.stretched";
  };
  const auto joined = [](const GlyphInstance& glyph) {
    return glyph.glyphName == "smallalef.joined" || glyph.glyphName == "hamzaabove.joined";
  };
  if (stretchedFatha(a) && joined(b)) return FixedFatha::A;
  if (joined(a) && stretchedFatha(b)) return FixedFatha::B;
  return FixedFatha::None;
}

double stretchedFathaJoinedGap(const GlyphInstance& a, const GlyphInstance& b, FixedFatha fixedFatha) {
  const auto& fatha = fixedFatha == FixedFatha::A ? a : b;
  const double added = fatha.justificationAddedLeftTatweel;
  return added > 0 ? kStretchedFathaJoinedGap * std::min(1.0, added) : 0;
}

}  // namespace

// Signed anti-misassociation residual, measured on the ink box.
//
// markB is the left mark (next owner), markA the right mark (earlier owner), so the
// overlap of markB's right ink edge past markA's left ink edge is O = B_R - A_L.
// Each mark must keep an exposed protrusion on its own-base side that clears both a
// self-floor (recognizability) and a cross-floor (not swamped by the neighbor):
//   exposed(A) = wA - O >= max(selfKeep*wA, crossKeep*wB)
//   exposed(B) = wB - O >= max(selfKeep*wB, crossKeep*wA)
// Collapsing to a single allowed overlap T (clamped to zero):
//   C = O - T,  want C <= 0.
// The bbox terms are constant w.r.t. dx, so the gradient is unchanged.
static double horizontalOrderResidual(const GlyphInstance& markA,
                                      const GlyphInstance& markB,
                                      double selfKeep, double crossKeep, FixedFatha fixedFatha) {
  const auto a = markA.worldPolys.boundingAABB();
  const auto b = markB.worldPolys.boundingAABB();
  // Preserve semantic right-to-left order, with a physical gap instead of
  // exposed-width fractions for this pair. No vertical clearance is implied.
  if (fixedFatha != FixedFatha::None) return b.maxx + stretchedFathaJoinedGap(markA, markB, fixedFatha) - a.minx;
  const double wA = a.maxx - a.minx;
  const double wB = b.maxx - b.minx;

  const double B_R = b.maxx;
  const double A_L = a.minx;
  const double O = B_R - A_L;

  const double TA = wA - std::max(selfKeep * wA, crossKeep * wB);
  const double TB = wB - std::max(selfKeep * wB, crossKeep * wA);
  const double T = std::min(TA, TB);

  return O - std::max(T, 0.0);
}

void HorizontalOrderConstraint::project(SolverContext& solverContext, double dt) {
  const auto fixedFatha = fixedFathaForJoinedPair(markA, markB);
  // Constraint-local mobility only: do not freeze Fatha for collision, lane,
  // stacking or any other projection.
  const double wAinv = fixedFatha == FixedFatha::A ? 0.0 : markA.mobility;
  const double wBinv = fixedFatha == FixedFatha::B ? 0.0 : markB.mobility;
  if (wAinv + wBinv == 0.0) return;

  double C = horizontalOrderResidual(markA, markB, selfKeep, crossKeep, fixedFatha);

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
  updateWorldPolys(markA);

  markB.dx += nB.x * (wBinv * applied);
  updateWorldPolys(markB);
}

void HorizontalOrderConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  const auto fixedFatha = fixedFathaForJoinedPair(markA, markB);
  const double C = horizontalOrderResidual(markA, markB, selfKeep, crossKeep, fixedFatha);
  if (C <= 0.0) return;  // correctly ordered

  ConstraintViolation v;
  v.type = ViolationType::HorizontalOrder;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = C;
  v.glyphA = markA.globalIndex;
  v.glyphB = markB.globalIndex;
  if (fixedFatha != FixedFatha::None) v.detail = "Stretched Fatha/joined mark horizontal gap (" + std::to_string(stretchedFathaJoinedGap(markA, markB, fixedFatha)) + "); Fatha fixed for ordering";
  // Marker: a segment connecting the two out-of-order marks' box centers.
  v.markerCount = 2;
  v.marker[0] = {boxCenterX(markA), boxCenterY(markA)};
  v.marker[1] = {boxCenterX(markB), boxCenterY(markB)};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
