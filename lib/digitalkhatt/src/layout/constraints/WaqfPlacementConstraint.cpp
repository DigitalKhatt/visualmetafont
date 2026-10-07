#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>
#include <locale>
#include <sstream>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

namespace {
struct WaqfFloor {
  double minimum;
  double leftEdge; // leftmost ink edge of the base and its ordinary top marks
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
  WaqfFloor floor{baseMinimum, base.worldPolys.boundingAABB().minx};
  const auto& line = context.pageGlyphs[base.lineIndex];
  const int end = waqf.nextBase ? waqf.nextBase->glyphIndex
                              : static_cast<int>(line.size());
  for (int index = base.glyphIndex + 1; index < end; ++index) {
    const auto& mark = line[index];
    if (&mark == &waqf || !mark.isMark || !mark.isTopMark || mark.prevBase != &base)
      continue;
    const auto markBounds = mark.worldPolys.boundingAABB();
    // Waqf signs cannot serve as one another's alignment reference.
    if (!context.isWaqf(mark) && !mark.worldPolys.empty())
      floor.leftEdge = std::min(floor.leftEdge, markBounds.minx);
    floor.topMinimum = std::max(floor.topMinimum,
                               markBounds.maxy - (bounds.maxy - bounds.miny));
    const double stackMinimum = markBounds.maxy + constraint.minGapToTopMarks;
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

double horizontalBandResidual(const WaqfPlacementConstraint& constraint,
                              double leftEdge) {
  const auto bounds = constraint.waqfMark.worldPolys.boundingAABB();
  const double percent = std::isfinite(constraint.xAlignmentBandPercent)
      ? std::clamp(constraint.xAlignmentBandPercent, 0.0, 1000.0) : 0.0;
  const double band = (bounds.maxx - bounds.minx) * percent / 100.0;
  return bounds.minx - std::clamp(bounds.minx, leftEdge - band, leftEdge + band);
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
  // 7) Soft horizontal band around the leftmost edge of the base/top stack.
  // Vertical pressure does not switch to an entirely different placement.
  // ----------------------------------------------------------
  {
    const double C = horizontalBandResidual(*this, floor.leftEdge);
    if (C == 0.0) {
      lambdaX = 0.0; // inactive inside the band; release the restoring force
      return;
    }
    // Opposite sides are different unilateral constraints and cannot inherit
    // one another's multiplier after a contact pushes across the band.
    if (C * lambdaX > 0.0) lambdaX = 0.0;

    double alpha = xAlignCompliance / (dt * dt);
    double denom = w + alpha;
    if (denom > 1e-12) {
      double deltaLambda = -(C + alpha * lambdaX) / denom;
      const double nextLambda = C < 0.0 ? std::max(0.0, lambdaX + deltaLambda)
                                       : std::min(0.0, lambdaX + deltaLambda);
      deltaLambda = nextLambda - lambdaX;
      lambdaX = nextLambda;
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
    v.type = ViolationType::WaqfBoundsResidual;
    v.diagnostic = "infeasible-band";
    v.detail = "Waqf lower and upper height bounds conflict";
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
    v.type = ViolationType::WaqfBoundsResidual;
    v.diagnostic = railY == yMin ? "lower-bound" : "upper-bound";
    v.detail = railY == yMin ? "Waqf minimum-height residual" : "Waqf upper-ceiling residual";
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
      horizontalBandResidual(*this, floor.leftEdge), "Waqf horizontal alignment band");
}

void collectWaqfPlacementViolations(SolverContext& context, const OptParams& params,
                                   std::vector<ConstraintViolation>& out) {
  const auto percent = [](double value, double maximum) {
    return std::isfinite(value) ? std::clamp(value, 0.0, maximum) / 100.0 : 0.0;
  };
  for (const auto& line : context.pageGlyphs) {
    for (const auto& waqf : line) {
      if (!context.isWaqf(waqf) || waqf.worldPolys.empty()) continue;
      const auto* base = waqf.prevBase;
      // The shared factory already rejects invalid owners. Check membership
      // before dereferencing here, since reporting is independent of forces.
      if (!base || std::none_of(line.begin(), line.end(), [&](const auto& g) {
            return &g == base && !g.isMark && g.glyphName != "space" && !g.worldPolys.empty();
          })) continue;
      const auto wb = waqf.worldPolys.boundingAABB();
      const auto bb = base->worldPolys.boundingAABB();
      const double width = std::max(0.0, wb.maxx - wb.minx);
      WaqfPlacementMeasurements measurements;
      measurements.baseIndex = base->globalIndex;
      measurements.horizontalOffset = wb.minx - bb.minx;
      measurements.allowedLeftDrift = width * percent(params.waqfLeftDriftTolerancePercent, 1000.0);
      measurements.allowedRightDrift = width * percent(params.waqfRightDriftTolerancePercent, 1000.0);
      measurements.heightAboveBaseline = wb.maxy - base->lineY;

      const GlyphInstance* previousGlyph = nullptr;
      double closestDistance = std::numeric_limits<double>::infinity();
      // Empty surah-header rows contain no solver geometry. Use the nearest
      // preceding populated row and its actual baseline distance.
      for (int l = waqf.lineIndex - 1; l >= 0; --l) {
        const auto& previous = context.pageGlyphs[l];
        for (const auto& g : previous) {
          if (g.glyphName == "space" || g.worldPolys.empty() || g.lineY <= base->lineY) continue;
          const auto gb = g.worldPolys.boundingAABB();
          const double xGap = std::max({0.0, gb.minx - wb.maxx, wb.minx - gb.maxx});
          const double yGap = std::max({0.0, gb.miny - wb.maxy, wb.miny - gb.maxy});
          const double distance = std::hypot(xGap, yGap);
          if (distance < closestDistance) {
            closestDistance = distance;
            previousGlyph = &g;
          }
        }
        if (previousGlyph) break;
      }
      if (previousGlyph) {
        const double spacing = previousGlyph->lineY - base->lineY;
        measurements.previousBaselineDistance = previousGlyph->lineY - wb.maxy;
        measurements.previousLineMargin = spacing * percent(params.waqfPreviousLineMarginPercent, 100.0);
        measurements.previousInkBoxClearance = closestDistance;
      }

      const auto describe = [&](const char* reason) {
        std::ostringstream text;
        text.imbue(std::locale::classic());
        text << std::fixed << std::setprecision(2) << reason
             << "; horizontal offset " << measurements.horizontalOffset
             << " (negative = left); allowed left/right " << measurements.allowedLeftDrift
             << '/' << measurements.allowedRightDrift
             << "; top above own baseline " << measurements.heightAboveBaseline;
        if (previousGlyph)
          text << "; distance below previous baseline " << *measurements.previousBaselineDistance
               << "; required margin " << *measurements.previousLineMargin
               << "; nearest previous-line glyph " << previousGlyph->glyphName
               << "; ink-box clearance " << closestDistance;
        return text.str();
      };
      const double offset = measurements.horizontalOffset;
      const double allowance = offset < 0 ? measurements.allowedLeftDrift : measurements.allowedRightDrift;
      if (std::abs(offset) > allowance) {
        ConstraintViolation v;
        v.type = ViolationType::WaqfPlacement;
        v.kind = ViolationKind::Soft; // association risk to review, not a force residual
        v.diagnostic = "horizontal-drift";
        v.residual = offset;
        v.severity = std::abs(offset);
        v.allowedResidual = allowance;
        v.glyphA = waqf.globalIndex;
        v.glyphB = base->globalIndex;
        v.waqf = measurements;
        v.detail = describe(offset < 0 ? "Waqf too far left of its base" : "Waqf too far right of its base");
        v.markerCount = 2;
        v.marker[0] = {bb.minx, wb.miny};
        v.marker[1] = {wb.minx, wb.miny};
        out.push_back(std::move(v));
      }
      if (previousGlyph && *measurements.previousBaselineDistance < *measurements.previousLineMargin) {
        const double safeTop = previousGlyph->lineY - *measurements.previousLineMargin;
        ConstraintViolation v;
        v.type = ViolationType::WaqfPlacement;
        v.kind = ViolationKind::Soft;
        v.diagnostic = "previous-line-intrusion";
        v.residual = wb.maxy - safeTop;
        v.severity = measurements.heightAboveBaseline;
        v.allowedResidual = safeTop - base->lineY;
        v.glyphA = waqf.globalIndex;
        v.glyphB = previousGlyph->globalIndex;
        v.waqf = measurements;
        v.detail = describe("Waqf enters the previous line's baseline margin");
        v.markerCount = 2;
        v.marker[0] = {wb.minx, safeTop};
        v.marker[1] = {wb.maxx, safeTop};
        out.push_back(std::move(v));
      }
    }
  }
}

}  // namespace digitalkhatt::layout
