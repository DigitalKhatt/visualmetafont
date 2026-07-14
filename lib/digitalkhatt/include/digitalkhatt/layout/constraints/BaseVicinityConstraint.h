#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Generalizes DotWidthWithinBaseConstraint: keeps a mark's horizontal extent
// within its own base glyph's footprint plus a margin, regardless of mark
// role. Prevents collision pressure from walking a position-sensitive mark
// (dots especially, but any mark under heavy squeeze) past its own base far
// enough that it visually reads as belonging to a neighboring glyph.
struct BaseVicinityConstraint : XPBDConstraint {
  GlyphInstance& mark;
  double margin;
  double complianceLeft;
  double complianceRight;

  double lambdaLeft = 0.0;
  double lambdaRight = 0.0;

  BaseVicinityConstraint(GlyphInstance& mark_, double margin_,
                         double complianceLeft_, double complianceRight_)
      : mark(mark_), margin(margin_), complianceLeft(complianceLeft_), complianceRight(complianceRight_) {
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
