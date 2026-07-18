#define NOMINMAX

#include "OtLayout.h"

#include <digitalkhatt/justify/FeatureJustifier.h>

namespace {

// Adapts the real (Qt) OtLayout to the small interface digitalkhatt::justify::FeatureJustifier
// needs, so the justification/regex algorithm itself (lib/digitalkhatt/src/justify/FeatureJustifier.cpp)
// has no Qt dependency.
class QtOtLayoutFontProvider final : public digitalkhatt::justify::FeatureJustificationLayout {
 public:
  explicit QtOtLayoutFontProvider(OtLayout& layout) : layout_(layout) {}

  hb_font_t* createFont(double scale, bool newFace) override {
    return layout_.createFont(scale, newFace);
  }
  int scaleBy() const override { return OtLayout::SCALEBY; }
  int topSpace() const override { return OtLayout::TopSpace; }
  int interLineSpacing() const override { return OtLayout::InterLineSpacing; }

 private:
  OtLayout& layout_;
};

}  // namespace

std::vector<LineLayoutInfo> OtLayout::justifyPageUsingFeatures(double emScale, int pageWidth, const std::vector<LineToJustify>& lines,
                                                               bool newFace, bool tajweedColor,
                                                               hb_buffer_cluster_level_t cluster_level,
                                                               JustOption justOption, std::string mushafLayout) {

  QtOtLayoutFontProvider fontProvider(*this);
  digitalkhatt::justify::FeatureJustifier justifier(fontProvider);

  return justifier.justifyPageUsingFeatures(emScale, pageWidth, lines, newFace, tajweedColor, cluster_level,
                                            justOption, mushafLayout);
}
