#pragma once

#include <string_view>
#include <digitalkhatt/core/GlyphParameters.h>

namespace digitalkhatt::justify::decl {
// Native deltas compose with parameters set by GSUB. A TAG_axis attribute is
// read-only: it queries the contextual parameter change made by that feature.
inline bool isParameterDelta(std::string_view name) { return name.ends_with("_delta"); }
inline bool isFeatureParameterQuery(std::string_view name) {
  return name.size() > 5 && name[4] == '_' && !isParameterDelta(name);
}
inline GlyphAxisId parameterAttributeAxis(const GlyphAxisRegistry& axes, std::string_view name) {
  if (const auto axis = axes.find(name); axis != NoGlyphAxis) return axis;
  if (isParameterDelta(name)) return axes.find(name.substr(0, name.size() - 6));
  if (isFeatureParameterQuery(name)) return axes.find(name.substr(5));
  return NoGlyphAxis;
}
} // namespace digitalkhatt::justify::decl
