#pragma once

#include <limits>
#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

struct OptParams;
struct WaqfPlacementConstraint;

// A contact sampled by the existing gap pass. Separation is positive along
// normal, pointing from the obstacle toward the waqf.
struct WaqfEscapeContact {
  const GlyphInstance* obstacle;
  geometry::Vec2 normal;
  geometry::Vec2 relativeShift;
  double gap;
  double desiredGap;
};

// Conditional left/down preferences triggered by persistent upper contacts.
// Ordinary contacts and the owning placement's visibility/top rail remain active.
struct WaqfEscapeConstraint : XPBDConstraint {
  WaqfPlacementConstraint& placement;
  const OptParams& params;
  std::vector<WaqfEscapeContact> contacts;
  double previousGap = std::numeric_limits<double>::infinity();
  double targetOffset = 0.0; // relative to the owning base's ink left edge
  double targetBottomOffset = 0.0; // relative to the owning baseline
  double lambdaDown = 0.0;
  int stalledIterations = 0;
  bool active = false;

  WaqfEscapeConstraint(WaqfPlacementConstraint& placement_, const OptParams& params_);
  void recordContact(const GlyphInstance& obstacle, geometry::Vec2 normal,
                     double gap, double desiredGap);
  void project(SolverContext& context, double dt) override;
  void reportViolations(SolverContext& context,
                        std::vector<ConstraintViolation>& out) const override;
};

} // namespace digitalkhatt::layout
