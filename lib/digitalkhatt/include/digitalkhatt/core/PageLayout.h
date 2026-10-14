#pragma once

#include <cmath>
#include <stdexcept>

namespace digitalkhatt {

// Geometry is independent of the selected font-size ratio. Margins and
// baseline spacing are in font units (1000 units = 1 em).
struct PageLayout {
  int pageWidth = 17000;
  int pageHeight = 27400;
  int leftMargin = 300;
  int rightMargin = 300;
  int topSpace = 1450;  // Distance from page top to the first baseline.
  int bottomMargin = 750;
  int lineSpacing = 1800;
  double paperWidthMM = 90.2;
  double paperHeightMM = paperWidthMM * pageHeight / pageWidth;

  void matchBottomMarginToHeight() {
    if (pageHeight >= 1 && pageHeight <= 100000 && topSpace >= 0 && topSpace <= 100000 && lineSpacing >= 1 && lineSpacing <= 100000)
      bottomMargin = pageHeight - lastBaseline(15);
  }
  void setPageHeight(int height) {
    pageHeight = height;
    matchBottomMarginToHeight();
    matchPaperAspectRatio();
  }
  void setBottomMargin(int margin) {
    if (margin < 0 || margin > 100000 || topSpace < 0 || topSpace > 100000 || lineSpacing < 1 || lineSpacing > 100000)
      throw std::invalid_argument("Invalid bottom margin or baseline geometry");
    bottomMargin = margin;
    pageHeight = lastBaseline(15) + margin;
    matchPaperAspectRatio();
  }

  void matchPaperAspectRatio() {
    if (pageWidth > 0 && pageHeight > 0)
      paperHeightMM = paperWidthMM * pageHeight / pageWidth;
  }
  void setPaperWidthMM(double width) {
    paperWidthMM = width;
    matchPaperAspectRatio();
  }
  void setPaperHeightMM(double height) {
    paperHeightMM = height;
    if (pageWidth > 0 && pageHeight > 0)
      paperWidthMM = height * pageWidth / pageHeight;
  }

  int textWidth() const { return pageWidth - leftMargin - rightMargin; }
  int lastBaseline(int lines) const { return topSpace + (lines - 1) * lineSpacing; }
  bool fitsLines(int lines) const { return lastBaseline(lines) <= pageHeight - bottomMargin; }

  void validate() const {
    if (pageWidth < 1 || pageWidth > 100000 || pageHeight < 1 || pageHeight > 100000 ||
        leftMargin < 0 || rightMargin < 0 || leftMargin >= pageWidth || rightMargin >= pageWidth || textWidth() < 1 ||
        topSpace < 0 || bottomMargin < 0 || topSpace >= pageHeight || bottomMargin >= pageHeight ||
        topSpace + bottomMargin >= pageHeight ||
        lineSpacing < 1 || lineSpacing > pageHeight ||
        !std::isfinite(paperWidthMM) || !std::isfinite(paperHeightMM) ||
        paperWidthMM < 10 || paperWidthMM > 2000 || paperHeightMM < 10 || paperHeightMM > 2000)
      throw std::invalid_argument("Invalid page layout: check page size, margins and line spacing");
  }
};

}  // namespace digitalkhatt
