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
    parameter.continuationMaximum.reset();
    parameter.continuationMinimum = 0;
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
    parameter.continuationMaximum = endpoint;
    parameter.continuationMinimum = parameter.minimumValue;
    // `vary` offers an endpoint; it does not change the mandatory staged
    // value observed by effects that follow it.
  }

  void varyWithMaximum(JustificationSiteRef site, JustAttributeId attribute, double preferred, double maximum) override {
    if (!std::isfinite(maximum) || maximum < preferred) throw std::runtime_error("vary maximum must be finite and at least the preferred endpoint");
    vary(site, attribute, preferred);
    parameterAt(site, attribute).continuationMaximum = maximum;
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

bool limitJustificationCandidateContinuation(JustificationCandidate& candidate, StageContinuation continuation) {
  if (!std::isfinite(continuation.limit) || continuation.limit < 0 || (continuation.mode == StageContinuation::Mode::By && continuation.limit == 0) || continuation.mode == StageContinuation::Mode::None) throw std::invalid_argument("invalid candidate continuation limit");
  if (continuation.percentage) {
    if ((continuation.mode != StageContinuation::Mode::By && continuation.mode != StageContinuation::Mode::To) || continuation.limit > 100) throw std::invalid_argument("invalid candidate continuation percentage");
    const auto endpoint = [&](const JustificationCandidateParameter& parameter) {
      const double maximum = *parameter.continuationMaximum;
      const double amount = continuation.limit / 100 * (maximum - parameter.continuationMinimum);
      if (continuation.mode == StageContinuation::Mode::To) return continuation.limit == 100 ? maximum : parameter.continuationMinimum + amount;
      return std::min(maximum, parameter.initialValue + amount);
    };
    bool grows = false;
    for (const auto& parameter : candidate.parameters) {
      if (!parameter.continuationMaximum) continue;
      const double value = endpoint(parameter);
      // Already accepted axes may be above a percentage target. Keep them
      // unchanged, but never activate a new site below its mandatory minimum.
      if (!std::isfinite(value) || (value + 1e-12 < parameter.minimumValue && parameter.minimumValue > parameter.initialValue)) return false;
      grows |= value > parameter.initialValue;
    }
    if (!grows) return false;
    for (auto& parameter : candidate.parameters) {
      if (parameter.continuationMaximum) parameter.maximumValue = std::max(parameter.minimumValue, endpoint(parameter));
    }
    candidate.minimumWidthDelta.reset();
    candidate.maximumWidthDelta.reset();
    return true;
  }
  double span = 0;
  double initial = 0;
  for (const auto& parameter : candidate.parameters) {
    if (parameter.continuationMaximum) {
      initial = parameter.initialValue;
      span = *parameter.continuationMaximum - parameter.initialValue;
      break;
    }
  }
  if (span <= 0) return false;
  const double offered = continuation.mode == StageContinuation::Mode::By ? continuation.limit : continuation.mode == StageContinuation::Mode::To ? continuation.limit - initial : span;
  if (offered <= 0) return false;
  const double ratio = std::min(1.0, offered / span);
  for (const auto& parameter : candidate.parameters) {
    if (!parameter.continuationMaximum) continue;
    const double endpoint = parameter.initialValue + ratio * (*parameter.continuationMaximum - parameter.initialValue);
    if (!std::isfinite(endpoint) || endpoint + 1e-12 < parameter.minimumValue) return false;
  }
  for (auto& parameter : candidate.parameters) {
    if (parameter.continuationMaximum) parameter.maximumValue = std::max(parameter.minimumValue, parameter.initialValue + ratio * (*parameter.continuationMaximum - parameter.initialValue));
  }
  candidate.minimumWidthDelta.reset();
  candidate.maximumWidthDelta.reset();
  return true;
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
