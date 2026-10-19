#include "SolverParametersWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTimer>
#include <QToolButton>

#include <iostream>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}
template<class T> T* field(QWidget& widget, const char* name) {
  auto* value = widget.findChild<T*>(name);
  require(value != nullptr, name);
  return value;
}
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("DigitalKhattTests");
  QCoreApplication::setApplicationName("SolverParameters");
  QTemporaryDir preferences;
  require(preferences.isValid(), "Temporary settings directory is required");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, preferences.path());
  try {
    QSettings().setValue("SolverTuning/rehWaw.belowTipFallback", false);
    {
      digitalkhatt::layout::OptParams migrated;
      SolverParametersWidget legacy(migrated);
      require(!migrated.rehWaw.oneDotBelowTip && !migrated.rehWaw.twoDotsBelowTip &&
              !QSettings().contains("SolverTuning/rehWaw.belowTipFallback"),
          "The former shared fallback setting must migrate to both independent dot choices");
    }
    QSettings().clear();
    digitalkhatt::layout::OptParams params;
    SolverParametersWidget widget(params);
    require(field<QDoubleSpinBox>(widget, "differentLineGap")->value() == 250,
        "Different-line gap must be exposed with a 250-unit default");
    require(field<QDoubleSpinBox>(widget, "compliance.extraLineSpacing")->value() == 1.0,
        "Extra line spacing compliance must be exposed with a 1.0 default");
    field<QDoubleSpinBox>(widget, "compliance.extraLineSpacing")->setValue(2.5);
    require(params.compliance.extraLineSpacing == 2.5 && params.differentLineGap == 250,
        "Extra line spacing softness must be independent of its distance target");
    field<QDoubleSpinBox>(widget, "differentWordGap")->setValue(120);
    require(params.differentLineGap == 250, "Editing the word gap must not change the line gap");
    field<QDoubleSpinBox>(widget, "differentLineGap")->setValue(300);
    require(params.differentLineGap == 300 && params.differentWordGap == 120,
        "The line gap must be independently editable");
    widget.resize(460, 850);
    widget.show();
    app.processEvents();
    auto* sections = field<QWidget>(widget, "ui.placementSection");
    auto* collision = field<QToolButton>(widget, "ui.placementSection.header.0");
    const int openHeight = sections->sizeHint().height();
    require(collision->isChecked(), "Initial collision section must be open");
    collision->click();
    app.processEvents();
    require(!collision->isChecked() && field<QWidget>(widget, "ui.placementSection.page.0")->isHidden(),
        "Clicking the open header must close it");
    require(sections->sizeHint().height() < openHeight - 20,
        "Closed content must not retain a blank expanding viewport");
    require(params.toggles.genericGapConstraint, "Closing a section must not disable its constraints");

    auto* downward = field<QGroupBox>(widget, "toggle.downwardEscape");
    auto* gaps = field<QGroupBox>(widget, "toggle.genericGapConstraint");
    require(!downward->isChecked(), "Downward escape must default to disabled independently of Reh/Waw");
    downward->setChecked(true);
    field<QSpinBox>(widget, "downwardEscape.repeatedCollisions")->setValue(7);
    field<QDoubleSpinBox>(widget, "downwardEscape.maxDescentPercent")->setValue(140);
    field<QDoubleSpinBox>(widget, "downwardEscape.maxBelowBasePercent")->setValue(180);
    require(params.toggles.downwardEscape && !params.toggles.rehWawBehPlacement && downward->isEnabled() &&
            params.downwardEscape.repeatedCollisions == 7 && params.downwardEscape.maxDescentPercent == 140 &&
            params.downwardEscape.maxBelowBasePercent == 180,
        "Downward escape controls must update live parameters with Reh/Waw disabled");
    gaps->setChecked(false);
    require(!downward->isEnabled() && downward->isChecked() && params.toggles.downwardEscape,
        "Disabling collision avoidance must suspend downward escape without losing its saved choice");
    gaps->setChecked(true);
    require(downward->isEnabled(), "Restoring collision avoidance must restore the independent escape control");

    require(!widget.findChild<QDoubleSpinBox*>("compliance.waqfTarget") &&
            !widget.findChild<QDoubleSpinBox*>("compliance.waqfXAlign"),
        "Removed preferred-height and horizontal-alignment springs must have no GUI controls");
    require(field<QDoubleSpinBox>(widget, "compliance.waqfMin") &&
            field<QDoubleSpinBox>(widget, "compliance.waqfMax"),
        "Safety-bound controls must remain available");
    auto* waqfFallback = field<QCheckBox>(widget, "toggle.waqfEscape");
    auto* waqfPlacement = field<QGroupBox>(widget, "toggle.waqfPlacement");
    require(field<QDoubleSpinBox>(widget, "waqfMinHeight")->value() == 700,
        "The waqf minimum-height parameter must retain the former 700-unit default");
    field<QDoubleSpinBox>(widget, "waqfMinHeight")->setValue(800);
    require(params.waqfMinHeight == 800,
        "The minimum-height GUI control must update the live solver parameter");
    require(waqfPlacement->isAncestorOf(waqfFallback) && waqfFallback->isChecked(),
        "The fallback checkbox must be inside Position waqf marks and retain its enabled default");
    waqfFallback->setChecked(false);
    require(!params.toggles.waqfEscape && params.toggles.waqfPlacement &&
            field<QDoubleSpinBox>(widget, "waqfEscapeMaxLeftPercent")->isEnabled() &&
            !field<QDoubleSpinBox>(widget, "waqfEscapeMinGap")->isEnabled(),
        "Disabling fallback must retain initial placement and its shared left allowance");
    gaps->setChecked(false);
    require(!waqfFallback->isEnabled(), "Fallback requires ordinary collision forces");
    gaps->setChecked(true);
    require(waqfFallback->isEnabled() && !waqfFallback->isChecked(),
        "Restoring gap forces must retain the disabled fallback choice");
    waqfPlacement->setChecked(false);
    require(!waqfFallback->isEnabled(), "Fallback requires waqf placement");
    waqfPlacement->setChecked(true);

    require(!widget.findChild<QDoubleSpinBox*>("compliance.rehWawDotTarget"),
        "Initial-positioned dots must not expose an obsolete point-target compliance control");
    auto* special = field<QToolButton>(widget, "ui.placementSection.header.4");
    special->click();
    field<QGroupBox>(widget, "toggle.rehWawBehPlacement")->setChecked(true);
    const std::pair<const char*, double> values[] = {
        {"rehWaw.tipGap", 44}, {"rehWaw.dotKasraGap", 65},
        {"rehWaw.compactGap", 18}, {"rehWaw.leftGap", 33}, {"rehWaw.preferredGap", 66}, {"rehWaw.neighborGap", 55},
        {"rehWaw.overhangPercent", 8}, {"rehWaw.overhangLimit", 12},
        {"compliance.rehWawKasraFollow", 0.45}, {"rehWaw.belowMarkGap", 12},
        {"compliance.rehWawBelowMarkGap", 1.4}, {"compliance.rehWawBelowBottom", 0.3}};
    for (const auto& [name, value] : values) field<QDoubleSpinBox>(widget, name)->setValue(value);
    field<QCheckBox>(widget, "rehWaw.oneDotBelowTip")->setChecked(false);
    field<QCheckBox>(widget, "rehWaw.twoDotsBelowTip")->setChecked(true);
    require(params.rehWaw.tipGap == 44 && params.rehWaw.dotKasraGap == 65 &&
            params.rehWaw.compactGap == 18 && params.rehWaw.leftGap == 33 && params.rehWaw.preferredGap == 66 && params.rehWaw.neighborGap == 55 &&
            params.rehWaw.overhangPercent == 8 && params.rehWaw.overhangLimit == 12 &&
            params.compliance.rehWawKasraFollow == 0.45 &&
            params.rehWaw.belowMarkGap == 12 && params.compliance.rehWawBelowMarkGap == 1.4 &&
            params.compliance.rehWawBelowBottom == 0.3 &&
            !params.rehWaw.oneDotBelowTip && params.rehWaw.twoDotsBelowTip,
        "GUI controls must update the live placement parameters");
    special->click();
    require(!special->isChecked() && params.toggles.rehWawBehPlacement,
        "Collapsing placement settings must leave the placement option enabled");
    QSettings().sync();

    digitalkhatt::layout::OptParams restored;
    SolverParametersWidget reopened(restored);
    require(!restored.toggles.waqfEscape && restored.toggles.waqfPlacement,
        "The disabled fallback choice must survive reopening independently of initial placement");
    require(restored.waqfMinHeight == 800,
        "The waqf minimum height must survive reopening the panel");
    require(restored.compliance.extraLineSpacing == 2.5,
        "Extra line spacing compliance must survive reopening the panel");
    require(restored.differentLineGap == 300 && restored.differentWordGap == 120,
        "Independent line and word gaps must survive reopening the panel");
    require(restored.toggles.downwardEscape && restored.downwardEscape.repeatedCollisions == 7 &&
            restored.downwardEscape.maxDescentPercent == 140 && restored.downwardEscape.maxBelowBasePercent == 180,
        "Independent escape and both descent bounds must survive reopening the GUI");
    require(restored.rehWaw.tipGap == 44 && restored.rehWaw.dotKasraGap == 65 &&
            restored.rehWaw.compactGap == 18 && restored.rehWaw.leftGap == 33 && restored.rehWaw.preferredGap == 66 && restored.rehWaw.neighborGap == 55 &&
            restored.rehWaw.overhangPercent == 8 && restored.rehWaw.overhangLimit == 12 &&
            restored.compliance.rehWawKasraFollow == 0.45 &&
            restored.rehWaw.belowMarkGap == 12 && restored.compliance.rehWawBelowMarkGap == 1.4 &&
            restored.compliance.rehWawBelowBottom == 0.3 &&
            !restored.rehWaw.oneDotBelowTip && restored.rehWaw.twoDotsBelowTip && restored.toggles.rehWawBehPlacement,
        "Placement tuning and fallback choice must survive reopening the panel");
    for (int i = 0; i < 5; ++i)
      require(!reopened.findChild<QToolButton*>("ui.placementSection.header." + QString::number(i))->isChecked(),
          "The all-closed state must survive reopening the panel");
    auto* tabs = reopened.findChild<QTabWidget*>();
    tabs->setCurrentIndex(1);
    for (int i = 0; i < 3; ++i) {
      auto* header = reopened.findChild<QToolButton*>("ui.reportSection.header." + QString::number(i));
      if (!header->isChecked()) header->click();
      header->click();
      require(!header->isChecked(), "Report sections must also close when clicked again");
    }

    int changed = 0;
    QObject::connect(&widget, &SolverParametersWidget::parametersChanged, &widget, [&] { ++changed; });
    field<QDoubleSpinBox>(widget, "rehWaw.tipGap")->setValue(45);
    field<QDoubleSpinBox>(widget, "rehWaw.tipGap")->setValue(46);
    QEventLoop wait;
    QTimer::singleShot(220, &wait, &QEventLoop::quit);
    wait.exec();
    require(changed == 1, "Rapid edits must emit one debounced layout update");

    if (argc > 1) {
      QSettings().clear();
      QSettings().setValue("SolverTuning/ui.placementSection", 4);
      QSettings().setValue("SolverTuning/toggle.rehWawBehPlacement", true);
      digitalkhatt::layout::OptParams defaults;
      SolverParametersWidget preview(defaults);
      preview.resize(460, 1200);
      preview.show();
      app.processEvents();
      QDir directory(QString::fromLocal8Bit(argv[1]));
      require(directory.mkpath("."), "Screenshot directory must be writable");
      require(preview.grab().save(directory.filePath("parameters.png")), "Save expanded panel preview");
      field<QCheckBox>(preview, "rehWaw.oneDotBelowTip")->setChecked(false);
      app.processEvents();
      require(preview.grab().save(directory.filePath("one-dot-balanced-parameters.png")), "Save independent dot choices preview");
      field<QCheckBox>(preview, "rehWaw.twoDotsBelowTip")->setChecked(false);
      app.processEvents();
      require(preview.grab().save(directory.filePath("balanced-parameters.png")), "Save balance-mode panel preview");
      field<QToolButton>(preview, "ui.placementSection.header.4")->click();
      app.processEvents();
      require(preview.grab().save(directory.filePath("collapsed.png")), "Save collapsed panel preview");
      field<QToolButton>(preview, "ui.placementSection.header.0")->click();
      field<QGroupBox>(preview, "toggle.rehWawBehPlacement")->setChecked(false);
      field<QGroupBox>(preview, "toggle.downwardEscape")->setChecked(true);
      app.processEvents();
      require(preview.grab().save(directory.filePath("downward-escape.png")), "Save independent downward-escape controls");
    }
    std::cout << "Solver parameter controls, persistence and collapsible sections passed\n";
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
