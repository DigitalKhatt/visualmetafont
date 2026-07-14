#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Soft, ONE-SIDED (up-only) vertical "lane": pushes the anchor mark's role-based
// vertical reference point UP toward a flat band at (baseline + laneHeight) when
// it sits below the band, but NEVER pulls it down. Pulling marks down drags them
// below where they belong (e.g. a hamza-above over alef dips below the letter)
// and hurts readability, so the downward branch is intentionally disabled.
// Intentionally soft: the near-rigid HardStayAboveConstraint rail and the hard
// StackOrderConstraint gap (both run later in the same solver iteration) always
// win when a real obstacle forces the mark higher, so the lane yields.
//
// One YlaneConstraint is created per base, on the anchor (bottom-most band
// participant of that base's above-group); stacked members ride above the
// anchor through StackOrderConstraint.
struct YlaneConstraint : XPBDConstraint {
  double laneHeight = 0.0;  // target band height above the baseline
  GlyphInstance& mark;      // the anchor mark this lane pins

  YlaneConstraint(GlyphInstance& mark, double comp, double laneHeight)
      : mark{mark} {
    compliance = comp;
    this->laneHeight = laneHeight;
    reportEnabled = false;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
