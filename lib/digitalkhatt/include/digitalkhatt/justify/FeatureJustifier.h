#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <hb.h>

#include "digitalkhatt/core/digitalkahtt_types.h"
#include "digitalkhatt/justify/declpolicy/JustificationCandidate.h"
#include "digitalkhatt/justify/declpolicy/JustificationCatalog.h"

namespace digitalkhatt::justify {

class FeatureJustificationLayout {
 public:
  using JustificationTraceCallback = std::function<void(const JustificationDecisionTrace&)>;

  virtual ~FeatureJustificationLayout() = default;

  virtual hb_font_t* createFont(double scale, bool newFace) = 0;
  virtual int scaleBy() const = 0;
  virtual int topSpace() const = 0;
  virtual int interLineSpacing() const = 0;
  virtual std::string glyphName(hb_font_t* font,
                                hb_codepoint_t glyph) const = 0;
  // Logical glyph identity for policy matching. An exported parameter
  // instance is still its source glyph (fatha at length 2 is still fatha),
  // whereas a real substitution such as noon.fina -> noon.fina.expa changes it.
  virtual std::string recognitionGlyphName(hb_font_t* font, hb_codepoint_t glyph) const {
    return glyphName(font, glyph);
  }
  virtual const CompiledJustificationCatalog* justificationCatalog() const {
    return nullptr;
  }
  // Static OTF providers intentionally decline native parameter actions.
  virtual bool supportsGlyphParameters() const { return false; }
  // Missing capability excludes a site from non-substituting baseline spacing.
  virtual std::optional<double> glyphParameterMaximum(hb_codepoint_t, GlyphAxisId) const { return std::nullopt; }
  virtual bool setGlyphParameter(hb_glyph_info_t&, GlyphAxisId, double) const { return false; }
  void setGlyphParameter(GlyphLayoutInfo& glyph, GlyphAxisId axis, double value) const {
    glyph.parameters.set(axis, value);
  }
  void setLineParameter(LineLayoutInfo& line, GlyphAxisId axis, double value) const {
    for (auto& glyph : line.glyphs) setGlyphParameter(glyph, axis, value);
  }
  virtual GlyphParameters glyphParameters(const hb_glyph_info_t&, double left, double right) const {
    return {.lefttatweel = left, .righttatweel = right};
  }
  // Fast endpoint measurement used by proportional policies. Live providers
  // can measure one glyph instance without replaying GSUB and GPOS for its
  // complete word. Static providers return no value and use shaped fallback.
  virtual std::optional<double> glyphAdvance(hb_font_t*, hb_codepoint_t, const GlyphParameters&) const {
    return std::nullopt;
  }
  // Resolves a structural one-to-one GSUB lookup for an already-shaped glyph.
  // No result means that the lookup does not cover the glyph.
  virtual std::optional<hb_codepoint_t> resolveJustificationLookup(hb_codepoint_t, std::string_view) const { return std::nullopt; }
  void setJustificationTraceCallback(JustificationTraceCallback callback) { justificationTraceCallback_ = std::move(callback); }
  bool justificationTracingEnabled() const { return static_cast<bool>(justificationTraceCallback_); }
  void traceJustificationDecision(const JustificationDecisionTrace& decision) const {
    if (justificationTraceCallback_) justificationTraceCallback_(decision);
  }

 private:
  JustificationTraceCallback justificationTraceCallback_;
};

class FeatureJustifier {
 public:
  explicit FeatureJustifier(FeatureJustificationLayout& layout) : layout_(layout) {}

  std::vector<LineLayoutInfo> justifyPage(
      double emScale,
      int pageWidth,
      const std::vector<LineToJustify>& lines,
      bool newFace,
      bool tajweedColor,
      hb_buffer_cluster_level_t clusterLevel,
      JustOption justOption,
      const std::string& mushafLayout) const;

 private:
  FeatureJustificationLayout& layout_;
};

}  // namespace digitalkhatt::justify
