#include "../../src/justify/declpolicy/JustificationPagePolicy.h"

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

DfaPagePolicySource fixture() {
  return {{"Line", "Sura", "Bism"}, {{"sizing", "style", "SameSizeByPage", "", {{"min_fit", {1}, {}}}}, {"sizing", "style", "FontSizeXScale", "", {{"bounded_fit", {0.95, 1.2, 0.02, 0.02}, {}}}}, {"sizing", "default", "", "", {{"fixed_size", {1}, {}}}}, {"line", "type", "Bism", "basm2", {{"natural", {}, {}}, {"final_features", {}, {"bism"}}}}, {"line", "type", "Bism", "", {{"natural", {}, {}}, {"final_features", {}, {"basm"}}}}, {"line", "type", "Sura", "", {{"natural", {}, {}}}}, {"line", "default", "", "", {{"use_line_policy", {}, {}}}}, {"render", "style", "FontSize", "", {{"font_scale", {}, {}}, {"normal_output", {}, {}}}}, {"render", "style", "SCLX", "", {{"axis_output", {}, {}}}}, {"render", "style", "XScale", "", {{"xscale_output", {}, {}}}}, {"render", "default", "", "", {{"normal_output", {}, {}}}}}};
}

bool expect(bool condition, const char* message) {
  if (!condition) std::cerr << message << '\n';
  return condition;
}
bool near(double a, double b) { return std::abs(a - b) < 1e-6; }

template <typename Mutation>
bool rejects(Mutation mutation) {
  auto source = fixture();
  mutation(source);
  try {
    (void)compilePageJustificationPolicy(source);
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}

}  // namespace

int main() {
  const auto policy = compilePageJustificationPolicy(fixture());
  bool ok = true;
  const auto ratio = [&](std::initializer_list<double> values, JustStyle style = JustStyle::FontSizeXScale) { return pageFontRatio(policy, style, {values.begin(), values.size()}); };
  ok &= expect(near(ratio({0.98, 1.1}), 1), "within thresholds keeps size");
  ok &= expect(near(ratio({1, 1.3}), 1.02), "stretch is capped");
  ok &= expect(near(ratio({0.9, 1.1}), 0.98), "shrink is capped");
  ok &= expect(near(ratio({0.9, 1.3}), 1), "inconsistent page keeps size");
  ok &= expect(near(ratio({0.96, 1.3}), 1.01), "stretch respects opposite threshold");
  ok &= expect(near(ratio({0.9, 1.19}), 0.99), "shrink respects opposite threshold");
  ok &= expect(near(ratio({0.95, 1.2}), 1), "exact boundaries keep size");
  ok &= expect(near(ratio({0.9, 1.3}, JustStyle::SameSizeByPage), 0.9), "minimum-fit uses the narrowest ratio");
  ok &= expect(near(ratio({1.1, 1.3}, JustStyle::SameSizeByPage), 1), "minimum-fit respects its ceiling");
  ok &= expect(near(ratio({0.9, 1.3}, JustStyle::FontSize), 1), "default sizing is explicit");
  ok &= expect(near(ratio({}), 1) && near(ratio({}, JustStyle::SameSizeByPage), 1), "empty page has finite fallback");
  ok &= expect(near(ratio({0, -1, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}), 1), "invalid measurements cannot poison page sizing");

  auto changed = fixture();
  changed.branches[1].steps[0].arguments[3] = 0.04;
  const std::vector<double> wide{1, 1.3};
  ok &= expect(near(pageFontRatio(compilePageJustificationPolicy(changed), JustStyle::FontSizeXScale, wide), 1.04), "table stretch limit is authoritative");
  changed = fixture();
  changed.branches[2].steps[0].arguments = {0.9};
  ok &= expect(near(pageFontRatio(compilePageJustificationPolicy(changed), JustStyle::None, wide), 0.9), "default size is data, not a C++ constant");

  LineToJustify line{u"text", 1000, LineJustification::Center, LineType::Bism, true};
  ok &= expect(pageMeasuresLine(policy, line), "headings contribute when declared");
  changed = fixture();
  changed.measureTypes = {"Line"};
  ok &= expect(!pageMeasuresLine(compilePageJustificationPolicy(changed), line), "table can exclude headings from sizing");
  line.width = 0;
  ok &= expect(!pageMeasuresLine(policy, line), "zero-width lines do not contribute");
  line.width = 1000;
  ok &= expect(!pageLineRule(policy, line).justify && pageLineRule(policy, line).features == std::vector<std::string>{"bism"}, "conditional basmala rule wins");
  line.basm2 = false;
  ok &= expect(pageLineRule(policy, line).features == std::vector<std::string>{"basm"}, "basmala fallback uses declared feature");
  changed = fixture();
  changed.branches[4].steps[1].features = {"demo"};
  ok &= expect(pageLineRule(compilePageJustificationPolicy(changed), line).features == std::vector<std::string>{"demo"}, "special-line feature is data");
  line.lineType = LineType::Sura;
  ok &= expect(!pageLineRule(policy, line).justify && pageLineRule(policy, line).features.empty(), "sura uses natural treatment");
  line.lineType = LineType::Line;
  ok &= expect(pageLineRule(policy, line).justify, "body delegates to linepolicy");

  JustResultByLine result;
  result.xScale = 0.8;
  const auto& fontRule = pageRenderRule(policy, JustStyle::FontSize);
  ok &= expect(near(pageLineEmScale(fontRule, 10, 1, result), 8) && result.xScale == 1, "font_scale consumes the residual before shaping");
  result.xScale = 0.8;
  ok &= expect(near(pageLineEmScale(fontRule, 10, 0.98, result), 9.8) && near(result.xScale, 0.8), "page size takes priority over per-line font scaling");
  const auto& axisRule = pageRenderRule(policy, JustStyle::SCLX);
  ok &= expect(pageLineEmScale(axisRule, 10, 1, result) == 10 && near(result.xScale, 0.8), "axis output leaves shaping scale unchanged");
  LineLayoutInfo output{};
  output.glyphs.push_back({});
  output.glyphs[0].parameters.set(ThirdAxis, 4);
  output.type = LineType::Line;
  output.fontSize = 10;
  result.sclxAxis = 90;
  finishPageLine(axisRule, result, output);
  ok &= expect(near(output.fontSize, 8) && output.xscale == 1 && output.glyphs[0].parameters.value(ScaleXAxis) == 90 && output.glyphs[0].parameters.value(ThirdAxis) == 4, "axis output stores scale without replacing other glyph parameters");
  finishPageLine(pageRenderRule(policy, JustStyle::XScale), result, output);
  ok &= expect(near(output.xscale, 0.8), "xscale output exposes body residual");
  finishPageLine(pageRenderRule(policy, JustStyle::None), result, output);
  ok &= expect(output.xscale == 1, "normal output resets body xscale");
  output.type = LineType::Sura;
  finishPageLine(pageRenderRule(policy, JustStyle::XScale), result, output);
  ok &= expect(output.xscale == 1, "xscale output does not stretch headings");

  ok &= expect(rejects([](auto& s) { s.measureTypes.clear(); }), "reject empty measurement set");
  ok &= expect(rejects([](auto& s) { s.measureTypes = {"Unknown"}; }), "reject unknown measured type");
  ok &= expect(rejects([](auto& s) { s.branches[1].steps[0].arguments = {1.1, 1.2, 0.02, 0.02}; }), "reject invalid threshold interval");
  ok &= expect(rejects([](auto& s) { s.branches[1].steps[0].arguments[2] = 1; }), "reject zero/negative font-size possibility");
  ok &= expect(rejects([](auto& s) { s.branches[1].steps[0].arguments[0] = std::numeric_limits<double>::quiet_NaN(); }), "reject non-finite argument");
  ok &= expect(rejects([](auto& s) { s.branches[0].value = "Unknown"; }), "reject unknown style");
  ok &= expect(rejects([](auto& s) { s.branches[0].steps[0].arguments.clear(); }), "reject bad arity");
  ok &= expect(rejects([](auto& s) { s.branches[3].condition = "typo"; }), "reject unknown condition");
  ok &= expect(rejects([](auto& s) { std::swap(s.branches[3], s.branches[4]); }), "reject hidden conditional branch");
  ok &= expect(rejects([](auto& s) { s.branches[3].steps[1].features = {"too_long"}; }), "reject malformed feature");
  ok &= expect(rejects([](auto& s) { s.branches[3].steps.push_back({"use_line_policy", {}, {}}); }), "reject ambiguous line treatment");
  ok &= expect(rejects([](auto& s) { s.branches[8].steps[0].operation = "typo"; }), "reject unknown output operation");
  ok &= expect(rejects([](auto& s) { s.branches.pop_back(); }), "require render default");
  ok &= expect(rejects([](auto& s) { std::swap(s.branches[0], s.branches[2]); }), "reject hidden sizing branch");
  return ok ? 0 : 1;
}
