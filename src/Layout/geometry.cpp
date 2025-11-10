#include "geometry.h"

#include <iostream>

#include "clipper2/clipper.h"
#include "contact_flexible.h"
#include "metafont.h"
#include "polypartition.h"

namespace geometry {
// ---------- utilities ----------

static inline Vec2 tripleProduct(const Vec2& a, const Vec2& b, const Vec2& c) {
  // (a x b) x c in 2D -> b*(c·a) - a*(c·b)
  double ac = dot(c, a), bc = dot(c, b);
  return Vec2{b.x * ac - a.x * bc, b.y * ac - a.y * bc};
}

static double polygonAreaSignedOld(const Poly& p) {
  if (p.size() < 2) return 0.0;
  double A = 0;
  for (size_t i = 1; i < p.size(); ++i) A += crossz(p[i - 1], p[i]);
  return 0.5 * A;
}
double polygonAreaSigned(const Poly& p) {
  if (p.size() < 2) return 0.0;
  double A = 0;
  int n = (int)p.size();
  for (int i = 0; i < n; i++) {
    int i2 = i + 1;
    if (i2 == n) i2 = 0;
    A += crossz(p[i], p[i2]);
  }
  return 0.5 * A;
}
void ensureClosed(Poly& p) {
  if (p.empty()) return;
  const Vec2& a = p.front();
  const Vec2& b = p.back();
  if (std::abs(a.x - b.x) > 1e-12 || std::abs(a.y - b.y) > 1e-12)
    p.push_back(a);
}
void makeCCW(Poly& p) {
  if (polygonAreaSigned(p) < 0) std::reverse(p.begin(), p.end());
}

static inline Vec2 applyTransform(const Vec2& p, double a, double b, double c,
                                  double d, double tx, double ty) {
  return {a * p.x + b * p.y + tx, c * p.x + d * p.y + ty};
}

static inline double applyScaleTranslate(double val, double s, double d) {
  return s * val + d;
};

static inline Vec2 applyScaleTranslate(const Vec2& p, double sx, double sy,
                                       double dx, double dy) {
  return {sx * p.x + dx, sy * p.y + dy};
}

GeometrySet GeometrySet::translated(double dx, double dy) const {
  return transformed(1, 0, 0, 1, dx, dy);
}

GeometrySet GeometrySet::scaled(double sx, double sy) const {
  return transformed(sx, 0, 0, sy, 0, 0);
}

GeometrySet GeometrySet::flipY() const {
  return transformed(1, 0, 0, -1, 0, 0);
}

GeometrySet GeometrySet::transformed(double a, double b, double c, double d,
                                     double tx, double ty) const {
  std::vector<Poly> out;
  out.reserve(polys_.size());
  for (const auto& P : polys_) {
    Poly Q;
    Q.reserve(P.size());
    for (auto& v : P) {
      Q.push_back(applyTransform(v, a, b, c, d, tx, ty));
    }
    out.push_back(std::move(Q));
  }
  return GeometrySet(std::move(out));
}
//  We suppose that y does not change orientation (i.e. sy > 0)
GeometrySet GeometrySet::scaleTranslate(double sx, double sy, double dx,
                                        double dy) const {
  std::vector<Poly> out;
  std::vector<AABB> aabb;
  out.reserve(polys_.size());
  if (!dirty_) {
    aabb.reserve(polys_.size());
  }

  for (int i = 0; i < polys_.size(); ++i) {
    auto& P = polys_[i];
    Poly Q;
    Q.reserve(P.size());
    for (const auto& v : P) {
      Q.emplace_back(applyScaleTranslate(v, sx, sy, dx, dy));
    }
    if (!dirty_) {
      auto& paabb = aabb_[i];
      aabb.emplace_back(AABB{applyScaleTranslate(paabb.minx, sx, dx),
                             applyScaleTranslate(paabb.miny, sy, dy),
                             applyScaleTranslate(paabb.maxx, sx, dx),
                             applyScaleTranslate(paabb.maxy, sy, dy)});
    }

#if false
    AABB test{applyScaleTranslate(paabb.minx, sx, dx),
              applyScaleTranslate(paabb.miny, sy, dy),
              applyScaleTranslate(paabb.maxx, sx, dx),
              applyScaleTranslate(paabb.maxy, sy, dy)};
    auto test2 = ::geometry::computeAABB(Q);
    if (true) {
      auto tt = 5;
    }
#endif

    out.emplace_back(std::move(Q));
  }
  if (!dirty_) {
    return GeometrySet(std::move(out),
                       std::move(aabb),
                       AABB{applyScaleTranslate(cachedAABB_.minx, sx, dx),
                            applyScaleTranslate(cachedAABB_.miny, sy, dy),
                            applyScaleTranslate(cachedAABB_.maxx, sx, dx),
                            applyScaleTranslate(cachedAABB_.maxy, sy, dy)});
  } else {
    return GeometrySet(std::move(out));
  }
}

void GeometrySet::computeAABB() const {
  AABB b;
  std::vector<AABB> aabb;
  b.minx = b.miny = +std::numeric_limits<double>::infinity();
  b.maxx = b.maxy = -std::numeric_limits<double>::infinity();

  aabb.reserve(polys_.size());

  for (const auto& P : polys_) {
    AABB paabb;
    paabb.minx = paabb.miny = +std::numeric_limits<double>::infinity();
    paabb.maxx = paabb.maxy = -std::numeric_limits<double>::infinity();
    for (const auto& v : P) {
      paabb.minx = std::min(paabb.minx, v.x);
      paabb.miny = std::min(paabb.miny, v.y);
      paabb.maxx = std::max(paabb.maxx, v.x);
      paabb.maxy = std::max(paabb.maxy, v.y);
    }
    b.minx = std::min(b.minx, paabb.minx);
    b.miny = std::min(b.miny, paabb.miny);
    b.maxx = std::max(b.maxx, paabb.maxx);
    b.maxy = std::max(b.maxy, paabb.maxy);
    aabb.emplace_back(std::move(paabb));
  }

  cachedAABB_ = b;
  aabb_ = std::move(aabb);
  dirty_ = false;
}

AABB GeometrySet::boundingAABB() const {
  if (dirty_) computeAABB();
  return cachedAABB_;
}

QPainterPath GeometrySet::toQPainterPath() const {
  QPainterPath path;

  for (const auto& P : polys_) {
    const size_t n = P.size();
    if (n == 0) continue;

    // Move to first point
    path.moveTo(P[0].x, P[0].y);

    // Draw edges
    for (size_t i = 1; i < n; ++i) {
      path.lineTo(P[i].x, P[i].y);
    }

    // Ensure closure
    if (std::abs(P.back().x - P.front().x) > 1e-12 ||
        std::abs(P.back().y - P.front().y) > 1e-12) {
      path.closeSubpath();
    } else {
      path.closeSubpath();
    }
  }

  return path;
}

// ------------------------- GeometrySet vs GeometrySet
// -------------------------

AABB computeAABB(const Poly& p) {
  AABB b;
  if (p.empty()) {
    b.minx = b.miny = 0.0;
    b.maxx = b.maxy = 0.0;
    return b;
  }
  b.minx = b.maxx = p[0].x;
  b.miny = b.maxy = p[0].y;
  for (const auto& v : p) {
    if (v.x < b.minx) b.minx = v.x;
    if (v.x > b.maxx) b.maxx = v.x;
    if (v.y < b.miny) b.miny = v.y;
    if (v.y > b.maxy) b.maxy = v.y;
  }
  return b;
}

static double aabbDistanceSquared(const AABB& a, const AABB& b) {
  double dx = 0.0;
  if (a.maxx < b.minx)
    dx = b.minx - a.maxx;
  else if (b.maxx < a.minx)
    dx = a.minx - b.maxx;

  double dy = 0.0;
  if (a.maxy < b.miny)
    dy = b.miny - a.maxy;
  else if (b.maxy < a.miny)
    dy = a.miny - b.maxy;

  if (dx == 0.0 && dy == 0.0) return 0.0;
  return dx * dx + dy * dy;
  // std::sqrt(dx * dx + dy * dy);
}

GSContact getDistance(const GeometrySet& A, const GeometrySet& B,
                      double maxAabbDistance) {
  GSContact best;

  best.contact.intersect = false;
  best.contact.depth_or_gap = std::numeric_limits<double>::infinity();

  double maxAabbDistanceSquared = maxAabbDistance * maxAabbDistance;

  double dAabb = aabbDistanceSquared(A.boundingAABB(), B.boundingAABB());
  if (dAabb > maxAabbDistanceSquared) return best;  // too far, skip

  for (size_t i = 0; i < A.size(); ++i) {
    auto& pa = A[i];
    if (pa.size() < 2) continue;
    auto& aabbA = A.aabb()[i];

    for (size_t j = 0; j < B.size(); ++j) {
      auto& pb = B[j];
      if (pb.size() < 2) continue;
      auto& aabbB = B.aabb()[j];

      double dAabb = aabbDistanceSquared(aabbA, aabbB);
      if (dAabb > maxAabbDistanceSquared) continue;  // too far, skip
      if (std::sqrt(dAabb) >= best.contact.depth_or_gap &&
          best.contact.intersect == false)
        continue;  // already have closer separated pair

      Contact c = contactGjkEpaPoly(pa, pb);
      Contact ctest = contactFlexiblePoly(pa, pb);
      // Flexible-GJK-and-EPA is buggy for EPA
      if (c.depth_or_gap > 0 &&
          std::fabs(c.depth_or_gap - ctest.depth_or_gap) > 0.01) {
        std::cout << c.depth_or_gap << " != " << ctest.depth_or_gap
                  << std::endl;
      }

      if (c.intersect) {
        if (!best.contact.intersect ||
            c.depth_or_gap < best.contact.depth_or_gap) {
          best.contact = c;
          best.polyA = i;
          best.polyB = j;
        }
      } else {
        if (!best.contact.intersect &&
            c.depth_or_gap < best.contact.depth_or_gap) {
          best.contact = c;
          best.polyA = i;
          best.polyB = j;
        }
      }
    }
  }

  return best;
}

// =====================================================
// 1) Flatten cubic Béziers (adaptive subdivision)
// =====================================================
static inline double distPtSeg(const Vec2& p, const Vec2& a, const Vec2& b) {
  Vec2 ab{b.x - a.x, b.y - a.y}, ap{p.x - a.x, p.y - a.y};
  double t = (dot(ap, ab)) / std::max(1e-20, dot(ab, ab));
  t = std::clamp(t, 0.0, 1.0);
  Vec2 c{a.x + ab.x * t, a.y + ab.y * t};
  return norm(p - c);
}
static inline double cubicFlatness(const Cubic& c) {
  return std::max(distPtSeg(c.p1, c.p0, c.p3), distPtSeg(c.p2, c.p0, c.p3));
}
static void splitCubicMid(const Cubic& c, Cubic& L, Cubic& R) {
  Vec2 p01{(c.p0.x + c.p1.x) * 0.5, (c.p0.y + c.p1.y) * 0.5};
  Vec2 p12{(c.p1.x + c.p2.x) * 0.5, (c.p1.y + c.p2.y) * 0.5};
  Vec2 p23{(c.p2.x + c.p3.x) * 0.5, (c.p2.y + c.p3.y) * 0.5};
  Vec2 p012{(p01.x + p12.x) * 0.5, (p01.y + p12.y) * 0.5};
  Vec2 p123{(p12.x + p23.x) * 0.5, (p12.y + p23.y) * 0.5};
  Vec2 p0123{(p012.x + p123.x) * 0.5, (p012.y + p123.y) * 0.5};
  L = {c.p0, p01, p012, p0123};
  R = {p0123, p123, p23, c.p3};
}

static void flattenCubicAppend(const Cubic& c, double tau, Poly& out,
                               bool& started) {
  std::vector<Cubic> stack;
  stack.push_back(c);
  // We only emit c.p0 once for the whole contour (when !started)
  bool willEmitP0 = !started;
  while (!stack.empty()) {
    Cubic s = stack.back();
    stack.pop_back();
    if (cubicFlatness(s) <= tau) {
      if (willEmitP0) {
        out.push_back(s.p0);
        willEmitP0 = false;
        started = true;
      }
      out.push_back(s.p3);
    } else {
      Cubic L, R;
      splitCubicMid(s, L, R);
      stack.push_back(R);
      stack.push_back(L);
    }
  }
}

std::vector<Poly> flattenCubicContours(const GlyphCubic& g, double tau) {
  std::vector<Poly> polys;
  polys.reserve(g.paths.size());
  for (const auto& path : g.paths) {
    if (path.segs.empty()) continue;
    Poly poly;
    poly.reserve(path.segs.size() * 4);
    bool started = false;
    for (const auto& seg : path.segs) {
      flattenCubicAppend(seg, tau, poly, started);
    }
    assert(poly.back() == poly.front());
    poly.pop_back();
    // ensureClosed(poly);
    // dedupePolyline(poly);
    polys.push_back(std::move(poly));
  }
  return polys;
}

std::vector<Poly> unionNoHoles(const std::vector<Poly>& polys, double scale) {
  using namespace Clipper2Lib;
  Paths64 subject;
  subject.reserve(polys.size());
  const double SCALE = scale;
  for (auto& p : polys) {
    Path64 path;
    path.reserve(p.size());
    for (auto& v : p)
      path.push_back(Point64{(long long)std::llround(v.x * SCALE),
                             (long long)std::llround(v.y * SCALE)});
    subject.push_back(std::move(path));
  }
  Paths64 sol = Union(subject, FillRule::NonZero);
  // Filter: keep only outer (area > 0 in our coordinate convention)
  std::vector<Poly> out;
  for (auto& q : sol) {
    double A = 0;
    for (size_t i = 1; i < q.size(); ++i)
      A += (double)q[i - 1].x * q[i].y - (double)q[i - 1].y * q[i].x;
    if (A <= 0) continue;  // drop holes Poly poly;
    Poly poly;
    poly.reserve(q.size() + 1);
    for (auto& pt : q) poly.push_back(Vec2{pt.x / SCALE, pt.y / SCALE});
    out.push_back(std::move(poly));
  }
  return out;
}

static TPPLPoly toTPPL(const Poly& P) {
  TPPLPoly poly;
  const int n = P.size();

  poly.Init(n);
  for (int i = 0; i < n; ++i) {
    poly[i].x = P[i].x;
    poly[i].y = P[i].y;
  }
  poly.SetHole(false);
  // Already TPPL_ORIENTATION_CCW from Clipper2
  // poly.SetOrientation(TPPL_ORIENTATION_CCW);
  return poly;
}

static Poly fromTPPL(const TPPLPoly& p) {
  Poly out;
  const int n = p.GetNumPoints();
  out.reserve(n + 1);
  for (int i = 0; i < n; ++i) {
    out.push_back({p[i].x, p[i].y});
  }
  // normalize for our pipeline: CLOSED + CCW
  if (out.size() >= 3) {
    // Close by adding the first point
    if (std::abs(out.front().x - out.back().x) > 1e-12 ||
        std::abs(out.front().y - out.back().y) > 1e-12) {
      out.push_back(out.front());
    }
  }
  return out;
}
std::vector<Poly> convexDecomposeBayazit(const Poly& implicitClosedCCW) {
  // already CCW due to clipper2 union

  TPPLPoly in = toTPPL(implicitClosedCCW);

  TPPLPartition part;
  TPPLPolyList result;

  // Choose one:
  // - ConvexPartition_HM : Hertel-Mehlhorn (fast, few parts, great default)
  // - ConvexPartition_OPT: Optimal (minimal #parts), O(n^3), slower
  bool ok = part.ConvexPartition_HM(&in, &result);
  // If you prefer optimal:
  // bool ok = part.ConvexPartition_OPT(&in, &result);

  std::vector<Poly> out;
  if (ok) {
    out.reserve(result.size());
    for (auto& rp : result) {
      out.push_back(fromTPPL(rp));  // CLOSED + CCW
    }
    // Defensive: drop any degenerate slivers
    out.erase(std::remove_if(out.begin(), out.end(),
                             [](const Poly& q) { return q.size() < 4; }),
              out.end());
    return out;
  } else {
    throw new std::runtime_error("convexDecomposeBayazit error");
  }
}

// Convenience
GeometrySet buildConvexPartsFromCubics(const GlyphCubic& g, double tau) {
  auto flats = flattenCubicContours(g, tau);
  auto outers = unionNoHoles(flats, 1000);
  std::vector<Poly> parts;
  for (auto outer : outers) {
    auto dec = convexDecomposeBayazit(outer);
    parts.insert(parts.end(), dec.begin(), dec.end());
  }
  return GeometrySet(std::move(parts));
}

inline bool aabbOverlap(const AABB& a, const AABB& b) {
  return !(a.maxx < b.minx || b.maxx < a.minx || a.maxy < b.miny ||
           b.maxy < a.miny);
}

// Ensure CLOSED + CCW for robust normals; GJK/EPA tolerates open, but we
// normalize.
static inline void normalizeConvex(Poly& p) {
  if (p.empty()) return;
  // close
  if (std::abs(p.front().x - p.back().x) > 1e-12 ||
      std::abs(p.front().y - p.back().y) > 1e-12) {
    p.push_back(p.front());
  }
  // CCW
  makeCCW(p);
}

static ContourCubic getContour(mp_gr_knot h) {
  mp_gr_knot p, q;
  ContourCubic contour;
  // path.setFillRule(Qt::OddEvenFill);
  if (h == NULL) return contour;

  // path.moveTo(h->x_coord, h->y_coord);
  p = h;
  do {
    q = p->next;
    // path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord,
    //              q->y_coord);

    contour.segs.push_back({{p->x_coord, p->y_coord},
                            {p->right_x, p->right_y},
                            {q->left_x, q->left_y},
                            {q->x_coord, q->y_coord}});

    p = q;
  } while (p != h);

  return contour;
}

GlyphCubic getGlyphCubic(mp_graphic_object* body) {
  GlyphCubic glyphCubic;

  if (body) {
    do {
      switch (body->type) {
        case mp_fill_code: {
          glyphCubic.paths.push_back(
              getContour(((mp_fill_object*)body)->path_p));

          break;
        }
        default:
          break;
      }

    } while (body = body->next);
  }

  return glyphCubic;
}

}  // namespace geometry
