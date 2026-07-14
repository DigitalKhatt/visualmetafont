#include "digitalkhatt/layout/constraints/SqueezeCenterConstraint.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

using namespace geometry;

namespace digitalkhatt::layout {

SqueezeCenterConstraint::SqueezeCenterConstraint(
    GlyphInstance& mark_,
    const OptParams& P_,
    const std::vector<GlyphInstance*>& obstacles_,
    double centerCompliance_)
    : mark(mark_), P(P_), centerCompliance(centerCompliance_) {
  obstacles.reserve(obstacles_.size());
  for (GlyphInstance* g : obstacles_) {
    if (!g) continue;
    if (g == &mark) continue;
    obstacles.push_back(ObstacleState{g});
  }
}

void SqueezeCenterConstraint::project(SolverContext& solverContext, double dt) {
  const double wM = mark.mobility;
  if (wM <= 0.0) return;
  if (obstacles.empty()) return;

  const double markCenterX = boxCenterX(mark);

  // Track the nearest violated obstacle on each side of the mark.
  ObstacleState* leftViolator = nullptr;
  ObstacleState* rightViolator = nullptr;
  double leftEdge = 0.0;
  double rightEdge = 0.0;
  double bestLeftDist = std::numeric_limits<double>::infinity();
  double bestRightDist = std::numeric_limits<double>::infinity();

  for (auto& os : obstacles) {
    GlyphInstance& obs = *os.obstacle;
    const double wO = obs.mobility;

    const double gmin = chooseMinGap(mark, obs, P);
    if (gmin <= 1e-9) continue;

    auto dr = getDistance(mark.worldPolys, obs.worldPolys, gmin);
    if (!std::isfinite(dr.contact.depth_or_gap)) continue;

    const double C = dr.contact.depth_or_gap - gmin;  // want C >= 0

    // -------- hard per-obstacle min-gap inequality (never overlap) --------
    const double contactCompliance = gapCompliance(mark, obs);
    const double alpha = contactCompliance / (dt * dt);
    const double denom = wM + wO + alpha;

    if (C >= 0.0) {
      os.lambda = 0.0;
    } else if (denom >= 1e-9) {
      Vec2 n = dr.contact.normal;
      double deltaLambda = (-C - alpha * os.lambda) / denom;
      double lambdaNew = std::max(0.0, os.lambda + deltaLambda);
      deltaLambda = lambdaNew - os.lambda;
      os.lambda = lambdaNew;

      if (wM > 0.0) {
        mark.dx += n.x * (-wM * deltaLambda);
        mark.dy += n.y * (-wM * deltaLambda);
        buildWorldPolys(mark);
      }
      if (wO > 0.0) {
        obs.dx += n.x * (wO * deltaLambda);
        obs.dy += n.y * (wO * deltaLambda);
        buildWorldPolys(obs);
      }
    }

    // -------- track violation side for the soft centering target --------
    if (C >= 0.0) continue;

    const double obsCenterX = boxCenterX(obs);
    auto box = obs.worldPolys.boundingAABB();
    if (obsCenterX <= markCenterX) {
      const double edge = box.maxx;  // right-facing edge of a left obstacle
      const double dist = markCenterX - edge;
      if (dist < bestLeftDist) {
        bestLeftDist = dist;
        leftViolator = &os;
        leftEdge = edge;
      }
    } else {
      const double edge = box.minx;  // left-facing edge of a right obstacle
      const double dist = edge - markCenterX;
      if (dist < bestRightDist) {
        bestRightDist = dist;
        rightViolator = &os;
        rightEdge = edge;
      }
    }
  }

  // -------- soft centering: only active when squeezed on both sides --------
  if (leftViolator != nullptr && rightViolator != nullptr) {
    const double target = 0.5 * (leftEdge + rightEdge);
    const double C = boxCenterX(mark) - target;

    const double alpha = centerCompliance / (dt * dt);
    const double denom = wM + alpha;
    if (denom >= 1e-9) {
      const double deltaLambda = (-C - alpha * lambdaCenter) / denom;
      lambdaCenter += deltaLambda;
      mark.dx += wM * deltaLambda;
      buildWorldPolys(mark);
    }
  } else {
    lambdaCenter = 0.0;
  }
}

void SqueezeCenterConstraint::reportViolations(
    SolverContext&, std::vector<ConstraintViolation>& out) const {
  // Per-obstacle hard min-gap breach; the soft centering arm is phase 2.
  for (const auto& os : obstacles) {
    GlyphInstance& obs = *os.obstacle;
    const double gmin = chooseMinGap(mark, obs, P);
    if (gmin <= 1e-9) continue;

    auto dr = getDistance(mark.worldPolys, obs.worldPolys, gmin);
    if (!std::isfinite(dr.contact.depth_or_gap)) continue;

    const double C = dr.contact.depth_or_gap - gmin;  // violation when C < 0
    if (C >= 0.0) continue;

    ConstraintViolation v;
    v.type = ViolationType::SqueezeCenter;
    v.kind = ViolationKind::Hard;
    v.residual = C;
    v.severity = -C;
    v.glyphA = mark.globalIndex;
    v.glyphB = obs.globalIndex;
    v.markerCount = 2;  // the contact segment between the two shapes
    v.marker[0] = dr.contact.pA;
    v.marker[1] = dr.contact.pB;
    out.push_back(v);
  }
}

}  // namespace digitalkhatt::layout
