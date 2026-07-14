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

QList<LineLayoutInfo> OtLayout::justifyPageUsingFeatures(double emScale, int pageWidth, const QVector<LineToJustify>& lines,
                                                         bool newFace, bool tajweedColor,
                                                         hb_buffer_cluster_level_t cluster_level,
                                                         JustOption justOption, QString mushafLayout) {
  std::vector<LineToJustify> coreLines;
  coreLines.reserve(lines.size());
  for (const auto& line : lines) {
    coreLines.push_back(line);
  }

  QtOtLayoutFontProvider fontProvider(*this);
  digitalkhatt::justify::FeatureJustifier justifier(fontProvider);

  auto coreResult = justifier.justifyPageUsingFeatures(emScale, pageWidth, coreLines, newFace, tajweedColor, cluster_level,
                                                       justOption, mushafLayout.toStdString());

  QList<LineLayoutInfo> page;
  page.reserve(static_cast<int>(coreResult.size()));
  for (const auto& line : coreResult) {
    page.append(std::move(line));
  }

  return page;
}
