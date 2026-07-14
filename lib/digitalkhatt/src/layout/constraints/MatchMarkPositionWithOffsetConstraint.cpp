#include "digitalkhatt/layout/constraints/MatchMarkPositionWithOffsetConstraint.h"

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void MatchMarkPositionWithOffsetConstraint::project(SolverContext& solverContext,
                                                    double dt) {
  const double wA = A.mobility;
  const double wB = B.mobility;
  if (wA + wB <= 0.0) return;

  const double alpha = compliance / (dt * dt);
  const double denom = wA + wB + alpha;
  if (denom < 1e-12) return;

  {
    double C = (boxCenterX(A) - boxCenterX(B)) - offsetX;
    double deltaLambda = (-C - alpha * lambdaX) / denom;
    lambdaX += deltaLambda;

    if (wA > 0.0) A.dx += wA * deltaLambda;
    if (wB > 0.0) B.dx -= wB * deltaLambda;

    buildWorldPolys(A);
    buildWorldPolys(B);
  }

  {
    double C = (boxCenterY(A) - boxCenterY(B)) - offsetY;
    double deltaLambda = (-C - alpha * lambdaY) / denom;
    lambdaY += deltaLambda;

    if (wA > 0.0) A.dy += wA * deltaLambda;
    if (wB > 0.0) B.dy -= wB * deltaLambda;

    buildWorldPolys(A);
    buildWorldPolys(B);
  }
}

}  // namespace digitalkhatt::layout
