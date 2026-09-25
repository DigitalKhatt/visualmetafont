#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>

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
  }
}

CompiledJustificationCatalog compileJustificationCatalog(
    const JustificationDfaSource& source, const GlyphAxisRegistry& axes) {
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
    (void)formMatches(fact.form, false, false, false, true);
    (void)positionMatches(fact.position, false, false);
    if (fact.form.empty() && fact.position.empty() &&
        fact.sourceCharacters.empty() && fact.glyphSetRef < 0) {
      throw std::invalid_argument("justification fact " + fact.name +
                                  " constrains nothing and would match every "
                                  "glyph");
    }
    if (!factBits.emplace(fact.name, std::uint64_t{1} << index).second) {
      throw std::invalid_argument("duplicate justification fact " + fact.name);
    }
    for (const auto character : fact.sourceCharacters) {
      if (character < 0 || character > 0xffff) {
        throw std::invalid_argument(
            "invalid source character in justification fact " + fact.name);
      }
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
    result.attachments.push_back({.name = attachment.name,
                                  .within = attachment.within,
                                  .skip = attachment.skip,
                                  .find = attachment.find,
                                  .skipGlyphSetRef = attachment.skipGlyphSetRef,
                                  .findGlyphSetRef = attachment.findGlyphSetRef});
  }

  // Action definitions ------------------------------------------------------
  std::unordered_map<std::string, JustAttributeId> attributeIds;
  const auto attributeId = [&](const std::string& tag) {
    // Native parameters are not truncated into OpenType feature tags.
    const auto axis = axes.find(tag);
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
          compiledEffect.attribute = attributeId(effect.attribute);
          compiledEffect.value = compileExpr(effect.value, definition.name);
          if (effect.clamp <= 0) {
            throw std::invalid_argument("justification action " +
                                        definition.name +
                                        " has an add without a positive clamp");
          }
          compiledEffect.clamp = effect.clamp;
        } else if (effect.kind == "update") {
          compiledEffect.kind = JustEffectKind::Update;
          compiledEffect.attribute = attributeId(effect.attribute);
          compiledEffect.value = compileExpr(effect.value, definition.name);
        } else if (effect.kind == "clear") {
          compiledEffect.kind = JustEffectKind::Clear;
        } else if (effect.kind == "lookup") {
          compiledEffect.kind = JustEffectKind::Lookup;
          if (effect.lookup.empty()) throw std::invalid_argument("justification action " + definition.name + " has an unnamed lookup effect");
          compiledEffect.lookup = effect.lookup;
        } else if (effect.kind == "vary") {
          compiledEffect.kind = JustEffectKind::Vary;
          compiledEffect.attribute = attributeId(effect.attribute);
          if (result.attributeAxes[compiledEffect.attribute] == NoGlyphAxis) {
            throw std::invalid_argument("justification action " + definition.name +
                                        " varies OpenType feature " + effect.attribute +
                                        "; candidate ranges require a native glyph parameter");
          }
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
                {.attribute = attributeId(write.attribute),
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
                                .requireRecorded = choice.requireRecorded});
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
    for (const auto& choice : compiled.rules) {
      validateRecords(choice.forbidRecorded, "choice forbid_recorded");
      validateRecords(choice.requireRecorded, "choice require_recorded");
      validateRecords(choice.differentSubword, "choice different_subword");
      validateRecords(choice.differentPosition, "choice different_position");
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
