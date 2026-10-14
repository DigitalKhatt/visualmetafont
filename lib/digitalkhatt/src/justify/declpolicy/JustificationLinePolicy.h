#pragma once

#include <span>

#include "../FeatureJustifierInternal.h"

namespace digitalkhatt::justify::decl {

// The line interpreter knows policy and metrics, not HarfBuzz or the font
// adapter. Its backend owns shaping, axis evaluation and glyph-level staging.
class LineJustificationBackend {
 public:
  virtual ~LineJustificationBackend() = default;
  virtual double measure(const std::vector<runtime::TextFontFeatures>& candidateFeatures, const std::vector<runtime::TextFontFeatures>& currentFeatures, double currentWidth) = 0;
  virtual double applyStage(std::span<const PolicyPhase> phases, const std::vector<runtime::TextFontFeatures>& globalFeatures, double currentWidth) = 0;
  virtual double measureSclx(float value) = 0;
};

struct LineJustificationMetrics {
  double targetWidth;
  double currentWidth;
  double spaceWidth;
  std::size_t simpleSpaces;
  std::size_t ayaSpaces;
};

runtime::JustResultByLine executeLineJustificationPolicy(std::span<const LinePolicyStep> stretchSteps, std::span<const LinePolicyStep> shrinkSteps, LineJustificationMetrics metrics, LineJustificationBackend& backend);

}  // namespace digitalkhatt::justify::decl
