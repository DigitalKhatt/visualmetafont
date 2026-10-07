#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

namespace {
struct WaqfFloor {
  double minimum;
  double stackMinimum;
  const GlyphInstance* mark = nullptr;
  bool aboveStack = false;
  double topMinimum = -std::numeric_limits<double>::infinity();
};

WaqfFloor placementFloor(const WaqfPlacementConstraint& constraint,
                        const SolverContext& context, const GlyphInstance& base) {
  const auto& waqf = constraint.waqfMark;
  const auto bounds = waqf.worldPolys.boundingAABB();
  const double baseMinimum = std::max(boxTopY(base) + constraint.minGapToBase,
                                    base.lineY + constraint.minDistFromBaseline);
  WaqfFloor floor{baseMinimum, baseMinimum};
  const auto& line = context.pageGlyphs[base.lineIndex];
  const int end = waqf.nextBase ? waqf.nextBase->glyphIndex
                              : static_cast<int>(line.size());
  for (int index = base.glyphIndex + 1; index < end; ++index) {
    const auto& mark = line[index];
    if (&mark == &waqf || !mark.isMark || !mark.isTopMark || mark.prevBase != &base)
      continue;
    const auto markBounds = mark.worldPolys.boundingAABB();
    floor.topMinimum = std::max(floor.topMinimum,
                               markBounds.maxy - (bounds.maxy - bounds.miny));
    const double stackMinimum = markBounds.maxy + constraint.minGapToTopMarks;
    floor.stackMinimum = std::max(floor.stackMinimum, stackMinimum);
    const bool overlaps = bounds.minx < markBounds.maxx && bounds.maxx > markBounds.minx;
    // Above an overlapping mark, the waqf's bottom must clear its top.
    // Beside it, descent is allowed only while the waqf's top stays above its top.
    const double markMinimum = overlaps ? stackMinimum
                                        : markBounds.maxy - (bounds.maxy - bounds.miny);
    if (markMinimum > floor.minimum) {
      floor.minimum = markMinimum;
      floor.mark = &mark;
      floor.aboveStack = overlaps;
    }
  }
  return floor;
}

double heightTarget(const WaqfPlacementConstraint& constraint, const WaqfFloor& floor,
                    double maximumBottom) {
  if (floor.minimum > maximumBottom) return 0.5 * (floor.minimum + maximumBottom);
  // Beside the stack, prefer the waqf's top slightly above the highest mark's
  // top. Overlapping marks retain the stronger bottom-above-top floor.
  const double preferred = std::max(floor.minimum,
                                   floor.topMinimum + constraint.minGapToTopMarks);
  return std::clamp(preferred + constraint.desiredExtraLift, floor.minimum, maximumBottom);
}

double horizontalTarget(const WaqfPlacementConstraint& constraint,
                        SolverContext& context, GlyphInstance& base,
                        double stackMinimum, double maximumBottom) {
  if (constraint.enableBelowMarkFallbackX && stackMinimum > maximumBottom) {
    if (auto* mark = WaqfPlacementConstraint::chooseBelowMark(
            constraint.waqfMark, &base, context.pageGlyphs)) {
      const auto bounds = constraint.waqfMark.worldPolys.boundingAABB();
      return mark->worldPolys.boundingAABB().minx - (bounds.maxx - bounds.minx);
    }
  }
  return base.worldPolys.boundingAABB().minx;
}
}  // namespace

void WaqfTopOrderConstraint::project(SolverContext& solverContext, double) {
  auto& waqf = placement.waqfMark;
  if (waqf.mobility <= 0.0 || !waqf.prevBase) return;
  const auto floor = placementFloor(placement, solverContext, *waqf.prevBase);
  const double correction = floor.topMinimum - boxBottomY(waqf);
  if (correction <= 0.0) return;
  // Zero compliance, with only the waqf mobile: the XPBD weight cancels.
  waqf.dy += correction;
  updateWorldPolys(waqf);
}

GlyphInstance* WaqfPlacementConstraint::chooseBelowMark(
    GlyphInstance& waqfMark,
    GlyphInstance* base,
    std::vector<std::vector<GlyphInstance>>& pageGlyphs) {
  if (!base) return nullptr;

  GlyphInstance* best = nullptr;
  double bestScore = std::numeric_limits<double>::infinity();
  double xW = boxCenterX(waqfMark);

  auto& line = pageGlyphs[base->lineIndex];
  int startIndex = base->glyphIndex + 1;
  int endIndex = waqfMark.nextBase != nullptr ? waqfMark.nextBase->glyphIndex
                                              : static_cast<int>(line.size());

  for (int index = startIndex; index < endIndex; ++index) {
    GlyphInstance& gi = line[index];
    if (&gi == &waqfMark) continue;
    if (gi.lineIndex != base->lineIndex) continue;
    if (!gi.isMark) continue;
    if (!gi.isTopMark) continue;        // want above mark
    if (gi.prevBase != base) continue;  // same base only

    double dx = std::abs(boxCenterX(gi) - xW);
    if (dx < bestScore) {
      bestScore = dx;
      best = &gi;
    }
  }
  return best;
}

void WaqfPlacementConstraint::project(SolverContext& solverContext,
                                      double dt) {
  if (waqfMark.mobility <= 0.0) return;

  GlyphInstance* base = waqfMark.prevBase;
  if (!base) return;

  const double w = waqfMark.mobility;
  if (w <= 0.0) return;

  // ----------------------------------------------------------
  // 1) Compute yMin = minimum allowed waqf bottom
  // ----------------------------------------------------------
  const auto floor = placementFloor(*this, solverContext, *base);
  const double yMin = floor.minimum;
  if (lowerBoundMark != floor.mark || lowerBoundAboveStack != floor.aboveStack) {
    // A released stack floor must not retain its old upward multiplier.
    lambdaMin = 0.0;
    lambdaTarget = 0.0;
    lowerBoundMark = floor.mark;
    lowerBoundAboveStack = floor.aboveStack;
  }

  // Current waqf geometry
  double h = boxHeight(waqfMark);

  double yMax = base->lineY + upperCeilingY;

  // Convert top ceiling to bottom ceiling
  double yMaxBottom = yMax - h;

  // ----------------------------------------------------------
  // 2) Detect infeasibility / pressure
  // ----------------------------------------------------------
  bool infeasible = (yMin > yMaxBottom);

  // ----------------------------------------------------------
  // 3) Conflicting vertical bounds cannot retain their old multipliers
  // ----------------------------------------------------------
  if (infeasible) {
    lambdaMin = 0.0;
    lambdaMax = 0.0;
  }

  // ----------------------------------------------------------
  // 4) Hard lower bound: yMin - yBottom <= 0
  // ----------------------------------------------------------
  if (!infeasible) {
    double C = yMin - boxBottomY(waqfMark);
    if (C > 0.0) {
      double alpha = minCompliance / (dt * dt);
      double denom = w + alpha;
      if (denom > 1e-12) {
        double deltaLambda = -(C + alpha * lambdaMin) / denom;
        double lambdaNew = std::min(0.0, lambdaMin + deltaLambda);
        double applied = lambdaNew - lambdaMin;
        lambdaMin = lambdaNew;

        // grad dC/dy = -1
        waqfMark.dy += (-1.0) * w * applied;
        updateWorldPolys(waqfMark);
      }
    }
  }

  // ----------------------------------------------------------
  // 5) Hard upper bound: yTop - yMax <= 0
  // ----------------------------------------------------------
  if (!infeasible) {
    double C = boxTopY(waqfMark) - yMax;
    if (C > 0.0) {
      double alpha = maxCompliance / (dt * dt);
      double denom = w + alpha;
      if (denom > 1e-12) {
        double deltaLambda = -(C + alpha * lambdaMax) / denom;
        double lambdaNew = std::min(0.0, lambdaMax + deltaLambda);
        double applied = lambdaNew - lambdaMax;
        lambdaMax = lambdaNew;

        // grad dC/dy = +1
        waqfMark.dy += w * applied;
        updateWorldPolys(waqfMark);
      }
    }
  }

  // ----------------------------------------------------------
  // 6) Soft vertical target: yBottom - yTargetBottom = 0
  // ----------------------------------------------------------
  {
    const double bottom = boxBottomY(waqfMark);
    const double C = bottom - heightTarget(*this, floor, yMaxBottom);
    double alpha = targetCompliance / (dt * dt);
    double denom = w + alpha;
    if (denom > 1e-12) {
      double deltaLambda = -(C + alpha * lambdaTarget) / denom;
      lambdaTarget += deltaLambda;

      // grad dC/dy = +1
      waqfMark.dy += w * deltaLambda;
      updateWorldPolys(waqfMark);
    }
  }

  // ----------------------------------------------------------
  // 7) Dynamic horizontal target
  //
  // Prefer the left fallback while the full stack cannot fit above.
  // Otherwise retain the normal base alignment, allowing an above placement.
  // ----------------------------------------------------------
  const double xTarget = horizontalTarget(*this, solverContext, *base,
                                         floor.stackMinimum, yMaxBottom);

  // ----------------------------------------------------------
  // 8) Soft horizontal equality: xWaqf - xTarget = 0
  // ----------------------------------------------------------
  {
    double xW = waqfMark.worldPolys.boundingAABB().minx;
    double C = xW - xTarget;

    double alpha = xAlignCompliance / (dt * dt);
    double denom = w + alpha;
    if (denom > 1e-12) {
      double deltaLambda = -(C + alpha * lambdaX) / denom;
      lambdaX += deltaLambda;

      // grad dC/dx = +1
      waqfMark.dx += w * deltaLambda;
      updateWorldPolys(waqfMark);
    }
  }
}

void WaqfPlacementConstraint::reportViolations(
    SolverContext& solverContext, std::vector<ConstraintViolation>& out) const {
  GlyphInstance* base = waqfMark.prevBase;
  if (!base) return;

  const auto floor = placementFloor(*this, solverContext, *base);
  const double yMin = floor.minimum;

  const double yMax = base->lineY + upperCeilingY;
  const double yMaxBottom = yMax - boxHeight(waqfMark);
  const auto wb = waqfMark.worldPolys.boundingAABB();
  const double xc = 0.5 * (wb.minx + wb.maxx);

  if (yMin > yMaxBottom) {
    // Infeasible band: the hard lower and upper bounds conflict.
    ConstraintViolation v;
    v.type = ViolationType::WaqfPlacement;
    v.kind = ViolationKind::Hard;
    v.residual = yMin - yMaxBottom;
    v.severity = yMin - yMaxBottom;
    v.glyphA = waqfMark.globalIndex;
    v.glyphB = base->globalIndex;
    v.markerCount = 2;  // the conflicting band, drawn at the waqf's center x
    v.marker[0] = {xc, yMaxBottom};
    v.marker[1] = {xc, yMin};
    out.push_back(v);
    return;
  }

  auto pushBound = [&](double C, double railY) {
    if (C <= 0.0) return;
    ConstraintViolation v;
    v.type = ViolationType::WaqfPlacement;
    v.kind = ViolationKind::Hard;
    v.residual = C;
    v.severity = C;
    v.glyphA = waqfMark.globalIndex;
    v.glyphB = base->globalIndex;
    v.markerCount = 2;
    v.marker[0] = {wb.minx, railY};
    v.marker[1] = {wb.maxx, railY};
    out.push_back(v);
  };

  pushBound(yMin - boxBottomY(waqfMark), yMin);  // hard lower bound breach
  pushBound(boxTopY(waqfMark) - yMax, yMax);     // hard upper bound breach
  const double yTarget = heightTarget(*this, floor, yMaxBottom);
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqfMark, base,
      boxBottomY(waqfMark) - yTarget, "Waqf target height");
  reportSoftTarget(out, ViolationType::SoftTargetResidual, waqfMark, base,
      wb.minx - horizontalTarget(*this, solverContext, *base,
                                floor.stackMinimum, yMaxBottom),
      "Waqf horizontal alignment");
}

}  // namespace digitalkhatt::layout
