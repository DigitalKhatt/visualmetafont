#include "contact_flexible.h"

#include <cmath>
#include <limits>

#include "geometry.h"

namespace geometry {

float dot_product(const Vec2& subject, const hull::Coordinate& direction) {
  float result = subject.x * direction.x;
  result += subject.y * direction.y;
  return result;
}

hull::Coordinate to_coordinate(const Vec2& subject) {
  return hull::Coordinate{(float)subject.x, (float)subject.y, 0.0};
}

// Small helpers
static inline Vec2 toVec2(const hull::Coordinate& c) {
  // Library stores 3D coords; we assume z == 0 for our 2D workflow
  return Vec2{c.x, c.y};
}

static inline Vec2 sub(const Vec2& a, const Vec2& b) {
  return Vec2{a.x - b.x, a.y - b.y};
}

static inline Vec2 normOr(const Vec2& v, const Vec2& fallback) {
  float l = len(v);
  if (l > 0.0) return Vec2{v.x / l, v.y / l};
  return fallback;
}

// ------------------------- ConvexShape builder -------------------------

Vector2D makeFlxConvexShape(const Poly& poly) { return Vector2D(poly); }

// ------------------------- Core adapter -------------------------

static Contact fromQueryResult(const flx::QueryResult& qr) {
  Contact c;

  const hull::Coordinate& a3 = qr.result.point_in_shape_a;
  const hull::Coordinate& b3 = qr.result.point_in_shape_b;

  c.pA = toVec2(a3);
  c.pB = toVec2(b3);

  Vec2 d = sub(c.pB, c.pA);
  double dist = len(d);

  if (qr.is_closest_pair_or_penetration_info) {
    // Closest points (no penetration)
    c.intersect = false;
    c.depth_or_gap = dist;
    c.normal = normOr(d, Vec2{1.0, 0.0});
  } else {
    // Penetration info: result is the extremals of the penetration vector
    // pointing from A to B (per library docs).
    c.intersect = true;
    c.depth_or_gap = -dist;  // treat as penetration depth
    c.normal = normOr(d, Vec2{1.0, 0.0});
  }

  return c;
}

Contact contactFlexiblePoly(const Poly& a, const Poly& b) {
  Contact c;

  if (a.size() < 2 || b.size() < 2)
    return c;  // degenerate; returns default non-intersect

  Vector2D sa = makeFlxConvexShape(a);
  Vector2D sb = makeFlxConvexShape(b);

  // One-shot query: either closest pair or penetration info
  flx::QueryResult qr = flx::get_closest_points_or_penetration_info(sa, sb);

  // If the library fails, you might add error status checks here
  return fromQueryResult(qr);
}

}  // namespace geometry
