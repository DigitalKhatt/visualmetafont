#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

StackOrderConstraint::FacingEdges StackOrderConstraint::facingEdges() const {
  const double x = 0.5 * (boxCenterX(inner) + boxCenterX(outer));
  if ((!outlineClearance && !outer.preserveGposHorizontal) || inner.worldPolys.empty() || outer.worldPolys.empty()) {
    return {true, {x, isAbove ? boxTopY(inner) : boxBottomY(inner)}, {x, isAbove ? boxBottomY(outer) : boxTopY(outer)}};
  }
  const auto innerBox = inner.worldPolys.boundingAABB(), outerBox = outer.worldPolys.boundingAABB();
  if (innerBox.maxx < outerBox.minx || outerBox.maxx < innerBox.minx)
    return {false, {}, {}};

  const geometry::Vec2 innerOrigin{inner.baseX + inner.dx, inner.baseY + inner.dy};
  const geometry::Vec2 outerOrigin{outer.baseX + outer.dx, outer.baseY + outer.dy};
  const double relativeX = outerOrigin.x - innerOrigin.x;
  if (!outlineEdgesCached || relativeX != outlineRelativeX) {
    const auto& lower = isAbove ? inner.worldPolys : outer.worldPolys;
    const auto& upper = isAbove ? outer.worldPolys : inner.worldPolys;
    double closestGap = std::numeric_limits<double>::infinity();
    geometry::Vec2 lowerPoint, upperPoint;
    // Polygon edges are linear. Their signed vertical difference is linear on
    // every shared X interval, so its minimum occurs at an interval endpoint.
    // Checking all edge pairs gives the lower envelope of upper ink minus the
    // upper envelope of lower ink, including disconnected dots and concavity.
    auto yAt = [](geometry::Vec2 a, geometry::Vec2 b, double atX, bool top) {
      if (a.x == b.x) return top ? std::max(a.y, b.y) : std::min(a.y, b.y);
      return a.y + (atX - a.x) * (b.y - a.y) / (b.x - a.x);
    };
    for (const auto& lowerPoly : lower.polys()) {
      for (std::size_t i = 0; i < lowerPoly.size(); ++i) {
        const auto a = lowerPoly[i], b = lowerPoly[(i + 1) % lowerPoly.size()];
        const double lowerMinX = std::min(a.x, b.x), lowerMaxX = std::max(a.x, b.x);
        for (const auto& upperPoly : upper.polys()) {
          for (std::size_t j = 0; j < upperPoly.size(); ++j) {
            const auto c = upperPoly[j], d = upperPoly[(j + 1) % upperPoly.size()];
            const double left = std::max(lowerMinX, std::min(c.x, d.x));
            const double right = std::min(lowerMaxX, std::max(c.x, d.x));
            if (left > right) continue;
            for (const double atX : {left, right}) {
              const double lowerY = yAt(a, b, atX, true), upperY = yAt(c, d, atX, false);
              const double gap = upperY - lowerY;
              if (gap < closestGap) {
                closestGap = gap;
                lowerPoint = {atX, lowerY};
                upperPoint = {atX, upperY};
              }
            }
          }
        }
      }
    }
    outlineEdges = {std::isfinite(closestGap), (isAbove ? lowerPoint : upperPoint) - innerOrigin, (isAbove ? upperPoint : lowerPoint) - outerOrigin};
    outlineRelativeX = relativeX;
    outlineEdgesCached = true;
  }
  return {outlineEdges.overlap, outlineEdges.inner + innerOrigin, outlineEdges.outer + outerOrigin};
}

void StackOrderConstraint::project(SolverContext& solverContext, double dt) {
  if (onlyAfterInkContact && !inkEscapeActive) {
    // The contact proxy is flattened; protect curved ink and final renderer
    // rounding with twice the flattening tolerance before declaring it clear.
    constexpr double inkSafety = 2.0 * geometry::CUBIC_FLATNESS_TOLERANCE;
    if (geometry::getDistance(inner.worldPolys, outer.worldPolys, inkSafety).contact.depth_or_gap < inkSafety)
      ++inkContactIterations;
    else
      inkContactIterations = 0;
    if (inkContactIterations < 3) return;
    inkEscapeActive = true;
  }
  const double wI = keepInnerForGap ? 0.0 : inner.mobility;
  const double wO = outer.mobility;
  if (wI + wO <= 0.0) return;

  // Use the same contour metric for preference and safety. Unlike the hard
  // bound, this equality can close excessive gaps as well as open short ones.
  // Shadda remains the reference; other constraints may still move either mark.
  if (preferredGap && wO > 0.0) {
    const auto edges = facingEdges();
    if (edges.overlap) {
      const double target = std::max(minGap, *preferredGap);
      const double C = (isAbove ? edges.outer.y - edges.inner.y : edges.inner.y - edges.outer.y) - target;
      const double alpha = preferredGapCompliance / (dt * dt);
      const double denom = wO + alpha;
      if (denom >= 1e-9) {
        const double deltaLambda = -(C + alpha * lambdaPreferredGap) / denom;
        lambdaPreferredGap += deltaLambda;
        outer.dy += wO * (isAbove ? 1.0 : -1.0) * deltaLambda;
        updateWorldPolys(outer);
      }
    } else {
      lambdaPreferredGap = 0.0;
    }
  }

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
      const auto edges = facingEdges();
      if (edges.overlap)
        C = (isAbove ? edges.outer.y - edges.inner.y : edges.inner.y - edges.outer.y) - minGap;
      else
        C = 0.0;  // No shared X means no vertical stack collision to resolve.
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
  if (alignHorizontal) {
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
  if (onlyAfterInkContact && !inkEscapeActive) return;
  if (alignHorizontal)
    reportSoftTarget(out, ViolationType::SoftTargetResidual, outer, &inner,
                     boxCenterX(inner) - boxCenterX(outer) - offsetX, "Stack horizontal alignment");
  if (preferredGap) {
    const auto edges = facingEdges();
    if (edges.overlap) reportSoftTarget(out, ViolationType::SoftTargetResidual, outer, &inner, (isAbove ? edges.outer.y - edges.inner.y : edges.inner.y - edges.outer.y) - std::max(minGap, *preferredGap), "Stack preferred outline clearance");
  }
  const double C = isAbove
                       ? (boxBottomY(outer) - boxTopY(inner)) - minGap
                       : (boxBottomY(inner) - boxTopY(outer)) - minGap;  // violation when C < 0
  if (C >= 0.0) return;

  const auto edges = facingEdges();
  if (!edges.overlap) return;
  const double clearance = (isAbove ? edges.outer.y - edges.inner.y : edges.inner.y - edges.outer.y) - minGap;
  if (clearance >= 0.0) return;

  ConstraintViolation v;
  v.type = ViolationType::StackOrderGap;
  v.kind = ViolationKind::Hard;
  v.residual = clearance;
  v.severity = -clearance;
  v.glyphA = inner.globalIndex;
  v.glyphB = outer.globalIndex;
  // Mark the actual limiting contour edges for outline-clearance stacks.
  v.markerCount = 2;
  if (isAbove) {
    v.marker[0] = edges.inner;
    v.marker[1] = edges.outer;
  } else {
    v.marker[0] = edges.outer;
    v.marker[1] = edges.inner;
  }
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
