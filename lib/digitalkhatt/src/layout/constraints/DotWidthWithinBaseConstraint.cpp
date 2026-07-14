#include "digitalkhatt/layout/constraints/DotWidthWithinBaseConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void DotWidthWithinBaseConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  const double alphaLeft = compLeft / (dt * dt);
  const double denomLeft = w + alphaLeft;

  const double alphaRight = compRight / (dt * dt);
  const double denomRight = w + compRight;

  if (denomLeft < 1e-12 && denomRight < 1e-12) return;

  // Current dot extents
  const double x = mark.baseX + mark.dx;
  const double xMin = x + mark.metrics.bboxLlx;
  const double xMax = x + mark.metrics.bboxUrx;

  auto base = mark.prevBase;

  auto baseX = base->baseX + base->dx;

  auto baseLeft = baseX + base->metrics.bboxLlx;
  auto baseRight = baseX + base->metrics.bboxUrx;

  double xLeft = baseLeft - mark.metrics.width / 2;
  double xRight = baseRight + mark.metrics.width / 2;

  // -------------------------
  // Left inequality:
  // C_L = xLeft - xMin <= 0
  // grad = dC/dx = -1
  // -------------------------
  double CL = xLeft - xMin;
  if (CL > 0.0 && denomLeft >= 1e-12) {
    double deltaLambda =
        -(CL + alphaLeft * lambdaLeft) / denomLeft;

    double lambdaNew = std::min(0.0, lambdaLeft + deltaLambda);
    double applied = lambdaNew - lambdaLeft;
    lambdaLeft = lambdaNew;

    // grad = -1 in x
    mark.dx += (-1.0) * (w * applied);
    buildWorldPolys(mark);
  }

  // -------------------------
  // Right inequality:
  // C_R = xMax - xRight <= 0
  // grad = dC/dx = +1
  // -------------------------
  double CR = xMax - xRight;
  if (CR > 0.0 && denomRight >= 1e-12) {
    double deltaLambda =
        -(CR + alphaRight * lambdaRight) / denomRight;

    double lambdaNew = std::min(0.0, lambdaRight + deltaLambda);
    double applied = lambdaNew - lambdaRight;
    lambdaRight = lambdaNew;

    mark.dx += (+1.0) * (w * applied);
    buildWorldPolys(mark);
  }
}

}  // namespace digitalkhatt::layout
