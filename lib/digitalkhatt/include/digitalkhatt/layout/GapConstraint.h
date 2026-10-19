#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

struct SolverContext;  // defined in SolverContext.h
struct ConstraintViolation;

// Per-solve contact history, shared across the pairs squeezing one lower mark.
// Only the ordinary gap pass writes it; reporting is read-only.
struct GapSqueezeState {
  int left = -1, right = -1;
  int previousLeft = -1, previousRight = -1;
  double leftGap = std::numeric_limits<double>::infinity();
  double rightGap = std::numeric_limits<double>::infinity();
  double previousGap = std::numeric_limits<double>::infinity();
  int stalledIterations = 0;
  bool active = false;
  double downwardBias = 0.0;
  double minimumTop = 0.0;
};

// Consume the previous pass's squeeze samples before solving this pass's pairs.
// This changes contact response, never glyph positioning by itself. Returns
// true while a stalled squeeze needs another sample before convergence.
bool beginGapPass(SolverContext& context,
    const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs, const OptParams& params,
    int iterationsRemaining);

std::pair<double, double> gapMobilities(const SolverContext& context,
    const GlyphInstance& A, const GlyphInstance& B);
void collectGapViolations(SolverContext& context,
    const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs,
    const OptParams& params, std::vector<ConstraintViolation>& out);

double gapCompliance(const GlyphInstance& A, const GlyphInstance& B);

double chooseMinGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& params);
// Cross-line ordinary clearance is capped at the inter-word target. Any
// larger line target is handled by a separate compliant spacing constraint.
double chooseOrdinaryGap(const GlyphInstance& A, const GlyphInstance& B, const OptParams& params);

// The compliance actually applied to a pair's XPBD update (gapCompliance(A,B)
// strengthened for gaps below 80; larger targets retain ordinary stiffness).
// Shared so the post-solve violation report can predict a pair's expected
// compliant-equilibrium residual with the same formula the solver used.
double effectiveGapCompliance(const GlyphInstance& A, const GlyphInstance& B, double gmin);
double effectiveGapCompliance(const SolverContext& context, const GlyphInstance& A,
    const GlyphInstance& B, const OptParams& params, double gmin);

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
    return (a == other.a && b == other.b) || (a == other.b && b == other.a);
  }
};

struct GapConstraintState {
  double lambda = 0;
  double count = 0;
  // The first C measured for this pair (before any correction), i.e. C0 in
  // expectedComplianceResidual(). Snapshotted once, when the entry is
  // created; used by the post-solve report to tell an expected compliant
  // residual from a genuine unresolved conflict.
  double initialC = 0;
  bool hasInitialC = false;
  bool escapingA = false, escapingB = false;
};

struct GapInfo {
  GapConstraintState ordinary;
  GapConstraintState extraLine;
};

struct GapKeyHasher {
  std::size_t operator()(const GapKey& k) const {
    std::size_t h1 = std::hash<GlyphInstance*>{}(k.a);
    std::size_t h2 = std::hash<GlyphInstance*>{}(k.b);

    // Equality accepts reversed pairs, so the hash must also be symmetric.
    return h1 ^ h2;
  }
};

using GapsInfo = std::unordered_map<GapKey, GapInfo, GapKeyHasher>;

void solveGapConstraint(SolverContext& solverContext,
                        GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& params,
                        double dt,
                        double& maxPenetration);

}  // namespace digitalkhatt::layout
