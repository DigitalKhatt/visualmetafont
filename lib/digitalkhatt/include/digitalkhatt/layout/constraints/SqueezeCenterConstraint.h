#pragma once

#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Detects and resolves a mark squeezed horizontally between two base
// glyphs, with no glyph-name matching involved: every iteration it checks
// whether the mark's min-gap is simultaneously violated against an obstacle
// on its left and one on its right (out of the candidate obstacle set:
// typically the mark's own base plus the immediate previous/next base), and
// if both sides are live, nudges the mark toward the midpoint of the
// available room. The hard non-overlap bound against every candidate
// obstacle is enforced unconditionally, independent of the centering; this
// is what lets the caller exclude these (mark, obstacle) pairs from the
// generic broadphase gap resolution without losing correctness.
struct SqueezeCenterConstraint : XPBDConstraint {
  struct ObstacleState {
    GlyphInstance* obstacle = nullptr;
    double lambda = 0.0;  // hard per-obstacle min-gap inequality
  };

  GlyphInstance& mark;
  const OptParams& P;
  std::vector<ObstacleState> obstacles;
  double centerCompliance;
  double lambdaCenter = 0.0;  // soft midpoint-equality

  SqueezeCenterConstraint(GlyphInstance& mark_,
                          const OptParams& P_,
                          const std::vector<GlyphInstance*>& obstacles_,
                          double centerCompliance_ = 0.3);

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
