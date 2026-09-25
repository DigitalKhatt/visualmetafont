#include <digitalkhatt/justify/declpolicy/FixedSlotDfa.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace digitalkhatt::justify {
namespace {

constexpr std::uint16_t kDeadState = std::numeric_limits<std::uint16_t>::max();

template <typename T>
void sortAndUnique(std::vector<T>& values) {
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
}

}  // namespace

bool GlyphPredicate::matches(std::uint64_t facts) const {
  return (facts & allOf) == allOf && (anyOf == 0 || (facts & anyOf) != 0) &&
         (facts & noneOf) == 0;
}

FixedSlotDfa::FixedSlotDfa(std::vector<FixedSlotRule> rules) {
  nfa_.resize(1);  // state zero is the anchored start state

  for (const auto& rule : rules) {
    if (rule.pattern.empty()) {
      throw std::invalid_argument("a fixed-slot DFA rule cannot be empty");
    }
    maximumRuleLength_ = std::max(maximumRuleLength_, rule.pattern.size());

    std::uint16_t current = 0;
    for (const auto& slot : rule.pattern) {
      auto predicate = std::find(predicates_.begin(), predicates_.end(), slot);
      if (predicate == predicates_.end()) {
        predicates_.push_back(slot);
        predicate = std::prev(predicates_.end());
      }
      const auto predicateIndex = static_cast<std::uint16_t>(predicate - predicates_.begin());

      if (nfa_.size() >= kDeadState) {
        throw std::length_error("fixed-slot DFA has too many NFA states");
      }
      const auto next = static_cast<std::uint16_t>(nfa_.size());
      nfa_.push_back({});
      nfa_[current].edges.push_back({predicateIndex, next});
      current = next;
    }
    nfa_[current].accepts.push_back(rule.id);
  }

  // One bit per predicate in the glyph signature, and predicate indices are
  // stored per state as bytes.  There is no separate cap on how many of them a
  // single position may distinguish: a position wider than
  // kDenseBranchingLimit memoizes instead of enumerating.
  if (predicates_.size() > 64) {
    throw std::length_error("fixed-slot DFA supports at most 64 distinct slot predicates");
  }

  intern(StateSet{0});
}

FixedSlotDfa::StateSet FixedSlotDfa::successor(const StateSet& set,
                                               std::uint64_t firing) const {
  StateSet next;
  for (auto nfaState : set) {
    for (const auto& edge : nfa_[nfaState].edges) {
      if ((firing & (std::uint64_t{1} << edge.predicate)) != 0) {
        next.push_back(edge.target);
      }
    }
  }
  sortAndUnique(next);
  return next;
}

std::uint64_t FixedSlotDfa::firingMask(const State& state,
                                       std::uint64_t key) const {
  std::uint64_t firing = 0;
  for (std::size_t index = 0; index < state.relevant.size(); ++index) {
    if ((key & (std::uint64_t{1} << index)) != 0) {
      firing |= std::uint64_t{1} << state.relevant[index];
    }
  }
  return firing;
}

std::uint16_t FixedSlotDfa::intern(StateSet set) const {
  const auto existing = stateNumbers_.find(set);
  if (existing != stateNumbers_.end()) return existing->second;

  if (states_.size() >= kDeadState) {
    throw std::length_error("fixed-slot DFA has too many deterministic states");
  }
  const auto number = static_cast<std::uint16_t>(states_.size());
  stateNumbers_.emplace(set, number);
  // References into a deque survive the appends the recursion below makes.
  State& state = states_.emplace_back();
  state.nfaStates = std::move(set);

  for (auto nfaState : state.nfaStates) {
    const auto& source = nfa_[nfaState];
    state.accepts.insert(state.accepts.end(), source.accepts.begin(),
                         source.accepts.end());
    // Predicates this state actually branches on.  Any other predicate cannot
    // appear on an outgoing edge, so it cannot change where this state goes.
    for (const auto& edge : source.edges) {
      state.relevant.push_back(static_cast<std::uint8_t>(edge.predicate));
    }
  }
  sortAndUnique(state.accepts);
  sortAndUnique(state.relevant);

  // Too wide to enumerate: leave `targets` empty and let match() memoize the
  // combinations that real glyphs turn out to produce.
  if (state.relevant.size() > kDenseBranchingLimit) return number;

  const std::size_t combinations = std::size_t{1} << state.relevant.size();
  state.targets.assign(combinations, kDeadState);
  for (std::size_t key = 0; key < combinations; ++key) {
    auto next = successor(state.nfaStates, firingMask(state, key));
    if (next.empty()) continue;
    state.targets[key] = intern(std::move(next));
  }
  return number;
}

std::uint16_t FixedSlotDfa::memoizedStep(State& state,
                                         std::uint64_t key) const {
  const auto cached = state.memo.find(key);
  if (cached != state.memo.end()) return cached->second;

  auto next = successor(state.nfaStates, firingMask(state, key));
  // Dead ends are cached too, so a combination costs one subset step ever.
  const auto target = next.empty() ? kDeadState : intern(std::move(next));
  state.memo.emplace(key, target);
  return target;
}

std::size_t FixedSlotDfa::transitionCount() const {
  std::size_t total = 0;
  for (const auto& state : states_) total += state.targets.size() + state.memo.size();
  return total;
}

std::size_t FixedSlotDfa::maximumBranchingFactor() const {
  std::size_t largest = 0;
  for (const auto& state : states_) largest = std::max(largest, state.relevant.size());
  return largest;
}

std::size_t FixedSlotDfa::lazyStateCount() const {
  std::size_t total = 0;
  for (const auto& state : states_) total += state.targets.empty() ? 1 : 0;
  return total;
}

std::uint64_t FixedSlotDfa::signature(std::uint64_t facts) const {
  std::uint64_t result = 0;
  for (std::size_t index = 0; index < predicates_.size(); ++index) {
    if (predicates_[index].matches(facts)) result |= std::uint64_t{1} << index;
  }
  return result;
}

std::vector<FixedSlotMatch> FixedSlotDfa::match(std::span<const std::uint64_t> glyphFacts) const {
  std::vector<FixedSlotMatch> result;
  if (states_.empty()) return result;

  // Each glyph is scanned once per start position within a rule length, so the
  // predicates are evaluated once per glyph rather than once per visit.
  std::vector<std::uint64_t> signatures(glyphFacts.size());
  for (std::size_t index = 0; index < glyphFacts.size(); ++index) {
    signatures[index] = signature(glyphFacts[index]);
  }

  for (std::size_t first = 0; first < glyphFacts.size(); ++first) {
    std::uint16_t state = 0;
    const auto end = std::min(glyphFacts.size(), first + maximumRuleLength_);
    for (std::size_t cursor = first; cursor < end; ++cursor) {
      State& current = states_[state];
      std::uint64_t key = 0;
      for (std::size_t index = 0; index < current.relevant.size(); ++index) {
        if ((signatures[cursor] >> current.relevant[index]) & 1) {
          key |= std::uint64_t{1} << index;
        }
      }
      state = current.targets.empty() ? memoizedStep(current, key)
                                      : current.targets[key];
      if (state == kDeadState) break;
      for (auto ruleId : states_[state].accepts) result.push_back({ruleId, first, cursor});
    }
  }

  sortAndUnique(result);
  return result;
}

}  // namespace digitalkhatt::justify
