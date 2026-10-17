#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {
struct OptParams;

// A contextual preference, not a new mark owner. Other ink contacts remain
// active. Dots use a clear full-width left/above slot, otherwise below the tip.
// Kasra follows its same-owner dots, or the lower tip contour when alone.
struct RehWawMarkConstraint : XPBDConstraint {
  GlyphInstance& mark;
  GlyphInstance& reference;
  const OptParams& params;
  double targetX;
  double tipY;
  bool belowTip;
  bool dotUsesLeftSlot = false;
  bool dotTargetClear = false;
  double dotTargetTop = 0.0;
  double dotMinX = 0.0, dotMaxX = 0.0;
  double dotMinY = 0.0, dotMaxY = 0.0;
  std::vector<GlyphInstance*> dots;
  std::vector<RehWawMarkConstraint*> kasras;
  RehWawMarkConstraint* followedDotPlacement = nullptr;
  std::vector<GlyphInstance*> obstacles;
  double lambdaX = 0.0;
  double lambdaY = 0.0;

  RehWawMarkConstraint(GlyphInstance& mark, GlyphInstance& reference, const OptParams& params);
  void prepareDotTarget();
  void projectDotBounds();
  void projectKasraFollow();
  void project(SolverContext&, double dt) override;
  void reportViolations(SolverContext&, std::vector<ConstraintViolation>&) const override;

 private:
  bool hasInkClearance(const geometry::GeometrySet& shape) const;
  bool dotHasClearance(const geometry::GeometrySet& shape) const;
  double followingTop(const geometry::GeometrySet& dot, const geometry::GeometrySet& kasra) const;
  geometry::GeometrySet followingShape(const geometry::GeometrySet& dot) const;
  double targetTop() const;
};

struct RehWawDotBoundsConstraint : XPBDConstraint {
  RehWawMarkConstraint& placement;
  explicit RehWawDotBoundsConstraint(RehWawMarkConstraint& placement) : placement(placement) {}
  void project(SolverContext&, double) override { placement.projectDotBounds(); }
};

// Run after the dot's final position is chosen, before dot-kasra gap checks.
struct RehWawKasraFollowConstraint : XPBDConstraint {
  RehWawMarkConstraint& placement;
  explicit RehWawKasraFollowConstraint(RehWawMarkConstraint& placement) : placement(placement) {}
  void project(SolverContext&, double) override { placement.projectKasraFollow(); }
};

}  // namespace digitalkhatt::layout
