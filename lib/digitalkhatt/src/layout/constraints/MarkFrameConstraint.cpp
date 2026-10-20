#include "digitalkhatt/layout/constraints/MarkFrameConstraint.h"

#include <algorithm>
#include "digitalkhatt/layout/ConstraintViolation.h"

namespace digitalkhatt::layout {

void MarkFrameConstraint::project(SolverContext&, double) {
  if (mark.worldPolys.empty()) return;
  const auto fit = [](double value, double minimum, double maximum) {
    return minimum <= maximum ? std::clamp(value, minimum, maximum)
                              : 0.5 * (minimum + maximum);
  };
  const double x = mark.baseX + mark.dx, y = mark.baseY + mark.dy;
  mark.dx += fit(x, bounds.minx, bounds.maxx) - x;
  mark.dy += fit(y, bounds.miny, bounds.maxy) - y;
  updateWorldPolys(mark);
}

void MarkFrameConstraint::reportViolations(SolverContext&,
                                          std::vector<ConstraintViolation>& out) const {
  if (mark.worldPolys.empty()) return;
  const double x = mark.baseX + mark.dx, y = mark.baseY + mark.dy;
  const double overflow = std::max({bounds.minx - x, x - bounds.maxx,
                                     bounds.miny - y, y - bounds.maxy});
  if (overflow <= 1e-8) return;
  ConstraintViolation v;
  v.type = ViolationType::MarkFrame;
  v.residual = v.severity = overflow;
  v.glyphA = mark.globalIndex;
  v.diagnostic = "mark-frame";
  v.detail = bounds.minx > bounds.maxx || bounds.miny > bounds.maxy
      ? "Mark ink cannot fit inside the outer page border"
      : "Mark ink exceeds the outer page border or its rounding allowance";
  out.push_back(std::move(v));
}

}  // namespace digitalkhatt::layout
