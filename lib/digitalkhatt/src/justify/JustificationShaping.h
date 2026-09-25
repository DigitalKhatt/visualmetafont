#pragma once

// Line analysis, shaping and measurement shared by the feature justifier and
// the DeclPolicy justifier. These are the parts that have nothing to do with
// which justification policy is running: they split a line into words and
// subwords, shape it, measure it, and turn a finished justification result
// into a laid-out line.

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <hb.h>

#include "FeatureJustifierInternal.h"

namespace digitalkhatt::justify::runtime {

inline constexpr int kFontSize = 1000;

inline bool contains(std::u16string_view text, char16_t character) {
  return text.find(character) != std::u16string_view::npos;
}

// "sk01" .. "sk20": the shrink features, applied whole-line in order.
std::string shrinkFeatureName(int index);

// Splits the line into words at spaces and words into joining subwords,
// recording where the bases are.  Every policy addresses glyphs through the
// indexes this produces.
LineTextInfo analyzeLineForJust(TextString lineText);

struct GlyphParameterAssignment {
  unsigned cluster;
  GlyphAxisId axis;
  double value;
  hb_codepoint_t substitute = static_cast<hb_codepoint_t>(-1);
};

hb_buffer_t* shape(TextString text, hb_font_t* font,
                   std::vector<hb_feature_t> features,
                   const FeatureJustificationLayout* layout = nullptr,
                   const std::vector<GlyphParameterAssignment>& parameters = {});
// Disables mark/mkmk for width measurement and glyph recognition. The returned
// buffer retains shaped glyphs and advances, but not final mark positioning.
hb_buffer_t* shapeForMeasurement(TextString text, hb_font_t* font, std::vector<hb_feature_t> features, const FeatureJustificationLayout* layout = nullptr, const std::vector<GlyphParameterAssignment>& parameters = {});
double getBufferWidth(hb_buffer_t* buffer);
double getWidth(const TextString& text, hb_font_t* font,
                const std::vector<hb_feature_t>& features);
// shapedWord optionally receives the measurement buffer; this avoids
// reshaping solely to recognize an accepted candidate.
double getWordWidth(
    const WordInfo& wordInfo,
    const std::map<int, std::vector<TextFontFeatures>>& justResults,
    hb_font_t* font, ShapingBuffer* shapedWord = nullptr, const FeatureJustificationLayout* layout = nullptr, std::span<const TextFontFeatures> globalFeatures = {}, const std::map<int, hb_codepoint_t>& substitutions = {}, const std::map<int, std::vector<TextFontFeatures>>& baseline = {});

std::map<int, std::vector<TextFontFeatures>> resolvedJustificationFeatures(const JustInfo& info);

LineLayoutInfo shapeLine(FeatureJustificationLayout& layout, int lineWidth,
                         int pageWidth, const LineTextInfo& lineTextInfo,
                         const JustResultByLine& justResult, bool tajweedColor,
                         double emScale, hb_font_t* font,
                         LineJustification justification, int& currentyPos,
                         digitalkhatt::TajweedMap& tajweedResult);

}  // namespace digitalkhatt::justify::runtime
