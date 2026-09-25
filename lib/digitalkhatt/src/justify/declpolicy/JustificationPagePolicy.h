#pragma once

#include "../FeatureJustifierInternal.h"

namespace digitalkhatt::justify::decl {

// Pure policy decisions. Font allocation, text measurement and final shaping
// belong to the page adapter, not this interpreter.
bool pageMeasuresLine(const PageJustificationPolicy& policy, const LineToJustify& line);
double pageFontRatio(const PageJustificationPolicy& policy, JustStyle style, std::span<const double> ratios);
const PageLineRule& pageLineRule(const PageJustificationPolicy& policy, const LineToJustify& line);
const PageRenderRule& pageRenderRule(const PageJustificationPolicy& policy, JustStyle style);
double pageLineEmScale(const PageRenderRule& rule, double emScale, double pageRatio, runtime::JustResultByLine& result);
void finishPageLine(const PageRenderRule& rule, const runtime::JustResultByLine& result, LineLayoutInfo& line);

}  // namespace digitalkhatt::justify::decl
