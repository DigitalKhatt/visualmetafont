#pragma once

#include <cstddef>
#include <functional>
#include <unordered_map>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

struct SolverContext;  // defined in SolverContext.h

double gapCompliance(const GlyphInstance& A, const GlyphInstance& B);

double chooseMinGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& P);

// The compliance actually applied to a pair's XPBD update (gapCompliance(A,B)
// normalized by minGap, exactly as solveGapConstraint computes it). Factored
// out so the post-solve violation report can predict a pair's expected
// compliant-equilibrium residual with the same formula the solver used.
double effectiveGapCompliance(const GlyphInstance& A, const GlyphInstance& B, double gmin);

// Closed-form equilibrium residual of a single pairwise XPBD inequality with
// constant gradient: the recursion C_{i+1} = C_i*alpha/(w+alpha) (alpha =
// compliance/dt^2) reaches its fixed point in one iteration, so
//   C_eq = C0 * alpha / (w + alpha)
// A soft (compliance > 0) constraint is a spring by design and is NOT meant
// to close fully -- this predicts exactly how much residual that spring
// should leave, given its own compliance and the participants' combined
// mobility. compliance == 0 (hard) correctly predicts C_eq == 0.
double expectedComplianceResidual(double initialC, double compliance, double w, double dt);

struct GapKey {
  GlyphInstance *a, *b;
  bool operator==(const GapKey& other) const {
    return ((a == other.a && b == other.b) || (a == other.b || b == other.a));
  }
};

struct GapInfo {
  double lambda = 0;
  double count = 0;
  // The first C measured for this pair (before any correction), i.e. C0 in
  // expectedComplianceResidual(). Snapshotted once, when the entry is
  // created; used by the post-solve report to tell an expected compliant
  // residual from a genuine unresolved conflict.
  double initialC = 0;
  bool hasInitialC = false;
};

struct GapKeyHasher {
  std::size_t operator()(const GapKey& k) const {
    std::size_t h1 = std::hash<GlyphInstance*>{}(k.a);
    std::size_t h2 = std::hash<GlyphInstance*>{}(k.b);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

using GapsInfo = std::unordered_map<GapKey, GapInfo, GapKeyHasher>;

void solveGapConstraint(SolverContext& solverContext,
                        GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& P,
                        double dt,
                        double& maxPenetration);

}  // namespace digitalkhatt::layout
