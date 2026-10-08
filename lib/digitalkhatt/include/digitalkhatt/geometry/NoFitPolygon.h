#pragma once

#include <cstddef>
#include <memory>
#include "digitalkhatt/geometry/geometry.h"

namespace geometry {

struct NoFitPolygonStatistics {
  std::size_t shapes = 0, regions = 0, builds = 0, hits = 0, evictions = 0;
  std::size_t queries = 0, convexGjkQueries = 0, boundaryVertices = 0;
  double buildSeconds = 0.0, querySeconds = 0.0;
};

// Tests simple polygons only; degenerate/repeated edges are not eligible for
// the shared convex contact path. Both winding directions are accepted.
bool isConvexPolygon(const Poly& polygon);

// Experimental translational contact oracle. Shapes are interned by their
// scaled local coordinates, never their world positions. Each cached region
// is the union of ALL B - A convex sums, expanded by the requested clearance.
// Original glyph holes follow GeometrySet's existing filled-hole policy.
class NoFitPolygonCache {
 public:
  explicit NoFitPolygonCache(std::size_t maximumRegions = 16384);
  ~NoFitPolygonCache();
  NoFitPolygonCache(const NoFitPolygonCache&) = delete;
  NoFitPolygonCache& operator=(const NoFitPolygonCache&) = delete;
  std::size_t registerShape(const GeometrySet& local);
  GSContact contact(std::size_t shapeA, Vec2 originA,
                    std::size_t shapeB, Vec2 originB, double clearance);
  // Caller verifies both outlines and world geometries are single convex
  // polygons. Uses the default solver's exact floating-point contact routine.
  GSContact contactConvex(const GeometrySet& worldA, const GeometrySet& worldB,
                         double clearance);
  NoFitPolygonStatistics statistics() const;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace geometry
