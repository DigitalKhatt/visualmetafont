#pragma once

#include <vector>

namespace digitalkhatt::layout {

struct SolverContext;        // defined in SolverContext.h
struct ConstraintViolation;  // defined in ConstraintViolation.h

struct XPBDConstraint {
  double lambda = 0.0;      // warm-start state
  double compliance = 0.0;  // XPBD compliance

  // Per-instance opt-out from the post-solve report even when reportViolations
  // is implemented: the collection loop (OptimizeLayout.cpp) checks this
  // before calling reportViolations. Lets a constraint type keep a working
  // reportViolations() (so it's ready to go) while being excluded from the
  // report for now -- e.g. because it's currently considered a soft
  // preference rather than a hard bound. A future include/exclude parameter
  // (e.g. driven from OptParams/toggles) would set this per instance instead
  // of the hardcoded default a constructor picks today.
  bool reportEnabled = false;

  virtual ~XPBDConstraint() = default;
  virtual void project(SolverContext& solverContext, double dt) = 0;

  // Read-only, post-solve diagnostic: append any hard-feasibility violations
  // this constraint currently exhibits (recomputed from the final worldPolys).
  // Default no-op, so soft-only constraints and not-yet-covered types need no
  // override. MUST NOT mutate glyphs, lambda, or worldPolys. Only called by
  // the collector when reportEnabled is true.
  virtual void reportViolations(SolverContext& solverContext,
                                std::vector<ConstraintViolation>& out) const {}
};

}  // namespace digitalkhatt::layout
