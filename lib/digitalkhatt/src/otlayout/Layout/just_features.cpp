#define NOMINMAX

#include "OtLayout.h"
#include "GlyphVis.h"

#include <digitalkhatt/justify/FeatureJustifier.h>
#include <digitalkhatt/justify/declpolicy/DeclPolicyPageJustifier.h>

namespace {

// Adapts the real (Qt) OtLayout to the small interface digitalkhatt::justify::FeatureJustifier
// needs, so the justification/regex algorithm itself (lib/digitalkhatt/src/justify/FeatureJustifier.cpp)
// has no Qt dependency.
class QtOtLayoutFontProvider final : public digitalkhatt::justify::FeatureJustificationLayout {
 public:
  explicit QtOtLayoutFontProvider(OtLayout& layout) : layout_(layout) {
    setJustificationTraceCallback(layout.justificationTraceCallback());
  }

  hb_font_t* createFont(double scale, bool newFace) override {
    return layout_.createFont(scale, newFace);
  }
  int scaleBy() const override { return OtLayout::SCALEBY; }
  int topSpace() const override { return OtLayout::TopSpace; }
  int interLineSpacing() const override { return OtLayout::InterLineSpacing; }
  std::string glyphName(hb_font_t*, hb_codepoint_t glyph) const override {
    const auto found = layout_.glyphNamePerCode.find(glyph);
    return found == layout_.glyphNamePerCode.end() ? std::string{}
                                                   : found->second;
  }
  std::string recognitionGlyphName(hb_font_t* font, hb_codepoint_t glyph) const override {
    const auto name = glyphName(font, glyph);
    const auto found = layout_.glyphs.find(name);
    if (found != layout_.glyphs.end() && found->second.isAlternate && !found->second.originalglyph.empty()) return found->second.originalglyph;
    return name;
  }
  const digitalkhatt::justify::CompiledJustificationCatalog*
  justificationCatalog() const override {
    if (!layout_.compiledJustificationCatalog) return nullptr;
    return &*layout_.compiledJustificationCatalog;
  }
  std::optional<hb_codepoint_t> resolveJustificationLookup(hb_codepoint_t glyph, std::string_view name) const override {
    return layout_.resolveJustificationLookup(glyph, name);
  }

 private:
  // Applied to final GSUB glyphs before default advances and GPOS. Resolving
  // first preserves the existing lookup-coordinate contributions on other axes.
  bool supportsGlyphParameters() const override { return true; }
  std::optional<double> glyphParameterMaximum(hb_codepoint_t code, digitalkhatt::GlyphAxisId axis) const override {
    const auto instance = layout_.resolveGlyphInstance(code);
    const auto found = layout_.expandableGlyphs.find(instance.sourceGlyph->name);
    if (found == layout_.expandableGlyphs.end()) return std::nullopt;
    if (axis == digitalkhatt::LeftTatweelAxis) return found->second.maxLeft;
    if (axis == digitalkhatt::RightTatweelAxis) return found->second.maxRight;
    return std::nullopt;
  }
  bool setGlyphParameter(hb_glyph_info_t& info, digitalkhatt::GlyphAxisId axis, double value) const override {
    if (axis >= layout_.axisRegistry.axes().size()) return false;
    auto parameters = layout_.glyphParameters(info);
    parameters.set(axis, value);
    layout_.setGlyphParameters(info, parameters);
    return true;
  }
  GlyphParameters glyphParameters(const hb_glyph_info_t& info, double, double) const override { return layout_.glyphParameters(info); }
  std::optional<double> glyphAdvance(hb_font_t* font, hb_codepoint_t glyph, const GlyphParameters& parameters) const override {
    return layout_.gethHorizontalAdvance(font, glyph, parameters, nullptr);
  }
  OtLayout& layout_;
};

}  // namespace

std::vector<LineLayoutInfo> OtLayout::justifyPageUsingFeatures(double emScale, int pageWidth, const std::vector<LineToJustify>& lines,
                                                               bool newFace, bool tajweedColor,
                                                               hb_buffer_cluster_level_t cluster_level,
                                                               JustOption justOption, std::string mushafLayout) {
  QtOtLayoutFontProvider fontProvider(*this);

  // The declarative engine is a separate justifier, not a branch inside the
  // feature one: it runs the declarative policy and nothing else.
  if (justOption.justType == JustType::DeclPolicy) {
    digitalkhatt::justify::DeclPolicyPageJustifier justifier(fontProvider);
    return justifier.justifyPage(emScale, pageWidth, lines, newFace, tajweedColor, cluster_level,
                                 justOption, mushafLayout);
  }

  digitalkhatt::justify::FeatureJustifier justifier(fontProvider);

  return justifier.justifyPage(emScale, pageWidth, lines, newFace, tajweedColor, cluster_level,
                               justOption, mushafLayout);
}
