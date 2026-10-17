#include "digitalkhatt/layout/constraints/RehWawMarkConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/BelowMarkPlacement.h"
#include "digitalkhatt/layout/ConstraintDiagnostics.h"
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
double lowerContour(const GlyphInstance& base, double left, double right) {
  return lowerContour(base.worldPolys, left, right);
}
// Slot feasibility needs an actual polygon gap. A support-mapping distance
// can overestimate separation at a touching edge, so use convex separation
// and edge distances for nearby parts; distant AABBs need no polygon work.
double boxGapSquared(const geometry::AABB& a, const geometry::AABB& b) {
  const double x = std::max({0.0, a.minx - b.maxx, b.minx - a.maxx});
  const double y = std::max({0.0, a.miny - b.maxy, b.miny - a.maxy});
  return x * x + y * y;
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
  auto pointEdgeGapSquared = [](geometry::Vec2 p, geometry::Vec2 from, geometry::Vec2 to) {
    const auto edge = to - from;
    const double t = edge.lengthSq() == 0 ? 0 :
        std::clamp(geometry::dot(p - from, edge) / edge.lengthSq(), 0.0, 1.0);
    return (p - from - edge * t).lengthSq();
  };
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
}  // namespace

RehWawMarkConstraint::RehWawMarkConstraint(GlyphInstance& m, GlyphInstance& r, const OptParams& p)
    : mark(m), reference(r), params(p) {
  compliance = 0.2;
  reportEnabled = true;
  auto tip = geometry::Vec2{std::numeric_limits<double>::infinity(), 0};
  for (const auto& poly : reference.worldPolys.polys())
    for (auto p : poly)
      if (p.x < tip.x || (p.x == tip.x && p.y < tip.y)) tip = p;
  tipY = tip.y;
  belowTip = isKasraMark(mark);
  // Keep the target associated with the owner, even for unusually wide gaps.
  const auto owner = mark.prevBase->worldPolys.boundingAABB();
  targetX = std::clamp(tip.x, owner.minx, std::max(owner.maxx, tip.x));
}

bool RehWawMarkConstraint::hasInkClearance(const geometry::GeometrySet& shape) const {
  // Protect the 10-unit ink gap against rounding of both glyph positions.
  constexpr double clearance = 14.0;
  for (const auto* obstacle : obstacles) {
    const bool otherLine = obstacle->lineIndex != mark.lineIndex;
    const double gap = clearance;
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

bool RehWawMarkConstraint::dotHasClearance(const geometry::GeometrySet& shape) const {
  if (!hasInkClearance(shape)) return false;
  // A slot must fit the moving dot/kasra pair, not just the dot. In
  // particular, following kasra must not be pulled into the next line's Waqf.
  for (const auto* kasra : kasras)
    if (!hasInkClearance(kasra->followingShape(shape))) return false;
  return true;
}

double RehWawMarkConstraint::followingTop(const geometry::GeometrySet& dot,
                                        const geometry::GeometrySet& kasra) const {
  const auto box = kasra.boundingAABB();
  const double contour = lowerContour(dot, box.minx, box.maxx);
  // A linked pair is checked against actual surrounding ink as a whole.
  // Forcing kasra below every overlapping base's lowest contour would reject
  // otherwise clear cavities between Reh and the following letter.
  return (std::isfinite(contour) ? contour : dot.boundingAABB().miny) - belowDotKasraGap;
}

geometry::GeometrySet RehWawMarkConstraint::followingShape(const geometry::GeometrySet& dotShape) const {
  const auto& dot = *dots.front();
  const double authoredOffset = (boxCenterX(dot) - dot.dx) - (boxCenterX(mark) - mark.dx);
  const auto box = dotShape.boundingAABB();
  auto trial = mark.worldPolys.translate((box.minx + box.maxx) * 0.5 - authoredOffset - boxCenterX(mark), 0.0);
  return trial.translate(0.0, followingTop(dotShape, trial) - trial.boundingAABB().maxy);
}

void RehWawMarkConstraint::prepareDotTarget() {
  if (isKasraMark(mark)) return;
  const auto box = mark.worldPolys.boundingAABB();
  const auto owner = mark.prevBase->worldPolys.boundingAABB();
  const double width = box.maxx - box.minx, height = box.maxy - box.miny;
  const double tipX = reference.worldPolys.boundingAABB().minx;
  const double originX = boxCenterX(mark), originY = boxCenterY(mark);
  double bestX = originX, bestY = originY;
  double best = std::numeric_limits<double>::infinity();
  dotTargetClear = false;

  // Only feasible candidates compete for a target. A narrow or obstructed
  // left slot must never beat the below-tip fallback through a weighted score.
  auto search = [&](double preferredX, double preferredY) {
    if (dotMinX > dotMaxX || dotMinY > dotMaxY) return false;
    auto consider = [&](double x, double y) {
      const auto trial = mark.worldPolys.translate(x - originX, y - originY);
      if (!dotHasClearance(trial)) return;
      const double refGap = geometry::getDistance(trial, reference.worldPolys, 1e6).contact.depth_or_gap;
      const double shortfall = std::max(0.0, rehWawTipGap - refGap);
      const double cost = (x - preferredX) * (x - preferredX) +
          (y - preferredY) * (y - preferredY) + 4.0 * shortfall * shortfall;
      if (cost < best) { best = cost; bestX = x; bestY = y; dotTargetClear = true; }
    };
    consider(std::clamp(preferredX, dotMinX, dotMaxX), std::clamp(preferredY, dotMinY, dotMaxY));
    auto sample = [&](double loX, double hiX, double loY, double hiY, int n) {
      for (int i = 0; i <= n; ++i) for (int j = 0; j <= n; ++j)
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

  // First try a slot that contains the whole dot, left of and above the tip.
  // Width alone is insufficient: every candidate must also clear nearby ink.
  dotMinX = owner.minx + width * 0.5;
  dotMaxX = std::min(owner.maxx - width * 0.5, tipX - width * 0.5);
  dotMinY = tipY + height * 0.5;
  // The owner's bbox bottom can be a remote low point on its sloping stroke.
  // Let a left-slot dot rise under the actual contour, while its bottom stays
  // below that bbox point. Exact ink clearance decides whether it fits.
  dotMaxY = owner.miny + height * 0.5 - 14.0;
  dotUsesLeftSlot = search((dotMinX + dotMaxX) * 0.5, dotMinY);
  if (!dotUsesLeftSlot) {
    // Keep the dot's center associated with Beh. With no full-width left
    // slot, prefer directly below the tip, rather than a cramped side gap.
    dotMinX = owner.minx;
    dotMaxX = owner.maxx;
    dotMaxY = tipY - rehWawTipGap - height * 0.5;
    dotMinY = dotMaxY - height - rehWawTipGap;
    double lowerLineLimit = -std::numeric_limits<double>::infinity();
    for (const auto* obstacle : obstacles) {
      if (obstacle->lineIndex == mark.lineIndex + 1)
        lowerLineLimit = std::max(lowerLineLimit,
            0.5 * (mark.lineY + obstacle->lineY) + height * 0.5);
    }
    dotMinY = std::max(dotMinY, lowerLineLimit);
    double preferredX = std::clamp(tipX, dotMinX, dotMaxX);
    const double preferredY = dotMaxY;
    if (!search(preferredX, preferredY)) {
      // Allow a local half-dot overhang before descending farther. This keeps
      // a cramped dot/kasra pair near its owner while making room sideways.
      dotMinX = owner.minx - width * 0.5;
      dotMaxX = owner.maxx + width * 0.5;
      preferredX = std::clamp(tipX, dotMinX, dotMaxX);
    }
    if (!dotTargetClear && !search(preferredX, preferredY)) {
      // A following Reh can descend farther than the reference tip (e.g.
      // page 107). Expand downward only after the near-tip region fails;
      // never use a following Ain's whole bounding box as the initial rail.
      for (const auto* obstacle : obstacles) {
        if (isRehWaw(*obstacle))
          dotMinY = std::min(dotMinY, obstacle->worldPolys.boundingAABB().miny -
              rehWawTipGap - height * 0.5);
      }
      dotMinY = std::max(dotMinY, lowerLineLimit);
      for (int attempt = 0; attempt < 3; ++attempt) {
        if (search(preferredX, preferredY)) break;
        const double nextMinY = std::max(lowerLineLimit, dotMinY - height - rehWawTipGap);
        if (nextMinY == dotMinY) break;
        dotMinY = nextMinY;
      }
    }
  }
  targetX = bestX;
  dotTargetTop = bestY + height * 0.5;
}

void RehWawMarkConstraint::projectDotBounds() {
  if (isKasraMark(mark) || mark.mobility <= 0.0) return;
  // Recheck a target against moving neighboring marks. Re-running the
  // staged search can switch from a blocked left slot to below the tip.
  auto trial = mark.worldPolys.translate(targetX - boxCenterX(mark), dotTargetTop - boxTopY(mark));
  if (!dotTargetClear || !dotHasClearance(trial)) {
    prepareDotTarget();
    lambdaX = lambdaY = 0.0;
    trial = mark.worldPolys.translate(targetX - boxCenterX(mark), dotTargetTop - boxTopY(mark));
  }
  if (!dotTargetClear || !dotHasClearance(trial)) return;
  // This runs after generic contacts. Do not let their shortest-direction
  // correction leave a dot back in the rejected side gap or above the tip.
  mark.dx += targetX - boxCenterX(mark);
  mark.dy += dotTargetTop - boxTopY(mark);
  updateWorldPolys(mark);
}

double RehWawMarkConstraint::targetTop() const {
  const auto box = mark.worldPolys.boundingAABB();
  if (belowTip) {
    const double contour = lowerContour(reference, box.minx, box.maxx);
    double target = (std::isfinite(contour) ? contour : tipY) - rehWawTipGap;
    if (!dots.empty()) {
      target = std::numeric_limits<double>::infinity();
      for (const auto* dot : dots)
        target = std::min(target, followingTop(dot->worldPolys, mark.worldPolys));
      return target;
    }
    for (const auto* obstacle : obstacles) {
      if (!obstacle || obstacle->lineIndex != mark.lineIndex || obstacle->worldPolys.empty()) continue;
      const double floor = lowerContour(*obstacle, box.minx, box.maxx);
      if (std::isfinite(floor)) target = std::min(target, floor - chooseMinGap(*obstacle, mark, params));
    }
    return target;
  }
  return dotTargetTop;
}

void RehWawMarkConstraint::projectKasraFollow() {
  if (dots.empty() || mark.mobility <= 0.0 ||
      (followedDotPlacement && !followedDotPlacement->dotTargetClear)) return;
  const auto& dot = *dots.front();
  const double authoredOffset = (boxCenterX(dot) - dot.dx) - (boxCenterX(mark) - mark.dx);
  targetX = boxCenterX(dot) - authoredOffset;
  mark.dx += targetX - boxCenterX(mark);
  updateWorldPolys(mark);
  mark.dy += targetTop() - boxTopY(mark);
  updateWorldPolys(mark);
}

void RehWawMarkConstraint::project(SolverContext&, double dt) {
  if (!isKasraMark(mark)) {
    projectDotBounds();
    return;
  }
  if (!dots.empty()) {
    projectKasraFollow();
    return;
  }
  const double w = mark.mobility;
  if (w <= 0.0) return;
  const double alpha = compliance / (dt * dt);
  auto projectAxis = [&](double residual, double& lambda, double& offset) {
    const double correction = -(residual + alpha * lambda) / (w + alpha);
    lambda += correction;
    offset += w * correction;
    updateWorldPolys(mark);
  };
  projectAxis(boxCenterX(mark) - targetX, lambdaX, mark.dx);
  projectAxis(boxTopY(mark) - targetTop(), lambdaY, mark.dy);
}

void RehWawMarkConstraint::reportViolations(SolverContext&, std::vector<ConstraintViolation>& out) const {
  reportSoftTarget(out, ViolationType::SoftTargetResidual, mark, &reference,
                   boxCenterX(mark) - targetX, "Reh/Waw tip horizontal preference");
  reportSoftTarget(out, ViolationType::SoftTargetResidual, mark, &reference,
                   boxTopY(mark) - targetTop(), "Reh/Waw tip lower-mark preference");
}

}  // namespace digitalkhatt::layout
