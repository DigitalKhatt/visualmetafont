#include "digitalkhatt/layout/MarkClassifier.h"

#include "digitalkhatt/layout/ClassMap.h"

namespace digitalkhatt::layout {

MarkRole classifyMark(GlyphInstance glyphInstrance) {
  auto& glyphName = glyphInstrance.glyphName;

  if (glyphName.starts_with("hamzaabove")) {
    return MarkRole::HamzaAbove;
  } else if (glyphName.starts_with("hamzabelow")) {
    return MarkRole::HamzaBelow;
  } else if (glyphName.starts_with("shadda")) {
    return MarkRole::Shadda;
  } else if (glyphName.starts_with("sukun")) {
    return MarkRole::Sukun;
  } else if (glyphName.starts_with("sukun")) {
    return MarkRole::Sukun;
  } else if (glyphName.starts_with("fatha") || glyphName.starts_with("damma") || glyphName.starts_with("kasra")) {
    return MarkRole::Haraka;
  } else if (glyphName.starts_with("maddahabove")) {
    return MarkRole::Maddah;
  } else if (containsSubstr(glyphName, "waqf")) {
    return MarkRole::WaqfSign;
  } else if (containsSubstr(glyphName, "dot")) {
    return MarkRole::Dots;
  } else {
    return MarkRole::None;
  }
}

}  // namespace digitalkhatt::layout
