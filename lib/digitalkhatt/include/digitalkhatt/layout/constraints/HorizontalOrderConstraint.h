#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct HorizontalOrderConstraint : XPBDConstraint {
  GlyphInstance& markA;  // belongs to base glyph
  GlyphInstance& markB;  // belongs to next glyph
  // Anti-misassociation thresholds, measured on the ink box.
  double selfKeep;   // fraction of a mark's OWN ink that must stay exposed
  double crossKeep;  // exposed protrusion must exceed this fraction of the NEIGHBOR's ink

  HorizontalOrderConstraint(
      GlyphInstance& markA, GlyphInstance& markB,
      double selfKeep, double crossKeep,
      double comp) : markA{markA}, markB{markB} {
    this->selfKeep = selfKeep;
    this->crossKeep = crossKeep;
    compliance = comp;
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
