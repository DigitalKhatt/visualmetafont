#pragma once

#include <string>
#include <vector>

#include <hb.h>

#include "digitalkhatt/core/digitalkahtt_types.h"

namespace digitalkhatt::justify {

class FeatureJustificationLayout {
 public:
  virtual ~FeatureJustificationLayout() = default;

  virtual hb_font_t* createFont(double scale, bool newFace) = 0;
  virtual int scaleBy() const = 0;
  virtual int topSpace() const = 0;
  virtual int interLineSpacing() const = 0;
};

class FeatureJustifier {
 public:
  explicit FeatureJustifier(FeatureJustificationLayout& layout) : layout_(layout) {}

  std::vector<LineLayoutInfo> justifyPageUsingFeatures(
      double emScale,
      int pageWidth,
      const std::vector<LineToJustify>& lines,
      bool newFace,
      bool tajweedColor,
      hb_buffer_cluster_level_t clusterLevel,
      JustOption justOption,
      const std::string& mushafLayout) const;

 private:
  FeatureJustificationLayout& layout_;
};

}  // namespace digitalkhatt::justify
