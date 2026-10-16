#include "JustificationShaping.h"
#include <digitalkhatt/justify/declpolicy/DependentMarkWidth.h>

#include <cmath>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <set>
#include <sstream>
#include <utility>

#include "hb-buffer.hh"
#include "hb-font.hh"
#include <hb-ot.h>

namespace digitalkhatt::justify::runtime {
namespace {

using std::map;
using std::set;
using std::vector;

const TextString rightNoJoinLetters = u"آاٱأإدذرزوؤءة";
const TextString dualJoinLetters = u"بتثجحخسشصضطظعغفقكلمنهيئى";

set<char16_t> bases{};

void initBases() {
  for (int i = 0; i < static_cast<int>(dualJoinLetters.size()); i++) {
    bases.insert(dualJoinLetters.at(i));
  }
  for (int i = 0; i < static_cast<int>(rightNoJoinLetters.size()); i++) {
    bases.insert(rightNoJoinLetters.at(i));
  }
}

hb_segment_properties_t savedprops{
    HB_DIRECTION_RTL,
    HB_SCRIPT_ARABIC,
    hb_language_from_string("ar", strlen("ar")),
    0,
    0};
std::map<std::string, int> tajweedNameToColor = {
    {"green", 0x00A650FF}, {"tafkim", 0x006694FF}, {"lgray", 0xB4B4B4FF}, {"lkalkala", 0x00ADEFFF}, {"red1", 0xC38A08FF}, {"red2", 0xF47216FF}, {"red3", 0xEC008CFF}, {"red4", 0x8C0000FF}};
}  // namespace

std::string shrinkFeatureName(int index) {
  std::ostringstream stream;
  stream << "sk" << std::setw(2) << std::setfill('0') << index;
  return stream.str();
}

LineTextInfo analyzeLineForJust(TextString lineText) {
  if (bases.size() == 0) {
    initBases();
  }

  LineTextInfo lineTextInfo = {
      .lineText = lineText,
      .ayaSpaceIndexes = {},
      .simpleSpaceIndexes = {},
      .spaces = {},
      .wordInfos = {}};

  lineTextInfo.wordInfos.push_back({});
  WordInfo* currentWord = &lineTextInfo.wordInfos.back();
  currentWord->startIndex = 0;
  currentWord->endIndex = -1;

  for (int i = 0; i < static_cast<int>(lineText.size()); i++) {
    char16_t qchar = lineText.at(i);
    if (qchar == ' ') {
      if ((i > 0 && lineText.at(i - 1) >= 0x0660 && lineText.at(i - 1) <= 0x0669) ||
          (i + 1 < static_cast<int>(lineText.size()) && lineText.at(i + 1) == 0x06DD)) {
        lineTextInfo.ayaSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({i, SpaceType::Aya});
      } else {
        lineTextInfo.simpleSpaceIndexes.push_back(i);
        lineTextInfo.spaces.insert({i, SpaceType::Simple});
      }
      lineTextInfo.wordInfos.push_back({});
      currentWord = &lineTextInfo.wordInfos.back();
      currentWord->startIndex = i + 1;
      currentWord->endIndex = i;
    } else {
      currentWord->text += qchar;
      if (bases.find(qchar) != bases.end()) {
        currentWord->baseText += qchar;
        currentWord->baseIndexes.push_back(i - currentWord->startIndex);
        if (qchar == U'ء') {
          currentWord->subwords.push_back({.baseIndexes = {}, .baseText = u""});
        }
        auto& subWord = currentWord->subwords.back();
        subWord.baseText += qchar;
        subWord.baseIndexes.push_back(i - currentWord->startIndex);
        if (i < static_cast<int>(lineText.size()) - 1 && qchar != U'ء' && contains(rightNoJoinLetters, qchar)) {
          currentWord->subwords.push_back({.baseIndexes = {}, .baseText = u""});
        }
      }
      currentWord->endIndex++;
    }
  }

  return lineTextInfo;
}

static void substituteAttachment(hb_glyph_info_t& mark, GlyphAxisId changedAxis, const FeatureJustificationLayout& layout, std::span<const ResolvedAttachmentSubstitution> substitutions) {
  for (const auto& substitution : substitutions) {
    if (substitution.axis != changedAxis || !substitution.matches(mark.codepoint)) continue;
    mark.codepoint = substitution.select(layout.glyphParameters(mark, 0, 0));
    break;
  }
}

// Attachments are resolved in logical character order, not buffer order (RTL
// and mark reordering may reverse it). Stop at an undeclared intervening glyph;
// never borrow a Fatha from the next base or across a word boundary.
static void applyAttachments(std::u16string_view text, hb_font_t* font, hb_buffer_t* buffer, const vector<hb_codepoint_t>& originals, const vector<GlyphParameters>& baselineParameters, const FeatureJustificationLayout& layout, LineJustificationOperation operation, bool deform) {
  const auto* catalog = layout.justificationCatalog();
  if (!catalog) return;
  const auto substitutions = layout.attachmentSubstitutions();
  vector<int> nextBases;
  if (deform && std::any_of(catalog->attachments.begin(), catalog->attachments.end(), [&](const auto& attachment) { return attachment.appliesDuring(operation) && !attachment.growthConnection.empty(); })) {
    // Arabic GSUB is still in logical order here; HB reverses RTL glyphs
    // after GPOS. Use adjacency, not cluster sorting, so GSUB splits with
    // equal clusters retain their order. Marks/format controls are skipped.
    // Reuse the policy's joining-subword analysis: a neighboring base in the
    // same word is not necessarily joined (e.g. a base following Reh/Waw).
    vector<int> subwords(text.size(), -1);
    const auto analyzed = analyzeLineForJust(TextString(text));
    int subwordId = 0;
    for (const auto& word : analyzed.wordInfos) {
      for (const auto& subword : word.subwords) {
        for (const auto position : subword.baseIndexes) subwords[word.startIndex + position] = subwordId;
        ++subwordId;
      }
    }
    nextBases.resize(buffer->len, -1);
    int next = -1;
    for (unsigned index = buffer->len; index-- > 0;) {
      const auto& glyph = buffer->info[index];
      const auto character = glyph.cluster < text.size() ? text[glyph.cluster] : 0;
      const auto category = hb_unicode_general_category(hb_buffer_get_unicode_funcs(buffer), character);
      if (category == HB_UNICODE_GENERAL_CATEGORY_SPACE_SEPARATOR || category == HB_UNICODE_GENERAL_CATEGORY_LINE_SEPARATOR || category == HB_UNICODE_GENERAL_CATEGORY_PARAGRAPH_SEPARATOR || category == HB_UNICODE_GENERAL_CATEGORY_CONTROL || character == 0x200C || character == 0x06DD) { next = -1; continue; }
      if (hb_ot_layout_get_glyph_class(hb_font_get_face(font), glyph.codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK || category == HB_UNICODE_GENERAL_CATEGORY_FORMAT || character == 0x034F) continue;
      if (next >= 0 && glyph.cluster < subwords.size() && buffer->info[next].cluster < subwords.size() && subwords[glyph.cluster] >= 0 && subwords[glyph.cluster] == subwords[buffer->info[next].cluster]) nextBases[index] = next;
      next = static_cast<int>(index);
    }
  }
  for (unsigned base = 0; base < buffer->len; ++base) {
    auto& info = buffer->info[base];
    // A contextual GSUB extension is not a justification response. Keep its
    // original identity unless an attachment actually changes its width axis.
    if (hb_ot_layout_get_glyph_class(hb_font_get_face(font), info.codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK) continue;
    if (!deform) continue;
    const auto parameters = layout.glyphParameters(info, 0, 0);
    const auto baseName = layout.recognitionGlyphName(font, info.codepoint);
    for (const auto& attachment : catalog->attachments) {
      if (!attachment.appliesDuring(operation)) continue;
      if (attachment.widthAxis == NoGlyphAxis && attachment.responses.empty()) continue;
      if (!attachment.excludeMarkGlyphNames.empty()) {
        bool excluded = false;
        // The blocker may be after Fatha/Shadda or several format controls:
        // inspect this base's entire mark run, not only attachment.within.
        for (unsigned index = base + 1; index < buffer->len; ++index) {
          const auto& mark = buffer->info[index];
          if (hb_ot_layout_get_glyph_class(hb_font_get_face(font), mark.codepoint) != HB_OT_LAYOUT_GLYPH_CLASS_MARK) {
            const auto character = mark.cluster < text.size() ? text[mark.cluster] : 0;
            if (character == 0x034F || hb_unicode_general_category(hb_buffer_get_unicode_funcs(buffer), character) == HB_UNICODE_GENERAL_CATEGORY_FORMAT) continue;
            break;
          }
          const auto name = layout.recognitionGlyphName(font, mark.codepoint);
          if (std::binary_search(attachment.excludeMarkGlyphNames.begin(), attachment.excludeMarkGlyphNames.end(), name)) { excluded = true; break; }
        }
        if (excluded) continue;
      }
      const JustMarkResponse* response = nullptr;
      for (const auto& candidate : attachment.responses)
        if (std::binary_search(candidate.glyphNames.begin(), candidate.glyphNames.end(), baseName)) { response = &candidate; break; }
      if (response && response->mode == MarkResponseMode::Keep) continue;
      const bool direct = response && response->mode == MarkResponseMode::Parameter;
      if (!direct && attachment.widthAxis == NoGlyphAxis) continue;
      const bool absolute = response && response->mode == MarkResponseMode::WidthRatio;
      if (!direct && !absolute && parameters.isDefault() && info.codepoint == originals[base]) continue;
      std::optional<double> growth;
      if (!direct) {
        if (!response && !attachment.growthConnection.empty()) {
          const auto next = nextBases[base];
          const auto nextParameters = next >= 0 ? layout.glyphParameters(buffer->info[next], 0, 0) : GlyphParameters{};
          growth = dependentMarkConnectionWidthGrowth(parameters, attachment.growthConnection[0], next >= 0 ? &nextParameters : nullptr, attachment.growthConnection[1],
            [&](const auto& measured) { return layout.glyphInkWidth(info.codepoint, measured); },
            [&](const auto& measured) -> std::optional<double> { return next >= 0 ? layout.glyphInkWidth(buffer->info[next].codepoint, measured) : std::nullopt; }, baselineParameters[base], next >= 0 ? &baselineParameters[next] : nullptr);
          if (!growth || *growth <= 0) continue;
        } else if (!response && !attachment.growthAxes.empty()) {
          growth = dependentMarkAxisWidthGrowth(parameters, attachment.growthAxes, [&](const auto& measured) { return layout.glyphInkWidth(info.codepoint, measured); }, baselineParameters[base]);
          if (!growth || *growth <= 0) continue;
        } else {
          const auto natural = layout.glyphDesignWidth(originals[base], baselineParameters[base]);
          const auto current = layout.glyphDesignWidth(info.codepoint, parameters);
          if (!natural || !current || (!absolute && *current <= *natural)) continue;
          growth = *current - *natural;
        }
      }
      for (unsigned offset = 1; offset <= unsigned(attachment.within) && info.cluster + offset < text.size(); ++offset) {
        const auto cluster = info.cluster + offset;
        hb_glyph_info_t* mark = nullptr;
        bool skip = false;
        for (unsigned index = 0; index < buffer->len; ++index) {
          auto& candidate = buffer->info[index];
          if (candidate.cluster != cluster) continue;
          const auto name = layout.recognitionGlyphName(font, candidate.codepoint);
          const auto character = int(text[cluster]);
          const bool found = attachment.matchesFindGlyphs() ? std::binary_search(attachment.findGlyphNames.begin(), attachment.findGlyphNames.end(), name) : std::find(attachment.find.begin(), attachment.find.end(), character) != attachment.find.end();
          if (found && hb_ot_layout_get_glyph_class(hb_font_get_face(font), candidate.codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK) mark = &candidate;
          const bool matchesSkip = attachment.matchesSkipGlyphs() ? std::binary_search(attachment.skipGlyphNames.begin(), attachment.skipGlyphNames.end(), name) : std::find(attachment.skip.begin(), attachment.skip.end(), character) != attachment.skip.end();
          skip |= matchesSkip;
        }
        if (!mark) { if (skip) continue; break; }
        const auto axis = direct ? response->parameterAxis : attachment.widthAxis;
        std::optional<double> value;
        if (direct) {
          // Absolute assignment: preserve the other axes and existing GPOS
          // ownership. No base-width measurement, inversion or quantization.
          value = response->parameterValue;
        } else {
          auto markParameters = layout.glyphParameters(*mark, 0, 0);
          markParameters.set(axis, 0);
          const auto markNatural = layout.glyphDesignWidth(mark->codepoint, markParameters);
          const auto maximum = layout.glyphParameterMaximum(mark->codepoint, axis);
          if (!markNatural || !maximum) break;
          const double ratio = response ? response->ratio : attachment.gain;
          double target = *markNatural + ratio * *growth;
          if (absolute) {
            const auto baseInkWidth = layout.glyphInkWidth(info.codepoint, parameters);
            if (!baseInkWidth) break;
            target = ratio * *baseInkWidth;
          }
          target = std::min(attachment.maximumWidth, target);
          value = dependentMarkWidthParameter(target, *maximum, attachment.widthGrid, [&](double parameter) {
            auto measured = markParameters;
            measured.set(axis, parameter);
            return layout.glyphDesignWidth(mark->codepoint, measured);
          });
        }
        if (value) {
          const auto previousValue = layout.glyphParameters(*mark, 0, 0).value(axis);
          if (*value != previousValue) {
            if (!layout.setAttachmentParameter(*mark, axis, *value, baselineParameters[mark - buffer->info])) throw std::runtime_error("unsupported dependent mark parameter");
            substituteAttachment(*mark, axis, layout, substitutions);
          }
        }
        break;
      }
    }
  }
}

hb_buffer_t* shape(TextString text, hb_font_t* font, vector<hb_feature_t> features, const FeatureJustificationLayout* layout, const vector<GlyphParameterAssignment>& parameters, bool dependentMarks, LineJustificationOperation operation) {
  ShapingBuffer owner(hb_buffer_create());
  auto* buffer = owner.get();

  // The fork's direct post-GSUB hook runs before advances and GPOS. Keep
  // native actions external without enabling diagnostic messages per lookup.
  struct ParameterContext {
    const FeatureJustificationLayout* layout;
    const vector<GlyphParameterAssignment>& parameters;
    std::u16string_view text;
    bool dependentMarks;
    LineJustificationOperation operation;
    bool applied = false;
    std::exception_ptr error;
  } context{layout, parameters, text, dependentMarks && layout && layout->dependentMarkStretchingEnabled(), operation};
  if (!parameters.empty() || context.dependentMarks) {
    const bool hasParameters = std::any_of(parameters.begin(), parameters.end(), [](const auto& parameter) { return parameter.axis != NoGlyphAxis; });
    if (hasParameters && (!layout || !layout->supportsGlyphParameters())) throw std::runtime_error("native justification parameters require a live layout provider");
    hb_buffer_set_post_gsub_func(buffer, [](hb_buffer_t* buffer, hb_font_t* font, void* data) {
      auto& context = *static_cast<ParameterContext*>(data);
      if (context.applied) return;
      context.applied = true;
      try {
        vector<hb_codepoint_t> originals;
        vector<GlyphParameters> baselineParameters;
        if (context.dependentMarks) {
          originals.reserve(buffer->len);
          baselineParameters.reserve(buffer->len);
          // Capture contextual GSUB deformation before applying native
          // justification. The reference keeps joined small Alef/Hamza and
          // before-Heh/Waw adjustments without charging them as kashida.
          for (unsigned index = 0; index < buffer->len; ++index) {
            originals.push_back(buffer->info[index].codepoint);
            baselineParameters.push_back(context.layout->glyphParameters(buffer->info[index], 0, 0));
          }
        }
        for (const auto& parameter : context.parameters) {
          hb_glyph_info_t* target = nullptr;
          for (unsigned i = 0; i < buffer->len; ++i) {
            auto& info = buffer->info[i];
            if (info.cluster != parameter.cluster || hb_ot_layout_get_glyph_class(hb_font_get_face(font), info.codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK) continue;
            target = &info;
            break;
          }
          // Prefer the base when a cluster contains both a base and marks.
          // An attachment target such as fatha normally owns a mark-only
          // character cluster, so fall back to that mark instead of silently
          // dropping its native parameter assignment.
          if (target == nullptr) {
            for (unsigned i = 0; i < buffer->len; ++i) {
              if (buffer->info[i].cluster == parameter.cluster) {
                target = &buffer->info[i];
                break;
              }
            }
          }
          if (target == nullptr) continue;
          if (parameter.substitute != static_cast<hb_codepoint_t>(-1)) target->codepoint = parameter.substitute;
          if (parameter.axis != NoGlyphAxis) {
            const auto previousParameters = context.layout->glyphParameters(*target, 0, 0);
            const auto previousValue = previousParameters.value(parameter.axis);
            const auto value = parameter.value + (parameter.additive ? previousValue : 0);
            const auto substitutions = context.layout->attachmentSubstitutions();
            const bool attachmentMark = hb_ot_layout_get_glyph_class(hb_font_get_face(font), target->codepoint) == HB_OT_LAYOUT_GLYPH_CLASS_MARK && std::any_of(substitutions.begin(), substitutions.end(), [&](const auto& substitution) { return substitution.axis == parameter.axis && substitution.matches(target->codepoint); });
            const auto& markBaseline = baselineParameters.empty() ? previousParameters : baselineParameters[target - buffer->info];
            const bool supported = attachmentMark && value != previousValue ? context.layout->setAttachmentParameter(*target, parameter.axis, value, markBaseline) : context.layout->setGlyphParameter(*target, parameter.axis, value);
            if (!supported) throw std::runtime_error("unsupported native glyph parameter slot");
            if (attachmentMark && (value != previousValue || parameter.substitute != static_cast<hb_codepoint_t>(-1))) substituteAttachment(*target, parameter.axis, *context.layout, substitutions);
          }
        }
        if (context.dependentMarks) applyAttachments(context.text, font, buffer, originals, baselineParameters, *context.layout, context.operation, true);
      } catch (...) { context.error = std::current_exception(); }
    }, &context, nullptr);
  }

  hb_buffer_set_segment_properties(buffer, &savedprops);
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  buffer->justContext = nullptr;
  buffer->useCallback = true;
  buffer->justifyLine = false;

  hb_buffer_add_utf16(buffer, reinterpret_cast<const uint16_t*>(text.data()), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));

  hb_shape(font, buffer, features.data(), features.size());
  hb_buffer_set_post_gsub_func(buffer, nullptr, nullptr, nullptr);
  if (context.error) std::rethrow_exception(context.error);
  if ((!parameters.empty() || context.dependentMarks) && !context.applied) throw std::runtime_error("shaper did not provide the required post-GSUB parameter stage");
  return owner.release();
}
double getBufferWidth(hb_buffer_t* buffer) {
  unsigned int glyphCount = 0;
  const auto* glyphPositions =
      hb_buffer_get_glyph_positions(buffer, &glyphCount);
  double totalWidth = 0.0;
  for (unsigned int index = 0; index < glyphCount; ++index)
    totalWidth += glyphPositions[index].x_advance;
  return totalWidth;
}
// Measurement only ever sums x_advance, and mark/mkmk are pure attachment:
// they move a mark's offsets, and a mark's advance is zero.  Turning them off
// skips the GPOS lookups that dominate this font (75 MarkToBase and 10
// MarkToMark) without changing the number being measured.  The final layout in
// shapeLine keeps them, because there the offsets are the point.
hb_buffer_t* shapeForMeasurement(TextString text, hb_font_t* font, vector<hb_feature_t> features, const FeatureJustificationLayout* layout, const vector<GlyphParameterAssignment>& parameters) {
  features.push_back({HB_TAG('m', 'a', 'r', 'k'), 0u, 0u, (unsigned int)-1});
  features.push_back({HB_TAG('m', 'k', 'm', 'k'), 0u, 0u, (unsigned int)-1});
  // Dependent mark deformation affects ink and GPOS offsets, not advances.
  // Defer it to the single final positioning pass, including full-shape probes
  // whose mark/mkmk features are disabled here.
  return shape(std::move(text), font, std::move(features), layout, parameters, false);
}

double getWidth(const TextString& text, hb_font_t* font, const vector<hb_feature_t>& features) {
  auto buffer = shapeForMeasurement(text, font, features);
  const auto totalWidth = getBufferWidth(buffer);
  hb_buffer_destroy(buffer);
  return totalWidth;
}
double getWordWidth(const WordInfo& wordInfo, const map<int, vector<TextFontFeatures>>& justResults,
                    hb_font_t* font, ShapingBuffer* shapedWord, const FeatureJustificationLayout* layout, std::span<const TextFontFeatures> globalFeatures, const map<int, hb_codepoint_t>& substitutions, const map<int, vector<TextFontFeatures>>& baseline) {
  vector<hb_feature_t> features{};
  vector<GlyphParameterAssignment> parameters;

  for (const auto& feature : globalFeatures) {
    features.push_back({hb_tag_from_string(feature.name.c_str(), static_cast<int>(feature.name.size())), static_cast<uint32_t>(feature.value), 0u, static_cast<unsigned int>(-1)});
  }

  for (const auto& [cluster, substitute] : substitutions) {
    if (cluster < wordInfo.startIndex || cluster > wordInfo.endIndex) continue;
    parameters.push_back({static_cast<unsigned>(cluster - wordInfo.startIndex), NoGlyphAxis, 0, substitute});
  }

  for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {
    if (const auto seed = baseline.find(i); seed != baseline.end()) {
      for (const auto& value : seed->second) parameters.push_back({static_cast<unsigned>(i - wordInfo.startIndex), value.axis, value.value, static_cast<hb_codepoint_t>(-1), value.additive});
    }
    auto justInfo = justResults.find(i);
    if (justInfo != justResults.end()) {
      for (auto& feat : justInfo->second) {
        if (feat.axis != NoGlyphAxis) {
          parameters.push_back({static_cast<unsigned>(i - wordInfo.startIndex), feat.axis, static_cast<double>(feat.value), static_cast<hb_codepoint_t>(-1), feat.additive});
          continue;
        }
        features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                            (uint32_t)feat.value,
                            (unsigned int)(i - wordInfo.startIndex),
                            (unsigned int)(i - wordInfo.startIndex + 1)});
      }
    }
  }

  if (shapedWord == nullptr && parameters.empty()) return getWidth(wordInfo.text, font, features);

  ShapingBuffer buffer(shapeForMeasurement(wordInfo.text, font, features, layout, parameters));
  const auto totalWidth = getBufferWidth(buffer.get());
  if (shapedWord) *shapedWord = std::move(buffer);
  return totalWidth;
}

// Reshapes the word with the staged features and measures it, keeping the
// result only when it fits the requested direction without crossing the target.
AppliedResult tryApplyFeatures(int wordIndex, const LineTextInfo& lineTextInfo, JustInfo& justInfo, const map<int, vector<TextFontFeatures>>& newFeatures,
                               bool retainGlyphs, const map<int, hb_codepoint_t>* newSubstitutions, bool shrinking) {
  auto& layout = justInfo.layoutResult[wordIndex];

  const auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

  ShapingBuffer candidate;
  const auto& substitutions = newSubstitutions == nullptr ? justInfo.substitutions : *newSubstitutions;
  const auto wordNewWidth = getWordWidth(wordInfo, newFeatures, justInfo.font, retainGlyphs ? &candidate : nullptr, justInfo.layout, justInfo.globalFeatures, substitutions, justInfo.baselineFeatures);
  auto diff = wordNewWidth - layout.parWidth;
  const bool fits = shrinking
      ? diff < 0 && justInfo.textLineWidth + diff >= justInfo.desiredWidth
      : wordNewWidth != layout.parWidth && justInfo.textLineWidth + diff < justInfo.desiredWidth;
  if (std::isfinite(wordNewWidth) && fits) {
    justInfo.textLineWidth += diff;
    layout.parWidth = wordNewWidth;
    justInfo.fontFeatures = newFeatures;
    if (newSubstitutions != nullptr) justInfo.substitutions = *newSubstitutions;
    if (retainGlyphs) {
      justInfo.acceptedWordBuffers.resize(lineTextInfo.wordInfos.size());
      justInfo.acceptedWordBuffers[wordIndex] = std::move(candidate);
    }
    return AppliedResult::Positive;
  } else if (diff == 0) {
    return AppliedResult::NoChange;
  } else {
    return AppliedResult::Overflow;
  }
}

map<int, vector<TextFontFeatures>> resolvedJustificationFeatures(const JustInfo& info) {
  auto result = info.baselineFeatures;
  for (const auto& [site, features] : info.fontFeatures) {
    auto& values = result[site];
    for (const auto& feature : features) {
      std::erase_if(values, [&](const auto& value) { return value.name == feature.name; });
      values.push_back(feature);
    }
  }
  return result;
}

LineLayoutInfo shapeLine(FeatureJustificationLayout& layout, int lineWidth, int pageWidth,
                         const LineTextInfo& lineTextInfo, const JustResultByLine& justResult, bool tajweedColor, double fontScale, hb_font_t* font,
                         LineJustification justification, int& currentyPos, digitalkhatt::TajweedMap& tajweedResult) {
  vector<hb_feature_t> features{};
  vector<GlyphParameterAssignment> parameters;

  for (auto& feat : justResult.globalFeatures) {
    features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                        (uint32_t)feat.value,
                        (unsigned int)0,
                        (unsigned int)-1});
  }

  for (const auto& [cluster, substitute] : justResult.substitutions) parameters.push_back({static_cast<unsigned>(cluster), NoGlyphAxis, 0, substitute});

  for (auto& wordInfo : lineTextInfo.wordInfos) {
    for (int i = wordInfo.startIndex; i <= wordInfo.endIndex; i++) {
      auto justInfo = justResult.fontFeatures.find(i);
      if (justInfo != justResult.fontFeatures.end()) {
        for (auto& feat : justInfo->second) {
          if (feat.axis != NoGlyphAxis) {
            parameters.push_back({static_cast<unsigned>(i), feat.axis, static_cast<double>(feat.value), static_cast<hb_codepoint_t>(-1), feat.additive});
            continue;
          }
          features.push_back({hb_tag_from_string(feat.name.c_str(), static_cast<int>(feat.name.size())),
                              (uint32_t)feat.value,
                              (unsigned int)(i),
                              (unsigned int)(i + 1)});
        }
      }
    }
  }

  auto buffer = shape(lineTextInfo.lineText, font, features, &layout, parameters, true, justResult.operation);

  unsigned int glyph_count;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  LineLayoutInfo lineLayout;
  int currentlineWidth = 0;
  int lastAdjustedSpaceCluster = -1;
  double spaceAdjustmentRemainder = 0.0;

  for (int i = glyph_count - 1; i >= 0; i--) {
    GlyphLayoutInfo glyphLayout;

    const auto tatweels = font->glyph_tatweels(glyph_info[i]);
    const auto provenance = font->glyph_positioning(glyph_info[i]);
    glyphLayout.codepoint = glyph_info[i].codepoint;
    glyphLayout.parameters = layout.glyphParameters(glyph_info[i], tatweels.left, tatweels.right);
    glyphLayout.justificationAddedLeftTatweel = std::max(0.0, layout.glyphJustificationDelta(glyph_info[i], LeftTatweelAxis));
    glyphLayout.cluster = glyph_info[i].cluster;
    glyphLayout.x_advance = glyph_pos[i].x_advance;
    glyphLayout.y_advance = glyph_pos[i].y_advance;
    glyphLayout.x_offset = glyph_pos[i].x_offset;
    glyphLayout.y_offset = glyph_pos[i].y_offset;
    glyphLayout.lookup_index = provenance.lookup_index;
    if (tajweedColor) {
      auto color = tajweedResult.find(glyphLayout.cluster);
      if (color != tajweedResult.end()) {
        glyphLayout.color = tajweedNameToColor[color->second];
      } else if (lineTextInfo.lineText[glyphLayout.cluster] == u'\u034F') {
        auto nextIndex = i - 1;
        auto nextCluster = glyph_info[nextIndex].cluster;
        auto currCluster = glyphLayout.cluster + 1;
        if (nextCluster > static_cast<uint32_t>(currCluster)) {
          auto color = tajweedResult.find(currCluster);
          if (color != tajweedResult.end()) {
            // happens only in لِيَسُ͏ࣳٓـٔ͏ُوا۟ page 282 line 14
            // console.log(`cgi***************************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
            glyphLayout.color = tajweedNameToColor[color->second];
          }
          /* debug
          if (nextCluster - currCluster > 1 && lineText[glyph.Cluster + 2] !== "\u034Fu") {
            console.log(`nextCluster - currCluster > 1**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
          }*/
        } /*else {
          // happens only for فَٱدَّٰرَٰ͏ْٔتُمْ page 11 line 5 no tajweed coloring so it is OK
          //console.log(`nextCluster<=currCluster**********************************************************Page ${this.pageIndex + 1} Line ${lineIndex + 1}`)
        }  */
      }
    } else {
      glyphLayout.color = 0;
    }
    glyphLayout.subtable_index = provenance.subtable_index;
    glyphLayout.base_codepoint = provenance.base_codepoint;

    glyphLayout.beginsajda = false;
    glyphLayout.endsajda = false;

    if (!justResult.isShrink) {
      auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

      if (space != lineTextInfo.spaces.end()) {
        const double spacing = space->second == SpaceType::Aya
            ? justResult.ayaSpacing : justResult.simpleSpacing;
        const double scaled = spacing + spaceAdjustmentRemainder;
        glyphLayout.x_advance = static_cast<int>(std::floor(scaled));
        spaceAdjustmentRemainder = scaled - glyphLayout.x_advance;
      }
    } else if (justResult.addedSpaceAfterShrink != 0) {
      auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

      if (space != lineTextInfo.spaces.end() && glyphLayout.cluster != lastAdjustedSpaceCluster) {
        // Carry fractional font units without filling beyond the target.
        const double scaled = justResult.addedSpaceAfterShrink + spaceAdjustmentRemainder;
        const int added = static_cast<int>(std::floor(scaled));
        spaceAdjustmentRemainder = scaled - added;
        glyphLayout.x_advance += added;
        lastAdjustedSpaceCluster = glyphLayout.cluster;
      }
    }

    currentlineWidth += glyphLayout.x_advance;

    lineLayout.glyphs.push_back(glyphLayout);
  }

  lineLayout.desiredLineWidth = lineWidth;
  lineLayout.currentLineWidth = currentlineWidth;

  if (justification == LineJustification::Distribute) {
    lineLayout.xstartposition = 0;
  } else {
    lineLayout.xstartposition = static_cast<int>((pageWidth - currentlineWidth * fontScale) / 2);
  }

  lineLayout.ystartposition = currentyPos;
  lineLayout.fontSize = fontScale;
  lineLayout.overfull = lineLayout.renderedOverflow();

  currentyPos = currentyPos + (layout.interLineSpacing() << layout.scaleBy());

  hb_buffer_destroy(buffer);

  return lineLayout;
}
}  // namespace digitalkhatt::justify::runtime
