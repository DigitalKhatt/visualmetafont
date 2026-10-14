#pragma once

#include <cstdint>
#include <vector>

class OtLayout;
struct LayoutPages;

namespace digitalkhatt {
// DKLY v4: final GlyphVis contours and integer placements in font units.
// fontSize/xscale transform the whole line. Shared by GUI and native CLI.
std::vector<uint8_t> wholeGlyphLayoutBinary(OtLayout& layout, const LayoutPages& pages);
}
