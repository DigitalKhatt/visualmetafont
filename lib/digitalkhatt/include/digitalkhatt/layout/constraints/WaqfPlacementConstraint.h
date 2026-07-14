#pragma once

#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct WaqfPlacementConstraint : XPBDConstraint {
  GlyphInstance& waqfMark;

  // ---------- Vertical parameters ----------
  double minCompliance = 0.0;     // hard lower bound
  double targetCompliance = 0.0;  // soft preferred height
  double maxCompliance = 0.0;     // hard upper bound
  double xAlignCompliance = 0.0;  // horizontal alignment

  double minDistFromBaseline = 0.0;
  double minGapToBase = 0.0;
  double minGapToTopMarks = 0.0;
  double desiredExtraLift = 0.0;

  // Upper bound on waqf TOP (usually from line above)
  double upperCeilingY = 0.0;

  // ---------- Horizontal switching policy ----------
  bool enableBelowMarkFallbackX = true;

  // If free vertical band is below this, blend x target toward below mark
  double tightBandThreshold = 80.0;

  // If waqf top is this close to upperCeilingY, blend x target toward below mark
  double upperPressureThreshold = 40.0;

  // ---------- Persistent XPBD state ----------
  double lambdaMin = 0.0;     // lower bound inequality
  double lambdaTarget = 0.0;  // target-height equality
  double lambdaMax = 0.0;     // upper bound inequality
  double lambdaX = 0.0;       // horizontal equality

  explicit WaqfPlacementConstraint(
      GlyphInstance& waqf,
      double minCompliance_,
      double targetCompliance_,
      double maxCompliance_,
      double xAlignCompliance_,
      double minDistFromBaseline_,
      double minGapToBase_,
      double minGapToTopMarks_,
      double desiredExtraLift_,
      double upperCeilingY_)
      : waqfMark(waqf),
        minCompliance(minCompliance_),
        targetCompliance(targetCompliance_),
        maxCompliance(maxCompliance_),
        xAlignCompliance(xAlignCompliance_),
        minDistFromBaseline(minDistFromBaseline_),
        minGapToBase(minGapToBase_),
        minGapToTopMarks(minGapToTopMarks_),
        desiredExtraLift(desiredExtraLift_),
        upperCeilingY(upperCeilingY_) {}

  // Choose a below mark belonging to the same base.
  // Preference: closest in x to the waqf.
  static GlyphInstance* chooseBelowMark(
      GlyphInstance& waqfMark,
      GlyphInstance* base,
      std::vector<std::vector<GlyphInstance>>& pageGlyphs);

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

}  // namespace digitalkhatt::layout
