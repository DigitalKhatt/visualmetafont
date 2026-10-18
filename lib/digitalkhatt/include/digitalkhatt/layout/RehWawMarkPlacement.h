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
  enum class Slot { CurrentGap, Left, Below };
  Slot searchSlot = Slot::CurrentGap;
  // Slot state is shared by lower dots and standalone kasra. A linked kasra
  // follows its dot instead of selecting an independent slot.
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
  void prepareTarget();
  geometry::GeometrySet preferredShape() const;
  bool hasInkClearance(const geometry::GeometrySet&, bool includeMarks = true) const;

 private:
  double localBottomLimit() const;
  Slot activeSlot() const {
    return followedDotPlacement ? followedDotPlacement->searchSlot : searchSlot;
  }
  bool usesBelowFallback() const {
    return activeSlot() == Slot::Below;
  }
  double neighboringGapPenalty(const geometry::GeometrySet&, const GlyphInstance&) const;
  bool dotHasClearance(const geometry::GeometrySet&) const;
  double followingTop(const geometry::GeometrySet&, const geometry::GeometrySet&) const;
  geometry::GeometrySet followingShape(const geometry::GeometrySet&) const;
};

}  // namespace digitalkhatt::layout
