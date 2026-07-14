#pragma once

#include <vector>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Centers a cluster of marks trapped inside a bowl-shaped base (final/isolated
// Jeem/Hah/Khah) as a single rigid block. When several marks sit inside the
// bowl there is no room for the generic gap constraint to separate them without
// pushing them into overlap or into the bowl walls; the caller therefore
// excludes those marks' gap pairs (intra-cluster and mark-vs-base) from the
// generic broadphase pass and hands ownership to this constraint instead.
//
// The marks keep their OpenType-anchor-relative arrangement (no separating
// force acts within the cluster); this constraint only applies a single shared
// horizontal translation each iteration so the cluster's combined extent is
// centered on the bowl. Soft (spring), x-only -- vertical stays at the anchor.
struct BowlClusterConstraint : XPBDConstraint {
  GlyphInstance& base;
  std::vector<GlyphInstance*> marks;
  double interiorInset;  // reserved for an optional hard interior clamp

  BowlClusterConstraint(GlyphInstance& base_,
                        const std::vector<GlyphInstance*>& marks_,
                        double compliance_ = 0.3,
                        double interiorInset_ = 0.0)
      : base(base_), marks(marks_), interiorInset(interiorInset_) {
    compliance = compliance_;
  }

  void project(SolverContext& solverContext, double dt) override;
};

}  // namespace digitalkhatt::layout
