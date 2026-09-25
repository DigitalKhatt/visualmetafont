#include "JustificationShaping.h"

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

hb_buffer_t* shape(TextString text, hb_font_t* font, vector<hb_feature_t> features, const FeatureJustificationLayout* layout, const vector<GlyphParameterAssignment>& parameters) {
  ShapingBuffer owner(hb_buffer_create());
  auto* buffer = owner.get();

  // This fork emits end-table-GSUB before initializing advances and running
  // GPOS. Keep native actions external: no second positioning pass, no use of
  // diagnostic callbacks when a line/word has no instance assignments.
  struct ParameterContext {
    const FeatureJustificationLayout* layout;
    const vector<GlyphParameterAssignment>& parameters;
    bool applied = false;
    std::exception_ptr error;
  } context{layout, parameters};
  if (!parameters.empty()) {
    const bool hasParameters = std::any_of(parameters.begin(), parameters.end(), [](const auto& parameter) { return parameter.axis != NoGlyphAxis; });
    if (hasParameters && (!layout || !layout->supportsGlyphParameters())) throw std::runtime_error("native justification parameters require a live layout provider");
    hb_buffer_set_message_func(buffer, [](hb_buffer_t* buffer, hb_font_t* font, const char* message, void* data) -> hb_bool_t {
      auto& context = *static_cast<ParameterContext*>(data);
      if (strcmp(message, "end table GSUB") != 0 || context.applied) return true;
      context.applied = true;
      try {
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
          if (parameter.axis != NoGlyphAxis && !context.layout->setGlyphParameter(*target, parameter.axis, parameter.value)) throw std::runtime_error("unsupported native glyph parameter slot");
        }
      } catch (...) { context.error = std::current_exception(); }
      return true; }, &context, nullptr);
  }

  hb_buffer_set_segment_properties(buffer, &savedprops);
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  buffer->justContext = nullptr;
  buffer->useCallback = true;
  buffer->justifyLine = false;

  hb_buffer_add_utf16(buffer, reinterpret_cast<const uint16_t*>(text.data()), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));

  hb_shape(font, buffer, features.data(), features.size());
  hb_buffer_set_message_func(buffer, nullptr, nullptr, nullptr);
  if (context.error) std::rethrow_exception(context.error);
  if (!parameters.empty() && !context.applied) throw std::runtime_error("shaper did not provide the required post-GSUB parameter stage");
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
  return shape(std::move(text), font, std::move(features), layout, parameters);
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
      for (const auto& value : seed->second) parameters.push_back({static_cast<unsigned>(i - wordInfo.startIndex), value.axis, value.value, static_cast<hb_codepoint_t>(-1)});
    }
    auto justInfo = justResults.find(i);
    if (justInfo != justResults.end()) {
      for (auto& feat : justInfo->second) {
        if (feat.axis != NoGlyphAxis) {
          parameters.push_back({static_cast<unsigned>(i - wordInfo.startIndex), feat.axis, static_cast<double>(feat.value), static_cast<hb_codepoint_t>(-1)});
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
// result only when it widens the line without crossing the target width.
AppliedResult tryApplyFeatures(int wordIndex, const LineTextInfo& lineTextInfo, JustInfo& justInfo, const map<int, vector<TextFontFeatures>>& newFeatures,
                               bool retainGlyphs, const map<int, hb_codepoint_t>* newSubstitutions) {
  auto& layout = justInfo.layoutResult[wordIndex];

  const auto& wordInfo = lineTextInfo.wordInfos[wordIndex];

  ShapingBuffer candidate;
  const auto& substitutions = newSubstitutions == nullptr ? justInfo.substitutions : *newSubstitutions;
  const auto wordNewWidth = getWordWidth(wordInfo, newFeatures, justInfo.font, retainGlyphs ? &candidate : nullptr, justInfo.layout, justInfo.globalFeatures, substitutions, justInfo.baselineFeatures);
  auto diff = wordNewWidth - layout.parWidth;
  if (wordNewWidth != layout.parWidth && justInfo.textLineWidth + diff < justInfo.desiredWidth) {
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
                         const LineTextInfo& lineTextInfo, const JustResultByLine& justResult, bool tajweedColor, double emScale, hb_font_t* font,
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
            parameters.push_back({static_cast<unsigned>(i), feat.axis, static_cast<double>(feat.value), static_cast<hb_codepoint_t>(-1)});
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

  auto buffer = shape(lineTextInfo.lineText, font, features, &layout, parameters);

  unsigned int glyph_count;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);

  LineLayoutInfo lineLayout;
  int currentlineWidth = 0;

  for (int i = glyph_count - 1; i >= 0; i--) {
    GlyphLayoutInfo glyphLayout;

    const auto tatweels = font->glyph_tatweels(glyph_info[i]);
    const auto provenance = font->glyph_positioning(glyph_info[i]);
    glyphLayout.codepoint = glyph_info[i].codepoint;
    glyphLayout.parameters = layout.glyphParameters(glyph_info[i], tatweels.left, tatweels.right);
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
        if (space->second == SpaceType::Aya) {
          glyphLayout.x_advance = justResult.ayaSpacing * emScale;
        } else if (space->second == SpaceType::Simple) {
          glyphLayout.x_advance = justResult.simpleSpacing * emScale;
        }
      }
    } else if (justResult.addedSpaceAfterShrink != 0) {
      auto space = lineTextInfo.spaces.find(glyphLayout.cluster);

      if (space != lineTextInfo.spaces.end()) {
        glyphLayout.x_advance += justResult.addedSpaceAfterShrink;
      }
    }

    currentlineWidth += glyphLayout.x_advance;

    lineLayout.glyphs.push_back(glyphLayout);
  }

  lineLayout.desiredLineWidth = lineWidth;
  lineLayout.currentLineWidth = currentlineWidth;

  lineLayout.overfull = lineWidth != 0 ? currentlineWidth - lineWidth : 0;

  if (justification == LineJustification::Distribute) {
    lineLayout.xstartposition = 0;
  } else {
    lineLayout.xstartposition = (pageWidth - currentlineWidth) / 2;
  }

  lineLayout.ystartposition = currentyPos;
  lineLayout.fontSize = emScale;

  currentyPos = currentyPos + (layout.interLineSpacing() << layout.scaleBy());

  hb_buffer_destroy(buffer);

  return lineLayout;
}
}  // namespace digitalkhatt::justify::runtime
