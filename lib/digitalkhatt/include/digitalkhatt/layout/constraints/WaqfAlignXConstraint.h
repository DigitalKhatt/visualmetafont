#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct WaqfAlignXConstraint : XPBDConstraint {
  GlyphInstance& mark;

  WaqfAlignXConstraint(GlyphInstance& mark, double comp) : mark{mark} {
    compliance = comp;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
