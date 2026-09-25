#include "JustificationPagePolicy.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <tuple>

namespace digitalkhatt::justify {
namespace {

JustStyle styleValue(const std::string& value) {
  if (value == "None") return JustStyle::None;
  if (value == "SameSizeByPage") return JustStyle::SameSizeByPage;
  if (value == "XScale") return JustStyle::XScale;
  if (value == "FontSize") return JustStyle::FontSize;
  if (value == "FontSizeXScale") return JustStyle::FontSizeXScale;
  if (value == "SCLX") return JustStyle::SCLX;
  throw std::invalid_argument("unknown pagepolicy style " + value);
}

LineType typeValue(const std::string& value) {
  if (value == "Line") return LineType::Line;
  if (value == "Sura") return LineType::Sura;
  if (value == "Bism") return LineType::Bism;
  throw std::invalid_argument("unknown pagepolicy line type " + value);
}

void arguments(const DfaLineStepSource& step, std::size_t count, bool features = false) {
  if (step.arguments.size() != count || features != !step.features.empty()) throw std::invalid_argument("invalid pagepolicy arguments for " + step.operation);
  for (double value : step.arguments)
    if (!std::isfinite(value)) throw std::invalid_argument("non-finite pagepolicy argument");
}

}  // namespace

PageJustificationPolicy compilePageJustificationPolicy(const DfaPagePolicySource& source) {
  PageJustificationPolicy result;
  for (const auto& name : source.measureTypes) {
    const auto type = typeValue(name);
    if (std::find(result.measureTypes.begin(), result.measureTypes.end(), type) != result.measureTypes.end()) throw std::invalid_argument("duplicate pagepolicy measured type " + name);
    result.measureTypes.push_back(type);
  }
  if (result.measureTypes.empty()) throw std::invalid_argument("pagepolicy needs measured line types");
  std::set<std::tuple<std::string, std::string, std::string>> selectors;
  std::set<std::string> defaults;
  for (const auto& branch : source.branches) {
    const bool line = branch.section == "line";
    if (!line && branch.section != "sizing" && branch.section != "render") throw std::invalid_argument("unknown pagepolicy section " + branch.section);
    if (defaults.contains(branch.section)) throw std::invalid_argument("pagepolicy default hides a later " + branch.section + " branch");
    const bool fallback = branch.selector == "default";
    if (fallback) {
      if (!branch.value.empty() || !branch.condition.empty()) throw std::invalid_argument("pagepolicy default cannot have a condition");
      defaults.insert(branch.section);
    } else if (branch.selector != (line ? "type" : "style")) {
      throw std::invalid_argument("invalid pagepolicy selector " + branch.selector);
    }
    if (!branch.condition.empty() && (!line || branch.condition != "basm2")) throw std::invalid_argument("unknown pagepolicy condition " + branch.condition);
    if (!selectors.emplace(branch.section, branch.value, branch.condition).second) throw std::invalid_argument("duplicate pagepolicy selector");
    if (!branch.condition.empty() && selectors.contains({branch.section, branch.value, ""})) throw std::invalid_argument("pagepolicy type branch hides a later conditional branch");
    if (branch.steps.empty()) throw std::invalid_argument("empty pagepolicy branch");
    if (branch.section == "sizing") {
      if (branch.steps.size() != 1) throw std::invalid_argument("pagepolicy sizing requires one operation");
      const auto& step = branch.steps[0];
      PageSizingRule rule;
      if (!fallback) rule.style = styleValue(branch.value);
      if (step.operation == "fixed_size" || step.operation == "min_fit") {
        arguments(step, 1);
        if (step.arguments[0] <= 0) throw std::invalid_argument("pagepolicy font ratio must be positive");
        rule.operation = step.operation == "fixed_size" ? PageSizingOp::Fixed : PageSizingOp::MinFit;
      } else if (step.operation == "bounded_fit") {
        arguments(step, 4);
        const auto& a = step.arguments;
        if (a[0] <= 0 || a[0] > 1 || a[1] < 1 || a[2] < 0 || a[2] >= 1 || a[3] < 0) throw std::invalid_argument("invalid pagepolicy bounded_fit thresholds or limits");
        rule.operation = PageSizingOp::BoundedFit;
      } else
        throw std::invalid_argument("unknown pagepolicy sizing operation " + step.operation);
      rule.arguments = step.arguments;
      result.sizing.push_back(std::move(rule));
    } else if (line) {
      PageLineRule rule;
      if (!fallback) rule.type = typeValue(branch.value);
      rule.requireBasm2 = !branch.condition.empty();
      bool hasTreatment = false;
      std::set<std::string> tags;
      for (const auto& step : branch.steps) {
        if (step.operation == "natural" || step.operation == "use_line_policy") {
          arguments(step, 0);
          if (hasTreatment) throw std::invalid_argument("duplicate pagepolicy line treatment");
          hasTreatment = true;
          rule.justify = step.operation == "use_line_policy";
        } else if (step.operation == "final_features") {
          arguments(step, 0, true);
          for (const auto& tag : step.features) {
            if (tag.size() != 4 || !std::all_of(tag.begin(), tag.end(), [](unsigned char c) { return c >= 32 && c <= 126; }) || !tags.insert(tag).second) throw std::invalid_argument("invalid or duplicate pagepolicy feature " + tag);
            rule.features.push_back(tag);
          }
        } else
          throw std::invalid_argument("unknown pagepolicy line operation " + step.operation);
      }
      if (!hasTreatment) throw std::invalid_argument("pagepolicy line branch needs natural or use_line_policy");
      result.lines.push_back(std::move(rule));
    } else {
      PageRenderRule rule;
      if (!fallback) rule.style = styleValue(branch.value);
      bool hasOutput = false;
      for (const auto& step : branch.steps) {
        arguments(step, 0);
        if (step.operation == "font_scale") {
          if (rule.fontScale) throw std::invalid_argument("duplicate pagepolicy font_scale");
          rule.fontScale = true;
        } else {
          if (hasOutput) throw std::invalid_argument("duplicate pagepolicy output operation");
          hasOutput = true;
          if (step.operation == "normal_output")
            rule.output = PageOutputOp::Normal;
          else if (step.operation == "xscale_output")
            rule.output = PageOutputOp::XScale;
          else if (step.operation == "axis_output")
            rule.output = PageOutputOp::AxisFont;
          else
            throw std::invalid_argument("unknown pagepolicy rendering operation " + step.operation);
        }
      }
      if (!hasOutput) throw std::invalid_argument("pagepolicy rendering needs an output operation");
      result.rendering.push_back(std::move(rule));
    }
  }
  if (defaults.size() != 3) throw std::invalid_argument("pagepolicy requires sizing, line and render defaults");
  return result;
}

namespace decl {

bool pageMeasuresLine(const PageJustificationPolicy& policy, const LineToJustify& line) {
  return line.width != 0 && std::find(policy.measureTypes.begin(), policy.measureTypes.end(), line.lineType) != policy.measureTypes.end();
}

double pageFontRatio(const PageJustificationPolicy& policy, JustStyle style, std::span<const double> ratios) {
  const PageSizingRule* selected = nullptr;
  for (const auto& rule : policy.sizing)
    if (!rule.style || *rule.style == style) {
      selected = &rule;
      break;
    }
  if (!selected) throw std::logic_error("pagepolicy has no matching sizing rule");
  const auto& a = selected->arguments;
  if (selected->operation == PageSizingOp::Fixed) return a[0];
  double minRatio = 0, maxRatio = 0;
  bool hasRatio = false;
  for (double ratio : ratios) {
    if (!std::isfinite(ratio) || ratio <= 0) continue;
    minRatio = hasRatio ? std::min(minRatio, ratio) : ratio;
    maxRatio = hasRatio ? std::max(maxRatio, ratio) : ratio;
    hasRatio = true;
  }
  if (selected->operation == PageSizingOp::MinFit) return hasRatio ? std::min(minRatio, a[0]) : a[0];
  if (!hasRatio) return 1;
  const double lower = a[0], upper = a[1], shrinkLimit = a[2], stretchLimit = a[3];
  if ((maxRatio > upper && minRatio < lower) || (maxRatio <= upper && minRatio >= lower)) return 1;
  if (maxRatio > upper) return 1 + std::min(std::min(maxRatio - upper, stretchLimit), minRatio - lower);
  return 1 - std::min(std::min(lower - minRatio, shrinkLimit), upper - maxRatio);
}

const PageLineRule& pageLineRule(const PageJustificationPolicy& policy, const LineToJustify& line) {
  for (const auto& rule : policy.lines)
    if ((!rule.type || *rule.type == line.lineType) && (!rule.requireBasm2 || line.basm2)) return rule;
  throw std::logic_error("pagepolicy has no matching line rule");
}

const PageRenderRule& pageRenderRule(const PageJustificationPolicy& policy, JustStyle style) {
  for (const auto& rule : policy.rendering)
    if (!rule.style || *rule.style == style) return rule;
  throw std::logic_error("pagepolicy has no matching render rule");
}

double pageLineEmScale(const PageRenderRule& rule, double emScale, double pageRatio, runtime::JustResultByLine& result) {
  if (pageRatio != 1) return emScale * pageRatio;
  if (!rule.fontScale) return emScale;
  const double scale = emScale * result.xScale;
  result.xScale = 1;
  return scale;
}

void finishPageLine(const PageRenderRule& rule, const runtime::JustResultByLine& result, LineLayoutInfo& line) {
  if (rule.output == PageOutputOp::AxisFont) {
    line.fontSize *= result.xScale;
    line.xscale = 1;
    for (auto& glyph : line.glyphs) glyph.parameters.set(ScaleXAxis, result.sclxAxis);
  } else if (line.type == LineType::Line) {
    line.xscale = rule.output == PageOutputOp::XScale ? result.xScale : 1;
  }
}

}  // namespace decl
}  // namespace digitalkhatt::justify
