#include "digitalkhatt/layout/BelowMarkPlacement.h"

namespace digitalkhatt::layout {

bool isKasraMark(const GlyphInstance& mark) {
  return mark.glyphName == "kasra" || mark.glyphName == "kasratan" ||
         mark.glyphName == "kasratanidgham";
}

bool isRehWaw(const GlyphInstance& base) {
  return !base.isMark && (base.glyphName.starts_with("reh.") || base.glyphName.starts_with("waw."));
}

bool isRehWawFollowingBase(const GlyphInstance& base) {
  return !base.isMark && (base.glyphName == "behshape.init" ||
                         base.glyphName == "behshape.init.beforeseen");
}

GlyphInstance* rehWawReference(const GlyphInstance& mark) {
  auto* owner = mark.prevBase;
  if (!owner || owner->lineIndex != mark.lineIndex) return nullptr;
  auto* preceding = owner->prevBase;
  if (!preceding || preceding->lineIndex != owner->lineIndex ||
      preceding->wordIndex != owner->wordIndex || !isRehWaw(*preceding) ||
      !isRehWawFollowingBase(*owner)) return nullptr;
  if (isKasraMark(mark) || isRehWawDot(mark)) return preceding;
  return nullptr;
}

bool isRehWawDot(const GlyphInstance& mark) {
  const auto* owner = mark.prevBase;
  return owner && isRehWawFollowingBase(*owner) &&
         (mark.glyphName == "onedotdown" || mark.glyphName == "twodotsdown");
}

}  // namespace digitalkhatt::layout
