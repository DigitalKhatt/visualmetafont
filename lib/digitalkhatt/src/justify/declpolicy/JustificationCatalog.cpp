#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

#include "NativeParameterAttribute.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace digitalkhatt::justify {
namespace {

bool has(std::string_view name, std::string_view part) {
  return name.find(part) != std::string_view::npos;
}

JustExprOp exprOp(const std::string& name);
int operandCount(JustExprOp op);

// Specialize action calls once, before compiling the ordinary effect IR.
// Range arguments are assigned at the action's original update site, not at
// the start of preparation: later guards/reads must keep their current order.
void lowerActionArguments(JustificationDfaSource& source) {
  struct ActionTemplate {
    DfaActionDefSource definition;
    std::size_t index;
    std::vector<std::pair<std::vector<DfaEffectSource>, std::string>> instances;
  };
  std::unordered_map<std::string, ActionTemplate> definitions;
  for (std::size_t index = 0; index < source.actionDefinitions.size(); ++index) {
    const auto& definition = source.actionDefinitions[index];
    if (definition.name.empty()) throw std::invalid_argument("a justification action has no name");
    if (!definitions.emplace(definition.name, ActionTemplate{definition, index, {}}).second) throw std::invalid_argument("duplicate justification action " + definition.name);
    std::unordered_set<std::string> names;
    for (const auto& name : definition.parameters)
      if (name.empty() || !names.insert(name).second) throw std::invalid_argument("empty or duplicate action argument in " + definition.name);
  }
  for (const auto& binding : source.actions) {
    const auto found = definitions.find(binding.action);
    if (found != definitions.end() && !found->second.definition.parameters.empty()) throw std::invalid_argument("parameterized actions require an opportunity call: " + binding.action);
  }
  for (auto& opportunity : source.opportunities) {
    const auto owner = "opportunity " + opportunity.name;
    if (opportunity.action.empty()) {
      if (!opportunity.ranges.empty() || !opportunity.arguments.empty()) throw std::invalid_argument(owner + " requires an action call for named ranges or arguments");
      continue;
    }
    if (!opportunity.effects.empty()) throw std::invalid_argument(owner + " cannot combine an action reference and an inline body");
    const auto found = definitions.find(opportunity.action);
    if (found == definitions.end()) throw std::invalid_argument(owner + " references unknown action " + opportunity.action);
    auto& action = found->second;
    if (opportunity.arguments.size() != action.definition.parameters.size()) throw std::invalid_argument(owner + " has the wrong number of arguments for " + opportunity.action);
    std::unordered_map<std::string, std::size_t> rangeIds;
    for (std::size_t index = 0; index < opportunity.ranges.size(); ++index) {
      const auto& range = opportunity.ranges[index];
      if (range.name.empty() || range.bounds.preferred.empty() || !rangeIds.emplace(range.name, index).second) throw std::invalid_argument(owner + " has an empty, duplicate or incomplete named range");
    }
    if (action.definition.parameters.empty()) {
      if (!opportunity.ranges.empty()) throw std::invalid_argument(owner + " declares unused named ranges");
      continue;
    }
    std::vector<int> argumentRanges(opportunity.arguments.size(), -1);
    std::vector<bool> usedRanges(opportunity.ranges.size());
    for (std::size_t index = 0; index < opportunity.arguments.size(); ++index) {
      const auto& argument = opportunity.arguments[index];
      if (argument.size() == 1 && argument.front().op == "argument") {
        const auto range = rangeIds.find(argument.front().name);
        if (range == rangeIds.end()) throw std::invalid_argument(owner + " references unknown range " + argument.front().name);
        argumentRanges[index] = static_cast<int>(range->second);
      } else if (std::any_of(argument.begin(), argument.end(), [](const auto& node) { return node.op == "argument"; })) throw std::invalid_argument(owner + " must pass named ranges directly, not inside expressions");
    }
    const auto argumentId = [&](const std::string& name) {
      const auto& parameters = action.definition.parameters;
      const auto parameter = std::find(parameters.begin(), parameters.end(), name);
      if (parameter == parameters.end()) throw std::invalid_argument("unknown argument " + name + " in action " + action.definition.name);
      return static_cast<std::size_t>(parameter - parameters.begin());
    };
    const auto substitute = [&](const DfaExprSource& expression) {
      DfaExprSource result;
      for (const auto& node : expression) {
        if (node.op != "argument") result.push_back(node);
        else {
          const auto index = argumentId(node.name);
          if (argumentRanges[index] >= 0) throw std::invalid_argument("range argument " + node.name + " requires a direct or proportional update in " + action.definition.name);
          const auto& argument = opportunity.arguments[index];
          result.insert(result.end(), argument.begin(), argument.end());
        }
      }
      return result;
    };
    // A range may be multiplied by a scalar expression (including facts and
    // scalar arguments). It cannot participate in arbitrary nonlinear math or
    // depend on another range. Keep a single independent driver per range.
    const auto proportional = [&](const DfaExprSource& expression, auto&& self) -> std::optional<std::pair<std::size_t, DfaExprSource>> {
      if (expression.size() == 1 && expression.front().op == "argument") {
        const auto range = argumentRanges[argumentId(expression.front().name)];
        if (range >= 0) return {{static_cast<std::size_t>(range), {{.op = "literal", .literal = 1}}}};
      }
      if (expression.empty() || (expression.back().op != "mul" && expression.back().op != "prod")) {
        (void)substitute(expression); // Reject a range hidden in unsupported math.
        return std::nullopt;
      }
      std::size_t split = expression.size() - 1;
      int needed = 1;
      while (split > 0 && needed > 0) {
        const auto& node = expression[--split];
        needed += (node.op == "argument" ? 0 : operandCount(exprOp(node.op))) - 1;
      }
      if (needed != 0 || split == 0) throw std::invalid_argument("unbalanced proportional argument in " + action.definition.name);
      const DfaExprSource left(expression.begin(), expression.begin() + split), right(expression.begin() + split, expression.end() - 1);
      auto a = self(left, self), b = self(right, self);
      if (a && b) throw std::invalid_argument("a derived output cannot multiply two ranges in " + action.definition.name);
      if (!a && !b) return std::nullopt;
      auto derived = a ? std::move(*a) : std::move(*b);
      const auto factor = substitute(a ? right : left);
      derived.second.insert(derived.second.end(), factor.begin(), factor.end());
      derived.second.push_back({.op = "mul"});
      return derived;
    };
    using Driver = std::pair<DfaTargetSource, std::string>;
    std::vector<std::optional<Driver>> drivers(opportunity.ranges.size());
    const auto expand = [&](const std::vector<DfaEffectSource>& effects, auto& activeDrivers, auto&& self) -> std::vector<DfaEffectSource> {
      std::vector<DfaEffectSource> result;
      for (const auto& effect : effects) {
        for (const auto& driver : activeDrivers) {
          if (!driver || driver->first != effect.target) continue;
          if (effect.kind == "clear" || effect.kind == "replace" || ((effect.kind == "update" || effect.kind == "add" || effect.kind == "vary") && driver->second == effect.attribute)) throw std::invalid_argument("an independent range output cannot be overwritten in " + action.definition.name);
        }
        if (effect.kind == "update") {
          if (auto derived = proportional(effect.value, proportional)) {
            const auto rangeIndex = derived->first;
            const auto& bounds = opportunity.ranges[rangeIndex].bounds;
            usedRanges[rangeIndex] = true;
            if (activeDrivers[rangeIndex]) {
              const auto& driver = *activeDrivers[rangeIndex];
              result.push_back({.kind = "derive", .target = effect.target, .attribute = effect.attribute, .value = std::move(derived->second), .driver = driver.first, .driverAttribute = driver.second});
            } else {
              // The first assignment defines the independent output; scaled
              // assignments are only valid after that direct assignment.
              if (effect.value.size() != 1) throw std::invalid_argument("assign the independent range before its derived outputs in " + action.definition.name);
              activeDrivers[rangeIndex] = Driver{effect.target, effect.attribute};
              if (!bounds.minimum.empty()) result.push_back({.kind = "update", .target = effect.target, .attribute = effect.attribute, .value = bounds.minimum});
              result.push_back({.kind = "vary", .target = effect.target, .attribute = effect.attribute, .value = bounds.preferred, .maximum = bounds.maximum});
            }
            continue;
          }
        }
        auto compiled = effect;
        compiled.value = substitute(effect.value);
        compiled.maximum = substitute(effect.maximum);
        compiled.condition = substitute(effect.condition);
        for (auto& write : compiled.writes) write.value = substitute(write.value);
        auto nestedDrivers = activeDrivers;
        compiled.nested = self(effect.nested, nestedDrivers, self);
        result.push_back(std::move(compiled));
      }
      return result;
    };
    auto effects = expand(action.definition.effects, drivers, expand);
    if (std::find(usedRanges.begin(), usedRanges.end(), false) != usedRanges.end()) throw std::invalid_argument(owner + " declares an unused named range");
    const auto existing = std::find_if(action.instances.begin(), action.instances.end(), [&](const auto& instance) { return instance.first == effects; });
    if (existing != action.instances.end()) opportunity.action = existing->second;
    else {
      const auto name = action.instances.empty() ? action.definition.name : "$call." + opportunity.name;
      if (action.instances.empty()) source.actionDefinitions[action.index] = {name, effects};
      else source.actionDefinitions.push_back({name, effects});
      action.instances.emplace_back(std::move(effects), name);
      opportunity.action = name;
    }
    opportunity.arguments.clear();
    opportunity.ranges.clear();
  }
  // Uncalled templates have no runtime work; no unresolved argument reaches
  // the evaluator. Called templates were replaced in place above.
  std::erase_if(source.actionDefinitions, [](const auto& definition) { return !definition.parameters.empty(); });
}

bool formMatches(std::string_view form, bool initial, bool medial, bool final,
                 bool isolated) {
  if (form.empty()) return true;  // the fact does not constrain form
  if (form == "init") return initial;
  if (form == "medi") return medial;
  if (form == "fina") return final;
  if (form == "isol") return isolated;
  if (form == "final") return final;
  if (form == "joins_right") return initial || medial;
  if (form == "joins_left") return medial || final;
  if (form == "terminal") return final || isolated;
  // A mark carries none of the four form substrings.  Spelled "nonspacing"
  // because "mark" is already token MARK in the feature-file lexer.
  if (form == "nonspacing") return !initial && !medial && !final && !isolated;
  throw std::invalid_argument("unknown justification glyph form " +
                              std::string(form));
}

bool positionMatches(std::string_view position, bool firstInSubword,
                     bool lastInSubword) {
  if (position.empty()) return true;  // the fact does not constrain position
  if (position == "first") return firstInSubword;
  if (position == "last") return lastInSubword;
  throw std::invalid_argument("unknown justification glyph position " +
                              std::string(position));
}

void validateFactDefinition(const DfaFactSource& fact, const std::string& label) {
  (void)formMatches(fact.form, false, false, false, true);
  (void)positionMatches(fact.position, false, false);
  if (fact.form.empty() && fact.position.empty() && fact.sourceCharacters.empty() && fact.glyphSetRef < 0) throw std::invalid_argument("justification fact " + label + " constrains nothing and would match every glyph");
  for (const auto character : fact.sourceCharacters) {
    if (character < 0 || character > 0xffff) throw std::invalid_argument("invalid source character in justification fact " + label);
  }
}

GlyphPredicate any(std::uint64_t facts) {
  return GlyphPredicate{.anyOf = facts};
}

JustExprOp exprOp(const std::string& name) {
  static const std::unordered_map<std::string, JustExprOp> operators{
      {"literal", JustExprOp::Literal},
      {"attribute", JustExprOp::Attribute},
      {"present", JustExprOp::Present},
      {"fact", JustExprOp::Fact},
      {"underfull", JustExprOp::Underfull},
      {"underfull_percent", JustExprOp::UnderfullPercent},
      {"initial_underfull", JustExprOp::InitialUnderfull},
      {"initial_underfull_percent", JustExprOp::InitialUnderfullPercent},
      {"stretched", JustExprOp::Stretched},
      {"stretched_percent", JustExprOp::StretchedPercent},
      {"add", JustExprOp::Add},
      {"sub", JustExprOp::Sub},
      {"mul", JustExprOp::Mul},
      {"floordiv", JustExprOp::FloorDiv},
      // Spellings usable from a feature file: "add" always lexes as token ADD
      // there, so an expression cannot call it by that name.
      {"sum", JustExprOp::Add},
      {"diff", JustExprOp::Sub},
      {"prod", JustExprOp::Mul},
      {"select", JustExprOp::Select},
      {"min", JustExprOp::Min},
      {"max", JustExprOp::Max},
      {"gt", JustExprOp::Gt},
      {"lt", JustExprOp::Lt},
      {"ge", JustExprOp::Ge},
      {"le", JustExprOp::Le},
      {"eq", JustExprOp::Eq},
      {"ne", JustExprOp::Ne},
      {"and", JustExprOp::And},
      {"or", JustExprOp::Or},
      {"not", JustExprOp::Not},
  };
  const auto found = operators.find(name);
  if (found == operators.end()) {
    throw std::invalid_argument("unknown justification expression operator " +
                                name);
  }
  return found->second;
}

// How many operands the operator consumes.  Used to prove at compile time that
// an expression leaves exactly one value on the stack.
int operandCount(JustExprOp op) {
  switch (op) {
    case JustExprOp::Literal:
    case JustExprOp::Attribute:
    case JustExprOp::Present:
    case JustExprOp::Fact:
    case JustExprOp::Underfull:
    case JustExprOp::UnderfullPercent:
    case JustExprOp::InitialUnderfull:
    case JustExprOp::InitialUnderfullPercent:
    case JustExprOp::Stretched:
    case JustExprOp::StretchedPercent:
      return 0;
    case JustExprOp::Not:
      return 1;
    case JustExprOp::Select:
      return 3;
    default:
      return 2;
  }
}

MatchDirection matchDirection(std::string_view name) {
  if (name == "first") return MatchDirection::LogicalFirst;
  if (name == "last") return MatchDirection::LogicalLast;
  throw std::invalid_argument("unknown match direction " + std::string(name));
}

MatchMultiplicity matchMultiplicity(std::string_view name) {
  if (name == "all") return MatchMultiplicity::All;
  if (name == "selected_only") return MatchMultiplicity::SelectedOnly;
  if (name == "all_non_overlapping")
    return MatchMultiplicity::AllNonOverlapping;
  throw std::invalid_argument("unknown match multiplicity " +
                              std::string(name));
}

PolicyPhase::Allocator phaseAllocator(std::string_view name) {
  if (name.empty() || name == "fixed_steps") return PolicyPhase::Allocator::FixedSteps;
  if (name == "candidate_pool") return PolicyPhase::Allocator::CandidatePool;
  if (name == "baseline_pool") return PolicyPhase::Allocator::BaselinePool;
  throw std::invalid_argument("unknown justification phase allocator " + std::string(name));
}

CandidateWidthMode candidateWidthMode(std::string_view name) {
  if (name.empty() || name == "advance") return CandidateWidthMode::Advance;
  if (name == "full_shape") return CandidateWidthMode::FullShape;
  throw std::invalid_argument("unknown candidate width measurement " +
                              std::string(name));
}

}  // namespace

std::uint64_t glyphFacts(const GlyphFactInput& glyph,
                         std::span<const DfaFactSource> sourceFacts) {
  if (sourceFacts.size() > 64) {
    throw std::invalid_argument(
        "a declarative-policy catalog cannot contain more than 64 facts");
  }
  const bool initial = has(glyph.glyphName, ".init");
  const bool medial = has(glyph.glyphName, ".medi");
  const bool final = has(glyph.glyphName, ".fina");
  const bool isolated = has(glyph.glyphName, ".isol");
  std::uint64_t result = 0;
  for (std::size_t index = 0; index < sourceFacts.size(); ++index) {
    const auto& fact = sourceFacts[index];
    // Every discriminator the fact declares must hold.
    if (!formMatches(fact.form, initial, medial, final, isolated)) continue;
    if (!positionMatches(fact.position, glyph.firstInSubword,
                         glyph.lastInSubword)) {
      continue;
    }
    if (!fact.sourceCharacters.empty() &&
        std::find(fact.sourceCharacters.begin(), fact.sourceCharacters.end(),
                  static_cast<int>(glyph.sourceCharacter)) ==
            fact.sourceCharacters.end()) {
      continue;
    }
    // glyphNames is kept sorted by resolveJustificationGlyphSets.
    if (!fact.glyphNames.empty() &&
        !std::binary_search(fact.glyphNames.begin(), fact.glyphNames.end(),
                            glyph.glyphName)) {
      continue;
    }
    result |= std::uint64_t{1} << index;
  }
  return result;
}

namespace {

std::vector<std::string> expandGlyphSet(
    const JustificationGlyphSetResolver& resolve, int glyphSetRef,
    const std::string& owner) {
  auto names = resolve(glyphSetRef);
  if (names.empty()) {
    throw std::invalid_argument(owner + " has an empty glyph set");
  }
  std::sort(names.begin(), names.end());
  names.erase(std::unique(names.begin(), names.end()), names.end());
  return names;
}

}  // namespace

void resolveJustificationGlyphSets(
    CompiledJustificationCatalog& catalog,
    const JustificationGlyphSetResolver& resolve) {
  for (auto& fact : catalog.facts) {
    if (fact.glyphSetRef < 0) continue;
    fact.glyphNames =
        expandGlyphSet(resolve, fact.glyphSetRef, "justification fact " + fact.name);
  }
  for (auto& attachment : catalog.attachments) {
    for (auto& response : attachment.responses)
      if (response.glyphSetRef >= 0) response.glyphNames = expandGlyphSet(resolve, response.glyphSetRef, "mark response on " + attachment.name);
    if (attachment.findGlyphSetRef >= 0) {
      attachment.findGlyphNames =
          expandGlyphSet(resolve, attachment.findGlyphSetRef,
                         "justification attachment " + attachment.name);
    }
    if (attachment.skipGlyphSetRef >= 0) {
      attachment.skipGlyphNames =
          expandGlyphSet(resolve, attachment.skipGlyphSetRef,
                         "justification attachment " + attachment.name);
    }
    if (attachment.excludeMarkGlyphSetRef >= 0) attachment.excludeMarkGlyphNames = expandGlyphSet(resolve, attachment.excludeMarkGlyphSetRef, "excluded marks on " + attachment.name);
    if (attachment.followingMarkGlyphSetRef >= 0) attachment.followingMarkGlyphNames = expandGlyphSet(resolve, attachment.followingMarkGlyphSetRef, "following marks on " + attachment.name);
  }
}

std::vector<ResolvedAttachmentSubstitution> resolveAttachmentSubstitutions(const CompiledJustificationCatalog& catalog, const AttachmentGlyphResolver& resolve) {
  std::vector<ResolvedAttachmentSubstitution> result;
  for (std::size_t index = 0; index < catalog.attachments.size(); ++index) {
    const auto& attachment = catalog.attachments[index];
    if (!attachment.substitution) continue;
    const auto& substitution = *attachment.substitution;
    const auto glyph = resolve(substitution.glyph), otherwiseGlyph = resolve(substitution.otherwiseGlyph);
    if (!glyph || !otherwiseGlyph || *glyph == *otherwiseGlyph) throw std::invalid_argument("attachment substitution requires two distinct font glyphs: " + attachment.name);
    for (const auto& previous : result)
      if (previous.matches(*glyph) || previous.matches(*otherwiseGlyph)) throw std::invalid_argument("overlapping attachment substitution families: " + attachment.name);
    result.push_back({static_cast<JustAttachmentId>(index), substitution.axis, substitution.threshold, *glyph, *otherwiseGlyph});
  }
  return result;
}

CompiledJustificationCatalog compileJustificationCatalog(
    const JustificationDfaSource& input, const GlyphAxisRegistry& axes) {
  // Opportunities are the authoring model. Lower them once to the existing
  // DFA/effect IR so matching, fixed steps and continuation share one engine.
  auto source = input;
  lowerActionArguments(source);
  std::unordered_map<std::string, const DfaPatternSource*> patterns;
  for (const auto& pattern : source.patterns) {
    if (pattern.name.empty() || pattern.slots.empty()) throw std::invalid_argument("a named justification pattern requires a name and nonempty slots");
    if (!patterns.emplace(pattern.name, &pattern).second) throw std::invalid_argument("duplicate justification pattern " + pattern.name);
    for (const auto& slot : pattern.slots) {
      if (slot.inlineFact) {
        if (!slot.fact.empty()) throw std::invalid_argument("named pattern " + pattern.name + " combines named and inline slot facts");
        validateFactDefinition(*slot.inlineFact, "in pattern " + pattern.name);
      } else if (std::none_of(source.facts.begin(), source.facts.end(), [&](const auto& fact) { return fact.name == slot.fact; })) throw std::invalid_argument("pattern " + pattern.name + " references unknown fact " + slot.fact);
    }
  }
  if (!source.opportunities.empty() && (!source.rules.empty() || !source.actions.empty())) throw std::invalid_argument("cannot mix opportunities with low-level DFA rules/bindings");
  for (const auto& opportunity : source.opportunities) {
    if (!opportunity.action.empty() && !opportunity.effects.empty()) throw std::invalid_argument("opportunity " + opportunity.name + " cannot combine an action reference and an inline body");
    const auto* slots = &opportunity.pattern;
    if (!opportunity.patternName.empty()) {
      if (!slots->empty()) throw std::invalid_argument("opportunity " + opportunity.name + " cannot combine named and inline patterns");
      const auto found = patterns.find(opportunity.patternName);
      if (found == patterns.end()) throw std::invalid_argument("opportunity " + opportunity.name + " references unknown pattern " + opportunity.patternName);
      slots = &found->second->slots;
    }
    DfaRuleSource rule{opportunity.name, {}};
    for (const auto& slot : *slots) {
      if (slot.inlineFact) {
        if (!slot.fact.empty()) throw std::invalid_argument("opportunity slot cannot combine named and inline facts");
        auto fact = *slot.inlineFact;
        fact.name.clear();
        const auto found = std::find_if(source.facts.begin(), source.facts.end(), [&](auto other) { other.name.clear(); return other == fact; });
        if (found != source.facts.end()) rule.pattern.push_back(found->name);
        else {
          fact.name = "$inline." + opportunity.name + "." + std::to_string(rule.pattern.size() + 1);
          rule.pattern.push_back(fact.name);
          source.facts.push_back(std::move(fact));
        }
      } else rule.pattern.push_back(slot.fact);
    }
    source.rules.push_back(std::move(rule));
    auto action = opportunity.action;
    if (action.empty()) {
      action = "$inline." + opportunity.name;
      source.actionDefinitions.push_back({action, opportunity.effects});
    }
    source.actions.push_back({opportunity.name, std::move(action)});
  }
  if (source.name.empty()) {
    throw std::invalid_argument("justification catalog has no name");
  }

  CompiledJustificationCatalog result;
  result.name = source.name;
  result.facts = source.facts;
  const bool usesDfaStages = !source.stages.empty();
  if ((usesDfaStages && result.facts.empty()) || result.facts.size() > 64) {
    throw std::invalid_argument(
        "declarative-policy catalogs with glyph stages require between 1 and 64 facts");
  }

  std::unordered_map<std::string, std::uint64_t> factBits;
  for (std::size_t index = 0; index < result.facts.size(); ++index) {
    const auto& fact = result.facts[index];
    if (fact.name.empty())
      throw std::invalid_argument("a justification fact has no name");
    // Validate the discriminator vocabulary now rather than per glyph.
    validateFactDefinition(fact, fact.name);
    if (!factBits.emplace(fact.name, std::uint64_t{1} << index).second) {
      throw std::invalid_argument("duplicate justification fact " + fact.name);
    }
  }

  if (usesDfaStages && source.rules.empty())
    throw std::invalid_argument("justification catalog has no rules");
  if (source.rules.size() >
      std::numeric_limits<JustificationRuleId>::max()) {
    throw std::invalid_argument("justification catalog has too many rules");
  }

  std::unordered_map<std::string, JustificationRuleId> ruleIds;
  for (std::size_t index = 0; index < source.rules.size(); ++index) {
    const auto& rule = source.rules[index];
    const auto id = static_cast<JustificationRuleId>(index + 1);
    if (rule.name.empty())
      throw std::invalid_argument("a justification rule has no name");
    if (!ruleIds.emplace(rule.name, id).second)
      throw std::invalid_argument("duplicate justification rule " + rule.name);
    if (rule.pattern.empty())
      throw std::invalid_argument("rule " + rule.name + " has no slots");

    FixedSlotRule compiled{.id = id, .name = rule.name};
    for (const auto& slot : rule.pattern) {
      const auto found = factBits.find(slot);
      if (found == factBits.end()) {
        throw std::invalid_argument("rule " + rule.name +
                                    " references unknown fact " + slot);
      }
      compiled.pattern.push_back(any(found->second));
    }
    result.rules.push_back(std::move(compiled));
  }

  // Attachments -------------------------------------------------------------
  std::unordered_map<std::string, int> attachmentIds;
  for (const auto& attachment : source.attachments) {
    if (attachment.name.empty()) {
      throw std::invalid_argument("a justification attachment has no name");
    }
    if (attachment.within < 1 || attachment.within > 255) {
      throw std::invalid_argument("justification attachment " +
                                  attachment.name +
                                  " needs a within of 1..255");
    }
    if (attachment.find.empty() && attachment.findGlyphSetRef < 0) {
      throw std::invalid_argument("justification attachment " +
                                  attachment.name + " finds nothing");
    }
    // Characters and glyphs scan different domains; mixing them on one side
    // would silently mean whichever the runtime happened to test first.
    if (!attachment.find.empty() && attachment.findGlyphSetRef >= 0) {
      throw std::invalid_argument("justification attachment " +
                                  attachment.name +
                                  " finds both characters and glyphs");
    }
    if (!attachment.skip.empty() && attachment.skipGlyphSetRef >= 0) {
      throw std::invalid_argument("justification attachment " +
                                  attachment.name +
                                  " skips both characters and glyphs");
    }
    if (!attachmentIds
             .emplace(attachment.name,
                      static_cast<int>(result.attachments.size()))
             .second) {
      throw std::invalid_argument("duplicate justification attachment " +
                                  attachment.name);
    }
    const auto widthAxis = attachment.widthAxis.empty() ? NoGlyphAxis : decl::parameterAttributeAxis(axes, attachment.widthAxis);
    if (!attachment.widthAxis.empty() && (widthAxis == NoGlyphAxis || !std::isfinite(attachment.gain) || attachment.gain <= 0 || !std::isfinite(attachment.maximumWidth) || attachment.maximumWidth <= 0 || attachment.widthGrid == 0 || attachment.widthGrid > 256))
      throw std::invalid_argument("invalid dependent width response on attachment " + attachment.name);
    if (attachment.widthAxis.empty() && (attachment.gain != 0 || attachment.maximumWidth != 0))
      throw std::invalid_argument("attachment width response requires width_axis: " + attachment.name);
    if (widthAxis == NoGlyphAxis && (!attachment.growthAxes.empty() || !attachment.growthConnection.empty() || attachment.alignment != "unchanged")) throw std::invalid_argument("default mark growth/alignment requires width_axis: " + attachment.name);
    if (!attachment.growthConnection.empty() && (!attachment.growthAxes.empty() || attachment.growthConnection.size() != 2)) throw std::invalid_argument("growth_connection requires exactly two axes and cannot be combined with growth_axes: " + attachment.name);
    result.attachments.push_back({.name = attachment.name,
                                  .within = attachment.within,
                                  .skip = attachment.skip,
                                  .find = attachment.find,
                                  .skipGlyphSetRef = attachment.skipGlyphSetRef,
                                  .findGlyphSetRef = attachment.findGlyphSetRef,
                                  .skipGlyphNames = attachment.skipGlyphNames,
                                  .findGlyphNames = attachment.findGlyphNames,
                                  .widthAxis = widthAxis,
                                  .gain = attachment.gain,
                                  .maximumWidth = attachment.maximumWidth,
                                  .widthGrid = attachment.widthGrid});
    auto& compiledAttachment = result.attachments.back();
    if (!std::isfinite(attachment.followingGrowthScale) || attachment.followingGrowthScale < 0 || attachment.followingGrowthScale > 1 ||
        ((attachment.followingMarkGlyphSetRef >= 0 || !attachment.followingMarkGlyphNames.empty()) && widthAxis == NoGlyphAxis) ||
        (attachment.followingGrowthScale != 1 && attachment.followingMarkGlyphSetRef < 0 && attachment.followingMarkGlyphNames.empty()))
      throw std::invalid_argument("following mark growth_scale requires width_axis, following glyphs and a finite value in [0,1]: " + attachment.name);
    compiledAttachment.followingMarkGlyphSetRef = attachment.followingMarkGlyphSetRef;
    compiledAttachment.followingMarkGlyphNames = attachment.followingMarkGlyphNames;
    std::sort(compiledAttachment.followingMarkGlyphNames.begin(), compiledAttachment.followingMarkGlyphNames.end());
    compiledAttachment.followingGrowthScale = attachment.followingGrowthScale;
    if (attachment.substitution) {
      const auto& substitution = *attachment.substitution;
      const auto axis = axes.find(substitution.axis);
      if (axis == NoGlyphAxis || !std::isfinite(substitution.threshold) || substitution.glyph.empty() || substitution.otherwiseGlyph.empty() || substitution.glyph == substitution.otherwiseGlyph)
        throw std::invalid_argument("invalid parameter-dependent attachment substitution: " + attachment.name);
      compiledAttachment.substitution = JustAttachmentSubstitution{substitution.glyph, axis, substitution.threshold, substitution.otherwiseGlyph};
    }
    if (attachment.during == "stretch") compiledAttachment.during = AttachmentDuring::Stretch;
    else if (attachment.during == "shrink") compiledAttachment.during = AttachmentDuring::Shrink;
    else if (attachment.during != "both") throw std::invalid_argument("unknown attachment operation " + attachment.during);
    compiledAttachment.excludeMarkGlyphSetRef = attachment.excludeMarkGlyphSetRef;
    compiledAttachment.excludeMarkGlyphNames = attachment.excludeMarkGlyphNames;
    std::sort(compiledAttachment.excludeMarkGlyphNames.begin(), compiledAttachment.excludeMarkGlyphNames.end());
    std::erase_if(compiledAttachment.excludeMarkGlyphNames, [](const auto& name) { return name.empty(); });
    compiledAttachment.excludeMarkGlyphNames.erase(std::unique(compiledAttachment.excludeMarkGlyphNames.begin(), compiledAttachment.excludeMarkGlyphNames.end()), compiledAttachment.excludeMarkGlyphNames.end());
    for (const auto& name : attachment.growthAxes) {
      const auto axis = axes.find(name);
      if (axis == NoGlyphAxis || std::find(compiledAttachment.growthAxes.begin(), compiledAttachment.growthAxes.end(), axis) != compiledAttachment.growthAxes.end()) throw std::invalid_argument("unknown or duplicate mark growth axis " + name);
      compiledAttachment.growthAxes.push_back(axis);
    }
    for (const auto& name : attachment.growthConnection) {
      const auto axis = axes.find(name);
      if (axis == NoGlyphAxis) throw std::invalid_argument("unknown mark connection axis " + name);
      compiledAttachment.growthConnection.push_back(axis);
    }
    if (attachment.alignment != "unchanged") throw std::invalid_argument("attachment positioning belongs in font GPOS anchors, not align: " + attachment.name);
    for (const auto& response : attachment.responses) {
      if (response.glyphSetRef < 0 && response.glyphNames.empty()) throw std::invalid_argument("mark response requires base glyphs: " + attachment.name);
      JustMarkResponse compiled{.glyphSetRef = response.glyphSetRef, .glyphNames = response.glyphNames};
      if (response.mode == "keep") compiled.mode = MarkResponseMode::Keep;
      else if (response.mode == "width_ratio") compiled.mode = MarkResponseMode::WidthRatio;
      else if (response.mode == "growth_ratio") compiled.mode = MarkResponseMode::GrowthRatio;
      else if (response.mode == "parameter") compiled.mode = MarkResponseMode::Parameter;
      else throw std::invalid_argument("unknown mark response mode " + response.mode);
      if (compiled.mode == MarkResponseMode::WidthRatio || compiled.mode == MarkResponseMode::GrowthRatio) {
        if (widthAxis == NoGlyphAxis) throw std::invalid_argument("mark width response requires width_axis: " + attachment.name);
        if (!std::isfinite(response.ratio) || response.ratio <= 0) throw std::invalid_argument("mark response ratio must be positive");
      }
      if (compiled.mode == MarkResponseMode::Parameter) {
        compiled.parameterAxis = axes.find(response.parameterAxis);
        if (compiled.parameterAxis == NoGlyphAxis || !std::isfinite(response.parameterValue)) throw std::invalid_argument("mark parameter response requires a native axis and a finite value: " + attachment.name);
        compiled.parameterValue = response.parameterValue;
      }
      compiled.ratio = response.ratio;
      if (response.alignment != "unchanged" || response.lowerGrowth) throw std::invalid_argument("attachment positioning belongs in font GPOS anchors, not align/lower_growth: " + attachment.name);
      result.attachments.back().responses.push_back(std::move(compiled));
    }
  }

  // Action definitions ------------------------------------------------------
  std::unordered_map<std::string, JustAttributeId> attributeIds;
  const auto attributeId = [&](const std::string& tag) {
    // Native parameters are not truncated into OpenType feature tags.
    const auto axis = decl::parameterAttributeAxis(axes, tag);
    if (tag.size() != 4 && axis == NoGlyphAxis) {
      throw std::invalid_argument(
          "justification attribute " + tag +
          " is neither a four-character tag nor a glyph parameter");
    }
    const auto found = attributeIds.find(tag);
    if (found != attributeIds.end()) return found->second;
    if (result.attributes.size() >
        std::numeric_limits<JustAttributeId>::max()) {
      throw std::invalid_argument("too many justification attributes");
    }
    const auto id = static_cast<JustAttributeId>(result.attributes.size());
    result.attributes.push_back(tag);
    result.attributeAxes.push_back(axis);
    attributeIds.emplace(tag, id);
    return id;
  };

  const auto writableAttributeId = [&](const std::string& tag) {
    const auto id = attributeId(tag);
    if (result.attributeAxes[id] != NoGlyphAxis && decl::isFeatureParameterQuery(tag))
      throw std::invalid_argument("feature parameter query " + tag + " is read-only");
    return id;
  };

  const auto compileTarget = [&](const DfaTargetSource& target,
                                 const std::string& actionName) {
    JustTarget compiled;
    // "$self" is -1; "$1" is the first slot.
    compiled.slot = target.slot < 0 ? -1 : target.slot - 1;
    if (target.slot == 0) {
      throw std::invalid_argument("justification action " + actionName +
                                  " uses slot 0; slots are numbered from 1");
    }
    if (!target.attachment.empty()) {
      const auto found = attachmentIds.find(target.attachment);
      if (found == attachmentIds.end()) {
        throw std::invalid_argument("justification action " + actionName +
                                    " references unknown attachment " +
                                    target.attachment);
      }
      compiled.attachment = found->second;
    }
    return compiled;
  };

  const auto compileExpr = [&](const DfaExprSource& expression,
                               const std::string& actionName) {
    JustExpr compiled;
    compiled.reserve(expression.size());
    int depth = 0;
    for (const auto& node : expression) {
      JustExprNode out;
      out.op = exprOp(node.op);
      out.target = compileTarget(node.target, actionName);
      out.literal = node.literal;
      if (out.op == JustExprOp::Attribute || out.op == JustExprOp::Present) {
        out.attribute = attributeId(node.name);
      } else if (out.op == JustExprOp::Fact) {
        const auto found = factBits.find(node.name);
        if (found == factBits.end()) {
          throw std::invalid_argument("justification action " + actionName +
                                      " references unknown fact " + node.name);
        }
        out.factMask = found->second;
      }
      depth -= operandCount(out.op);
      if (depth < 0) {
        throw std::invalid_argument("justification action " + actionName +
                                    " has an expression that underflows");
      }
      depth += 1;
      compiled.push_back(std::move(out));
    }
    if (depth != 1) {
      throw std::invalid_argument("justification action " + actionName +
                                  " has an expression that does not produce "
                                  "exactly one value");
    }
    return compiled;
  };

  std::unordered_map<std::string, int> actionDefIds;
  for (const auto& definition : source.actionDefinitions) {
    if (definition.name.empty()) {
      throw std::invalid_argument("a justification action has no name");
    }
    if (!actionDefIds
             .emplace(definition.name,
                      static_cast<int>(result.actionDefinitions.size()))
             .second) {
      throw std::invalid_argument("duplicate justification action " +
                                  definition.name);
    }
    JustificationAction compiled;
    compiled.name = definition.name;
    bool wrote = false;
    const auto compileEffects =
        [&](const std::vector<DfaEffectSource>& effects, auto&& self)
        -> std::vector<JustEffect> {
      std::vector<JustEffect> out;
      for (const auto& effect : effects) {
        JustEffect compiledEffect;
        if (effect.kind == "forbid") {
          compiledEffect.kind = JustEffectKind::Forbid;
          // Guards observe committed state only because they all run before
          // any write; enforce that rather than leaving it to the author.
          if (wrote) {
            throw std::invalid_argument(
                "justification action " + definition.name +
                " has a guard after a write; guards must come first");
          }
          compiledEffect.condition =
              compileExpr(effect.condition, definition.name);
          out.push_back(std::move(compiledEffect));
          continue;
        }
        if (effect.kind == "when") {
          // A conditional block counts as a write: it may contain one, and a
          // guard placed after it could no longer be read as unconditional.
          wrote = true;
          compiledEffect.kind = JustEffectKind::When;
          compiledEffect.condition =
              compileExpr(effect.condition, definition.name);
          if (effect.nested.empty()) {
            throw std::invalid_argument("justification action " +
                                        definition.name +
                                        " has an empty when block");
          }
          compiledEffect.nested = self(effect.nested, self);
          out.push_back(std::move(compiledEffect));
          continue;
        }
        wrote = true;
        compiledEffect.target = compileTarget(effect.target, definition.name);
        if (effect.kind == "add") {
          compiledEffect.kind = JustEffectKind::Add;
          compiledEffect.attribute = writableAttributeId(effect.attribute);
          compiledEffect.value = compileExpr(effect.value, definition.name);
          if (effect.clamp <= 0) {
            throw std::invalid_argument("justification action " +
                                        definition.name +
                                        " has an add without a positive clamp");
          }
          compiledEffect.clamp = effect.clamp;
        } else if (effect.kind == "update") {
          compiledEffect.kind = JustEffectKind::Update;
          compiledEffect.attribute = writableAttributeId(effect.attribute);
          compiledEffect.value = compileExpr(effect.value, definition.name);
        } else if (effect.kind == "clear") {
          compiledEffect.kind = JustEffectKind::Clear;
        } else if (effect.kind == "lookup") {
          compiledEffect.kind = JustEffectKind::Lookup;
          if (effect.lookup.empty()) throw std::invalid_argument("justification action " + definition.name + " has an unnamed lookup effect");
          compiledEffect.lookup = effect.lookup;
        } else if (effect.kind == "vary") {
          compiledEffect.kind = JustEffectKind::Vary;
          compiledEffect.attribute = writableAttributeId(effect.attribute);
          if (result.attributeAxes[compiledEffect.attribute] == NoGlyphAxis) {
            throw std::invalid_argument("justification action " + definition.name +
                                        " varies OpenType feature " + effect.attribute +
                                        "; candidate ranges require a native glyph parameter");
          }
          compiledEffect.value = compileExpr(effect.value, definition.name);
          if (!effect.maximum.empty()) compiledEffect.maximum = compileExpr(effect.maximum, definition.name);
        } else if (effect.kind == "derive") {
          compiledEffect.kind = JustEffectKind::Derive;
          compiledEffect.attribute = writableAttributeId(effect.attribute);
          compiledEffect.driver = compileTarget(effect.driver, definition.name);
          compiledEffect.driverAttribute = writableAttributeId(effect.driverAttribute);
          if (result.attributeAxes.at(compiledEffect.attribute) == NoGlyphAxis || result.attributeAxes.at(compiledEffect.driverAttribute) == NoGlyphAxis) throw std::invalid_argument("derived outputs require native glyph parameters in " + definition.name);
          compiledEffect.value = compileExpr(effect.value, definition.name);
        } else if (effect.kind == "replace") {
          compiledEffect.kind = JustEffectKind::Replace;
          if (effect.writes.empty()) {
            throw std::invalid_argument("justification action " +
                                        definition.name +
                                        " has a replace that writes nothing");
          }
          for (const auto& write : effect.writes) {
            compiledEffect.writes.push_back(
                {.attribute = writableAttributeId(write.attribute),
                 .value = compileExpr(write.value, definition.name)});
          }
        } else {
          throw std::invalid_argument("unknown justification effect " +
                                      effect.kind);
        }
        out.push_back(std::move(compiledEffect));
      }
      return out;
    };
    compiled.effects = compileEffects(definition.effects, compileEffects);
    if (compiled.effects.empty()) {
      throw std::invalid_argument("justification action " + definition.name +
                                  " does nothing");
    }
    result.actionDefinitions.push_back(std::move(compiled));
  }

  std::unordered_set<JustificationRuleId> rulesWithActions;
  for (const auto& action : source.actions) {
    const auto rule = ruleIds.find(action.rule);
    if (rule == ruleIds.end())
      throw std::invalid_argument("action references undefined rule " +
                                  action.rule);
    if (!rulesWithActions.insert(rule->second).second) {
      throw std::invalid_argument("duplicate action for justification rule " +
                                  action.rule);
    }
    const auto declared = actionDefIds.find(action.action);
    if (declared == actionDefIds.end())
      throw std::invalid_argument("bind references undeclared action " +
                                  action.action);
    result.actions.push_back(
        {.rule = rule->second, .definition = declared->second});
  }
  for (const auto& [name, id] : ruleIds) {
    if (!rulesWithActions.contains(id))
      throw std::invalid_argument("justification rule " + name +
                                  " has no action");
  }

  if (usesDfaStages && source.selections.empty())
    throw std::invalid_argument("justification catalog has no selections");
  if (source.selections.size() >
      std::numeric_limits<JustificationSelectionId>::max()) {
    throw std::invalid_argument("justification catalog has too many selections");
  }

  std::unordered_map<std::string, JustificationSelectionId> selectionIds;
  std::unordered_set<std::string> recordNames;
  for (std::size_t index = 0; index < source.selections.size(); ++index) {
    const auto& selection = source.selections[index];
    const auto id = static_cast<JustificationSelectionId>(index);
    if (selection.name.empty())
      throw std::invalid_argument("a justification selection has no name");
    if (!selectionIds.emplace(selection.name, id).second) {
      throw std::invalid_argument("duplicate justification selection " +
                                  selection.name);
    }
    if (!selection.record.empty()) recordNames.insert(selection.record);
    for (const auto& choice : selection.choices)
      if (!choice.record.empty()) recordNames.insert(choice.record);
  }

  for (const auto& selection : source.selections) {
    if (selection.choices.empty()) {
      throw std::invalid_argument("selection " + selection.name +
                                  " has no choices");
    }
    SelectionPolicy compiled;
    compiled.name = selection.name;
    compiled.lastSubwordFirst = selection.traversal == "last_first";
    if (!compiled.lastSubwordFirst && selection.traversal != "first_first") {
      throw std::invalid_argument("unknown subword traversal " +
                                  selection.traversal);
    }
    compiled.record = selection.record;
    compiled.forbidRecorded = selection.forbidRecorded;
    compiled.differentSubword = selection.differentSubword;
    compiled.differentPosition = selection.differentPosition;
    compiled.requireRecorded = selection.requireRecorded;
    compiled.notAdjacent = selection.notAdjacent;

    for (const auto& choice : selection.choices) {
      const auto rule = ruleIds.find(choice.rule);
      if (rule == ruleIds.end()) {
        throw std::invalid_argument("selection " + selection.name +
                                    " references undefined rule " +
                                    choice.rule);
      }
      if (!std::isfinite(choice.weight) || choice.weight <= 0) throw std::invalid_argument("selection " + selection.name + " has a non-positive choice weight");
      if (!std::isfinite(choice.decay) || choice.decay <= 0 || choice.decay > 1) throw std::invalid_argument("selection " + selection.name + " has a choice decay outside (0, 1]");
      compiled.rules.push_back({.rule = rule->second,
                                .direction = matchDirection(choice.direction),
                                .multiplicity = matchMultiplicity(choice.multiplicity),
                                .weight = choice.weight,
                                .decay = choice.decay,
                                .record = choice.record,
                                .forbidRecorded = choice.forbidRecorded,
                                .differentSubword = choice.differentSubword,
                                .differentPosition = choice.differentPosition,
                                .requireRecorded = choice.requireRecorded,
                                .decayGroup = choice.decayGroup,
                                .notAdjacent = choice.notAdjacent});
    }

    const auto validateRecords = [&](const std::vector<std::string>& names,
                                     std::string_view property) {
      for (const auto& name : names) {
        if (!recordNames.contains(name)) {
          throw std::invalid_argument("selection " + selection.name + " " +
                                      std::string(property) +
                                      " references unknown record " + name);
        }
      }
    };
    validateRecords(compiled.forbidRecorded, "forbid_recorded");
    validateRecords(compiled.requireRecorded, "require_recorded");
    validateRecords(compiled.differentSubword, "different_subword");
    validateRecords(compiled.differentPosition, "different_position");
    validateRecords(compiled.notAdjacent, "not_adjacent");
    for (const auto& choice : compiled.rules) {
      validateRecords(choice.forbidRecorded, "choice forbid_recorded");
      validateRecords(choice.requireRecorded, "choice require_recorded");
      validateRecords(choice.differentSubword, "choice different_subword");
      validateRecords(choice.differentPosition, "choice different_position");
      validateRecords(choice.notAdjacent, "choice not_adjacent");
    }
    result.selections.push_back(std::move(compiled));
  }

  if (source.linePolicy) result.linePolicy = compileLineJustificationPolicy(*source.linePolicy);
  if (source.pagePolicy)
    result.pagePolicy = compilePageJustificationPolicy(*source.pagePolicy);

  const auto compilePhases = [&](const std::vector<DfaPhaseSource>& phases,
                                 const std::string& policyName) {
    std::vector<PolicyPhase> compiled;
    if (phases.empty()) {
      throw std::invalid_argument("justification stage " + policyName + " has no phases");
    }
    for (const auto& phase : phases) {
      const auto selection = selectionIds.find(phase.selection);
      if (selection == selectionIds.end()) {
        throw std::invalid_argument("phase references undefined selection " +
                                    phase.selection);
      }
      const auto allocator = phaseAllocator(phase.allocator);
      if (phase.levels < 1 || phase.levels > 255) {
        throw std::invalid_argument("invalid level count in phase " +
                                    phase.selection);
      }
      if (!std::isfinite(phase.weight) || phase.weight <= 0) throw std::invalid_argument("phase " + phase.selection + " has a non-positive weight");
      if (!std::isfinite(phase.longerSubword) || !std::isfinite(phase.centralConnection) || !std::isfinite(phase.wordPosition)) throw std::invalid_argument("phase " + phase.selection + " has a non-finite candidate score term");
      const auto validLimit = [](int value) { return value >= 0 && value <= std::numeric_limits<std::uint16_t>::max(); };
      if (!validLimit(phase.limit) || !validLimit(phase.perWord) || !validLimit(phase.perSubword)) throw std::invalid_argument("invalid candidate limit in phase " + phase.selection);
      if (allocator != PolicyPhase::Allocator::CandidatePool && (phase.weight != 1 || phase.limit != 0 || phase.perWord != 0 || phase.perSubword != 0 || phase.longerSubword != 0 || phase.centralConnection != 0 || phase.wordPosition != 0)) throw std::invalid_argument("candidate options require candidate_pool in phase " + phase.selection);
      compiled.push_back({selection->second, static_cast<std::uint8_t>(phase.levels), allocator, phase.weight, static_cast<std::uint16_t>(phase.limit), static_cast<std::uint16_t>(phase.perWord), static_cast<std::uint16_t>(phase.perSubword), phase.longerSubword, phase.centralConnection, phase.wordPosition});
    }
    if (std::any_of(compiled.begin(), compiled.end(), [&](const auto& phase) { return phase.allocator != compiled.front().allocator; })) throw std::invalid_argument("allocation modes require a dedicated stage " + policyName);
    if (compiled.front().allocator == PolicyPhase::Allocator::BaselinePool) {
      if (compiled.size() != 1 || compiled.front().levels != 1) throw std::invalid_argument("baseline_pool requires one single-pass phase in stage " + policyName);
      const auto& selection = result.selections.at(compiled.front().selection);
      if (!selection.record.empty() || std::any_of(selection.rules.begin(), selection.rules.end(), [](const auto& rule) { return !rule.record.empty() || rule.weight != 1 || rule.decay != 1; })) throw std::invalid_argument("baseline_pool does not rank candidates or create records in stage " + policyName);
      const auto validateEffects = [&](auto&& self, const std::vector<JustEffect>& effects) -> void {
        for (const auto& effect : effects) {
          if (effect.kind != JustEffectKind::Forbid && effect.kind != JustEffectKind::When && effect.kind != JustEffectKind::Vary) throw std::invalid_argument("baseline_pool accepts only guards and native vary actions in stage " + policyName);
          if (effect.kind == JustEffectKind::Vary && result.attributeAxes.at(effect.attribute) == NoGlyphAxis) throw std::invalid_argument("baseline_pool requires native parameters in stage " + policyName);
          self(self, effect.nested);
        }
      };
      for (const auto& choice : selection.rules) {
        for (const auto& binding : result.actions) {
          if (binding.rule == choice.rule) validateEffects(validateEffects, result.actionDefinitions.at(binding.definition).effects);
        }
      }
    }
    return compiled;
  };

  std::map<std::string, std::vector<PolicyPhase>> stages;
  for (const auto& stage : source.stages) {
    if (stage.name.empty()) throw std::invalid_argument("justification stage has no name");
    if (!stages.emplace(stage.name, compilePhases(stage.phases, stage.name)).second) throw std::invalid_argument("duplicate justification stage " + stage.name);
  }

  const auto compileRecipeSteps = [&](const DfaLineRecipeSource& policy, bool stretch) {
    std::vector<LinePolicyStep> compiled;
    const std::string direction = stretch ? "stretch" : "shrink";
    if (policy.steps.empty()) throw std::invalid_argument("empty " + direction + " policy " + policy.name);
    for (const auto& sourceStep : policy.steps) {
      auto step = compileLinePolicyStep(sourceStep, stretch);
      if (step.operation == LineStepOp::Stage) {
        const auto found = stages.find(step.stage);
        if (found == stages.end()) throw std::invalid_argument(direction + " policy " + policy.name + " references undefined stage " + step.stage);
        step.phases = found->second;
        if (step.continuation.mode != StageContinuation::Mode::None && std::any_of(step.phases.begin(), step.phases.end(), [](const auto& phase) { return phase.allocator != PolicyPhase::Allocator::CandidatePool; }))
          throw std::invalid_argument("stage continuation requires candidate_pool allocation");
        if (!stretch && std::any_of(step.phases.begin(), step.phases.end(), [](const auto& phase) {
              return phase.allocator != PolicyPhase::Allocator::CandidatePool;
            }))
          throw std::invalid_argument("shrink stages require candidate_pool allocation");
      }
      compiled.push_back(std::move(step));
    }
    return compiled;
  };

  if (source.stretchPolicies.empty()) throw std::invalid_argument("justification catalog " + source.name + " has no stretch policy");
  for (const auto& policy : source.stretchPolicies) {
    if (policy.name.empty())
      throw std::invalid_argument("stretch policy has no name");
    if (result.stretchPolicyIndex(policy.name) >= 0)
      throw std::invalid_argument("duplicate stretch policy " + policy.name);
    result.stretchPolicies.push_back({.name = policy.name,
                                      .parameterQuantization = policy.parameterQuantization.value_or(kDefaultProportionalParameterQuantization),
                                      .candidateWidth = candidateWidthMode(policy.candidateWidth.value_or("advance")),
                                      .steps = compileRecipeSteps(policy, true)});
  }

  if (source.shrinkPolicies.empty()) throw std::invalid_argument("justification catalog " + source.name + " has no shrink policy");
  for (const auto& policy : source.shrinkPolicies) {
    if (policy.name.empty()) throw std::invalid_argument("shrink policy has no name");
    if (result.shrinkPolicyIndex(policy.name) >= 0) throw std::invalid_argument("duplicate shrink policy " + policy.name);
    result.shrinkPolicies.push_back({.name = policy.name,
                                     .parameterQuantization = policy.parameterQuantization.value_or(kDefaultProportionalParameterQuantization),
                                     .candidateWidth = candidateWidthMode(policy.candidateWidth.value_or("advance")),
                                     .steps = compileRecipeSteps(policy, false)});
  }

  if (result.linePolicy) {
    result.linePolicy->stretchPolicyIndex = result.stretchPolicyIndex(result.linePolicy->stretchPolicy);
    if (result.linePolicy->stretchPolicyIndex < 0) throw std::invalid_argument("linepolicy references undefined stretch policy " + result.linePolicy->stretchPolicy);
    result.linePolicy->shrinkPolicyIndex = result.shrinkPolicyIndex(result.linePolicy->shrinkPolicy);
    if (result.linePolicy->shrinkPolicyIndex < 0) throw std::invalid_argument("linepolicy references undefined shrink policy " + result.linePolicy->shrinkPolicy);
  }

  result.dfa = std::make_shared<const FixedSlotDfa>(result.rules);
  return result;
}

}  // namespace digitalkhatt::justify
