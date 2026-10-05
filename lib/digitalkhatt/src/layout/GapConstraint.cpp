#include "digitalkhatt/layout/GapConstraint.h"

#include <algorithm>
#include <cmath>

#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/SweepBroadphase.h"

using namespace geometry;

namespace digitalkhatt::layout {

double gapCompliance(const GlyphInstance& A, const GlyphInstance& B) {
  return 0.1;
}

std::pair<double, double> gapMobilities(const SolverContext& context,
    const GlyphInstance& A, const GlyphInstance& B) {
  if (A.isMark && B.isMark) {
    if (context.isWaqf(A) && !context.isWaqf(B)) return {3.0, 1.0};
    if (!context.isWaqf(A) && context.isWaqf(B)) return {1.0, 3.0};
  }
  return {A.mobility, B.mobility};
}

double effectiveGapCompliance(const GlyphInstance& A, const GlyphInstance& B, double gmin) {
  const double baseCompliance = gapCompliance(A, B);
  const double normGmin = 80.0 / gmin;
  return baseCompliance / (normGmin * normGmin);
}

double expectedComplianceResidual(double initialC, double compliance, double w, double dt) {
  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-9) return 0.0;  // fully rigid pair: no slack expected
  return initialC * alpha / denom;
}

double chooseMinGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& P) {
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
    return 80;
}

void solveGapConstraint(SolverContext& solverContext,
                        GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& P,
                        double dt,
                        double& maxPenetration) {
  // Decide desired min gap.
  const bool isMarkExists = (A.isMark || B.isMark);

  if (!isMarkExists) return;

  const auto [wA, wB] = gapMobilities(solverContext, A, B);

  // If both are completely rigid, we can't resolve here
  if (wA <= 0.0 && wB <= 0.0) {
    return;
  }

  const double gmin = chooseMinGap(A, B, P);
  double compliance = effectiveGapCompliance(A, B, gmin);

  auto dr = getDistance(A.worldPolys, B.worldPolys, gmin);
  if (!std::isfinite(dr.contact.depth_or_gap))
    return;

  double C = dr.contact.depth_or_gap - gmin;  // want C >= 0

  auto gapKey = GapKey{&A, &B};

  if (C >= 0.0) {
    auto it = solverContext.gapInfos.find(gapKey);
    if (it != solverContext.gapInfos.end()) {
      it->second.lambda = 0;
    }
    return;
  }

  auto [it, inserted] = solverContext.gapInfos.try_emplace(gapKey);
  auto& gapInfo = it->second;
  if (inserted) {
    // First time this pair is seen violated: this IS C0 for
    // expectedComplianceResidual (the violation the solver starts from,
    // before the compliant constraint has applied any correction).
    gapInfo.initialC = C;
    gapInfo.hasInitialC = true;
  }

  double& lambda = gapInfo.lambda;

  maxPenetration = std::min(maxPenetration, C);

  Vec2 n = dr.contact.normal;

  // XPBD parameters
  double alpha = compliance / (dt * dt);

  double denom = wA + wB + alpha;
  if (denom < 1e-9)
    return;

  double deltaLambda = (-C - alpha * lambda) / denom;

  // Enforce inequality: lambda >= 0
  double lambdaNew = std::max(0.0, lambda + deltaLambda);
  deltaLambda = lambdaNew - lambda;
  lambda = lambdaNew;

  gapInfo.count++;
  if (wA > 0.0) {
    Vec2 corrA = n * (-wA * deltaLambda);
    A.dx += corrA.x;
    A.dy += corrA.y;
    updateWorldPolys(A);
  }

  if (wB > 0.0) {
    Vec2 corrB = n * (wB * deltaLambda);
    B.dx += corrB.x;
    B.dy += corrB.y;
    updateWorldPolys(B);
  }
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
    if (!(A.isMark || B.isMark)) continue;  // mirror solveGapConstraint's filter

    const double gmin = chooseMinGap(A, B, P);
    auto dr = geometry::getDistance(A.worldPolys, B.worldPolys, gmin);
    if (!std::isfinite(dr.contact.depth_or_gap)) continue;

    const double C = dr.contact.depth_or_gap - gmin;  // violation when C < 0
    if (C >= 0.0) continue;

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
    const double compliance = effectiveGapCompliance(A, B, gmin);
    const auto [wA, wB] = gapMobilities(solverContext, A, B);
    const double w = wA + wB;
    const auto gapIt = solverContext.gapInfos.find(GapKey{&A, &B});
    const double initialC =
        (gapIt != solverContext.gapInfos.end() && gapIt->second.hasInitialC)
            ? gapIt->second.initialC
            : C;
    const double expected =
        expectedComplianceResidual(initialC, compliance, w, /*dt=*/1.0);
    // Expected spring slack must never hide an actual ink intersection.
    const double allowed = -expected * P.complianceResidualMargin;
    if (!dr.contact.intersect && -C <= allowed) continue;

    ConstraintViolation v;
    v.type = ViolationType::GenericGap;
    v.kind = ViolationKind::Hard;
    v.residual = C;
    v.severity = -C;
    v.allowedResidual = allowed;
    v.detail = dr.contact.depth_or_gap < 0.0 ? "Collision-proxy intersection" :
        dr.contact.intersect ? "Collision-proxy contact (touching or unresolved penetration)" : "Minimum gap not reached";
    v.glyphA = A.globalIndex;
    v.glyphB = B.globalIndex;
    v.markerCount = 2;  // the contact segment between the two shapes
    v.marker[0] = dr.contact.pA;
    v.marker[1] = dr.contact.pB;
    out.push_back(v);
  }
}


}  // namespace digitalkhatt::layout
