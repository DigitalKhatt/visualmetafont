#pragma once

#include <unordered_map>
#include "OtLayout.h"
#include "digitalkhatt/layout/OptimizeLayout.h"

namespace digitalkhatt::layout {

// Owns diagnostic geometry. Moving this result preserves its base pointers.
struct PlacementPage {
  std::vector<std::vector<GlyphInstance>> glyphs;
  std::vector<ConstraintViolation> initialViolations;
  std::vector<ConstraintViolation> violations;
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
  std::unordered_map<const GlyphVis*, geometry::GeometrySet> geometry_;
};

}  // namespace digitalkhatt::layout
