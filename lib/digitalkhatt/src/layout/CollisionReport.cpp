#include "digitalkhatt/layout/CollisionReport.h"

#include <algorithm>
#include <cmath>

namespace digitalkhatt::layout {

bool isCollisionSpace(std::string_view name) {
  return name.find("space") != name.npos || name.find("linefeed") != name.npos;
}

std::vector<CollisionContact> findCollisionContacts(
    const std::vector<CollisionGlyph>& glyphs, double minimumGap) {
  struct Box { std::size_t index; geometry::AABB bounds; };
  std::vector<Box> boxes;
  boxes.reserve(glyphs.size());
  const double pad = minimumGap / 2.0;
  for (std::size_t i = 0; i < glyphs.size(); ++i) {
    const auto& g = glyphs[i];
    if (isCollisionSpace(g.name) || g.geometry->empty()) continue;
    auto b = g.geometry->boundingAABB();
    boxes.push_back({i, {b.minx-pad, b.miny-pad, b.maxx+pad, b.maxy+pad}});
  }
  std::sort(boxes.begin(), boxes.end(), [](const Box& a, const Box& b) {
    return a.bounds.minx < b.bounds.minx;
  });
  std::vector<CollisionContact> contacts;
  for (std::size_t i = 0; i < boxes.size(); ++i) {
    const auto& a = boxes[i];
    for (std::size_t j = i + 1; j < boxes.size(); ++j) {
      const auto& b = boxes[j];
      if (b.bounds.minx > a.bounds.maxx) break;
      if (a.bounds.maxy < b.bounds.miny || a.bounds.miny > b.bounds.maxy) continue;
      // Input is in line/glyph order, so first precedes second in the source.
      const auto first = std::min(a.index, b.index), second = std::max(a.index, b.index);
      const auto& A = glyphs[first];
      const auto& B = glyphs[second];
      const bool sameLine = A.line == B.line;
      if (sameLine && A.word == B.word &&
          (B.name.find(".medi") != B.name.npos || B.name.find(".fina") != B.name.npos) &&
          (A.name.find(".init") != A.name.npos || A.name.find(".medi") != A.name.npos))
        continue; // intentional cursive connections
      if (!sameLine && !(A.isMark || B.isMark)) continue;
      const double gap = sameLine ? minimumGap * A.fontSize : minimumGap;
      const auto contact = geometry::getDistance(*A.geometry, *B.geometry, gap).contact;
      if (std::isfinite(contact.depth_or_gap) && contact.depth_or_gap < gap)
        contacts.push_back({first, second, contact, gap});
    }
  }
  return contacts;
}

} // namespace digitalkhatt::layout
