#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Soft pull back toward the OpenType-computed anchor position (dx=0, dy=0).
// Used instead of a hard stay-above/below rail for marks whose base glyph
// already has a dedicated GPOS anchor for this mark that the solver should
// trust rather than override (e.g. a below-mark that belongs inside the
// open bowl of a final/isolated Jeem/Hah/Khah, not under its bounding box).
struct ReturnToAnchorConstraint : XPBDConstraint {
  GlyphInstance& mark;

  double lambdaX = 0.0;
  double lambdaY = 0.0;

  explicit ReturnToAnchorConstraint(GlyphInstance& mark_, double compliance_) : mark(mark_) {
    compliance = compliance_;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
