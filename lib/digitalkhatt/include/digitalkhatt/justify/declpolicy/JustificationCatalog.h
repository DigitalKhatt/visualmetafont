#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <digitalkhatt/justify/declpolicy/FixedSlotDfa.h>
#include <digitalkhatt/justify/declpolicy/JustificationDfaSource.h>
#include <digitalkhatt/core/digitalkahtt_types.h>

namespace digitalkhatt::justify {

using JustificationRuleId = std::uint16_t;
using JustificationSelectionId = std::uint16_t;

using JustAttributeId = std::uint16_t;
using JustAttachmentId = std::uint16_t;

// A bind names one of the catalog's declared actions.
struct JustificationActionBinding {
  JustificationRuleId rule;
  // Index into CompiledJustificationCatalog::actionDefinitions.
  int definition = -1;
  // Slot of the matched span this action is anchored at: -1 means the first
  // slot.  Reserved for per-slot binds; only -1 is produced today.
  int slot = -1;
  bool operator==(const JustificationActionBinding&) const = default;
};

enum class JustExprOp : std::uint8_t {
  Literal,    // push literal
  Attribute,  // push read(target, attribute), 0 when absent
  Present,    // push present(target, attribute)
  Fact,       // push (facts(target) & factMask) != 0
  Underfull,  // remaining non-negative line gap, in measurement units
  UnderfullPercent,  // 100 * gap / target width
  InitialUnderfull,
  InitialUnderfullPercent,
  Stretched,  // non-negative net growth from the initial width
  StretchedPercent,
  Add,
  Sub,
  Mul,
  FloorDiv,
  Min,
  Max,
  Select,  // pops b, a, condition -> condition ? a : b
  Gt,
  Lt,
  Ge,
  Le,
  Eq,
  Ne,
  And,
  Or,
  Not,
};

struct JustTarget {
  // -1 for the anchor slot ("$self"), otherwise a 0-based slot index.
  int slot = -1;
  // Index into CompiledJustificationCatalog::attachments, or -1 for the slot.
  int attachment = -1;
  bool operator==(const JustTarget&) const = default;
};

struct JustExprNode {
  JustExprOp op = JustExprOp::Literal;
  JustTarget target;
  JustAttributeId attribute = 0;
  std::uint64_t factMask = 0;
  double literal = 0;
  bool operator==(const JustExprNode&) const = default;
};

// Postfix; evaluated left to right over a small numeric stack.
using JustExpr = std::vector<JustExprNode>;

enum class JustEffectKind : std::uint8_t { Forbid,
                                           Add,
                                           Update,
                                           Replace,
                                           Clear,
                                           Lookup,
                                           When,
                                           Vary };

struct JustWrite {
  JustAttributeId attribute = 0;
  JustExpr value;
  bool operator==(const JustWrite&) const = default;
};

struct JustEffect {
  JustEffectKind kind = JustEffectKind::Forbid;
  JustTarget target;
  JustAttributeId attribute = 0;
  std::string lookup;
  // For Add, the increment.
  JustExpr value;
  // For Add, the saturating maximum.
  double clamp = 0;
  std::vector<JustWrite> writes;
  JustExpr condition;
  std::vector<JustEffect> nested;
  bool operator==(const JustEffect&) const = default;
};

struct JustificationAction {
  std::string name;
  // Strictly ordered; never sorted.  Each read observes the staged state left
  // by the effects before it, which is what lets an expression use a value the
  // action itself has just written.
  std::vector<JustEffect> effects;
  bool operator==(const JustificationAction&) const = default;
};

struct JustAttachment {
  std::string name;
  int within = 1;
  std::vector<int> skip;
  std::vector<int> find;
  int skipGlyphSetRef = -1;
  int findGlyphSetRef = -1;
  // Sorted; empty when that side matches characters rather than glyphs.
  std::vector<std::string> skipGlyphNames;
  std::vector<std::string> findGlyphNames;
  bool matchesSkipGlyphs() const { return !skipGlyphNames.empty(); }
  bool matchesFindGlyphs() const { return !findGlyphNames.empty(); }
  bool operator==(const JustAttachment&) const = default;
};

enum class MatchDirection : std::uint8_t {
  LogicalFirst,
  LogicalLast,
};

enum class MatchMultiplicity : std::uint8_t {
  SelectedOnly,
  AllNonOverlapping,
  All,
};

struct RuleSelection {
  JustificationRuleId rule = 0;
  MatchDirection direction = MatchDirection::LogicalLast;
  MatchMultiplicity multiplicity = MatchMultiplicity::SelectedOnly;
  double weight = 1;
  double decay = 1;
  std::string record;
  std::vector<std::string> forbidRecorded;
  std::vector<std::string> differentSubword;
  std::vector<std::string> differentPosition;
  std::vector<std::string> requireRecorded;
  bool operator==(const RuleSelection&) const = default;
};

struct SelectionPolicy {
  std::string name;
  std::vector<RuleSelection> rules;
  bool lastSubwordFirst = true;
  std::string record;
  std::vector<std::string> forbidRecorded;
  std::vector<std::string> differentSubword;
  std::vector<std::string> differentPosition;
  std::vector<std::string> requireRecorded;
  bool operator==(const SelectionPolicy&) const = default;
};

struct PolicyPhase {
  enum class Allocator : std::uint8_t {
    FixedSteps,
    CandidatePool,
    BaselinePool,
  };

  JustificationSelectionId selection = 0;
  std::uint8_t levels = 0;
  Allocator allocator = Allocator::FixedSteps;
  double weight = 1;
  std::uint16_t limit = 0;
  std::uint16_t perWord = 0;
  std::uint16_t perSubword = 0;
  double longerSubword = 0;
  double centralConnection = 0;
  double wordPosition = 0;
  bool operator==(const PolicyPhase&) const = default;
};

enum class LineStepOp : std::uint8_t { CapSpaces,
                                       Stage,
                                       FillSpaces,
                                       FitFeatures,
                                       AllFeatures,
                                       FitSclx,
                                       Balance,
                                       Scale };
struct LinePolicyStep {
  LineStepOp operation;
  std::vector<double> arguments;
  std::vector<std::string> features;
  std::string stage;
  std::vector<PolicyPhase> phases;
};

struct LineJustificationPolicy {
  std::string stretchPolicy;
  std::string shrinkPolicy;
  int stretchPolicyIndex = -1;
  int shrinkPolicyIndex = -1;
};

enum class CandidateWidthMode : std::uint8_t {
  Advance,
  FullShape,
};

LineJustificationPolicy compileLineJustificationPolicy(const DfaLinePolicySource& source);
LinePolicyStep compileLinePolicyStep(const DfaLineStepSource& step, bool stretch);

enum class PageSizingOp : std::uint8_t { Fixed,
                                         MinFit,
                                         BoundedFit };
enum class PageOutputOp : std::uint8_t { Normal,
                                         XScale,
                                         AxisFont };

struct PageSizingRule {
  std::optional<JustStyle> style;
  PageSizingOp operation;
  std::vector<double> arguments;
};

struct PageLineRule {
  std::optional<LineType> type;
  bool requireBasm2 = false;
  bool justify = false;
  std::vector<std::string> features;
};

struct PageRenderRule {
  std::optional<JustStyle> style;
  bool fontScale = false;
  PageOutputOp output = PageOutputOp::Normal;
};

struct PageJustificationPolicy {
  std::vector<LineType> measureTypes;
  // Each group uses first-match semantics and requires an explicit default.
  std::vector<PageSizingRule> sizing;
  std::vector<PageLineRule> lines;
  std::vector<PageRenderRule> rendering;
};

PageJustificationPolicy compilePageJustificationPolicy(const DfaPagePolicySource& source);

// A selectable complete stretch or shrink recipe. Everything else in the
// catalog is shared.
struct CompiledLineRecipe {
  std::string name;
  unsigned parameterQuantization = kDefaultProportionalParameterQuantization;
  CandidateWidthMode candidateWidth = CandidateWidthMode::Advance;
  std::vector<LinePolicyStep> steps;
};

struct CompiledJustificationCatalog {
  std::string name;
  std::vector<DfaFactSource> facts;
  std::vector<FixedSlotRule> rules;
  std::vector<JustificationActionBinding> actions;
  std::vector<SelectionPolicy> selections;
  std::vector<JustAttachment> attachments;
  std::vector<JustificationAction> actionDefinitions;
  // Attribute ids index feature tags or native glyph-parameter names.
  std::vector<std::string> attributes;
  std::vector<GlyphAxisId> attributeAxes;  // NoGlyphAxis means an OpenType feature.
  std::shared_ptr<const FixedSlotDfa> dfa;
  std::optional<LineJustificationPolicy> linePolicy;
  std::optional<PageJustificationPolicy> pagePolicy;
  // Never empty: every catalog declares at least one recipe in each direction.
  std::vector<CompiledLineRecipe> stretchPolicies;
  std::vector<CompiledLineRecipe> shrinkPolicies;

  // Index into `stretchPolicies`, or -1 when no policy carries that name.
  int stretchPolicyIndex(std::string_view name) const {
    for (std::size_t index = 0; index < stretchPolicies.size(); ++index)
      if (stretchPolicies[index].name == name) return static_cast<int>(index);
    return -1;
  }
  int shrinkPolicyIndex(std::string_view name) const {
    for (std::size_t index = 0; index < shrinkPolicies.size(); ++index)
      if (shrinkPolicies[index].name == name) return static_cast<int>(index);
    return -1;
  }
  // Bounds-checked, because the index reaches here from JustOption, which
  // callers fill by hand.
  const CompiledLineRecipe& stretchPolicy(int index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= stretchPolicies.size())
      throw std::out_of_range("stretch policy index out of range");
    return stretchPolicies[static_cast<std::size_t>(index)];
  }
  const CompiledLineRecipe& shrinkPolicy(int index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= shrinkPolicies.size())
      throw std::out_of_range("shrink policy index out of range");
    return shrinkPolicies[static_cast<std::size_t>(index)];
  }
};

// Everything the fact discriminators can test about one shaped base glyph.
// The subword flags come from the layout adapter's real base indexes rather
// than from the glyph name, because a glyph whose name carries no .init/.medi/
// .fina/.isol substring yields no form facts at all.
struct GlyphFactInput {
  std::string_view glyphName;
  char16_t sourceCharacter = 0;
  bool firstInSubword = false;
  bool lastInSubword = false;
};

std::uint64_t glyphFacts(const GlyphFactInput& glyph,
                         std::span<const DfaFactSource> facts);

CompiledJustificationCatalog compileJustificationCatalog(
    const JustificationDfaSource& source, const GlyphAxisRegistry& axes = {});

// Expands one glyph set, named by its parser-side handle, into glyph names.
// Supplied by the font adapter because glyph sets need the font; the justify
// library stays font-free.
using JustificationGlyphSetResolver =
    std::function<std::vector<std::string>(int glyphSetRef)>;

// Second, font-dependent compilation pass.  Names rather than glyph ids, so one
// catalog stays valid across two fonts with independent id spaces -- which is
// exactly what the shaping comparison tool does.
void resolveJustificationGlyphSets(CompiledJustificationCatalog& catalog,
                                   const JustificationGlyphSetResolver& resolve);

}  // namespace digitalkhatt::justify
