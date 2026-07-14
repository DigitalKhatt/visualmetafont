#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct MatchMarkPositionWithOffsetConstraint : XPBDConstraint {
  GlyphInstance& A;
  GlyphInstance& B;
  double offsetX = 0.0;
  double offsetY = 0.0;

  double lambdaX = 0.0;
  double lambdaY = 0.0;

  explicit MatchMarkPositionWithOffsetConstraint(GlyphInstance& a,
                                                 GlyphInstance& b,
                                                 double offsetX_,
                                                 double offsetY_,
                                                 double compliance_) : A(a), B(b), offsetX(offsetX_), offsetY(offsetY_) {
    compliance = compliance_;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
