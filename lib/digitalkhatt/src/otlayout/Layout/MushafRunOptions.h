#pragma once
#include <string>
#include <vector>
#include "OtLayout.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt {

// Portable snapshot of Generate Mushaf options. The CLI reads this JSON and
// applies command-line overrides; the editor exports its current selections.
struct MushafRunOptions {
  std::string font;
  std::string layout = "qpc_v2_layout";
  std::string justifier = "harfbuzz";
  std::string style = "same-size-by-page";
  std::string shrink = "none";
  std::string features = "features.fea";
  std::string database, resources, pdfResources;
  std::string stretchPolicy, shrinkPolicy;
  std::vector<std::string> disabledLookups;
  int lineSpacing = OtLayout::DefaultInterLineSpacing;
  int textWidth = OtLayout::TextWidth;
  double emScale = 1.0;
  double pageWidthMM = 90.2, pageHeightMM = 144.5;
  bool force = false, tajweed = false, notice = true;
  bool report = false, pdf = true;
  int firstPage = 1, lastPage = 0;
  int summaryLimit = 1000;  // legacy alias of xpbd.reportMaxFindings in snapshots
  layout::OptParams xpbd;
};

inline std::string justifierName(JustType value) {
  switch (value) {
    case JustType::None: return "none";
    case JustType::HarfBuzz: return "harfbuzz";
    case JustType::Madina: return "madina";
    case JustType::IndoPak: return "indopak";
    case JustType::Experimental: return "experimental";
    case JustType::Experimental2: return "experimental2";
    case JustType::DeclPolicy: return "decl-policy";
    default: throw std::runtime_error("Unsupported justification engine");
  }
}
inline JustType parseJustifier(const std::string& name) {
  for (auto value : {JustType::None, JustType::HarfBuzz, JustType::Madina, JustType::IndoPak,
      JustType::Experimental, JustType::Experimental2, JustType::DeclPolicy})
    if (name == justifierName(value)) return value;
  throw std::runtime_error("Unknown justifier: " + name);
}
inline std::string styleName(JustStyle value) {
  switch (value) {
    case JustStyle::None: return "none";
    case JustStyle::SameSizeByPage: return "same-size-by-page";
    case JustStyle::XScale: return "xscale";
    case JustStyle::FontSize: return "font-size";
    case JustStyle::FontSizeXScale: return "font-size-xscale";
    default: throw std::runtime_error("Unsupported justification style");
  }
}
inline JustStyle parseStyle(const std::string& name) {
  for (auto value : {JustStyle::None, JustStyle::SameSizeByPage, JustStyle::XScale, JustStyle::FontSize, JustStyle::FontSizeXScale})
    if (name == styleName(value)) return value;
  throw std::runtime_error("Unknown style: " + name);
}
inline std::string shrinkName(ShrinkType value) {
  return value == ShrinkType::Standard ? "standard" : value == ShrinkType::Test ? "test" : "none";
}
inline ShrinkType parseShrink(const std::string& name) {
  for (auto value : {ShrinkType::None, ShrinkType::Standard, ShrinkType::Test})
    if (name == shrinkName(value)) return value;
  throw std::runtime_error("Unknown shrink option: " + name);
}
}  // namespace digitalkhatt
