#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"

#include <algorithm>
#include <cmath>

#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/SolverContext.h"
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

bool WaqfEscapeConstraint::seedInitialSqueeze(SolverContext& context) {
  const auto& waqf = placement.waqfMark;
  const auto* base = waqf.prevBase;
  const double threshold = std::max(0.0, params.waqfEscapeMinGap);
  const GlyphInstance* stackMark = nullptr;
  geometry::Contact stackContact;
  double stackGap = threshold;
  const auto bounds = waqf.worldPolys.boundingAABB();
  for (const auto& mark : context.pageGlyphs[waqf.lineIndex]) {
    if (&mark == &waqf || mark.prevBase != base || !mark.isTopMark ||
        context.isWaqf(mark) || mark.worldPolys.empty() || boxTopY(mark) > bounds.maxy) continue;
    const auto contact = geometry::getDistance(waqf.worldPolys, mark.worldPolys, threshold).contact;
    if (contact.depth_or_gap < stackGap) {
      stackMark = &mark; stackContact = contact; stackGap = contact.depth_or_gap;
    }
  }
  if (!stackMark) return false;

  // Before the first ordinary contact pass, recognize an already occupied
  // pocket between the previous line and the owner's top stack. A nearest
  // penetration normal can point sideways here, so opposing normals alone
  // miss the squeeze and let the first contact push the waqf far to the right.
  for (int line = waqf.lineIndex - 1; line >= 0; --line) {
    const auto& previous = context.pageGlyphs[line];
    if (previous.empty()) continue;
    for (const auto& obstacle : previous) {
      if (obstacle.worldPolys.empty()) continue;
      const auto ob = obstacle.worldPolys.boundingAABB();
      if (bounds.minx >= ob.maxx || bounds.maxx <= ob.minx ||
          bounds.miny >= ob.maxy || bounds.maxy <= ob.miny) continue;
      const auto contact = geometry::getDistance(waqf.worldPolys, obstacle.worldPolys, 0.0).contact;
      if (contact.depth_or_gap >= 0.0) continue;
      recordContact(obstacle, contact.normal * -1.0, contact.depth_or_gap,
                    chooseMinGap(waqf, obstacle, params));
    }
    break;
  }
  if (contacts.empty()) return false;
  recordContact(*stackMark, stackContact.normal * -1.0, stackGap,
                chooseMinGap(waqf, *stackMark, params));
  return true;
}

double WaqfEscapeConstraint::opposedStackCompression(const SolverContext& context,
                                                    double bandResidual) const {
  const auto& waqf = placement.waqfMark;
  const double restoringDirection = bandResidual < 0.0 ? 1.0 : -1.0;
  double compression = 0.0;
  for (const auto& sample : contacts) {
    const auto& mark = *sample.obstacle;
    if (!mark.isMark || !mark.isTopMark || mark.prevBase != waqf.prevBase ||
        mark.lineIndex != waqf.lineIndex || context.isWaqf(mark) ||
        sample.desiredGap <= 0.0) continue;
    // Waqf signs are not ordinary members of the owning top stack.
    // Only recorded gap contacts qualify; remeasure completed geometry rather
    // than using a stale normal sampled halfway through the previous pass.
    const auto contact = geometry::getDistance(waqf.worldPolys, mark.worldPolys, sample.desiredGap).contact;
    if (!std::isfinite(contact.depth_or_gap) || contact.depth_or_gap >= sample.desiredGap ||
        restoringDirection * -contact.normal.x >= -0.2) continue;
    compression = std::max(compression, std::clamp(
        (sample.desiredGap - contact.depth_or_gap) / sample.desiredGap, 0.0, 1.0));
  }
  return compression;
}

void WaqfEscapeConstraint::project(SolverContext& context, double dt) {
  auto& waqf = placement.waqfMark;
  const auto* base = waqf.prevBase;
  if (!base || waqf.mobility <= 0.0 || waqf.worldPolys.empty()) return;

  const bool initialSqueeze = !initialContactsChecked && contacts.empty() && seedInitialSqueeze(context);
  initialContactsChecked = true;
  startedFromInitialSqueeze |= initialSqueeze;

  double smallestGap = std::numeric_limits<double>::infinity();
  double desiredGap = std::numeric_limits<double>::infinity();
  bool squeezedByOwnStack = false;
  const auto estimate = [&](const WaqfEscapeContact& c) {
    // An initial penetration can change its nearest feature as it exits left.
    // Recheck that small set of contacts in the completed geometry so an old
    // normal does not keep advancing the preference after the pocket clears.
    if (startedFromInitialSqueeze)
      return geometry::getDistance(waqf.worldPolys, c.obstacle->worldPolys, c.desiredGap).contact.depth_or_gap;
    const geometry::Vec2 relative{waqf.dx - c.obstacle->dx,
                                 waqf.dy - c.obstacle->dy};
    return c.gap + geometry::dot(c.normal, relative - c.relativeShift);
  };
  // The previous pass's pairs were sampled at different points in that pass.
  // Account for subsequent translations using their existing contact normals.
  // These estimates only trigger a preference; they never certify clearance.
  for (const auto& upper : contacts) {
    // A persistent contact with the previous line can alternate its nearest
    // feature and separation direction while the waqf is trapped. Identify
    // upper obstacles by their owning line, rather than the current normal.
    if (upper.obstacle->lineIndex >= waqf.lineIndex) continue;
    const double upperGap = estimate(upper);
    if (upperGap >= 0.9 * upper.desiredGap) continue;
    desiredGap = std::min(desiredGap, upper.desiredGap);
    smallestGap = std::min(smallestGap, upperGap);
    // Opposing contacts from the upper line and the owning top stack already
    // identify a squeeze. Waiting for stalled progress lets normal alignment
    // pull the waqf back across the upper outline and select a rightward exit.
    for (const auto& lower : contacts) {
      if (lower.obstacle->lineIndex != waqf.lineIndex ||
          lower.obstacle->prevBase != base || !lower.obstacle->isTopMark ||
          context.isWaqf(*lower.obstacle)) continue;
      if (estimate(lower) < 0.9 * lower.desiredGap &&
          (initialSqueeze || geometry::dot(upper.normal, lower.normal) < -0.5))
        squeezedByOwnStack = true;
    }
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
  // The initial preference only asks for the left side of the ordinary
  // alignment band. Contacts decide how much farther it must move. Spending
  // the full escape allowance before the first pass can overshoot the base.
  const double leftPercent = startedFromInitialSqueeze
      ? std::min(params.waqfEscapeMaxLeftPercent, placement.xAlignmentBandPercent)
      : params.waqfEscapeMaxLeftPercent;
  const double limit = -width * std::clamp(leftPercent, 0.0, 1000.0) / 100.0;
  const double step = width * std::clamp(params.waqfEscapeStepPercent, 0.0, 100.0) / 100.0;
  const double downStep = (bounds.maxy - bounds.miny) *
      std::clamp(params.waqfEscapeDownStepPercent, 0.0, 100.0) / 100.0;
  const double currentOffset = bounds.minx - baseLeft;
  if (tight && (active || squeezedByOwnStack || stalledIterations >= 3) &&
      (step > 0.0 || downStep > 0.0)) {
    if (!active) {
      targetOffset = initialSqueeze ? std::min(currentOffset, limit) : currentOffset;
      leftBandMinimum = std::min(targetOffset, -width * std::max(0.0,
          params.waqfEscapeMaxLeftPercent - placement.xAlignmentBandPercent) / 100.0);
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
      // Early contact corrections may briefly carry the mark much lower.
      // Advance the preference by one configured step, without adopting that
      // displacement as a new target and ratcheting the whole pocket down.
      const double from = startedFromInitialSqueeze ? targetBottomOffset
          : std::min(targetBottomOffset, bounds.miny - waqf.lineY);
      targetBottomOffset = std::max(floorOffset,
          from - downStep);
      lambdaDown = 0.0;
    }
  }
  if (!active) return;

  if (startedFromInitialSqueeze && !tight && downStep > 0.0) {
    // Clearing the previous line does not make the current height preferable.
    // Resume the ordinary soft height target beside the owning stack, instead
    // of leaving the sign stranded at the last contact-driven escape height.
    const double preferred = placement.preferredBottom(context) - waqf.lineY;
    if (targetBottomOffset > preferred + 1e-6) {
      targetBottomOffset = std::max(preferred, targetBottomOffset - downStep);
      lambdaDown = 0.0;
    }
  }

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
  if (startedFromInitialSqueeze) {
    // A soft target on the left side of the band must also resist an
    // overshoot caused by contacts; a one-sided pull would abandon the sign
    // after it crosses the target and allow it to drift beyond its base.
    const double C = currentOffset - std::clamp(currentOffset, leftBandMinimum, targetOffset);
    if (C == 0.0) { lambda = 0.0; bandRelaxed = false; }
    else {
      if (C * lambda > 0.0) lambda = 0.0;
      double compression = opposedStackCompression(context, C);
      if (C < 0.0) {
        // Yield inside the existing left allowance, then restore ordinary
        // association stiffness as that allowance runs out. The allowance is
        // unchanged and remains soft; this is not a final position projection.
        const double outerLimit = -width * std::clamp(params.waqfEscapeMaxLeftPercent, 0.0, 1000.0) / 100.0;
        const double reserve = leftBandMinimum - outerLimit;
        compression *= reserve > 1e-9
            ? std::clamp((currentOffset - outerLimit) / reserve, 0.0, 1.0) : 0.0;
      }
      const bool relaxed = compliance > 0.0 && compression > 1e-6;
      // Release the old spring when entering/leaving this contact response,
      // not on every iteration: repeated resets would harden the preference.
      if (relaxed != bandRelaxed) lambda = 0.0;
      bandRelaxed = relaxed;
      const double effectiveCompliance = compliance * (1.0 + 9.0 * compression);
      const double alpha = effectiveCompliance / (dt * dt);
      const double delta = (-C - alpha * lambda) / (waqf.mobility + alpha);
      lambda += delta;
      waqf.dx += waqf.mobility * delta;
    }
  } else {
    waqf.dx += correction(currentOffset - targetOffset, lambda, compliance);
  }
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
  const double left = waqf.worldPolys.boundingAABB().minx;
  const double minimum = waqf.prevBase->worldPolys.boundingAABB().minx + leftBandMinimum;
  const double horizontalResidual = startedFromInitialSqueeze
      ? std::abs(left - std::clamp(left, minimum, target)) : std::max(0.0, left - target);
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqf, waqf.prevBase,
      horizontalResidual, startedFromInitialSqueeze ? "Waqf early left escape band" : "Waqf left escape preference");
  const double bottomTarget = std::max(placement.escapeMinimumBottom(context),
                                      waqf.lineY + targetBottomOffset);
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqf, waqf.prevBase,
      std::max(0.0, boxBottomY(waqf) - bottomTarget), "Waqf downward escape preference");
}

} // namespace digitalkhatt::layout
