#pragma once

// Types shared between the feature justifier and the declarative policy that
// runs on top of it.  Internal to src/justify: nothing here is part of the
// library's public interface.

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <hb.h>

#include <digitalkhatt/core/digitalkahtt_types.h>
#include <digitalkhatt/justify/FeatureJustifier.h>
#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

namespace digitalkhatt::justify::runtime {

using digitalkhatt::TextString;

struct ShapingBufferDeleter {
  void operator()(hb_buffer_t* buffer) const { hb_buffer_destroy(buffer); }
};
using ShapingBuffer = std::unique_ptr<hb_buffer_t, ShapingBufferDeleter>;

enum class SpaceType {
  Simple = 1,
  Aya,
};

enum class StretchType {
  None = 0,
  Beh = 1,
  FinaAscendant = 2,
  OtherKashidas = 3,
  Kaf = 4,
  SecondKashidaNotSameSubWord = 5,
  SecondKashidaSameSubWord = 6,
  BehNonGreedy = 7,
};

enum class AppliedResult {
  NoChange,
  Positive,
  Overflow,
  Forbiden,
};

struct SubWordCharIndex {
  int subWordIndex;
  int characterIndexInSubWord;
};

struct LayoutResult {
  double parWidth;
  std::map<StretchType, SubWordCharIndex> appliedKashidas;
};

struct SubWordInfo {
  std::vector<int> baseIndexes;
  TextString baseText;
};

struct WordInfo {
  TextString text;
  TextString baseText;
  int startIndex;
  int endIndex;
  std::vector<int> baseIndexes;
  std::vector<SubWordInfo> subwords = {{}};
};

struct LineTextInfo {
  TextString lineText;
  std::vector<int> ayaSpaceIndexes;
  std::vector<int> simpleSpaceIndexes;
  std::map<int, SpaceType> spaces;
  std::vector<WordInfo> wordInfos;
};

struct TextFontFeatures {
  std::string name;
  // OpenType feature values are integral. Native glyph parameters retain the
  // fractional values produced by proportional justification.
  double value;
  GlyphAxisId axis = NoGlyphAxis;
  bool operator==(const TextFontFeatures&) const = default;
};

struct JustResultByLine {
  float sclxAxis = 0;
  std::vector<TextFontFeatures> globalFeatures = {};
  /* FontFeatures by character index in the line */
  std::map<int, std::vector<TextFontFeatures>> fontFeatures = {};
  // Direct glyph replacements selected by native justification actions.
  // Native parameters remain in fontFeatures and compose with these glyphs.
  std::map<int, hb_codepoint_t> substitutions = {};
  double simpleSpacing;
  double ayaSpacing;
  double xScale;
  bool isShrink = false;
  double addedSpaceAfterShrink = 0.0;
};

struct JustInfo {
  std::map<int, std::vector<TextFontFeatures>> fontFeatures = {};
  // Initial connection spacing is a separate layer, not an accepted action.
  // Reads/guards observe fontFeatures; shaping applies these defaults first.
  std::map<int, std::vector<TextFontFeatures>> baselineFeatures = {};
  std::map<int, hb_codepoint_t> substitutions = {};
  std::vector<TextFontFeatures> globalFeatures;
  double desiredWidth;
  double textLineWidth;
  double initialLineWidth = 0;
  std::vector<LayoutResult> layoutResult;
  hb_font_t* font;
  const FeatureJustificationLayout* layout = nullptr;
  const CompiledJustificationCatalog* catalog = nullptr;
  int lineIndex = -1;
  hb_buffer_t* recognitionBuffer = nullptr;
  // Last accepted measurement per word, with word-relative clusters. Used
  // only for recognition; GSUB still replays from the original text in order.
  std::vector<ShapingBuffer> acceptedWordBuffers;
  DeclPolicyExecutionState declPolicyState;
  // Per character index: bit 2*i when the glyph there is in attachment i's
  // find set, bit 2*i+1 when it is in its skip set.  Empty when no attachment
  // matches glyphs.
  std::vector<std::uint64_t> attachmentGlyphs;
};

// Reshapes the word with the staged features, measures it, and keeps the
// result only if it widens the line without crossing the target width. With
// retainGlyphs, also commits the measured buffer into acceptedWordBuffers;
// rejected candidates are destroyed without replacing it.
// Defined in JustificationShaping.cpp.
AppliedResult tryApplyFeatures(
    int wordIndex, const LineTextInfo& lineTextInfo, JustInfo& justInfo,
    const std::map<int, std::vector<TextFontFeatures>>& newFeatures,
    bool retainGlyphs = false,
    const std::map<int, hb_codepoint_t>* newSubstitutions = nullptr);

}  // namespace digitalkhatt::justify::runtime
