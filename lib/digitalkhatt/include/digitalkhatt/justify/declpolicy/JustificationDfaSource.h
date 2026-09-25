#pragma once

#include <string>
#include <optional>
#include <vector>

namespace digitalkhatt::justify {

inline constexpr unsigned kDefaultProportionalParameterQuantization = 16;

// Parser-facing representation of table(justdfa). It deliberately contains
// names rather than runtime enums so the feature parser stays independent of a
// particular font catalog.  A font adapter validates and compiles these names.
// A fact is the conjunction of the discriminators that are present.  Every one
// of them is optional, but a fact with none would match every glyph and is
// rejected when the catalog is compiled.
struct DfaFactSource {
  std::string name;
  // Glyph form: init/medi/fina/isol plus the derived joins_right/joins_left/
  // terminal/nonspacing aliases.  Empty means the fact does not constrain form.
  std::string form;
  // Source characters the glyph's cluster must be one of.  Empty means any.
  std::vector<int> sourceCharacters;
  // Position of the glyph inside its joining subword: "first", "last", or
  // empty for no constraint.
  std::string position;
  // Parser-side handle into FeaContext::justificationGlyphSets, or -1 when the
  // fact has no "glyphs" discriminator.  A glyph set needs the font to expand,
  // so the parser only records the handle and a later pass fills glyphNames in.
  int glyphSetRef = -1;
  // Sorted glyph names the set expanded to.  Empty means no glyph constraint.
  std::vector<std::string> glyphNames;
  bool operator==(const DfaFactSource&) const = default;
};

struct DfaRuleSource {
  std::string name;
  std::vector<std::string> pattern;
  bool operator==(const DfaRuleSource&) const = default;
};

// A mark position derived from a base, resolved against the raw line text so
// that mark reordering and cluster levels cannot perturb it.  Scan forward at
// most `within` characters, stepping over `skip`, stopping at the first `find`.
// Each of find/skip is expressed either as line-text characters or as a glyph
// set, never both.  The two scan different domains: characters are what the
// line holds, glyphs are what shaping produced from it.  They coincide for a
// plain mark and diverge wherever GSUB does anything interesting.
struct DfaAttachmentSource {
  std::string name;
  int within = 1;
  std::vector<int> skip;
  std::vector<int> find;
  // Parser-side handles, or -1 when that side was given characters instead.
  int skipGlyphSetRef = -1;
  int findGlyphSetRef = -1;
  // Sorted glyph names the sets expanded to.
  std::vector<std::string> skipGlyphNames;
  std::vector<std::string> findGlyphNames;
  bool operator==(const DfaAttachmentSource&) const = default;
};

// Where an effect writes, or an expression reads.  `slot` is -1 for "$self"
// (the slot the action is bound at) and otherwise a 1-based position in the
// matched input span.  An empty `attachment` means the slot itself.
struct DfaTargetSource {
  int slot = -1;
  std::string attachment;
  bool operator==(const DfaTargetSource&) const = default;
};

// Expressions are emitted by the grammar directly in postfix order: one
// left-to-right pass over a small stack, no tree, and trivially serialisable.
//
// Note on syntax: arithmetic is functional -- add(a,b) rather than a + b --
// because the feature-file lexer absorbs a leading sign into numeric literals,
// so the sign of an infix operator would be absorbed into the following value.
// For the same reason there is no '/' operator: '/' opens a REGEXP token.
struct DfaExprNodeSource {
  // literal | attribute | present | fact | underfull | underfull_percent | add | sub | mul |
  // floordiv | min | max | gt | lt | eq | ne | ge | le | and | or | not
  // initial_underfull | initial_underfull_percent | stretched | stretched_percent
  std::string op;
  DfaTargetSource target;
  // Attribute tag for attribute/present, fact name for fact.
  std::string name;
  double literal = 0;
  bool operator==(const DfaExprNodeSource&) const = default;
};
using DfaExprSource = std::vector<DfaExprNodeSource>;

struct DfaWriteSource {
  std::string attribute;
  DfaExprSource value;
  bool operator==(const DfaWriteSource&) const = default;
};

struct DfaEffectSource {
  // forbid  -- evaluate `condition` against committed state; abort if true
  // add     -- merge attribute = min(previous + value, clamp)
  // update  -- merge attribute = value, keeping the target's other attributes
  // replace -- clear the target, then apply `writes`
  // clear   -- drop everything staged at the target
  // lookup  -- apply a named structural one-to-one GSUB lookup
  // vary    -- offer a native glyph parameter's current value..value range
  // when    -- run `nested` if `condition` holds
  std::string kind;
  DfaTargetSource target;
  std::string attribute;
  std::string lookup;
  DfaExprSource value;
  double clamp = 0;
  std::vector<DfaWriteSource> writes;
  DfaExprSource condition;
  std::vector<DfaEffectSource> nested;
  bool operator==(const DfaEffectSource&) const = default;
};

struct DfaActionDefSource {
  std::string name;
  std::vector<DfaEffectSource> effects;
  bool operator==(const DfaActionDefSource&) const = default;
};

struct DfaActionSource {
  std::string rule;
  std::string action;
  bool operator==(const DfaActionSource&) const = default;
};

// The actions block holds both binds and action definitions.
struct DfaActionsBlockSource {
  std::vector<DfaActionSource> binds;
  std::vector<DfaActionDefSource> definitions;
  bool operator==(const DfaActionsBlockSource&) const = default;
};

struct DfaChoiceSource {
  std::string rule;
  std::string direction;
  std::string multiplicity = "selected_only";
  // `all` also retains overlapping matches (for independent baseline axes).
  // Candidate-pool score and multiplier for each later occurrence of this
  // rule in the selection's stable traversal order.
  double weight = 1;
  double decay = 1;
  // Choice-local records let one scored selection mix independent kinds of
  // opportunity. Selection-level records remain as shared defaults.
  std::string record;
  std::vector<std::string> forbidRecorded;
  std::vector<std::string> differentSubword;
  std::vector<std::string> differentPosition;
  // Every named record must already have been accepted in this word.
  std::vector<std::string> requireRecorded;
  bool operator==(const DfaChoiceSource&) const = default;
};

struct DfaSelectionSource {
  std::string name;
  std::vector<DfaChoiceSource> choices;
  std::string traversal = "last_first";
  std::string record;
  std::vector<std::string> forbidRecorded;
  std::vector<std::string> differentSubword;
  std::vector<std::string> differentPosition;
  std::vector<std::string> requireRecorded;
  bool operator==(const DfaSelectionSource&) const = default;
};

struct DfaPhaseSource {
  std::string selection;
  int levels = 0;
  // fixed_steps applies one action at a time in traversal order.
  // candidate_pool scores choices line-wide and interpolates their ranges.
  // baseline_pool batches all selected native vary endpoints at one shared
  // ratio, without substitutions/records; later action values override it.
  // `levels` is 1..255 and caps repetition of one site in candidate_pool;
  // zero limits below mean unlimited.
  std::string allocator = "fixed_steps";
  double weight = 1;
  int limit = 0;
  int perWord = 0;
  int perSubword = 0;
  double longerSubword = 0;
  double centralConnection = 0;
  double wordPosition = 0;
  bool operator==(const DfaPhaseSource&) const = default;
};

// Ordered line-level recipes. Numeric arguments use the measurement font's
// 1000-unit em. `features` holds four-character feature tags for feature
// operations.
struct DfaLineStepSource {
  std::string operation;
  std::vector<double> arguments;
  std::vector<std::string> features;
  // Named stage for a `stage Name;` stretch-policy step.
  std::string stage;
  bool operator==(const DfaLineStepSource&) const = default;
};

struct DfaLinePolicySelectionSource {
  std::string selector;  // stretch_policy | shrink_policy
  std::string value;
  bool operator==(const DfaLinePolicySelectionSource&) const = default;
};

struct DfaLinePolicySource {
  // Default complete recipes selected by `stretch Name;` and `shrink Name;`.
  std::string stretchPolicy;
  std::string shrinkPolicy;
  bool operator==(const DfaLinePolicySource&) const = default;
};

struct DfaPageBranchSource {
  std::string section;   // sizing | line | render
  std::string selector;  // style | type | default
  std::string value;
  std::string condition;  // empty or basm2
  std::vector<DfaLineStepSource> steps;
  bool operator==(const DfaPageBranchSource&) const = default;
};

struct DfaPagePolicySource {
  std::vector<std::string> measureTypes;
  std::vector<DfaPageBranchSource> branches;
  bool operator==(const DfaPagePolicySource&) const = default;
};

// A reusable glyph-action stage. Each reference creates a fresh phase
// invocation, while accepted glyph state and selection records remain owned by
// the complete stretch-policy execution.
struct DfaStageSource {
  std::string name;
  std::vector<DfaPhaseSource> phases;
  bool operator==(const DfaStageSource&) const = default;
};

// One selectable, complete line recipe. Stretch and shrink policies share the
// same representation but are validated against direction-specific steps.
struct DfaLineRecipeSource {
  std::string name;
  // Parameter grid subdivisions per unit. Omission uses the compiled default;
  // zero is produced only by the declarative `quantize off;` setting.
  std::optional<unsigned> parameterQuantization;
  // Candidate endpoint measurement: "advance" (default) or "full_shape".
  std::optional<std::string> candidateWidth;
  std::vector<DfaLineStepSource> steps;
  bool operator==(const DfaLineRecipeSource&) const = default;
};

struct JustificationDfaSource {
  std::string name;
  std::vector<DfaFactSource> facts;
  std::vector<DfaRuleSource> rules;
  std::vector<DfaActionSource> actions;
  std::vector<DfaAttachmentSource> attachments;
  std::vector<DfaActionDefSource> actionDefinitions;
  std::vector<DfaSelectionSource> selections;
  std::vector<DfaStageSource> stages;
  std::vector<DfaLineRecipeSource> stretchPolicies;
  std::vector<DfaLineRecipeSource> shrinkPolicies;
  std::optional<DfaLinePolicySource> linePolicy;
  std::optional<DfaPagePolicySource> pagePolicy;
  bool operator==(const JustificationDfaSource&) const = default;
};

}  // namespace digitalkhatt::justify
