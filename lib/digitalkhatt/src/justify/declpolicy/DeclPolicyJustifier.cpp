#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

namespace digitalkhatt::justify {
namespace {

// Extents into the word's subword-ordered slot list, so an action can see the
// whole subword without every opportunity copying it.
struct Opportunity {
  JustificationRuleId rule;
  int subwordIndex;
  int firstBaseIndex;
  int secondBaseIndex;
  int subwordFirst;
  int subwordCount;
  int matchOffset;
  int matchCount;
};

struct WordRecognition {
  std::vector<FixedSlotGlyphInput> glyphs;
  std::vector<FixedSlotSlot> slots;
  std::vector<Opportunity> opportunities;
  bool initialized = false;
};

void recognizeWord(int wordIndex, std::vector<FixedSlotGlyphInput> glyphs, const FixedSlotDfa& dfa, std::uint64_t predicateFacts, WordRecognition& state) {
  for (const auto& glyph : glyphs) {
    if (glyph.wordIndex != wordIndex || glyph.subwordIndex < 0 || glyph.baseIndex < 0 || glyph.indexInLine < 0) throw std::runtime_error("declarative-policy glyph has an invalid word or slot");
  }
  // HarfBuzz buffers are visual; policy slots keep stable source/logical order.
  std::stable_sort(glyphs.begin(), glyphs.end(), [](const auto& left, const auto& right) {
    if (left.subwordIndex != right.subwordIndex) return left.subwordIndex < right.subwordIndex;
    return left.baseIndex < right.baseIndex;
  });
  glyphs.erase(std::unique(glyphs.begin(), glyphs.end(), [](const auto& left, const auto& right) { return left.subwordIndex == right.subwordIndex && left.baseIndex == right.baseIndex; }), glyphs.end());

  bool rematch = !state.initialized || glyphs.size() != state.glyphs.size();
  if (!rematch) {
    for (std::size_t i = 0; i < glyphs.size(); ++i) {
      const auto& previous = state.glyphs[i];
      const auto& current = glyphs[i];
      if (previous.subwordIndex != current.subwordIndex || previous.baseIndex != current.baseIndex || previous.indexInLine != current.indexInLine || ((previous.facts ^ current.facts) & predicateFacts) != 0) {
        rematch = true;
        break;
      }
    }
  }
  state.glyphs = std::move(glyphs);
  state.initialized = true;
  // Refresh ALL facts, including guard-only facts not used in DFA predicates.
  state.slots.clear();
  state.slots.reserve(state.glyphs.size());
  for (const auto& glyph : state.glyphs) state.slots.push_back({.baseIndex = glyph.baseIndex, .indexInLine = glyph.indexInLine, .facts = glyph.facts});
  if (!rematch) return;

  state.opportunities.clear();
  const auto& tokens = state.glyphs;
  std::size_t firstToken = 0;
  while (firstToken < tokens.size()) {
    const auto subwordIndex = tokens[firstToken].subwordIndex;
    auto endToken = firstToken + 1;
    while (endToken < tokens.size() && tokens[endToken].subwordIndex == subwordIndex) ++endToken;
    std::vector<std::uint64_t> facts;
    facts.reserve(endToken - firstToken);
    for (auto token = firstToken; token < endToken; ++token) facts.push_back(tokens[token].facts);
    for (const auto& match : dfa.match(facts)) {
      state.opportunities.push_back({
          .rule = match.ruleId,
          .subwordIndex = subwordIndex,
          .firstBaseIndex = tokens[firstToken + match.first].baseIndex,
          .secondBaseIndex = tokens[firstToken + match.last].baseIndex,
          .subwordFirst = static_cast<int>(firstToken),
          .subwordCount = static_cast<int>(endToken - firstToken),
          .matchOffset = static_cast<int>(match.first),
          .matchCount = static_cast<int>(match.last - match.first + 1),
      });
    }
    firstToken = endToken;
  }
}

const RuleSelection* ruleSelection(const SelectionPolicy& policy, JustificationRuleId rule) {
  const auto found = std::find_if(policy.rules.begin(), policy.rules.end(), [rule](const auto& choice) { return choice.rule == rule; });
  return found == policy.rules.end() ? nullptr : &*found;
}

const JustificationActionBinding* actionFor(JustificationRuleId rule, std::span<const JustificationActionBinding> bindings) {
  const auto found = std::find_if(bindings.begin(), bindings.end(), [rule](const auto& binding) { return binding.rule == rule; });
  return found == bindings.end() ? nullptr : &*found;
}

bool hasAnyRecord(const std::map<std::string, DeclPolicyRecordedPosition>& records, std::span<const std::string> names) {
  return std::any_of(names.begin(), names.end(), [&](const auto& name) { return records.contains(name); });
}

bool hasAllRecords(const std::map<std::string, DeclPolicyRecordedPosition>& records, std::span<const std::string> names) {
  return std::all_of(names.begin(), names.end(), [&](const auto& name) { return records.contains(name); });
}

bool passesPositionConstraints(std::span<const std::string> differentSubword, std::span<const std::string> differentPosition, const Opportunity& opportunity, const std::map<std::string, DeclPolicyRecordedPosition>& records) {
  for (const auto& name : differentSubword) {
    const auto found = records.find(name);
    if (found != records.end() && found->second.subwordIndex == opportunity.subwordIndex) {
      return false;
    }
  }
  for (const auto& name : differentPosition) {
    const auto found = records.find(name);
    if (found != records.end() && found->second.subwordIndex == opportunity.subwordIndex && found->second.baseIndex == opportunity.firstBaseIndex) {
      return false;
    }
  }
  return true;
}

bool passesRecordConstraints(const SelectionPolicy& selection, const Opportunity& opportunity, const std::map<std::string, DeclPolicyRecordedPosition>& records) {
  const auto* choice = ruleSelection(selection, opportunity.rule);
  if (choice == nullptr) return false;
  if (!hasAllRecords(records, selection.requireRecorded) || !hasAllRecords(records, choice->requireRecorded)) return false;
  if (hasAnyRecord(records, selection.forbidRecorded) || hasAnyRecord(records, choice->forbidRecorded)) return false;
  return passesPositionConstraints(selection.differentSubword, selection.differentPosition, opportunity, records) && passesPositionConstraints(choice->differentSubword, choice->differentPosition, opportunity, records);
}

void recordOpportunity(const SelectionPolicy& selection, const Opportunity& opportunity, std::map<std::string, DeclPolicyRecordedPosition>& records) {
  const auto position = DeclPolicyRecordedPosition{opportunity.subwordIndex, opportunity.firstBaseIndex};
  if (!selection.record.empty()) records.insert_or_assign(selection.record, position);
  const auto* choice = ruleSelection(selection, opportunity.rule);
  if (choice != nullptr && !choice->record.empty()) records.insert_or_assign(choice->record, position);
}

std::vector<const Opportunity*> candidatesFor(const SelectionPolicy& selection, const WordRecognition& recognition, const std::map<std::string, DeclPolicyRecordedPosition>& records) {
  std::vector<const Opportunity*> candidates;
  if (!hasAllRecords(records, selection.requireRecorded) || hasAnyRecord(records, selection.forbidRecorded)) return candidates;
  for (const auto& opportunity : recognition.opportunities) {
    if (passesRecordConstraints(selection, opportunity, records)) candidates.push_back(&opportunity);
  }
  std::stable_sort(candidates.begin(), candidates.end(), [&](const auto* lhs, const auto* rhs) {
    if (lhs->subwordIndex != rhs->subwordIndex) return selection.lastSubwordFirst ? lhs->subwordIndex > rhs->subwordIndex : lhs->subwordIndex < rhs->subwordIndex;
    const auto* lhsChoice = ruleSelection(selection, lhs->rule);
    const auto* rhsChoice = ruleSelection(selection, rhs->rule);
    const auto lhsPriority = lhsChoice - selection.rules.data();
    const auto rhsPriority = rhsChoice - selection.rules.data();
    if (lhsPriority != rhsPriority) return lhsPriority < rhsPriority;
    if (lhsChoice->direction == MatchDirection::LogicalFirst) return lhs->firstBaseIndex < rhs->firstBaseIndex;
    return lhs->firstBaseIndex > rhs->firstBaseIndex;
  });

  std::vector<const Opportunity*> filtered;
  filtered.reserve(candidates.size());
  for (const auto* candidate : candidates) {
    const auto* choice = ruleSelection(selection, candidate->rule);
    const auto sameChoice = [&](const auto* accepted) { return accepted->subwordIndex == candidate->subwordIndex && accepted->rule == candidate->rule; };
    if (choice->multiplicity == MatchMultiplicity::SelectedOnly) {
      if (std::find_if(filtered.begin(), filtered.end(), sameChoice) != filtered.end()) continue;
    } else if (choice->multiplicity == MatchMultiplicity::AllNonOverlapping) {
      const auto overlaps = [&](const auto* accepted) { return sameChoice(accepted) && candidate->firstBaseIndex <= accepted->secondBaseIndex && accepted->firstBaseIndex <= candidate->secondBaseIndex; };
      if (std::find_if(filtered.begin(), filtered.end(), overlaps) != filtered.end()) continue;
    }
    filtered.push_back(candidate);
  }
  return filtered;
}

}  // namespace

bool DeclPolicyJustifier::applyStage(std::size_t wordCount, std::span<const FixedSlotGlyphInput> glyphs, std::span<const PolicyPhase> phases, DeclPolicyExecutionState& state, const FixedSlotActionCallback& applyAction, const FixedSlotRecognitionCallback& recognizeAcceptedWord) const {
  if (!catalog_.dfa) throw std::runtime_error("compiled justification catalog has no DFA");
  if (state.stopped) return true;
  if (state.currentGlyphs.empty()) state.currentGlyphs.assign(glyphs.begin(), glyphs.end());
  std::uint64_t predicateFacts = 0;
  for (const auto& rule : catalog_.rules)
    for (const auto& slot : rule.pattern) predicateFacts |= slot.allOf | slot.anyOf | slot.noneOf;
  std::vector<std::vector<FixedSlotGlyphInput>> glyphsByWord(wordCount);
  for (const auto& glyph : state.currentGlyphs) {
    if (glyph.wordIndex < 0 || glyph.wordIndex >= static_cast<int>(wordCount)) throw std::runtime_error("declarative-policy glyph has an invalid word index");
    glyphsByWord[glyph.wordIndex].push_back(glyph);
  }
  std::vector<WordRecognition> recognition(wordCount);
  for (int wordIndex = 0; wordIndex < static_cast<int>(wordCount); ++wordIndex) recognizeWord(wordIndex, std::move(glyphsByWord[wordIndex]), *catalog_.dfa, predicateFacts, recognition[wordIndex]);

  if (state.recordsByWord.empty()) state.recordsByWord.resize(wordCount);
  if (state.recordsByWord.size() != wordCount) throw std::runtime_error("declarative-policy execution state has the wrong word count");
  auto& recordsByWord = state.recordsByWord;

  const auto runOpportunity = [&](const SelectionPolicy& selection, int wordIndex, const Opportunity& opportunity) {
    auto& records = recordsByWord[wordIndex];
    if (!passesRecordConstraints(selection, opportunity, records)) return FixedSlotActionResult::Forbidden;
    const auto* action = actionFor(opportunity.rule, catalog_.actions);
    if (action == nullptr) return FixedSlotActionResult::NoChange;
    const auto& wordSlots = recognition[wordIndex].slots;
    const std::span<const FixedSlotSlot> context(wordSlots.data() + opportunity.subwordFirst, static_cast<std::size_t>(opportunity.subwordCount));
    const auto result = applyAction({
        .definition = action->definition,
        .anchorSlot = action->slot < 0 ? 0 : action->slot,
        .rule = opportunity.rule,
        .wordIndex = wordIndex,
        .subwordIndex = opportunity.subwordIndex,
        .matchSlots = context.subspan(static_cast<std::size_t>(opportunity.matchOffset), static_cast<std::size_t>(opportunity.matchCount)),
        .context = context,
        .matchOffset = opportunity.matchOffset,
    });
    if (result == FixedSlotActionResult::Positive) recordOpportunity(selection, opportunity, records);
    if (result == FixedSlotActionResult::Positive && recognizeAcceptedWord) {
      auto currentWord = recognizeAcceptedWord(wordIndex);
      recognizeWord(wordIndex, currentWord, *catalog_.dfa, predicateFacts, recognition[wordIndex]);
      std::erase_if(state.currentGlyphs, [&](const auto& glyph) { return glyph.wordIndex == wordIndex; });
      state.currentGlyphs.insert(state.currentGlyphs.end(), currentWord.begin(), currentWord.end());
    }
    return result;
  };

  const auto runFixedPhase = [&](const PolicyPhase& phase) {
    if (phase.selection >= catalog_.selections.size()) {
      throw std::runtime_error("compiled justification phase is invalid");
    }
    const auto& selection = catalog_.selections[phase.selection];
    for (int level = 0; level < phase.levels; ++level) {
      for (int wordIndex = 0; wordIndex < static_cast<int>(wordCount); ++wordIndex) {
        for (const auto* opportunity : candidatesFor(selection, recognition[wordIndex], recordsByWord[wordIndex])) {
          const auto result = runOpportunity(selection, wordIndex, *opportunity);
          if (result == FixedSlotActionResult::Overflow) return true;
          if (result == FixedSlotActionResult::Forbidden) continue;
          break;
        }
      }
    }
    return false;
  };

  for (const auto& phase : phases) {
    if (phase.allocator != PolicyPhase::Allocator::FixedSteps) throw std::runtime_error("candidate_pool phases require the line-wide candidate planner");
    if (runFixedPhase(phase)) {
      state.stopped = true;
      return true;
    }
  }
  return false;
}

void DeclPolicyJustifier::collectStageCandidates(std::size_t wordCount, std::span<const FixedSlotGlyphInput> glyphs, std::span<const PolicyPhase> phases, const DeclPolicyExecutionState& state, const FixedSlotCandidateCallback& collectCandidate) const {
  if (!catalog_.dfa) throw std::runtime_error("compiled justification catalog has no DFA");
  if (!collectCandidate || state.stopped) return;
  if (!state.recordsByWord.empty() && state.recordsByWord.size() != wordCount) throw std::runtime_error("declarative-policy execution state has the wrong word count");

  std::uint64_t predicateFacts = 0;
  for (const auto& rule : catalog_.rules)
    for (const auto& slot : rule.pattern) predicateFacts |= slot.allOf | slot.anyOf | slot.noneOf;

  const auto currentGlyphs = state.currentGlyphs.empty() ? glyphs : std::span<const FixedSlotGlyphInput>(state.currentGlyphs);
  std::vector<std::vector<FixedSlotGlyphInput>> glyphsByWord(wordCount);
  for (const auto& glyph : currentGlyphs) {
    if (glyph.wordIndex < 0 || glyph.wordIndex >= static_cast<int>(wordCount)) throw std::runtime_error("declarative-policy glyph has an invalid word index");
    glyphsByWord[glyph.wordIndex].push_back(glyph);
  }

  std::vector<WordRecognition> recognition(wordCount);
  for (int wordIndex = 0; wordIndex < static_cast<int>(wordCount); ++wordIndex) recognizeWord(wordIndex, std::move(glyphsByWord[wordIndex]), *catalog_.dfa, predicateFacts, recognition[wordIndex]);
  const std::map<std::string, DeclPolicyRecordedPosition> noRecords;

  for (std::size_t phaseIndex = 0; phaseIndex < phases.size(); ++phaseIndex) {
    const auto& phase = phases[phaseIndex];
    if (phase.selection >= catalog_.selections.size()) throw std::runtime_error("compiled justification phase is invalid");
    const auto& selection = catalog_.selections[phase.selection];
    std::map<JustificationRuleId, std::size_t> occurrences;
    for (int wordIndex = 0; wordIndex < static_cast<int>(wordCount); ++wordIndex) {
      const auto& records = state.recordsByWord.empty() ? noRecords : state.recordsByWord[wordIndex];
      for (const auto* opportunity : candidatesFor(selection, recognition[wordIndex], records)) {
        if (!passesRecordConstraints(selection, *opportunity, records)) continue;
        const auto* action = actionFor(opportunity->rule, catalog_.actions);
        if (action == nullptr) continue;
        const auto* choice = ruleSelection(selection, opportunity->rule);
        const auto occurrence = occurrences[opportunity->rule]++;
        FixedSlotCandidateScore score;
        score.occurrence = static_cast<int>(occurrence);
        score.baseWeight = phase.weight * choice->weight;
        score.occurrenceAdjustment = score.baseWeight * (std::pow(choice->decay, static_cast<double>(occurrence)) - 1);
        score.subwordLengthAdjustment = phase.longerSubword * std::max(0, opportunity->subwordCount - 1);
        if (opportunity->matchCount == 2 && opportunity->subwordCount > 1) {
          const auto connection = opportunity->matchOffset + 1;
          const auto centrality = 1 - std::abs(2.0 * connection - opportunity->subwordCount) / opportunity->subwordCount;
          score.centralityAdjustment = phase.centralConnection * centrality;
        }
        const auto logicalPosition = wordCount > 1 ? 2.0 * wordIndex / (wordCount - 1) - 1 : 0;
        score.wordPositionAdjustment = phase.wordPosition * logicalPosition;
        score.total = std::max(0.000001, score.baseWeight + score.occurrenceAdjustment + score.subwordLengthAdjustment + score.centralityAdjustment + score.wordPositionAdjustment);
        const auto& wordSlots = recognition[wordIndex].slots;
        const std::span<const FixedSlotSlot> context(wordSlots.data() + opportunity->subwordFirst, static_cast<std::size_t>(opportunity->subwordCount));
        collectCandidate({
                             .definition = action->definition,
                             .anchorSlot = action->slot < 0 ? 0 : action->slot,
                             .rule = opportunity->rule,
                             .wordIndex = wordIndex,
                             .subwordIndex = opportunity->subwordIndex,
                             .matchSlots = context.subspan(static_cast<std::size_t>(opportunity->matchOffset), static_cast<std::size_t>(opportunity->matchCount)),
                             .context = context,
                             .matchOffset = opportunity->matchOffset,
                         },
                         phaseIndex, score);
      }
    }
  }
}

bool DeclPolicyJustifier::apply(std::size_t wordCount, std::span<const FixedSlotGlyphInput> glyphs, const FixedSlotActionCallback& applyAction, const FixedSlotRecognitionCallback& recognizeAcceptedWord) const {
  DeclPolicyExecutionState state;
  const auto& policy = catalog_.stretchPolicy(stretchPolicy_);
  for (const auto& step : policy.steps) {
    if (step.operation != LineStepOp::Stage) continue;
    if (applyStage(wordCount, glyphs, step.phases, state, applyAction, recognizeAcceptedWord)) return true;
  }
  return false;
}

}  // namespace digitalkhatt::justify
