#include "SolverParametersWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QGroupBox>
#include <QSettings>
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
    }
    std::cout << "Solver parameter controls, persistence and collapsible sections passed\n";
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
