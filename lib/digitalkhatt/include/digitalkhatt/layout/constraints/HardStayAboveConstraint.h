#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct HardStayAboveConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Hard lower bounds
  double minGapToBase = 0.0;
  double minDistFromBaseline = 0.0;

  // If true, use the base top as the single lower bound; otherwise use the
  // baseline. Ignored when testBothReferences is true.
  bool useBaseTop = true;

  // If true, test BOTH references (base top and baseline) and treat the
  // constraint as satisfied if the mark clears EITHER bound (OR semantics).
  // Overrides useBaseTop when set.
  bool testBothReferences = false;

  explicit HardStayAboveConstraint(GlyphInstance& mark_,
                                   double compliance_,
                                   double minGapToBase_ = 0.0,
                                   double minDistFromBaseline_ = 0.0,
                                   bool useBaseTop_ = false,
                                   bool testBothReferences_ = false)
      : mark(mark_),
        minGapToBase(minGapToBase_),
        minDistFromBaseline(minDistFromBaseline_),
        useBaseTop(useBaseTop_),
        testBothReferences(testBothReferences_) {
    compliance = compliance_;
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
