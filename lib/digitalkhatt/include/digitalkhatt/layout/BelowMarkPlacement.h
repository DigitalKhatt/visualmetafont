#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

// Named defaults for callers and proofs; live constraints use OptParams.
inline constexpr double rehWawTipGap = RehWawPlacementParams{}.tipGap;
inline constexpr double belowDotKasraGap = RehWawPlacementParams{}.dotKasraGap;
inline constexpr double rehWawCompactInkGap = RehWawPlacementParams{}.compactGap;
inline constexpr double rehWawNeighborInkGap = RehWawPlacementParams{}.neighborGap;
inline constexpr double rehWawLeftOverhangFraction = RehWawPlacementParams{}.overhangPercent / 100.0;
inline constexpr double rehWawLeftOverhangLimit = RehWawPlacementParams{}.overhangLimit;

bool isKasraMark(const GlyphInstance& mark);
bool isRehWaw(const GlyphInstance& base);
bool isRehWawFollowingBase(const GlyphInstance& base);
// Call only after validating the owner and its preceding base on this page.
GlyphInstance* rehWawReference(const GlyphInstance& mark);
bool isRehWawDot(const GlyphInstance& mark);

}  // namespace digitalkhatt::layout
