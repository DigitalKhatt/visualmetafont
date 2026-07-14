#include "SolverParametersWidget.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>
#include <vector>

#include "FlowLayout.h"

namespace {

constexpr int kDebounceMs = 150;
constexpr char kSettingsPrefix[] = "SolverTuning/";

// Note: each lambda below constructs its own QSettings when saving, rather
// than capturing the constructor's local `settings` by reference — that
// local goes out of scope once the widget is built, but the lambdas live as
// long as the signal connections do.

void addDoubleRow(QFormLayout* form, const QString& label, QSettings& settings,
                  const QString& key, double& field, double minV, double maxV,
                  double step, int decimals, SolverParametersWidget* owner,
                  void (SolverParametersWidget::*scheduleChanged)()) {
  const QString fullKey = QString(kSettingsPrefix) + key;
  field = settings.value(fullKey, field).toDouble();

  auto* spin = new QDoubleSpinBox(owner);
  spin->setRange(minV, maxV);
  spin->setSingleStep(step);
  spin->setDecimals(decimals);
  spin->setKeyboardTracking(false);
  spin->setValue(field);
  form->addRow(label, spin);

  QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), owner,
                    [&field, fullKey, owner, scheduleChanged](double v) {
                      field = v;
                      QSettings().setValue(fullKey, v);
                      (owner->*scheduleChanged)();
                    });
}

void addToggleRow(QFormLayout* form, const QString& label, QSettings& settings,
                  const QString& key, bool& field, SolverParametersWidget* owner,
                  void (SolverParametersWidget::*scheduleChanged)()) {
  const QString fullKey = QString(kSettingsPrefix) + key;
  field = settings.value(fullKey, field).toBool();

  auto* check = new QCheckBox(owner);
  check->setChecked(field);
  form->addRow(label, check);

  QObject::connect(check, &QCheckBox::toggled, owner,
                    [&field, fullKey, owner, scheduleChanged](bool checked) {
                      field = checked;
                      QSettings().setValue(fullKey, checked);
                      (owner->*scheduleChanged)();
                    });
}

// One tunable double belonging to a param row (see makeParamRow below).
struct FieldSpec {
  QString label;
  QString key;
  double* field;
  double minV;
  double maxV;
  double step;
  int decimals;
};

// Packs a constraint type's enable/disable checkbox and every tunable it owns
// into one compact horizontal strip (checkbox, bold type name, then
// "label: spinbox" for each field) instead of a separate toggle list plus one
// QFormLayout row per field. The caller adds the returned widget to a
// FlowLayout, so several single/few-field constraint types end up sharing a
// line, and only the wide ones (e.g. Waqf's 4 fields) force a wrap -- this is
// what keeps the dock's total height down. Fields grey out (but keep their
// stored value) while the checkbox is unchecked, since they have no effect
// on a disabled constraint.
QWidget* makeParamRow(QWidget* parentWidget, QSettings& settings, const QString& rowLabel,
                      const QString& toggleKey, bool& toggleField,
                      const std::vector<FieldSpec>& fields, SolverParametersWidget* owner,
                      void (SolverParametersWidget::*scheduleChanged)()) {
  auto* frame = new QFrame(parentWidget);
  frame->setFrameShape(QFrame::StyledPanel);

  auto* rowLayout = new QHBoxLayout(frame);
  rowLayout->setContentsMargins(6, 4, 6, 4);
  rowLayout->setSpacing(6);

  const QString toggleFullKey = QString(kSettingsPrefix) + toggleKey;
  toggleField = settings.value(toggleFullKey, toggleField).toBool();

  auto* check = new QCheckBox(frame);
  check->setChecked(toggleField);
  rowLayout->addWidget(check);

  auto* title = new QLabel(rowLabel, frame);
  QFont boldFont = title->font();
  boldFont.setBold(true);
  title->setFont(boldFont);
  title->setEnabled(toggleField);
  rowLayout->addWidget(title);

  auto fieldWidgets = std::make_shared<std::vector<QWidget*>>();
  fieldWidgets->push_back(title);

  for (const FieldSpec& spec : fields) {
    const QString fullKey = QString(kSettingsPrefix) + spec.key;
    *spec.field = settings.value(fullKey, *spec.field).toDouble();

    auto* label = new QLabel(spec.label, frame);
    label->setEnabled(toggleField);
    rowLayout->addWidget(label);
    fieldWidgets->push_back(label);

    auto* spin = new QDoubleSpinBox(frame);
    spin->setRange(spec.minV, spec.maxV);
    spin->setSingleStep(spec.step);
    spin->setDecimals(spec.decimals);
    spin->setKeyboardTracking(false);
    spin->setValue(*spec.field);
    spin->setMaximumWidth(85);
    spin->setEnabled(toggleField);
    rowLayout->addWidget(spin);
    fieldWidgets->push_back(spin);

    double* field = spec.field;
    QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), owner,
                      [field, fullKey, owner, scheduleChanged](double v) {
                        *field = v;
                        QSettings().setValue(fullKey, v);
                        (owner->*scheduleChanged)();
                      });
  }

  QObject::connect(check, &QCheckBox::toggled, owner,
                    [&toggleField, toggleFullKey, owner, scheduleChanged, fieldWidgets](bool checked) {
                      toggleField = checked;
                      QSettings().setValue(toggleFullKey, checked);
                      for (QWidget* w : *fieldWidgets) w->setEnabled(checked);
                      (owner->*scheduleChanged)();
                    });

  return frame;
}

}  // namespace

SolverParametersWidget::SolverParametersWidget(digitalkhatt::layout::OptParams& params, QWidget* parent)
    : QWidget(parent), m_params(params) {
  m_debounce = new QTimer(this);
  m_debounce->setSingleShot(true);
  connect(m_debounce, &QTimer::timeout, this, &SolverParametersWidget::parametersChanged);

  QSettings settings;

  auto* mainLayout = new QVBoxLayout(this);

  auto* gapsBox = new QGroupBox(tr("Gaps"), this);
  auto* gapsForm = new QFormLayout(gapsBox);
  addDoubleRow(gapsForm, tr("Min gap (body)"), settings, "minGapBody", m_params.minGapBody,
              0.0, 2000.0, 1.0, 1, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(gapsForm, tr("Min gap (mark)"), settings, "minGapMark", m_params.minGapMark,
              0.0, 2000.0, 1.0, 1, this, &SolverParametersWidget::scheduleChanged);
  mainLayout->addWidget(gapsBox);

  auto* shiftsBox = new QGroupBox(tr("Shifts"), this);
  auto* shiftsForm = new QFormLayout(shiftsBox);
  addDoubleRow(shiftsForm, tr("Max shift body X"), settings, "maxShiftBodyX", m_params.maxShiftBodyX,
              0.0, 500.0, 1.0, 1, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(shiftsForm, tr("Max shift body Y"), settings, "maxShiftBodyY", m_params.maxShiftBodyY,
              0.0, 500.0, 1.0, 1, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(shiftsForm, tr("Max shift mark"), settings, "maxShiftMark", m_params.maxShiftMark,
              0.0, 500.0, 1.0, 1, this, &SolverParametersWidget::scheduleChanged);
  mainLayout->addWidget(shiftsBox);

  auto* solverBox = new QGroupBox(tr("Solver"), this);
  auto* solverForm = new QFormLayout(solverBox);

  {
    const QString fullKey = QString(kSettingsPrefix) + "maxIters";
    m_params.maxIters = settings.value(fullKey, m_params.maxIters).toInt();
    auto* itersSpin = new QSpinBox(this);
    itersSpin->setRange(1, 200);
    itersSpin->setKeyboardTracking(false);
    itersSpin->setValue(m_params.maxIters);
    solverForm->addRow(tr("Max iterations"), itersSpin);
    connect(itersSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, fullKey](int v) {
      m_params.maxIters = v;
      QSettings().setValue(fullKey, v);
      scheduleChanged();
    });
  }

  addDoubleRow(solverForm, tr("Tolerance (collision)"), settings, "tolCollision", m_params.tolCollision,
              0.0, 50.0, 0.1, 2, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(solverForm, tr("Smooth strength"), settings, "smoothStrength", m_params.smoothStrength,
              0.0, 2.0, 0.01, 3, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(solverForm, tr("Attach strength"), settings, "attachStrength", m_params.attachStrength,
              0.0, 2.0, 0.01, 3, this, &SolverParametersWidget::scheduleChanged);
  addDoubleRow(solverForm, tr("Sep. overshoot"), settings, "sepOvershoot", m_params.sepOvershoot,
              1.0, 3.0, 0.01, 3, this, &SolverParametersWidget::scheduleChanged);
  mainLayout->addWidget(solverBox);

  // Constraint types with no single-instance compliance knob of their own
  // (GenericGap is a free function over broadphase pairs, ReportViolations is
  // a diagnostic flag) -- every other constraint's enable checkbox now lives
  // next to its own parameters below.
  auto* constraintsBox = new QGroupBox(tr("Constraints"), this);
  auto* constraintsForm = new QFormLayout(constraintsBox);
  auto& toggles = m_params.toggles;
  addToggleRow(constraintsForm, tr("Generic gap constraint (broadphase)"), settings, "toggle.genericGapConstraint",
              toggles.genericGapConstraint, this, &SolverParametersWidget::scheduleChanged);
  addToggleRow(constraintsForm, tr("Report constraint violations (PDF+log)"), settings, "toggle.reportViolations",
              toggles.reportViolations, this, &SolverParametersWidget::scheduleChanged);
  mainLayout->addWidget(constraintsBox);

  // Every per-constraint-type tunable (HorizontalOrder's self/cross-keep
  // thresholds plus every constraint's XPBD compliance) alongside that
  // constraint's own enable/disable checkbox, grouped one row per constraint
  // type and packed into a FlowLayout so rows with few fields share a line
  // instead of each field claiming its own -- keeps this section's height
  // down even though it now covers the toggles plus ~20 tunable values.
  auto* paramsBox = new QGroupBox(tr("Constraint Parameters"), this);
  new FlowLayout(paramsBox, 6, 6, 6);
  auto* paramsFlow = static_cast<FlowLayout*>(paramsBox->layout());
  auto& comp = m_params.compliance;

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Horizontal order"),
      "toggle.horizontalOrder", toggles.horizontalOrder,
      {
          {tr("Self keep"), "horizontalOrderSelfKeep", &m_params.horizontalOrderSelfKeep, 0.0, 1.0, 0.01, 2},
          {tr("Cross keep"), "horizontalOrderCrossKeep", &m_params.horizontalOrderCrossKeep, 0.0, 1.0, 0.01, 2},
          {tr("Compliance"), "compliance.horizontalOrder", &comp.horizontalOrder, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Waqf placement"),
      "toggle.waqfPlacement", toggles.waqfPlacement,
      {
          {tr("Min"), "compliance.waqfMin", &comp.waqfMin, 0.0, 2.0, 0.01, 3},
          {tr("Target"), "compliance.waqfTarget", &comp.waqfTarget, 0.0, 2.0, 0.01, 3},
          {tr("Max"), "compliance.waqfMax", &comp.waqfMax, 0.0, 2.0, 0.01, 3},
          {tr("X-align"), "compliance.waqfXAlign", &comp.waqfXAlign, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Hard stay above/below"),
      "toggle.hardStayAboveBelow", toggles.hardStayAboveBelow,
      {
          {tr("Above"), "compliance.hardStayAbove", &comp.hardStayAbove, 0.0, 0.01, 0.0001, 6},
          {tr("Below"), "compliance.hardStayBelow", &comp.hardStayBelow, 0.0, 0.01, 0.0001, 6},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Base vicinity"),
      "toggle.baseVicinity", toggles.baseVicinity,
      {
          {tr("Left"), "compliance.baseVicinityLeft", &comp.baseVicinityLeft, 0.0, 2.0, 0.01, 3},
          {tr("Right"), "compliance.baseVicinityRight", &comp.baseVicinityRight, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Stack order"),
      "toggle.stackOrder", toggles.stackOrder,
      {
          {tr("Gap"), "compliance.stackOrderGap", &comp.stackOrderGap, 0.0, 2.0, 0.01, 3},
          {tr("X-align"), "compliance.stackOrderXAlign", &comp.stackOrderXAlign, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Return to anchor"),
      "toggle.returnToAnchor", toggles.returnToAnchor,
      {
          {tr("Compliance"), "compliance.returnToAnchor", &comp.returnToAnchor, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Squeeze center"),
      "toggle.squeezeCenter", toggles.squeezeCenter,
      {
          {tr("Compliance"), "compliance.squeezeCenter", &comp.squeezeCenter, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Match mark position"),
      "toggle.matchMarkPosition", toggles.matchMarkPosition,
      {
          {tr("Compliance"), "compliance.matchMarkPosition", &comp.matchMarkPosition, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Bowl cluster"),
      "toggle.bowlCluster", toggles.bowlCluster,
      {
          {tr("Compliance"), "compliance.bowlCluster", &comp.bowlCluster, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  paramsFlow->addWidget(makeParamRow(paramsBox, settings, tr("Y-lane"),
      "toggle.ylane", toggles.ylane,
      {
          {tr("Compliance"), "compliance.ylane", &comp.ylane, 0.0, 2.0, 0.01, 3},
      },
      this, &SolverParametersWidget::scheduleChanged));

  mainLayout->addWidget(paramsBox);

  mainLayout->addStretch(1);
}

void SolverParametersWidget::scheduleChanged() {
  m_debounce->start(kDebounceMs);
}
