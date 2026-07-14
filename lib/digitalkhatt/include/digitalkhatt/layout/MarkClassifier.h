#pragma once

#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

enum class MarkRole {
  None,
  Haraka,  // fatha/damma/kasra/tanween
  Shadda,
  Sukun,
  Maddah,
  HamzaAbove,
  HamzaBelow,
  WaqfSign,
  Dots
};

MarkRole classifyMark(GlyphInstance glyphInstrance);

}  // namespace digitalkhatt::layout
