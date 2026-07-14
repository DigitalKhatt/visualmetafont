#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct VerticalStackAlignConstraint : XPBDConstraint {
  GlyphInstance& lowerMark;  // e.g. shadda
  GlyphInstance& upperMark;  // e.g. fatha / damma

  // Optional optical offset: x_lower - x_upper = offsetX
  double offsetX = 0.0;

  explicit VerticalStackAlignConstraint(GlyphInstance& lowerMark_,
                                        GlyphInstance& upperMark_,
                                        double compliance_,
                                        double offsetX_ = 0.0)
      : lowerMark(lowerMark_),
        upperMark(upperMark_),
        offsetX(offsetX_) {
    compliance = compliance_;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
