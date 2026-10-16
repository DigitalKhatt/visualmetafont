#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Keep an attached mark near its GPOS parent without using their convex hulls
// as ink. Both marks share corrections according to their mobility; other
// collisions remain active.
struct CompactMarkConstraint : XPBDConstraint {
  GlyphInstance& parent;
  GlyphInstance& child;
  double minGap;
  geometry::Vec2 offset;
  geometry::GeometrySet parentParts, childParts;
  double lambdaX = 0, lambdaY = 0;

  CompactMarkConstraint(GlyphInstance& parent_, GlyphInstance& child_, double minGap_, double compliance_);
  void project(SolverContext& context, double dt) override;
  void reportViolations(SolverContext&, std::vector<ConstraintViolation>& out) const override;

 private:
  geometry::GSContact contact() const;
};

}  // namespace digitalkhatt::layout
