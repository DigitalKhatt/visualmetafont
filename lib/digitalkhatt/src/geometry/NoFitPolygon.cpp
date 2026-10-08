#include "digitalkhatt/geometry/NoFitPolygon.h"

#include <chrono>
#include <cstdint>
#include <list>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include "clipper2/clipper.h"

namespace geometry {
// Existing Hertel-Mehlhorn decomposition; used only during cache construction.
std::vector<Poly> convexDecomposeBayazit(const Poly& implicitClosedCCW);

namespace {
using namespace Clipper2Lib;
using Clock = std::chrono::steady_clock;
constexpr double scale = 1000.0;
constexpr double arcError = 0.02; // font units; much smaller than flattening tolerance

double elapsed(Clock::time_point start) {
  return std::chrono::duration<double>(Clock::now() - start).count();
}
void mix(std::size_t& hash, std::size_t value) {
  hash ^= value + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
}
Paths64 quantize(const GeometrySet& geometry) {
  Paths64 result;
  for (const auto& poly : geometry.polys()) {
    Path64 path;
    for (const auto& p : poly) {
      if (!std::isfinite(p.x) || !std::isfinite(p.y) ||
          std::abs(p.x) > 1e9 || std::abs(p.y) > 1e9)
        throw std::invalid_argument("Invalid no-fit polygon coordinate");
      Point64 point(std::llround(p.x * scale), std::llround(p.y * scale));
      if (path.empty() || path.back() != point) path.push_back(point);
    }
    if (path.size() > 1 && path.front() == path.back()) path.pop_back();
    if (path.size() < 3 || Area(path) == 0.0) continue;
    if (!IsPositive(path)) std::reverse(path.begin(), path.end());
    result.push_back(std::move(path));
  }
  return result;
}
std::size_t hashPaths(const Paths64& paths) {
  std::size_t hash = paths.size();
  for (const auto& path : paths) {
    mix(hash, path.size());
    for (const auto& p : path) {
      mix(hash, std::hash<int64_t>{}(p.x));
      mix(hash, std::hash<int64_t>{}(p.y));
    }
  }
  return hash;
}
Path64 convexSum(const Path64& a, const Path64& b) {
  // B + (-A): merge the edge directions of two CCW convex polygons in O(n+m).
  // Negation preserves winding. Start both at their bottom-left vertex so the
  // edge sequences have the same angular origin; parallel edges advance together.
  if (a.size() < 3 || b.size() < 3) return {};
  std::size_t startA = 0, startB = 0;
  for (std::size_t i = 1; i < a.size(); ++i)
    if (a[i].y > a[startA].y || (a[i].y == a[startA].y && a[i].x > a[startA].x)) startA = i;
  for (std::size_t i = 1; i < b.size(); ++i)
    if (b[i].y < b[startB].y || (b[i].y == b[startB].y && b[i].x < b[startB].x)) startB = i;

  Path64 sum;
  sum.reserve(a.size() + b.size());
  std::size_t i = 0, j = 0;
  while (i < a.size() || j < b.size()) {
    const auto& pa = a[(startA+i) % a.size()];
    const auto& pb = b[(startB+j) % b.size()];
    const Point64 point(pb.x-pa.x, pb.y-pa.y);
    if (sum.empty() || sum.back() != point) sum.push_back(point);
    if (i == a.size()) { ++j; continue; }
    if (j == b.size()) { ++i; continue; }
    const auto& nextA = a[(startA+i+1) % a.size()];
    const auto& nextB = b[(startB+j+1) % b.size()];
    const Point64 edgeA(pa.x-nextA.x, pa.y-nextA.y);
    const Point64 edgeB(nextB.x-pb.x, nextB.y-pb.y);
    const long double cross = static_cast<long double>(edgeA.x)*edgeB.y -
        static_cast<long double>(edgeA.y)*edgeB.x;
    if (cross >= 0) ++i;
    if (cross <= 0) ++j;
  }
  if (sum.size() > 1 && sum.front() == sum.back()) sum.pop_back();
  return sum;
}
// Union paths have positive outer rings and negative holes. Even/odd containment
// preserves free pockets; querying individual rings without their union is wrong.
bool contains(const PolySet& rings, Vec2 p) {
  bool inside = false;
  for (const auto& ring : rings) {
    bool inRing = false;
    for (std::size_t i=0,j=ring.size()-1; i<ring.size(); j=i++) {
      const auto a=ring[i], b=ring[j];
      if ((a.y > p.y) != (b.y > p.y) &&
          p.x < (b.x-a.x)*(p.y-a.y)/(b.y-a.y)+a.x) inRing=!inRing;
    }
    if (inRing) inside=!inside;
  }
  return inside;
}
PolySet rings(const Paths64& paths) {
  PolySet result;
  for (const auto& path : paths) {
    if (path.size() < 3) continue;
    Poly poly;
    for (const auto& p : path) poly.emplace_back(p.x/scale,p.y/scale);
    result.push_back(std::move(poly));
  }
  return result;
}
struct Key {
  std::size_t a,b;
  double gap;
  bool operator==(const Key&) const = default;
};
struct KeyHash {
  std::size_t operator()(const Key& key) const {
    std::size_t hash=key.a; mix(hash,key.b);mix(hash,std::hash<double>{}(key.gap));return hash;
  }
};
struct Shape {
  Paths64 paths;
  std::optional<Paths64> convexParts;
};
struct Region {
  PolySet ink, expanded;
  AABB bounds;
  std::size_t vertices = 0;
  std::list<Key>::iterator lru;
};
}  // namespace

struct NoFitPolygonCache::Impl {
  explicit Impl(std::size_t capacity_) : capacity(std::max(std::size_t{1},capacity_)) {}
  std::size_t capacity;
  std::vector<Shape> shapes;
  std::unordered_map<std::size_t,std::vector<std::size_t>> shapeIds;
  std::unordered_map<Key,Region,KeyHash> regions;
  std::list<Key> lru;
  NoFitPolygonStatistics stats;

  const Paths64& parts(std::size_t id) {
    auto& shape=shapes.at(id);
    if (!shape.convexParts) {
      Paths64 result;
      for (const auto& path : shape.paths) {
        Poly polygon;
        for (const auto& p : path) polygon.emplace_back(p.x/scale,p.y/scale);
        if (isConvexPolygon(polygon)) {
          result.push_back(path);
          continue;
        }
        for (auto& part : convexDecomposeBayazit(polygon)) {
          auto quantized=quantize(GeometrySet(PolySet{std::move(part)}));
          result.insert(result.end(),quantized.begin(),quantized.end());
        }
      }
      shape.convexParts=std::move(result);
    }
    return *shape.convexParts;
  }
  Region& region(Key key) {
    auto found=regions.find(key);
    if (found!=regions.end()) {
      ++stats.hits;
      lru.splice(lru.begin(),lru,found->second.lru);
      return found->second;
    }
    const auto start=Clock::now();
    Paths64 sums;
    for (const auto& a : parts(key.a)) for (const auto& b : parts(key.b)) {
      auto sum=convexSum(a,b);
      if (!sum.empty()) sums.push_back(std::move(sum));
    }
    const auto ink=Union(sums,FillRule::NonZero);
    // Round joins approximate the same requested clearance used by GJK/EPA.
    // Arc/grid approximation remains, but there is no extra clearance margin.
    const auto expanded=key.gap > 0.0 ? InflatePaths(ink,key.gap*scale,
        JoinType::Round,EndType::Polygon,2.0,arcError*scale) : ink;
    Region result;
    result.ink=rings(ink); result.expanded=rings(expanded);
    result.bounds={INFINITY,INFINITY,-INFINITY,-INFINITY};
    for (const auto& ring : result.expanded) for (const auto& p : ring) {
      ++result.vertices;
      result.bounds.minx=std::min(result.bounds.minx,p.x);
      result.bounds.miny=std::min(result.bounds.miny,p.y);
      result.bounds.maxx=std::max(result.bounds.maxx,p.x);
      result.bounds.maxy=std::max(result.bounds.maxy,p.y);
    }
    if (regions.size() >= capacity) {
      const auto last=lru.back();
      stats.boundaryVertices-=regions.at(last).vertices;
      regions.erase(last);lru.pop_back();++stats.evictions;
    }
    lru.push_front(key);result.lru=lru.begin();
    stats.boundaryVertices+=result.vertices;
    ++stats.builds;stats.buildSeconds+=elapsed(start);
    return regions.emplace(key,std::move(result)).first->second;
  }
};

NoFitPolygonCache::NoFitPolygonCache(std::size_t maximumRegions)
    : impl_(std::make_unique<Impl>(maximumRegions)) {}
NoFitPolygonCache::~NoFitPolygonCache()=default;
bool isConvexPolygon(const Poly& polygon) {
  auto count = polygon.size();
  if (count > 1 && polygon.front() == polygon.back()) --count;
  if (count < 3) return false;
  int winding = 0;
  for (std::size_t i = 0; i < count; ++i) {
    const auto a = polygon[i], b = polygon[(i+1) % count], c = polygon[(i+2) % count];
    if (a == b) return false;
    const long double cross = static_cast<long double>(b.x-a.x)*(c.y-b.y) -
        static_cast<long double>(b.y-a.y)*(c.x-b.x);
    if (cross == 0) continue;
    const int sign = cross > 0 ? 1 : -1;
    if (winding && sign != winding) return false;
    winding = sign;
  }
  return winding != 0;
}

GSContact NoFitPolygonCache::contactConvex(const GeometrySet& worldA,
    const GeometrySet& worldB, double clearance) {
  ++impl_->stats.convexGjkQueries;
  return getDistance(worldA, worldB, clearance);
}
std::size_t NoFitPolygonCache::registerShape(const GeometrySet& local) {
  auto paths=quantize(local);
  auto& ids=impl_->shapeIds[hashPaths(paths)];
  for (auto id : ids) if (impl_->shapes[id].paths==paths) return id;
  auto id=impl_->shapes.size();
  impl_->shapes.push_back({std::move(paths),std::nullopt});ids.push_back(id);
  return id;
}
GSContact NoFitPolygonCache::contact(std::size_t shapeA, Vec2 originA,
    std::size_t shapeB, Vec2 originB, double clearance) {
  if (!std::isfinite(clearance) || clearance < 0.0)
    throw std::invalid_argument("Invalid no-fit polygon clearance");
  // Canonical pair order also reuses the cache when contact arguments reverse.
  const bool reversed=shapeA > shapeB;
  const Key key{std::min(shapeA,shapeB),std::max(shapeA,shapeB),clearance};
  const auto& region=impl_->region(key);
  const auto start=Clock::now();
  ++impl_->stats.queries;
  const auto t=reversed ? originB-originA : originA-originB;
  GSContact result;
  result.contact.depth_or_gap=INFINITY;
  // If the point is farther than the clearance from the expanded AABB, the
  // gap is already satisfied. The solver never needs the exact distant gap.
  const double boxDx=std::max({region.bounds.minx-t.x,0.0,t.x-region.bounds.maxx});
  const double boxDy=std::max({region.bounds.miny-t.y,0.0,t.y-region.bounds.maxy});
  if (boxDx*boxDx+boxDy*boxDy > clearance*clearance) {
    impl_->stats.querySeconds+=elapsed(start);return result;
  }
  Vec2 nearest, outward{1,0};
  double best=INFINITY;
  for (const auto& ring : region.expanded) for (std::size_t i=0; i<ring.size(); ++i) {
    const auto a=ring[i],b=ring[(i+1)%ring.size()],edge=b-a;
    const double parameter=std::clamp(dot(t-a,edge)/norm2(edge),0.0,1.0);
    const auto q=a+edge*parameter;
    const double distance=norm2(q-t);
    if (distance < best) {best=distance;nearest=q;outward=normSafe({edge.y,-edge.x});}
  }
  if (!std::isfinite(best)) {
    impl_->stats.querySeconds+=elapsed(start);return result;
  }
  const double distance=std::sqrt(best);
  const bool inside=contains(region.expanded,t);
  // Normal A -> B: XPBD applies -normal to A and +normal to B.
  Vec2 normal=distance > 1e-9 ? (inside ? t-nearest : nearest-t)/distance : outward*-1.0;
  if (reversed) normal=normal*-1.0;
  result.contact.normal=normal;
  result.contact.intersect=contains(region.ink,t);
  // This is clearance + signed distance to the EXPANDED region. Subtracting
  // clearance gives the correct whole-pair XPBD residual. It is not an exact
  // unexpanded penetration depth in concave pockets. No fake witness points.
  result.contact.depth_or_gap=clearance+(inside ? -distance : distance);
  impl_->stats.querySeconds+=elapsed(start);
  return result;
}
NoFitPolygonStatistics NoFitPolygonCache::statistics() const {
  auto result=impl_->stats;result.shapes=impl_->shapes.size();result.regions=impl_->regions.size();return result;
}
}  // namespace geometry
