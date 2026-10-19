#include "digitalkhatt/layout/WaqfMarkPlacement.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/CollisionReport.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

namespace digitalkhatt::layout {
namespace {
double boxDistance(const geometry::AABB& a, const geometry::AABB& b) {
  return std::hypot(std::max({0.0, a.minx - b.maxx, b.minx - a.maxx}),
                    std::max({0.0, a.miny - b.maxy, b.miny - a.maxy}));
}

struct Plan {
  WaqfPlacementConstraint* placement;
  geometry::GeometrySet original;
  geometry::Vec2 shift;
  bool accepted = false;
};

// In a crowded slot, prefer the least unmet ordinary clearance before height
// and alignment. This keeps spacing separate from the softer line-gap target.
using Score = std::pair<double, double>;
const Score unavailable{std::numeric_limits<double>::infinity(),
                        std::numeric_limits<double>::infinity()};

Plan search(WaqfPlacementConstraint& placement, SolverContext& context,
            const OptParams& params, bool crowded = false) {
  auto& waqf = placement.waqfMark;
  Plan plan{&placement, waqf.worldPolys, {}};
  if (!waqf.prevBase || waqf.mobility <= 0.0 || waqf.worldPolys.empty()) return plan;
  const auto& base = *waqf.prevBase;
  const auto bounds = waqf.worldPolys.boundingAABB();
  const double width = bounds.maxx - bounds.minx, height = bounds.maxy - bounds.miny;
  if (width <= 0.0 || height <= 0.0) return plan;
  const auto bb = base.worldPolys.boundingAABB();
  const double left = bb.minx - width * std::clamp(params.waqfEscapeMaxLeftPercent, 0.0, 1000.0) / 100.0;
  const double right = bb.minx + width * std::clamp(placement.maxRightPercent, 0.0, 1000.0) / 100.0;
  double bottom = std::max(bb.maxy + placement.minGapToBase,
                           base.lineY + placement.minDistFromBaseline);
  double top = base.lineY + placement.upperCeilingY;
  double stackTopMinimum = -std::numeric_limits<double>::infinity();
  double preferredLeft = bb.minx;
  // Preserve the owning stack's top ordering even in a slot beside it.
  for (const auto& mark : context.pageGlyphs[waqf.lineIndex])
    if (&mark != &waqf && mark.prevBase == &base && mark.isMark && mark.isTopMark &&
        !context.isWaqf(mark) && !mark.worldPolys.empty()) {
      const auto markBounds = mark.worldPolys.boundingAABB();
      stackTopMinimum = std::max(stackTopMinimum, markBounds.maxy - height);
      preferredLeft = std::min(preferredLeft, markBounds.minx);
    }
  bottom = std::max(bottom, stackTopMinimum);
  const double maxBottom = top - height;
  if (right < left || maxBottom < bottom) return plan;
  // The removed placement preferences now belong to this one initial search.
  // Align with an owning top mark extending left of the base, otherwise the
  // base itself. Prefer a top just above the owning top stack,
  // subject to the existing base/baseline floor and upper safety ceiling.
  const geometry::Vec2 preferred{
      std::clamp(preferredLeft, left, right),
      std::clamp(std::max(bottom, stackTopMinimum + placement.minGapToTopMarks),
                         bottom, maxBottom)};

  // Only nearby ink can intersect any candidate or contribute a spacing cost.
  const geometry::AABB envelope{std::min(left, bounds.minx), std::min(bottom, bounds.miny),
                                std::max(right + width, bounds.maxx), std::max(top, bounds.maxy)};
  struct Obstacle {
    const geometry::GeometrySet* ink;
    double ordinary, preferred;
  };
  std::vector<Obstacle> obstacles;
  for (const auto& line : context.pageGlyphs) for (const auto& other : line) {
    if (&other == &waqf || other.worldPolys.empty() || isCollisionSpace(other.glyphName)) continue;
    const double target = chooseMinGap(waqf, other, params);
    if (boxDistance(envelope, other.worldPolys.boundingAABB()) <= target) {
      const double ordinary = std::max(0.0, chooseOrdinaryGap(waqf, other, params));
      obstacles.push_back({&other.worldPolys, ordinary, std::max(ordinary, target)});
    }
  }
  std::vector<geometry::AABB> otherWordBounds;
  if (base.wordIndex >= 0) {
    for (const auto& other : context.pageGlyphs[waqf.lineIndex]) {
      if (other.isMark || other.wordIndex < 0 || other.wordIndex == base.wordIndex ||
          other.worldPolys.empty()) continue;
      const auto ob = other.worldPolys.boundingAABB();
      if (ob.maxx >= left + width / 2.0 && ob.minx <= right + width / 2.0)
        otherWordBounds.push_back(ob);
    }
  }
  const auto evaluate = [&](const geometry::GeometrySet& shape) {
    double comfort = 0.0, shortfall = 0.0;
    for (const auto& other : obstacles) {
      const auto [ink, ordinary, preferred] = other;
      const auto contact = geometry::getDistance(shape, *ink, preferred).contact;
      // First try full clearance. If that is impossible, still require clear
      // ink with up to ten units of margin instead of retaining a colliding
      // authored anchor. Ordinary XPBD contacts then refine the missing gap.
      const double required = crowded ? std::min(10.0, ordinary) : ordinary;
      if (contact.intersect || contact.depth_or_gap < required - 1e-6)
        return unavailable;
      if (std::isfinite(contact.depth_or_gap)) {
        const double missing = std::max(0.0, ordinary - contact.depth_or_gap);
        if (crowded) shortfall += missing * missing;
        const double deficit = std::max(0.0, preferred - contact.depth_or_gap);
        comfort += deficit * deficit;
      }
    }
    return Score{shortfall, comfort};
  };
  const double comfortWeight = params.compliance.extraLineSpacing > 0.0
      ? std::min(1.0, 0.1 / params.compliance.extraLineSpacing) : 1.0;
  Score best = unavailable;
  geometry::Vec2 selected;
  const auto consider = [&](double x, double y, bool needScore = false) -> Score {
    if (x < left || x > right || y < bottom || y > maxBottom)
      return unavailable;
    const geometry::Vec2 shift{x - bounds.minx, y - bounds.miny};
    const geometry::Vec2 deviation{x - preferred.x, y - preferred.y};
    // Clearance and comfort costs cannot be negative. Skip a candidate that
    // cannot improve the selected slot even with both costs zero. Axis probes
    // still need their exact scores to preserve their own refinement paths.
    if (!needScore && Score{0.0, deviation.lengthSq() + 1e-6 * shift.lengthSq()} >= best)
      return unavailable;
    // A new slot must not move the sign over a different word's base.
    const double centerX = x + width / 2.0;
    if (base.wordIndex >= 0 && (centerX < bb.minx || centerX > bb.maxx)) {
      for (const auto& ob : otherWordBounds) {
        if (centerX >= ob.minx && centerX <= ob.maxx)
          return unavailable;
      }
    }
    const auto [shortfall, comfort] = evaluate(plan.original.translate(shift.x, shift.y));
    // Extra spacing is only a comfort cost; it cannot displace the sign to an
    // unrelated word to obtain the full gap.
    // A clear but excessively high authored anchor is still a candidate, not
    // an early exit. Movement from that anchor is only a tie breaker.
    const Score score{shortfall, deviation.lengthSq() + comfortWeight * comfort +
                                1e-6 * shift.lengthSq()};
    if (score < best) { best = score; selected = shift; }
    return score;
  };
  const double step = std::max(10.0, std::min(width, height) / 10.0);
  const int nx = std::max(1, static_cast<int>(std::ceil((right - left) / step)));
  const int ny = std::max(1, static_cast<int>(std::ceil((maxBottom - bottom) / step)));
  consider(preferred.x, preferred.y);
  consider(bounds.minx, bounds.miny);
  for (int x = 0; x <= nx; ++x) for (int y = 0; y <= ny; ++y)
    consider(left + (right - left) * x / nx, bottom + (maxBottom - bottom) * y / ny);
  // Refine preferred and authored axes independently. A coarse grid must not
  // hide a better aligned or lower slot before local refinement starts.
  const auto searchAxis = [&](bool vertical, geometry::Vec2 reference) {
    Score axisBest = unavailable;
    double position = 0.0;
    const auto probe = [&](double value) {
      const Score score = vertical
          ? consider(std::clamp(reference.x, left, right), value, true)
          : consider(value, std::clamp(reference.y, bottom, maxBottom), true);
      if (score < axisBest) { axisBest = score; position = value; }
    };
    const int count = vertical ? ny : nx;
    const double minimum = vertical ? bottom : left;
    const double maximum = vertical ? maxBottom : right;
    for (int i = 0; i <= count; ++i) probe(minimum + (maximum - minimum) * i / count);
    if (!std::isfinite(axisBest.first)) return;
    for (double refine = step / 2.0; refine >= 1.0; refine /= 2.0) {
      const double center = position;
      probe(center - refine); probe(center + refine);
    }
  };
  searchAxis(true, preferred);
  searchAxis(false, preferred);
  if (bounds.minx != preferred.x) searchAxis(true, {bounds.minx, bounds.miny});
  if (bounds.miny != preferred.y) searchAxis(false, {bounds.minx, bounds.miny});
  if (!std::isfinite(best.first))
    return crowded ? plan : search(placement, context, params, true);
  for (double refine = step / 2.0; refine >= 1.0; refine /= 2.0) {
    const auto center = selected;
    for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y)
      consider(bounds.minx + center.x + x * refine, bounds.miny + center.y + y * refine);
  }
  plan.shift = selected;
  plan.accepted = true;
  return plan;
}
}

void prepareWaqfPositions(const std::vector<WaqfPlacementConstraint*>& placements,
                          SolverContext& context, const OptParams& params) {
  std::vector<Plan> plans;
  for (auto* placement : placements) plans.push_back(search(*placement, context, params));
  // Validate all planned signs together. Rejected signs revert to their original
  // ink and can invalidate another plan, so repeat until no rejection occurs.
  bool changed;
  do {
    changed = false;
    for (size_t i = 0; i < plans.size(); ++i) for (size_t j = i + 1; j < plans.size(); ++j) {
      auto& a = plans[i]; auto& b = plans[j];
      if (!a.accepted && !b.accepted) continue;
      const double gap = std::max(0.0, chooseOrdinaryGap(a.placement->waqfMark, b.placement->waqfMark, params));
      auto boundsA = a.original.boundingAABB(), boundsB = b.original.boundingAABB();
      if (a.accepted) boundsA = boundsA.translate(a.shift.x, a.shift.y);
      if (b.accepted) boundsB = boundsB.translate(b.shift.x, b.shift.y);
      if (boxDistance(boundsA, boundsB) > gap) continue;
      const auto shapeA = a.accepted ? a.original.translate(a.shift.x, a.shift.y) : a.original;
      const auto shapeB = b.accepted ? b.original.translate(b.shift.x, b.shift.y) : b.original;
      const auto contact = geometry::getDistance(shapeA, shapeB, gap).contact;
      if (contact.intersect || contact.depth_or_gap < gap - 1e-6) {
        a.accepted = b.accepted = false;
        changed = true;
      }
    }
  } while (changed);
  for (auto& plan : plans) {
    if (!plan.accepted) continue;
    auto& mark = plan.placement->waqfMark;
    mark.dx += plan.shift.x; mark.dy += plan.shift.y;
    mark.initialPositioningDx += plan.shift.x; mark.initialPositioningDy += plan.shift.y;
    mark.initialPlacement = GlyphInstance::InitialPlacement::WaqfSlot;
    updateWorldPolys(mark);
    plan.placement->hasInitialSlot = true;
  }
}
}
