#include "digitalkhatt/layout/RehWawMarkPlacement.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/BelowMarkPlacement.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/GapConstraint.h"

namespace digitalkhatt::layout {
namespace {
// Lowest contour over the mark's horizontal footprint, rather than the
// reference's entire bounding box (whose bowl bottom is away from the tip).
double lowerContour(const geometry::GeometrySet& shape, double left, double right) {
  double result = std::numeric_limits<double>::infinity();
  for (const auto& poly : shape.polys()) {
    for (size_t i = 0; i < poly.size(); ++i) {
      auto a = poly[i], b = poly[(i + 1) % poly.size()];
      if (a.x > b.x) std::swap(a, b);
      const double lo = std::max(left, a.x), hi = std::min(right, b.x);
      if (lo > hi) continue;
      if (a.x == b.x) result = std::min({result, a.y, b.y});
      else for (double x : {lo, hi})
        result = std::min(result, a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x));
    }
  }
  return result;
}
// Slot feasibility needs an actual polygon gap. A support-mapping distance
// can overestimate separation at a touching edge, so use convex separation
// and edge distances for nearby parts; distant AABBs need no polygon work.
double boxGapSquared(const geometry::AABB& a, const geometry::AABB& b) {
  const double x = std::max({0.0, a.minx - b.maxx, b.minx - a.maxx});
  const double y = std::max({0.0, a.miny - b.maxy, b.miny - a.maxy});
  return x * x + y * y;
}

double pointEdgeGapSquared(geometry::Vec2 p, geometry::Vec2 from, geometry::Vec2 to) {
  const auto edge = to - from;
  const double t = edge.lengthSq() == 0 ? 0 :
      std::clamp(geometry::dot(p - from, edge) / edge.lengthSq(), 0.0, 1.0);
  return (p - from - edge * t).lengthSq();
}

bool convexHasClearance(const geometry::Poly& a, const geometry::Poly& b, double gapSquared) {
  bool separated = false;
  for (const auto* polygon : {&a, &b}) {
    for (size_t i = 0; i < polygon->size(); ++i) {
      const auto edge = (*polygon)[(i + 1) % polygon->size()] - (*polygon)[i];
      if (edge.lengthSq() == 0) continue;
      const auto axis = geometry::perp(edge);
      double aMin = std::numeric_limits<double>::infinity(), aMax = -aMin;
      double bMin = aMin, bMax = aMax;
      for (const auto p : a) { const double v = geometry::dot(p, axis); aMin = std::min(aMin,v); aMax = std::max(aMax,v); }
      for (const auto p : b) { const double v = geometry::dot(p, axis); bMin = std::min(bMin,v); bMax = std::max(bMax,v); }
      if (aMax < bMin || bMax < aMin) separated = true;
    }
  }
  if (!separated) return false;  // Includes touching ink.
  for (const auto p : a)
    for (size_t j = 0; j < b.size(); ++j)
      if (pointEdgeGapSquared(p, b[j], b[(j+1)%b.size()]) < gapSquared) return false;
  for (const auto p : b)
    for (size_t j = 0; j < a.size(); ++j)
      if (pointEdgeGapSquared(p, a[j], a[(j+1)%a.size()]) < gapSquared) return false;
  return true;
}

bool shapesHaveClearance(const geometry::GeometrySet& a, const geometry::GeometrySet& b, double gap) {
  const double gapSquared = (gap - 1e-6) * (gap - 1e-6);
  if (boxGapSquared(a.boundingAABB(), b.boundingAABB()) >= gapSquared) return true;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].size() < 3) continue;
    for (size_t j = 0; j < b.size(); ++j) {
      if (b[j].size() < 3 || boxGapSquared(a.aabb()[i], b.aabb()[j]) >= gapSquared) continue;
      if (!convexHasClearance(a[i], b[j], gapSquared)) return false;
    }
  }
  return true;
}

// Called after ink feasibility establishes separation. Measure actual edges
// for ranking too, capped at the comfort target; distant parts need no work.
double separatedShapeGap(const geometry::GeometrySet& a, const geometry::GeometrySet& b, double cap) {
  double gapSquared = cap * cap;
  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i].size() < 3) continue;
    for (size_t j = 0; j < b.size(); ++j) {
      if (b[j].size() < 3 || boxGapSquared(a.aabb()[i], b.aabb()[j]) >= gapSquared) continue;
      for (const auto p : a[i])
        for (size_t k = 0; k < b[j].size(); ++k)
          gapSquared = std::min(gapSquared, pointEdgeGapSquared(p, b[j][k], b[j][(k+1)%b[j].size()]));
      for (const auto p : b[j])
        for (size_t k = 0; k < a[i].size(); ++k)
          gapSquared = std::min(gapSquared, pointEdgeGapSquared(p, a[i][k], a[i][(k+1)%a[i].size()]));
    }
  }
  return std::sqrt(gapSquared);
}
}  // namespace

RehWawMarkPlacement::RehWawMarkPlacement(GlyphInstance& m, GlyphInstance& r, const OptParams& p)
    : mark(m), reference(r), params(p) {
  auto tip = geometry::Vec2{std::numeric_limits<double>::infinity(), 0};
  for (const auto& poly : reference.worldPolys.polys())
    for (auto p : poly)
      if (p.x < tip.x || (p.x == tip.x && p.y < tip.y)) tip = p;
  tipY = tip.y;
  // Keep the target associated with the owner, even for unusually wide gaps.
  const auto owner = mark.prevBase->worldPolys.boundingAABB();
  targetX = std::clamp(tip.x, owner.minx, std::max(owner.maxx, tip.x));
}

double RehWawMarkPlacement::localBottomLimit() const {
  // A following bowl is an obstacle, not another level of the owner's stack.
  // Do not clear it by putting the mark underneath its entire descending body.
  // Reh/Waw remain eligible references (including a second descending Reh).
  const auto owner = mark.prevBase->worldPolys.boundingAABB();
  const double referenceBottom = reference.worldPolys.boundingAABB().miny;
  const auto& dot = followedDotPlacement ? followedDotPlacement->mark : mark;
  const double localBottom = referenceBottom - params.rehWaw.tipGap - boxHeight(dot);
  double bottom = -std::numeric_limits<double>::infinity();
  for (const auto* obstacle : obstacles) {
    if (obstacle->isMark || obstacle == mark.prevBase || obstacle == &reference ||
        obstacle->lineIndex != mark.lineIndex || isRehWaw(*obstacle)) continue;
    const auto box = obstacle->worldPolys.boundingAABB();
    // A shallow neighbor may naturally end above the near-reference slot.
    // Only a body extending BELOW that normal slot can cause a remote escape.
    if (box.miny < localBottom && box.minx < owner.maxx && box.maxx > owner.minx)
      bottom = std::max(bottom, box.miny);
  }
  return bottom;
}

bool RehWawMarkPlacement::hasInkClearance(const geometry::GeometrySet& shape, bool includeMarks) const {
  if (isKasraMark(mark) && dots.empty() && params.toggles.hardStayAboveBelow) {
    // A centered kasra is pinned, so its candidate must already satisfy the
    // ordinary below-mark rail: either its top clears the owner bottom or
    // the baseline clears the point 66% up its bounding box.
    const auto box = shape.boundingAABB();
    const double baselinePoint = box.miny + 0.66 * (box.maxy - box.miny);
    if (std::min(box.maxy - boxBottomY(*mark.prevBase), baselinePoint - mark.prevBase->lineY) > 1e-6)
      return false;
  }
  if (usesBelowFallback() && shape.boundingAABB().miny < localBottomLimit() - 1e-6)
    return false;
  // Compact clearance only decides whether the current gap fits. Relocated
  // marks use their selected slot's reference gap and ordinary owner ink
  // clearance; linked kasra inherits its dot's slot throughout planning.
  for (const auto* obstacle : obstacles) {
    if (!includeMarks && obstacle->isMark) continue;
    const bool otherLine = obstacle->lineIndex != mark.lineIndex;
    const bool ownerOrReference = obstacle == &reference || obstacle == mark.prevBase;
    const bool neighboringMark = usesBelowFallback() && obstacle->isMark && !otherLine &&
                                 obstacle->prevBase != mark.prevBase;
    double gap = neighboringMark ? params.rehWaw.belowMarkGap : params.rehWaw.neighborGap;
    if (ownerOrReference && activeSlot() == Slot::CurrentGap)
      gap = params.rehWaw.compactGap;
    else if (obstacle == &reference)
      gap = activeSlot() == Slot::Left ? params.rehWaw.leftGap : params.rehWaw.tipGap;
    // Pinning a current-gap mark requires the compact minimum against ALL
    // surrounding ink, including another base's lower dots. A large compact
    // clearance must not be silently replaced by the smaller neighbor gap.
    if (activeSlot() == Slot::CurrentGap)
      gap = std::max(gap, params.rehWaw.compactGap);
    if (!shapesHaveClearance(shape, obstacle->worldPolys, gap))
      return false;
    // A lower dot must not claim space made available only by pushing the
    // next line's marks into their own stacks. Protect their authored ink too.
    if (otherLine && obstacle->isMark &&
        !shapesHaveClearance(shape, obstacle->worldPolys.translate(-obstacle->dx, -obstacle->dy), gap))
      return false;
  }
  return true;
}

double RehWawMarkPlacement::neighboringGapPenalty(const geometry::GeometrySet& shape,
                                                  const GlyphInstance& movingMark) const {
  double penalty = 0.0;
  for (const auto* obstacle : obstacles) {
    if (obstacle == &reference || obstacle == mark.prevBase) continue;
    const double preferredGap = std::max(params.rehWaw.neighborGap, chooseMinGap(movingMark, *obstacle, params));
    if (boxGapSquared(shape.boundingAABB(), obstacle->worldPolys.boundingAABB()) >= preferredGap * preferredGap) continue;
    const double gap = geometry::getDistance(shape, obstacle->worldPolys, preferredGap).contact.depth_or_gap;
    const double shortfall = std::max(0.0, preferredGap - gap);
    // Below fallback prioritizes ownership and proximity to the reference
    // bottom over extra breathing room to a neighboring mark.
    const bool softMark = usesBelowFallback() && obstacle->isMark &&
                          obstacle->lineIndex == mark.lineIndex && obstacle->prevBase != mark.prevBase;
    penalty += (softMark ? 0.05 : 1.0) * shortfall * shortfall;
  }
  return penalty;
}

bool RehWawMarkPlacement::dotHasClearance(const geometry::GeometrySet& shape,
                                         std::vector<geometry::GeometrySet>& followingShapes) const {
  followingShapes.clear();
  if (!hasInkClearance(shape)) return false;
  // A slot must fit the moving dot/kasra pair, not just the dot. In
  // particular, following kasra must not be pulled into the next line's Waqf.
  for (const auto* kasra : kasras) {
    auto following = kasra->followingShape(shape);
    if (following.empty()) return false;
    followingShapes.push_back(std::move(following));
  }
  return true;
}

double RehWawMarkPlacement::followingTop(const geometry::GeometrySet& dot,
                                        const geometry::GeometrySet& kasra) const {
  const auto box = kasra.boundingAABB();
  const double contour = lowerContour(dot, box.minx, box.maxx);
  // A linked pair is checked against actual surrounding ink as a whole.
  // Forcing kasra below every overlapping base's lowest contour would reject
  // otherwise clear cavities between Reh and the following letter.
  return (std::isfinite(contour) ? contour : dot.boundingAABB().miny) - params.rehWaw.dotKasraGap;
}

geometry::GeometrySet RehWawMarkPlacement::followingShape(const geometry::GeometrySet& dotShape) const {
  const auto& dot = *dots.front();
  const double authoredOffset = (boxCenterX(dot) - dot.dx) - (boxCenterX(mark) - mark.dx);
  const auto box = dotShape.boundingAABB();
  auto trial = mark.worldPolys.translate((box.minx + box.maxx) * 0.5 - authoredOffset - boxCenterX(mark), 0.0);
  trial = trial.translate(0.0, followingTop(dotShape, trial) - trial.boundingAABB().maxy);
  // Use the dot's full obstacle set, including neighboring marks and lines.
  const auto& checks = followedDotPlacement ? *followedDotPlacement : *this;
  if (checks.hasInkClearance(trial)) return trial;

  // The stack gap is a minimum, while following is compliant. An obstructed
  // compact target must not reject a clear dot if kasra can stay farther below.
  // Bound that extra separation by following the dot from kasra's authored
  // position, rather than allowing an arbitrary descent toward the next line.
  const double authoredDotBottom = dot.worldPolys.boundingAABB().miny - dot.dy;
  const double preservedTop = boxTopY(mark) - mark.dy + box.miny - authoredDotBottom;
  const double extraGap = trial.boundingAABB().maxy - preservedTop;
  if (extraGap <= 0.0) return {};
  constexpr int samples = 8;
  for (int i = 1; i <= samples; ++i) {
    double blockedDrop = extraGap * (i - 1) / samples;
    double clearDrop = extraGap * i / samples;
    auto candidate = trial.translate(0.0, -clearDrop);
    if (!checks.hasInkClearance(candidate)) continue;
    // Refine the first clear interval while keeping the preferred compact
    // target when possible. All accepted samples still satisfy outline gaps.
    for (int j = 0; j < 4; ++j) {
      const double drop = 0.5 * (blockedDrop + clearDrop);
      auto refined = trial.translate(0.0, -drop);
      if (checks.hasInkClearance(refined)) { clearDrop = drop; candidate = std::move(refined); }
      else blockedDrop = drop;
    }
    return candidate;
  }
  return {};
}

void RehWawMarkPlacement::prepareTarget() {
  // Linked kasra follows its own dots. Every independent lower mark uses
  // the same current-gap, left-tip, then local below-tip slot search.
  if (!dots.empty()) return;
  const auto box = mark.worldPolys.boundingAABB();
  const auto owner = mark.prevBase->worldPolys.boundingAABB();
  const double width = box.maxx - box.minx, height = box.maxy - box.miny;
  const double tipX = reference.worldPolys.boundingAABB().minx;
  const double originX = boxCenterX(mark), originY = boxCenterY(mark);
  double bestX = originX, bestY = originY;
  double best = std::numeric_limits<double>::infinity();
  double bestRoom = -std::numeric_limits<double>::infinity();
  dotTargetClear = false;
  searchSlot = Slot::CurrentGap;
  dotUsesLeftSlot = false;
  dotUsesCurrentGap = false;
  std::vector<geometry::GeometrySet> followingShapes;
  followingShapes.reserve(kasras.size());

  // Only feasible candidates compete within a slot. Slot priority is decided
  // separately, so a weighted score cannot displace a clear existing gap.
  auto search = [&](double preferredX, double preferredY) {
    if (dotMinX > dotMaxX || dotMinY > dotMaxY) return false;
    auto consider = [&](double x, double y) {
      const double displacement = (x - preferredX) * (x - preferredX) +
          (y - preferredY) * (y - preferredY);
      // Left/below slots have no current-gap room priority. Their spacing
      // penalties are nonnegative, so displacement alone can rule out a win.
      // Current-gap candidates must still compete for their breathing room.
      if (searchSlot != Slot::CurrentGap && displacement >= best) return;
      const auto trial = mark.worldPolys.translate(x - originX, y - originY);
      if (!dotHasClearance(trial, followingShapes)) return;
      const double refGap = geometry::getDistance(trial, reference.worldPolys, 1e6).contact.depth_or_gap;
      const double shortfall = std::max(0.0, params.rehWaw.tipGap - refGap);
      double penalty = shortfall * shortfall + neighboringGapPenalty(trial, mark);
      double room = 0.0;
      if (searchSlot == Slot::CurrentGap) {
        const double ownerGap = geometry::getDistance(trial, mark.prevBase->worldPolys, 1e6).contact.depth_or_gap;
        const double imbalance = ownerGap - refGap;
        penalty += imbalance * imbalance;
        // Within a valid current gap, use the available breathing room before
        // preferring the authored position. The comfort target is not another
        // clearance floor, so failing to reach it never triggers relocation.
        // Half-unit bands keep equivalent candidates stable under ink noise.
        const double preferred = std::max(params.rehWaw.compactGap, params.rehWaw.preferredGap);
        room = std::floor(2.0 * std::min(
            separatedShapeGap(trial, mark.prevBase->worldPolys, preferred),
            separatedShapeGap(trial, reference.worldPolys, preferred)));
      }
      for (size_t i = 0; i < kasras.size(); ++i)
        penalty += neighboringGapPenalty(followingShapes[i], kasras[i]->mark);
      const double cost = displacement + 4.0 * penalty;
      if (room > bestRoom || (room == bestRoom && cost < best)) {
        bestRoom = room;
        best = cost; bestX = x; bestY = y; dotTargetClear = true;
      }
    };
    consider(std::clamp(preferredX, dotMinX, dotMaxX), std::clamp(preferredY, dotMinY, dotMaxY));
    auto sample = [&](double loX, double hiX, double loY, double hiY, int n) {
      for (int i = 0; i <= (loX == hiX ? 0 : n); ++i)
        for (int j = 0; j <= (loY == hiY ? 0 : n); ++j)
          consider(loX + (hiX - loX) * i / n, loY + (hiY - loY) * j / n);
    };
    sample(dotMinX, dotMaxX, dotMinY, dotMaxY, 8);
    if (!dotTargetClear) return false;
    double stepX = (dotMaxX - dotMinX) / 8, stepY = (dotMaxY - dotMinY) / 8;
    for (int i = 0; i < 3; ++i) {
      sample(std::max(dotMinX, bestX - stepX), std::min(dotMaxX, bestX + stepX),
             std::max(dotMinY, bestY - stepY), std::min(dotMaxY, bestY + stepY), 4);
      stepX *= 0.5; stepY *= 0.5;
    }
    return true;
  };

  // Keep a clear balanced position in the existing gap before relocating to
  // the left or below. This is the cavity at the tip: keep the dot's footprint
  // across the tip's x coordinate, with its center on the gap side of that
  // tip. Otherwise a wholly left-shifted mark can masquerade as centering
  // and bypass left clearance/alignment before being pinned. Exact outlines,
  // not full-width left-slot bounds or
  // the below-tip checkbox, decide whether the current gap fits.
  dotMinX = std::max(owner.minx, tipX);
  dotMaxX = std::min(owner.maxx, tipX + width * 0.5);
  const double authoredY = originY - mark.dy;
  dotMinY = std::max(tipY, std::min(tipY + height * 0.5, authoredY - height * 0.5));
  dotMaxY = owner.miny + height * 0.5 - params.rehWaw.compactGap;
  if (search(originX - mark.dx, authoredY)) {
    dotUsesCurrentGap = true;
    targetX = bestX;
    dotTargetTop = bestY + height * 0.5;
    return;
  }
  searchSlot = Slot::Left;

  // A left mark's vertical CENTER aligns with the leftmost reference tip.
  // Its right edge must also keep an explicit horizontal gap from Reh/Waw.
  const double referenceBottom = reference.worldPolys.boundingAABB().miny;
  dotMinX = owner.minx + width * 0.5;
  dotMaxX = std::min(owner.maxx - width * 0.5,
                    tipX - params.rehWaw.leftGap - width * 0.5);
  dotMinY = dotMaxY = tipY;
  const double leftPreferredX = (dotMinX + dotMaxX) * 0.5;
  dotUsesLeftSlot = search(leftPreferredX, dotMinY);
  if (!dotUsesLeftSlot) {
    const double overhang = std::min(params.rehWaw.overhangLimit, width * (params.rehWaw.overhangPercent / 100.0));
    dotMinX -= overhang;
    dotMaxX = std::min(owner.maxx - width * 0.5 + overhang,
                      tipX - params.rehWaw.leftGap - width * 0.5);
    dotUsesLeftSlot = search(leftPreferredX, dotMinY);
  }
  // Dot-specific fallback switches do not disable standalone kasra's search.
  const bool belowTipFallback = isKasraMark(mark) || (mark.glyphName == "twodotsdown" ?
      params.rehWaw.twoDotsBelowTip : params.rehWaw.oneDotBelowTip);
  if (!dotUsesLeftSlot && !belowTipFallback) {
    // Both non-descending searches failed. Leave ordinary collision handling
    // active rather than forcing a below-tip target against the user's choice.
    targetX = bestX;
    dotTargetTop = bestY + height * 0.5;
    return;
  }
  if (!dotUsesLeftSlot) {
    // First keep the mark's center under its semantic owner. Neighboring
    // marks need ink clearance, but their larger spacing target may yield.
    searchSlot = Slot::Below;
    dotMinX = owner.minx;
    dotMaxX = owner.maxx;
    dotMaxY = referenceBottom - params.rehWaw.tipGap - height * 0.5;
    dotMinY = dotMaxY - height - params.rehWaw.tipGap;
    // Stop at a descending neighbor's bottom as well as the next line. If
    // no local slot fits, leave the authored input to ordinary constraints;
    // a distant ink-clear slot would lose the mark's semantic association.
    double lowerLineLimit = localBottomLimit() + height * 0.5;
    for (const auto* obstacle : obstacles) {
      if (obstacle->lineIndex == mark.lineIndex + 1)
        lowerLineLimit = std::max(lowerLineLimit,
            0.5 * (mark.lineY + obstacle->lineY) + height * 0.5);
    }
    dotMinY = std::max(dotMinY, lowerLineLimit);
    const double preferredX = 0.5 * (owner.minx + owner.maxx);
    const double preferredY = dotMaxY;
    if (!search(preferredX, preferredY)) {
      // A following Reh can descend farther than the reference tip (e.g.
      // page 107). Expand downward only after the near-tip region fails;
      // never use a following Ain's whole bounding box as the initial rail.
      for (const auto* obstacle : obstacles) {
        if (isRehWaw(*obstacle))
          dotMinY = std::min(dotMinY, obstacle->worldPolys.boundingAABB().miny -
              params.rehWaw.tipGap - height * 0.5);
      }
      dotMinY = std::max(dotMinY, lowerLineLimit);
      for (int attempt = 0; attempt < 3; ++attempt) {
        if (search(preferredX, preferredY)) break;
        const double nextMinY = std::max(lowerLineLimit, dotMinY - height - params.rehWaw.tipGap);
        if (nextMinY == dotMinY) break;
        dotMinY = nextMinY;
      }
    }
  }
  targetX = bestX;
  dotTargetTop = bestY + height * 0.5;
}

geometry::GeometrySet RehWawMarkPlacement::preferredShape() const {
  if (dots.empty()) {
    if (!dotTargetClear) return {};
    return mark.worldPolys.translate(targetX - boxCenterX(mark), dotTargetTop - boxTopY(mark));
  }
  if (followedDotPlacement && !followedDotPlacement->dotTargetClear) return {};
  const auto* placement = followedDotPlacement;
  if (!placement) return followingShape(dots.front()->worldPolys);
  const auto dotShape = placement->preferredShape();
  return dotShape.empty() ? geometry::GeometrySet{} : followingShape(dotShape);
}

}  // namespace digitalkhatt::layout
