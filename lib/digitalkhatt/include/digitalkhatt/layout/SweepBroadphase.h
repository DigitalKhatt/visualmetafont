#pragma once

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include "digitalkhatt/geometry/geometry.h"
#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

inline geometry::AABB padAABB(const geometry::AABB& b, double pad) {
  return {b.minx - pad, b.miny - pad, b.maxx + pad, b.maxy + pad};
}

struct SweepBroadphase {
  struct Box {
    int index;          // glyph index
    double minx, maxx;  // padded x-range
    double miny, maxy;  // padded y-range
  };

  std::vector<Box> boxes;  // all active AABBs (padded)

  SweepBroadphase(const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs,
                  const OptParams& P) {
    const double maxGap =
        std::max(P.minGapBody, P.minGapMark);
    const double maxShift =
        std::max({P.maxShiftBodyX, P.maxShiftBodyY, P.maxShiftMark});
    double pad = maxGap + 2.0 * maxShift;

    boxes.reserve(glyphs.size());
    for (int idx = 0; idx < static_cast<int>(glyphs.size()); ++idx) {
      geometry::AABB b = glyphs[idx].get().worldPolys.boundingAABB();
      b = padAABB(b, pad);
      boxes.push_back({idx, b.minx, b.maxx, b.miny, b.maxy});
    }

    std::sort(boxes.begin(), boxes.end(),
              [](const Box& a, const Box& b) {
                return a.minx < b.minx;
              });
  }

  // Run sweep and output candidate pairs
  void findPairs(std::vector<std::pair<int, int>>& pairs) const {
    pairs.clear();
    const size_t n = boxes.size();

    for (size_t i = 0; i < n; ++i) {
      const Box& A = boxes[i];
      // advance until B.minx > A.maxx
      for (size_t j = i + 1; j < n; ++j) {
        const Box& B = boxes[j];
        if (B.minx > A.maxx)
          break;  // rest are further right → done

        // check y-overlap
        if (!(A.maxy < B.miny || A.miny > B.maxy)) {
          A.index < B.index ? pairs.emplace_back(A.index, B.index) : pairs.emplace_back(B.index, A.index);
        }
      }
    }
  }
};

}  // namespace digitalkhatt::layout
