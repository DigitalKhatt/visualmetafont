#include <digitalkhatt/justify/declpolicy/FixedSlotDfa.h>
#include <digitalkhatt/justify/declpolicy/JustificationActionEvaluator.h>
#include <digitalkhatt/justify/declpolicy/JustificationCandidate.h>
#include <digitalkhatt/justify/declpolicy/JustificationStagingBackend.h>
#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>
#include <digitalkhatt/justify/declpolicy/DependentMarkWidth.h>
#include "../../src/otlayout/Layout/GlyphInstanceStore.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <map>
#include <utility>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {

// Stands in for the cvXX feature map, so the evaluator can be exercised with
// no font, no HarfBuzz and no line to measure.
class FakeBackend final : public digitalkhatt::justify::JustificationStagingBackend {
 public:
  using Attribute = digitalkhatt::justify::JustAttributeId;
  std::map<int, std::vector<std::pair<Attribute, double>>> committed;
  std::map<int, std::vector<std::pair<Attribute, double>>> staged;
  int commits = 0;
  int aborts = 0;
  digitalkhatt::justify::JustificationLineMetrics metrics;
  mutable int metricReads = 0;
  digitalkhatt::justify::JustificationLineMetrics lineMetrics() const override {
    ++metricReads;
    return metrics;
  }
  // Glyph model, so a substitution has something to act on.
  const digitalkhatt::justify::CompiledJustificationCatalog* catalog = nullptr;
  std::map<int, std::string> committedGlyphs;
  std::map<int, std::string> glyphs;
  std::map<int, std::uint32_t> committedGlyphIds;
  std::map<int, std::uint32_t> glyphIds;
  std::map<std::string, std::uint32_t> lookupResults;
  std::vector<std::pair<int, std::string>> committedLookups;
  std::vector<std::pair<int, std::string>> lookups;

  void beginTransaction() override {
    staged = committed;
    glyphs = committedGlyphs;
    glyphIds = committedGlyphIds;
    lookups = committedLookups;
  }
  void abortTransaction() override {
    ++aborts;
    staged.clear();
    glyphs.clear();
    glyphIds.clear();
    lookups.clear();
  }
  digitalkhatt::justify::FixedSlotActionResult commitTransaction(int) override {
    ++commits;
    committed = staged;
    committedGlyphs = glyphs;
    committedGlyphIds = glyphIds;
    committedLookups = lookups;
    return digitalkhatt::justify::FixedSlotActionResult::Positive;
  }
  bool present(int site, Attribute attribute) const override {
    return find(site, attribute) != nullptr;
  }
  double read(int site, Attribute attribute) const override {
    const auto* found = find(site, attribute);
    return found == nullptr ? 0 : found->second;
  }
  void update(int site, Attribute attribute, double value) override {
    for (auto& entry : staged[site]) {
      if (entry.first == attribute) {
        entry.second = value;
        return;
      }
    }
    staged[site].push_back({attribute, value});
  }
  void clear(int site) override { staged[site].clear(); }
  void applyLookup(int site, std::string_view lookup) override {
    lookups.emplace_back(site, lookup);
    if (const auto found = lookupResults.find(std::string(lookup));
        found != lookupResults.end()) {
      glyphIds[site] = found->second;
    }
  }
  std::optional<std::uint32_t> glyph(int site) const override {
    const auto found = glyphIds.find(site);
    return found == glyphIds.end()
               ? std::nullopt
               : std::optional<std::uint32_t>(found->second);
  }
  std::optional<double> measureStagedWidthDelta(int) override {
    ++measurements;
    double width = 0;
    for (const auto& [site, values] : staged) {
      (void)site;
      for (const auto& [attribute, value] : values) {
        (void)attribute;
        width += value;
      }
    }
    return width;
  }
  int measurements = 0;
  void vary(int site, Attribute attribute, double endpoint) override {
    varied.emplace_back(site, attribute, endpoint);
  }
  std::vector<std::tuple<int, Attribute, double>> varied;
  Attribute leftTatweel = 0;
  // The line puts a fatha immediately after every base.
  int attachment(int anchor, digitalkhatt::justify::JustAttachmentId) const override {
    return anchor + 1;
  }

 private:
  const std::pair<Attribute, double>* find(int site, Attribute attribute) const {
    const auto entry = staged.find(site);
    if (entry == staged.end()) return nullptr;
    for (const auto& value : entry->second) {
      if (value.first == attribute) return &value;
    }
    return nullptr;
  }
};

bool expect(bool condition, const char* message) {
  if (!condition) std::cerr << "FAILED: " << message << '\n';
  return condition;
}

void setSingleStagePolicy(digitalkhatt::justify::JustificationDfaSource& source, std::vector<digitalkhatt::justify::DfaPhaseSource> phases, std::string policyName = "standard") {
  source.stages = {{.name = "Main", .phases = std::move(phases)}};
  source.stretchPolicies = {{.name = std::move(policyName), .steps = {{.operation = "stage", .stage = "Main"}}}};
  source.shrinkPolicies = {{.name = "standard", .steps = {{.operation = "scale"}}}};
}

}  // namespace

int main() {
  using namespace digitalkhatt::justify;
  constexpr std::uint64_t a = 1 << 0;
  constexpr std::uint64_t b = 1 << 1;
  constexpr std::uint64_t c = 1 << 2;

  FixedSlotDfa dfa({
      {1, "ab", {{.anyOf = a}, {.anyOf = b}}},
      {2, "a-or-c,b", {{.anyOf = a | c}, {.anyOf = b}}},
      {3, "c", {{.anyOf = c}}},
  });
  const std::vector<std::uint64_t> input{a, b, c, a, 0, b};
  const auto matches = dfa.match(input);
  bool ok = true;
  {
    GlyphInstanceStore store;
    GlyphInstanceState baseline;
    baseline.parameters.lefttatweel = 0.5;
    baseline.parameters.set(9, 3);
    const auto baselineId = store.intern(baseline);
    auto justified = baseline;
    justified.parameters.lefttatweel = 1.25;
    const auto withoutBaseline = store.intern(justified);
    justified.justificationBaseline = baselineId;
    const auto justifiedId = store.intern(justified);
    ok &= expect(justifiedId != withoutBaseline && store.intern(justified) == justifiedId, "mark justification provenance participates in immutable instance identity");
    const auto saved = store.get(*store.get(justifiedId).justificationBaseline);
    ok &= expect(saved.parameters.lefttatweel == 0.5 && saved.parameters.value(9) == 3, "justification baselines reuse immutable records and retain dynamic axes");
    justified.justificationBaseline = 0;
    ok &= expect(store.get(store.intern(justified)).justificationBaseline == 0, "neutral baseline ID zero is present, not missing provenance");
    justified.justificationBaseline = 999999;
    bool rejected = false;
    try { store.intern(justified); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "mark baseline references must already exist in the instance store");
  }
  {
    digitalkhatt::GlyphAxisRegistry axes;
    const auto extra = axes.add("penAngle", 17);
    ok &= expect(extra == 6 && axes.add("angleAlias", 17) == extra, "dynamic axes use dense slots and support aliases");
    digitalkhatt::GlyphParameters parameters;
    parameters.set(extra, 3);
    auto more = parameters;
    more.set(40, 9);
    ok &= expect(parameters.value(40) == 0 && more.value(40) == 9, "dynamic parameter values are independent");
    more.set(40, 0);
    ok &= expect(more == parameters, "trailing zero coordinates are canonical");
    bool rejected = false;
    try {
      parameters.set(digitalkhatt::NoGlyphAxis, 0);
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    ok &= expect(rejected, "unchanged parameter update must reject invalid axis");
    rejected = false;
    try {
      parameters.set(extra, std::numeric_limits<double>::infinity());
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    ok &= expect(rejected, "additional parameters must be finite");
    digitalkhatt::GlyphLayoutInfo glyph{};
    glyph.parameters.set(extra, 7);
    ok &= expect(glyph.parameters.value(extra) == 7, "layout glyph stores dynamic parameters directly");
  }
  ok &= expect(matches.size() == 3, "fixed patterns match without a Kleene-star bridge");
  ok &= expect(matches[0] == FixedSlotMatch{1, 0, 1}, "specific AB rule matches");
  ok &= expect(matches[1] == FixedSlotMatch{2, 0, 1}, "overlapping predicate is determinized");
  ok &= expect(matches[2] == FixedSlotMatch{3, 2, 2}, "one-slot rule matches");
  ok &= expect(dfa.maximumRuleLength() == 2, "maximum scan length is bounded");

  // A rule set may name far more facts than any single position distinguishes.
  // Twenty predicates, ten of which the start state branches on.  Enumerating
  // that position would cost 2^10 entries for combinations of independent
  // facts, most of which no glyph can produce, so it memoizes instead.
  {
    std::vector<FixedSlotRule> wide;
    for (std::uint16_t rule = 0; rule < 10; ++rule) {
      wide.push_back({static_cast<std::uint16_t>(rule + 1), "wide", {{.anyOf = std::uint64_t{1} << rule}, {.anyOf = std::uint64_t{1} << (rule + 10)}}});
    }
    const FixedSlotDfa wideDfa(wide);
    ok &= expect(wideDfa.predicateCount() == 20 &&
                     wideDfa.maximumBranchingFactor() == 10,
                 "a rule set may use more predicates than any state branches on");
    ok &= expect(wideDfa.lazyStateCount() == 1 && wideDfa.transitionCount() == 0,
                 "a position too wide to enumerate holds no transitions yet");
    const std::vector<std::uint64_t> wideInput{std::uint64_t{1} << 3,
                                               std::uint64_t{1} << 13};
    const auto wideMatch = wideDfa.match(wideInput);
    ok &= expect(wideMatch.size() == 1 && wideMatch[0] == FixedSlotMatch{4, 0, 1},
                 "a twenty-predicate rule set still matches exactly");
    ok &= expect(wideDfa.transitionCount() < 32,
                 "only the combinations the text produced are materialized");
    ok &= expect(wideDfa.match(wideInput) == wideMatch,
                 "a memoized transition answers the same on a second pass");
  }

  // The two cases above only reach the memoizing path from the start state,
  // with predicates no glyph can satisfy at once.  This one drives it against
  // a brute-force reference: rules wide enough at BOTH positions that a lazy
  // state's successor is itself lazy, predicates that deliberately overlap so
  // several fire on one glyph, and inputs that revisit combinations so the
  // memo is read back as well as filled.
  {
    // Every slot is `anyOf` over two adjacent bits, so consecutive predicates
    // share a bit and a glyph routinely satisfies several at once.
    const auto slotFor = [](std::uint16_t index) {
      return GlyphPredicate{.anyOf = (std::uint64_t{3} << index)};
    };
    std::vector<FixedSlotRule> overlapping;
    for (std::uint16_t rule = 0; rule < 12; ++rule) {
      overlapping.push_back({static_cast<std::uint16_t>(rule + 1), "overlap", {slotFor(rule), slotFor(static_cast<std::uint16_t>(rule + 12))}});
    }
    const FixedSlotDfa lazyDfa(overlapping);
    ok &= expect(lazyDfa.maximumBranchingFactor() == 12 &&
                     lazyDfa.lazyStateCount() == 1,
                 "twelve overlapping predicates at one position memoize");

    // What the rules mean, stated directly: a rule matches at `first` when
    // every slot accepts the glyph under it.
    const auto reference = [&](const std::vector<std::uint64_t>& input) {
      std::vector<FixedSlotMatch> expected;
      for (std::size_t first = 0; first < input.size(); ++first) {
        for (const auto& rule : overlapping) {
          if (first + rule.pattern.size() > input.size()) continue;
          bool all = true;
          for (std::size_t slot = 0; slot < rule.pattern.size(); ++slot) {
            if (!rule.pattern[slot].matches(input[first + slot])) all = false;
          }
          if (all) {
            expected.push_back({rule.id, first, first + rule.pattern.size() - 1});
          }
        }
      }
      std::sort(expected.begin(), expected.end());
      return expected;
    };

    std::uint64_t seed = 0x9e3779b97f4a7c15ull;
    const auto nextRandom = [&seed] {
      seed ^= seed << 13;
      seed ^= seed >> 7;
      seed ^= seed << 17;
      return seed;
    };
    bool agreed = true;
    bool sawMatch = false;
    bool sawLazySuccessor = false;
    for (int trial = 0; trial < 400 && agreed; ++trial) {
      std::vector<std::uint64_t> input(2 + (nextRandom() % 6));
      for (auto& facts : input) {
        // A narrow mask keeps real matches frequent; the low 24 bits are the
        // ones any predicate looks at.
        facts = nextRandom() & ((std::uint64_t{1} << 24) - 1);
        if ((nextRandom() & 3) == 0) facts &= facts - 1;  // sometimes sparser
      }
      const auto produced = lazyDfa.match(input);
      const auto expected = reference(input);
      if (produced != expected) agreed = false;
      if (!produced.empty()) sawMatch = true;
      // A second slot reached through the lazy start state is lazy too.
      if (lazyDfa.lazyStateCount() > 1) sawLazySuccessor = true;
    }
    ok &= expect(agreed,
                 "the memoizing path agrees with a brute-force matcher on 400 inputs");
    ok &= expect(sawMatch, "the randomized inputs actually produce matches");
    ok &= expect(sawLazySuccessor,
                 "a state discovered through the memoizing path memoizes in turn");
    // Replaying the same inputs must read the memo back, not rebuild it.
    const std::size_t settled = lazyDfa.transitionCount();
    const std::size_t settledStates = lazyDfa.stateCount();
    seed = 0x9e3779b97f4a7c15ull;
    for (int trial = 0; trial < 400; ++trial) {
      std::vector<std::uint64_t> input(2 + (nextRandom() % 6));
      for (auto& facts : input) {
        facts = nextRandom() & ((std::uint64_t{1} << 24) - 1);
        if ((nextRandom() & 3) == 0) facts &= facts - 1;
      }
      if (lazyDfa.match(input) != reference(input)) agreed = false;
    }
    ok &= expect(agreed && lazyDfa.transitionCount() == settled &&
                     lazyDfa.stateCount() == settledStates,
                 "a replay is answered from the memo without growing the table");
    // Worth stating plainly: memoizing is proportional to what the input
    // produces, and random facts over overlapping predicates produce a great
    // deal -- this settles at roughly 3800 states and 57,000 entries.  That is
    // still two orders of magnitude below enumerating 2^12 for each of them,
    // but it is not free, and a rule set this ambiguous fed genuinely random
    // text is the shape that would grow without bound.
    ok &= expect(settled < settledStates * (std::size_t{1} << 12),
                 "memoizing stays far below enumerating every state's row");
  }

  // Seventeen predicates at one position was previously rejected outright.
  // Enumerating it would cost 2^17 entries, so the state memoizes and the
  // table ends up holding only what the text actually asked for.
  {
    std::vector<FixedSlotRule> branching;
    for (std::uint16_t rule = 0; rule < 17; ++rule) {
      branching.push_back({static_cast<std::uint16_t>(rule + 1), "branch", {{.anyOf = std::uint64_t{1} << rule}}});
    }
    const FixedSlotDfa manyWays(branching);
    ok &= expect(manyWays.maximumBranchingFactor() == 17 &&
                     manyWays.lazyStateCount() == 1,
                 "a state branching on seventeen predicates is accepted");
    const std::vector<std::uint64_t> branchInput{std::uint64_t{1} << 16,
                                                 std::uint64_t{1} << 0};
    const auto branchMatch = manyWays.match(branchInput);
    ok &= expect(branchMatch.size() == 2 &&
                     branchMatch[0] == FixedSlotMatch{1, 1, 1} &&
                     branchMatch[1] == FixedSlotMatch{17, 0, 0},
                 "both ends of the seventeen-way branch are reachable");
    ok &= expect(manyWays.transitionCount() < 8,
                 "seventeen predicates cost a handful of entries, not 2^17");
  }

  // The test supplies the complete declarative policy. There is deliberately no C++
  // default catalog for it to match or fall back to.
  JustificationDfaSource source{
      .name = "test.fixed_slot",
      .facts = {{"CustomTerminal", "terminal", {u'ب'}}},
      .rules = {{"ArbitraryRuleName", {"CustomTerminal"}}},
      .actions = {{"ArbitraryRuleName", "IncrementAlternate"}},
      .actionDefinitions = {{.name = "IncrementAlternate",
                             .effects = {{.kind = "update",
                                          .attribute = "cv01",
                                          .value = {{.op = "literal",
                                                     .literal = 1}}}}}},
      .selections = {{.name = "ArbitrarySelectionName",
                      .choices = {{"ArbitraryRuleName", "last",
                                   "selected_only"}},
                      .traversal = "last_first",
                      .record = "ArbitraryRecordName"}},
  };
  setSingleStagePolicy(source, {{"ArbitrarySelectionName", 1}});
  const auto compiled = compileJustificationCatalog(source);
  {
    auto authored = source;
    authored.rules.clear();
    authored.actions.clear();
    authored.opportunities = {{.name = "ArbitraryRuleName", .pattern = {{.fact = "CustomTerminal"}}, .action = "IncrementAlternate"}};
    const auto named = compileJustificationCatalog(authored);
    ok &= expect(named.rules == compiled.rules && named.actions == compiled.actions, "named opportunities lower to the same DFA and action dispatch");
    authored.opportunities[0].pattern = {{.inlineFact = DfaFactSource{.form = "terminal", .sourceCharacters = {u'ب'}}}};
    const auto inlineFact = compileJustificationCatalog(authored);
    ok &= expect(inlineFact.facts == named.facts && inlineFact.rules == named.rules, "an inline fact identical to a named fact reuses its bit");
    authored.opportunities[0].action.clear();
    authored.opportunities[0].effects = source.actionDefinitions[0].effects;
    const auto inlineBody = compileJustificationCatalog(authored);
    ok &= expect(inlineBody.actionDefinitions.at(inlineBody.actions[0].definition).effects == compiled.actionDefinitions[0].effects, "inline opportunity bodies use the same action evaluator");
    authored.opportunities[0].pattern.push_back({.inlineFact = DfaFactSource{.form = "medi", .position = "first"}});
    const auto mixed = compileJustificationCatalog(authored);
    ok &= expect(mixed.rules[0].pattern.size() == 2 && mixed.facts.size() == 2, "named and inline slot facts can be mixed in a fixed-slot opportunity");
    auto sharedPattern = authored;
    sharedPattern.patterns = {{.name = "Shared", .slots = sharedPattern.opportunities[0].pattern}};
    sharedPattern.opportunities[0].pattern.clear();
    sharedPattern.opportunities[0].patternName = "Shared";
    const auto reusedPattern = compileJustificationCatalog(sharedPattern);
    ok &= expect(reusedPattern.rules == mixed.rules && reusedPattern.facts == mixed.facts && reusedPattern.actions == mixed.actions, "named patterns expand at compilation without changing rule IDs, fact bits or dispatch");
    auto secondOpportunity = sharedPattern.opportunities[0];
    secondOpportunity.name = "SecondOpportunity";
    sharedPattern.opportunities.push_back(secondOpportunity);
    const auto independent = compileJustificationCatalog(sharedPattern);
    ok &= expect(independent.facts.size() == mixed.facts.size() && independent.rules[0].pattern == independent.rules[1].pattern && independent.rules[0].id != independent.rules[1].id && independent.actions.size() == 2, "opportunities sharing a pattern retain independent identities while reusing inline fact bits");
    for (int invalid = 0; invalid != 9; ++invalid) {
      auto bad = sharedPattern;
      if (invalid == 0) bad.patterns.push_back(bad.patterns[0]);
      if (invalid == 1) bad.patterns[0].name.clear();
      if (invalid == 2) bad.patterns[0].slots.clear();
      if (invalid == 3) bad.opportunities[0].patternName = "Missing";
      if (invalid == 4) bad.opportunities[0].pattern = {{.fact = "CustomTerminal"}};
      if (invalid == 5) bad.patterns.push_back({.name = "UnusedInvalid", .slots = {{.fact = "MissingFact"}}});
      if (invalid == 6) bad.patterns.push_back({.name = "UnusedEmpty", .slots = {{.inlineFact = DfaFactSource{}}}});
      if (invalid == 7) bad.patterns.push_back({.name = "UnusedMixed", .slots = {{.fact = "CustomTerminal", .inlineFact = DfaFactSource{.form = "medi"}}}});
      if (invalid == 8) bad.patterns.push_back({.name = "UnusedBadForm", .slots = {{.inlineFact = DfaFactSource{.form = "typo"}}}});
      bool rejected = false;
      try { (void)compileJustificationCatalog(bad); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "named patterns reject duplicate/empty definitions, unknown references, mixed matches and invalid unused definitions");
    }
    for (int invalid = 0; invalid != 6; ++invalid) {
      auto bad = authored;
      if (invalid == 0) bad.opportunities[0].action = "IncrementAlternate";
      if (invalid == 1) bad.opportunities[0].pattern[0].fact = "CustomTerminal";
      if (invalid == 2) bad.opportunities[0].pattern[0].inlineFact = DfaFactSource{};
      if (invalid == 3) bad.opportunities[0].pattern.clear();
      if (invalid == 4) bad.opportunities.push_back(bad.opportunities[0]);
      if (invalid == 5) bad.rules = source.rules;
      bool rejected = false;
      try { (void)compileJustificationCatalog(bad); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "opportunities reject ambiguous bodies/slots, empty inline facts/patterns, duplicate names and mixed low-level IR");
    }
    auto interned = authored;
    interned.opportunities[0].pattern.resize(1);
    for (int index = 1; index != 70; ++index) {
      auto opportunity = interned.opportunities[0];
      opportunity.name = "Inline" + std::to_string(index);
      interned.opportunities.push_back(std::move(opportunity));
    }
    const auto reused = compileJustificationCatalog(interned);
    ok &= expect(reused.facts.size() == 1 && reused.rules.size() == 70, "repeated inline facts do not exhaust the 64-bit fact budget");
  }
  // Opportunity-owned ranges expand at the argument assignment, retaining
  // exactly the old update/vary sequence and therefore its quantization.
  {
    const DfaExprSource one{{.op = "literal", .literal = 1}}, two{{.op = "literal", .literal = 2}}, five{{.op = "literal", .literal = 5}};
    const DfaExprSource half{{.op = "literal", .literal = 0.5}}, twoHalf{{.op = "literal", .literal = 2.5}};
    auto legacy = source;
    legacy.actionDefinitions[0].effects = {
        {.kind = "update", .attribute = "third", .value = one},
        {.kind = "vary", .attribute = "third", .value = two, .maximum = five},
        {.kind = "update", .attribute = "fifth", .value = {{.op = "attribute", .name = "third"}}},
        {.kind = "update", .attribute = "fourth", .value = half},
        {.kind = "vary", .attribute = "fourth", .value = one, .maximum = twoHalf},
    };
    auto authored = legacy;
    authored.rules.clear();
    authored.actions.clear();
    authored.actionDefinitions[0].parameters = {"outgoing", "incoming"};
    authored.actionDefinitions[0].effects = {
        {.kind = "update", .attribute = "third", .value = {{.op = "argument", .name = "outgoing"}}},
        legacy.actionDefinitions[0].effects[2],
        {.kind = "update", .attribute = "fourth", .value = {{.op = "argument", .name = "incoming"}}},
    };
    authored.opportunities = {{.name = "ArbitraryRuleName", .pattern = {{.fact = "CustomTerminal"}}, .action = "IncrementAlternate", .ranges = {{"left", {one, two, five}}, {"right", {half, one, twoHalf}}}, .arguments = {{{.op = "argument", .name = "left"}}, {{.op = "argument", .name = "right"}}}}};
    const auto expected = compileJustificationCatalog(legacy);
    const auto actual = compileJustificationCatalog(authored);
    ok &= expect(actual.actionDefinitions == expected.actionDefinitions && actual.attributes == expected.attributes && actual.actions == expected.actions, "range arguments lower to identical ordered effects, attributes and dispatch");
    FakeBackend backend;
    const std::array slots{FixedSlotSlot{.indexInLine = 10}};
    const auto candidate = collectJustificationCandidate(actual, actual.actionDefinitions[0], {.definition = 0, .rule = 1, .wordIndex = 0, .context = slots}, backend);
    ok &= expect(candidate && candidate->parameters.size() == 3 && candidate->parameters[0].minimumValue == 1 && candidate->parameters[0].maximumValue == 2 && candidate->parameters[0].continuationMaximum == 5 && candidate->parameters[1].minimumValue == 1 && candidate->parameters[2].minimumValue == 0.5, "later preparation observes the range minimum, not the preferred endpoint");

    auto shared = authored;
    auto another = shared.opportunities[0];
    another.name = "Second";
    shared.opportunities.push_back(another);
    auto reused = compileJustificationCatalog(shared);
    ok &= expect(reused.actionDefinitions.size() == 1 && reused.actions[0].definition == reused.actions[1].definition, "identical range calls share one compiled action");
    shared.opportunities[1].ranges[0].bounds.maximum = {{.op = "literal", .literal = 9}};
    reused = compileJustificationCatalog(shared);
    ok &= expect(reused.actionDefinitions.size() == 2 && reused.actions[0].definition != reused.actions[1].definition && reused.actionDefinitions.at(reused.actions[0].definition).effects[1].maximum != reused.actionDefinitions.at(reused.actions[1].definition).effects[1].maximum, "one shared action supports opportunity-specific bounds without mutating other calls");

    auto scalar = authored;
    scalar.opportunities[0].ranges.pop_back();
    scalar.opportunities[0].arguments[1] = two;
    const auto scalarCatalog = compileJustificationCatalog(scalar);
    ok &= expect(scalarCatalog.actionDefinitions[0].effects.size() == 4 && scalarCatalog.actionDefinitions[0].effects.back().kind == JustEffectKind::Update && scalarCatalog.actionDefinitions[0].effects.back().value[0].literal == 2, "scalar action arguments remain ordinary updates rather than ranges");
    auto nested = authored;
    nested.actionDefinitions[0].effects = {{.kind = "when", .condition = one, .nested = nested.actionDefinitions[0].effects}};
    const auto nestedCatalog = compileJustificationCatalog(nested);
    ok &= expect(nestedCatalog.actionDefinitions[0].effects[0].nested == actual.actionDefinitions[0].effects, "conditional range assignments preserve nested preparation order");
    auto noMinimum = authored;
    noMinimum.opportunities[0].ranges[0].bounds.minimum.clear();
    const auto currentMinimum = compileJustificationCatalog(noMinimum);
    ok &= expect(currentMinimum.actionDefinitions[0].effects.front().kind == JustEffectKind::Vary, "omitted range minima keep the staged value without inserting an update");

    auto derived = authored;
    derived.actionDefinitions[0].parameters[1] = "scale";
    derived.actionDefinitions[0].effects.back().value = {{.op = "argument", .name = "scale"}, {.op = "argument", .name = "outgoing"}, {.op = "mul"}};
    derived.opportunities[0].ranges.pop_back();
    derived.opportunities[0].arguments[1] = half;
    const auto derivedCatalog = compileJustificationCatalog(derived);
    auto dependent = collectJustificationCandidate(derivedCatalog, derivedCatalog.actionDefinitions[0], {.definition = 0, .rule = 1, .wordIndex = 0, .context = slots}, backend);
    ok &= expect(dependent && dependent->parameters.size() == 3 && dependent->parameters[2].driverSite == 10 && dependent->parameters[2].driverScale == 0.5 && dependent->parameters[2].minimumValue == 0.5 && dependent->parameters[2].maximumValue == 1 && dependent->parameters[2].continuationMaximum == 2.5, "derived outputs retain one independent range and its scaled endpoints");
    if (dependent) {
      const auto outgoing = justificationCandidateParameterValue(*dependent, 0, 0.1, 16);
      const auto incoming = justificationCandidateParameterValue(*dependent, 2, 0.1, 16);
      ok &= expect(outgoing == 1.0625 && incoming == 0.53125 && incoming == outgoing / 2, "quantize the driver once, then calculate the exact half without independent rounding");
      ok &= expect(justificationCandidateParameterValue(*dependent, 2, 0.1, 0) == justificationCandidateParameterValue(*dependent, 0, 0.1, 0) / 2, "unquantized allocation retains the same dependency");
      ok &= expect(limitJustificationCandidateContinuation(*dependent, {StageContinuation::Mode::To, 50, true}) && dependent->parameters[0].maximumValue == 3 && dependent->parameters[2].maximumValue == 1.5, "percentage continuation limits only the independent range and follows it on dependent outputs");
      dependent->parameters[0].initialValue = dependent->parameters[0].minimumValue = outgoing;
      dependent->parameters[2].initialValue = dependent->parameters[2].minimumValue = incoming;
      ok &= expect(limitJustificationCandidateContinuation(*dependent, {StageContinuation::Mode::By, 1}) && dependent->parameters[0].maximumValue == 2.0625 && dependent->parameters[2].maximumValue == 1.03125 && justificationCandidateParameterValue(*dependent, 2, 0.5, 16) == justificationCandidateParameterValue(*dependent, 0, 0.5, 16) / 2, "continued allocation retains the exact dependency after accepted fractional values");
    }
    auto identity = derived;
    identity.actionDefinitions[0].effects.back().value = {{.op = "argument", .name = "outgoing"}};
    const auto identityCatalog = compileJustificationCatalog(identity);
    const auto identical = collectJustificationCandidate(identityCatalog, identityCatalog.actionDefinitions[0], {.wordIndex = 0, .context = slots}, backend);
    ok &= expect(identical && identical->parameters[2].driverScale == 1 && justificationCandidateParameterValue(*identical, 2, 0.1, 16) == justificationCandidateParameterValue(*identical, 0, 0.1, 16), "a repeated direct range assignment is a dependent identity output");
    for (int invalid = 0; invalid < 5; ++invalid) {
      auto bad = derived;
      if (invalid == 0) bad.actionDefinitions[0].effects.back().value.back().op = "add";
      if (invalid == 1) bad.actionDefinitions[0].effects.back().value = {{.op = "argument", .name = "outgoing"}, {.op = "argument", .name = "outgoing"}, {.op = "mul"}};
      if (invalid == 2) bad.actionDefinitions[0].effects.push_back({.kind = "clear"});
      if (invalid == 3) bad.actionDefinitions[0].effects.back().attribute = "cv01";
      if (invalid == 4) bad.actionDefinitions[0].effects.push_back({.kind = "update", .attribute = "third", .value = one});
      bool rejected = false;
      try { (void)compileJustificationCatalog(bad); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "derived ranges reject nonlinear/multi-range expressions, non-native outputs and overwriting the driver");
    }

    for (int invalid = 0; invalid != 12; ++invalid) {
      auto bad = authored;
      if (invalid == 0) bad.actionDefinitions[0].parameters[1] = "outgoing";
      if (invalid == 1) bad.opportunities[0].arguments.pop_back();
      if (invalid == 2) bad.opportunities[0].arguments[0][0].name = "missing";
      if (invalid == 3) bad.opportunities[0].ranges.push_back(bad.opportunities[0].ranges[0]);
      if (invalid == 4) bad.opportunities[0].ranges[0].bounds.preferred.clear();
      if (invalid == 5) bad.opportunities[0].action.clear();
      if (invalid == 6) bad.opportunities[0].ranges.push_back({"unused", {one, two, five}});
      if (invalid == 7) bad.actionDefinitions[0].effects[0].value[0].name = "missing";
      if (invalid == 8) bad.actionDefinitions[0].effects[0].value = {{.op = "argument", .name = "outgoing"}, {.op = "literal", .literal = 0.5}, {.op = "mul"}};
      if (invalid == 9) bad.opportunities[0].arguments[0] = {{.op = "argument", .name = "left"}, {.op = "literal", .literal = 0.5}, {.op = "mul"}};
      if (invalid == 10) bad.opportunities[0].effects = legacy.actionDefinitions[0].effects;
      if (invalid == 11) bad.actionDefinitions[0].effects[0].attribute = "cv01";
      bool rejected = false;
      try { (void)compileJustificationCatalog(bad); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "range calls reject invalid arguments, ambiguous bodies, unused ranges, non-native outputs and unsupported derived ranges");
    }
  }
  const std::array tableMatchFacts{glyphFacts(
      {.glyphName = "behshape.fina", .sourceCharacter = u'ب'},
      compiled.facts)};
  const std::array tableMissFacts{glyphFacts(
      {.glyphName = "behshape.fina", .sourceCharacter = u'ت'},
      compiled.facts)};
  const auto tableMatch = compiled.dfa->match(tableMatchFacts);
  const auto tableMiss = compiled.dfa->match(tableMissFacts);
  ok &= expect(tableMatch.size() == 1 && tableMiss.empty(),
               "compiled features.fea facts are authoritative at runtime");
  ok &= expect(compiled.dfa != nullptr && compiled.actions.size() == 1 &&
                   compiled.selections.size() == 1 &&
                   compiled.stretchPolicies.size() == 1 &&
                   compiled.stretchPolicies[0].steps.size() == 1 &&
                   compiled.stretchPolicies[0].steps[0].phases.size() == 1 &&
                   compiled.rules[0].name == "ArbitraryRuleName" &&
                   compiled.selections[0].name == "ArbitrarySelectionName",
               "DFA, rule, selection, and record names are compiled once");

  const auto isolatedFacts = glyphFacts(
      {.glyphName = "behshape.isol", .sourceCharacter = u'ب'},
      compiled.facts);
  const auto unformedFacts = glyphFacts(
      {.glyphName = "behshape", .sourceCharacter = u'ب'}, compiled.facts);
  const auto medialFacts = glyphFacts(
      {.glyphName = "behshape.medi", .sourceCharacter = u'ب'},
      compiled.facts);
  ok &= expect(isolatedFacts != 0 && unformedFacts == 0 && medialFacts == 0,
               "catalog form predicates are interpreted without hard-coded glyph classes");

  const std::array<DfaFactSource, 4> explicitForms{{
      {"Initial", "init", {}},
      {"Medial", "medi", {}},
      {"Final", "fina", {}},
      {"Isolated", "isol", {}},
  }};
  ok &= expect(
      glyphFacts({.glyphName = "behshape.init.medi.fina.isol",
                  .sourceCharacter = u'ب'},
                 explicitForms) == 0b1111,
      "the four explicit form tags are independent glyph facts");

  const std::array preparedGlyphs{
      FixedSlotGlyphInput{.facts = tableMatchFacts[0],
                          .wordIndex = 0,
                          .subwordIndex = 0,
                          .baseIndex = 0,
                          .indexInLine = 0}};
  bool actionApplied = false;
  const DeclPolicyJustifier justifier(compiled);
  const auto overflow = justifier.apply(
      1, preparedGlyphs, [&](const FixedSlotActionSite& site) {
        actionApplied = site.definition == 0 &&
                        site.wordIndex == 0 && site.subwordIndex == 0 &&
                        site.matchSlots.size() == 1 &&
                        site.matchSlots[0].baseIndex == 0 &&
                        site.matchSlots[0].indexInLine == 0 &&
                        site.matchSlots[0].facts == tableMatchFacts[0];
        return FixedSlotActionResult::Positive;
      });
  ok &= expect(actionApplied && !overflow,
               "declarative policy consumes prepared glyph facts without HarfBuzz");
  ok &= expect(justifier.apply(1, preparedGlyphs, [](const FixedSlotActionSite&) { return FixedSlotActionResult::Overflow; }), "fixed-step overflow retains the historical stop signal");

  {
    auto choiceRecordSource = source;
    choiceRecordSource.selections[0].record.clear();
    choiceRecordSource.selections[0].choices[0].record = "Horizontal";
    choiceRecordSource.selections[0].choices[0].forbidRecorded = {"Horizontal"};
    setSingleStagePolicy(choiceRecordSource, {{"ArbitrarySelectionName", 2}});
    const auto choiceRecordCatalog = compileJustificationCatalog(choiceRecordSource);
    int choiceRecordActions = 0;
    DeclPolicyJustifier(choiceRecordCatalog).apply(1, preparedGlyphs, [&](const FixedSlotActionSite&) {
      ++choiceRecordActions;
      return FixedSlotActionResult::Positive;
    });
    ok &= expect(choiceRecordCatalog.selections[0].rules[0].record == "Horizontal" && choiceRecordCatalog.selections[0].rules[0].forbidRecorded == std::vector<std::string>{"Horizontal"} && choiceRecordActions == 1,
                 "choice-local records constrain later candidates without selection-wide records");
  }

  // Adjacency compares base positions (not marks or the entire subword).
  for (const bool selectionScope : {false, true}) {
    auto adjacentSource = source;
    auto& selection = adjacentSource.selections[0];
    (selectionScope ? selection.notAdjacent : selection.choices[0].notAdjacent) = {"ArbitraryRecordName"};
    const auto adjacentCatalog = compileJustificationCatalog(adjacentSource);
    const DeclPolicyJustifier adjacentEngine(adjacentCatalog);
    const std::array phases{PolicyPhase{.selection = 0, .levels = 1, .allocator = PolicyPhase::Allocator::CandidatePool}};
    for (const auto [base, subword, blocked] : std::array<std::tuple<int, int, bool>, 5>{{{1, 0, true}, {2, 0, false}, {1, 1, false}, {0, 0, false}, {3, 0, false}}}) {
      DeclPolicyExecutionState state;
      state.recordsByWord.resize(1);
      state.recordsByWord[0]["ArbitraryRecordName"] = {subword, base};
      int collected = 0;
      adjacentEngine.collectStageCandidates(1, preparedGlyphs, phases, state, [&](const auto&, auto, const auto&) { ++collected; });
      ok &= expect(collected == (blocked ? 0 : 1), "not_adjacent only blocks neighbouring base positions in the same subword");
      int applied = 0;
      auto fixedPhases = phases;
      fixedPhases[0].allocator = PolicyPhase::Allocator::FixedSteps;
      adjacentEngine.applyStage(1, preparedGlyphs, fixedPhases, state, [&](const auto&) { ++applied; return FixedSlotActionResult::Positive; });
      ok &= expect(applied == collected, "fixed-step and candidate collection share adjacency semantics");
    }
    auto later = preparedGlyphs;
    later[0].baseIndex = 2;
    DeclPolicyExecutionState state;
    state.recordsByWord.resize(1);
    state.recordsByWord[0]["ArbitraryRecordName"] = {0, 1};
    int collected = 0;
    adjacentEngine.collectStageCandidates(1, later, phases, state, [&](const auto&, auto, const auto&) { ++collected; });
    ok &= expect(collected == 0, "not_adjacent also rejects an earlier recorded neighbour");
    (selectionScope ? selection.notAdjacent : selection.choices[0].notAdjacent) = {"MissingRecord"};
    bool rejected = false;
    try { (void)compileJustificationCatalog(adjacentSource); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "unknown adjacency records are rejected during compilation");
  }

  // Required records refer to accepted state in the same word, at both
  // selection and choice scope. Empty lists preserve the old behavior.
  {
    auto requiredSource = source;
    auto seed = requiredSource.selections[0];
    seed.name = "Seed";
    seed.record = "KafBody";
    seed.choices[0].record = "Terminal";
    requiredSource.selections.push_back(seed);
    auto& companion = requiredSource.selections[0];
    companion.record = "Horizontal";
    companion.requireRecorded = {"KafBody", "Terminal"};
    companion.choices[0].requireRecorded = {"KafBody"};
    companion.forbidRecorded = {"Horizontal"};
    const auto requiredCatalog = compileJustificationCatalog(requiredSource);
    const DeclPolicyJustifier requiredJustifier(requiredCatalog);
    const auto& phases = requiredCatalog.stretchPolicies[0].steps[0].phases;
    DeclPolicyExecutionState state;
    state.recordsByWord.resize(2);
    const auto countCandidates = [&] {
      int count = 0;
      requiredJustifier.collectStageCandidates(2, preparedGlyphs, phases, state, [&](const FixedSlotActionSite&, std::size_t, const FixedSlotCandidateScore&) { ++count; });
      return count;
    };
    ok &= expect(countCandidates() == 0, "required records exclude unprepared words");
    state.recordsByWord[1]["KafBody"] = {};
    state.recordsByWord[1]["Terminal"] = {};
    ok &= expect(countCandidates() == 0, "required records cannot be borrowed from another word");
    state.recordsByWord[0]["KafBody"] = {};
    ok &= expect(countCandidates() == 0, "require_recorded requires every listed record, not any one");
    state.recordsByWord[0]["Terminal"] = {};
    ok &= expect(countCandidates() == 1 && requiredCatalog.selections[0].rules[0].requireRecorded == std::vector<std::string>{"KafBody"}, "selection and choice required records compile and admit a prepared word");
    int applied = 0;
    requiredJustifier.applyStage(2, preparedGlyphs, phases, state, [&](const FixedSlotActionSite&) { ++applied; return FixedSlotActionResult::Positive; });
    ok &= expect(applied == 1 && state.recordsByWord[0].contains("Horizontal") && countCandidates() == 0, "an accepted companion records Horizontal and prevents a second connection");
    state.recordsByWord[0].clear();
    requiredJustifier.applyStage(2, preparedGlyphs, phases, state, [&](const FixedSlotActionSite&) { ++applied; return FixedSlotActionResult::Positive; });
    ok &= expect(applied == 1, "fixed-step execution also enforces missing required records");

    requiredSource.selections[0].requireRecorded.clear();
    const auto choiceOnlyCatalog = compileJustificationCatalog(requiredSource);
    int choiceOnlyActions = 0;
    DeclPolicyJustifier(choiceOnlyCatalog).apply(1, preparedGlyphs, [&](const FixedSlotActionSite&) { ++choiceOnlyActions; return FixedSlotActionResult::Positive; });
    ok &= expect(choiceOnlyActions == 0, "choice-local requirements are enforced independently of selection requirements");
    requiredSource.selections[0].choices[0].requireRecorded = {"MisspelledRecord"};
    bool rejectedUnknownRecord = false;
    try { (void)compileJustificationCatalog(requiredSource); } catch (const std::invalid_argument&) { rejectedUnknownRecord = true; }
    ok &= expect(rejectedUnknownRecord, "unknown required record names are rejected during compilation");
  }

  // Allocator names are validated rather than silently falling back.
  {
    bool rejectedAllocator = false;
    try {
      auto invalid = source;
      invalid.stages[0].phases[0].allocator = "unknown";
      (void)compileJustificationCatalog(invalid);
    } catch (const std::invalid_argument&) {
      rejectedAllocator = true;
    }
    ok &= expect(rejectedAllocator, "an unknown phase allocator is rejected");
  }

  // Candidate ranges and lookup effects remain independent of discrete
  // action execution. Collection records them without committing changes.
  {
    auto rangeSource = source;
    rangeSource.selections[0].choices[0].multiplicity = "all_non_overlapping";
    rangeSource.actionDefinitions[0].effects = {{.kind = "vary", .target = {}, .attribute = "third", .value = {{.op = "literal", .literal = 7}}}};
    setSingleStagePolicy(rangeSource, {{"ArbitrarySelectionName", 1, "candidate_pool"}});
    const auto rangeCatalog = compileJustificationCatalog(rangeSource);
    {
      auto boundedSource = rangeSource;
      boundedSource.actionDefinitions[0].effects[0].maximum = {{.op = "literal", .literal = 9}};
      const auto boundedCatalog = compileJustificationCatalog(boundedSource);
      FakeBackend boundedBackend;
      const std::array slots{FixedSlotSlot{.indexInLine = 10}};
      FixedSlotActionSite site{.definition = 0, .rule = 0, .wordIndex = 0, .subwordIndex = 0, .context = slots};
      const auto candidate = collectJustificationCandidate(boundedCatalog, boundedCatalog.actionDefinitions[0], site, boundedBackend);
      ok &= expect(candidate && candidate->parameters[0].maximumValue == 7 && candidate->parameters[0].continuationMaximum == 9, "ordinary allocation retains its preferred endpoint while continuation can use the full maximum");
      JustificationCandidate pair;
      pair.parameters = {{.initialValue = 0, .minimumValue = 0.5, .maximumValue = 1.75, .continuationMaximum = 4}, {.initialValue = 0, .minimumValue = 0.25, .maximumValue = 0.875, .continuationMaximum = 2}};
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 1}) && pair.parameters[0].maximumValue == 1 && pair.parameters[1].maximumValue == 0.5, "relative continuation preserves the half-length incoming connection");
      pair.parameters = {{.initialValue = 1, .minimumValue = 1, .maximumValue = 4, .continuationMaximum = 4}, {.initialValue = 0.5, .minimumValue = 0.5, .maximumValue = 2, .continuationMaximum = 2}};
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 1}) && pair.parameters[0].maximumValue == 2 && pair.parameters[1].maximumValue == 1, "relative continuation starts from accepted values instead of resetting activation minima");
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 3}) && pair.parameters[0].maximumValue == 3 && pair.parameters[1].maximumValue == 1.5, "absolute continuation preserves paired interpolation");
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::Maximum}) && pair.parameters[0].maximumValue == 4 && pair.parameters[1].maximumValue == 2, "bare continuation offers the full remaining capacity");
      ok &= expect(!limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 0.5}), "absolute continuation never reduces an accepted value");
      pair.parameters = {{.initialValue = 0, .minimumValue = 1, .maximumValue = 3, .continuationMaximum = 9}};
      ok &= expect(!limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 0.5}) && pair.parameters[0].maximumValue == 3, "a step smaller than mandatory preparation is rejected without modifying its range");
      pair.parameters = {{.initialValue = 8.5, .minimumValue = 8.5, .maximumValue = 9, .continuationMaximum = 9}};
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 20}) && pair.parameters[0].maximumValue == 9, "absolute continuation stops at the declared full maximum");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 9;
      ok &= expect(!limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 1}), "saturated opportunities are not offered again");
      pair.parameters = {{.initialValue = 0, .minimumValue = 0.5, .maximumValue = 1.75, .continuationMaximum = 4, .continuationMinimum = 0.5}, {.initialValue = 0, .minimumValue = 0.25, .maximumValue = 0.875, .continuationMaximum = 2, .continuationMinimum = 0.25}, {.initialValue = 0, .minimumValue = 7, .maximumValue = 7}};
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 25, true}) && pair.parameters[0].maximumValue == 1.375 && pair.parameters[1].maximumValue == 0.6875 && pair.parameters[2].maximumValue == 7, "percentage targets use each original minimum-to-maximum range and leave fixed writes alone");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 1;
      pair.parameters[1].initialValue = pair.parameters[1].minimumValue = 0.5;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 25, true}) && pair.parameters[0].maximumValue == 1.875 && pair.parameters[1].maximumValue == 0.9375, "percentage increments use the full declared range, not remaining capacity");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 1.875;
      pair.parameters[1].initialValue = pair.parameters[1].minimumValue = 0.9375;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 50, true}) && pair.parameters[0].maximumValue == 2.25 && pair.parameters[1].maximumValue == 1.125, "percentage targets remain stable after previous continuations");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 3;
      pair.parameters[1].initialValue = pair.parameters[1].minimumValue = 0.5;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 50, true}) && pair.parameters[0].maximumValue == 3 && pair.parameters[1].maximumValue == 1.125, "an axis above its percentage target stays unchanged while another can continue");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 4;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 100, true}) && pair.parameters[0].maximumValue == 4 && pair.parameters[1].maximumValue == 2, "a saturated leading axis does not prevent percentage continuation of another axis");
      pair.parameters[1].initialValue = pair.parameters[1].minimumValue = 2;
      const auto saturatedParameters = pair.parameters;
      ok &= expect(!limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 25, true}) && pair.parameters == saturatedParameters, "a percentage target below all accepted values leaves the candidate unchanged");
      pair.parameters = {{.initialValue = 0, .minimumValue = 1, .maximumValue = 3, .continuationMaximum = 9, .continuationMinimum = 1}};
      ok &= expect(!limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 1, true}) && pair.parameters[0].maximumValue == 3, "small percentage increments cannot bypass mandatory activation minima");
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 0, true}) && pair.parameters[0].maximumValue == 1, "zero-percent targets expose only the mandatory minimum for a new site");
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 100, true}) && pair.parameters[0].maximumValue == 8, "percentage increments do not include activation minimum twice");
      pair.parameters[0].initialValue = pair.parameters[0].minimumValue = 8;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::By, 25, true}) && pair.parameters[0].maximumValue == 9, "percentage increments stop at the declared maximum");
      pair.parameters = {{.initialValue = 0.5, .minimumValue = 0.5, .maximumValue = 1.75, .continuationMaximum = 4, .continuationMinimum = 0.5}, {.initialValue = 2, .minimumValue = 2, .maximumValue = 3, .continuationMaximum = 8, .continuationMinimum = 2}};
      pair.minimumWidthDelta = 10;
      pair.maximumWidthDelta = 20;
      ok &= expect(limitJustificationCandidateContinuation(pair, {StageContinuation::Mode::To, 50, true}) && pair.parameters[0].maximumValue == 2.25 && pair.parameters[1].maximumValue == 5 && !pair.minimumWidthDelta && !pair.maximumWidthDelta, "unrelated parameter ranges get independent percentage endpoints and invalidate cached widths");
      boundedSource.actionDefinitions[0].effects.insert(boundedSource.actionDefinitions[0].effects.begin(), {.kind = "update", .target = {}, .attribute = "third", .value = {{.op = "literal", .literal = 1}}});
      const auto minimumCatalog = compileJustificationCatalog(boundedSource);
      const auto minimumCandidate = collectJustificationCandidate(minimumCatalog, minimumCatalog.actionDefinitions[0], site, boundedBackend);
      ok &= expect(minimumCandidate && minimumCandidate->parameters[0].initialValue == 0 && minimumCandidate->parameters[0].continuationMinimum == 1 && minimumCandidate->parameters[0].continuationMaximum == 9, "candidate collection records the post-preparation activation minimum separately from the initial value");
      boundedSource.stretchPolicies[0].steps[0].continuation = "by";
      boundedSource.stretchPolicies[0].steps[0].continuationLimit = 1;
      ok &= expect(compileJustificationCatalog(boundedSource).stretchPolicies[0].steps[0].continuation == StageContinuation{StageContinuation::Mode::By, 1}, "named candidate stages retain their continuation limit");
      boundedSource.stretchPolicies[0].steps[0].continuationLimit = 25;
      boundedSource.stretchPolicies[0].steps[0].continuationPercentage = true;
      ok &= expect(compileJustificationCatalog(boundedSource).stretchPolicies[0].steps[0].continuation == StageContinuation{StageContinuation::Mode::By, 25, true}, "named candidate stages retain percentage units");
      boundedSource.stages[0].phases[0].allocator = "fixed_steps";
      bool rejected = false;
      try { (void)compileJustificationCatalog(boundedSource); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "continuation stages reject incompatible allocation modes");
    }
    {
      auto baselineSource = rangeSource;
      baselineSource.selections[0].record.clear();
      baselineSource.selections[0].choices[0].record.clear();
      baselineSource.selections[0].choices[0].multiplicity = "all";
      baselineSource.rules[0].pattern.push_back(baselineSource.rules[0].pattern.front());
      setSingleStagePolicy(baselineSource, {{"ArbitrarySelectionName", 1, "baseline_pool"}});
      const auto baselineCatalog = compileJustificationCatalog(baselineSource);
      const std::array joinedGlyphs{
          FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 0, .subwordIndex = 0, .baseIndex = 0, .indexInLine = 0},
          FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 0, .subwordIndex = 0, .baseIndex = 1, .indexInLine = 1},
          FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 0, .subwordIndex = 0, .baseIndex = 2, .indexInLine = 2},
      };
      int count = 0;
      DeclPolicyJustifier(baselineCatalog).collectStageCandidates(1, joinedGlyphs, baselineCatalog.stretchPolicies[0].steps[0].phases, {}, [&](const auto&, auto, const auto&) { ++count; });
      ok &= expect(count == 2, "all retains both adjacent two-slot baseline matches");
      for (const auto badKind : {"clear", "lookup", "update"}) {
        auto invalid = baselineSource;
        invalid.actionDefinitions[0].effects[0].kind = badKind;
        invalid.actionDefinitions[0].effects[0].lookup = "justify.expand_terminal";
        bool rejected = false;
        try { (void)compileJustificationCatalog(invalid); } catch (const std::invalid_argument&) { rejected = true; }
        ok &= expect(rejected, "baseline spacing cannot clear, substitute or update ordinary action state");
      }
    }
    {
      auto shrinkSource = rangeSource;
      shrinkSource.shrinkPolicies = {{.name = "standard", .steps = {{.operation = "stage", .stage = "Main"}}}};
      const auto shrinkCatalog = compileJustificationCatalog(shrinkSource);
      ok &= expect(shrinkCatalog.shrinkPolicies[0].steps[0].phases[0].allocator == PolicyPhase::Allocator::CandidatePool,
                   "shrink recipes accept scored candidate stages");
      for (const auto allocator : {"baseline_pool", "fixed_steps"}) {
        auto invalid = shrinkSource;
        invalid.stages[0].phases[0].allocator = allocator;
        bool rejected = false;
        try { (void)compileJustificationCatalog(invalid); } catch (const std::invalid_argument&) { rejected = true; }
        ok &= expect(rejected, "shrink recipes reject allocators with stretch-only semantics");
      }
    }
    ok &= expect(rangeCatalog.stretchPolicies[0].steps[0].phases[0].allocator == PolicyPhase::Allocator::CandidatePool,
                 "candidate ranges compile with the candidate-pool allocator");

    FakeBackend rangeBackend;
    (void)evaluateJustificationAction(rangeCatalog,
                                      rangeCatalog.actionDefinitions[0],
                                      std::array<FixedSlotSlot, 1>{{{.indexInLine = 10}}},
                                      0, 0, 0, rangeBackend);
    ok &= expect(rangeBackend.varied == std::vector<std::tuple<int, JustAttributeId, double>>{{10, 0, 7}},
                 "vary forwards a native parameter endpoint without quantizing it");

    auto lookupSource = rangeSource;
    lookupSource.actionDefinitions[0].effects = {{.kind = "lookup", .target = {}, .lookup = "justify.expand_terminal"}};
    const auto lookupCatalog = compileJustificationCatalog(lookupSource);
    FakeBackend lookupBackend;
    const std::array<FixedSlotSlot, 1> lookupSlots{{{.indexInLine = 10}}};
    (void)evaluateJustificationAction(lookupCatalog, lookupCatalog.actionDefinitions[0], lookupSlots, 0, 0, 0, lookupBackend);
    ok &= expect(lookupBackend.committedLookups == std::vector<std::pair<int, std::string>>{{10, "justify.expand_terminal"}},
                 "lookup forwards the selected site and full lookup name transactionally");

    // Candidate collection executes the same guards, attachments and ordered
    // effects as a real action, but aborts the staged transaction. The result
    // owns enough state for a later line-wide planner.
    auto candidateSource = rangeSource;
    candidateSource.actionDefinitions[0].effects = {
        {.kind = "clear"},
        {.kind = "lookup", .lookup = "justify.expand_terminal"},
        {.kind = "update", .attribute = "third", .value = {{.op = "literal", .literal = 1}}},
        {.kind = "vary", .attribute = "third", .value = {{.op = "literal", .literal = 7}}},
    };
    const auto candidateCatalog = compileJustificationCatalog(candidateSource);
    FakeBackend candidateBackend;
    candidateBackend.committedGlyphIds[20] = 100;
    candidateBackend.lookupResults["justify.expand_terminal"] = 101;
    const std::array<FixedSlotSlot, 4> candidateSlots{{
        {.baseIndex = 0, .indexInLine = 10},
        {.baseIndex = 1, .indexInLine = 20},
        {.baseIndex = 2, .indexInLine = 30},
        {.baseIndex = 3, .indexInLine = 40},
    }};
    const FixedSlotActionSite candidateSite{
        .definition = 0,
        .anchorSlot = 0,
        .rule = candidateCatalog.rules[0].id,
        .wordIndex = 2,
        .subwordIndex = 3,
        .matchSlots = std::span(candidateSlots).subspan(1, 2),
        .context = candidateSlots,
        .matchOffset = 1,
    };
    const auto candidate = collectJustificationCandidate(
        candidateCatalog, candidateCatalog.actionDefinitions[0],
        candidateSite, candidateBackend, 2.5, 4);
    ok &= expect(candidate && candidate->definition == 0 &&
                     candidate->rule == candidateCatalog.rules[0].id &&
                     candidate->wordIndex == 2 &&
                     candidate->subwordIndex == 3 &&
                     candidate->subwordLength == 4 &&
                     candidate->connectionAfter == 2 &&
                     candidate->priorityBand == 4 && candidate->weight == 2.5 &&
                     candidate->context == std::vector<FixedSlotSlot>(candidateSlots.begin(), candidateSlots.end()),
                 "a collected candidate owns its action and matched subword geometry");
    ok &= expect(candidate && candidate->clearedSites == std::vector<int>{20} &&
                     candidate->substitutions ==
                         std::vector<JustificationCandidateSubstitution>{{20, "justify.expand_terminal", 100, 101}},
                 "candidate collection resolves and records structural substitution without committing it");
    ok &= expect(candidate && candidate->parameters.size() == 1 &&
                     candidateCatalog.attributes[candidate->parameters[0].attribute] == "third" &&
                     candidate->parameters[0].site == 20 &&
                     candidate->parameters[0].initialValue == 0 &&
                     candidate->parameters[0].minimumValue == 1 &&
                     candidate->parameters[0].maximumValue == 7 &&
                     candidate->minimumWidthDelta == 1 &&
                     candidate->maximumWidthDelta == 7,
                 "candidate collection records mandatory and maximum parameter values");
    ok &= expect(candidateBackend.commits == 0 && candidateBackend.aborts == 1 &&
                     candidateBackend.committed.empty() &&
                     candidateBackend.committedGlyphIds.at(20) == 100,
                 "candidate collection leaves committed parameter and glyph state unchanged");

    auto shrinkingSource = rangeSource;
    shrinkingSource.actionDefinitions[0].effects = {
        {.kind = "vary", .attribute = "third", .value = {{.op = "literal", .literal = -0.2}}}};
    const auto shrinkingCatalog = compileJustificationCatalog(shrinkingSource);
    FakeBackend shrinkingBackend;
    const auto shrinkingCandidate = collectJustificationCandidate(shrinkingCatalog,
        shrinkingCatalog.actionDefinitions[0], candidateSite, shrinkingBackend, 2.5, 0);
    ok &= expect(shrinkingCandidate && shrinkingCandidate->parameters[0].minimumValue == 0 &&
                     shrinkingCandidate->parameters[0].maximumValue == -0.2 &&
                     shrinkingCandidate->minimumWidthDelta == 0 && shrinkingCandidate->maximumWidthDelta == -0.2 &&
                     shrinkingBackend.committed.empty(),
                 "decreasing native ranges retain signed widths without mutating accepted state");

    FakeBackend lazyCandidateBackend;
    lazyCandidateBackend.committedGlyphIds[20] = 100;
    lazyCandidateBackend.lookupResults["justify.expand_terminal"] = 101;
    auto lazyCandidate = collectJustificationCandidate(candidateCatalog, candidateCatalog.actionDefinitions[0], candidateSite, lazyCandidateBackend, 2.5, 4, CandidateWidthMeasurement::Deferred);
    ok &= expect(lazyCandidate && !lazyCandidate->minimumWidthDelta && !lazyCandidate->maximumWidthDelta && lazyCandidateBackend.measurements == 0,
                 "deferred candidate collection performs no width measurements");
    ok &= expect(lazyCandidate && measureJustificationCandidate(*lazyCandidate, lazyCandidateBackend) && lazyCandidate->minimumWidthDelta == 1 && lazyCandidate->maximumWidthDelta == 7 && lazyCandidateBackend.measurements == 2,
                 "a retained candidate measures both endpoints on demand");

    // Manual continuation uses existing guards and vary, without update,
    // clear, lookup, or any special scheduler state.
    auto continuationSource = candidateSource;
    continuationSource.actionDefinitions[0].effects = {
        {.kind = "forbid", .condition = {{.op = "attribute", .name = "third"}, {.op = "literal", .literal = 0}, {.op = "gt"}, {.op = "not"}}},
        {.kind = "vary", .attribute = "third", .value = {{.op = "attribute", .name = "third"}, {.op = "literal", .literal = 7}, {.op = "max"}}},
    };
    const auto continuationCatalog = compileJustificationCatalog(continuationSource);
    FakeBackend continuationBackend;
    ok &= expect(!collectJustificationCandidate(continuationCatalog, continuationCatalog.actionDefinitions[0], candidateSite, continuationBackend),
                 "an ordinary positive-parameter guard rejects inactive continuation sites");
    const auto thirdAttribute = static_cast<JustAttributeId>(std::find(continuationCatalog.attributes.begin(), continuationCatalog.attributes.end(), "third") - continuationCatalog.attributes.begin());
    continuationBackend.committed[20] = {{thirdAttribute, 3}};
    continuationBackend.committedGlyphIds[20] = 101;
    const auto continuation = collectJustificationCandidate(continuationCatalog, continuationCatalog.actionDefinitions[0], candidateSite, continuationBackend);
    ok &= expect(continuation && continuation->parameters.size() == 1 && continuation->parameters[0].initialValue == 3 && continuation->parameters[0].minimumValue == 3 && continuation->parameters[0].maximumValue == 7,
                 "vary without update continues from the current accepted parameter value");
    ok &= expect(continuation && continuation->clearedSites.empty() && continuation->substitutions.empty() && continuationBackend.commits == 0 && continuationBackend.committed.at(20)[0].second == 3 && continuationBackend.committedGlyphIds.at(20) == 101,
                 "manual continuation neither resets nor substitutes the accepted glyph during collection");
    continuationBackend.committed[20][0].second = 8;
    const auto cappedContinuation = collectJustificationCandidate(continuationCatalog, continuationCatalog.actionDefinitions[0], candidateSite, continuationBackend);
    ok &= expect(cappedContinuation && cappedContinuation->parameters[0].minimumValue == 8 && cappedContinuation->parameters[0].maximumValue == 8,
                 "an explicit max expression prevents continuation from reducing an existing value");

    auto forbiddenCandidateSource = candidateSource;
    forbiddenCandidateSource.actionDefinitions[0].effects.insert(
        forbiddenCandidateSource.actionDefinitions[0].effects.begin(),
        {.kind = "forbid", .condition = {{.op = "literal", .literal = 1}}});
    const auto forbiddenCandidateCatalog =
        compileJustificationCatalog(forbiddenCandidateSource);
    FakeBackend forbiddenCandidateBackend;
    ok &= expect(!collectJustificationCandidate(
                     forbiddenCandidateCatalog,
                     forbiddenCandidateCatalog.actionDefinitions[0],
                     candidateSite, forbiddenCandidateBackend),
                 "a true action guard excludes the candidate during collection");

    const std::array inventoryGlyphs{
        FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 0, .subwordIndex = 0, .baseIndex = 0, .indexInLine = 10},
        FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 0, .subwordIndex = 0, .baseIndex = 1, .indexInLine = 20},
        FixedSlotGlyphInput{.facts = tableMatchFacts[0], .wordIndex = 1, .subwordIndex = 0, .baseIndex = 0, .indexInLine = 30},
    };
    std::vector<std::tuple<std::size_t, int, int>> inventory;
    DeclPolicyJustifier(candidateCatalog).collectStageCandidates(2, inventoryGlyphs, candidateCatalog.stretchPolicies[0].steps[0].phases, {}, [&](const FixedSlotActionSite& site, std::size_t phaseIndex, const FixedSlotCandidateScore&) {
      inventory.emplace_back(phaseIndex, site.wordIndex, site.matchSlots[0].baseIndex);
    });
    ok &= expect(inventory == std::vector<std::tuple<std::size_t, int, int>>{{0, 0, 1}, {0, 0, 0}, {0, 1, 0}},
                 "stage collection exposes the selected opportunities from every word in stable policy order");

    auto poolSource = candidateSource;
    poolSource.selections[0].choices[0].weight = 4;
    poolSource.selections[0].choices[0].decay = 0.5;
    setSingleStagePolicy(poolSource, {{"ArbitrarySelectionName", 3, "candidate_pool", 3, 5, 1, 1}});
    poolSource.stages[0].phases[0].longerSubword = 2;
    poolSource.stages[0].phases[0].centralConnection = 3;
    poolSource.stages[0].phases[0].wordPosition = 1;
    const auto poolCatalog = compileJustificationCatalog(poolSource);
    const auto& poolPhase = poolCatalog.stretchPolicies[0].steps[0].phases[0];
    ok &= expect(poolPhase.allocator == PolicyPhase::Allocator::CandidatePool && poolPhase.levels == 3 && poolPhase.weight == 3 && poolPhase.limit == 5 && poolPhase.perWord == 1 && poolPhase.perSubword == 1 && poolPhase.longerSubword == 2 && poolPhase.centralConnection == 3 && poolPhase.wordPosition == 1,
                 "candidate-pool score terms and distribution limits compile into the runtime policy");
    std::vector<double> weights;
    DeclPolicyJustifier(poolCatalog).collectStageCandidates(2, inventoryGlyphs, poolCatalog.stretchPolicies[0].steps[0].phases, {}, [&](const FixedSlotActionSite&, std::size_t, const FixedSlotCandidateScore& score) { weights.push_back(score.total); });
    ok &= expect(weights == std::vector<double>{13, 7, 4},
                 "choice decay, subword length, and word position compose into stable candidate scores");

    bool rejectedMixedPool = false;
    try {
      auto invalid = poolSource;
      invalid.stages[0].phases.push_back({"ArbitrarySelectionName", 1});
      (void)compileJustificationCatalog(invalid);
    } catch (const std::invalid_argument&) {
      rejectedMixedPool = true;
    }
    ok &= expect(rejectedMixedPool, "candidate-pool phases use a dedicated stage");

    bool rejectedStandalone = false;
    try {
      auto invalid = rangeSource;
      invalid.stages[0].phases[0].allocator = "proportional";
      (void)compileJustificationCatalog(invalid);
    } catch (const std::invalid_argument& error) {
      rejectedStandalone = std::string_view(error.what()) == "unknown justification phase allocator proportional";
    }
    ok &= expect(rejectedStandalone, "reject the removed standalone proportional allocator");

    for (const auto allocator : {"fixed_steps", "candidate_pool"}) {
      for (const int levels : {0, 256}) {
        bool rejectedLevels = false;
        try {
          auto invalid = rangeSource;
          invalid.stages[0].phases[0].allocator = allocator;
          invalid.stages[0].phases[0].levels = levels;
          (void)compileJustificationCatalog(invalid);
        } catch (const std::invalid_argument&) {
          rejectedLevels = true;
        }
        ok &= expect(rejectedLevels, "both allocators require level counts in 1..255");
      }
    }
  }

  // Discriminators are ANDed, and each is independently optional.
  const std::array<DfaFactSource, 4> discriminators{{
      {.name = "MedialOnly", .form = "medi"},
      {.name = "LastOnly", .position = "last"},
      {.name = "MedialAndLast", .form = "medi", .position = "last"},
      {.name = "NamedGlyph", .glyphNames = {"behshape.medi", "tehshape.medi"}},
  }};
  const auto medialLast =
      glyphFacts({.glyphName = "behshape.medi", .lastInSubword = true},
                 discriminators);
  const auto medialFirst =
      glyphFacts({.glyphName = "behshape.medi", .firstInSubword = true},
                 discriminators);
  const auto otherLast =
      glyphFacts({.glyphName = "jeemshape.medi", .lastInSubword = true},
                 discriminators);
  ok &= expect(medialLast == 0b1111,
               "every satisfied discriminator contributes its bit");
  ok &= expect(medialFirst == 0b1001,
               "a position discriminator excludes the wrong subword end");
  ok &= expect(otherLast == 0b0111,
               "a glyph-set discriminator excludes an unnamed glyph");

  // These ligature glyphs do not encode enough information to remove the
  // remaining lam/dal policy guard: dal/dhal share a skeleton, and the right
  // component can also follow medial lam (e.g. the corpus's khalidun).
  JustificationDfaSource lamDalSource{
      .name = "test.shared_ligature_guard",
      .facts = {{.name = "Lam", .sourceCharacters = {0x0644}}, {.name = "First", .position = "first"}, {.name = "Dal", .sourceCharacters = {0x062F}}},
      .rules = {{"R", {"Lam", "Dal"}}},
      .actions = {{"R", "Decompose"}},
      .actionDefinitions = {{.name = "Decompose", .effects = {{.kind = "when", .condition = {{.op = "fact", .target = {.slot = 1}, .name = "Lam"}, {.op = "fact", .target = {.slot = 1}, .name = "First"}, {.op = "and"}, {.op = "fact", .target = {.slot = 2}, .name = "Dal"}, {.op = "and"}}, .nested = {{.kind = "update", .target = {.slot = 1}, .attribute = "test", .value = {{.op = "literal", .literal = 1}}}, {.kind = "update", .target = {.slot = 2}, .attribute = "test", .value = {{.op = "literal", .literal = 1}}}}}}}},
      .selections = {{.name = "S", .choices = {{"R", "first"}}}},
  };
  setSingleStagePolicy(lamDalSource, {{"S", 1}});
  const auto lamDal = compileJustificationCatalog(lamDalSource);
  for (const bool first : {false, true}) {
    for (const char16_t character : {u'د', u'ذ'}) {
      FakeBackend backend;
      const std::array<FixedSlotSlot, 2> slots{{
          {.indexInLine = 0, .facts = glyphFacts({.glyphName = first ? "lam.init.beforedal" : "lam.medi.beforeheh", .sourceCharacter = u'ل', .firstInSubword = first}, lamDal.facts)},
          {.indexInLine = 1, .facts = glyphFacts({.glyphName = "dal.fina.afterlam", .sourceCharacter = character, .lastInSubword = true}, lamDal.facts)},
      }};
      evaluateJustificationAction(lamDal, lamDal.actionDefinitions[0], slots, 0, 0, 0, backend);
      const auto written = [&](int site) {
        const auto found = backend.committed.find(site);
        return found != backend.committed.end() && !found->second.empty();
      };
      const auto writtenCount =
          (written(0) ? 1u : 0u) + (written(1) ? 1u : 0u);
      ok &= expect(writtenCount == (first && character == u'د' ? 2u : 0u), "shared glyph coverage must not replace the initial-lam/dal-only policy guard");
    }
  }

  // Existing first/last fact discriminators express a whole two-base-glyph
  // Reh subword, not merely the last pair of a longer joining sequence.
  {
    const std::array<DfaFactSource, 2> shortRehFacts{{
        {.name = "Start", .form = "joins_right", .sourceCharacters = {u'ي', u'خ'}, .position = "first"},
        {.name = "End", .form = "final", .sourceCharacters = {u'ر'}, .position = "last"},
    }};
    const FixedSlotDfa shortRehDfa({{1, "ShortReh", {{.anyOf = 1}, {.anyOf = 2}}}});
    for (const auto first : {false, true}) {
      for (const auto last : {false, true}) {
        for (const auto endCharacter : {u'ر', u'ز'}) {
          const std::array facts{
              glyphFacts({.glyphName = "behshape.init.beforereh", .sourceCharacter = u'ي', .firstInSubword = first}, shortRehFacts),
              glyphFacts({.glyphName = "reh.fina", .sourceCharacter = endCharacter, .lastInSubword = last}, shortRehFacts),
          };
          ok &= expect(!shortRehDfa.match(facts).empty() == (first && last && endCharacter == u'ر'),
                       "two-slot Reh priority requires both subword boundaries and excludes Zay");
        }
      }
    }
    const std::array<DfaFactSource, 1> beforeHah{{
        {.name = "BeforeHah", .glyphNames = {"behshape.init.beforehah", "fehshape.init.beforehah", "meem.init.beforehah", "sad.init.beforehah"}},
    }};
    for (const auto& glyph : beforeHah[0].glyphNames) {
      ok &= expect(glyphFacts({.glyphName = glyph, .sourceCharacter = u'ي'}, beforeHah) == 1 && glyphFacts({.glyphName = glyph, .sourceCharacter = u'ض'}, beforeHah) == 1,
                   "glyph-only beforehah recognition is independent of the shared skeleton's source character");
    }
    ok &= expect(glyphFacts({.glyphName = "lam.init.beforehahyeh"}, beforeHah) == 0,
                 "the beforehah family does not include the different beforehahyeh compound");
  }

  // Marks carry none of the four form substrings.
  const std::array<DfaFactSource, 1> markForm{{{"Mark", "nonspacing"}}};
  ok &= expect(glyphFacts({.glyphName = "fatha"}, markForm) == 1 &&
                   glyphFacts({.glyphName = "behshape.medi"}, markForm) == 0,
               "the nonspacing form matches a glyph with no joining form");

  // A fact that constrains nothing would match every glyph.
  bool rejectedEmptyFact = false;
  try {
    JustificationDfaSource emptyFactSource{
        .name = "test.empty_fact",
        .facts = {{.name = "MatchesEverything"}},
        .rules = {{"R", {"MatchesEverything"}}},
        .actions = {{"R", "IncrementAlternate"}},
        .actionDefinitions = {{.name = "IncrementAlternate", .effects = {{.kind = "update", .attribute = "cv01", .value = {{.op = "literal", .literal = 1}}}}}},
        .selections = {{.name = "S", .choices = {{"R", "last"}}}},
    };
    setSingleStagePolicy(emptyFactSource, {{"S", 1}});
    compileJustificationCatalog(emptyFactSource);
  } catch (const std::invalid_argument&) {
    rejectedEmptyFact = true;
  }
  ok &= expect(rejectedEmptyFact,
               "a fact with no discriminator is rejected at compile time");

  // The glyph-set pass is font-supplied and normalises what it is handed.
  JustificationDfaSource glyphSetSource{
      .name = "test.glyph_set",
      .facts = {{.name = "FromSet", .glyphSetRef = 0}},
      .rules = {{"R", {"FromSet"}}},
      .actions = {{"R", "IncrementAlternate"}},
      .actionDefinitions = {{.name = "IncrementAlternate",
                             .effects = {{.kind = "update",
                                          .attribute = "cv01",
                                          .value = {{.op = "literal",
                                                     .literal = 1}}}}}},
      .selections = {{.name = "S", .choices = {{"R", "last"}}}},
  };
  setSingleStagePolicy(glyphSetSource, {{"S", 1}});
  auto resolved = compileJustificationCatalog(glyphSetSource);
  resolveJustificationGlyphSets(resolved, [](int) {
    return std::vector<std::string>{"zain.fina", "behshape.medi", "zain.fina"};
  });
  ok &= expect(resolved.facts[0].glyphNames ==
                   std::vector<std::string>{"behshape.medi", "zain.fina"},
               "resolved glyph names are sorted and de-duplicated");
  ok &= expect(glyphFacts({.glyphName = "zain.fina"}, resolved.facts) == 1 &&
                   glyphFacts({.glyphName = "zain.init"}, resolved.facts) == 0,
               "a resolved glyph set is matched by name");

  // The declarative form of applyAlternate, built as the parser would emit it.
  const DfaTargetSource self{};
  const DfaTargetSource fatha{.slot = -1, .attachment = "Fatha"};
  JustificationDfaSource alternateSource{
      .name = "test.alternate",
      .facts = {{.name = "Terminal", .form = "terminal"}},
      .rules = {{"AlternateRule", {"Terminal"}}},
      .actions = {{"AlternateRule", "IncrementAlternate"}},
      .attachments = {{.name = "Fatha", .within = 2, .skip = {0x0651}, .find = {0x064E}}},
      .actionDefinitions = {{
          .name = "IncrementAlternate",
          .effects =
              {
                  {.kind = "forbid",
                   .condition = {{.op = "attribute", .target = self, .name = "cv02"},
                                 {.op = "literal", .literal = 0},
                                 {.op = "gt"}}},
                  {.kind = "add",
                   .target = self,
                   .attribute = "cv01",
                   .value = {{.op = "literal", .literal = 1}},
                   .clamp = 12},
                  {.kind = "replace",
                   .target = fatha,
                   .writes = {{.attribute = "cv01",
                               .value = {{.op = "literal", .literal = 1},
                                         {.op = "attribute", .target = self, .name = "cv01"},
                                         {.op = "literal", .literal = 3},
                                         {.op = "floordiv"},
                                         {.op = "sum"}}}}},
              },
      }},
      .selections = {{.name = "Alternate", .choices = {{"AlternateRule", "last"}}}},
  };
  setSingleStagePolicy(alternateSource, {{"Alternate", 1}});
  // Native axes use the same transactional arithmetic as feature values.
  // No OpenType tag truncation ("third" -> "thir") is allowed here.
  auto bodySource = alternateSource;
  bodySource.actionDefinitions[0].effects = {{.kind = "add", .target = self, .attribute = "third", .value = {{.op = "literal", .literal = 1}}, .clamp = 20}};
  const auto bodyCatalog = compileJustificationCatalog(bodySource);
  ok &= expect(bodyCatalog.attributes == std::vector<std::string>{"third"}, "native axis name is preserved");
  FakeBackend bodyBackend;
  const std::array<FixedSlotSlot, 1> bodySlots{{{.baseIndex = 0, .indexInLine = 10, .facts = 1}}};
  for (int step = 0; step < 25; ++step) {
    (void)evaluateJustificationAction(bodyCatalog, bodyCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, bodyBackend);
    ok &= expect(bodyBackend.read(10, 0) == std::min(step + 1, 20), "body axis accumulates and clamps");
  }

  ok &= expect(bodyBackend.metricReads == 0, "existing actions do not query line metrics");
  for (const auto* metric : {"underfull", "underfull_percent"}) {
    auto thresholdSource = bodySource;
    const double threshold = std::string_view(metric) == "underfull" ? 100 : 10;
    thresholdSource.actionDefinitions[0].effects.insert(thresholdSource.actionDefinitions[0].effects.begin(),
        {.kind = "forbid", .condition = {{.op = metric}, {.op = "literal", .literal = threshold}, {.op = "gt"}, {.op = "not"}}});
    const auto thresholdCatalog = compileJustificationCatalog(thresholdSource);
    FakeBackend thresholdBackend;
    for (double width : {900.0, 901.0, 1000.0, 1100.0}) {
      thresholdBackend.metrics = {1000, width};
      ok &= expect(evaluateJustificationAction(thresholdCatalog, thresholdCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, thresholdBackend) == FixedSlotActionResult::Forbidden,
                   "underfull guard rejects equal/smaller gaps and overfull lines");
      ok &= expect(thresholdBackend.committed.empty(), "rejected width guard must not write");
    }
    thresholdBackend.metrics = {1000, 899.5};
    ok &= expect(evaluateJustificationAction(thresholdCatalog, thresholdCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, thresholdBackend) == FixedSlotActionResult::Positive,
                 "underfull guard accepts a gap strictly above its threshold");
    thresholdBackend.metrics = {1000, 950};
    ok &= expect(evaluateJustificationAction(thresholdCatalog, thresholdCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, thresholdBackend) == FixedSlotActionResult::Forbidden,
                 "later actions observe the updated committed line width");

    const FixedSlotActionSite thresholdSite{.matchSlots = bodySlots, .context = bodySlots};
    ok &= expect(!collectJustificationCandidate(thresholdCatalog, thresholdCatalog.actionDefinitions[0], thresholdSite, thresholdBackend),
                 "candidate recording forwards line metrics and applies width guards");
    thresholdBackend.metrics = {1000, 800};
    ok &= expect(collectJustificationCandidate(thresholdCatalog, thresholdCatalog.actionDefinitions[0], thresholdSite, thresholdBackend).has_value(),
                 "candidate collection accepts an eligible underfull action");

    auto metricSource = bodySource;
    metricSource.actionDefinitions[0].effects = {{.kind = "when", .condition = {{.op = metric}, {.op = "literal", .literal = threshold}, {.op = "gt"}}, .nested = bodySource.actionDefinitions[0].effects}};
    const auto metricCatalog = compileJustificationCatalog(metricSource);
    FakeBackend whenBackend;
    whenBackend.metrics = {1000, 900};
    (void)evaluateJustificationAction(metricCatalog, metricCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, whenBackend);
    ok &= expect(whenBackend.committed.empty(), "when skips an ineligible width branch");
    whenBackend.metrics = {1000, 800};
    (void)evaluateJustificationAction(metricCatalog, metricCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, whenBackend);
    ok &= expect(whenBackend.read(10, 0) == 1, "when executes an eligible width branch");

    metricSource.actionDefinitions[0].effects = {{.kind = "update", .target = self, .attribute = "third", .value = {{.op = metric}}}};
    const auto valueCatalog = compileJustificationCatalog(metricSource);
    FakeBackend valueBackend;
    for (const auto metrics : {JustificationLineMetrics{1000, 1100}, JustificationLineMetrics{0, 0}}) {
      valueBackend.metrics = metrics;
      (void)evaluateJustificationAction(valueCatalog, valueCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, valueBackend);
      ok &= expect(valueBackend.read(10, 0) == 0, "overfull and zero-width lines yield zero underfull");
    }
  }

  // Initial gap is stable across recipe stages, while remaining gap and net
  // stretch follow committed width. All percentages use the same denominator.
  const std::array<const char*, 6> lineMetricNames{"initial_underfull", "initial_underfull_percent", "underfull", "underfull_percent", "stretched", "stretched_percent"};
  struct MetricCase {
    JustificationLineMetrics metrics;
    std::array<double, 6> expected;
  };
  const std::array<MetricCase, 5> metricCases{{
      {{1000, 850, 700}, {300, 30, 150, 15, 150, 15}},
      {{1000, 925, 700}, {300, 30, 75, 7.5, 225, 22.5}},
      {{1000, 650, 700}, {300, 30, 350, 35, 0, 0}},
      {{1000, 1200, 1100}, {0, 0, 0, 0, 100, 10}},
      {{0, 100, 0}, {0, 0, 0, 0, 100, 0}},
  }};
  for (std::size_t metricIndex = 0; metricIndex < lineMetricNames.size(); ++metricIndex) {
    auto metricSource = bodySource;
    metricSource.actionDefinitions[0].effects = {{.kind = "update", .target = self, .attribute = "third", .value = {{.op = lineMetricNames[metricIndex]}}}};
    const auto metricCatalog = compileJustificationCatalog(metricSource);
    FakeBackend metricBackend;
    for (const auto& test : metricCases) {
      metricBackend.metrics = test.metrics;
      (void)evaluateJustificationAction(metricCatalog, metricCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, metricBackend);
      ok &= expect(std::abs(metricBackend.read(10, 0) - test.expected[metricIndex]) < 1e-10, "line expression returns the initial, remaining, or net stretched width");
    }
    metricSource.actionDefinitions[0].effects.insert(metricSource.actionDefinitions[0].effects.begin(),
        {.kind = "forbid", .condition = {{.op = lineMetricNames[metricIndex]}, {.op = "literal", .literal = 1}, {.op = "gt"}, {.op = "not"}}});
    const auto candidateCatalog = compileJustificationCatalog(metricSource);
    const FixedSlotActionSite metricSite{.matchSlots = bodySlots, .context = bodySlots};
    metricBackend.metrics = {1000, 850, 700};
    ok &= expect(collectJustificationCandidate(candidateCatalog, candidateCatalog.actionDefinitions[0], metricSite, metricBackend).has_value(), "candidate backend forwards all three line widths");
    metricBackend.metrics = {1000, 1000, 1000};
    ok &= expect(!collectJustificationCandidate(candidateCatalog, candidateCatalog.actionDefinitions[0], metricSite, metricBackend), "candidate guard rejects a zero initial gap, remaining gap, or stretch");
  }

  auto fractionalSource = bodySource;
  fractionalSource.actionDefinitions[0].effects = {{.kind = "add", .target = self, .attribute = "righttatweel", .value = {{.op = "literal", .literal = 0.5}}, .clamp = 2.0}};
  const auto fractionalCatalog = compileJustificationCatalog(fractionalSource);
  FakeBackend fractionalBackend;
  (void)evaluateJustificationAction(fractionalCatalog, fractionalCatalog.actionDefinitions[0], bodySlots, 0, 0, 0, fractionalBackend);
  ok &= expect(fractionalBackend.read(10, 0) == 0.5, "native axes accept fractional fixed-step values");

  auto deltaSource = bodySource;
  deltaSource.actionDefinitions[0].effects = {{.kind = "update", .target = self, .attribute = "lefttatweel_delta", .value = {{.op = "attribute", .target = self, .name = "sk04_lefttatweel"}}}};
  const auto deltaCatalog = compileJustificationCatalog(deltaSource);
  ok &= expect(deltaCatalog.attributeAxes.size() == 2 &&
                   deltaCatalog.attributeAxes[0] == digitalkhatt::LeftTatweelAxis &&
                   deltaCatalog.attributeAxes[1] == digitalkhatt::LeftTatweelAxis,
               "parameter deltas and contextual feature queries resolve to their native axis");
  for (const auto* kind : {"update", "add", "vary", "replace"}) {
    auto queryWrite = deltaSource;
    auto& effect = queryWrite.actionDefinitions[0].effects[0];
    effect.kind = kind;
    effect.attribute = "sk04_lefttatweel";
    effect.clamp = 1;
    effect.writes = {{.attribute = "sk04_lefttatweel", .value = {{.op = "literal", .literal = -0.1}}}};
    bool rejected = false;
    try { (void)compileJustificationCatalog(queryWrite); }
    catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "feature parameter queries reject every kind of write");
  }

  const auto alternate = compileJustificationCatalog(alternateSource);
  {
    auto substitutionSource = alternateSource;
    substitutionSource.attachments[0].substitution = DfaAttachmentSubstitutionSource{"fatha.stretched", "lefttatweel", 0, "fatha"};
    const auto substitutionCatalog = compileJustificationCatalog(substitutionSource);
    const auto resolve = [](std::string_view name) -> std::optional<std::uint32_t> {
      if (name == "fatha") return 10;
      if (name == "fatha.stretched") return 11;
      return std::nullopt;
    };
    const auto substitutions = resolveAttachmentSubstitutions(substitutionCatalog, resolve);
    ok &= expect(substitutions.size() == 1 && substitutions[0].axis == digitalkhatt::LeftTatweelAxis && substitutions[0].attachment == 0, "attachment substitution resolves its native axis and font-local glyph IDs");
    for (double value : {-1.0, 0.0, 0.8}) {
      GlyphParameters parameters;
      parameters.lefttatweel = value;
      parameters.third = 4;
      ok &= expect(substitutions[0].matches(10) && substitutions[0].matches(11) && !substitutions[0].matches(12) && substitutions[0].select(parameters) == (value > 0 ? 11 : 10) && parameters.third == 4,
                   "attachment identity selection is reversible, leaves other axes intact, and ignores other glyph families");
    }
    for (int invalid = 0; invalid < 4; ++invalid) {
      auto bad = substitutionSource;
      auto& rule = *bad.attachments[0].substitution;
      if (invalid == 0) rule.axis = "cv01";
      if (invalid == 1) rule.threshold = std::numeric_limits<double>::infinity();
      if (invalid == 2) rule.glyph = rule.otherwiseGlyph;
      if (invalid == 3) rule.otherwiseGlyph.clear();
      bool rejected = false;
      try { (void)compileJustificationCatalog(bad); }
      catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "attachment substitutions reject features as axes, nonfinite thresholds, empty names and identical identities");
    }
    bool rejectedMissing = false;
    try { (void)resolveAttachmentSubstitutions(substitutionCatalog, [](std::string_view) -> std::optional<std::uint32_t> { return std::nullopt; }); }
    catch (const std::invalid_argument&) { rejectedMissing = true; }
    ok &= expect(rejectedMissing, "attachment substitution fails before shaping if a declared font glyph is absent");
    auto overlapping = substitutionCatalog;
    overlapping.attachments.push_back(overlapping.attachments[0]);
    overlapping.attachments.back().name = "OtherAttachment";
    bool rejectedOverlap = false;
    try { (void)resolveAttachmentSubstitutions(overlapping, resolve); }
    catch (const std::invalid_argument&) { rejectedOverlap = true; }
    ok &= expect(rejectedOverlap, "overlapping substitution families cannot silently depend on attachment order");
  }
  ok &= expect(alternate.actionDefinitions.size() == 1 &&
                   alternate.attachments.size() == 1 &&
                   alternate.actions[0].definition == 0,
               "a bind naming a declared action resolves to its definition");

  // Attribute ids are interned in first-appearance order, so look them up.
  const auto attributeId = [&](std::string_view tag) {
    const auto found = std::find(alternate.attributes.begin(),
                                 alternate.attributes.end(), tag);
    return static_cast<JustAttributeId>(found - alternate.attributes.begin());
  };
  const auto cv01 = attributeId("cv01");
  const auto cv02 = attributeId("cv02");

  const std::array<FixedSlotSlot, 1> alternateSlots{
      {{.baseIndex = 0, .indexInLine = 10, .facts = 1}}};
  FakeBackend backend;
  const auto& action = alternate.actionDefinitions[0];

  // Seven applications must saturate at the clamp, and the attached mark must
  // track 1 + floor(cv01 / 3) at every step.
  const std::array<int, 7> expectedBase{1, 2, 3, 4, 5, 6, 7};
  bool sequenceOk = true;
  for (std::size_t step = 0; step < expectedBase.size(); ++step) {
    const auto result = evaluateJustificationAction(alternate, action,
                                                    alternateSlots, 0, 0, 0, backend);
    sequenceOk &= result == FixedSlotActionResult::Positive;
    sequenceOk &= backend.read(10, cv01) == expectedBase[step];
    sequenceOk &= backend.read(11, cv01) == 1 + expectedBase[step] / 3;
  }
  ok &= expect(sequenceOk,
               "the accumulate and the attached mark track each other");

  for (int step = 0; step < 20; ++step) {
    (void)evaluateJustificationAction(alternate, action, alternateSlots, 0, 0, 0,
                                      backend);
  }
  ok &= expect(backend.read(10, cv01) == 12,
               "the accumulate saturates at its clamp");

  // The replace must leave exactly one attribute at the mark, not merge into
  // whatever was there: applyAlternate assigned the fatha a fresh vector.
  backend.beginTransaction();
  backend.update(11, cv02, 7);
  (void)backend.commitTransaction(0);
  (void)evaluateJustificationAction(alternate, action, alternateSlots, 0, 0, 0,
                                    backend);
  ok &= expect(backend.committed[11].size() == 1 &&
                   backend.committed[11][0].first == cv01,
               "replace clears the target rather than merging into it");

  // A guard reads committed state and aborts without staging anything.
  backend.beginTransaction();
  backend.update(10, cv02, 1);
  (void)backend.commitTransaction(0);
  const auto committedValue = [&](int site, JustAttributeId attribute) {
    for (const auto& entry : backend.committed[site]) {
      if (entry.first == attribute) return entry.second;
    }
    return 0.0;
  };
  const auto commitsBefore = backend.commits;
  const auto forbidden = evaluateJustificationAction(alternate, action,
                                                     alternateSlots, 0, 0, 0, backend);
  ok &= expect(forbidden == FixedSlotActionResult::Forbidden &&
                   backend.commits == commitsBefore &&
                   committedValue(10, cv01) == 12,
               "a failed guard forbids the action and commits nothing");

  // A guard after a write would silently observe staged state instead.
  bool rejectedLateGuard = false;
  try {
    auto lateGuard = alternateSource;
    std::swap(lateGuard.actionDefinitions[0].effects[0],
              lateGuard.actionDefinitions[0].effects[1]);
    compileJustificationCatalog(lateGuard);
  } catch (const std::invalid_argument&) {
    rejectedLateGuard = true;
  }
  ok &= expect(rejectedLateGuard, "a guard placed after a write is rejected");

  // Every expression must leave exactly one value on the stack.
  bool rejectedUnbalanced = false;
  try {
    auto unbalanced = alternateSource;
    unbalanced.actionDefinitions[0].effects[1].value.push_back(
        {.op = "literal", .literal = 2});
    compileJustificationCatalog(unbalanced);
  } catch (const std::invalid_argument&) {
    rejectedUnbalanced = true;
  }
  ok &= expect(rejectedUnbalanced, "an unbalanced expression is rejected");

  // Stretch policies share one vocabulary -- and therefore one compiled DFA
  // and one interned id space -- while each complete recipe controls spacing,
  // features, and reusable glyph stages.
  {
    auto multi = source;
    multi.stages = {
        {.name = "Standard", .phases = {{"ArbitrarySelectionName", 1}}},
        {.name = "Aggressive", .phases = {{"ArbitrarySelectionName", 2}, {"ArbitrarySelectionName", 3}}},
    };
    multi.stretchPolicies = {
        {.name = "standard", .steps = {{.operation = "stage", .stage = "Standard"}}},
        {.name = "aggressive", .parameterQuantization = 0, .candidateWidth = "full_shape", .steps = {{.operation = "stage", .stage = "Aggressive"}}},
    };
    multi.shrinkPolicies = {
        {.name = "standard", .steps = {{.operation = "fit_features", .features = {"sk01"}}, {.operation = "balance"}}},
        {.name = "scale", .steps = {{.operation = "scale"}}},
    };
    const auto twoWay = compileJustificationCatalog(multi);
    ok &= expect(twoWay.stretchPolicies.size() == 2 &&
                     twoWay.stretchPolicies[0].name == "standard" &&
                     twoWay.stretchPolicies[1].name == "aggressive" &&
                     twoWay.stretchPolicies[0].parameterQuantization == 16 &&
                     twoWay.stretchPolicies[1].parameterQuantization == 0 &&
                     twoWay.stretchPolicies[0].candidateWidth == CandidateWidthMode::Advance &&
                     twoWay.stretchPolicies[1].candidateWidth == CandidateWidthMode::FullShape &&
                     twoWay.stretchPolicies[0].steps[0].phases.size() == 1 &&
                     twoWay.stretchPolicies[1].steps[0].phases.size() == 2,
                 "each stretch policy keeps its own schedule, quantization and candidate-width measurement");
    ok &= expect(twoWay.stretchPolicyIndex("aggressive") == 1 &&
                     twoWay.stretchPolicyIndex("absent") == -1,
                 "stretch policies are addressable by name");
    ok &= expect(twoWay.shrinkPolicies.size() == 2 &&
                     twoWay.shrinkPolicyIndex("scale") == 1 &&
                     twoWay.shrinkPolicyIndex("absent") == -1,
                 "shrink policies are compiled and addressable symmetrically");
    // The vocabulary is compiled once, so a fact bit means the same thing in
    // every policy -- the reason they live in one catalog.
    ok &= expect(twoWay.dfa != nullptr && twoWay.facts == compiled.facts &&
                     twoWay.selections.size() == compiled.selections.size(),
                 "stretch policies share the catalog's facts, rules and selections");

    bool rejectedDuplicate = false;
    try {
      auto duplicate = multi;
      duplicate.stretchPolicies[1].name = "standard";
      compileJustificationCatalog(duplicate);
    } catch (const std::invalid_argument&) {
      rejectedDuplicate = true;
    }
    ok &= expect(rejectedDuplicate, "duplicate stretch policy names are rejected");

    bool rejectedEmpty = false;
    try {
      auto empty = multi;
      empty.stretchPolicies[1].steps.clear();
      compileJustificationCatalog(empty);
    } catch (const std::invalid_argument&) {
      rejectedEmpty = true;
    }
    ok &= expect(rejectedEmpty, "a stretch policy with no steps is rejected");

    bool rejectedDuplicateShrink = false;
    try {
      auto duplicate = multi;
      duplicate.shrinkPolicies[1].name = "standard";
      compileJustificationCatalog(duplicate);
    } catch (const std::invalid_argument&) {
      rejectedDuplicateShrink = true;
    }
    ok &= expect(rejectedDuplicateShrink, "duplicate shrink policy names are rejected");

    bool rejectedEmptyShrink = false;
    try {
      auto empty = multi;
      empty.shrinkPolicies[1].steps.clear();
      compileJustificationCatalog(empty);
    } catch (const std::invalid_argument&) {
      rejectedEmptyShrink = true;
    }
    ok &= expect(rejectedEmptyShrink, "a shrink policy with no steps is rejected");

    auto complete = source;
    complete.selections.push_back({.name = "BlockedSelection", .choices = {{"ArbitraryRuleName", "last", "selected_only"}}, .traversal = "last_first", .forbidRecorded = {"ArbitraryRecordName"}});
    complete.stages = {{.name = "First", .phases = {{"ArbitrarySelectionName", 1}}}, {.name = "Second", .phases = {{"BlockedSelection", 1}}}};
    complete.stretchPolicies = {{.name = "standard", .steps = {{.operation = "cap_spaces", .arguments = {250, 250}}, {.operation = "stage", .stage = "First"}, {.operation = "fit_features", .features = {"kr01"}}, {.operation = "stage", .stage = "Second"}, {.operation = "fill_spaces"}}}};
    complete.shrinkPolicies = {{.name = "standard", .steps = {{.operation = "scale"}}}};
    DfaLinePolicySource completeLinePolicy;
    completeLinePolicy.stretchPolicy = "standard";
    completeLinePolicy.shrinkPolicy = "standard";
    complete.linePolicy = std::move(completeLinePolicy);
    const auto completeCatalog = compileJustificationCatalog(complete);
    ok &= expect(completeCatalog.linePolicy->stretchPolicyIndex == 0 && completeCatalog.linePolicy->shrinkPolicyIndex == 0 && completeCatalog.stretchPolicies[0].steps.size() == 5 && completeCatalog.stretchPolicies[0].steps[1].phases.size() == 1, "linepolicy selects complete stretch and shrink recipes");
    int completeActions = 0;
    DeclPolicyJustifier(completeCatalog).apply(1, preparedGlyphs, [&](const FixedSlotActionSite&) {
      ++completeActions;
      return FixedSlotActionResult::Positive;
    });
    ok &= expect(completeActions == 1, "selection records persist between stage invocations in one stretch recipe");

    bool rejectedUnknownStage = false;
    try {
      auto invalid = complete;
      invalid.stretchPolicies[0].steps[1].stage = "Missing";
      compileJustificationCatalog(invalid);
    } catch (const std::invalid_argument&) {
      rejectedUnknownStage = true;
    }
    ok &= expect(rejectedUnknownStage, "a stretch policy cannot reference an undefined stage");

    bool rejectedUnknownShrinkPolicy = false;
    try {
      auto invalid = complete;
      invalid.linePolicy->shrinkPolicy = "missing";
      compileJustificationCatalog(invalid);
    } catch (const std::invalid_argument&) {
      rejectedUnknownShrinkPolicy = true;
    }
    ok &= expect(rejectedUnknownShrinkPolicy, "linepolicy cannot reference an undefined shrink policy");

    bool rejectedRange = false;
    try {
      (void)twoWay.stretchPolicy(2);
    } catch (const std::out_of_range&) {
      rejectedRange = true;
    }
    ok &= expect(rejectedRange, "an out-of-range stretch policy index is rejected");
    bool rejectedShrinkRange = false;
    try {
      (void)twoWay.shrinkPolicy(2);
    } catch (const std::out_of_range&) {
      rejectedShrinkRange = true;
    }
    ok &= expect(rejectedShrinkRange, "an out-of-range shrink policy index is rejected");
  }

  // Every catalog has explicitly named stretch and shrink policies.
  ok &= expect(compiled.stretchPolicies.size() == 1 &&
                   compiled.stretchPolicies[0].name == "standard" &&
                   compiled.shrinkPolicies.size() == 1 &&
                   compiled.shrinkPolicies[0].name == "standard",
               "the named stretch and shrink policies are preserved");

  // An attachment matches either characters or glyphs on each side, never both.
  const auto attachmentSource = [](DfaAttachmentSource attachment) {
    JustificationDfaSource source{
        .name = "test.attachment",
        .facts = {{.name = "Terminal", .form = "terminal"}},
        .rules = {{"R", {"Terminal"}}},
        .actions = {{"R", "IncrementAlternate"}},
        .attachments = {std::move(attachment)},
        .actionDefinitions = {{.name = "IncrementAlternate", .effects = {{.kind = "update", .attribute = "cv01", .value = {{.op = "literal", .literal = 1}}}}}},
        .selections = {{.name = "S", .choices = {{"R", "last"}}}},
    };
    setSingleStagePolicy(source, {{"S", 1}});
    return source;
  };
  bool rejectedMixedFind = false;
  try {
    compileJustificationCatalog(attachmentSource(
        {.name = "Mixed", .within = 2, .find = {0x064E}, .findGlyphSetRef = 0}));
  } catch (const std::invalid_argument&) {
    rejectedMixedFind = true;
  }
  ok &= expect(rejectedMixedFind,
               "an attachment cannot find characters and glyphs at once");

  bool rejectedEmptyFind = false;
  try {
    compileJustificationCatalog(attachmentSource({.name = "Empty", .within = 2}));
  } catch (const std::invalid_argument&) {
    rejectedEmptyFind = true;
  }
  ok &= expect(rejectedEmptyFind, "an attachment that finds nothing is rejected");

  {
    auto response = attachmentSource({.name = "Fatha", .within = 2, .find = {0x064E}, .widthAxis = "lefttatweel", .gain = 1.0 / 3, .maximumWidth = 1614.06});
    const auto catalog = compileJustificationCatalog(response);
    ok &= expect(catalog.attachments[0].widthAxis == digitalkhatt::LeftTatweelAxis && catalog.attachments[0].gain == 1.0 / 3, "dependent mark width response compiles independently of actions");
    for (const auto scope : {"stretch", "shrink", "both"}) {
      response.attachments[0].during = scope;
      const auto scoped = compileJustificationCatalog(response).attachments[0];
      ok &= expect(scoped.appliesDuring(LineJustificationOperation::Stretch) == (std::string_view(scope) != "shrink") && scoped.appliesDuring(LineJustificationOperation::Shrink) == (std::string_view(scope) != "stretch") && scoped.appliesDuring(LineJustificationOperation::None) == (std::string_view(scope) == "both"), "attachment scope filters the selected line recipe, including exact-fit lines");
    }
    response.attachments[0].during = "unknown";
    bool badScope = false;
    try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { badScope = true; }
    ok &= expect(badScope, "unknown attachment scope is rejected");
    response.attachments[0].during = "both";
    response.attachments[0].followingMarkGlyphSetRef = 6;
    response.attachments[0].followingGrowthScale = 0.5;
    auto moderated = compileJustificationCatalog(response);
    resolveJustificationGlyphSets(moderated, [](int) { return std::vector<std::string>{"meemiqlab"}; });
    ok &= expect(moderated.attachments[0].followingMarkGlyphNames == std::vector<std::string>{"meemiqlab"} && moderated.attachments[0].followingGrowthScale == 0.5, "following-mark moderation is font declared and resolves glyph classes");
    for (const double invalid : {-0.1, 1.1, std::numeric_limits<double>::infinity()}) {
      response.attachments[0].followingGrowthScale = invalid;
      bool badScale = false;
      try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { badScale = true; }
      ok &= expect(badScale, "following-mark scale must be finite and between zero and one");
    }
    response.attachments[0].followingMarkGlyphSetRef = -1;
    response.attachments[0].followingGrowthScale = 1;
    response.attachments[0].excludeMarkGlyphSetRef = 5;
    auto blockedCatalog = compileJustificationCatalog(response);
    resolveJustificationGlyphSets(blockedCatalog, [](int) { return std::vector<std::string>{"smallalef.joined", "smallalef", "smallalef"}; });
    ok &= expect(blockedCatalog.attachments[0].excludeMarkGlyphNames == std::vector<std::string>{"smallalef", "smallalef.joined"}, "excluded marks use the font-resolved glyph set rather than hard-coded runtime names");
    response.attachments[0].excludeMarkGlyphSetRef = -1;
    response.attachments[0].widthAxis = "cv01";
    bool rejected = false;
    try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "dependent mark deformation requires a native axis");
    response.attachments[0].widthAxis = "lefttatweel";
    response.attachments[0].growthAxes = {"lefttatweel", "righttatweel"};
    const auto axisCatalog = compileJustificationCatalog(response);
    ok &= expect(axisCatalog.attachments[0].growthAxes == std::vector<digitalkhatt::GlyphAxisId>{digitalkhatt::LeftTatweelAxis, digitalkhatt::RightTatweelAxis}, "default mark growth axes compile independently of GPOS positioning");
    for (const auto badAxes : {std::vector<std::string>{"unknown"}, std::vector<std::string>{"cv01_lefttatweel"}, std::vector<std::string>{"lefttatweel", "lefttatweel"}}) {
      response.attachments[0].growthAxes = badAxes;
      rejected = false;
      try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "unknown, feature-query and duplicate mark growth axes are rejected");
    }
    response.attachments[0].growthAxes = {"lefttatweel", "righttatweel"};
    response.attachments[0].alignment = "right";
    rejected = false;
    try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "legacy attachment alignment is rejected rather than silently ignored");
    response.attachments[0].alignment = "unchanged";
    const auto physicalWidth = [](const digitalkhatt::GlyphParameters& p) -> std::optional<double> { return 100 + 10 * p.lefttatweel + 20 * p.righttatweel + 100 * p.third + 5 * p.lefttatweel * p.righttatweel; };
    const std::array<digitalkhatt::GlyphAxisId, 2> joiningAxes{digitalkhatt::LeftTatweelAxis, digitalkhatt::RightTatweelAxis};
    const digitalkhatt::GlyphParameters joined{.lefttatweel = 2, .righttatweel = 3, .third = 4};
    ok &= expect(dependentMarkAxisWidthGrowth(joined, joiningAxes, physicalWidth) == 110, "physical axis growth includes both tatweels and their interaction but excludes body growth");
    const std::array<digitalkhatt::GlyphAxisId, 1> leftOnly{digitalkhatt::LeftTatweelAxis};
    ok &= expect(dependentMarkAxisWidthGrowth(joined, leftOnly, physicalWidth) == 50, "unselected axes remain at their accepted values in the width reference");
    unsigned growthProbes = 0;
    const auto countedWidth = [&](const auto& p) { ++growthProbes; return physicalWidth(p); };
    ok &= expect(dependentMarkAxisWidthGrowth({.third = 4}, joiningAxes, countedWidth) == 0 && growthProbes == 0, "body-only changes do not expand the default mark or probe geometry");
    ok &= expect(dependentMarkAxisWidthGrowth({.lefttatweel = -2}, joiningAxes, physicalWidth) == 0, "shrinking axes never produce negative mark growth");
    const auto missingWidth = [](const auto&) -> std::optional<double> { return std::nullopt; };
    const auto invalidWidth = [](const auto&) -> std::optional<double> { return std::numeric_limits<double>::quiet_NaN(); };
    ok &= expect(!dependentMarkAxisWidthGrowth(joined, joiningAxes, missingWidth) && !dependentMarkAxisWidthGrowth(joined, joiningAxes, invalidWidth), "missing or nonfinite geometry safely declines selected-axis growth");
    const digitalkhatt::GlyphParameters neighbor{.lefttatweel = 1, .righttatweel = 4, .third = 5};
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth) == 150, "one connection sums current outgoing and next incoming physical width, retaining other axes");
    growthProbes = 0;
    ok &= expect(dependentMarkConnectionWidthGrowth({.righttatweel = 4}, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, countedWidth, countedWidth) == 0 && growthProbes == 0, "incoming-only deformation never stretches the left glyph's own mark or borrows another connection");
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, nullptr, digitalkhatt::RightTatweelAxis, physicalWidth, missingWidth) == 50, "a word boundary contributes no neighbor width");
    ok &= expect(!dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, missingWidth), "missing neighbor geometry safely declines the connection response");
    const digitalkhatt::GlyphParameters contextualCurrent{.lefttatweel = 1, .righttatweel = 3}, contextualNext{.righttatweel = 2};
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth, contextualCurrent, &contextualNext) == 75, "connection width excludes contextual GSUB tatweels on both glyphs and retains current body axes");
    growthProbes = 0;
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, countedWidth, countedWidth, joined, &neighbor) == 0 && growthProbes == 0, "context-only tatweel does not trigger Fatha stretching or geometry probes");
    ok &= expect(dependentMarkAxisWidthGrowth(joined, leftOnly, physicalWidth, contextualCurrent) == 25, "selected-axis growth resets to the shaped baseline instead of zero");
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth, contextualCurrent, &neighbor) == 25, "unchanged contextual incoming deformation contributes zero");
    const digitalkhatt::GlyphParameters partialCompensation{.righttatweel = 4.5}, fullCompensation{.righttatweel = 6};
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth, contextualCurrent, &partialCompensation) == 12.5, "incoming contraction offsets outgoing growth before charging Fatha width");
    ok &= expect(dependentMarkConnectionWidthGrowth(joined, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth, contextualCurrent, &fullCompensation) == 0, "a net-shortened connection never stretches its Fatha");
    ok &= expect(dependentMarkConnectionWidthGrowth({.lefttatweel = 0.5}, digitalkhatt::LeftTatweelAxis, &neighbor, digitalkhatt::RightTatweelAxis, physicalWidth, physicalWidth, contextualCurrent, &contextualNext) == 0, "shrinking below the contextual outgoing baseline does not claim a kashida");
    auto connection = response;
    connection.attachments[0].growthAxes.clear();
    connection.attachments[0].growthConnection = {"lefttatweel", "righttatweel"};
    const auto compiledConnection = compileJustificationCatalog(connection).attachments[0];
    ok &= expect(compiledConnection.growthConnection == std::vector<digitalkhatt::GlyphAxisId>{digitalkhatt::LeftTatweelAxis, digitalkhatt::RightTatweelAxis}, "connection axes compile in current/next order");
    for (const auto badAxes : {std::vector<std::string>{"lefttatweel"}, std::vector<std::string>{"lefttatweel", "righttatweel", "third"}, std::vector<std::string>{"lefttatweel", "unknown"}}) {
      connection.attachments[0].growthConnection = badAxes;
      rejected = false;
      try { (void)compileJustificationCatalog(connection); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "connection response requires exactly two registered axes");
    }
    connection.attachments[0].growthConnection = {"lefttatweel", "righttatweel"};
    connection.attachments[0].growthAxes = {"lefttatweel"};
    rejected = false;
    try { (void)compileJustificationCatalog(connection); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "same-glyph growth_axes and cross-glyph growth_connection cannot be mixed");
    unsigned probes = 0;
    const auto linear = [&](double value) -> std::optional<double> { ++probes; return 250 + 137 * value; };
    const auto value = dependentMarkWidthParameter(250 + 137 * 2.4, 10, 16, linear);
    ok &= expect(value == 2.375 && probes <= 4, "linear mark width inversion uses a secant and a bounded grid neighbour");
    ok &= expect(dependentMarkWidthParameter(250, 10, 16, linear) == 0 && dependentMarkWidthParameter(2000, 10, 16, linear) == 10, "dependent width clamps at the natural and maximum shapes");
    const auto curved = [](double value) -> std::optional<double> { return 250 + 10 * value * value; };
    ok &= expect(dependentMarkWidthParameter(250 + 10 * 2.4 * 2.4, 10, 16, curved) == 2.375, "nonlinear monotone mark profiles are inverted on the same grid");
    const auto absent = [](double) -> std::optional<double> { return std::nullopt; };
    ok &= expect(!dependentMarkWidthParameter(500, 10, 16, absent) && !dependentMarkWidthParameter(500, 10, 0, linear), "missing geometry and invalid grids decline the response safely");
    ok &= expect(dependentMarkWidthParameter(260, 0.01, 16, linear) == 0.01, "sub-grid maximum endpoints remain usable");
    response.attachments[0].widthAxis = "lefttatweel";
    response.attachments[0].responses = {{.glyphSetRef = 2, .mode = "keep"}, {.glyphSetRef = 3, .mode = "width_ratio", .ratio = 0.6}, {.glyphSetRef = 4, .mode = "growth_ratio", .ratio = 0.5}};
    auto specific = compileJustificationCatalog(response);
    resolveJustificationGlyphSets(specific, [](int reference) { return std::vector<std::string>{reference == 2 ? "kaf.init.ii" : reference == 3 ? "noon.fina.expa" : "lam.isol.expa"}; });
    const auto& profiles = specific.attachments[0].responses;
    ok &= expect(profiles[0].mode == MarkResponseMode::Keep && profiles[1].mode == MarkResponseMode::WidthRatio && profiles[1].ratio == 0.6 && profiles[2].mode == MarkResponseMode::GrowthRatio, "per-base width responses compile without positioning instructions");
    response.attachments[0].responses[1].alignment = "center";
    rejected = false;
    try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "legacy response alignment is rejected with a GPOS migration error");
    response.attachments[0].responses[1].alignment = "unchanged";
    for (const auto invalid : {0.0, 0.15, -1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
      response.attachments[0].responses[1].lowerGrowth = invalid;
      rejected = false;
      try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "invalid downward mark growth is rejected at font load");
    }
    response.attachments[0].responses[1].lowerGrowth.reset();
    response.attachments[0].responses[0].lowerGrowth = 1.0 / 7;
    rejected = false;
    try { (void)compileJustificationCatalog(response); } catch (const std::invalid_argument&) { rejected = true; }
    ok &= expect(rejected, "keep responses cannot also reposition marks");
    auto direct = attachmentSource({.name = "Fatha", .within = 2, .find = {0x064E}});
    direct.attachments[0].responses = {{.glyphNames = {"kaf.init.ii", "kaf.medi.ii"}, .mode = "parameter", .parameterAxis = "lefttatweel", .parameterValue = 1.875}};
    const auto parameterResponse = compileJustificationCatalog(direct).attachments[0].responses[0];
    ok &= expect(parameterResponse.mode == MarkResponseMode::Parameter && parameterResponse.parameterAxis == digitalkhatt::LeftTatweelAxis && parameterResponse.parameterValue == 1.875, "direct mark parameters compile without a base-width response or inversion");
    for (const auto value : {0.0, -1.0, 1.8723}) {
      direct.attachments[0].responses[0].parameterValue = value;
      ok &= expect(compileJustificationCatalog(direct).attachments[0].responses[0].parameterValue == value, "direct mark assignment preserves zero, negative and off-grid values exactly");
    }
    for (const auto axis : {"cv01", "cv01_lefttatweel", "lefttatweel_delta", "unknown"}) {
      direct.attachments[0].responses[0].parameterAxis = axis;
      rejected = false;
      try { (void)compileJustificationCatalog(direct); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "direct mark assignment requires an actual registered axis, not a feature query or delta");
    }
    direct.attachments[0].responses[0].parameterAxis = "third";
    direct.attachments[0].responses[0].parameterValue = 2;
    ok &= expect(compileJustificationCatalog(direct).attachments[0].responses[0].parameterAxis == digitalkhatt::ThirdAxis, "direct mark responses are not hard-coded to tatweel axes");
    for (const auto invalid : {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
      direct.attachments[0].responses[0].parameterValue = invalid;
      rejected = false;
      try { (void)compileJustificationCatalog(direct); } catch (const std::invalid_argument&) { rejected = true; }
      ok &= expect(rejected, "nonfinite direct mark values are rejected at font load");
    }
  }

  auto glyphAttachment = compileJustificationCatalog(attachmentSource(
      {.name = "Fatha", .within = 2, .skipGlyphSetRef = 1, .findGlyphSetRef = 0}));
  resolveJustificationGlyphSets(glyphAttachment, [](int reference) {
    return reference == 0 ? std::vector<std::string>{"fatha.alt", "fatha"}
                          : std::vector<std::string>{"shadda"};
  });
  ok &= expect(glyphAttachment.attachments[0].findGlyphNames ==
                       std::vector<std::string>{"fatha", "fatha.alt"} &&
                   glyphAttachment.attachments[0].matchesFindGlyphs() &&
                   glyphAttachment.attachments[0].matchesSkipGlyphs(),
               "attachment glyph sets are resolved, sorted and flagged");

  // A two-slot action: a fact-guarded forbid, writes to both slots, and a mark
  // hung off slot 1 that takes its value from slot 2 -- the shape of applyKaf.
  constexpr std::uint64_t noonBit = 1;
  const DfaTargetSource first{.slot = 1};
  const DfaTargetSource second{.slot = 2};
  const DfaTargetSource firstFatha{.slot = 1, .attachment = "Fatha"};
  JustificationDfaSource kafSource{
      .name = "test.kaf",
      .facts = {{.name = "Noon", .sourceCharacters = {1606}}, {.name = "JoinsLeft", .form = "joins_left"}},
      .rules = {{"KafRule", {"JoinsLeft", "JoinsLeft"}}},
      .actions = {{"KafRule", "ApplyKafPair"}},
      .attachments = {{.name = "Fatha", .within = 2, .find = {0x064E}}},
      .actionDefinitions = {{
          .name = "ApplyKafPair",
          .effects =
              {
                  {.kind = "forbid", .condition = {{.op = "fact", .target = second, .name = "Noon"}, {.op = "attribute", .target = second, .name = "cv01"}, {.op = "literal", .literal = 0}, {.op = "gt"}, {.op = "and"}}},
                  {.kind = "update", .target = first, .attribute = "cv03", .value = {{.op = "literal", .literal = 1}}},
                  {.kind = "update", .target = second, .attribute = "cv03", .value = {{.op = "literal", .literal = 1}}},
                  {.kind = "replace", .target = firstFatha, .writes = {{.attribute = "cv01", .value = {{.op = "literal", .literal = 1}, {.op = "attribute", .target = second, .name = "cv01"}, {.op = "literal", .literal = 3}, {.op = "floordiv"}, {.op = "sum"}}}}},
              },
      }},
      .selections = {{.name = "Kaf", .choices = {{"KafRule", "last"}}}},
  };
  setSingleStagePolicy(kafSource, {{"Kaf", 1}});
  const auto kaf = compileJustificationCatalog(kafSource);
  const auto kafAttribute = [&](std::string_view tag) {
    const auto found = std::find(kaf.attributes.begin(), kaf.attributes.end(), tag);
    return static_cast<JustAttributeId>(found - kaf.attributes.begin());
  };
  const auto kafCv01 = kafAttribute("cv01");
  const auto kafCv03 = kafAttribute("cv03");
  const auto& kafAction = kaf.actionDefinitions[0];

  // Slot 2 is a noon carrying cv01, so the guard must forbid.
  const std::array<FixedSlotSlot, 2> noonSlots{
      {{.baseIndex = 0, .indexInLine = 20, .facts = 0},
       {.baseIndex = 1, .indexInLine = 30, .facts = noonBit}}};
  FakeBackend kafBackend;
  kafBackend.beginTransaction();
  kafBackend.update(30, kafCv01, 4);
  (void)kafBackend.commitTransaction(0);
  ok &= expect(evaluateJustificationAction(kaf, kafAction, noonSlots, 0, 0, 0,
                                           kafBackend) ==
                   FixedSlotActionResult::Forbidden,
               "a fact-guarded forbid fires when both halves hold");

  // The same slots without the noon fact must pass: "and" needs both halves.
  const std::array<FixedSlotSlot, 2> plainSlots{
      {{.baseIndex = 0, .indexInLine = 20, .facts = 0},
       {.baseIndex = 1, .indexInLine = 30, .facts = 0}}};
  const auto passed = evaluateJustificationAction(kaf, kafAction, plainSlots, 0,
                                                  0, 0, kafBackend);
  ok &= expect(passed == FixedSlotActionResult::Positive,
               "the same guard passes when the fact half is false");
  ok &= expect(kafBackend.read(20, kafCv03) == 1 &&
                   kafBackend.read(30, kafCv03) == 1,
               "a two-slot action writes to both of its slots");
  // The mark hangs off slot 1 but reads slot 2: 1 + floor(4 / 3) == 2.
  ok &= expect(kafBackend.read(21, kafCv01) == 2,
               "a mark on one slot can take its value from another");

  // isfact must read the slot it names, not the anchor.
  const std::array<FixedSlotSlot, 2> firstIsNoon{
      {{.baseIndex = 0, .indexInLine = 20, .facts = noonBit},
       {.baseIndex = 1, .indexInLine = 30, .facts = 0}}};
  ok &= expect(evaluateJustificationAction(kaf, kafAction, firstIsNoon, 0, 0, 0,
                                           kafBackend) !=
                   FixedSlotActionResult::Forbidden,
               "isfact reads the slot it names rather than the anchor");

  // Lookahead, when/clear/select/has -- the kashida machinery.  The mushaf
  // never exercises the lookahead branches of the decomposition cascade, so
  // this is the only thing that proves they work.
  constexpr std::uint64_t markerBit = 1;
  const DfaTargetSource slot1{.slot = 1};
  const DfaTargetSource slot2{.slot = 2};
  const DfaTargetSource slot3{.slot = 3};
  JustificationDfaSource aheadSource{
      .name = "test.lookahead",
      .facts = {{.name = "Marker", .sourceCharacters = {1}}, {.name = "AnyForm", .form = "joins_left"}},
      .rules = {{"Pair", {"AnyForm", "AnyForm"}}},
      .actions = {{"Pair", "Look"}},
      .actionDefinitions = {{
          .name = "Look",
          .effects =
              {
                  // Presence, not "> 0": cv05 is staged as zero below.
                  {.kind = "forbid", .condition = {{.op = "present", .target = slot1, .name = "cv05"}}},
                  {.kind = "clear", .target = slot2},
                  // Only fires when the glyph past the match carries Marker.
                  {.kind = "when", .condition = {{.op = "fact", .target = slot3, .name = "Marker"}}, .nested = {{.kind = "update", .target = slot1, .attribute = "cv07", .value = {{.op = "literal", .literal = 9}}}}},
                  {.kind = "update", .target = slot2, .attribute = "cv08", .value = {{.op = "fact", .target = slot3, .name = "Marker"}, {.op = "literal", .literal = 100}, {.op = "literal", .literal = 200}, {.op = "select"}}},
              },
      }},
      .selections = {{.name = "S", .choices = {{"Pair", "last"}}}},
  };
  setSingleStagePolicy(aheadSource, {{"S", 1}});
  const auto ahead = compileJustificationCatalog(aheadSource);
  const auto aheadAttribute = [&](std::string_view tag) {
    const auto found = std::find(ahead.attributes.begin(), ahead.attributes.end(), tag);
    return static_cast<JustAttributeId>(found - ahead.attributes.begin());
  };
  const auto cv05 = aheadAttribute("cv05");
  const auto cv07 = aheadAttribute("cv07");
  const auto cv08 = aheadAttribute("cv08");
  const auto& lookAction = ahead.actionDefinitions[0];

  // A three-glyph subword whose third glyph carries Marker, matched at offset 0.
  const std::array<FixedSlotSlot, 3> subword{
      {{.baseIndex = 0, .indexInLine = 40, .facts = 0},
       {.baseIndex = 1, .indexInLine = 41, .facts = 0},
       {.baseIndex = 2, .indexInLine = 42, .facts = markerBit}}};
  FakeBackend look;
  (void)evaluateJustificationAction(ahead, lookAction, subword, 0, 0, 0, look);
  ok &= expect(look.read(40, cv07) == 9,
               "a two-slot match can read the glyph past it");
  ok &= expect(look.read(41, cv08) == 100,
               "select takes the branch the lookahead condition chose");

  // The same match at the end of a two-glyph subword: $3 falls outside, which
  // must read as no-match rather than throwing.
  const std::array<FixedSlotSlot, 2> shortSubword{
      {{.baseIndex = 0, .indexInLine = 50, .facts = 0},
       {.baseIndex = 1, .indexInLine = 51, .facts = 0}}};
  FakeBackend edge;
  (void)evaluateJustificationAction(ahead, lookAction, shortSubword, 0, 0, 0, edge);
  ok &= expect(!edge.present(50, cv07) && edge.read(51, cv08) == 200,
               "a lookahead past the subword end matches nothing");

  // Matching later in the subword shifts what $1..$3 address.
  FakeBackend shifted;
  (void)evaluateJustificationAction(ahead, lookAction, subword, 1, 0, 0, shifted);
  ok &= expect(!shifted.present(41, cv07) && shifted.read(42, cv08) == 200,
               "slots are addressed relative to the match, not the subword");

  // "has" is presence, which a zero value satisfies but "> 0" would not.
  FakeBackend presence;
  presence.beginTransaction();
  presence.update(40, cv05, 0);
  (void)presence.commitTransaction(0);
  ok &= expect(evaluateJustificationAction(ahead, lookAction, subword, 0, 0, 0,
                                           presence) ==
                   FixedSlotActionResult::Forbidden,
               "has() tests presence rather than a positive value");

  // clear drops attributes the action does not rewrite.
  FakeBackend cleared;
  cleared.beginTransaction();
  cleared.update(41, cv07, 5);
  (void)cleared.commitTransaction(0);
  (void)evaluateJustificationAction(ahead, lookAction, subword, 0, 0, 0, cleared);
  ok &= expect(!cleared.present(41, cv07) && cleared.read(41, cv08) == 100,
               "clear drops the target's other attributes");

  // Recognition follows accepted state, not the original token list. Rule A
  // disappears after substitution and rule B becomes eligible at the next
  // level; a subsequent guard-only change must update the context as well.
  JustificationDfaSource currentSource{
      .name = "test.current_state",
      .facts = {{.name = "A", .form = "init"}, {.name = "B", .form = "medi"}, {.name = "Guard", .position = "first"}},
      .rules = {{"RA", {"A"}}, {"RB", {"B"}}},
      .actions = {{"RA", "Step"}, {"RB", "Step"}},
      .actionDefinitions = {{.name = "Step", .effects = {{.kind = "update", .attribute = "test", .value = {{.op = "literal", .literal = 1}}}}}},
      .selections = {{.name = "S", .choices = {{"RA", "first", "selected_only"}, {"RB", "first", "selected_only"}}, .traversal = "first_first"}},
  };
  setSingleStagePolicy(currentSource, {{"S", 3}});
  const auto currentCatalog = compileJustificationCatalog(currentSource);
  const auto ruleA = currentCatalog.rules[0].id;
  const auto ruleB = currentCatalog.rules[1].id;
  std::array currentGlyphs{
      FixedSlotGlyphInput{.facts = 1, .wordIndex = 0, .subwordIndex = 0, .baseIndex = 2, .indexInLine = 7},
      FixedSlotGlyphInput{.facts = 1, .wordIndex = 1, .subwordIndex = 0, .baseIndex = 0, .indexInLine = 12}};
  std::vector<JustificationRuleId> seenRules;
  std::vector<std::uint64_t> seenFacts;
  int refreshed = 0;
  int rejectedCalls = 0;
  DeclPolicyJustifier(currentCatalog).apply(2, currentGlyphs, [&](const FixedSlotActionSite& site) {
    if (site.wordIndex == 1) {
      ++rejectedCalls;
      ok &= expect(site.rule == ruleA && site.matchSlots[0].facts == 1, "rejected trial must not change recognition");
      currentGlyphs[1].facts = 2; // Uncommitted candidate must never be read.
      return FixedSlotActionResult::Forbidden;
    }
    seenRules.push_back(site.rule);
    seenFacts.push_back(site.matchSlots[0].facts);
    ok &= expect(site.matchSlots[0].indexInLine == 7 && site.matchSlots[0].baseIndex == 2, "substitution must preserve source-backed slot ids");
    currentGlyphs[0].facts = seenRules.size() == 1 ? 6 : 2;
    return FixedSlotActionResult::Positive; }, [&](int wordIndex) {
    ++refreshed;
    ok &= expect(wordIndex == 0, "only the word with an accepted action is refreshed");
    return std::vector{currentGlyphs[wordIndex]}; });
  ok &= expect(seenRules == std::vector<JustificationRuleId>{ruleA, ruleB, ruleB}, "new matches become eligible and stale matches disappear");
  ok &= expect(seenFacts == std::vector<std::uint64_t>{1, 6, 2}, "guard-only facts refresh even when DFA predicates do not change");
  ok &= expect(refreshed == 3 && rejectedCalls == 3, "refresh occurs only after accepted transactions");
  for (const auto rejected : {FixedSlotActionResult::NoChange, FixedSlotActionResult::Overflow}) {
    bool readCandidate = false;
    DeclPolicyJustifier(currentCatalog).apply(1, std::span(currentGlyphs).first(1), [&](const auto&) { return rejected; }, [&](int) {
      readCandidate = true;
      return std::vector<FixedSlotGlyphInput>{}; });
    ok &= expect(!readCandidate, "no-change and overflow trials never refresh recognition");
  }

  return ok ? 0 : 1;
}
