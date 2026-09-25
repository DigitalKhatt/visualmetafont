#include "DeclPolicyAdapter.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <hb-ot.h>

#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationActionEvaluator.h>
#include <digitalkhatt/justify/declpolicy/JustificationCandidate.h>
#include <digitalkhatt/justify/declpolicy/JustificationStagingBackend.h>

#include "../JustificationShaping.h"

namespace digitalkhatt::justify::decl {

double quantizeProportionalParameter(double value, double from, double to, unsigned subdivisions) {
  if (subdivisions == 0) return value;
  const auto scaled = value * subdivisions;
  const auto snapped = to >= from ? std::floor(scaled + 1e-12) / subdivisions : std::ceil(scaled - 1e-12) / subdivisions;
  return std::clamp(snapped, std::min(from, to), std::max(from, to));
}

namespace {

using runtime::AppliedResult;
using runtime::getBufferWidth;
using runtime::getWordWidth;
using runtime::JustInfo;
using runtime::LineTextInfo;
using runtime::ShapingBuffer;
using runtime::TextFontFeatures;
using std::map;
using std::vector;

struct BaseLocation {
  int wordIndex = -1;
  int subwordIndex = -1;
  int baseIndex = -1;
  // Taken from the real base indexes, never inferred from the glyph name:
  // a glyph whose name lacks .init/.medi/.fina/.isol carries no form fact.
  bool firstInSubword = false;
  bool lastInSubword = false;
};

vector<BaseLocation> prepareBaseLocations(const LineTextInfo& lineTextInfo) {
  vector<BaseLocation> locations(lineTextInfo.lineText.size());
  for (int wordIndex = 0;
       wordIndex < static_cast<int>(lineTextInfo.wordInfos.size());
       ++wordIndex) {
    const auto& word = lineTextInfo.wordInfos[wordIndex];
    for (int subwordIndex = 0;
         subwordIndex < static_cast<int>(word.subwords.size());
         ++subwordIndex) {
      const auto& baseIndexes = word.subwords[subwordIndex].baseIndexes;
      for (int baseIndex = 0;
           baseIndex < static_cast<int>(baseIndexes.size()); ++baseIndex) {
        const auto characterIndex = word.startIndex + baseIndexes[baseIndex];
        if (characterIndex < 0 ||
            characterIndex >= static_cast<int>(locations.size())) {
          throw std::runtime_error(
              "declarative-policy base index is outside the line text");
        }
        locations[characterIndex] = {
            .wordIndex = wordIndex,
            .subwordIndex = subwordIndex,
            .baseIndex = baseIndex,
            .firstInSubword = baseIndex == 0,
            .lastInSubword =
                baseIndex == static_cast<int>(baseIndexes.size()) - 1,
        };
      }
    }
  }

  return locations;
}

vector<FixedSlotGlyphInput> prepareFixedSlotGlyphs(const LineTextInfo& lineTextInfo, hb_font_t* font, hb_buffer_t* buffer, const FeatureJustificationLayout& layout, const CompiledJustificationCatalog& catalog, std::span<const BaseLocation> locations, int clusterOffset = 0, const map<int, hb_codepoint_t>* substitutions = nullptr) {
  unsigned int glyphCount = 0;
  const auto* glyphInfos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
  auto* face = hb_font_get_face(font);
  vector<FixedSlotGlyphInput> glyphs;
  glyphs.reserve(glyphCount);
  for (unsigned int glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
    const auto& glyph = glyphInfos[glyphIndex];
    const auto characterIndex = static_cast<int>(glyph.cluster) + clusterOffset;
    auto codepoint = glyph.codepoint;
    if (substitutions) {
      const auto substitute = substitutions->find(characterIndex);
      if (substitute != substitutions->end()) codepoint = substitute->second;
    }
    const auto glyphClass = hb_ot_layout_get_glyph_class(face, codepoint);
    if (glyphClass != HB_OT_LAYOUT_GLYPH_CLASS_BASE_GLYPH &&
        glyphClass != HB_OT_LAYOUT_GLYPH_CLASS_LIGATURE) {
      continue;
    }

    if (characterIndex < 0 ||
        characterIndex >= static_cast<int>(locations.size()))
      continue;
    const auto location = locations[characterIndex];
    if (location.wordIndex == -1) continue;

    const auto glyphName = layout.recognitionGlyphName(font, codepoint);
    if (glyphName.empty()) continue;
    glyphs.push_back({
        .facts = glyphFacts({.glyphName = glyphName,
                             .sourceCharacter =
                                 lineTextInfo.lineText[characterIndex],
                             .firstInSubword = location.firstInSubword,
                             .lastInSubword = location.lastInSubword},
                            catalog.facts),
        .wordIndex = location.wordIndex,
        .subwordIndex = location.subwordIndex,
        .baseIndex = location.baseIndex,
        .indexInLine = characterIndex,
    });
  }
  return glyphs;
}

std::optional<hb_glyph_info_t> glyphInfoAtCluster(hb_font_t* font, hb_buffer_t* buffer, int cluster) {
  if (buffer == nullptr) return std::nullopt;
  unsigned glyphCount = 0;
  const auto* glyphs = hb_buffer_get_glyph_infos(buffer, &glyphCount);
  auto* face = hb_font_get_face(font);
  std::optional<hb_glyph_info_t> fallback;
  for (unsigned index = 0; index < glyphCount; ++index) {
    if (glyphs[index].cluster != static_cast<unsigned>(cluster)) continue;
    if (!fallback) fallback = glyphs[index];
    if (hb_ot_layout_get_glyph_class(face, glyphs[index].codepoint) != HB_OT_LAYOUT_GLYPH_CLASS_MARK) return glyphs[index];
  }
  return fallback;
}

hb_codepoint_t currentGlyphAtSite(const LineTextInfo& lineTextInfo, const JustInfo& justInfo, const map<int, hb_codepoint_t>& substitutions, JustificationSiteRef site) {
  const auto staged = substitutions.find(site);
  if (staged != substitutions.end()) return staged->second;
  for (std::size_t wordIndex = 0; wordIndex < lineTextInfo.wordInfos.size(); ++wordIndex) {
    const auto& word = lineTextInfo.wordInfos[wordIndex];
    if (site < word.startIndex || site > word.endIndex) continue;
    if (wordIndex < justInfo.acceptedWordBuffers.size() && justInfo.acceptedWordBuffers[wordIndex]) {
      if (const auto glyph = glyphInfoAtCluster(justInfo.font, justInfo.acceptedWordBuffers[wordIndex].get(), site - word.startIndex)) return glyph->codepoint;
    }
    break;
  }
  if (const auto glyph = glyphInfoAtCluster(justInfo.font, justInfo.recognitionBuffer, site)) return glyph->codepoint;
  throw std::runtime_error("action lookup target has no shaped glyph");
}

struct MeasuredGlyphState {
  hb_codepoint_t codepoint;
  GlyphParameters parameters;
};

std::optional<MeasuredGlyphState> glyphStateAtSite(const JustInfo& justInfo, const map<int, vector<TextFontFeatures>>& features, const map<int, hb_codepoint_t>& substitutions, JustificationSiteRef site) {
  auto info = glyphInfoAtCluster(justInfo.font, justInfo.recognitionBuffer, site);
  if (!info) return std::nullopt;
  if (const auto substitute = substitutions.find(site); substitute != substitutions.end()) info->codepoint = substitute->second;
  auto parameters = justInfo.layout->glyphParameters(*info, 0, 0);
  if (const auto seed = justInfo.baselineFeatures.find(site); seed != justInfo.baselineFeatures.end()) {
    for (const auto& value : seed->second) parameters.set(value.axis, value.value);
  }
  if (const auto values = features.find(site); values != features.end()) {
    for (const auto& value : values->second)
      if (value.axis != NoGlyphAxis) parameters.set(value.axis, value.value);
  }
  return MeasuredGlyphState{info->codepoint, std::move(parameters)};
}

std::optional<double> measureGlyphDelta(const JustInfo& justInfo, const map<int, vector<TextFontFeatures>>& beforeFeatures, const map<int, hb_codepoint_t>& beforeSubstitutions, const map<int, vector<TextFontFeatures>>& afterFeatures, const map<int, hb_codepoint_t>& afterSubstitutions, const std::set<JustificationSiteRef>& sites) {
  double delta = 0;
  for (const auto site : sites) {
    const auto before = glyphStateAtSite(justInfo, beforeFeatures, beforeSubstitutions, site);
    const auto after = glyphStateAtSite(justInfo, afterFeatures, afterSubstitutions, site);
    if (!before || !after) return std::nullopt;
    const auto beforeAdvance = justInfo.layout->glyphAdvance(justInfo.font, before->codepoint, before->parameters);
    const auto afterAdvance = justInfo.layout->glyphAdvance(justInfo.font, after->codepoint, after->parameters);
    if (!beforeAdvance || !afterAdvance) return std::nullopt;
    delta += *afterAdvance - *beforeAdvance;
  }
  return delta;
}

// Stages lookup applications and optional feature attributes. Accepted trials
// retain the shaped word for current-state recognition; rejected trials do not.
class FeatureStagingBackend final : public JustificationStagingBackend {
 public:
  FeatureStagingBackend(const LineTextInfo& lineTextInfo, JustInfo& justInfo,
                        const CompiledJustificationCatalog& catalog,
                        CandidateWidthMode candidateWidth = CandidateWidthMode::FullShape)
      : lineTextInfo_(lineTextInfo), justInfo_(justInfo), catalog_(catalog), candidateWidth_(candidateWidth) {}

  void beginTransaction() override {
    staged_ = justInfo_.fontFeatures;
    stagedSubstitutions_ = justInfo_.substitutions;
    touchedSites_.clear();
    canMeasureAdvance_ = true;
  }
  void abortTransaction() override {
    staged_.clear();
    stagedSubstitutions_.clear();
  }

  FixedSlotActionResult commitTransaction(int wordIndex) override {
    switch (tryApplyFeatures(wordIndex, lineTextInfo_, justInfo_, staged_, true, &stagedSubstitutions_)) {
      case AppliedResult::NoChange:
        return FixedSlotActionResult::NoChange;
      case AppliedResult::Positive:
        return FixedSlotActionResult::Positive;
      case AppliedResult::Overflow:
        return FixedSlotActionResult::Overflow;
      case AppliedResult::Forbiden:
        return FixedSlotActionResult::Forbidden;
    }
    return FixedSlotActionResult::Forbidden;
  }

  bool present(JustificationSiteRef site,
               JustAttributeId attribute) const override {
    return find(site, attribute) != nullptr;
  }

  JustificationLineMetrics lineMetrics() const override {
    return {justInfo_.desiredWidth, justInfo_.textLineWidth, justInfo_.initialLineWidth};
  }

  double read(JustificationSiteRef site,
              JustAttributeId attribute) const override {
    const auto* feature = find(site, attribute);
    return feature == nullptr ? 0 : feature->value;
  }

  void update(JustificationSiteRef site, JustAttributeId attribute,
              double value) override {
    auto& features = staged_[site];
    const auto& tag = tagOf(attribute);
    const auto axis = catalog_.attributeAxes.at(attribute);
    // A policy may pair a native axis with an OpenType compatibility feature
    // (for example third + cv04). Static providers ignore the native write and
    // retain the feature write; a native-only action still measures no change.
    if (axis != NoGlyphAxis && !justInfo_.layout->supportsGlyphParameters())
      return;
    touchedSites_.insert(site);
    if (axis == NoGlyphAxis) canMeasureAdvance_ = false;
    if (axis == NoGlyphAxis && value != std::trunc(value))
      throw std::runtime_error("OpenType justification feature values must be integers");
    // In place when the attribute is already there, so the order HarfBuzz sees
    // the features in does not depend on the order they were written.
    for (auto& feature : features) {
      if (feature.name == tag) {
        feature.value = value;
        return;
      }
    }
    features.push_back({.name = tag, .value = static_cast<double>(value), .axis = axis});
  }

  void clear(JustificationSiteRef site) override {
    touchedSites_.insert(site);
    if (const auto found = staged_.find(site); found != staged_.end()) {
      canMeasureAdvance_ = canMeasureAdvance_ && std::none_of(found->second.begin(), found->second.end(), [](const auto& feature) { return feature.axis == NoGlyphAxis; });
    }
    staged_[site].clear();
    stagedSubstitutions_.erase(site);
  }

  void applyLookup(JustificationSiteRef site, std::string_view lookup) override {
    touchedSites_.insert(site);
    const auto glyph = currentGlyphAtSite(lineTextInfo_, justInfo_, stagedSubstitutions_, site);
    const auto substitute = justInfo_.layout->resolveJustificationLookup(glyph, lookup);
    if (substitute && *substitute != glyph) stagedSubstitutions_[site] = *substitute;
  }

  std::optional<std::uint32_t> glyph(
      JustificationSiteRef site) const override {
    return currentGlyphAtSite(lineTextInfo_, justInfo_,
                              stagedSubstitutions_, site);
  }

  std::optional<double> measureStagedWidthDelta(int wordIndex) override {
    if (candidateWidth_ == CandidateWidthMode::Advance && canMeasureAdvance_) {
      const auto delta = measureGlyphDelta(justInfo_, justInfo_.fontFeatures, justInfo_.substitutions, staged_, stagedSubstitutions_, touchedSites_);
      if (delta) return delta;
    }
    const auto& word = lineTextInfo_.wordInfos.at(wordIndex);
    const auto width = getWordWidth(word, staged_, justInfo_.font, nullptr, justInfo_.layout, justInfo_.globalFeatures, stagedSubstitutions_, justInfo_.baselineFeatures);
    return width - justInfo_.layoutResult.at(wordIndex).parWidth;
  }

  JustificationSiteRef attachment(JustificationSiteRef anchor,
                                  JustAttachmentId attachment) const override {
    if (attachment >= catalog_.attachments.size()) return kNoSite;
    const auto& spec = catalog_.attachments[attachment];
    const auto& lineText = lineTextInfo_.lineText;
    const auto findBit = std::uint64_t{1} << (2 * attachment);
    const auto skipBit = std::uint64_t{1} << (2 * attachment + 1);
    const auto& attachmentGlyphs = justInfo_.attachmentGlyphs;
    for (int offset = 1; offset <= spec.within; ++offset) {
      const auto index = anchor + offset;
      if (index < 0 || index >= static_cast<int>(lineText.size()))
        return kNoSite;
      const auto character = static_cast<int>(lineText[index]);
      const auto glyphBits =
          index < static_cast<int>(attachmentGlyphs.size())
              ? attachmentGlyphs[index]
              : std::uint64_t{0};
      const auto matches = [&](bool byGlyph, std::uint64_t bit,
                               const std::vector<int>& characters) {
        return byGlyph ? (glyphBits & bit) != 0
                       : std::find(characters.begin(), characters.end(),
                                   character) != characters.end();
      };
      if (matches(spec.matchesFindGlyphs(), findBit, spec.find)) return index;
      if (matches(spec.matchesSkipGlyphs(), skipBit, spec.skip)) continue;
      return kNoSite;
    }
    return kNoSite;
  }

 private:
  const std::string& tagOf(JustAttributeId attribute) const {
    if (attribute >= catalog_.attributes.size()) {
      throw std::runtime_error("justification attribute is out of range");
    }
    return catalog_.attributes[attribute];
  }

  const TextFontFeatures* find(JustificationSiteRef site,
                               JustAttributeId attribute) const {
    const auto entry = staged_.find(site);
    if (entry == staged_.end()) return nullptr;
    const auto& tag = tagOf(attribute);
    for (const auto& feature : entry->second) {
      if (feature.name == tag) return &feature;
    }
    return nullptr;
  }

  const LineTextInfo& lineTextInfo_;
  JustInfo& justInfo_;
  const CompiledJustificationCatalog& catalog_;
  CandidateWidthMode candidateWidth_;
  map<int, vector<TextFontFeatures>> staged_;
  map<int, hb_codepoint_t> stagedSubstitutions_;
  std::set<JustificationSiteRef> touchedSites_;
  bool canMeasureAdvance_ = true;
};

// Refresh glyph-based attachments alongside base facts. A word refresh clears
// only its own character range, including marks removed/replaced by a lookup.
void updateAttachmentGlyphs(
    const LineTextInfo& lineTextInfo,
    hb_font_t* font,
    hb_buffer_t* buffer,
    const FeatureJustificationLayout& layout,
    const CompiledJustificationCatalog& catalog, vector<std::uint64_t>& attachmentGlyphs, int clusterOffset = 0, int length = -1, const map<int, hb_codepoint_t>* substitutions = nullptr) {
  const auto matchesGlyphs = [](const JustAttachment& attachment) {
    return attachment.matchesFindGlyphs() || attachment.matchesSkipGlyphs();
  };
  if (std::none_of(catalog.attachments.begin(), catalog.attachments.end(),
                   matchesGlyphs)) {
    attachmentGlyphs.clear();
    return;
  }
  if (catalog.attachments.size() > 32) {
    throw std::runtime_error(
        "at most 32 justification attachments can match glyphs");
  }

  attachmentGlyphs.resize(lineTextInfo.lineText.size(), 0);
  if (length < 0) length = static_cast<int>(attachmentGlyphs.size());
  if (clusterOffset < 0 || clusterOffset > static_cast<int>(attachmentGlyphs.size()) || length > static_cast<int>(attachmentGlyphs.size()) - clusterOffset) throw std::runtime_error("attachment recognition range is outside the line");
  std::fill(attachmentGlyphs.begin() + clusterOffset, attachmentGlyphs.begin() + clusterOffset + length, 0);
  unsigned int glyphCount = 0;
  const auto* glyphInfos = hb_buffer_get_glyph_infos(buffer, &glyphCount);
  for (unsigned int glyphIndex = 0; glyphIndex < glyphCount; ++glyphIndex) {
    const auto& glyph = glyphInfos[glyphIndex];
    const auto characterIndex = static_cast<int>(glyph.cluster) + clusterOffset;
    if (characterIndex < clusterOffset || characterIndex >= clusterOffset + length) {
      continue;
    }
    auto codepoint = glyph.codepoint;
    if (substitutions) {
      const auto substitute = substitutions->find(characterIndex);
      if (substitute != substitutions->end()) codepoint = substitute->second;
    }
    const auto glyphName = layout.recognitionGlyphName(font, codepoint);
    if (glyphName.empty()) continue;
    for (std::size_t index = 0; index < catalog.attachments.size(); ++index) {
      const auto& attachment = catalog.attachments[index];
      if (attachment.matchesFindGlyphs() &&
          std::binary_search(attachment.findGlyphNames.begin(),
                             attachment.findGlyphNames.end(), glyphName)) {
        attachmentGlyphs[characterIndex] |= std::uint64_t{1} << (2 * index);
      }
      if (attachment.matchesSkipGlyphs() &&
          std::binary_search(attachment.skipGlyphNames.begin(),
                             attachment.skipGlyphNames.end(), glyphName)) {
        attachmentGlyphs[characterIndex] |= std::uint64_t{1} << (2 * index + 1);
      }
    }
  }
}

}  // namespace

std::vector<JustificationCandidate> collectDeclPolicyStageCandidates(const LineTextInfo& lineTextInfo, JustInfo& justInfo, std::span<const PolicyPhase> phases) {
  if (justInfo.catalog == nullptr) throw std::runtime_error("declarative-policy justification requires table(justdfa)");
  if (justInfo.recognitionBuffer == nullptr) throw std::runtime_error("declarative-policy justification requires a prepared glyph buffer");
  if (justInfo.layout == nullptr) throw std::runtime_error("declarative-policy justification requires a glyph-name provider");

  const auto locations = prepareBaseLocations(lineTextInfo);
  updateAttachmentGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, justInfo.attachmentGlyphs, 0, -1, &justInfo.substitutions);

  std::vector<FixedSlotGlyphInput> glyphs;
  if (justInfo.declPolicyState.currentGlyphs.empty()) {
    glyphs = prepareFixedSlotGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, locations, 0, &justInfo.substitutions);
  }
  for (int wordIndex = 0; wordIndex < static_cast<int>(justInfo.acceptedWordBuffers.size()); ++wordIndex) {
    auto* buffer = justInfo.acceptedWordBuffers[wordIndex].get();
    if (buffer == nullptr) continue;
    const auto& word = lineTextInfo.wordInfos.at(wordIndex);
    updateAttachmentGlyphs(lineTextInfo, justInfo.font, buffer, *justInfo.layout, *justInfo.catalog, justInfo.attachmentGlyphs, word.startIndex, word.endIndex - word.startIndex + 1, &justInfo.substitutions);
    if (justInfo.declPolicyState.currentGlyphs.empty()) {
      auto current = prepareFixedSlotGlyphs(lineTextInfo, justInfo.font, buffer, *justInfo.layout, *justInfo.catalog, locations, word.startIndex, &justInfo.substitutions);
      std::erase_if(glyphs, [&](const auto& glyph) { return glyph.wordIndex == wordIndex; });
      glyphs.insert(glyphs.end(), current.begin(), current.end());
    }
  }

  std::vector<JustificationCandidate> candidates;
  const DeclPolicyJustifier policyEngine(*justInfo.catalog);
  policyEngine.collectStageCandidates(lineTextInfo.wordInfos.size(), glyphs, phases, justInfo.declPolicyState, [&](const FixedSlotActionSite& site, std::size_t phaseIndex, const FixedSlotCandidateScore& score) {
    FeatureStagingBackend backend(lineTextInfo, justInfo, *justInfo.catalog);
    auto candidate = collectJustificationCandidate(*justInfo.catalog, justInfo.catalog->actionDefinitions.at(site.definition), site, backend, score.total, static_cast<int>(phaseIndex), CandidateWidthMeasurement::Deferred);
    if (candidate) {
      candidate->occurrence = score.occurrence;
      candidate->baseWeight = score.baseWeight;
      candidate->occurrenceAdjustment = score.occurrenceAdjustment;
      candidate->subwordLengthAdjustment = score.subwordLengthAdjustment;
      candidate->centralityAdjustment = score.centralityAdjustment;
      candidate->wordPositionAdjustment = score.wordPositionAdjustment;
      candidates.push_back(std::move(*candidate));
    }
  });
  return candidates;
}

namespace {

struct CandidateKey {
  int phase = -1;
  int word = -1;
  int subword = -1;
  JustificationRuleId rule = 0;
  int site = -1;
  auto operator<=>(const CandidateKey&) const = default;
};

CandidateKey candidateKey(const JustificationCandidate& candidate) {
  const auto site = candidate.matchOffset >= 0 && candidate.matchOffset < static_cast<int>(candidate.context.size()) ? candidate.context[candidate.matchOffset].indexInLine : -1;
  return {candidate.priorityBand, candidate.wordIndex, candidate.subwordIndex, candidate.rule, site};
}

const RuleSelection* candidateChoice(const SelectionPolicy& selection, const JustificationCandidate& candidate) {
  const auto found = std::find_if(selection.rules.begin(), selection.rules.end(), [&](const auto& choice) { return choice.rule == candidate.rule; });
  return found == selection.rules.end() ? nullptr : &*found;
}

bool passesCandidatePositionRecords(std::span<const std::string> differentSubword, std::span<const std::string> differentPosition, const JustificationCandidate& candidate, const std::map<std::string, DeclPolicyRecordedPosition>& records) {
  const auto baseIndex = candidate.context.at(candidate.matchOffset).baseIndex;
  for (const auto& name : differentSubword) {
    const auto found = records.find(name);
    if (found != records.end() && found->second.subwordIndex == candidate.subwordIndex) return false;
  }
  for (const auto& name : differentPosition) {
    const auto found = records.find(name);
    if (found != records.end() && found->second.subwordIndex == candidate.subwordIndex && found->second.baseIndex == baseIndex) return false;
  }
  return true;
}

bool passesCandidateRecords(const SelectionPolicy& selection, const JustificationCandidate& candidate, const std::map<std::string, DeclPolicyRecordedPosition>& records) {
  const auto* choice = candidateChoice(selection, candidate);
  if (choice == nullptr) return false;
  const auto hasAllRecords = [&](std::span<const std::string> names) {
    return std::all_of(names.begin(), names.end(), [&](const auto& name) { return records.contains(name); });
  };
  if (!hasAllRecords(selection.requireRecorded) || !hasAllRecords(choice->requireRecorded)) return false;
  const auto hasRecorded = [&](std::span<const std::string> names) {
    return std::any_of(names.begin(), names.end(), [&](const auto& name) {
      const auto found = records.find(name);
      return found != records.end();
    });
  };
  if (hasRecorded(selection.forbidRecorded) || hasRecorded(choice->forbidRecorded)) return false;
  return passesCandidatePositionRecords(selection.differentSubword, selection.differentPosition, candidate, records) && passesCandidatePositionRecords(choice->differentSubword, choice->differentPosition, candidate, records);
}

void recordCandidate(const SelectionPolicy& selection, const JustificationCandidate& candidate, std::map<std::string, DeclPolicyRecordedPosition>& records) {
  const auto position = DeclPolicyRecordedPosition{candidate.subwordIndex, candidate.context.at(candidate.matchOffset).baseIndex};
  if (!selection.record.empty()) records.insert_or_assign(selection.record, position);
  const auto* choice = candidateChoice(selection, candidate);
  if (choice != nullptr && !choice->record.empty()) records.insert_or_assign(choice->record, position);
}

FixedSlotActionResult commitCandidate(const LineTextInfo& lineTextInfo, JustInfo& justInfo, const JustificationCandidate& candidate, double ratio, unsigned parameterQuantization) {
  FeatureStagingBackend backend(lineTextInfo, justInfo, *justInfo.catalog);
  backend.beginTransaction();
  for (const auto site : candidate.clearedSites) backend.clear(site);
  for (const auto& substitution : candidate.substitutions) backend.applyLookup(substitution.site, substitution.lookup);
  for (const auto& parameter : candidate.parameters) {
    const auto value = quantizeProportionalParameter(parameter.minimumValue + ratio * (parameter.maximumValue - parameter.minimumValue), parameter.minimumValue, parameter.maximumValue, parameterQuantization);
    backend.update(parameter.site, parameter.attribute, value);
  }
  return backend.commitTransaction(candidate.wordIndex);
}

bool measureCandidate(const LineTextInfo& lineTextInfo, JustInfo& justInfo, JustificationCandidate& candidate, CandidateWidthMode candidateWidth) {
  FeatureStagingBackend backend(lineTextInfo, justInfo, *justInfo.catalog, candidateWidth);
  return measureJustificationCandidate(candidate, backend);
}

void traceCandidateDecision(const JustInfo& justInfo, std::span<const PolicyPhase> phases, const JustificationCandidate& candidate, int pass, std::string_view decision, std::string_view reason, double remainingWidth, double ratio, unsigned parameterQuantization) {
  if (!justInfo.layout->justificationTracingEnabled()) return;
  JustificationDecisionTrace trace;
  trace.lineIndex = justInfo.lineIndex;
  trace.pass = pass;
  trace.priorityBand = candidate.priorityBand;
  if (candidate.priorityBand >= 0 && candidate.priorityBand < static_cast<int>(phases.size())) {
    const auto selectionIndex = phases[candidate.priorityBand].selection;
    if (selectionIndex < justInfo.catalog->selections.size()) trace.selection = justInfo.catalog->selections[selectionIndex].name;
  }
  const auto rule = std::find_if(justInfo.catalog->rules.begin(), justInfo.catalog->rules.end(), [&](const auto& value) { return value.id == candidate.rule; });
  if (rule != justInfo.catalog->rules.end()) trace.rule = rule->name;
  trace.wordIndex = candidate.wordIndex;
  trace.subwordIndex = candidate.subwordIndex;
  trace.site = candidateKey(candidate).site;
  trace.subwordLength = candidate.subwordLength;
  trace.connectionAfter = candidate.connectionAfter;
  trace.occurrence = candidate.occurrence;
  trace.baseWeight = candidate.baseWeight;
  trace.occurrenceAdjustment = candidate.occurrenceAdjustment;
  trace.subwordLengthAdjustment = candidate.subwordLengthAdjustment;
  trace.centralityAdjustment = candidate.centralityAdjustment;
  trace.wordPositionAdjustment = candidate.wordPositionAdjustment;
  trace.score = candidate.weight;
  trace.minimumWidthDelta = candidate.minimumWidthDelta;
  trace.maximumWidthDelta = candidate.maximumWidthDelta;
  trace.remainingWidth = remainingWidth;
  trace.appliedRatio = ratio;
  trace.decision = decision;
  trace.reason = reason;
  const bool applies = trace.decision == "selected" || trace.decision == "applied" || trace.reason.starts_with("commit_");
  for (const auto& parameter : candidate.parameters) {
    const auto appliedValue = applies ? quantizeProportionalParameter(parameter.minimumValue + ratio * (parameter.maximumValue - parameter.minimumValue), parameter.minimumValue, parameter.maximumValue, parameterQuantization) : parameter.initialValue;
    trace.parameters.push_back({parameter.site, justInfo.catalog->attributes.at(parameter.attribute), parameter.initialValue, parameter.minimumValue, parameter.maximumValue, appliedValue});
  }
  justInfo.layout->traceJustificationDecision(trace);
}

void distributeCandidateWidth(const std::vector<const JustificationCandidate*>& candidates, double available, std::vector<double>& allocations) {
  allocations.assign(candidates.size(), 0);
  std::vector<std::size_t> active;
  active.reserve(candidates.size());
  for (std::size_t index = 0; index < candidates.size(); ++index) {
    const auto capacity = *candidates[index]->maximumWidthDelta - *candidates[index]->minimumWidthDelta;
    if (capacity > 0) active.push_back(index);
  }
  while (available > 0.000001 && !active.empty()) {
    double totalWeight = 0;
    for (const auto index : active) totalWeight += candidates[index]->weight;
    const auto unit = available / totalWeight;
    std::vector<std::size_t> remaining;
    bool saturated = false;
    for (const auto index : active) {
      const auto capacity = *candidates[index]->maximumWidthDelta - *candidates[index]->minimumWidthDelta;
      const auto room = capacity - allocations[index];
      const auto share = unit * candidates[index]->weight;
      if (room <= share + 0.000001) {
        allocations[index] += room;
        available -= room;
        saturated = true;
      } else {
        remaining.push_back(index);
      }
    }
    if (!saturated) {
      for (const auto index : active) allocations[index] += unit * candidates[index]->weight;
      break;
    }
    active = std::move(remaining);
  }
}

// Baseline spacing is collected once, including adjacent connections. It has
// no substitutions or records and remains invisible to normal action reads.
// Merge by (site, axis), then shape whole words: individual candidate widths
// cannot account for two sides of the same glyph being adjusted together.
bool applyBaselinePoolStage(const LineTextInfo& text, JustInfo& info, std::span<const PolicyPhase> phases, unsigned quantization) {
  const double available = info.desiredWidth - info.textLineWidth;
  if (available <= 0) return false;
  if (!info.layout->supportsGlyphParameters()) throw std::runtime_error("baseline_pool requires native glyph parameters");
  const auto candidates = collectDeclPolicyStageCandidates(text, info, phases);
  map<int, vector<TextFontFeatures>> endpoints;
  std::set<int> words;
  vector<const JustificationCandidate*> accepted;
  for (const auto& candidate : candidates) {
    if (!candidate.clearedSites.empty() || !candidate.substitutions.empty()) throw std::runtime_error("baseline_pool actions must not clear or substitute glyphs");
    bool eligible = !candidate.parameters.empty();
    for (const auto& parameter : candidate.parameters) {
      const auto axis = info.catalog->attributeAxes.at(parameter.attribute);
      if (axis == NoGlyphAxis || parameter.minimumValue != 0 || !std::isfinite(parameter.maximumValue) || parameter.maximumValue <= 0) throw std::runtime_error("baseline_pool requires native vary ranges from zero to a positive endpoint");
      const auto glyph = glyphStateAtSite(info, info.fontFeatures, info.substitutions, parameter.site);
      const auto maximum = glyph ? info.layout->glyphParameterMaximum(glyph->codepoint, axis) : std::nullopt;
      if (!glyph || glyph->parameters.value(axis) != 0 || !maximum || *maximum < parameter.maximumValue) eligible = false;
    }
    if (!eligible) {
      traceCandidateDecision(info, phases, candidate, 0, "rejected", "baseline_unsupported_or_nonzero", available, 0, 0);
      continue;
    }
    for (const auto& parameter : candidate.parameters) {
      auto& values = endpoints[parameter.site];
      const auto axis = info.catalog->attributeAxes.at(parameter.attribute);
      const auto found = std::find_if(values.begin(), values.end(), [&](const auto& value) { return value.axis == axis; });
      if (found == values.end()) values.push_back({info.catalog->attributes.at(parameter.attribute), parameter.maximumValue, axis});
      else if (found->value != parameter.maximumValue) throw std::runtime_error("baseline_pool has conflicting endpoints for the same glyph axis");
    }
    words.insert(candidate.wordIndex);
    accepted.push_back(&candidate);
  }
  if (words.empty()) return false;

  // Quantize the common ratio, not each side separately, so the ascendant
  // half-length relation remains exact even when the line has little room.
  double ratio = 1;
  for (int attempt = 0; attempt < 16 && ratio > 0; ++attempt) {
    auto baseline = info.baselineFeatures;
    for (const auto& [site, values] : endpoints) {
      for (auto value : values) {
        value.value *= ratio;
        baseline[site].push_back(std::move(value));
      }
    }
    vector<ShapingBuffer> buffers(text.wordInfos.size());
    vector<double> widths(text.wordInfos.size());
    double delta = 0;
    bool nonShrinking = true;
    for (const auto word : words) {
      widths[word] = getWordWidth(text.wordInfos[word], info.fontFeatures, info.font, &buffers[word], info.layout, info.globalFeatures, info.substitutions, baseline);
      const double wordDelta = widths[word] - info.layoutResult[word].parWidth;
      nonShrinking &= wordDelta >= 0;
      delta += wordDelta;
    }
    if (nonShrinking && delta > 0 && delta < available) {
      info.baselineFeatures = std::move(baseline);
      info.textLineWidth += delta;
      info.acceptedWordBuffers.resize(text.wordInfos.size());
      for (const auto word : words) {
        info.layoutResult[word].parWidth = widths[word];
        info.acceptedWordBuffers[word] = std::move(buffers[word]);
      }
      info.declPolicyState.currentGlyphs.clear();
      for (const auto* candidate : accepted) traceCandidateDecision(info, phases, *candidate, 0, "applied", "baseline_spacing", available, ratio, 0);
      return false;
    }
    if (!nonShrinking || delta <= 0) break;
    ratio *= std::min(0.95, available / delta * 0.99);
    if (quantization) ratio = std::floor(ratio * quantization) / quantization;
  }
  for (const auto* candidate : accepted) traceCandidateDecision(info, phases, *candidate, 0, "rejected", "baseline_does_not_fit", available, 0, 0);
  return false;
}

constexpr double candidateWidthReserve = 0.5;

double capCandidateRatioToRemainingWidth(const JustificationCandidate& candidate, double ratio, double remainingWidth) {
  const auto capacity = *candidate.maximumWidthDelta - *candidate.minimumWidthDelta;
  if (capacity <= 0) return 0;
  const auto ratioThatFits = std::clamp((remainingWidth - *candidate.minimumWidthDelta - candidateWidthReserve) / capacity, 0.0, 1.0);
  return std::min(ratio, ratioThatFits);
}

bool applyCandidatePoolStage(const LineTextInfo& lineTextInfo, JustInfo& justInfo, std::span<const PolicyPhase> phases, unsigned parameterQuantization, CandidateWidthMode candidateWidth) {
  std::map<CandidateKey, int> repetitions;
  std::set<CandidateKey> blocked;
  std::vector<int> phaseTotals(phases.size());
  std::map<int, int> wordTotals;
  std::map<std::pair<int, int>, int> subwordTotals;
  int pass = 0;

  while (justInfo.textLineWidth + 0.000001 < justInfo.desiredWidth) {
    ++pass;
    auto candidates = collectDeclPolicyStageCandidates(lineTextInfo, justInfo, phases);
    std::stable_sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) {
      if (left.priorityBand != right.priorityBand) return left.priorityBand < right.priorityBand;
      return left.weight > right.weight;
    });

    int activeBand = -1;
    std::vector<const JustificationCandidate*> accepted;
    std::set<JustificationSiteRef> touchedSites;
    auto trialRecords = justInfo.declPolicyState.recordsByWord;
    if (trialRecords.empty()) trialRecords.resize(lineTextInfo.wordInfos.size());
    auto trialPhaseTotals = phaseTotals;
    auto trialWordTotals = wordTotals;
    auto trialSubwordTotals = subwordTotals;
    double mandatoryWidth = 0;
    const auto availableWidth = justInfo.desiredWidth - justInfo.textLineWidth;

    for (auto& candidate : candidates) {
      const auto reject = [&](std::string_view reason) { traceCandidateDecision(justInfo, phases, candidate, pass, "rejected", reason, availableWidth - mandatoryWidth, 0, parameterQuantization); };
      if (candidate.priorityBand < 0 || candidate.priorityBand >= static_cast<int>(phases.size())) {
        reject("invalid_priority_band");
        continue;
      }
      const auto& phase = phases[candidate.priorityBand];
      const auto& selection = justInfo.catalog->selections.at(phase.selection);
      const auto key = candidateKey(candidate);
      if (blocked.contains(key)) {
        reject("blocked_after_failed_commit");
        continue;
      }
      if (repetitions[key] >= phase.levels) {
        reject("repetition_limit");
        continue;
      }
      if (phase.limit != 0 && trialPhaseTotals[candidate.priorityBand] >= phase.limit) {
        reject("phase_limit");
        continue;
      }
      if (phase.perWord != 0 && trialWordTotals[candidate.wordIndex] >= phase.perWord) {
        reject("word_limit");
        continue;
      }
      if (phase.perSubword != 0 && trialSubwordTotals[{candidate.wordIndex, candidate.subwordIndex}] >= phase.perSubword) {
        reject("subword_limit");
        continue;
      }
      if (!passesCandidateRecords(selection, candidate, trialRecords[candidate.wordIndex])) {
        reject("record_constraint");
        continue;
      }
      if (activeBand != -1 && candidate.priorityBand != activeBand) {
        reject("lower_priority_band");
        continue;
      }

      std::set<JustificationSiteRef> candidateSites(candidate.clearedSites.begin(), candidate.clearedSites.end());
      for (const auto& parameter : candidate.parameters) candidateSites.insert(parameter.site);
      for (const auto& substitution : candidate.substitutions) candidateSites.insert(substitution.site);
      if (std::any_of(candidateSites.begin(), candidateSites.end(), [&](const auto site) { return touchedSites.contains(site); })) {
        reject("site_conflict");
        continue;
      }
      if (!measureCandidate(lineTextInfo, justInfo, candidate, candidateWidth)) {
        reject("unmeasured_width");
        continue;
      }
      if (*candidate.maximumWidthDelta <= 0) {
        reject("non_expanding");
        continue;
      }
      if (mandatoryWidth + *candidate.minimumWidthDelta > availableWidth - 0.000001) {
        reject("mandatory_width_does_not_fit");
        continue;
      }
      if (activeBand == -1) activeBand = candidate.priorityBand;

      accepted.push_back(&candidate);
      touchedSites.insert(candidateSites.begin(), candidateSites.end());
      mandatoryWidth += *candidate.minimumWidthDelta;
      ++trialPhaseTotals[candidate.priorityBand];
      ++trialWordTotals[candidate.wordIndex];
      ++trialSubwordTotals[{candidate.wordIndex, candidate.subwordIndex}];
      recordCandidate(selection, candidate, trialRecords[candidate.wordIndex]);
    }

    if (accepted.empty()) break;
    std::vector<double> allocations;
    distributeCandidateWidth(accepted, std::max(0.0, availableWidth - mandatoryWidth - candidateWidthReserve), allocations);
    bool changed = false;
    for (std::size_t index = 0; index < accepted.size(); ++index) {
      const auto& candidate = *accepted[index];
      const auto capacity = *candidate.maximumWidthDelta - *candidate.minimumWidthDelta;
      const auto allocatedRatio = capacity > 0 ? std::clamp(allocations[index] / capacity, 0.0, 1.0) : 0;
      const auto ratio = capCandidateRatioToRemainingWidth(candidate, allocatedRatio, justInfo.desiredWidth - justInfo.textLineWidth);
      const auto reason = ratio + 0.000000000001 < allocatedRatio ? "candidate_pool_width_cap" : "candidate_pool";
      traceCandidateDecision(justInfo, phases, candidate, pass, "selected", reason, justInfo.desiredWidth - justInfo.textLineWidth, ratio, parameterQuantization);
      const auto result = commitCandidate(lineTextInfo, justInfo, candidate, ratio, parameterQuantization);
      const auto key = candidateKey(candidate);
      if (result != FixedSlotActionResult::Positive) {
        const auto reason = result == FixedSlotActionResult::Overflow ? "commit_overflow" : result == FixedSlotActionResult::Forbidden ? "commit_forbidden"
                                                                                                                                       : "commit_no_change";
        traceCandidateDecision(justInfo, phases, candidate, pass, "rejected", reason, justInfo.desiredWidth - justInfo.textLineWidth, ratio, parameterQuantization);
        blocked.insert(key);
        continue;
      }
      traceCandidateDecision(justInfo, phases, candidate, pass, "applied", "accepted", justInfo.desiredWidth - justInfo.textLineWidth, ratio, parameterQuantization);
      changed = true;
      ++repetitions[key];
      ++phaseTotals[candidate.priorityBand];
      const auto& selection = justInfo.catalog->selections.at(phases[candidate.priorityBand].selection);
      ++wordTotals[candidate.wordIndex];
      ++subwordTotals[{candidate.wordIndex, candidate.subwordIndex}];
      if (justInfo.declPolicyState.recordsByWord.empty()) justInfo.declPolicyState.recordsByWord.resize(lineTextInfo.wordInfos.size());
      recordCandidate(selection, candidate, justInfo.declPolicyState.recordsByWord[candidate.wordIndex]);
      justInfo.declPolicyState.currentGlyphs.clear();
    }
    if (!changed) break;
  }
  return false;
}

}  // namespace

bool applyDeclPolicyStage(const LineTextInfo& lineTextInfo, JustInfo& justInfo, std::span<const PolicyPhase> phases, unsigned parameterQuantization, CandidateWidthMode candidateWidth) {
  if (justInfo.declPolicyState.stopped) return true;
  if (justInfo.catalog == nullptr) {
    throw std::runtime_error("declarative-policy justification requires table(justdfa)");
  }
  if (justInfo.recognitionBuffer == nullptr) {
    throw std::runtime_error(
        "declarative-policy justification requires a prepared glyph buffer");
  }
  if (justInfo.layout == nullptr) {
    throw std::runtime_error(
        "declarative-policy justification requires a glyph-name provider");
  }
  if (!phases.empty() && phases.front().allocator == PolicyPhase::Allocator::CandidatePool) return applyCandidatePoolStage(lineTextInfo, justInfo, phases, parameterQuantization, candidateWidth);
  if (!phases.empty() && phases.front().allocator == PolicyPhase::Allocator::BaselinePool) return applyBaselinePoolStage(lineTextInfo, justInfo, phases, parameterQuantization);
  const auto locations = prepareBaseLocations(lineTextInfo);
  const auto recognizeAcceptedWord = [&](int wordIndex) {
    const auto& word = lineTextInfo.wordInfos.at(wordIndex);
    auto* buffer = wordIndex < static_cast<int>(justInfo.acceptedWordBuffers.size()) ? justInfo.acceptedWordBuffers[wordIndex].get() : nullptr;
    if (buffer != nullptr) {
      updateAttachmentGlyphs(lineTextInfo, justInfo.font, buffer, *justInfo.layout, *justInfo.catalog, justInfo.attachmentGlyphs, word.startIndex, word.endIndex - word.startIndex + 1, &justInfo.substitutions);
      return prepareFixedSlotGlyphs(lineTextInfo, justInfo.font, buffer, *justInfo.layout, *justInfo.catalog, locations, word.startIndex, &justInfo.substitutions);
    }
    updateAttachmentGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, justInfo.attachmentGlyphs, 0, -1, &justInfo.substitutions);
    auto current = prepareFixedSlotGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, locations, 0, &justInfo.substitutions);
    std::erase_if(current, [&](const auto& glyph) { return glyph.wordIndex != wordIndex; });
    return current;
  };

  std::vector<FixedSlotGlyphInput> glyphs;
  if (justInfo.declPolicyState.currentGlyphs.empty()) {
    glyphs = prepareFixedSlotGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, locations, 0, &justInfo.substitutions);
    updateAttachmentGlyphs(lineTextInfo, justInfo.font, justInfo.recognitionBuffer, *justInfo.layout, *justInfo.catalog, justInfo.attachmentGlyphs, 0, -1, &justInfo.substitutions);
    for (int wordIndex = 0; wordIndex < static_cast<int>(justInfo.acceptedWordBuffers.size()); ++wordIndex) {
      if (!justInfo.acceptedWordBuffers[wordIndex]) continue;
      auto current = recognizeAcceptedWord(wordIndex);
      std::erase_if(glyphs, [&](const auto& glyph) { return glyph.wordIndex == wordIndex; });
      glyphs.insert(glyphs.end(), current.begin(), current.end());
    }
  }
  const DeclPolicyJustifier policyEngine(*justInfo.catalog);
  return policyEngine.applyStage(
      lineTextInfo.wordInfos.size(), glyphs, phases, justInfo.declPolicyState,
      [&](const FixedSlotActionSite& site) {
        FeatureStagingBackend backend(lineTextInfo, justInfo,
                                      *justInfo.catalog);
        return evaluateJustificationAction(
            *justInfo.catalog,
            justInfo.catalog->actionDefinitions[site.definition],
            site.context, site.matchOffset, site.anchorSlot, site.wordIndex,
            backend);
      },
      recognizeAcceptedWord);
}

}  // namespace digitalkhatt::justify::decl
