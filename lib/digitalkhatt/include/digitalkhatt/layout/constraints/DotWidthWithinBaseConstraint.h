#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct DotWidthWithinBaseConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Separate lambdas for left and right inequalities
  double lambdaLeft = 0.0;
  double lambdaRight = 0.0;
  double compLeft;
  double compRight;

  DotWidthWithinBaseConstraint(GlyphInstance& mark, double compLeft, double compRight) : mark{mark} {
    this->compLeft = compLeft;
    this->compRight = compRight;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
