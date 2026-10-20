#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

inline geometry::AABB markFrameOriginBounds(const geometry::AABB& frame,
                                            const geometry::AABB& localInk,
                                            double xStep, double yStep,
                                            double borderHalfWidth = 0.0) {
  constexpr double horizontalInsetRatio = 0.005;  // 0.5% of page width.
  constexpr double verticalInsetRatio = 0.001;    // 0.1% of page height.
  const double xInset = borderHalfWidth + (frame.maxx - frame.minx) * horizontalInsetRatio;
  const double yInset = borderHalfWidth + (frame.maxy - frame.miny) * verticalInsetRatio;
  return {frame.minx + xInset - localInk.minx + xStep / 2.0,
          frame.miny + yInset - localInk.miny + yStep / 2.0,
          frame.maxx - xInset - localInk.maxx - xStep / 2.0,
          frame.maxy - yInset - localInk.maxy - yStep / 2.0};
}

// A rigid safety bound, projected after all contacts and placement constraints.
// Applies even to otherwise pinned/manual marks. Oversized ink is centered and
// reported as infeasible rather than passed to clamp with a reversed interval.
struct MarkFrameConstraint final : XPBDConstraint {
  GlyphInstance& mark;
  geometry::AABB bounds;

  MarkFrameConstraint(GlyphInstance& mark, const geometry::AABB& bounds)
      : mark(mark), bounds(bounds) { reportEnabled = true; }

  void project(SolverContext&, double) override;
  void reportViolations(SolverContext&, std::vector<ConstraintViolation>&) const override;
};

}  // namespace digitalkhatt::layout
