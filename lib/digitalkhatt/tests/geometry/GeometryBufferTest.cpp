#include <iostream>
#include <stdexcept>

#include "digitalkhatt/geometry/geometry.h"

using namespace geometry;

namespace {
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

void equalBounds(const AABB& a, const AABB& b) {
  require(a.minx == b.minx && a.miny == b.miny && a.maxx == b.maxx && a.maxy == b.maxy,
      "Reusable translations must preserve exact cached bounds");
}

void equivalent(const GeometrySet& actual, const GeometrySet& expected) {
  require(actual.size() == expected.size(), "Polygon counts differ");
  equalBounds(actual.boundingAABB(), expected.boundingAABB());
  for (size_t i = 0; i < actual.size(); ++i) {
    require(actual[i].size() == expected[i].size(), "Vertex counts differ");
    equalBounds(actual.aabb()[i], expected.aabb()[i]);
    for (size_t j = 0; j < actual[i].size(); ++j)
      require(actual[i][j].x == expected[i][j].x && actual[i][j].y == expected[i][j].y,
          "Translation arithmetic must remain identical");
  }
}

void absoluteTranslationsReuseStorage() {
  const GeometrySet original(std::vector<Poly>{
      {{0.1,0.2},{100,0.3},{80.25,130.1},{-30,75}},
      {{200,10},{220,10},{220,30},{200,30}}});
  GeometrySet output;
  output.assignTranslated(original, 0, 0);
  const auto* outerStorage = output.polys().data();
  const auto* firstStorage = output[0].data();
  const auto* secondStorage = output[1].data();
  const auto* boundsStorage = output.aabb().data();
  for (int i = 0; i < 1000; ++i) {
    const double x = (i % 2 ? 1e9 : -1e9) + i * 0.013;
    const double y = (i % 3 ? -50000 : 90000) - i * 0.019;
    output.assignTranslated(original, x, y);
    equivalent(output, original.translate(x, y));
    require(output.polys().data() == outerStorage && output[0].data() == firstStorage &&
            output[1].data() == secondStorage && output.aabb().data() == boundsStorage,
        "Unchanged topology must reuse polygon, vertex and bounds allocations");
  }
  // Returning to zero after large excursions must not accumulate drift.
  output.assignTranslated(original, 0, 0);
  equivalent(output, original);
  output.assignTranslated(output, 7, -3);
  equivalent(output, original.translate(7, -3));
}

void changingTopologyAndEmptyInput() {
  GeometrySet output;
  for (const auto& source : {GeometrySet(std::vector<Poly>{{{0,0},{10,0},{0,10}}}),
           GeometrySet{}, GeometrySet(std::vector<Poly>{{},{{1,1},{2,2}}}),
           GeometrySet(std::vector<Poly>{{{0,0},{5,0},{5,5},{0,5}}})}) {
    output.assignTranslated(source, 25, -40);
    equivalent(output, source.translate(25, -40));
  }
}
} // namespace

int main() {
  try {
    absoluteTranslationsReuseStorage();
    changingTopologyAndEmptyInput();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
