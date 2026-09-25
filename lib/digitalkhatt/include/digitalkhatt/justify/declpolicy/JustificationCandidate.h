#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <digitalkhatt/justify/declpolicy/JustificationStagingBackend.h>

namespace digitalkhatt::justify {

// One parameter contribution offered by a candidate. `minimumValue` is the
// mandatory state installed when the candidate is activated;
// `maximumValue` is the optional endpoint offered by `vary`. They are equal
// for today's discrete actions.
struct JustificationCandidateParameter {
  JustificationSiteRef site = kNoSite;
  JustAttributeId attribute = 0;
  double initialValue = 0;
  double minimumValue = 0;
  double maximumValue = 0;
  bool operator==(const JustificationCandidateParameter&) const = default;
};

// A named lookup remains in the candidate so it can be replayed. Backends
// that expose their staged glyph state also provide the resolved glyph pair.
struct JustificationCandidateSubstitution {
  JustificationSiteRef site = kNoSite;
  std::string lookup;
  std::optional<std::uint32_t> sourceGlyph;
  std::optional<std::uint32_t> replacementGlyph;
  bool operator==(const JustificationCandidateSubstitution&) const = default;
};

// Immutable description of one matched action. It owns its subword context so
// a line planner can retain candidates after recognition's temporary spans go
// out of scope. Width deltas are deliberately optional: the font adapter adds
// them in the measurement pass, while policy-only tests need no font.
struct JustificationCandidate {
  int definition = -1;
  JustificationRuleId rule = 0;
  int wordIndex = -1;
  int subwordIndex = -1;
  int matchOffset = 0;
  int anchorSlot = 0;
  int subwordLength = 0;
  // One-based letter position after which a two-slot connection occurs.
  // Single-glyph alternates and wider patterns do not imply a connection.
  int connectionAfter = -1;
  int priorityBand = 0;
  int occurrence = 0;
  double baseWeight = 1;
  double occurrenceAdjustment = 0;
  double subwordLengthAdjustment = 0;
  double centralityAdjustment = 0;
  double wordPositionAdjustment = 0;
  double weight = 1;
  std::optional<double> minimumWidthDelta;
  std::optional<double> maximumWidthDelta;
  std::vector<FixedSlotSlot> context;
  std::vector<JustificationSiteRef> clearedSites;
  std::vector<JustificationCandidateParameter> parameters;
  std::vector<JustificationCandidateSubstitution> substitutions;
};

struct JustificationDecisionParameter {
  JustificationSiteRef site = kNoSite;
  std::string attribute;
  double initialValue = 0;
  double minimumValue = 0;
  double maximumValue = 0;
  double appliedValue = 0;
};

struct JustificationDecisionTrace {
  int lineIndex = -1;
  int pass = 0;
  int priorityBand = 0;
  std::string selection;
  std::string rule;
  int wordIndex = -1;
  int subwordIndex = -1;
  int site = -1;
  int subwordLength = 0;
  int connectionAfter = -1;
  int occurrence = 0;
  double baseWeight = 1;
  double occurrenceAdjustment = 0;
  double subwordLengthAdjustment = 0;
  double centralityAdjustment = 0;
  double wordPositionAdjustment = 0;
  double score = 1;
  std::optional<double> minimumWidthDelta;
  std::optional<double> maximumWidthDelta;
  double remainingWidth = 0;
  double appliedRatio = 0;
  std::string decision;
  std::string reason;
  std::vector<JustificationDecisionParameter> parameters;
};

enum class CandidateWidthMeasurement : std::uint8_t { Immediate,
                                                      Deferred };

// Evaluates an action against staged state, captures the resulting candidate,
// then aborts the transaction. A forbidden action produces no candidate.
// Committed state is never changed.
std::optional<JustificationCandidate> collectJustificationCandidate(
    const CompiledJustificationCatalog& catalog,
    const JustificationAction& action,
    const FixedSlotActionSite& site,
    JustificationStagingBackend& backend,
    double weight = 1,
    int priorityBand = 0,
    CandidateWidthMeasurement widthMeasurement = CandidateWidthMeasurement::Immediate);

// Replays a previously collected candidate into a transaction, measures its
// mandatory and maximum endpoints, then aborts. This lets a line planner
// eliminate candidates by policy limits and site conflicts before paying for
// font shaping.
bool measureJustificationCandidate(JustificationCandidate& candidate,
                                   JustificationStagingBackend& backend);

}  // namespace digitalkhatt::justify
