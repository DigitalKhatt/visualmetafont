#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

inline constexpr double rehWawTipGap = 30.0;
inline constexpr double belowDotKasraGap = 40.0;

bool isKasraMark(const GlyphInstance& mark);
bool isRehWaw(const GlyphInstance& base);
bool isRehWawFollowingBase(const GlyphInstance& base);
// Call only after validating the owner and its preceding base on this page.
GlyphInstance* rehWawReference(const GlyphInstance& mark);
bool isRehWawDot(const GlyphInstance& mark);

}  // namespace digitalkhatt::layout
