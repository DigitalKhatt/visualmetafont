#pragma once

#include <algorithm>
#include <set>
#include <vector>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

// Include both participants and their owning bases, expanding each seed to its
// entire word. Cross-word/line diagnostics must show the competing attachment.
inline std::vector<int> violationContextIndices(const std::vector<GlyphInstance*>& flat,
    const std::vector<int>& lineStart, const ConstraintViolation& v) {
  std::set<int> selected;
  auto addWord = [&](int index) {
    if (index < 0 || index >= static_cast<int>(flat.size()) || lineStart.size() < 2) return;
    const auto upper = std::upper_bound(lineStart.begin(), lineStart.end(), index);
    const int line = static_cast<int>(upper - lineStart.begin()) - 1;
    int left = index, right = index;
    while (left > lineStart[line] && flat[left - 1]->glyphName != "space") --left;
    while (right + 1 < lineStart[line + 1] && flat[right + 1]->glyphName != "space") ++right;
    for (int i = left; i <= right; ++i)
      if (flat[i]->glyphName != "space") selected.insert(i);
  };
  for (int index : {v.glyphA, v.glyphB}) {
    if (index < 0 || index >= static_cast<int>(flat.size())) continue;
    addWord(index);
    const auto owner = std::find(flat.begin(), flat.end(), flat[index]->prevBase);
    if (flat[index]->isMark && owner != flat.end()) addWord(static_cast<int>(owner - flat.begin()));
  }
  return {selected.begin(), selected.end()};
}

}  // namespace digitalkhatt::layout
