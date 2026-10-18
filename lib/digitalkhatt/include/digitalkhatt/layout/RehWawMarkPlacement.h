#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {
struct OptParams;

// Initial positioning only: plan from unchanged HarfBuzz geometry, then apply
// the complete page's plans before constructing ordinary XPBD constraints.
struct RehWawMarkPlacement {
  GlyphInstance& mark;
  GlyphInstance& reference;
  const OptParams& params;
  double targetX;
  double tipY;
  bool dotUsesCurrentGap = false;
  bool dotUsesLeftSlot = false;
  bool dotTargetClear = false;
  double dotTargetTop = 0.0;
  double dotMinX = 0.0, dotMaxX = 0.0;
  double dotMinY = 0.0, dotMaxY = 0.0;
  std::vector<GlyphInstance*> dots;
  std::vector<RehWawMarkPlacement*> kasras;
  RehWawMarkPlacement* followedDotPlacement = nullptr;
  std::vector<GlyphInstance*> obstacles;

  RehWawMarkPlacement(GlyphInstance&, GlyphInstance&, const OptParams&);
  void prepareDotTarget();
  geometry::GeometrySet preferredShape() const;
  bool hasInkClearance(const geometry::GeometrySet&) const;

 private:
  double neighboringGapPenalty(const geometry::GeometrySet&, const GlyphInstance&) const;
  bool dotHasClearance(const geometry::GeometrySet&) const;
  double followingTop(const geometry::GeometrySet&, const geometry::GeometrySet&) const;
  geometry::GeometrySet followingShape(const geometry::GeometrySet&) const;
};

}  // namespace digitalkhatt::layout
