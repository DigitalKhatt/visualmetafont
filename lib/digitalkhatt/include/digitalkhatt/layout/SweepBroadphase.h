#pragma once

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
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
    // Each box supplies half the requested clearance; a larger tuned gap
    // must not disappear outside the candidate horizon.
    const double pad = std::max(P.broadphasePadding,
        0.5 * std::max({P.sameWordGap, P.differentWordGap, P.differentLineGap, 40.0}));

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

  // Y buckets contain bits in the original X-sweep order. Visiting set bits
  // in ascending rank retains the exact projection order, including X ties.
  void findPairs(std::vector<std::pair<int, int>>& pairs) const {
    pairs.clear();
    const size_t n = boxes.size();
    if (n < 2) return;

    const auto emptyX = [](const Box& box) {
      return box.minx == std::numeric_limits<double>::infinity() &&
             box.maxx == -std::numeric_limits<double>::infinity();
    };
    double bottom = std::numeric_limits<double>::infinity(), top = -bottom;
    for (const auto& box : boxes) {
      // Empty geometry sorts last and cannot pass the old X-sweep test.
      if (emptyX(box)) continue;
      if (!std::isfinite(box.minx) || !std::isfinite(box.maxx) ||
          !std::isfinite(box.miny) || !std::isfinite(box.maxy) ||
          box.minx > box.maxx || box.miny > box.maxy) {
        findPairsSweep(pairs);
        return;
      }
      bottom = std::min(bottom, box.miny);
      top = std::max(top, box.maxy);
    }
    const double height = top - bottom;
    if (!(height > 0.0) || !std::isfinite(height)) {
      findPairsSweep(pairs);
      return;
    }

    constexpr size_t bucketCount = 64;
    const size_t wordCount = (n + 63) / 64;
    std::vector<std::uint64_t> buckets(bucketCount * wordCount, 0);
    const auto bucketFor = [&](double y) {
      // Divide first to avoid overflowing on large finite coordinates.
      return std::min(bucketCount - 1,
          static_cast<size_t>(((y - bottom) / height) * bucketCount));
    };
    for (size_t rank = 0; rank < n; ++rank) {
      const auto& box = boxes[rank];
      if (emptyX(box)) continue;
      const size_t lastBucket = bucketFor(box.maxy);
      for (size_t bucket = bucketFor(box.miny); bucket <= lastBucket; ++bucket)
        buckets[bucket * wordCount + rank / 64] |= std::uint64_t{1} << (rank % 64);
    }

    for (size_t i = 0; i + 1 < n; ++i) {
      const auto& a = boxes[i];
      if (emptyX(a)) continue;
      const size_t end = std::upper_bound(boxes.begin() + i + 1, boxes.end(), a.maxx,
          [](double x, const Box& b) { return x < b.minx; }) - boxes.begin();
      if (end == i + 1) continue;
      const size_t firstBucket = bucketFor(a.miny), lastBucket = bucketFor(a.maxy);
      for (size_t word = (i + 1) / 64; word <= (end - 1) / 64; ++word) {
        std::uint64_t candidates = 0;
        for (size_t bucket = firstBucket; bucket <= lastBucket; ++bucket)
          candidates |= buckets[bucket * wordCount + word];
        if (word == (i + 1) / 64) candidates &= ~std::uint64_t{0} << ((i + 1) % 64);
        if (word == end / 64 && end % 64) candidates &= (std::uint64_t{1} << (end % 64)) - 1;
        while (candidates) {
          const size_t j = word * 64 + std::countr_zero(candidates);
          candidates &= candidates - 1;
          const auto& b = boxes[j];
          if (!(a.maxy < b.miny || a.miny > b.maxy))
            a.index < b.index ? pairs.emplace_back(a.index, b.index) : pairs.emplace_back(b.index, a.index);
        }
      }
    }
  }

 private:
  // Preserve the established handling of empty/non-finite boxes and ranges
  // that cannot be indexed safely.
  void findPairsSweep(std::vector<std::pair<int, int>>& pairs) const {
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
