#pragma once

#include <optional>

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/XPBDConstraint.h"

namespace digitalkhatt::layout {

// Generalizes VerticalStackAlignConstraint: two marks stacked on the same
// side of the same base (e.g. shadda then fatha/damma above it) must keep a
// hard minimum gap in stacking order, in addition to the existing soft
// X-alignment. `inner` is the mark closer to the base, `outer` is further
// out; `isAbove` says whether the stack extends upward (top marks) or
// downward (bottom marks) from the base.
// If inner is dots, horizontal alignment moves only outer; the vertical
// minimum-gap constraint still uses both marks' mobilities.
// Outline clearance can be authored independently of horizontal alignment.
// A deformed outer mark preserving GPOS X also retains its outline clearance.
// An optional soft preferred gap moves only the outer mark; the hard minimum
// still uses both mobilities and runs afterward, so preference cannot erase it.
struct StackOrderConstraint : XPBDConstraint {
  GlyphInstance& inner;
  GlyphInstance& outer;
  bool isAbove;
  double minGap;
  double gapCompliance;
  double xAlignCompliance;
  double offsetX = 0.0;
  bool outlineClearance = false;
  std::optional<double> preferredGap;
  double preferredGapCompliance;
  // Lower stacks preserve dot placement and may have a separate contextual
  // X target (e.g. kasra beside the tip of the preceding Reh/Waw).
  bool keepInnerForGap = false;
  bool alignHorizontal = true;
  // A dot may occupy a neighboring bowl's free interior. Escape beneath it
  // only after a persistent ink contact, rather than from bbox overlap alone.
  bool onlyAfterInkContact = false;
  int inkContactIterations = 0;
  bool inkEscapeActive = false;

  double lambdaGap = 0.0;
  double lambdaX = 0.0;
  double lambdaPreferredGap = 0.0;

  StackOrderConstraint(GlyphInstance& inner_, GlyphInstance& outer_, bool isAbove_,
                       double minGap_, double gapCompliance_, double xAlignCompliance_,
                       double offsetX_ = 0.0, bool outlineClearance_ = false, std::optional<double> preferredGap_ = std::nullopt, double preferredGapCompliance_ = 0.1)
      : inner(inner_), outer(outer_), isAbove(isAbove_), minGap(minGap_), gapCompliance(gapCompliance_), xAlignCompliance(xAlignCompliance_), offsetX(offsetX_), outlineClearance(outlineClearance_), preferredGap(preferredGap_), preferredGapCompliance(preferredGapCompliance_) {
    reportEnabled = true;
  }

  void project(SolverContext& solverContext, double dt) override;
  void reportViolations(SolverContext& solverContext,
                        std::vector<ConstraintViolation>& out) const override;

 private:
  struct FacingEdges {
    bool overlap = true;
    geometry::Vec2 inner;
    geometry::Vec2 outer;
  };
  FacingEdges facingEdges() const;

  // Local geometry stays fixed during a solve. Vertical translations and a
  // common X translation need no new contour scan; relative X changes do.
  mutable bool outlineEdgesCached = false;
  mutable double outlineRelativeX = 0.0;
  mutable FacingEdges outlineEdges;
};

}  // namespace digitalkhatt::layout
