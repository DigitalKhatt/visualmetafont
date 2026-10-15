#pragma once

#include <cstdint>
#include <vector>

class OtLayout;
struct LayoutPages;

namespace digitalkhatt {
// DKLY v4: final GlyphVis contours and integer placements in font units.
// The header includes uint32 pageWidth and minimum lineSpacing after the version.
// Line x is an inward offset from the right page edge, including the right margin.
// fontSize/xscale transform the whole line. Shared by GUI and native CLI.
std::vector<uint8_t> wholeGlyphLayoutBinary(OtLayout& layout, const LayoutPages& pages);
}
