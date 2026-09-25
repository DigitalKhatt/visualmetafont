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
  double measure(const std::vector<TextFontFeatures>& features, const std::vector<TextFontFeatures>&, double) override {
    trials.push_back(features);
    return widths.at(trials.size() - 1);
  }
  double applyStage(std::span<const PolicyPhase>, const std::vector<TextFontFeatures>&, double width) override {
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
  ok &= expect(near(quantizeProportionalParameter(3.141, 0, 20, 16), 3.125), "default proportional grid rounds expansion downward to a sixteenth");
  ok &= expect(near(quantizeProportionalParameter(3.141, 0, 20, 0), 3.141), "zero subdivisions disable proportional quantization");
  ok &= expect(near(quantizeProportionalParameter(3.141, 3.13, 20, 16), 3.13), "quantization never crosses the accepted minimum endpoint");
  const auto stretch = stretchFixture();
  const auto standardShrink = standardShrinkFixture();
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

  backend = {};
  backend.widths = {1100, 900, 700};
  result = execute({750, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.trials.size() == 3 && backend.trials.back().size() == 3 && backend.trials.back()[0].name == "t001", "non-reducing trials remain in cumulative measurements");
  ok &= expect(result.globalFeatures.size() == 2 && result.globalFeatures[0].name == "t002" && result.globalFeatures[1].name == "t003", "only reducing steps remain in the final feature list");
  ok &= expect(result.isShrink && result.addedSpaceAfterShrink == 25 && result.xScale == 1, "balance fills undershoot after shrinking");

  backend = {};
  backend.widths = {900};
  result = execute({950, 1000, 100, 1, 1}, backend);
  ok &= expect(backend.trials.size() == 1 && result.globalFeatures.size() == 1, "stop measuring features as soon as width fits");
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
