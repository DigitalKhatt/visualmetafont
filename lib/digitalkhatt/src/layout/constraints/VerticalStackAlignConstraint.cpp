#include "digitalkhatt/layout/constraints/VerticalStackAlignConstraint.h"

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void VerticalStackAlignConstraint::project(SolverContext& solverContext, double dt) {
  const double wL = lowerMark.mobility;
  const double wU = upperMark.mobility;
  if (wL + wU <= 0.0) return;

  // Equality: x_lower - x_upper - offsetX = 0
  const double C = (boxCenterX(lowerMark) - boxCenterX(upperMark)) - offsetX;

  const double alpha = compliance / (dt * dt);
  const double denom = wL + wU + alpha;
  if (denom < 1e-12) return;

  const double deltaLambda = (-C - alpha * lambda) / denom;
  lambda += deltaLambda;

  // grad wrt lowerMark.x = +1
  // grad wrt upperMark.x = -1
  if (wL > 0.0) {
    lowerMark.dx += wL * deltaLambda;
    buildWorldPolys(lowerMark);
  }

  if (wU > 0.0) {
    upperMark.dx -= wU * deltaLambda;
    buildWorldPolys(upperMark);
  }
}

}  // namespace digitalkhatt::layout
