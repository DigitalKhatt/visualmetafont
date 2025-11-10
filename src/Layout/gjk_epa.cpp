#include <algorithm>

#include "contact_flexible.h"
#include "geometry.h"

namespace geometry {

inline double clamp01(double t) {
  if (t < 0.0) return 0.0;
  if (t > 1.0) return 1.0;
  return t;
}

struct SupportPoint {
  Vec2 a;  // point on shape A
  Vec2 b;  // point on shape B
  Vec2 p;  // Minkowski difference = a - b
};

struct Simplex {
  SupportPoint pts[3];
  int count = 0;
  Vec2 dir;
};

// ---------- GJK (feature-aware separated witnesses) ----------
struct GJKResult {
  bool intersect = false;
  Vec2 closestA{0, 0}, closestB{0, 0};
  double distance = 0.0;
  Vec2 normal{1, 0};
  Simplex simplex;
};

struct EPAResult {
  bool success = false;
  Vec2 normal;         // from A to B (unit)
  double depth = 0.0;  // penetration depth
  Vec2 contactA;       // witness on A
  Vec2 contactB;       // witness on B
};

// ======================= Helpers =======================

// Farthest point in direction d on a convex polygon
inline Vec2 supportPoint(const std::vector<Vec2>& poly, const Vec2& d) {
  assert(!poly.empty());
  int best = 0;
  double bestDot = Vec2::dot(poly[0], d);
  for (int i = 1; i < (int)poly.size(); ++i) {
    double v = Vec2::dot(poly[i], d);
    if (v > bestDot) {
      bestDot = v;
      best = i;
    }
  }
  return poly[best];
}
// Support of Minkowski difference A - B along direction d
inline SupportPoint support(const std::vector<Vec2>& a,
                            const std::vector<Vec2>& b, const Vec2& d) {
  SupportPoint sp;
  sp.a = supportPoint(a, d);
  sp.b = supportPoint(b, Vec2(-d.x, -d.y));
  sp.p = sp.a - sp.b;
  return sp;
}

// Compute closest points on segment AB to origin given endpoints as
// SupportPoints Returns (closestA, closestB), using barycentric interpolation
// on A and B.
inline void closestOnSegmentToOrigin(const SupportPoint& s0,
                                     const SupportPoint& s1, Vec2& outA,
                                     Vec2& outB) {
  Vec2 A = s0.p;
  Vec2 B = s1.p;
  Vec2 d = B - A;
  double denom = d.lengthSq();

  double t = 0.0;
  if (denom > 0.0) {
    t = -Vec2::dot(A, d) / denom;
  }
  t = clamp01(t);

  outA = s0.a + (s1.a - s0.a) * t;
  outB = s0.b + (s1.b - s0.b) * t;
}

// ======================= GJK simplex update =======================

// Update simplex for line case; sets new search direction towards origin.
inline bool handleLine(Simplex& s) {
  // Simplex: [B, A] where A is newest point (last)
  const Vec2& A = s.pts[1].p;
  const Vec2& B = s.pts[0].p;

  Vec2 AB = B - A;
  Vec2 AO = Vec2(-A.x, -A.y);

  // If origin is in direction of AB, keep segment, search perpendicular to AB
  // towards origin
  if (Vec2::dot(AB, AO) > 0.0) {
    // New dir = perpendicular to AB towards origin
    // 2D perpendicular (left): (y, -x)
    // Choose sign so it points towards origin.
    Vec2 perp(AB.y, -AB.x);
    if (Vec2::dot(perp, AO) < 0.0) {
      perp = Vec2(-perp.x, -perp.y);
    }
    s.dir = normSafe(perp);
    // s.dir = normSafe(Vec2::perp(AB, AO, AB));
  } else {
    // Otherwise, drop B; go directly towards origin from A
    s.pts[0] = s.pts[1];
    s.count = 1;
    s.dir = normSafe(AO);
  }

  return false;  // cannot contain origin yet
}

// Update simplex for triangle case; sets new search direction or detects
// containment.
inline bool handleTriangle(Simplex& s) {
  // Simplex: [C, B, A] (A is newest)
  const Vec2& A = s.pts[2].p;
  const Vec2& B = s.pts[1].p;
  const Vec2& C = s.pts[0].p;

  Vec2 AO = Vec2(-A.x, -A.y);
  Vec2 AB = B - A;
  Vec2 AC = C - A;

  // Vec2 ACperp = Vec2::perp(AB, AC, AC);

  Vec2 ACperp(AC.y, -AC.x);
  if (Vec2::dot(ACperp, AB) > 0.0) {
    ACperp = Vec2(-ACperp.x, -ACperp.y);
  }

  // Check region AC
  if (Vec2::dot(ACperp, AO) >= 0.0) {
    if (Vec2::dot(AC, AO) >= 0.0) {
      // Remove B, keep A,C
      s.pts[1] = s.pts[2];
      s.count = 2;
      s.dir = normSafe(ACperp);
      // s.dir = normSafe(Vec2::perp(AC, AO, AC));
      return false;
    } else {
      // Remove C, keep A,B
      s.pts[0] = s.pts[1];
      s.pts[1] = s.pts[2];
      s.count = 2;
      return handleLine(s);
    }
  } else {
    // Vec2 ABperp = Vec2::perp(AC, AB, AB);
    //  Perps towards outside
    Vec2 ABperp(AB.y, -AB.x);
    if (Vec2::dot(ABperp, AC) > 0.0) {
      ABperp = Vec2(-ABperp.x, -ABperp.y);
    }
    // Check region AB
    if (Vec2::dot(ABperp, AO) >= 0.0) {
      // Remove C, keep A,B
      s.pts[0] = s.pts[1];
      s.pts[1] = s.pts[2];
      s.count = 2;
      return handleLine(s);
    }
  }

  // Otherwise origin is inside the triangle → intersection
  return true;
}

// GJK step: update simplex and direction; returns true if origin is contained.
inline bool updateSimplex(Simplex& s) {
  assert(s.count == 2 || s.count == 3);
  if (s.count == 2) {
    return handleLine(s);
  } else {
    return handleTriangle(s);
  }
}

inline Vec2 getDela(const Simplex& s) {
  assert(s.count == 2 || s.count == 3);
  if (s.count == 2)
    return Vec2::sub(s.pts[1].p, s.pts[0].p);
  else
    return Vec2::sub(s.pts[2].p, s.pts[1].p);
}

// ======================= GJK main =======================

constexpr double EPSILON = 1e-3;
constexpr double EPSILON_SQUARED = EPSILON * EPSILON;

inline GJKResult gjk(const Poly& A, const Poly& B, int maxIterations = 32) {
  GJKResult result;
  Simplex simplex;

  // Initial direction: from centroid(A) to centroid(B) or arbitrary
  Vec2 ca(0.0, 0.0), cb(0.0, 0.0);
  for (auto& p : A) ca += p;
  for (auto& p : B) cb += p;
  if (!A.empty()) ca = ca / (double)A.size();
  if (!B.empty()) cb = cb / (double)B.size();
  simplex.dir = cb - ca;
  // if (simplex.dir.lengthSq() < EPSILON_SQUARED) simplex.dir = Vec2(1.0, 0.0);
  simplex.dir = Vec2(1.0, 0.0);

  // First point
  simplex.pts[0] = support(A, B, simplex.dir);
  simplex.count = 1;
  simplex.dir = normSafe(Vec2(-simplex.pts[0].p.x, -simplex.pts[0].p.y));
  if (simplex.dir.lengthSq() < EPSILON_SQUARED) {
    // Very degenerate: shapes directly overlap at that point.
    result.intersect = true;
    result.simplex = simplex;
    return result;
  }

  double lastSqDist = std::numeric_limits<double>::infinity();

  for (int iter = 0; iter < maxIterations; ++iter) {
    // New support in current direction
    SupportPoint newPt = support(A, B, simplex.dir);

    double proj = Vec2::dot(newPt.p, simplex.dir);

    auto newSqDist = newPt.p.lengthSq();

    // No progress towards origin: no intersection; compute
    // distance from current simplex
    if (proj <= -EPSILON ||
        std::fabs(lastSqDist - newSqDist) <= EPSILON_SQUARED) {
      // no intersection
      simplex.pts[simplex.count++] = newPt;
      auto delta = getDela(simplex);
      auto dd = dot(simplex.dir, delta);
      while (dd > EPSILON) {
        auto tt = updateSimplex(simplex);
        assert(!tt);
        newPt = support(A, B, simplex.dir);
        simplex.pts[simplex.count++] = newPt;
        delta = getDela(simplex);
        dd = dot(simplex.dir, delta);
      }

      // Closest distance from origin to current simplex
      if (simplex.count == 2) {
        result.closestA = simplex.pts[0].a;
        result.closestB = simplex.pts[0].b;
      } else {
        closestOnSegmentToOrigin(simplex.pts[1], simplex.pts[0],
                                 result.closestA, result.closestB);
      }
      Vec2 diff = result.closestA - result.closestB;
      result.distance = std::sqrt(diff.lengthSq());
      result.intersect = result.distance <= EPSILON;
      result.simplex = simplex;
      return result;
    }

    // Add new point and update simplex
    simplex.pts[simplex.count++] = newPt;

    if (updateSimplex(simplex)) {
      // Origin inside simplex => intersection
      result.intersect = true;
      result.simplex = simplex;
      return result;
    }

    double dirSq = simplex.dir.lengthSq();
    if (dirSq < EPSILON_SQUARED) {
      // Direction collapsed; treat as intersecting
      result.intersect = true;
      result.simplex = simplex;
      return result;
    }

    lastSqDist = newSqDist;
  }

  // If we get here, treat as intersecting or fail-safe
  result.intersect = true;
  result.simplex = simplex;
  return result;
}

inline EPAResult epa(const Poly& A, const Poly& B, const Simplex& simplexInit,
                     int maxIterations = 64, double eps = 1e-6) {
  EPAResult res;

  // Build initial polytope from GJK simplex (triangle in 2D)
  std::vector<SupportPoint> polytope;
  polytope.reserve(16);

  if (simplexInit.count < 3) {
    // Need at least a triangle to run EPA in 2D
    return res;
  }

  for (int i = 0; i < simplexInit.count; ++i) {
    polytope.push_back(simplexInit.pts[i]);
  }

  for (int iter = 0; iter < maxIterations; ++iter) {
    // Find edge closest to origin
    int closestEdge = -1;
    double minDist = std::numeric_limits<double>::infinity();
    Vec2 bestNormal;

    int n = (int)polytope.size();
    for (int i = 0; i < n; ++i) {
      const SupportPoint& sp0 = polytope[i];
      const SupportPoint& sp1 = polytope[(i + 1) % n];

      Vec2 e = sp1.p - sp0.p;

      // Edge normal (perp)
      Vec2 nrm(e.y, -e.x);

      // Ensure outward (pointing away from origin)
      // If dot(normal, midpoint) < 0 → flip
      Vec2 mid = (sp0.p + sp1.p) * 0.5;
      if (Vec2::dot(nrm, mid) < 0.0) {
        nrm = Vec2(-nrm.x, -nrm.y);
      }

      double len = nrm.length();
      if (len <= eps) continue;

      // Distance from origin along this normal
      double dist = Vec2::dot(nrm, sp0.p) / len;
      if (dist < 0.0) {
        // Shouldn't really happen if origin inside, but guard
        dist = -dist;
      }

      if (dist < minDist) {
        minDist = dist;
        closestEdge = i;
        bestNormal = nrm / len;
      }
    }

    if (closestEdge < 0) {
      return res;  // failed
    }

    // Get new support point in bestNormal direction
    SupportPoint newPt = support(A, B, bestNormal);

    double separation = Vec2::dot(newPt.p, bestNormal);

    // If the new point doesn't push boundary out significantly, we converged
    if (separation - minDist < eps) {
      res.success = true;
      res.normal = bestNormal;
      res.depth = separation;

      // Compute contact points on that closest edge by projecting origin onto
      // segment in Minkowski space
      const SupportPoint& e0 = polytope[closestEdge];
      const SupportPoint& e1 = polytope[(closestEdge + 1) % n];

      Vec2 cA, cB;
      closestOnSegmentToOrigin(e0, e1, cA, cB);
      res.contactA = cA;
      res.contactB = cB;
      return res;
    }

    // Otherwise, insert new point into polytope after closestEdge
    polytope.insert(polytope.begin() + closestEdge + 1, newPt);
  }

  // If reached max iterations without convergence
  return res;
}

// ---------- public wrapper ----------
Contact contactGjkEpaPoly(const Poly& A, const Poly& B, int gjkIters,
                          int epaIters) {
  Contact C;

  if (A.size() < 2 || B.size() < 2) {
    C.intersect = false;
    C.depth_or_gap = std::numeric_limits<double>::infinity();
    C.normal = {1, 0};
    C.pA = C.pB = {0, 0};
    return C;
  }

  auto G = gjk(A, B, gjkIters);
  if (!G.intersect) {
    C.intersect = false;
    C.pA = G.closestA;
    C.pB = G.closestB;
    Vec2 m = Vec2::sub(C.pB, C.pA);
    C.depth_or_gap = Vec2::len(m);
    C.normal = (C.depth_or_gap > 0) ? Vec2::scale(m, 1.0 / C.depth_or_gap)
                                    : Vec2{1, 0};
    return C;
  }

  auto E = epa(A, B, G.simplex, epaIters, EPSILON);
  if (!E.success) {
    C.intersect = true;
    C.depth_or_gap = 0.0;
    C.normal = {1, 0};
    C.pA = C.pB = {0, 0};
    return C;
  }
  C.intersect = true;
  C.depth_or_gap = -E.depth;
  C.normal = E.normal;  // A -> B
  C.pA = E.contactA;
  C.pB = E.contactB;
  return C;
}

}  // namespace geometry
