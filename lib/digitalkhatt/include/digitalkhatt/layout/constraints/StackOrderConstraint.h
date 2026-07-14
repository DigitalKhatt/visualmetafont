#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Generalizes VerticalStackAlignConstraint: two marks stacked on the same
// side of the same base (e.g. shadda then fatha/damma above it) must keep a
// hard minimum gap in stacking order, in addition to the existing soft
// X-alignment. `inner` is the mark closer to the base, `outer` is further
// out; `isAbove` says whether the stack extends upward (top marks) or
// downward (bottom marks) from the base.
struct StackOrderConstraint : XPBDConstraint {
  GlyphInstance& inner;
  GlyphInstance& outer;
  bool isAbove;
  double minGap;
  double gapCompliance;
  double xAlignCompliance;
  double offsetX = 0.0;

  double lambdaGap = 0.0;
  double lambdaX = 0.0;

  StackOrderConstraint(GlyphInstance& inner_, GlyphInstance& outer_, bool isAbove_,
                       double minGap_, double gapCompliance_, double xAlignCompliance_,
                       double offsetX_ = 0.0)
      : inner(inner_), outer(outer_), isAbove(isAbove_), minGap(minGap_),
        gapCompliance(gapCompliance_), xAlignCompliance(xAlignCompliance_), offsetX(offsetX_) {}

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
