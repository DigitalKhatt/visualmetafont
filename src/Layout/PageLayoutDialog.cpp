#include "PageLayoutDialog.h"

#include <algorithm>
#include <QtWidgets>

namespace {
struct IntegerField {
  const char* key;
  int digitalkhatt::PageLayout::* member;
};
constexpr IntegerField fields[] = {
    {"PageWidth", &digitalkhatt::PageLayout::pageWidth},
    {"PageHeight", &digitalkhatt::PageLayout::pageHeight},
    {"LeftMargin", &digitalkhatt::PageLayout::leftMargin},
    {"RightMargin", &digitalkhatt::PageLayout::rightMargin},
    {"TopSpace", &digitalkhatt::PageLayout::topSpace},
    {"BottomMargin", &digitalkhatt::PageLayout::bottomMargin},
    {"InterLineSpacing", &digitalkhatt::PageLayout::lineSpacing}};
}

digitalkhatt::PageLayout loadPageLayoutSettings() {
  digitalkhatt::PageLayout layout;
  QSettings settings;
  settings.beginGroup("Layout");
  for (const auto& field : fields)
    layout.*field.member = settings.value(field.key, layout.*field.member).toInt();
  layout.paperWidthMM = settings.value("PaperWidthMM", layout.paperWidthMM).toDouble();
  layout.paperHeightMM = settings.value("PaperHeightMM", layout.paperHeightMM).toDouble();
  layout.matchBottomMarginToHeight();
  layout.matchPaperAspectRatio();
  try { layout.validate(); }
  catch (const std::exception& error) {
    qWarning() << "Ignoring invalid saved page layout:" << error.what();
    layout = {};
  }
  return layout;
}

void savePageLayoutSettings(const digitalkhatt::PageLayout& layout) {
  layout.validate();
  QSettings settings;
  settings.beginGroup("Layout");
  for (const auto& field : fields) settings.setValue(field.key, layout.*field.member);
  settings.setValue("PaperWidthMM", layout.paperWidthMM);
  settings.setValue("PaperHeightMM", layout.paperHeightMM);
  settings.sync();
}

PageLayoutDialog::PageLayoutDialog(const digitalkhatt::PageLayout& layout, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle(tr("Page layout"));
  auto* main = new QVBoxLayout(this);
  auto* description = new QLabel(tr("Geometry is in font units (1000 units = 1 em). "
      "PDF width and height are linked to this page's aspect ratio."));
  description->setWordWrap(true);
  main->addWidget(description);
  auto* geometry = new QGroupBox(tr("Page geometry"));
  auto* form = new QFormLayout(geometry);
  const auto add = [&](const QString& label, int minimum) {
    auto* box = new QSpinBox;
    box->setRange(minimum, 100000);
    box->setSuffix(tr(" units"));
    box->setKeyboardTracking(false);
    form->addRow(label, box);
    connect(box, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateSummary(); });
    return box;
  };
  width_ = add(tr("Page width"), 1);
  height_ = add(tr("Page height"), 1);
  left_ = add(tr("Left margin"), 0);
  right_ = add(tr("Right margin"), 0);
  top_ = add(tr("Top / first baseline"), 0);
  top_->setToolTip(tr("Distance from the top of the page to the first baseline. Marks can extend above it."));
  bottom_ = add(tr("Bottom margin"), 0);
  bottom_->setToolTip(tr("Distance below the last baseline of a 15-line page. Changing it updates page height."));
  height_->setToolTip(tr("Changing page height updates the bottom margin below the last baseline of a 15-line page."));
  spacing_ = add(tr("Line spacing"), 1);
  spacing_->setToolTip(tr("Baseline distance, used before shaping and collision resolution."));
  main->addWidget(geometry);
  auto* paper = new QGroupBox(tr("PDF paper size"));
  auto* paperForm = new QFormLayout(paper);
  const auto addPaper = [&](const QString& label) {
    auto* box = new QDoubleSpinBox;
    box->setRange(10, 2000);
    box->setDecimals(2);
    box->setSuffix(tr(" mm"));
    box->setKeyboardTracking(false);
    paperForm->addRow(label, box);
    return box;
  };
  paperWidth_ = addPaper(tr("Width"));
  paperHeight_ = addPaper(tr("Height"));
  main->addWidget(paper);
  summary_ = new QLabel;
  summary_->setWordWrap(true);
  main->addWidget(summary_);
  paperWidth_->setToolTip(tr("Changing width updates height using the page geometry's aspect ratio."));
  paperHeight_->setToolTip(tr("Changing height updates width using the page geometry's aspect ratio."));
  connect(paperWidth_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double width) {
    paperWidthMM_ = width;
    updatePaperSize();
  });
  connect(paperHeight_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double height) {
    paperWidthMM_ = height * width_->value() / height_->value();
    updatePaperSize();
  });
  connect(width_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updatePaperSize(); });
  connect(height_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateVerticalGeometry(false); });
  connect(bottom_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateVerticalGeometry(true); });
  connect(top_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateVerticalGeometry(false); });
  connect(spacing_, qOverload<int>(&QSpinBox::valueChanged), this, [this] { updateVerticalGeometry(false); });
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
  main->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this,
      [this] { setPageLayout({}); });
  connect(buttons, &QDialogButtonBox::accepted, this, [this] {
    try {
      const auto layout = pageLayout();
      layout.validate();
      if (!layout.fitsLines(15)) {
        QMessageBox::warning(this, tr("Page layout"), tr("The 15 baselines do not fit above the bottom margin. "
            "Reduce line spacing or increase page height."));
        return;
      }
      accept();
    }
    catch (const std::exception& error) { QMessageBox::warning(this, tr("Page layout"), QString::fromUtf8(error.what())); }
  });
  // Block updates while populating fields; all widgets must exist first.
  setPageLayout(layout);
}

digitalkhatt::PageLayout PageLayoutDialog::pageLayout() const {
  digitalkhatt::PageLayout layout{width_->value(), height_->value(), left_->value(), right_->value(), top_->value(),
      bottom_->value(), spacing_->value()};
  layout.setPaperWidthMM(paperWidthMM_);
  return layout;
}

void PageLayoutDialog::setPageLayout(const digitalkhatt::PageLayout& layout) {
  const QSignalBlocker a(width_), b(height_), c(left_), d(right_), e(top_), f(bottom_), g(spacing_),
      h(paperWidth_), i(paperHeight_);
  width_->setValue(layout.pageWidth); height_->setValue(layout.pageHeight);
  left_->setValue(layout.leftMargin); right_->setValue(layout.rightMargin);
  top_->setValue(layout.topSpace); bottom_->setValue(layout.bottomMargin);
  spacing_->setValue(layout.lineSpacing);
  paperWidthMM_ = layout.paperWidthMM;
  updateVerticalGeometry(false);
}

void PageLayoutDialog::updateVerticalGeometry(bool fromBottomMargin) {
  const QSignalBlocker heightBlock(height_), bottomBlock(bottom_);
  const int baseline = top_->value() + 14 * spacing_->value();
  const int desiredHeight = fromBottomMargin ? baseline + bottom_->value()
                                              : std::max(height_->value(), baseline);
  height_->setValue(desiredHeight);
  bottom_->setValue(std::max(0, height_->value() - baseline));
  updatePaperSize();
}

void PageLayoutDialog::updatePaperSize() {
  const QSignalBlocker widthBlock(paperWidth_), heightBlock(paperHeight_);
  const double ratio = double(height_->value()) / width_->value();
  const double minimumWidth = std::max(10.0, 10.0 / ratio);
  const double maximumWidth = std::min(2000.0, 2000.0 / ratio);
  if (minimumWidth <= maximumWidth)
    paperWidthMM_ = std::clamp(paperWidthMM_, minimumWidth, maximumWidth);
  const auto layout = pageLayout();
  // Keep full precision in the model; the millimeter controls display two decimals.
  paperWidth_->setValue(layout.paperWidthMM);
  paperHeight_->setValue(layout.paperHeightMM);
  updateSummary();
}

void PageLayoutDialog::updateSummary() {
  const auto layout = pageLayout();
  QString message = tr("Text width: %1 units. Last baseline on a 15-line page: %2 units.")
      .arg(layout.textWidth()).arg(layout.lastBaseline(15));
  if (!layout.fitsLines(15)) message += tr("\nThe last baseline exceeds the bottom margin by %1 units. "
      "Reduce spacing or increase page height.").arg(layout.lastBaseline(15) - (layout.pageHeight - layout.bottomMargin));
  try { layout.validate(); }
  catch (const std::exception& error) { message += "\n" + QString::fromUtf8(error.what()); }
  summary_->setText(message);
}
