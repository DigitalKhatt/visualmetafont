#include <digitalkhatt/justify/declpolicy/JustificationCandidate.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include <digitalkhatt/justify/declpolicy/JustificationActionEvaluator.h>

namespace digitalkhatt::justify {
namespace {

class CandidateRecordingBackend final : public JustificationStagingBackend {
 public:
  CandidateRecordingBackend(JustificationStagingBackend& backend,
                            JustificationCandidate& candidate,
                            bool measureWidths)
      : backend_(backend), candidate_(candidate), measureWidths_(measureWidths) {}

  void beginTransaction() override { backend_.beginTransaction(); }

  void abortTransaction() override { backend_.abortTransaction(); }

  FixedSlotActionResult commitTransaction(int wordIndex) override {
    // Collection is a deliberately aborted dry run. The caller will replay
    // the candidate only after the line planner has selected it.
    if (measureWidths_) {
      candidate_.minimumWidthDelta = backend_.measureStagedWidthDelta(wordIndex);
      bool hasRange = false;
      for (const auto& parameter : candidate_.parameters) {
        if (parameter.maximumValue == parameter.minimumValue) continue;
        backend_.update(parameter.site, parameter.attribute, parameter.maximumValue);
        hasRange = true;
      }
      candidate_.maximumWidthDelta = hasRange ? backend_.measureStagedWidthDelta(wordIndex) : candidate_.minimumWidthDelta;
    }
    backend_.abortTransaction();
    return FixedSlotActionResult::Positive;
  }

  bool present(JustificationSiteRef site,
               JustAttributeId attribute) const override {
    return backend_.present(site, attribute);
  }

  double read(JustificationSiteRef site,
              JustAttributeId attribute) const override {
    return backend_.read(site, attribute);
  }

  JustificationLineMetrics lineMetrics() const override {
    return backend_.lineMetrics();
  }

  void update(JustificationSiteRef site,
              JustAttributeId attribute,
              double value) override {
    auto& parameter = parameterAt(site, attribute);
    parameter.minimumValue = value;
    parameter.maximumValue = value;
    backend_.update(site, attribute, value);
  }

  void clear(JustificationSiteRef site) override {
    if (std::find(candidate_.clearedSites.begin(),
                  candidate_.clearedSites.end(), site) ==
        candidate_.clearedSites.end()) {
      candidate_.clearedSites.push_back(site);
    }
    std::erase_if(candidate_.parameters,
                  [&](const auto& parameter) { return parameter.site == site; });
    std::erase_if(candidate_.substitutions,
                  [&](const auto& substitution) { return substitution.site == site; });
    backend_.clear(site);
  }

  void applyLookup(JustificationSiteRef site,
                   std::string_view lookup) override {
    const auto source = backend_.glyph(site);
    backend_.applyLookup(site, lookup);
    const auto replacement = backend_.glyph(site);
    if (!source || !replacement || source != replacement) {
      candidate_.substitutions.push_back({
          .site = site,
          .lookup = std::string(lookup),
          .sourceGlyph = source,
          .replacementGlyph = replacement,
      });
    }
  }

  void vary(JustificationSiteRef site,
            JustAttributeId attribute,
            double endpoint) override {
    auto& parameter = parameterAt(site, attribute);
    parameter.minimumValue = backend_.read(site, attribute);
    parameter.maximumValue = endpoint;
    // `vary` offers an endpoint; it does not change the mandatory staged
    // value observed by effects that follow it.
  }

  JustificationSiteRef attachment(
      JustificationSiteRef anchor,
      JustAttachmentId attachment) const override {
    return backend_.attachment(anchor, attachment);
  }

  std::optional<std::uint32_t> glyph(
      JustificationSiteRef site) const override {
    return backend_.glyph(site);
  }

 private:
  JustificationCandidateParameter& parameterAt(
      JustificationSiteRef site,
      JustAttributeId attribute) {
    const auto found = std::find_if(
        candidate_.parameters.begin(), candidate_.parameters.end(),
        [&](const auto& parameter) {
          return parameter.site == site && parameter.attribute == attribute;
        });
    if (found != candidate_.parameters.end()) return *found;
    const auto initial = backend_.read(site, attribute);
    return candidate_.parameters.emplace_back(JustificationCandidateParameter{
        .site = site,
        .attribute = attribute,
        .initialValue = initial,
        .minimumValue = initial,
        .maximumValue = initial,
    });
  }

  JustificationStagingBackend& backend_;
  JustificationCandidate& candidate_;
  bool measureWidths_;
};

}  // namespace

std::optional<JustificationCandidate> collectJustificationCandidate(
    const CompiledJustificationCatalog& catalog,
    const JustificationAction& action,
    const FixedSlotActionSite& site,
    JustificationStagingBackend& backend,
    double weight,
    int priorityBand,
    CandidateWidthMeasurement widthMeasurement) {
  if (!std::isfinite(weight) || weight < 0) {
    throw std::invalid_argument(
        "justification candidate weight must be finite and non-negative");
  }

  JustificationCandidate candidate{
      .definition = site.definition,
      .rule = site.rule,
      .wordIndex = site.wordIndex,
      .subwordIndex = site.subwordIndex,
      .matchOffset = site.matchOffset,
      .anchorSlot = site.anchorSlot,
      .subwordLength = static_cast<int>(site.context.size()),
      .connectionAfter = site.matchSlots.size() == 2
                             ? site.matchOffset + 1
                             : -1,
      .priorityBand = priorityBand,
      .baseWeight = weight,
      .weight = weight,
      .context = {site.context.begin(), site.context.end()},
  };
  CandidateRecordingBackend recording(backend, candidate, widthMeasurement == CandidateWidthMeasurement::Immediate);
  const auto result = evaluateJustificationAction(
      catalog, action, site.context, site.matchOffset, site.anchorSlot,
      site.wordIndex, recording);
  if (result == FixedSlotActionResult::Forbidden) return std::nullopt;
  return candidate;
}

bool measureJustificationCandidate(JustificationCandidate& candidate,
                                   JustificationStagingBackend& backend) {
  backend.beginTransaction();
  for (const auto site : candidate.clearedSites) backend.clear(site);
  for (const auto& substitution : candidate.substitutions) backend.applyLookup(substitution.site, substitution.lookup);
  for (const auto& parameter : candidate.parameters) backend.update(parameter.site, parameter.attribute, parameter.minimumValue);
  candidate.minimumWidthDelta = backend.measureStagedWidthDelta(candidate.wordIndex);
  bool hasRange = false;
  for (const auto& parameter : candidate.parameters) {
    if (parameter.maximumValue == parameter.minimumValue) continue;
    backend.update(parameter.site, parameter.attribute, parameter.maximumValue);
    hasRange = true;
  }
  candidate.maximumWidthDelta = hasRange ? backend.measureStagedWidthDelta(candidate.wordIndex) : candidate.minimumWidthDelta;
  backend.abortTransaction();
  return candidate.minimumWidthDelta.has_value() && candidate.maximumWidthDelta.has_value();
}

}  // namespace digitalkhatt::justify
