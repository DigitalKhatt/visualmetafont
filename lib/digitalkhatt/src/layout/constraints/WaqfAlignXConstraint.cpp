#include "digitalkhatt/layout/constraints/WaqfAlignXConstraint.h"

#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void WaqfAlignXConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  const double markX = mark.baseX + mark.dx;

  auto base = mark.prevBase;
  auto baseX = base->baseX + base->dx;

  // Equality constraint: C = x_mark - x_base = 0
  double C = markX - baseX;

  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-12) return;

  // Equality XPBD update: no clamp
  double deltaLambda = (-C - alpha * lambda) / denom;
  lambda += deltaLambda;

  // grad = +1 in x
  mark.dx += w * deltaLambda;
  buildWorldPolys(mark);
}

}  // namespace digitalkhatt::layout
