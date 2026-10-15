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

void addAdaptive(DfaPagePolicySource& source, double first = 300, double second = 800, bool relative = false) {
  source.branches.insert(source.branches.begin(), {"sizing", "style", "AdaptiveMargins", "", {{relative ? "relative_margins" : "margins", {first, second}, {}}, {"bounded_fit", {0.95, 1.2, 0.02, 0.02}, {}}}});
}

bool checkRelativeMargins() {
  auto source = fixture();
  addAdaptive(source, 300, 900, true);
  auto policy = compilePageJustificationPolicy(source);
  const auto fit = [&](std::initializer_list<double> natural, int nominal = 16400, int left = 600, int right = 600, double unit = 1) {
    return pageTextFrame(policy, JustStyle::AdaptiveMargins, {natural.begin(), natural.size()}, nominal, left, right, unit);
  };
  bool ok = true;
  ok &= expect(policy.sizing.front().relativeMargins, "relative margin declaration must retain its mode");
  ok &= expect(fit({30000}).width == 17000 && fit({30000}).rightOffset == -300, "decreasing both margins widens the nominal frame by twice the allowed decrease");
  ok &= expect(fit({10000}).width == 14600 && fit({10000}).rightOffset == 900, "increasing both margins narrows the nominal frame by twice the allowed increase");
  ok &= expect(fit({15000}).width == 16400 && fit({15000}).rightOffset == 0, "an acceptable relative frame keeps the nominal layout");
  ok &= expect(fit({10000, 30000}).width == 16400 && fit({}).width == 16400, "conflicting and empty relative pages retain nominal width");
  ok &= expect(fit({10000}, 18000, 900, 900).width == 16200 && fit({30000}, 18000, 900, 900).width == 18600, "the same policy adapts to different nominal geometry");
  ok &= expect(fit({30000}, 16400, 200, 800).width == 16800 && fit({30000}, 16400, 200, 800).rightOffset == -200, "a shared decrease cannot make the smaller nominal margin negative");
  ok &= expect(fit({15000}, 16400, 400, 800).rightOffset == 0 && fit({10000}, 16400, 400, 800).rightOffset == 900, "relative adjustments preserve asymmetric nominal centering");
  ok &= expect(fit({30000}, 16400, 0, 1200).width == 16400, "a zero nominal margin prevents widening beyond the page");
  const auto scaled = fit({10000 * 16}, 16400 * 16, 600, 600, 16);
  ok &= expect(scaled.width == 14600 * 16 && scaled.rightOffset == 900 * 16, "relative physical adjustments are independent of coordinate scale");
  source = fixture();
  addAdaptive(source, 100, 100, true);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({14350, 20864}).width == 16600 && fit({14350, 20864}).rightOffset == -100, "page 577 respects the edited 100-unit decrease");
  ok &= expect(fit({11709, 10967, 13706, 11977, 12896, 13040}).width == 16200 && fit({11709, 10967, 13706, 11977, 12896, 13040}).rightOffset == 100, "page 604 respects the edited 100-unit increase");
  source = fixture();
  addAdaptive(source, 300.5, 900.5, true);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({10000}).width == 14600 && fit({30000}).width == 17000, "fractional relative adjustments round inward");
  source = fixture();
  addAdaptive(source, 0, 0, true);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({30000}).width == 16400 && fit({10000}).width == 16400, "zero adjustments disable frame movement");
  source = fixture();
  addAdaptive(source, 900, 300, true);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({10000}).width == 15800, "relative decrease may exceed increase; they are not sorted absolute bounds");
  bool rejected = false;
  try { (void)fit({10000}, 400); } catch (const std::invalid_argument&) { rejected = true; }
  ok &= expect(rejected, "reject relative increases that consume the nominal text frame");
  return ok;
}

bool checkAdaptive() {
  auto source = fixture();
  addAdaptive(source);
  auto policy = compilePageJustificationPolicy(source);
  bool ok = true;
  const auto fit = [&](std::initializer_list<double> natural, int nominal = 15800, int left = 600, int right = 600, double unit = 1) {
    return pageTextFrame(policy, JustStyle::AdaptiveMargins, {natural.begin(), natural.size()}, nominal, left, right, unit);
  };
  ok &= expect(fit({15000, 16000}).width == 15800, "acceptable page keeps nominal width");
  ok &= expect(fit({17200}).width == 16340 && fit({17200}).rightOffset == -270, "tight page widens when a common permitted target exists");
  ok &= expect(fit({13000}).width == 15600 && fit({13000}).rightOffset == 100, "loose page narrows when a common permitted target exists");
  ok &= expect(fit({17000}).width == 16150, "choose nearest nominal width inside the preferred ratio interval");
  const auto conflict = fit({12000, 18000});
  ok &= expect(conflict.width == 15800 && conflict.rightOffset == 0, "conflicting lines retain the nominal frame");
  ok &= expect(fit({18000}).width == 16400 && fit({18000}).rightOffset == -300, "compatible tight lines widen up to the minimum margin");
  ok &= expect(fit({12000}).width == 15400 && fit({12000}).rightOffset == 200, "compatible loose lines narrow up to the maximum margin");
  ok &= expect(fit({10000, 20000}, 15000, 1000, 1000).width == 15400, "conflicting page clamps a too-narrow nominal frame to the margin bound");
  ok &= expect(fit({10000, 20000}, 17000, 100, 100).width == 16600, "conflicting page clamps a too-wide nominal frame to the margin bound");
  ok &= expect(fit({}).width == 15800 && fit({0, -1, std::numeric_limits<double>::quiet_NaN()}).width == 15800, "empty or invalid measurements keep a bounded nominal width");
  ok &= expect(fit({}, 15000, 600, 600).width == 15000, "empty page uses its actual frame geometry");
  ok &= expect(fit({15000}, 15800, 400, 800).rightOffset == -200, "asymmetric nominal margins become centered");
  const auto scaled = fit({17200 * 16}, 15800 * 16, 600, 600, 16);
  ok &= expect(scaled.width == 16340 * 16 && scaled.rightOffset == -270 * 16, "physical margins scale with coordinates, not font size");
  const auto legacy = pageTextFrame(policy, JustStyle::FontSizeXScale, {}, 15800, 600, 600, 1);
  ok &= expect(legacy.width == 15800 && legacy.rightOffset == 0, "legacy fitting never changes the frame");
  source = fixture();
  addAdaptive(source, 300, 900);
  policy = compilePageJustificationPolicy(source);
  const auto page577 = fit({17316, 16179, 16305, 14350, 17558, 17325, 19292, 20864, 19046, 19646, 19061, 18137, 17848}, 16400);
  ok &= expect(page577.width == 17000 && page577.rightOffset == -300, "all page-577 body lines agree on the widest achievable frame after individual interval clamping");
  ok &= expect(fit({14350, 20864}, 16400).width == 17000 && fit({20864, 14350}, 16400).width == 17000, "an unreachable tight preference can agree with a shorter line regardless of measurement order");
  ok &= expect(fit({10000, 20000}, 16400).width == 16400, "opposite preferred margin limits still retain nominal width");
  ok &= expect(fit({8000, 10500}, 16400).width == 15800, "disjoint raw intervals both below the permitted frame agree on the narrowest width");
  ok &= expect(fit({20000, 30000}, 16400).width == 17000, "disjoint raw intervals both above the permitted frame agree on the widest width");
  const auto page577Scaled = fit({14350 * 16, 20864 * 16}, 16400 * 16, 600, 600, 16);
  ok &= expect(page577Scaled.width == 17000 * 16 && page577Scaled.rightOffset == -300 * 16, "individual interval clamping is independent of coordinate scale");
  const auto page604 = fit({11709, 10967, 13706, 11977, 12896, 13040}, 16400);
  ok &= expect(page604.width == 15800 && page604.rightOffset == 300, "page 604 uses the nearest margin bound when its line preferences overlap outside the permitted frame");
  const auto page604Scaled = fit({11709 * 16, 10967 * 16, 13706 * 16, 11977 * 16, 12896 * 16, 13040 * 16}, 16400 * 16, 600, 600, 16);
  ok &= expect(page604Scaled.width == 15800 * 16 && page604Scaled.rightOffset == 300 * 16, "page 604 frame selection is independent of coordinate scale");
  source = fixture();
  addAdaptive(source, 300, 2500);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({10000, 20000}).width == 15800, "mixed page retains nominal width rather than minimizing a penalty");
  const auto conflictScaled = fit({10000 * 16, 20000 * 16}, 15800 * 16, 600, 600, 16);
  ok &= expect(conflictScaled.width == 15800 * 16 && conflictScaled.rightOffset == 0, "nominal fallback is independent of coordinate scale");
  std::vector<double> majorityLoose(14, 0.89 * 15800);
  majorityLoose.push_back(1.1 * 15800);
  ok &= expect(pageTextFrame(policy, JustStyle::AdaptiveMargins, majorityLoose, 15800, 600, 600, 1).width == 16400, "a mixed page widens up to the margin bound when loose and tight lines have a common preferred width");
  std::fill(majorityLoose.begin(), majorityLoose.end() - 1, 0.85 * 15800);
  ok &= expect(pageTextFrame(policy, JustStyle::AdaptiveMargins, majorityLoose, 15800, 600, 600, 1).width == 15800, "fourteen incompatible loose lines cannot force a compromise against one tight line");
  source = fixture();
  addAdaptive(source, 300.5, 800.5);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({10000, 20000}, 18000, 100, 100).width == 17598 && fit({1000, 20000}, 10000, 1000, 1000).width == 10400, "fractional bounds round inward when nominal fallback is outside the permitted frame");
  ok &= expect(fit({10000, 20000}, 10000, 1000, 1000).width == 11398, "fractional maximum is shared when a shorter line accepts the tight line's best achievable width");
  source = fixture();
  addAdaptive(source, 600, 600);
  policy = compilePageJustificationPolicy(source);
  ok &= expect(fit({12000, 20000}).width == 15800, "equal margin bounds force a fixed frame");
  source = fixture();
  addAdaptive(source, 0, 9000);
  policy = compilePageJustificationPolicy(source);
  bool rejected = false;
  try { (void)fit({15000}); } catch (const std::invalid_argument&) { rejected = true; }
  ok &= expect(rejected, "reject margins that consume the page");
  rejected = false;
  try { (void)pageFontRatio(compilePageJustificationPolicy(fixture()), JustStyle::AdaptiveMargins, {}); } catch (const std::invalid_argument&) { rejected = true; }
  ok &= expect(rejected, "unsupported fonts must not silently use the default sizing rule");
  return ok;
}

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
  ok &= checkAdaptive();
  ok &= checkRelativeMargins();
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
  ok &= expect(rejects([](auto& s) { addAdaptive(s, -1, 800); }), "reject negative minimum margin");
  ok &= expect(rejects([](auto& s) { addAdaptive(s, 800, 300); }), "reject inverted margins");
  ok &= expect(rejects([](auto& s) { addAdaptive(s, -1, 900, true); }), "reject negative relative decrease");
  ok &= expect(rejects([](auto& s) { addAdaptive(s, 300, -1, true); }), "reject negative relative increase");
  ok &= expect(rejects([](auto& s) { addAdaptive(s, 300, std::numeric_limits<double>::infinity(), true); }), "reject nonfinite relative adjustments");
  ok &= expect(rejects([](auto& s) { addAdaptive(s); s.branches[0].steps.push_back({"relative_margins", {300, 900}, {}}); }), "reject mixing absolute and relative margins");
  ok &= expect(rejects([](auto& s) { addAdaptive(s); s.branches[0].steps.pop_back(); }), "adaptive fitting needs bounded_fit");
  ok &= expect(rejects([](auto& s) { addAdaptive(s); s.branches[0].steps.erase(s.branches[0].steps.begin()); }), "adaptive fitting needs margin bounds");
  ok &= expect(rejects([](auto& s) { addAdaptive(s); s.branches[0].steps.push_back({"margins", {300, 800}, {}}); }), "reject duplicate margin bounds");
  ok &= expect(rejects([](auto& s) { s.branches[1].steps.push_back({"margins", {300, 800}, {}}); }), "margin bounds cannot alter FontSizeXScale");
  ok &= expect(rejects([](auto& s) { addAdaptive(s); s.branches[0].steps.push_back({"penalty_weights", {4, 1}, {}}); }), "reject removed penalty_weights construct");
  return ok ? 0 : 1;
}
