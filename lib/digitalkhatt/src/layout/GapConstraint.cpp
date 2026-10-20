#include "digitalkhatt/layout/GapConstraint.h"

#include <algorithm>
#include <cmath>

#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/BelowMarkPlacement.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/SweepBroadphase.h"
#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"

using namespace geometry;

namespace digitalkhatt::layout {

double gapCompliance(const GlyphInstance& A, const GlyphInstance& B) {
  return 0.1;
}

std::pair<double, double> gapMobilities(const SolverContext& context,
    const GlyphInstance& A, const GlyphInstance& B) {
  // A collision-only base transports its children. A parent/child contact must
  // refine the child alone; translating their common frame cannot separate it.
  if (A.collisionAsMark && B.isMark && B.prevBase == &A) return {0.0, B.mobility};
  if (B.collisionAsMark && A.isMark && A.prevBase == &B) return {A.mobility, 0.0};
  if (context.gapExcludedGlyphs.contains(A.globalIndex) || context.gapExcludedGlyphs.contains(B.globalIndex))
    return {context.gapExcludedGlyphs.contains(A.globalIndex) ? 0.0 : A.mobility,
            context.gapExcludedGlyphs.contains(B.globalIndex) ? 0.0 : B.mobility};
  if (context.rehWawBehPlacement && A.lineIndex != B.lineIndex) {
    if (A.lineIndex < B.lineIndex && context.contextualBelowMarks.contains(&A))
      return {A.mobility, 0.0};
    if (B.lineIndex < A.lineIndex && context.contextualBelowMarks.contains(&B))
      return {0.0, B.mobility};
  }
  if (A.isCollisionMark() && B.isCollisionMark()) {
    if (context.isWaqf(A) && !context.isWaqf(B))
      return {A.mobility > 0.0 ? 3.0 : 0.0, B.mobility > 0.0 ? 1.0 : 0.0};
    if (!context.isWaqf(A) && context.isWaqf(B))
      return {A.mobility > 0.0 ? 1.0 : 0.0, B.mobility > 0.0 ? 3.0 : 0.0};
  }
  return {A.mobility, B.mobility};
}

double effectiveGapCompliance(const GlyphInstance& A, const GlyphInstance& B, double gmin) {
  if (gmin <= 0.0) return 0.0;  // zero clearance still resolves intersections
  const double baseCompliance = gapCompliance(A, B);
  // Preserve the stronger historical 40/10-unit attachment contacts, while
  // larger tuned targets retain ordinary stiffness instead of becoming softer.
  const double normGmin = 80.0 / std::min(gmin, 80.0);
  return baseCompliance / (normGmin * normGmin);
}

namespace {
constexpr double squeezeInkGap = 20.0;
constexpr int squeezeMinimumRemainingIterations = 6;
constexpr double squeezeProgress = 0.5;

bool softBelowMarkPair(const SolverContext& context, const GlyphInstance& A, const GlyphInstance& B) {
  return A.isMark && B.isMark && A.lineIndex == B.lineIndex && A.prevBase != B.prevBase &&
      (context.belowFallbackMarks.contains(&A) || context.belowFallbackMarks.contains(&B));
}

void recordSqueezeContact(SolverContext& context, const GlyphInstance& mark,
    const GlyphInstance& obstacle, geometry::Vec2 away, double gap, double target, const OptParams& P) {
  if (!P.toggles.downwardEscape || !mark.isMark || mark.mobility <= 0.0 || !mark.prevBase ||
      (!isKasraMark(mark) && !context.downdotmarks.contains(mark.glyphName)) ||
      (obstacle.isMark && mark.prevBase == obstacle.prevBase) ||
      mark.lineIndex != obstacle.lineIndex ||
      (mark.wordIndex >= 0 && obstacle.wordIndex >= 0 && mark.wordIndex != obstacle.wordIndex) ||
      gap >= target || std::abs(away.x) < 0.2) return;
  auto& state = context.gapSqueezes.try_emplace(&mark).first->second;
  auto& closest = away.x > 0.0 ? state.leftGap : state.rightGap;
  auto& index = away.x > 0.0 ? state.left : state.right;
  if (gap < closest) { closest = gap; index = obstacle.globalIndex; }
}

const GapSqueezeState* squeezeState(const SolverContext& context, const GlyphInstance& mark) {
  if (context.gapSqueezes.empty()) return nullptr;
  const auto it = context.gapSqueezes.find(&mark);
  return it == context.gapSqueezes.end() || !it->second.active ? nullptr : &it->second;
}

bool earlyWaqfOwnStack(const SolverContext& context, const GlyphInstance& mark,
    const GlyphInstance& waqf) {
  if (context.waqfEscapes.empty() || !mark.isTopMark || mark.prevBase != waqf.prevBase ||
      context.isWaqf(mark)) return false;
  const auto* escape = context.waqfEscapes[waqf.globalIndex];
  return escape && escape->active && escape->startedFromInitialSqueeze;
}

geometry::Vec2 gapResponse(const GapSqueezeState* state, geometry::Vec2 away,
    bool horizontalOnly = false) {
  // An oblique contact response keeps its horizontal separation arm and adds
  // a downward preference. With bias <= 1.5, dot(away,response) stays >= .25,
  // so this remains a separating direction even for an upward-facing normal.
  if (state) away.y -= state->downwardBias * std::abs(away.x);
  // During an initial waqf escape, its own top marks may give way sideways,
  // but the contact must not press them down into their dots/harakat stack.
  // Their vertical mobility under all other constraints stays unchanged.
  if (horizontalOnly) away.y = 0.0;
  return away;
}

void applyGapMove(GlyphInstance& mark, geometry::Vec2 move, const GapSqueezeState* state) {
  if (state && move.y < 0.0) {
    // Directional mobility at the owner's local descent limit. Do not force
    // a mark upward if another constraint already put it below this limit.
    move.y = std::max(move.y, -std::max(0.0, boxTopY(mark) - state->minimumTop));
  }
  mark.dx += move.x; mark.dy += move.y;
  updateWorldPolys(mark);
}
}

bool beginGapPass(SolverContext& context,
    const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs, const OptParams& P,
    int iterationsRemaining) {
  if (!P.toggles.downwardEscape) { context.gapSqueezes.clear(); return false; }
  const int repeats = std::clamp(P.downwardEscape.repeatedCollisions, 1, 100);
  bool pending = false;
  for (auto& [mark, state] : context.gapSqueezes) {
    // Count unresolved contacts in the previous iteration's FINAL geometry,
    // not transient intersections created halfway through the pair pass.
    auto finalGap = [&](int& index, bool left) {
      if (index < 0 || static_cast<size_t>(index) >= glyphs.size())
        return std::numeric_limits<double>::infinity();
      const auto contact = geometry::getDistance(mark->worldPolys, glyphs[index].get().worldPolys, 2 * squeezeInkGap).contact;
      const auto away = contact.normal * -1.0;
      if ((left ? away.x : -away.x) < 0.2) { index = -1; return std::numeric_limits<double>::infinity(); }
      return contact.depth_or_gap;
    };
    state.leftGap = finalGap(state.left, true);
    state.rightGap = finalGap(state.right, false);
    const double gap = std::min(state.leftGap, state.rightGap);
    const bool squeezed = state.left >= 0 && state.right >= 0 && gap < squeezeInkGap;
    const bool sameObstacles = state.left == state.previousLeft && state.right == state.previousRight;
    const double progressThreshold = std::max(squeezeProgress, 0.01 * boxHeight(*mark));
    const bool improving = gap > state.previousGap + progressThreshold;
    state.stalledIterations = !squeezed ? 0 : sameObstacles && !improving ? state.stalledIterations + 1 : 1;
    if (!state.active && state.stalledIterations >= repeats && mark->prevBase &&
        iterationsRemaining >= squeezeMinimumRemainingIterations) {
      const double height = boxHeight(*mark);
      const double initialTop = boxTopY(*mark) - mark->dy + mark->initialPositioningDy;
      const double maxDrop = std::clamp(P.downwardEscape.maxDescentPercent, 0.0, 500.0) * height / 100.0;
      const double baseDepth = std::clamp(P.downwardEscape.maxBelowBasePercent, 0.0, 500.0) * height / 100.0;
      state.minimumTop = std::max(boxBottomY(*mark->prevBase) - baseDepth, initialTop - maxDrop);
      if (boxTopY(*mark) > state.minimumTop) state.active = true;
    }
    pending |= !state.active && squeezed && state.stalledIterations < repeats &&
               iterationsRemaining >= squeezeMinimumRemainingIterations + repeats - state.stalledIterations;
    // Fade the preference after the small ink gap opens. The 80-unit comfort
    // target alone never starts or sustains a downward escape.
    state.downwardBias = state.active ? 1.5 * std::clamp((2 * squeezeInkGap - gap) / squeezeInkGap, 0.0, 1.0) : 0.0;
    state.previousLeft = state.left; state.previousRight = state.right; state.previousGap = gap;
    state.left = state.right = -1;
    state.leftGap = state.rightGap = std::numeric_limits<double>::infinity();
  }
  return pending;
}

double effectiveGapCompliance(const SolverContext& context, const GlyphInstance& A,
    const GlyphInstance& B, const OptParams& P, double gmin) {
  if (A.lineIndex != B.lineIndex) return gmin > 0.0 ? 0.1 : 0.0;
  return softBelowMarkPair(context, A, B) ? P.compliance.rehWawBelowMarkGap :
                                          effectiveGapCompliance(A, B, gmin);
}

double expectedComplianceResidual(double initialC, double compliance, double w, double dt) {
  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-9) return 0.0;  // fully rigid pair: no slack expected
  return initialC * alpha / denom;
}

double chooseMinGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& P) {
  if (A.lineIndex != B.lineIndex) return std::max(P.differentWordGap, P.differentLineGap);
  if (A.wordIndex >= 0 && B.wordIndex >= 0 && A.wordIndex != B.wordIndex)
    return P.differentWordGap;
  const auto tipPair = [](const GlyphInstance& base, const GlyphInstance& mark) {
    if (!isRehWaw(base) || !mark.isMark) return false;
    // Check against base.nextBase so no unvalidated mark owner is traversed.
    const auto* owner = base.nextBase;
    return owner && owner == mark.prevBase && isRehWawFollowingBase(*owner) &&
           (isKasraMark(mark) || isRehWawDot(mark));
  };
  if (P.toggles.rehWawBehPlacement) {
    if (tipPair(A, B) || tipPair(B, A)) return std::min(P.sameWordGap, P.rehWaw.tipGap);
    const auto lowerMarkOwnerPair = [](const GlyphInstance& base, const GlyphInstance& mark) {
      const bool independentKasra = isKasraMark(mark) &&
          mark.initialPlacement != GlyphInstance::InitialPlacement::None;
      if (!mark.isMark || (!isRehWawDot(mark) && !independentKasra) || !rehWawReference(mark)) return false;
      return &base == mark.prevBase;
    };
    if (lowerMarkOwnerPair(A, B) || lowerMarkOwnerPair(B, A)) return std::min(P.sameWordGap, P.rehWaw.tipGap);
  }
  if (A.glyphName.starts_with("alef") &&
      B.prevBase == &A &&
      (B.glyphName.starts_with("hamza") ||
       B.glyphName.starts_with("wasla") ||
       B.glyphName.starts_with("smallhighroundedzero"))) {
    return 40;
  }

  auto isSLMKasra = A.glyphName == "kasra" && B.glyphName == "smalllowmeem" && B.glyphIndex == A.glyphIndex + 1;
  if (isSLMKasra)
    return 10;
  else
    return P.sameWordGap;
}

double chooseOrdinaryGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& P) {
  return A.lineIndex != B.lineIndex ? P.differentWordGap : chooseMinGap(A, B, P);
}

static void projectGapConstraint(SolverContext& solverContext,
                        GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& P,
                        double dt,
                        double& maxPenetration, bool extraLine) {
  // Decide desired min gap.
  const bool isMarkExists = (A.isCollisionMark() || B.isCollisionMark());

  if (!isMarkExists) return;

  const auto [wA, wB] = gapMobilities(solverContext, A, B);

  // If both are completely rigid, we can't resolve here
  if (wA <= 0.0 && wB <= 0.0) {
    return;
  }

  const double gmin = extraLine ? P.differentLineGap : chooseOrdinaryGap(A, B, P);
  const double compliance = extraLine ? P.compliance.extraLineSpacing :
      effectiveGapCompliance(solverContext, A, B, P, gmin);

  const auto clearMultiplier = [&] {
    const auto it = solverContext.gapInfos.find(GapKey{&A, &B});
    if (it != solverContext.gapInfos.end())
      (extraLine ? it->second.extraLine : it->second.ordinary).lambda = 0;
  };
  auto dr = getDistance(A.worldPolys, B.worldPolys, gmin);
  if (!std::isfinite(dr.contact.depth_or_gap)) {
    // The bounded distance query can omit a satisfied target entirely. The
    // two cross-line constraints must still release their states separately.
    if (A.lineIndex != B.lineIndex && P.differentLineGap > P.differentWordGap)
      clearMultiplier();
    return;
  }

  double C = dr.contact.depth_or_gap - gmin;  // want C >= 0

  if (!extraLine && wA > 0.0) recordSqueezeContact(solverContext, A, B, dr.contact.normal * -1.0, dr.contact.depth_or_gap, gmin, P);
  if (!extraLine && wB > 0.0) recordSqueezeContact(solverContext, B, A, dr.contact.normal, dr.contact.depth_or_gap, gmin, P);

  if (!extraLine && C < 0.0 && !solverContext.waqfEscapes.empty()) {
    if (auto* escape = solverContext.waqfEscapes[A.globalIndex]; escape && wA > 0.0)
      escape->recordContact(B, dr.contact.normal * -1.0, dr.contact.depth_or_gap, gmin);
    if (auto* escape = solverContext.waqfEscapes[B.globalIndex]; escape && wB > 0.0)
      escape->recordContact(A, dr.contact.normal, dr.contact.depth_or_gap, gmin);
  }

  auto gapKey = GapKey{&A, &B};

  if (C >= 0.0) {
    clearMultiplier();
    return;
  }

  auto& pairInfo = solverContext.gapInfos[gapKey];
  auto& gapInfo = extraLine ? pairInfo.extraLine : pairInfo.ordinary;
  if (!gapInfo.hasInitialC) {
    // First time this pair is seen violated: this IS C0 for
    // expectedComplianceResidual (the violation the solver starts from,
    // before the compliant constraint has applied any correction).
    gapInfo.initialC = C;
    gapInfo.hasInitialC = true;
  }

  double& lambda = gapInfo.lambda;

  const auto* escapeA = P.toggles.downwardEscape ? squeezeState(solverContext, A) : nullptr;
  const auto* escapeB = P.toggles.downwardEscape ? squeezeState(solverContext, B) : nullptr;
  if (gapInfo.escapingA != (escapeA != nullptr) || gapInfo.escapingB != (escapeB != nullptr)) {
    // A new response direction starts a fresh contact multiplier, rather than
    // inheriting a spring equilibrium computed before the squeeze activated.
    lambda = 0.0;
    gapInfo.escapingA = escapeA != nullptr; gapInfo.escapingB = escapeB != nullptr;
  }

  maxPenetration = std::min(maxPenetration, C);

  Vec2 n = dr.contact.normal;
  const bool horizontalA = earlyWaqfOwnStack(solverContext, A, B);
  const bool horizontalB = earlyWaqfOwnStack(solverContext, B, A);
  const auto responseA = gapResponse(escapeA, n * -1.0, horizontalA);
  const auto responseB = gapResponse(escapeB, n, horizontalB);

  // XPBD parameters
  double alpha = compliance / (dt * dt);

  const double weight = !escapeA && !escapeB && !horizontalA && !horizontalB ? wA + wB :
      wA * geometry::dot(n * -1.0, responseA) + wB * geometry::dot(n, responseB);
  double denom = weight + alpha;
  if (denom < 1e-9)
    return;

  double deltaLambda = (-C - alpha * lambda) / denom;

  // Escape by small contact steps, not a single jump across the neighboring
  // letter. The owner-relative floor below also limits cumulative descent.
  double maxStep = std::numeric_limits<double>::infinity();
  if (escapeA && wA > 0.0)
    maxStep = std::min(maxStep, boxHeight(A) * 0.125 / (wA * geometry::len(responseA)));
  if (escapeB && wB > 0.0)
    maxStep = std::min(maxStep, boxHeight(B) * 0.125 / (wB * geometry::len(responseB)));
  deltaLambda = std::clamp(deltaLambda, -maxStep, maxStep);

  // Enforce inequality: lambda >= 0
  double lambdaNew = std::max(0.0, lambda + deltaLambda);
  deltaLambda = lambdaNew - lambda;
  lambda = lambdaNew;

  gapInfo.count++;
  if (wA > 0.0) {
    applyGapMove(A, responseA * (wA * deltaLambda), escapeA);
  }

  if (wB > 0.0) {
    applyGapMove(B, responseB * (wB * deltaLambda), escapeB);
  }
}

void solveGapConstraint(SolverContext& context, GlyphInstance& A, GlyphInstance& B,
                        const OptParams& P, double dt, double& maxPenetration) {
  projectGapConstraint(context, A, B, P, dt, maxPenetration, false);
  if (A.lineIndex != B.lineIndex && P.differentLineGap > P.differentWordGap)
    projectGapConstraint(context, A, B, P, dt, maxPenetration, true);
}

// Re-evaluate the generic broadphase gap pass read-only, after the solve, to
// collect penetrations. The gap pass has no persistent constraint objects, so
// it must be rebuilt here. Squeezed pairs are owned by SqueezeCenterConstraint
// (whose reportViolations covers them), so they are skipped -- the same
// partition the solver uses, avoiding double-counting and coverage gaps.
void collectGapViolations(
    SolverContext& solverContext,
    const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs,
    const OptParams& P,
    std::vector<ConstraintViolation>& out) {
  std::vector<std::pair<int, int>> pairs;
  SweepBroadphase sweep(glyphs, P);
  sweep.findPairs(pairs);

  for (auto& pr : pairs) {
    if (solverContext.isGapExcluded(pr)) continue;
    GlyphInstance& A = glyphs[pr.first];
    GlyphInstance& B = glyphs[pr.second];
    if (!(A.isCollisionMark() || B.isCollisionMark())) continue;

    const auto report = [&](bool extraLine) {
      const double gmin = extraLine ? P.differentLineGap : chooseOrdinaryGap(A, B, P);
      auto dr = geometry::getDistance(A.worldPolys, B.worldPolys, gmin);
      if (!std::isfinite(dr.contact.depth_or_gap)) return;

      if (extraLine && dr.contact.intersect) return;
      const double C = dr.contact.depth_or_gap - gmin;  // violation when C < 0
      if (C >= 0.0) return;

      // A soft (compliance > 0) gap constraint is a spring by design: it is
      // expected to settle at a nonzero residual, not close fully. Predict that
      // residual from the pair's own compliance/mobility (same formula the
      // solver's convergence follows) and drop the violation if the actual
      // residual is at or below it -- that's the constraint behaving exactly as
      // configured, not a defect. initialC comes from the live solve's
      // gapInfos (set the first time this pair was seen violated); if this
      // exact pair was never processed there (no entry), fall back to the
      // current C itself, i.e. assume no compliance shrinkage history and don't
      // suppress -- conservative, avoids hiding a genuine issue.
      const double compliance = extraLine ? P.compliance.extraLineSpacing :
          effectiveGapCompliance(solverContext, A, B, P, gmin);
      const auto [wA, wB] = gapMobilities(solverContext, A, B);
      const auto n = dr.contact.normal;
      const auto* escapeA = P.toggles.downwardEscape ? squeezeState(solverContext, A) : nullptr;
      const auto* escapeB = P.toggles.downwardEscape ? squeezeState(solverContext, B) : nullptr;
      const bool horizontalA = earlyWaqfOwnStack(solverContext, A, B);
      const bool horizontalB = earlyWaqfOwnStack(solverContext, B, A);
      const double w = !escapeA && !escapeB && !horizontalA && !horizontalB ? wA + wB :
          wA * geometry::dot(n * -1.0, gapResponse(escapeA, n * -1.0, horizontalA)) +
          wB * geometry::dot(n, gapResponse(escapeB, n, horizontalB));
      const auto gapIt = solverContext.gapInfos.find(GapKey{&A, &B});
      const auto* state = gapIt == solverContext.gapInfos.end() ? nullptr :
          (extraLine ? &gapIt->second.extraLine : &gapIt->second.ordinary);
      const double initialC = state && state->hasInitialC ? state->initialC : C;
      const double expected =
          expectedComplianceResidual(initialC, compliance, w, /*dt=*/1.0);
      // Expected spring slack must never hide an actual ink intersection.
      const double allowed = -expected * P.complianceResidualMargin;
      if (!dr.contact.intersect && -C <= allowed) return;

      ConstraintViolation v;
      v.type = ViolationType::GenericGap;
      v.kind = (extraLine || (softBelowMarkPair(solverContext, A, B) && compliance > 0.0)) &&
          !dr.contact.intersect && dr.contact.depth_or_gap >= 0.0 ?
          ViolationKind::Soft : ViolationKind::Hard;
      v.residual = C;
      v.severity = -C;
      v.allowedResidual = allowed;
      if (extraLine) v.diagnostic = "extra-line-spacing";
      v.detail = extraLine ? "Extra line spacing not reached" : dr.contact.depth_or_gap < 0.0 ? "Collision-proxy intersection" :
          dr.contact.intersect ? "Collision-proxy contact (touching or unresolved penetration)" : "Minimum gap not reached";
      v.glyphA = A.globalIndex;
      v.glyphB = B.globalIndex;
      v.markerCount = 2;  // the contact segment between the two shapes
      v.marker[0] = dr.contact.pA;
      v.marker[1] = dr.contact.pB;
      out.push_back(v);
    };
    report(false);
    if (A.lineIndex != B.lineIndex && P.differentLineGap > P.differentWordGap)
      report(true);
  }
}


}  // namespace digitalkhatt::layout
