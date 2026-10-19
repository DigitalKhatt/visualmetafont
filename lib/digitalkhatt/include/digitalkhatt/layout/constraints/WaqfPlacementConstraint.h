#pragma once

#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {
struct OptParams;
void collectWaqfPlacementViolations(SolverContext& context, const OptParams& params,
                                   std::vector<ConstraintViolation>& out);

// Safety intervals only. Initial search chooses the position; no preferred
// height or horizontal alignment target is projected during ordinary XPBD.
struct WaqfPlacementConstraint : XPBDConstraint {
  GlyphInstance& waqfMark;
  double minCompliance;
  double maxCompliance;
  double minDistFromBaseline;
  double minGapToBase;
  double minGapToTopMarks;
  double upperCeilingY;
  double maxLeftPercent = 100.0;
  double maxRightPercent = 25.0;
  double leftCompliance = 0.1;
  double rightCompliance = 0.1;
  bool collisionEscapeActive = false;
  bool hasInitialSlot = false;

  double lambdaMin = 0.0;
  double lambdaMax = 0.0;
  double lambdaAssociation = 0.0;
  const GlyphInstance* lowerBoundMark = nullptr;
  bool lowerBoundAboveStack = false;

  WaqfPlacementConstraint(GlyphInstance& waqf, double minCompliance_, double maxCompliance_,
      double minDistFromBaseline_, double minGapToBase_, double minGapToTopMarks_, double upperCeilingY_)
      : waqfMark(waqf), minCompliance(minCompliance_), maxCompliance(maxCompliance_),
        minDistFromBaseline(minDistFromBaseline_), minGapToBase(minGapToBase_),
        minGapToTopMarks(minGapToTopMarks_), upperCeilingY(upperCeilingY_) {
    reportEnabled = true;
  }
  void project(SolverContext& context, double dt) override;
  double escapeMinimumBottom(const SolverContext& context) const;
  void reportViolations(SolverContext& context, std::vector<ConstraintViolation>& out) const override;
};

// Projected after ordinary contacts to preserve owning-stack visibility/order.
struct WaqfTopOrderConstraint : XPBDConstraint {
  WaqfPlacementConstraint& placement;
  explicit WaqfTopOrderConstraint(WaqfPlacementConstraint& value) : placement(value) {}
  void project(SolverContext& context, double dt) override;
};
}
