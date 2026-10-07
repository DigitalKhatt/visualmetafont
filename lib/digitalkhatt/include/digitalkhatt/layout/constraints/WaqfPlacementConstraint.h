#pragma once

#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct OptParams;
// Read-only association diagnostics, independent of constraint force switches.
void collectWaqfPlacementViolations(SolverContext& context, const OptParams& params,
                                   std::vector<ConstraintViolation>& out);

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

  // No alignment force inside this band around the leftmost ink edge of
  // the base and its attached top marks (excluding waqf signs).
  double xAlignmentBandPercent = 25.0;
  bool collisionEscapeActive = false; // bounded left/down preference owns alignment

  // ---------- Persistent XPBD state ----------
  double lambdaMin = 0.0;     // lower bound inequality
  double lambdaTarget = 0.0;  // target-height equality
  double lambdaMax = 0.0;     // upper bound inequality
  double lambdaX = 0.0;       // signed horizontal band multiplier
  const GlyphInstance* lowerBoundMark = nullptr;  // active above-stack floor
  bool lowerBoundAboveStack = false;

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
      double upperCeilingY_,
      double xAlignmentBandPercent_ = 25.0)
      : waqfMark(waqf),
        minCompliance(minCompliance_),
        targetCompliance(targetCompliance_),
        maxCompliance(maxCompliance_),
        xAlignCompliance(xAlignCompliance_),
        minDistFromBaseline(minDistFromBaseline_),
        minGapToBase(minGapToBase_),
        minGapToTopMarks(minGapToTopMarks_),
        desiredExtraLift(desiredExtraLift_),
        upperCeilingY(upperCeilingY_),
        xAlignmentBandPercent(xAlignmentBandPercent_) {
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  double escapeMinimumBottom(const SolverContext& context) const;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;
};

// Rigid top-edge ordering, projected after contacts alongside the other hard
// rails. Diagnostics are included in WaqfPlacementConstraint's shared floor.
struct WaqfTopOrderConstraint : XPBDConstraint {
  WaqfPlacementConstraint& placement;
  explicit WaqfTopOrderConstraint(WaqfPlacementConstraint& placement_)
      : placement(placement_) {}
  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
