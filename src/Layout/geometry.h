
#pragma once
#include <QtGui/qpainterpath.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

struct mp_graphic_object;

namespace geometry {

// -------- basic types --------
struct Vec2 {
  double x = 0, y = 0;
  Vec2() = default;
  Vec2(double X, double Y) : x(X), y(Y) {}
  Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
  Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
  Vec2 operator*(double s) const { return {x * s, y * s}; }
  Vec2& operator+=(const Vec2& o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  Vec2 operator/(double s) const { return Vec2(x / s, y / s); }
  double lengthSq() const { return x * x + y * y; }

  double length() const { return std::sqrt(lengthSq()); }
  bool operator==(const Vec2& other) const {
    return this->x == other.x && this->y == other.y;
  }
  static double dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
  }
  static inline Vec2 sub(const Vec2& a, const Vec2& b) {
    return {a.x - b.x, a.y - b.y};
  }
  static inline Vec2 scale(const Vec2& a, double s) {
    return {a.x * s, a.y * s};
  }
  static inline double len(const Vec2& a) { return std::sqrt(dot(a, a)); }
  static inline Vec2 perp(const Vec2& a, const Vec2& b, const Vec2& c) {
    return (b * dot(a, c)) - (a * dot(b, c));
  }
};
using Poly = std::vector<Vec2>;

inline double dot(const Vec2& a, const Vec2& b) {
  return a.x * b.x + a.y * b.y;
}
inline double crossz(const Vec2& a, const Vec2& b) {
  return a.x * b.y - a.y * b.x;
}
inline double len(const Vec2& a) { return std::sqrt(dot(a, a)); }
inline Vec2 perp(const Vec2& a) { return Vec2{-a.y, a.x}; }
inline Vec2 normSafe(const Vec2& v) {
  double L = len(v);
  return (L > 0) ? v * (1.0 / L) : Vec2{1, 0};
}

inline double norm2(const Vec2& a) { return dot(a, a); }
inline double norm(const Vec2& a) { return std::sqrt(norm2(a)); }

// -------- Contact result --------
struct Contact {
  bool intersect = false;
  double depth_or_gap = 0.0;
  Vec2 normal{1, 0};  // A -> B
  Vec2 pA{0, 0}, pB{0, 0};
};

// Per-pair result with provenance
struct GSContact {
  Contact contact;            // intersect/gap, normal A->B, pA, pB, depth_or_gap
  size_t polyA = (size_t)-1;  // index in A.polys()
  size_t polyB = (size_t)-1;  // index in B.polys()
};

// -------- API --------
// Robust polygon orientation helpers (Y-up math)
void ensureClosed(Poly& p);
void makeCCW(Poly& p);

// Main: convex polygon vs convex polygon
Contact contactGjkEpaPoly(const Poly& A, const Poly& B, int gjkIters = 32,
                          int epaIters = 64);

struct Cubic {
  Vec2 p0, p1, p2, p3;
};

struct ContourCubic {
  std::vector<Cubic> segs;
};  // one closed contour (cubic segments)
struct GlyphCubic {
  std::vector<ContourCubic> paths;
};  // multiple contours

struct AABB {
  double minx, miny, maxx, maxy;
};

class GeometrySet {
 public:
  GeometrySet() = default;
  explicit GeometrySet(const std::vector<Poly>& polys) : polys_(polys) {
    computeAABB();
  }
  explicit GeometrySet(std::vector<Poly>&& polys) : polys_(std::move(polys)) {
    computeAABB();
  }
  explicit GeometrySet(std::vector<Poly>&& polys, std::vector<AABB>&& aabb, AABB bbox)
      : polys_(std::move(polys)), aabb_(std::move(aabb)), cachedAABB_(bbox) {
    // computeAABB();
    dirty_ = false;
  }

  // Access
  const std::vector<Poly>& polys() const { return polys_; }
  const std::vector<AABB>& aabb() const { return aabb_; }
  bool empty() const { return polys_.empty(); }
  size_t size() const { return polys_.size(); }
  const Poly& operator[](size_t i) const { return polys_[i]; }

  // Immutable transforms
  GeometrySet translated(double dx, double dy) const;
  GeometrySet scaled(double sx, double sy) const;
  GeometrySet flipY() const;  // y -> -y
  GeometrySet scaledY(double s) const { return scaled(1.0, s); }
  GeometrySet scaledX(double s) const { return scaled(s, 1.0); }

  // 2×2 matrix + translation affine transform:
  // [ a b | tx ]
  // [ c d | ty ]
  GeometrySet transformed(double a, double b, double c, double d,
                          double tx = 0.0, double ty = 0.0) const;

  GeometrySet scaleTranslate(double sx, double sy, double dx, double dy) const;

  AABB boundingAABB() const;

  QPainterPath toQPainterPath() const;

  void computeAABB() const;

 private:
  std::vector<Poly> polys_;
  mutable std::vector<AABB> aabb_;
  mutable bool dirty_ = true;
  mutable AABB cachedAABB_{0, 0, 0, 0};
  void markDirty() const { dirty_ = true; }
};

GSContact getDistance(const GeometrySet& A, const GeometrySet& B,
                      double maxAabbDistance);

GeometrySet buildConvexPartsFromCubics(const GlyphCubic& g, double tau);

AABB computeAABB(const Poly& P);

GlyphCubic getGlyphCubic(mp_graphic_object* body);

}  // namespace geometry
