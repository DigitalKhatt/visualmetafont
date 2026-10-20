#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

#include "digitalkhatt/layout/SweepBroadphase.h"

using namespace digitalkhatt::layout;

namespace {
using Pairs = std::vector<std::pair<int, int>>;

// Exhaustive reference in the original X-rank order, independent of buckets.
Pairs referencePairs(const SweepBroadphase& sweep) {
  Pairs result;
  for (size_t i = 0; i < sweep.boxes.size(); ++i) {
    const auto& a = sweep.boxes[i];
    for (size_t j = i + 1; j < sweep.boxes.size(); ++j) {
      const auto& b = sweep.boxes[j];
      if (b.minx > a.maxx) break;
      if (!(a.maxy < b.miny || a.miny > b.maxy))
        result.emplace_back(std::min(a.index, b.index), std::max(a.index, b.index));
    }
  }
  return result;
}

void check(const std::vector<geometry::AABB>& input, double padding) {
  std::vector<std::reference_wrapper<GlyphInstance>> refs;
  OptParams params;
  SweepBroadphase sweep(refs, params);
  for (const auto& box : input) {
    const auto padded = padAABB(box, padding);
    sweep.boxes.push_back({static_cast<int>(sweep.boxes.size()),
        padded.minx, padded.maxx, padded.miny, padded.maxy});
  }
  std::sort(sweep.boxes.begin(), sweep.boxes.end(),
      [](const auto& a, const auto& b) { return a.minx < b.minx; });
  Pairs actual;
  sweep.findPairs(actual);
  if (actual != referencePairs(sweep))
    throw std::runtime_error("Broadphase must preserve every pair and its X-sweep projection order");
}

void boundariesAndEmptyGeometry() {
  check({}, 0);
  check({{0,0,1,1}}, 140);
  // Equal X, touching Y edges, negative coordinates, long vertical spans,
  // and ranks on both sides of the 64-bit word boundary.
  std::vector<geometry::AABB> boxes;
  for (int i = 0; i < 193; ++i)
    boxes.push_back({double(i % 3), double(i - 100), 100, double(i - 99)});
  boxes.push_back({-20,-1000,20,1000});
  const double inf = std::numeric_limits<double>::infinity();
  boxes.push_back({inf,inf,-inf,-inf});
  check(boxes, 0);
  check(boxes, 140);
  for (int count : {63,64,65,127,128,129}) {
    std::vector<geometry::AABB> fullWord;
    for (int i = 0; i < count; ++i) fullWord.push_back({0,0,100,1});
    check(fullWord, 0); // Exclusive X endpoint exactly on a bitset-word boundary.
  }
  check({{inf,inf,-inf,-inf},{inf,inf,-inf,-inf}}, 0);
  check({{-10,0,10,0},{-10,0,10,0}}, 0); // Collapsed Y range.
  check({{-10,0,10,inf},{0,-1,10,1}}, 0); // Non-finite fallback.
  check({{-10,-1e308,10,1e308},{0,0,10,1}}, 0); // Overflowing Y range.
  check({{0,10,10,5},{0,0,10,20}}, 0); // Preserve invalid-box fallback.
}

void randomizedLayouts() {
  std::mt19937 random(152871);
  for (int run = 0; run < 80; ++run) {
    std::vector<geometry::AABB> boxes;
    const int count = run == 0 ? 1777 : 300;
    const double scale = run % 5 == 0 ? 0.001 : run % 5 == 1 ? 1e6 : 1;
    for (int i = 0; i < count; ++i) {
      const double x = double(random() % 160) * 100;
      const double y = int(random() % 26000) - 26000;
      const double width = random() % 1000;
      const double height = run % 7 == 0 ? 20000 : random() % 1500;
      boxes.push_back({x * scale, y * scale, (x + width) * scale, (y + height) * scale});
    }
    check(boxes, (run % 3 == 0 ? 140 : run % 3 == 1 ? 0 : 1000) * scale);
  }
}
} // namespace

int main() {
  try {
    boundariesAndEmptyGeometry();
    randomizedLayouts();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
