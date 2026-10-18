#pragma once

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// A directional edge-to-edge minimum; vertical positioning remains free.
struct HorizontalGapConstraint : XPBDConstraint {
  GlyphInstance& right;
  GlyphInstance& left;
  double minGap;

  HorizontalGapConstraint(GlyphInstance& right, GlyphInstance& left, double gap)
      : right(right), left(left), minGap(gap) { reportEnabled = true; }

  double residual() const {
    return right.worldPolys.boundingAABB().minx -
           left.worldPolys.boundingAABB().maxx - minGap;
  }
  void project(SolverContext&, double dt) override {
    const double c = residual();
    if (c >= 0.0) { lambda = 0.0; return; }
    const double w = right.mobility + left.mobility;
    if (w <= 0.0) return;
    const double alpha = compliance / (dt * dt);
    const double correction = -(c + alpha * lambda) / (w + alpha);
    const double next = std::max(0.0, lambda + correction);
    const double applied = next - lambda;
    lambda = next;
    right.dx += right.mobility * applied;
    left.dx -= left.mobility * applied;
    updateWorldPolys(right);
    updateWorldPolys(left);
  }
  void reportViolations(SolverContext&, std::vector<ConstraintViolation>& out) const override {
    const double c = residual();
    if (c >= 0.0) return;
    ConstraintViolation v;
    v.type = ViolationType::HorizontalOrder;
    v.kind = ViolationKind::Hard;
    v.residual = c;
    v.severity = -c;
    v.glyphA = right.globalIndex;
    v.glyphB = left.globalIndex;
    v.detail = "Minimum horizontal outline-edge clearance";
    v.markerCount = 2;
    v.marker[0] = {boxCenterX(right), boxCenterY(right)};
    v.marker[1] = {boxCenterX(left), boxCenterY(left)};
    out.push_back(std::move(v));
  }
};

}  // namespace digitalkhatt::layout
