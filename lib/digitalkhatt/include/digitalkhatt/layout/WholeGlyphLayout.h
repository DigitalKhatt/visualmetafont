#pragma once

#include <cstdint>
#include <vector>

class OtLayout;
struct LayoutPages;

namespace digitalkhatt {
// DKLY v4: final GlyphVis contours and rounded layout placements. Shared by
// the GUI and native CLI; no interpolation or Qt dependency.
std::vector<uint8_t> wholeGlyphLayoutBinary(OtLayout& layout, const LayoutPages& pages);
}
