#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"

#include <algorithm>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void StackOrderConstraint::project(SolverContext& solverContext, double dt) {
  const double wI = inner.mobility;
  const double wO = outer.mobility;
  if (wI + wO <= 0.0) return;

  // ------------------------------------------------------------
  // Hard inequality: keep at least minGap between inner's outward-facing
  // edge and outer's inward-facing edge, whichever side of the base the
  // stack extends toward.
  // ------------------------------------------------------------
  {
    double C;
    double gradInner;  // dC/d(inner.dy)
    double gradOuter;  // dC/d(outer.dy)

    if (isAbove) {
      // inner below outer: outer's bottom must clear inner's top.
      C = (boxBottomY(outer) - boxTopY(inner)) - minGap;
      gradInner = -1.0;
      gradOuter = 1.0;
    } else {
      // inner above outer: inner's bottom must clear outer's top.
      C = (boxBottomY(inner) - boxTopY(outer)) - minGap;
      gradInner = 1.0;
      gradOuter = -1.0;
    }

    if (C < 0.0) {
      const double alpha = gapCompliance / (dt * dt);
      const double denom = wI + wO + alpha;
      if (denom >= 1e-9) {
        double deltaLambda = -(C + alpha * lambdaGap) / denom;
        double lambdaNew = std::max(0.0, lambdaGap + deltaLambda);
        deltaLambda = lambdaNew - lambdaGap;
        lambdaGap = lambdaNew;

        if (wI > 0.0) inner.dy += wI * gradInner * deltaLambda;
        if (wO > 0.0) outer.dy += wO * gradOuter * deltaLambda;
        updateWorldPolys(inner);
        updateWorldPolys(outer);
      }
    } else {
      lambdaGap = 0.0;
    }
  }

  // ------------------------------------------------------------
  // Soft equality: keep the two marks horizontally aligned (± offsetX).
  // Dots are the reference: only the outer mark follows their position.
  // Their actual mobility remains available to gap and other constraints.
  // ------------------------------------------------------------
  {
    const bool innerIsDots = solverContext.topdotmarks.contains(inner.glyphName) ||
                             solverContext.downdotmarks.contains(inner.glyphName);
    const double wAlignInner = innerIsDots ? 0.0 : wI;
    const double C = (boxCenterX(inner) - boxCenterX(outer)) - offsetX;

    const double alpha = xAlignCompliance / (dt * dt);
    const double denom = wAlignInner + wO + alpha;
    if (denom >= 1e-9) {
      const double deltaLambda = (-C - alpha * lambdaX) / denom;
      lambdaX += deltaLambda;

      if (wAlignInner > 0.0) {
        inner.dx += wAlignInner * deltaLambda;
        updateWorldPolys(inner);
      }
      if (wO > 0.0) {
        outer.dx -= wO * deltaLambda;
        updateWorldPolys(outer);
      }
    }
  }
}

void StackOrderConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  reportSoftTarget(out, ViolationType::SoftTargetResidual, outer, &inner,
      boxCenterX(inner) - boxCenterX(outer) - offsetX, "Stack horizontal alignment");
  const double C = isAbove
      ? (boxBottomY(outer) - boxTopY(inner)) - minGap
      : (boxBottomY(inner) - boxTopY(outer)) - minGap;  // violation when C < 0
  if (C >= 0.0) return;

  ConstraintViolation v;
  v.type = ViolationType::StackOrderGap;
  v.kind = ViolationKind::Hard;
  v.residual = C;
  v.severity = -C;
  v.glyphA = inner.globalIndex;
  v.glyphB = outer.globalIndex;
  // Marker: vertical segment between the two facing edges, at their mid-x.
  const double x = 0.5 * (boxCenterX(inner) + boxCenterX(outer));
  v.markerCount = 2;
  if (isAbove) {
    v.marker[0] = {x, boxTopY(inner)};
    v.marker[1] = {x, boxBottomY(outer)};
  } else {
    v.marker[0] = {x, boxTopY(outer)};
    v.marker[1] = {x, boxBottomY(inner)};
  }
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
