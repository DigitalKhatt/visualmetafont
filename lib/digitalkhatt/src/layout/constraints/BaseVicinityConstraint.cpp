#include "digitalkhatt/layout/constraints/BaseVicinityConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void BaseVicinityConstraint::project(SolverContext& solverContext, double dt) {
  const double w = mark.mobility;
  if (w <= 0.0) return;

  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  const auto markBox = mark.worldPolys.boundingAABB();
  const auto baseBox = base->worldPolys.boundingAABB();

  const double xLeft = baseBox.minx - margin;
  const double xRight = baseBox.maxx + margin;

  // -------------------------
  // Left inequality: xLeft - markBox.minx <= 0, grad = -1
  // -------------------------
  {
    const double C = xLeft - markBox.minx;
    if (C > 0.0) {
      const double alpha = complianceLeft / (dt * dt);
      const double denom = w + alpha;
      if (denom >= 1e-12) {
        double deltaLambda = -(C + alpha * lambdaLeft) / denom;
        double lambdaNew = std::min(0.0, lambdaLeft + deltaLambda);
        double applied = lambdaNew - lambdaLeft;
        lambdaLeft = lambdaNew;

        mark.dx += (-1.0) * (w * applied);
        buildWorldPolys(mark);
      }
    } else {
      lambdaLeft = 0.0;
    }
  }

  // -------------------------
  // Right inequality: markBox.maxx - xRight <= 0, grad = +1
  // -------------------------
  {
    const double C = mark.worldPolys.boundingAABB().maxx - xRight;
    if (C > 0.0) {
      const double alpha = complianceRight / (dt * dt);
      const double denom = w + alpha;
      if (denom >= 1e-12) {
        double deltaLambda = -(C + alpha * lambdaRight) / denom;
        double lambdaNew = std::min(0.0, lambdaRight + deltaLambda);
        double applied = lambdaNew - lambdaRight;
        lambdaRight = lambdaNew;

        mark.dx += (+1.0) * (w * applied);
        buildWorldPolys(mark);
      }
    } else {
      lambdaRight = 0.0;
    }
  }
}

void BaseVicinityConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  GlyphInstance* base = mark.prevBase;
  if (!base) return;

  const auto markBox = mark.worldPolys.boundingAABB();
  const auto baseBox = base->worldPolys.boundingAABB();
  const double xLeft = baseBox.minx - margin;
  const double xRight = baseBox.maxx + margin;

  auto pushArm = [&](double C, double railX) {
    if (C <= 0.0) return;  // within the leash on this side
    ConstraintViolation v;
    v.type = ViolationType::BaseVicinity;
    v.kind = ViolationKind::Hard;
    v.residual = C;
    v.severity = C;
    v.glyphA = mark.globalIndex;
    v.glyphB = base->globalIndex;
    // Marker: vertical rail segment spanning the mark's y-extent at the breach.
    v.markerCount = 2;
    v.marker[0] = {railX, markBox.miny};
    v.marker[1] = {railX, markBox.maxy};
    out.push_back(v);
  };

  pushArm(xLeft - markBox.minx, xLeft);   // left overflow
  pushArm(markBox.maxx - xRight, xRight);  // right overflow
}

}  // namespace digitalkhatt::layout
