#pragma once

#include <QDialog>
#include <digitalkhatt/core/PageLayout.h>

class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QCheckBox;

digitalkhatt::PageLayout loadPageLayoutSettings();
void savePageLayoutSettings(const digitalkhatt::PageLayout& layout);

class PageLayoutDialog : public QDialog {
 public:
  explicit PageLayoutDialog(const digitalkhatt::PageLayout& layout, QWidget* parent);
  digitalkhatt::PageLayout pageLayout() const;

 private:
  void setPageLayout(const digitalkhatt::PageLayout& layout);
  void updateSummary();
  void updatePaperSize();
  void updateVerticalGeometry(bool fromBottomMargin);
  QSpinBox *width_, *height_, *left_, *right_, *top_, *bottom_, *spacing_;
  QDoubleSpinBox *paperWidth_, *paperHeight_;
  QLabel* summary_;
  QCheckBox* centerLines_;
  double paperWidthMM_ = 90.2;
};
