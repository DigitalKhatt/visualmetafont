#include "digitalkhatt/layout/constraints/YlaneConstraint.h"

#include <algorithm>
#include <string>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

namespace {

// Role-based fraction of the mark's world bbox height used as the vertical
// reference point (0 = bottom of ink, 1 = top of ink). Authored per mark
// family, in the same spirit as topStackTable(): damma is referenced near its
// top loop so its hanging tail does not float the whole band up; fatha / sukun
// / shadda are referenced at their optical center. This replaces the old
// ad-hoc `metrics.height/2` (+ damma `height/6`) DESIGN-space fudge with a
// principled reference taken from real WORLD geometry.
double bandRefFrac(const std::string& name) {
  if (name.starts_with("damma")) return 0.66;  // ~ old center + height/6
  return 0.5;                                   // fatha, sukun, shadda, default
}

}  // namespace

void YlaneConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  // Baseline reference: prefer the owning base's baseline, fall back to the
  // mark's own line baseline. (The anchor always has prevBase == its base.)
  const double baseline = mark.prevBase ? mark.prevBase->lineY : mark.lineY;

  // Real world-geometry reference point (NOT design metrics).
  const double yRef =
      boxBottomY(mark) + bandRefFrac(mark.glyphName) * boxHeight(mark);

  double yTarget = baseline + laneHeight;
  // --- PHASE 2 HOOK (disabled in phase 1) ---------------------------------
  // Letter-aware undulation / dot-cluster lift beyond what the hard rails do:
  //   yTarget = std::max(yTarget, seatClearance(mark, solverContext));
  // Left off for now: the parallel hard rails already force marks above the
  // band when a real obstacle intrudes, and phase 1 wants a pure flat band.
  // ------------------------------------------------------------------------

  // One-sided FLOOR (inequality yRef >= yTarget):  C = yRef - yTarget.
  // yRef moves 1:1 with mark.dy, so grad g = dC/d(dy) = +1 when active.
  const double C = yRef - yTarget;

  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;  // (w * g^2 + alpha), g^2 = 1
  if (denom < 1e-12) return;

  // The lane may push a mark UP toward the band but must NEVER pull it DOWN (a
  // downward pull drags e.g. a hamza-above over alef below the letter and hurts
  // readability). When satisfied (C >= 0), zero the gradient so the mark does
  // NOT move, but still let lambda DECAY toward 0 rather than freeze it. A
  // frozen stale positive multiplier can, on reactivation, form a phantom fixed
  // point at C = -alpha*lambda < 0 -- parking the mark BELOW the lane (the
  // max(0,.) clamp can't undo it since lambda stays positive). Decaying while
  // inactive keeps warm-start continuity for brief excursions yet forgets stale
  // force under sustained satisfaction, so reactivation pushes cleanly up again.
  const double Cactive = (C < 0.0) ? C : 0.0;
  const double grad = (C < 0.0) ? 1.0 : 0.0;

  const double deltaLambda = -(Cactive + alpha * lambda) / denom;
  const double lambdaNew = std::max(0.0, lambda + deltaLambda);
  const double applied = lambdaNew - lambda;
  lambda = lambdaNew;

  if (grad != 0.0) {
    // Correction along grad = +1 (up only): dy += w * applied.
    mark.dy += grad * w * applied;
    buildWorldPolys(mark);
  }
}

void YlaneConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  const double baseline = mark.prevBase ? mark.prevBase->lineY : mark.lineY;
  const double yRef =
      boxBottomY(mark) + bandRefFrac(mark.glyphName) * boxHeight(mark);
  const double yTarget = baseline + laneHeight;

  const double C = yRef - yTarget;  // violation when C < 0 (below the band)
  if (C >= 0.0) return;

  const auto mb = mark.worldPolys.boundingAABB();
  ConstraintViolation v;
  v.type = ViolationType::Ylane;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = -C;
  v.glyphA = mark.globalIndex;
  v.glyphB = mark.prevBase ? mark.prevBase->globalIndex : -1;
  v.markerCount = 2;  // the band line as a segment at y = yTarget under the mark
  v.marker[0] = {mb.minx, yTarget};
  v.marker[1] = {mb.maxx, yTarget};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
