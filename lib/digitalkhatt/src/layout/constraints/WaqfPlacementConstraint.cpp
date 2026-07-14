#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

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
  double yMin = boxTopY(*base) + minGapToBase;
  yMin = std::max(yMin, base->lineY + minDistFromBaseline);

  const auto& line = solverContext.pageGlyphs[base->lineIndex];
  int startIndex = base->glyphIndex + 1;
  int endIndex = waqfMark.nextBase != nullptr ? waqfMark.nextBase->glyphIndex
                                              : static_cast<int>(line.size());

  for (int index = startIndex; index < endIndex; ++index) {
    const GlyphInstance& gi = line[index];
    if (&gi == &waqfMark) continue;
    if (!gi.isMark) continue;
    if (!gi.isTopMark) continue;
    if (gi.prevBase != base) continue;

    yMin = std::max(yMin, boxTopY(gi) + minGapToTopMarks);
  }

  // Current waqf geometry
  double yBottom = boxBottomY(waqfMark);
  double yTop = boxTopY(waqfMark);
  double h = boxHeight(waqfMark);

  double yMax = base->lineY + upperCeilingY;

  // Convert top ceiling to bottom ceiling
  double yMaxBottom = yMax - h;

  // ----------------------------------------------------------
  // 2) Detect infeasibility / pressure
  // ----------------------------------------------------------
  bool infeasible = (yMin > yMaxBottom);
  double freeBand = yMaxBottom - yMin;
  bool tightBand = (freeBand < tightBandThreshold);
  bool upperPressure = ((yMax - yTop) < upperPressureThreshold);

  // ----------------------------------------------------------
  // 3) Choose vertical target
  // ----------------------------------------------------------
  double yTargetBottom = 0.0;

  if (!infeasible) {
    yTargetBottom = yMin + desiredExtraLift;
    yTargetBottom = std::min(std::max(yTargetBottom, yMin), yMaxBottom);
  } else {
    // midpoint compromise
    yTargetBottom = 0.5 * (yMin + yMaxBottom);

    // Hard bounds conflict; do not warm-start them
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
        buildWorldPolys(waqfMark);
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
        buildWorldPolys(waqfMark);
      }
    }
  }

  // ----------------------------------------------------------
  // 6) Soft vertical target: yBottom - yTargetBottom = 0
  // ----------------------------------------------------------
  {
    double C = boxBottomY(waqfMark) - yTargetBottom;
    double alpha = targetCompliance / (dt * dt);
    double denom = w + alpha;
    if (denom > 1e-12) {
      double deltaLambda = -(C + alpha * lambdaTarget) / denom;
      lambdaTarget += deltaLambda;

      // grad dC/dy = +1
      waqfMark.dy += w * deltaLambda;
      buildWorldPolys(waqfMark);
    }
  }

  // ----------------------------------------------------------
  // 7) Dynamic horizontal target
  //
  // Normal case: align with base
  // Tight/infeasible case: blend toward below-mark x
  // ----------------------------------------------------------
  double xBase = base->worldPolys.boundingAABB().minx;
  double xTarget = xBase;

  if (enableBelowMarkFallbackX && infeasible) {
    if (GlyphInstance* below = chooseBelowMark(waqfMark, base, solverContext.pageGlyphs)) {
      double xBelow = below->worldPolys.boundingAABB().minx;

      auto bbox = waqfMark.worldPolys.boundingAABB();

      xTarget = xBelow - (bbox.maxx - bbox.minx);
    }
  }

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
      buildWorldPolys(waqfMark);
    }
  }
}

void WaqfPlacementConstraint::reportViolations(
    SolverContext& solverContext, std::vector<ConstraintViolation>& out) const {
  GlyphInstance* base = waqfMark.prevBase;
  if (!base) return;

  // Recompute yMin (bumped up to clear base + baseline + same-base top marks),
  // mirroring project().
  double yMin = boxTopY(*base) + minGapToBase;
  yMin = std::max(yMin, base->lineY + minDistFromBaseline);

  const auto& line = solverContext.pageGlyphs[base->lineIndex];
  int startIndex = base->glyphIndex + 1;
  int endIndex = waqfMark.nextBase != nullptr ? waqfMark.nextBase->glyphIndex
                                              : static_cast<int>(line.size());
  for (int index = startIndex; index < endIndex; ++index) {
    const GlyphInstance& gi = line[index];
    if (&gi == &waqfMark) continue;
    if (!gi.isMark) continue;
    if (!gi.isTopMark) continue;
    if (gi.prevBase != base) continue;
    yMin = std::max(yMin, boxTopY(gi) + minGapToTopMarks);
  }

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
}

}  // namespace digitalkhatt::layout
