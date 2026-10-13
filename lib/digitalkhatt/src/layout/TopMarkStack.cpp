#include "digitalkhatt/layout/TopMarkStack.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"

#include <algorithm>
#include <string>
#include <unordered_set>

namespace digitalkhatt::layout {
namespace {
struct StackSlot {
  std::unordered_set<std::string> aboveNames;
  std::unordered_set<std::string> belowNames;
  double offsetX = 0.0;  // target centerX(below) - centerX(above)
  double minGap = 40.0;
  bool outlineClearance = false;
  std::optional<double> preferredGap;
};
using StackTable = std::vector<StackSlot>;

// Rows describe direct neighbors in the full shaped glyph sequence.
// Open/plain and idgham tanween share the same authored relationships.
const StackTable& topStackTable() {
  static const StackTable table = {
      {{"shadda"}, {"onedotup", "twodotsup", "three_dots"}},
      {{"fatha"}, {"shadda"}, 0, 60, true},
      {{"fatha.stretched"}, {"shadda"}, 0, 60, true, 80},
      {{"fathatanidgham", "fathatan"}, {"shadda"}, 0, 0},
      {{"damma", "dammatanidgham", "dammatan"}, {"shadda"}, 30, 0},
      {{"fatha", "fatha.stretched", "fathatanidgham", "fathatan", "damma", "dammatanidgham", "dammatan"}, {"hamzaabove", "hamzaabove.joined", "hamzaabove.lamalef"}, 30, 0},
      {{"fatha", "fatha.stretched", "fathatanidgham", "fathatan", "damma", "dammatanidgham", "dammatan"}, {"onedotup", "twodotsup", "three_dots"}, 30, 10},
      {{"maddahabove"}, {"smallalef.isol", "smallalef.replacement", "smallalef.joined"}, 30, 0}};
  return table;
}

}  // namespace

std::vector<TopMarkStackLink> buildTopMarkStackLinks(
    const std::vector<GlyphInstance*>& group) {
  auto ordered = group;
  std::sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) {
    return a->glyphIndex < b->glyphIndex;
  });
  std::vector<TopMarkStackLink> links;
  // Keep projection order unchanged. Row order no longer selects a parent:
  // original glyph indices retain adjacency even when group members are filtered.
  for (const auto& rule : topStackTable()) {
    for (size_t i = 1; i < ordered.size(); ++i) {
      auto* below = ordered[i - 1];
      auto* above = ordered[i];
      if (above->lineIndex == below->lineIndex &&
          above->glyphIndex == below->glyphIndex + 1 &&
          rule.belowNames.contains(below->glyphName) &&
          rule.aboveNames.contains(above->glyphName))
        links.push_back({below, above, above->preserveGposHorizontal ? (boxCenterX(*below) - below->dx) - (boxCenterX(*above) - above->dx) : rule.offsetX, rule.minGap, rule.outlineClearance, rule.preferredGap});
    }
  }
  return links;
}
}  // namespace digitalkhatt::layout
