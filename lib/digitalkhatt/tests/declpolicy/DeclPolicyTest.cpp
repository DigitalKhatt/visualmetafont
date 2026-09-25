#include <digitalkhatt/justify/declpolicy/FixedSlotDfa.h>
#include <digitalkhatt/justify/declpolicy/JustificationActionEvaluator.h>
#include <digitalkhatt/justify/declpolicy/JustificationCandidate.h>
#include <digitalkhatt/justify/declpolicy/JustificationStagingBackend.h>
#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

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

  const auto alternate = compileJustificationCatalog(alternateSource);
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
