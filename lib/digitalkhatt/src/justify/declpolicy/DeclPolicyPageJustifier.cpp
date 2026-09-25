#include <digitalkhatt/justify/declpolicy/DeclPolicyPageJustifier.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <hb.h>

#include "hb-font.hh"

#include <digitalkhatt/core/tajweed/tajweed_service.h>

#include "../FeatureJustifierInternal.h"
#include "DeclPolicyAdapter.h"
#include "JustificationLinePolicy.h"
#include "JustificationPagePolicy.h"
#include "../JustificationShaping.h"

namespace digitalkhatt::justify {
namespace {

using runtime::analyzeLineForJust;
using runtime::getBufferWidth;
using runtime::getWidth;
using runtime::JustInfo;
using runtime::JustResultByLine;
using runtime::kFontSize;
using runtime::LayoutResult;
using runtime::LineTextInfo;
using runtime::shapeForMeasurement;
using runtime::shapeLine;
using std::vector;

constexpr int FONTSIZE = runtime::kFontSize;

// Shaping and glyph staging stay behind a narrow backend; all line-level
// choices (branch precedence, step order, limits and feature tags) are data.
class ShapingLineBackend final : public decl::LineJustificationBackend {
 public:
  ShapingLineBackend(const LineTextInfo& text, JustInfo& info, FeatureJustificationLayout& layout, unsigned parameterQuantization, CandidateWidthMode candidateWidth) : text_(text), info_(info), layout_(layout), parameterQuantization_(parameterQuantization), candidateWidth_(candidateWidth), measuredWidths_{{{}, info.textLineWidth}} {}
  double measure(const vector<runtime::TextFontFeatures>& candidateFeatures, const vector<runtime::TextFontFeatures>& currentFeatures, double currentWidth) override {
    return currentWidth + measureCached(candidateFeatures) - measureCached(currentFeatures);
  }
  double applyStage(std::span<const PolicyPhase> phases, const vector<runtime::TextFontFeatures>& globalFeatures, double currentWidth) override {
    measuredWidths_.clear();
    synchronizeGlobalFeatures(globalFeatures);
    info_.textLineWidth = currentWidth;
    decl::applyDeclPolicyStage(text_, info_, phases, parameterQuantization_, candidateWidth_);
    return info_.textLineWidth;
  }
  double measureSclx(float value) override {
    auto* font = layout_.createFont(info_.font->x_scale / 1000, false);
    hb_font_set_variation(font, HB_TAG('S', 'C', 'L', 'X'), value);
    const double width = getWidth(text_.lineText, font, {});
    hb_font_destroy(font);
    return width;
  }

 private:
  void synchronizeGlobalFeatures(const vector<runtime::TextFontFeatures>& globalFeatures) {
    if (info_.globalFeatures == globalFeatures) return;
    info_.globalFeatures = globalFeatures;
    info_.declPolicyState.currentGlyphs.clear();
    info_.acceptedWordBuffers.resize(text_.wordInfos.size());
    for (std::size_t wordIndex = 0; wordIndex < text_.wordInfos.size(); ++wordIndex) {
      auto& wordBuffer = info_.acceptedWordBuffers[wordIndex];
      info_.layoutResult[wordIndex].parWidth = getWordWidth(text_.wordInfos[wordIndex], info_.fontFeatures, info_.font, &wordBuffer, info_.layout, info_.globalFeatures, info_.substitutions, info_.baselineFeatures);
    }
  }
  double measureCached(const vector<runtime::TextFontFeatures>& globalFeatures) {
    const auto cached = std::find_if(measuredWidths_.begin(), measuredWidths_.end(), [&](const auto& entry) { return entry.first == globalFeatures; });
    if (cached != measuredWidths_.end()) return cached->second;
    double width = 0;
    if (info_.fontFeatures.empty() && info_.baselineFeatures.empty() && info_.substitutions.empty()) {
      vector<hb_feature_t> features;
      features.reserve(globalFeatures.size());
      for (const auto& feature : globalFeatures) features.push_back({hb_tag_from_string(feature.name.c_str(), static_cast<int>(feature.name.size())), static_cast<std::uint32_t>(feature.value), 0u, static_cast<unsigned int>(-1)});
      width = getWidth(text_.lineText, info_.font, features);
    } else {
      width = measureWords(globalFeatures);
    }
    measuredWidths_.emplace_back(globalFeatures, width);
    return width;
  }

  double measureWords(const vector<runtime::TextFontFeatures>& globalFeatures) const {
    double width = 0;
    for (const auto& word : text_.wordInfos) width += getWordWidth(word, info_.fontFeatures, info_.font, nullptr, info_.layout, globalFeatures, info_.substitutions, info_.baselineFeatures);
    return width;
  }

  const LineTextInfo& text_;
  JustInfo& info_;
  FeatureJustificationLayout& layout_;
  unsigned parameterQuantization_;
  CandidateWidthMode candidateWidth_;
  vector<std::pair<vector<runtime::TextFontFeatures>, double>> measuredWidths_;
};

JustResultByLine justifyLine(const LineTextInfo& lineTextInfo, hb_font_t* font, double fontSizeLineWidthRatio, int spaceWidth, JustOption justOption, FeatureJustificationLayout& layout, int lineIndex) {
  const auto* catalog = layout.justificationCatalog();
  if (!catalog || !catalog->linePolicy) throw std::runtime_error("declarative-policy line justification requires linepolicy in table(justdfa)");
  const int stretchPolicyIndex = justOption.justStretchPolicy >= 0 ? justOption.justStretchPolicy : catalog->linePolicy->stretchPolicyIndex;
  const int shrinkPolicyIndex = justOption.justShrinkPolicy >= 0 ? justOption.justShrinkPolicy : catalog->linePolicy->shrinkPolicyIndex;
  const auto& stretchPolicy = catalog->stretchPolicy(stretchPolicyIndex);
  const auto& shrinkPolicy = catalog->shrinkPolicy(shrinkPolicyIndex);
  const double desiredWidth = FONTSIZE / fontSizeLineWidthRatio;
  vector<LayoutResult> layoutResult;
  for (const auto& word : lineTextInfo.wordInfos) layoutResult.push_back({getWidth(word.text, font, {}), {}});

  // Recognition shares the initial measurement buffer and does not need GPOS
  // mark offsets. Keep ownership here even if executing a policy throws.
  std::unique_ptr<hb_buffer_t, decltype(&hb_buffer_destroy)> recognitionBuffer(shapeForMeasurement(lineTextInfo.lineText, font, {}), hb_buffer_destroy);
  const double currentLineWidth = getBufferWidth(recognitionBuffer.get());
  JustInfo justInfo{.fontFeatures = {},
                    .desiredWidth = desiredWidth,
                    .textLineWidth = currentLineWidth,
                    .initialLineWidth = currentLineWidth,
                    .layoutResult = std::move(layoutResult),
                    .font = font,
                    .layout = &layout,
                    .catalog = catalog,
                    .lineIndex = lineIndex,
                    .recognitionBuffer = recognitionBuffer.get()};
  const auto parameterQuantization = currentLineWidth < desiredWidth ? stretchPolicy.parameterQuantization : shrinkPolicy.parameterQuantization;
  const auto candidateWidth = currentLineWidth < desiredWidth ? stretchPolicy.candidateWidth : shrinkPolicy.candidateWidth;
  ShapingLineBackend backend(lineTextInfo, justInfo, layout, parameterQuantization, candidateWidth);
  auto result = decl::executeLineJustificationPolicy(stretchPolicy.steps, shrinkPolicy.steps, {desiredWidth, currentLineWidth, static_cast<double>(spaceWidth), lineTextInfo.simpleSpaceIndexes.size(), lineTextInfo.ayaSpaceIndexes.size()}, backend);
  result.fontFeatures = justInfo.baselineFeatures.empty() ? std::move(justInfo.fontFeatures) : runtime::resolvedJustificationFeatures(justInfo);
  result.substitutions = std::move(justInfo.substitutions);
  return result;
}

}  // namespace

std::vector<LineLayoutInfo> DeclPolicyPageJustifier::justifyPage(double emScale, int pageWidth, const std::vector<LineToJustify>& lines, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t clusterLevel, JustOption justOption, const std::string& mushafLayout) const {
  using FontOwner = std::unique_ptr<hb_font_t, decltype(&hb_font_destroy)>;
  FontOwner defaultShapeFont(layout_.createFont(emScale, newFace), hb_font_destroy);
  FontOwner justifyFont(layout_.createFont(1, false), hb_font_destroy);
  const auto* catalog = layout_.justificationCatalog();
  if (!catalog || !catalog->pagePolicy) throw std::runtime_error("declarative-policy page justification requires pagepolicy in table(justdfa)");
  const auto& policy = *catalog->pagePolicy;
  const auto& render = decl::pageRenderRule(policy, justOption.justStyle);

  // The policy chooses which lines contribute. Measuring and font ownership
  // remain adapter work; the ratio decision itself is independent of shaping.
  const int spaceWidth = getWidth(u" ", justifyFont.get(), {});
  vector<LineTextInfo> linesTextInfo;
  vector<double> ratios;
  linesTextInfo.reserve(lines.size());
  for (const auto& line : lines) {
    linesTextInfo.push_back(analyzeLineForJust(line.text));
    if (!decl::pageMeasuresLine(policy, line)) continue;
    const double fontSizeLineWidthRatio = static_cast<double>(FONTSIZE) * emScale / line.width;
    const double desiredWidth = FONTSIZE / fontSizeLineWidthRatio;
    const double width = getWidth(line.text, justifyFont.get(), {});
    if (width > 0) ratios.push_back(desiredWidth / width);
  }
  const double fontRatio = decl::pageFontRatio(policy, justOption.justStyle, ratios);

  digitalkhatt::PageTajweedResult tajweedResults(lines.size());
  if (tajweedColor) {
    digitalkhatt::tajweed::TajweedService tajweedService;
    tajweedResults = tajweedService.applyTajweedByPage(lines, mushafLayout.find("indopak") != std::string::npos);
  }
  vector<LineLayoutInfo> page;
  page.reserve(lines.size());
  int currentyPos = layout_.topSpace() << layout_.scaleBy();
  for (std::size_t index = 0; index < lines.size(); ++index) {
    const auto& line = lines[index];
    const auto& lineTextInfo = linesTextInfo[index];
    const auto& treatment = decl::pageLineRule(policy, line);
    JustResultByLine result;
    if (treatment.justify) {
      const double ratio = line.width != 0 ? static_cast<double>(FONTSIZE) * emScale / line.width : 1;
      result = justifyLine(lineTextInfo, justifyFont.get(), ratio * fontRatio, spaceWidth, justOption, layout_, static_cast<int>(index));
    } else {
      result.simpleSpacing = result.ayaSpacing = spaceWidth;
      result.xScale = 1;
    }
    for (const auto& tag : treatment.features) result.globalFeatures.push_back({tag, 1});

    const double newEmScale = decl::pageLineEmScale(render, emScale, fontRatio, result);
    FontOwner ownedShapeFont(nullptr, hb_font_destroy);
    auto* shapeFont = defaultShapeFont.get();
    if (fontRatio != 1 || newEmScale != emScale || result.sclxAxis != 0) {
      ownedShapeFont.reset(layout_.createFont(newEmScale, false));
      shapeFont = ownedShapeFont.get();
    }
    if (result.sclxAxis != 0) hb_font_set_variation(shapeFont, HB_TAG('S', 'C', 'L', 'X'), result.sclxAxis);
    auto lineLayout = shapeLine(layout_, line.width, pageWidth, lineTextInfo, result, tajweedColor, newEmScale, shapeFont, line.lineJustification, currentyPos, tajweedResults[index]);
    lineLayout.type = line.lineType;
    decl::finishPageLine(render, result, lineLayout);
    page.push_back(std::move(lineLayout));
  }
  return page;
}

}  // namespace digitalkhatt::justify
