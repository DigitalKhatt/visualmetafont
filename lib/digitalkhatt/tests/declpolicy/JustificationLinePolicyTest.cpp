#include "../../src/justify/declpolicy/JustificationLinePolicy.h"
#include "../../src/justify/declpolicy/DeclPolicyAdapter.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace digitalkhatt;
using namespace digitalkhatt::justify;
using namespace digitalkhatt::justify::decl;
using namespace digitalkhatt::justify::runtime;

namespace {

struct Backend final : LineJustificationBackend {
  std::vector<double> widths;
  std::vector<std::vector<TextFontFeatures>> trials;
  double glyphGain = 200;
  double glyphInput = 0;
  int glyphCalls = 0;
  float axisValue = 0;
  double axisWidth = 850;
  std::vector<std::pair<std::string, StageContinuation>> continuations;
  std::vector<bool> retainedStages;
  double measure(const std::vector<TextFontFeatures>& features, const std::vector<TextFontFeatures>&, double) override {
    trials.push_back(features);
    return widths.at(trials.size() - 1);
  }
  double applyStage(const LinePolicyStep& step, const std::vector<TextFontFeatures>&, double width, bool retain) override {
    continuations.emplace_back(step.stage, step.continuation);
    retainedStages.push_back(retain);
    ++glyphCalls;
    glyphInput = width;
    return width + glyphGain;
  }
  double measureSclx(float value) override {
    axisValue = value;
    return axisWidth;
  }
};

DfaLinePolicySource fixture() {
  return {.stretchPolicy = "standard", .shrinkPolicy = "standard"};
}

std::vector<LinePolicyStep> stretchFixture() {
  return {compileLinePolicyStep({"cap_spaces", {250, 300}, {}}, true), compileLinePolicyStep({.operation = "stage", .stage = "Main"}, true), compileLinePolicyStep({"fill_spaces", {}, {}}, true)};
}

std::vector<LinePolicyStep> standardShrinkFixture() {
  return {compileLinePolicyStep({"fit_features", {}, {"t001", "t002", "t003"}}, false), compileLinePolicyStep({"balance", {}, {}}, false)};
}

bool expect(bool condition, const char* message) {
  if (!condition) std::cerr << message << '\n';
  return condition;
}

bool near(double a, double b) { return std::abs(a - b) < 1e-6; }

bool rejectsLinePolicy(DfaLinePolicySource source) {
  try {
    (void)compileLineJustificationPolicy(source);
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}

template <typename Mutation>
bool rejectsShrinkStep(Mutation mutation) {
  DfaLineStepSource source{"fit_features", {}, {"t001"}};
  mutation(source);
  try {
    (void)compileLinePolicyStep(source, false);
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}

template <typename Mutation>
bool rejectsStretchStep(Mutation mutation) {
  DfaLineStepSource source{"cap_spaces", {250, 300}, {}};
  mutation(source);
  try {
    (void)compileLinePolicyStep(source, true);
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}

}  // namespace

int main() {
  const auto policy = compileLineJustificationPolicy(fixture());
  Backend backend;
  bool ok = true;
  const std::vector<LinePolicyStep> scaleOnly{compileLinePolicyStep({.operation = "scale"}, false)};
  auto scopedResult = executeLineJustificationPolicy({}, scaleOnly, {1000, 1200, 100, 0, 0}, backend);
  ok &= expect(scopedResult.operation == LineJustificationOperation::Shrink, "shrink recipe is recorded even without a glyph-action stage");
  scopedResult = executeLineJustificationPolicy({}, {}, {1200, 1000, 100, 0, 0}, backend);
  ok &= expect(scopedResult.operation == LineJustificationOperation::Stretch, "stretch recipe is retained even if no stage changes the glyphs");
  scopedResult = executeLineJustificationPolicy({}, {}, {1000, 1000, 100, 0, 0}, backend);
  ok &= expect(scopedResult.operation == LineJustificationOperation::None, "exact-fit lines have no stretch or shrink operation");
  ok &= expect(near(quantizeProportionalParameter(3.141, 0, 20, 16), 3.125), "default proportional grid rounds expansion downward to a sixteenth");
  ok &= expect(near(quantizeProportionalParameter(3.141, 0, 20, 0), 3.141), "zero subdivisions disable proportional quantization");
  ok &= expect(near(quantizeProportionalParameter(3.141, 3.13, 20, 16), 3.13), "quantization never crosses the accepted minimum endpoint");
  ok &= expect(near(quantizeProportionalParameter(-0.141, 0, -0.2, 100), -0.14), "decreasing shrink ranges round toward the uncompressed endpoint");
  const auto stretch = stretchFixture();
  const auto standardShrink = standardShrinkFixture();
  const std::vector<LinePolicyStep> repeatedStage{
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred"}, true),
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationLimit = 1}, true),
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationLimit = 9}, true),
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "maximum"}, true)};
  backend = {};
  (void)executeLineJustificationPolicy(repeatedStage, {}, {3000, 1000, 100, 0, 0}, backend);
  ok &= expect(backend.continuations == std::vector<std::pair<std::string, StageContinuation>>{{"Preferred", {}}, {"Preferred", {StageContinuation::Mode::By, 1}}, {"Preferred", {StageContinuation::Mode::To, 9}}, {"Preferred", {StageContinuation::Mode::Maximum, 0}}} && backend.glyphCalls == 4 && backend.glyphInput == 1600, "stage invocations preserve identity, continuation mode, limit and current width");
  ok &= expect(backend.retainedStages == std::vector<bool>(4, true), "ordinary stages retain accepted sites when the recipe later continues them");
  const std::vector<LinePolicyStep> percentageStages{
      repeatedStage.front(),
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationLimit = 50, .continuationPercentage = true}, true),
      compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationLimit = 12.5, .continuationPercentage = true}, true)};
  backend = {};
  (void)executeLineJustificationPolicy(percentageStages, {}, {3000, 1000, 100, 0, 0}, backend);
  ok &= expect(backend.continuations == std::vector<std::pair<std::string, StageContinuation>>{{"Preferred", {}}, {"Preferred", {StageContinuation::Mode::To, 50, true}}, {"Preferred", {StageContinuation::Mode::By, 12.5, true}}} && backend.retainedStages == std::vector<bool>(3, true), "percentage limits survive compilation, dispatch and retained-stage tracking");
  backend = {};
  (void)executeLineJustificationPolicy(std::span(repeatedStage).first(1), {}, {3000, 1000, 100, 0, 0}, backend);
  ok &= expect(backend.retainedStages == std::vector<bool>{false}, "recipes without continuation avoid retaining stage state");
  backend = {};
  const auto executeWith = [](std::span<const LinePolicyStep> stretchSteps, std::span<const LinePolicyStep> shrinkSteps, LineJustificationMetrics metrics, LineJustificationBackend& selectedBackend) { return executeLineJustificationPolicy(stretchSteps, shrinkSteps, metrics, selectedBackend); };
  const auto execute = [&](LineJustificationMetrics metrics, LineJustificationBackend& selectedBackend) { return executeWith(stretch, standardShrink, metrics, selectedBackend); };
  auto result = execute({1800, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphInput == 1350 && backend.glyphCalls == 1, "space caps must precede glyph actions");
  ok &= expect(result.simpleSpacing == 375 && result.ayaSpacing == 425, "remaining stretch is shared equally after different space caps");

  auto changedStretch = stretch;
  changedStretch[0].arguments = {100, 100};
  backend = {};
  result = executeWith(changedStretch, standardShrink, {1800, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphInput == 1000 && result.simpleSpacing == 400 && result.ayaSpacing == 400, "editing policy caps changes execution without changing C++");

  backend = {};
  result = execute({1250, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphCalls == 0, "do not invoke glyph policy when spaces meet the target");
  changedStretch = stretch;
  std::swap(changedStretch[0], changedStretch[1]);
  backend = {};
  result = executeWith(changedStretch, standardShrink, {1250, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphCalls == 1 && backend.glyphInput == 1000, "feature-file step order is authoritative");

  const std::vector<LinePolicyStep> pooledShrink{
      compileLinePolicyStep({.operation = "stage", .stage = "Shrink"}, false),
      compileLinePolicyStep({.operation = "stage", .stage = "Fallback"}, false),
      compileLinePolicyStep({.operation = "balance"}, false)};
  backend = {};
  backend.glyphGain = -100;
  result = executeWith(stretch, pooledShrink, {900, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphCalls == 1 && backend.glyphInput == 1000 && result.isShrink && result.xScale == 1 && result.simpleSpacing == 100,
               "shrink stages preserve shaped spaces and skip subsequent stages at the target");
  backend = {};
  backend.glyphGain = -50;
  result = executeWith(stretch, pooledShrink, {800, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphCalls == 2 && backend.glyphInput == 950 && near(result.xScale, 800.0 / 900),
               "successive shrink stages receive the accepted current width before balance");
  backend = {};
  result = executeWith(stretch, pooledShrink, {1000, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.glyphCalls == 0 && result.isShrink, "exact-width shrink stages do not run");

  backend = {};
  backend.widths = {1100, 900, 700};
  result = execute({750, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.trials.size() == 3 && backend.trials.back().size() == 2 && backend.trials.back()[0].name == "t002", "shrink trials start from only accepted features");
  ok &= expect(result.globalFeatures.size() == 2 && result.globalFeatures[0].name == "t002" && result.globalFeatures[1].name == "t003", "reject non-reducing steps but accept the first undershoot");
  ok &= expect(result.isShrink && near(result.addedSpaceAfterShrink, 25) && result.xScale == 1 && result.simpleSpacing == 100, "balance distributes undershoot equally on top of shaped spaces");

  backend = {};
  backend.widths = {900, 975, 950};
  result = execute({950, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.trials.size() == 1 && result.globalFeatures.size() == 1 && result.globalFeatures[0].name == "t001" && near(result.addedSpaceAfterShrink, 25) && result.xScale == 1, "stop immediately after accepting an undershooting feature");
  const std::vector<LinePolicyStep> undershootShrink{
      compileLinePolicyStep({"fit_features", {}, {"t001", "t002"}}, false),
      compileLinePolicyStep({.operation = "stage", .stage = "Fallback"}, false),
      compileLinePolicyStep({"fit_features", {}, {"t003"}}, false),
      compileLinePolicyStep({"all_features", {}, {"t004"}}, false),
      compileLinePolicyStep({"fit_sclx", {100}, {}}, false),
      compileLinePolicyStep({.operation = "balance"}, false)};
  backend = {};
  backend.widths = {900};
  result = executeWith(stretch, undershootShrink, {950, 1000, 100, 2, 1}, backend);
  ok &= expect(backend.trials.size() == 1 && backend.glyphCalls == 0 && backend.axisValue == 0 && result.globalFeatures.size() == 1 && near(result.addedSpaceAfterShrink, 50.0 / 3) && result.xScale == 1, "skip all later shrink operations, then balance simple and aya spaces equally");
  backend = {};
  backend.widths = {900};
  result = executeWith(stretch, undershootShrink, {950, 1000, 100, 0, 0}, backend);
  ok &= expect(backend.trials.size() == 1 && backend.glyphCalls == 0 && result.addedSpaceAfterShrink == 0 && near(result.xScale, 950.0 / 900), "undershoot without spaces uses finite balance scaling");
  backend = {};
  backend.widths = {950};
  result = executeWith(stretch, undershootShrink, {950, 1000, 100, 2, 1}, backend);
  ok &= expect(backend.trials.size() == 1 && backend.glyphCalls == 0 && result.addedSpaceAfterShrink == 0 && result.xScale == 1, "exact-fitting feature stops shrinking without adding space");
  backend = {};
  backend.widths = {950, 900, 850};
  result = execute({800, 1000, 100, 1, 1}, backend);
  ok &= expect(near(result.xScale, 800.0 / 850), "balance scales when features cannot shrink enough");

  backend = {};
  result = execute({1000, 1000, 100, 1, 1}, backend);
  ok &= expect(result.isShrink && backend.trials.empty() && result.xScale == 1, "exact-width lines retain Standard shrink semantics without trials");
  const std::vector<LinePolicyStep> testShrink{compileLinePolicyStep({"all_features", {}, {"t001", "t002"}}, false)};
  result = executeWith(stretch, testShrink, {1000, 1000, 100, 1, 1}, backend);
  ok &= expect(result.isShrink && result.globalFeatures.size() == 2 && backend.trials.empty(), "an explicitly selected shrink recipe applies its declared list at exact width");

  const std::vector<LinePolicyStep> axisShrink{compileLinePolicyStep({"fit_sclx", {100}, {}}, false)};
  backend = {};
  result = executeWith(stretch, axisShrink, {800, 1000, 100, 1, 1}, backend);
  ok &= expect(result.sclxAxis == 80 && result.globalFeatures.empty() && near(result.xScale, 800.0 / 850), "an explicitly selected axis shrink recipe measures the axis");
  backend = {};
  backend.axisWidth = 1000;
  result = executeWith(stretch, axisShrink, {800, 1000, 100, 1, 1}, backend);
  ok &= expect(result.sclxAxis == 0 && result.xScale == 1, "a non-reducing axis trial preserves the original result");

  const std::vector<LinePolicyStep> scaleShrink{compileLinePolicyStep({"scale", {}, {}}, false)};
  backend = {};
  result = executeWith(stretch, scaleShrink, {800, 1000, 100, 0, 0}, backend);
  ok &= expect(near(result.xScale, 0.8) && backend.trials.empty() && !result.isShrink, "default recipe scales without trials");
  backend = {};
  backend.glyphGain = 0;
  result = execute({1200, 1000, 100, 0, 0}, backend);
  ok &= expect(near(result.xScale, 1.2) && std::isfinite(result.simpleSpacing), "a line without spaces uses finite scale fallback");
  result = execute({0, 0, 100, 0, 0}, backend);
  ok &= expect(result.xScale == 1, "empty line does not divide by zero");

  // Fixed OpenType stretch steps can precede the glyph-action allocator.
  // An overflowing feature is skipped, and later trials start from only the
  // accepted set so the measured and finally shaped feature lists agree.
  auto hybridStretch = stretch;
  hybridStretch.insert(hybridStretch.begin(), compileLinePolicyStep({"fit_features", {}, {"st01", "st02", "st03"}}, true));
  backend = {};
  backend.widths = {1150, 1500, 1250};
  backend.glyphGain = 100;
  result = executeWith(hybridStretch, standardShrink, {1400, 1000, 100, 0, 0}, backend);
  ok &= expect(backend.trials.size() == 3 && backend.trials[2].size() == 2 && backend.trials[2][0].name == "st01" && backend.trials[2][1].name == "st03", "stretch feature trials retain only earlier accepted steps");
  ok &= expect(result.globalFeatures.size() == 2 && result.globalFeatures[0].name == "st01" && result.globalFeatures[1].name == "st03" && backend.glyphInput == 1250 && !result.isShrink, "accepted stretch features feed the following glyph actions");

  ok &= expect(rejectsStretchStep([](auto& s) { s.arguments = {-1, 250}; }), "reject negative caps");
  ok &= expect(rejectsStretchStep([](auto& s) { s.arguments = {250}; }), "reject wrong arity");
  ok &= expect(rejectsStretchStep([](auto& s) { s.arguments[0] = std::numeric_limits<double>::infinity(); }), "reject non-finite values");
  ok &= expect(rejectsShrinkStep([](auto& s) { s.operation = "typo"; }), "reject unknown shrink operation");
  ok &= expect(rejectsShrinkStep([](auto& s) { s.features = {"too_long"}; }), "reject invalid feature tag");
  ok &= expect(rejectsShrinkStep([](auto& s) { s.features = {"t001", "t001"}; }), "reject repeated feature");
  ok &= expect(rejectsShrinkStep([](auto& s) { s.features.clear(); }), "reject empty feature sequence");
  ok &= expect(rejectsShrinkStep([](auto& s) { s.operation = "glyph_actions"; }), "reject removed legacy glyph_actions primitive");
  ok &= expect(rejectsShrinkStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationLimit = 1}; }), "reject shrink stage continuation");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationLimit = -1}; }), "reject negative continuation limits");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationLimit = std::numeric_limits<double>::infinity()}; }), "reject non-finite continuation limits");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationLimit = 0}; }), "reject zero relative continuation");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "increment", .continuationLimit = 1}; }), "reject replaced increment spelling");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "maximum", .continuationLimit = 1}; }), "bare continuation takes no numeric limit");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "fill_spaces", .continuation = "maximum"}; }), "reject continuation on non-stage operations");
  ok &= expect(rejectsShrinkStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationLimit = 50, .continuationPercentage = true}; }), "reject percentage continuation during shrinking");
  for (double invalidPercentage : {-1.0, 100.1, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
    ok &= expect(rejectsStretchStep([&](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationLimit = invalidPercentage, .continuationPercentage = true}; }), "reject invalid percentage continuation limits");
  }
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "by", .continuationPercentage = true}; }), "reject zero percentage increments");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuation = "maximum", .continuationPercentage = true}; }), "bare continuation takes no percentage unit");
  ok &= expect(rejectsStretchStep([](auto& s) { s = {.operation = "stage", .stage = "Preferred", .continuationPercentage = true}; }), "percentage units require continuation");
  ok &= expect(compileLinePolicyStep({.operation = "stage", .stage = "Preferred", .continuation = "to", .continuationPercentage = true}, true).continuation == StageContinuation{StageContinuation::Mode::To, 0, true}, "zero-percent absolute targets are valid");
  ok &= expect(rejectsShrinkStep([](auto& s) { s = {.operation = "reduce_spaces", .arguments = {0.95}}; }), "reject removed reduce_spaces primitive");
  ok &= expect(rejectsStretchStep([](auto& s) { s.operation = "typo"; }), "reject unknown stretch operation");
  auto missingStretch = fixture();
  missingStretch.stretchPolicy.clear();
  ok &= expect(rejectsLinePolicy(missingStretch), "require a named stretch policy");
  auto missingShrink = fixture();
  missingShrink.shrinkPolicy.clear();
  ok &= expect(rejectsLinePolicy(missingShrink), "require a named shrink policy");
  ok &= expect(policy.stretchPolicy == "standard" && policy.shrinkPolicy == "standard", "linepolicy selects both default recipe names");
  return ok ? 0 : 1;
}
