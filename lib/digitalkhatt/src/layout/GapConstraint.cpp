#include "digitalkhatt/layout/GapConstraint.h"

#include <algorithm>
#include <cmath>

#include "digitalkhatt/layout/SolverContext.h"

using namespace geometry;

namespace digitalkhatt::layout {

double gapCompliance(const GlyphInstance& A, const GlyphInstance& B) {
  return 0.1;
  if (!A.isMark && !B.isMark)
    return 0.0;  // hard for bases
  if (A.isMark && B.isMark)
    return 1e-5;
  return 1e-6;  // mark vs base
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

  const bool twoMarks = A.isMark && B.isMark;

  // You can bias here: e.g. move marks more than base letters.
  double wA = A.mobility;
  double wB = B.mobility;

  if (twoMarks) {
    if (solverContext.isWaqf(A) && !solverContext.isWaqf(B)) {
      wA = 3;
      wB = 1;
    } else if (!solverContext.isWaqf(A) && solverContext.isWaqf(B)) {
      wA = 1;
      wB = 3;
    }
  }

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

  auto it = solverContext.gapInfos.find(gapKey);
  const bool isNewEntry = (it == solverContext.gapInfos.end());
  auto& gapInfo = solverContext.gapInfos[gapKey];
  if (isNewEntry) {
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
    buildWorldPolys(A);
  }

  if (wB > 0.0) {
    Vec2 corrB = n * (wB * deltaLambda);
    B.dx += corrB.x;
    B.dy += corrB.y;
    buildWorldPolys(B);
  }
}

}  // namespace digitalkhatt::layout
