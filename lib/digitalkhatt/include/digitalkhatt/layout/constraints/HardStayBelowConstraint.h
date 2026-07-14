#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct HardStayBelowConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Hard upper bounds
  double maxGapToBase = 0.0;       // distance BELOW base bottom
  double maxDistToBaseline = 0.0;  // distance BELOW baseline

  // If true, use the base bottom as the single upper bound; otherwise use the
  // baseline. Ignored when testBothReferences is true.
  bool useBaseBottom = false;

  // If true, test BOTH references (base bottom and baseline) and treat the
  // constraint as satisfied if the mark clears EITHER bound (OR semantics).
  // Overrides useBaseBottom when set.
  bool testBothReferences = false;

  // Fraction of the mark's bbox height that must sit at/below the baseline
  // bound (the baseline arm only; the base-bottom arm is unaffected). Scales
  // with the mark's own height instead of an absolute tolerance: 1.0 = whole
  // mark below (the old behavior), 0.66 = at least two thirds below.
  double belowBaselineFraction = 0.66;

  explicit HardStayBelowConstraint(GlyphInstance& mark_,
                                   double compliance_,
                                   double maxGapToBase_ = 0.0,
                                   double maxDistToBaseline_ = 0.0,
                                   bool useBaseBottom_ = false,
                                   bool testBothReferences_ = false,
                                   double belowBaselineFraction_ = 0.66)
      : mark(mark_),
        maxGapToBase(maxGapToBase_),
        maxDistToBaseline(maxDistToBaseline_),
        useBaseBottom(useBaseBottom_),
        testBothReferences(testBothReferences_),
        belowBaselineFraction(belowBaselineFraction_) {
    compliance = compliance_;
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
