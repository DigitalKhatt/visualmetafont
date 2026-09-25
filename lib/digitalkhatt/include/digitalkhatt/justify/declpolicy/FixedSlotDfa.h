#pragma once

#include <cstddef>
#include <compare>
#include <cstdint>
#include <deque>
#include <map>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace digitalkhatt::justify {

// A predicate over facts attached to one shaped glyph.  Facts are deliberately
// owned by the rule set, not by HarfBuzz: a font adapter may derive them from a
// glyph id/name, source cluster, form, or a future glyph parameter axis.
struct GlyphPredicate {
  std::uint64_t allOf = 0;
  std::uint64_t anyOf = 0;
  std::uint64_t noneOf = 0;

  bool matches(std::uint64_t facts) const;
  bool operator==(const GlyphPredicate&) const = default;
};

// A fixed-slot rule is intentionally less expressive than a regular expression.
// Every element consumes exactly one glyph; there is no repetition or
// backtracking.  This lets the compiler produce a small deterministic table.
struct FixedSlotRule {
  std::uint16_t id = 0;
  std::string name;
  std::vector<GlyphPredicate> pattern;
  bool operator==(const FixedSlotRule&) const = default;
};

struct FixedSlotMatch {
  std::uint16_t ruleId = 0;
  std::size_t first = 0;
  std::size_t last = 0;  // inclusive

  bool operator==(const FixedSlotMatch&) const = default;
  auto operator<=>(const FixedSlotMatch&) const = default;
};

// Determinizes fixed-length rules over glyph facts.
//
// A glyph is not a symbol here: it satisfies a *set* of predicates at once, so
// a state's alphabet is the powerset of the predicates it branches on rather
// than a flat character range.  Enumerating that powerset is what a lexer
// generator can afford and this cannot, because the concrete alphabet -- glyph
// name crossed with source character and subword position -- is not known when
// the rules are compiled.  So narrow states are enumerated up front and wide
// ones discover their transitions from the text that actually arrives, which
// removes any bound on how many predicates one position may distinguish.
//
// Memoizing on use means match() mutates internal caches through a const
// object.  Like the rest of this library it is single-threaded: one instance
// must not be matched from several threads at once.
class FixedSlotDfa {
 public:
  explicit FixedSlotDfa(std::vector<FixedSlotRule> rules);

  std::vector<FixedSlotMatch> match(std::span<const std::uint64_t> glyphFacts) const;

  // The four counters below describe the table as it stands now.  Matching can
  // grow it, so they are only stable once the text has been seen.
  std::size_t stateCount() const { return states_.size(); }
  std::size_t maximumRuleLength() const { return maximumRuleLength_; }
  // Distinct slot predicates over the whole rule set.  Unlike the per-state
  // branching factor this number costs nothing: it does not size any table.
  std::size_t predicateCount() const { return predicates_.size(); }
  // Enumerated entries plus memoized ones -- the real memory figure.
  std::size_t transitionCount() const;
  // The largest number of predicates any single state branches on.  Above
  // kDenseBranchingLimit a state memoizes instead of enumerating, so this is no
  // longer bounded.
  std::size_t maximumBranchingFactor() const;
  // States that memoize rather than enumerate.
  std::size_t lazyStateCount() const;

 private:
  // Enumerating a row costs 2^branching entries, so only narrow states get one.
  // Eight keeps any single row to 256 entries.
  static constexpr std::size_t kDenseBranchingLimit = 8;

  struct NfaEdge {
    std::uint16_t predicate = 0;
    std::uint16_t target = 0;
  };

  struct NfaState {
    std::vector<NfaEdge> edges;
    std::vector<std::uint16_t> accepts;
  };

  // A state consults only the predicates that appear on its own outgoing
  // edges, so its row covers those and nothing else.  `targets` is dense over
  // them when the state is narrow; when it is wide `targets` is empty and
  // `memo` holds only the combinations real glyphs have produced.
  struct State {
    std::vector<std::uint8_t> relevant;   // predicate indices, ascending
    std::vector<std::uint16_t> targets;   // dense over 1 << relevant.size()
    std::vector<std::uint16_t> accepts;
    std::vector<std::uint16_t> nfaStates;
    std::unordered_map<std::uint64_t, std::uint16_t> memo;
  };

  using StateSet = std::vector<std::uint16_t>;

  // Returns the state for this NFA subset, creating it if new.  A narrow state
  // is enumerated immediately, which recurses -- bounded by the rule length,
  // since fixed-length rules make the state graph a layered DAG.
  std::uint16_t intern(StateSet set) const;
  // The NFA subset reached from `set` when exactly `firing` holds.
  StateSet successor(const StateSet& set, std::uint64_t firing) const;
  // Expands a compacted key back into the global predicate numbering.
  std::uint64_t firingMask(const State& state, std::uint64_t key) const;
  // One transition out of a wide state, memoizing both hits and dead ends.
  std::uint16_t memoizedStep(State& state, std::uint64_t key) const;

  std::vector<GlyphPredicate> predicates_;
  std::vector<NfaState> nfa_;
  // Deque, not vector: match() appends states while the loop holds a reference
  // to the one it is leaving, and only a deque keeps that reference valid.
  mutable std::deque<State> states_;
  mutable std::map<StateSet, std::uint16_t> stateNumbers_;
  std::size_t maximumRuleLength_ = 0;

  std::uint64_t signature(std::uint64_t facts) const;
};

}  // namespace digitalkhatt::justify
