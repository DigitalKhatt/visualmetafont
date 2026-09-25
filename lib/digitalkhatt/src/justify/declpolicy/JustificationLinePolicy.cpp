#include "JustificationLinePolicy.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string_view>

namespace digitalkhatt::justify {

LinePolicyStep compileLinePolicyStep(const DfaLineStepSource& step, bool stretch) {
  struct Operation {
    std::string_view name;
    LineStepOp code;
    std::size_t arguments;
    bool features;
    bool allowStretch;
    bool allowShrink;
  };
  static constexpr Operation operations[] = {
      {"cap_spaces", LineStepOp::CapSpaces, 2, false, true, false},
      {"stage", LineStepOp::Stage, 0, false, true, false},
      {"fill_spaces", LineStepOp::FillSpaces, 0, false, true, false},
      {"fit_features", LineStepOp::FitFeatures, 0, true, true, true},
      {"all_features", LineStepOp::AllFeatures, 0, true, false, true},
      {"fit_sclx", LineStepOp::FitSclx, 1, false, false, true},
      {"balance", LineStepOp::Balance, 0, false, false, true},
      {"scale", LineStepOp::Scale, 0, false, false, true},
  };
  const auto operation = std::find_if(std::begin(operations), std::end(operations), [&](const auto& entry) { return entry.name == step.operation; });
  if (operation == std::end(operations)) throw std::invalid_argument("unknown linepolicy operation " + step.operation);
  if ((stretch && !operation->allowStretch) || (!stretch && !operation->allowShrink)) throw std::invalid_argument("linepolicy operation in wrong branch: " + step.operation);
  if (step.arguments.size() != operation->arguments || operation->features != !step.features.empty()) throw std::invalid_argument("invalid linepolicy arguments for " + step.operation);
  if ((operation->code == LineStepOp::Stage) != !step.stage.empty()) throw std::invalid_argument("invalid stage reference in " + step.operation);
  for (double value : step.arguments) {
    if (!std::isfinite(value) || value < 0 || (operation->code == LineStepOp::FitSclx && value == 0)) throw std::invalid_argument("invalid linepolicy numeric argument for " + step.operation);
  }
  std::set<std::string> tags;
  for (const auto& tag : step.features) {
    if (tag.size() != 4 || !std::all_of(tag.begin(), tag.end(), [](unsigned char c) { return c >= 32 && c <= 126; })) throw std::invalid_argument("invalid linepolicy feature tag " + tag);
    if (!tags.insert(tag).second) throw std::invalid_argument("duplicate linepolicy feature " + tag);
  }
  return {operation->code, step.arguments, step.features, step.stage, {}};
}

LineJustificationPolicy compileLineJustificationPolicy(const DfaLinePolicySource& source) {
  if (source.stretchPolicy.empty() || source.shrinkPolicy.empty()) throw std::invalid_argument("linepolicy requires named stretch and shrink policies");
  return {.stretchPolicy = source.stretchPolicy, .shrinkPolicy = source.shrinkPolicy};
}

namespace decl {

runtime::JustResultByLine executeLineJustificationPolicy(std::span<const LinePolicyStep> stretchSteps, std::span<const LinePolicyStep> shrinkSteps, LineJustificationMetrics metrics, LineJustificationBackend& backend) {
  runtime::JustResultByLine result;
  result.xScale = 1;
  result.simpleSpacing = result.ayaSpacing = metrics.spaceWidth;
  const bool stretching = metrics.currentWidth < metrics.targetWidth;
  const std::span<const LinePolicyStep> steps = stretching ? stretchSteps : shrinkSteps;
  const auto spaceCount = metrics.simpleSpaces + metrics.ayaSpaces;
  const auto scale = [&] { result.xScale = metrics.currentWidth != 0 ? metrics.targetWidth / metrics.currentWidth : 1; };
  const auto fill = [&] {
    if (metrics.targetWidth <= metrics.currentWidth) return;
    if (spaceCount == 0) {
      scale();
      return;
    }
    const double added = (metrics.targetWidth - metrics.currentWidth) / spaceCount;
    if (result.isShrink)
      result.addedSpaceAfterShrink += added;
    else {
      result.simpleSpacing += added;
      result.ayaSpacing += added;
    }
    metrics.currentWidth = metrics.targetWidth;
  };
  for (const auto& step : steps) {
    switch (step.operation) {
      case LineStepOp::CapSpaces: {
        const double simple = std::max(step.arguments[0] - result.simpleSpacing, 0.0);
        const double aya = std::max(step.arguments[1] - result.ayaSpacing, 0.0);
        const double capacity = simple * metrics.simpleSpaces + aya * metrics.ayaSpaces;
        const double added = std::min(std::max(metrics.targetWidth - metrics.currentWidth, 0.0), capacity);
        const double ratio = capacity != 0 ? added / capacity : 0;
        result.simpleSpacing += ratio * simple;
        result.ayaSpacing += ratio * aya;
        metrics.currentWidth += added;
        break;
      }
      case LineStepOp::Stage:
        if (metrics.targetWidth > metrics.currentWidth) metrics.currentWidth = backend.applyStage(step.phases, result.globalFeatures, metrics.currentWidth);
        break;
      case LineStepOp::FillSpaces:
        fill();
        break;
      case LineStepOp::AllFeatures:
        for (const auto& tag : step.features) result.globalFeatures.push_back({tag, 1});
        result.isShrink = true;
        break;
      case LineStepOp::FitFeatures: {
        if (stretching) {
          // A stretch feature is a fixed, standard-OpenType step. Keep only
          // features whose measured positive delta still fits; each later
          // trial starts from the already accepted feature set.
          for (const auto& tag : step.features) {
            if (metrics.currentWidth >= metrics.targetWidth) break;
            auto features = result.globalFeatures;
            features.push_back({tag, 1});
            const double width = backend.measure(features, result.globalFeatures, metrics.currentWidth);
            if (width > metrics.currentWidth && width <= metrics.targetWidth) {
              metrics.currentWidth = width;
              result.globalFeatures.push_back({tag, 1});
            }
          }
          break;
        }
        // Trials are cumulative, including non-reducing trials. Only reducing
        // steps are kept for final shaping: preserve the reference semantics.
        auto features = result.globalFeatures;
        for (const auto& tag : step.features) {
          if (metrics.currentWidth <= metrics.targetWidth) break;
          features.push_back({tag, 1});
          const double width = backend.measure(features, result.globalFeatures, metrics.currentWidth);
          if (width < metrics.currentWidth) {
            metrics.currentWidth = width;
            result.globalFeatures.push_back({tag, 1});
          }
        }
        result.isShrink = true;
        break;
      }
      case LineStepOp::FitSclx: {
        if (metrics.currentWidth == 0) break;
        const float ratio = metrics.targetWidth / metrics.currentWidth;
        const float value = ratio * step.arguments[0];
        const double width = backend.measureSclx(value);
        if (width < metrics.currentWidth && width > 0) {
          result.sclxAxis = value;
          metrics.currentWidth = width;
          scale();
        }
        break;
      }
      case LineStepOp::Balance:
        result.xScale = 1;
        if (metrics.currentWidth < metrics.targetWidth)
          fill();
        else
          scale();
        break;
      case LineStepOp::Scale:
        scale();
        break;
    }
  }
  return result;
}

}  // namespace decl
}  // namespace digitalkhatt::justify
