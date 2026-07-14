#include "digitalkhatt/layout/constraints/ReturnToAnchorConstraint.h"

namespace digitalkhatt::layout {

void ReturnToAnchorConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  const double alpha = compliance / (dt * dt);
  const double denom = w + alpha;
  if (denom < 1e-12) return;

  // Equality: dx == 0
  {
    const double C = mark.dx;
    const double deltaLambda = (-C - alpha * lambdaX) / denom;
    lambdaX += deltaLambda;
    mark.dx += w * deltaLambda;
  }

  // Equality: dy == 0
  {
    const double C = mark.dy;
    const double deltaLambda = (-C - alpha * lambdaY) / denom;
    lambdaY += deltaLambda;
    mark.dy += w * deltaLambda;
  }

  buildWorldPolys(mark);
}

}  // namespace digitalkhatt::layout
