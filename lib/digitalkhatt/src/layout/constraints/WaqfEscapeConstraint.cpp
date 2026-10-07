#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"

#include <algorithm>
#include <cmath>

#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

namespace digitalkhatt::layout {

WaqfEscapeConstraint::WaqfEscapeConstraint(WaqfPlacementConstraint& placement_,
                                         const OptParams& params_)
    : placement(placement_), params(params_) {
  compliance = placement.xAlignCompliance;
  reportEnabled = true;
}

void WaqfEscapeConstraint::recordContact(const GlyphInstance& obstacle,
                                        geometry::Vec2 normal, double gap,
                                        double desiredGap) {
  const auto& waqf = placement.waqfMark;
  contacts.push_back({&obstacle, normal,
      {waqf.dx - obstacle.dx, waqf.dy - obstacle.dy}, gap, desiredGap});
}

void WaqfEscapeConstraint::project(SolverContext& context, double dt) {
  auto& waqf = placement.waqfMark;
  const auto* base = waqf.prevBase;
  if (!base || waqf.mobility <= 0.0 || waqf.worldPolys.empty()) return;

  double smallestGap = std::numeric_limits<double>::infinity();
  double desiredGap = std::numeric_limits<double>::infinity();
  // The previous pass's pairs were sampled at different points in that pass.
  // Account for subsequent translations using their existing contact normals.
  // These estimates only trigger a preference; they never certify clearance.
  for (const auto& upper : contacts) {
    if (upper.normal.y > -0.5 || upper.obstacle->lineIndex >= waqf.lineIndex) continue;
    const auto estimate = [&](const WaqfEscapeContact& c) {
      const geometry::Vec2 relative{waqf.dx - c.obstacle->dx,
                                   waqf.dy - c.obstacle->dy};
      return c.gap + geometry::dot(c.normal, relative - c.relativeShift);
    };
    const double upperGap = estimate(upper);
    if (upperGap >= 0.9 * upper.desiredGap) continue;
    desiredGap = std::min(desiredGap, upper.desiredGap);
    smallestGap = std::min(smallestGap, upperGap);
  }

  const double threshold = std::max(0.0, params.waqfEscapeMinGap);
  const bool tight = smallestGap < threshold;
  const double progressTolerance = std::max(params.tolCollision, 0.1 * desiredGap);
  if (tight && smallestGap <= previousGap + progressTolerance)
    ++stalledIterations;
  else
    stalledIterations = 0;
  previousGap = smallestGap;

  const auto bounds = waqf.worldPolys.boundingAABB();
  const double width = bounds.maxx - bounds.minx;
  const double baseLeft = base->worldPolys.boundingAABB().minx;
  const double limit = -width * std::clamp(params.waqfEscapeMaxLeftPercent, 0.0, 1000.0) / 100.0;
  const double step = width * std::clamp(params.waqfEscapeStepPercent, 0.0, 100.0) / 100.0;
  const double downStep = (bounds.maxy - bounds.miny) *
      std::clamp(params.waqfEscapeDownStepPercent, 0.0, 100.0) / 100.0;
  const double currentOffset = bounds.minx - baseLeft;
  if (tight && (active || stalledIterations >= 3) && (step > 0.0 || downStep > 0.0)) {
    if (!active) {
      targetOffset = currentOffset;
      targetBottomOffset = bounds.miny - waqf.lineY;
      active = placement.collisionEscapeActive = true;
      placement.lambdaX = 0.0;
      placement.lambdaMin = placement.lambdaTarget = 0.0;
    }
    // Advance only while tight. Hold the chosen preference for the remainder
    // of this solve so normal alignment cannot restore the contact pocket.
    if (currentOffset > limit && step > 0.0) {
      targetOffset = std::max(limit, std::min(targetOffset, currentOffset) - step);
      lambda = 0.0; // a changed target cannot inherit the old spring multiplier
    }
    const double floorOffset = placement.escapeMinimumBottom(context) - waqf.lineY;
    if (downStep > 0.0 && bounds.miny - waqf.lineY > floorOffset) {
      targetBottomOffset = std::max(floorOffset,
          std::min(targetBottomOffset, bounds.miny - waqf.lineY) - downStep);
      lambdaDown = 0.0;
    }
  }
  if (!active) return;

  const auto correction = [&](double C, double& multiplier, double softness) {
    if (C <= 0.0) { multiplier = 0.0; return 0.0; }
    const double alpha = softness / (dt * dt);
    const double denom = waqf.mobility + alpha;
    if (denom <= 1e-12) return 0.0;
    const double next = std::min(0.0, multiplier - (C + alpha * multiplier) / denom);
    const double delta = waqf.mobility * (next - multiplier);
    multiplier = next;
    return delta;
  };
  waqf.dx += correction(currentOffset - targetOffset, lambda, compliance);
  const double minimumBottom = placement.escapeMinimumBottom(context);
  const double targetBottom = std::max(minimumBottom, waqf.lineY + targetBottomOffset);
  const double descent = correction(bounds.miny - targetBottom, lambdaDown, placement.targetCompliance);
  waqf.dy += std::max(descent, std::min(0.0, minimumBottom - bounds.miny));
  updateWorldPolys(waqf);
}

void WaqfEscapeConstraint::reportViolations(SolverContext& context,
                                           std::vector<ConstraintViolation>& out) const {
  const auto& waqf = placement.waqfMark;
  if (!active || !waqf.prevBase) return;
  const double target = waqf.prevBase->worldPolys.boundingAABB().minx + targetOffset;
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqf, waqf.prevBase,
      std::max(0.0, waqf.worldPolys.boundingAABB().minx - target), "Waqf left escape preference");
  const double bottomTarget = std::max(placement.escapeMinimumBottom(context),
                                      waqf.lineY + targetBottomOffset);
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqf, waqf.prevBase,
      std::max(0.0, boxBottomY(waqf) - bottomTarget), "Waqf downward escape preference");
}

} // namespace digitalkhatt::layout
