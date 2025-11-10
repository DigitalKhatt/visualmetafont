#pragma once

#include <optional>
#include <vector>

#include "geometry.h"

// Flexible-GJK-and-EPA headers (paths may differ slightly by version)
#include <Flexible-GJK-and-EPA/GjkEpa.h>  // or the umbrella header documented in the repo
#include <Flexible-GJK-and-EPA/shape/PointCloud.h>

namespace geometry {

hull::Coordinate to_coordinate(const Vec2& subject);

float dot_product(const Vec2& subject, const hull::Coordinate& direction);

class Vector2D
    : public flx::shape::PointCloud<std::vector<Vec2>::const_iterator> {
 public:
  Vector2D(const std::vector<Vec2>& wrapped)
      : flx::shape::PointCloud<std::vector<Vec2>::const_iterator>(
            wrapped.begin(), wrapped.end(), dot_product, to_coordinate) {};
};

// Build a Flexible-GJK-and-EPA convex shape from a 2D convex polygon.
// We embed (x,y) -> (x,y,0).
Vector2D makeFlxConvexShape(const Poly& poly);

// Single convex poly vs poly
Contact contactFlexiblePoly(const Poly& a, const Poly& b);

}  // namespace geometry
