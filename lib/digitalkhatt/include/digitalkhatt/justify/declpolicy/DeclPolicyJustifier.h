#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <vector>

#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

namespace digitalkhatt::justify {

// Prepared by the layout adapter from its existing post-GSUB glyph stream.
// The policy engine deliberately has no HarfBuzz or font dependency.
struct FixedSlotGlyphInput {
  std::uint64_t facts = 0;
  int wordIndex = -1;
  int subwordIndex = -1;
  int baseIndex = -1;
  int indexInLine = -1;
};

// One glyph of a matched rule, carrying the latest accepted recognition facts.
// Facts reach the action because a declarative action needs to test them; the
// engine used to discard them at match time.
struct FixedSlotSlot {
  int baseIndex = -1;    // index into the subword's base indexes
  int indexInLine = -1;  // character index in the line text
  std::uint64_t facts = 0;
  bool operator==(const FixedSlotSlot&) const = default;
};

struct FixedSlotActionSite {
  // Index into CompiledJustificationCatalog::actionDefinitions.
  int definition = -1;
  // Slot the action is anchored at, as a 0-based index into `matchSlots`.
  int anchorSlot = 0;
  JustificationRuleId rule = 0;
  int wordIndex = -1;
  int subwordIndex = -1;
  // The matched slots in logical order.  A one-slot rule has exactly one.
  std::span<const FixedSlotSlot> matchSlots;
  // The whole joining subword the match sits in, and where the match starts
  // inside it.  An action addresses slots relative to the match, so it can
  // look past either end -- which is how a two-slot rule reaches the glyph
  // after it without changing what the DFA selects.
  std::span<const FixedSlotSlot> context;
  int matchOffset = 0;
};

enum class FixedSlotActionResult {
  NoChange,
  Positive,
  Overflow,
  Forbidden,
};

using FixedSlotActionCallback =
    std::function<FixedSlotActionResult(const FixedSlotActionSite&)>;

// A read-only stage inventory reports every currently eligible action site
// together with the phase that supplied its hard priority band.
struct FixedSlotCandidateScore {
  int occurrence = 0;
  double baseWeight = 1;
  double occurrenceAdjustment = 0;
  double subwordLengthAdjustment = 0;
  double centralityAdjustment = 0;
  double wordPositionAdjustment = 0;
  double total = 1;
};

using FixedSlotCandidateCallback = std::function<void(const FixedSlotActionSite&, std::size_t phaseIndex, const FixedSlotCandidateScore& score)>;

// Called only after a Positive transaction, once its glyph state is accepted.
// Returns the complete current word, with stable source-backed slot ids.
// Production supplies this callback; omit it only for immutable recognition.
using FixedSlotRecognitionCallback = std::function<std::vector<FixedSlotGlyphInput>(int wordIndex)>;

struct DeclPolicyRecordedPosition {
  int subwordIndex = -1;
  int baseIndex = -1;
};

// State shared by all stage invocations in one complete stretch recipe.
// Phase queues and counters are invocation-local; selection records persist.
struct DeclPolicyExecutionState {
  std::vector<std::map<std::string, DeclPolicyRecordedPosition>> recordsByWord;
  std::vector<FixedSlotGlyphInput> currentGlyphs;
  bool stopped = false;
};

class DeclPolicyJustifier {
 public:
  // `stretchPolicy` indexes catalog.stretchPolicies for the convenience
  // apply() entry point used by tests and embedders. Production executes each Stage step through applyStage() while the line interpreter handles the complete recipe.
  explicit DeclPolicyJustifier(const CompiledJustificationCatalog& catalog,
                               int stretchPolicy = 0)
      : catalog_(catalog), stretchPolicy_(stretchPolicy) {}

  bool apply(std::size_t wordCount,
             std::span<const FixedSlotGlyphInput> glyphs,
             const FixedSlotActionCallback& applyAction,
             const FixedSlotRecognitionCallback& recognizeAcceptedWord = {}) const;

  bool applyStage(std::size_t wordCount,
                  std::span<const FixedSlotGlyphInput> glyphs,
                  std::span<const PolicyPhase> phases,
                  DeclPolicyExecutionState& state,
                  const FixedSlotActionCallback& applyAction,
                  const FixedSlotRecognitionCallback& recognizeAcceptedWord = {}) const;

  // Enumerates a stable snapshot of all candidates across all words without
  // executing an action or changing recognition/record state.
  void collectStageCandidates(
      std::size_t wordCount,
      std::span<const FixedSlotGlyphInput> glyphs,
      std::span<const PolicyPhase> phases,
      const DeclPolicyExecutionState& state,
      const FixedSlotCandidateCallback& collectCandidate) const;

 private:
  const CompiledJustificationCatalog& catalog_;
  int stretchPolicy_ = 0;
};

}  // namespace digitalkhatt::justify
