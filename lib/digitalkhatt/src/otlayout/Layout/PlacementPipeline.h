#pragma once

#include <unordered_map>
#include <array>
#include "OtLayout.h"
#include "digitalkhatt/layout/OptimizeLayout.h"

namespace digitalkhatt::layout {

// Owns diagnostic geometry. Moving this result preserves its base pointers.
struct PlacementPage {
  std::vector<std::vector<GlyphInstance>> glyphs;
  std::vector<ConstraintViolation> initialViolations;
  std::vector<ConstraintViolation> violations;
  std::vector<geometry::GeometrySet> collisionGeometry;
  const geometry::GeometrySet& reportGeometry(const GlyphInstance& glyph) const {
    return collisionGeometry.empty() ? glyph.worldPolys : collisionGeometry[glyph.globalIndex];
  }
};

// Shared by the editor and native tools: identical outline scaling, base
// associations, solver defaults and offset conversion. No Qt dependencies.
class PlacementPipeline {
 public:
  explicit PlacementPipeline(OtLayout& layout, double emScale);
  PlacementPage solve(std::vector<LineLayoutInfo>& page, const OptParams& params,
                      bool force = true, bool report = false);
 private:
  OtLayout& layout_;
  double emScale_;
  // A caller can edit the collision-only class between solves.
  std::array<std::unordered_map<const GlyphVis*, geometry::GeometrySet>, 2> geometry_;
  std::unordered_map<const GlyphVis*, geometry::GeometrySet> collisionGeometry_;
  void collectCollisions(const std::vector<LineLayoutInfo>& page, PlacementPage& result,
      double minimumGap, std::vector<ConstraintViolation>& violations);
};

}  // namespace digitalkhatt::layout
