#include "SolverParametersWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr int kDebounceMs = 150;
constexpr char kSettingsPrefix[] = "SolverTuning/";
using Changed = void (SolverParametersWidget::*)();
QString settingKey(const QString& key) { return QString(kSettingsPrefix) + key; }

// Settings are created inside callbacks; the constructor's QSettings is local.
QDoubleSpinBox* addDoubleRow(QFormLayout* form, const QString& label, QSettings& settings,
    const QString& key, double& field, double minV, double maxV, double step, int decimals,
    SolverParametersWidget* owner, Changed changed, const QString& tip = {},
    const QString& suffix = {}, double displayScale = 1.0) {
  const QString fullKey = settingKey(key);
  const double saved = settings.value(fullKey, field).toDouble();
  if (std::isfinite(saved)) field = std::clamp(saved, minV / displayScale, maxV / displayScale);
  auto* spin = new QDoubleSpinBox(owner);
  spin->setObjectName(key);
  spin->setDecimals(decimals);
  spin->setRange(minV, maxV);
  spin->setSingleStep(step);
  spin->setKeyboardTracking(false);
  spin->setSuffix(suffix);
  spin->setValue(field * displayScale);
  spin->setToolTip(tip);
  form->addRow(label, spin);
  form->labelForField(spin)->setToolTip(tip);
  QObject::connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), owner,
      [&field, fullKey, owner, changed, displayScale](double v) {
        field = v / displayScale;
        QSettings().setValue(fullKey, field);
        (owner->*changed)();
      });
  return spin;
}

QCheckBox* addToggleRow(QFormLayout* form, const QString& label, QSettings& settings,
    const QString& key, bool& field, SolverParametersWidget* owner, Changed changed,
    const QString& tip = {}) {
  const QString fullKey = settingKey(key);
  field = settings.value(fullKey, field).toBool();
  auto* check = new QCheckBox(owner);
  check->setObjectName(key);
  check->setChecked(field);
  check->setToolTip(tip);
  form->addRow(label, check);
  form->labelForField(check)->setToolTip(tip);
  QObject::connect(check, &QCheckBox::toggled, owner,
      [&field, fullKey, owner, changed](bool v) {
        field = v;
        QSettings().setValue(fullKey, v);
        (owner->*changed)();
      });
  return check;
}

void addIntRow(QFormLayout* form, const QString& label, QSettings& settings,
    const QString& key, int& field, int minV, int maxV, SolverParametersWidget* owner,
    Changed changed, const QString& tip = {}, const QString& zeroText = {}) {
  const QString fullKey = settingKey(key);
  field = std::clamp(settings.value(fullKey, field).toInt(), minV, maxV);
  auto* spin = new QSpinBox(owner);
  spin->setObjectName(key);
  spin->setRange(minV, maxV);
  spin->setKeyboardTracking(false);
  spin->setSpecialValueText(zeroText);
  spin->setValue(field);
  spin->setToolTip(tip);
  form->addRow(label, spin);
  QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), owner,
      [&field, fullKey, owner, changed](int v) {
        field = v;
        QSettings().setValue(fullKey, v);
        (owner->*changed)();
      });
}

QFormLayout* makeForm(QWidget* parent) {
  auto* form = new QFormLayout(parent);
  form->setContentsMargins(8, 6, 8, 6);
  form->setVerticalSpacing(4);
  form->setFormAlignment(Qt::AlignTop);
  form->setRowWrapPolicy(QFormLayout::WrapLongRows);
  form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  return form;
}

QWidget* addTab(QTabWidget* tabs, const QString& title) {
  auto* scroll = new QScrollArea(tabs);
  scroll->setWidgetResizable(true);
  auto* page = new QWidget(scroll);
  auto* layout = new QVBoxLayout(page);
  layout->setContentsMargins(6, 6, 6, 6);
  layout->setSpacing(6);
  layout->setAlignment(Qt::AlignTop);
  scroll->setWidget(page);
  tabs->addTab(scroll, title);
  return page;
}

// Unlike QToolBox, every section may close and hidden pages reserve no height.
// The outer tab scrolls the actual content, without a second expanding viewport.
class CollapsibleSections : public QWidget {
 public:
  explicit CollapsibleSections(QWidget* parent) : QWidget(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);
    m_layout->setAlignment(Qt::AlignTop);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  }
  QWidget* addSection(const QString& title) {
    const int index = static_cast<int>(m_sections.size());
    auto* header = new QToolButton(this);
    header->setText(title);
    header->setCheckable(true);
    header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setArrowType(Qt::RightArrow);
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(2, 2, 2, 4);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop);
    page->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    page->hide();
    m_layout->addWidget(header);
    m_layout->addWidget(page);
    m_sections.push_back({header, page});
    connect(header, &QToolButton::clicked, this, [this, index]() {
      setCurrentIndex(m_current == index ? -1 : index);
    });
    return page;
  }
  void restore(QSettings& settings, const QString& key) {
    m_key = settingKey(key);
    setObjectName(key);
    for (int i = 0; i < static_cast<int>(m_sections.size()); ++i) {
      m_sections[i].header->setObjectName(key + ".header." + QString::number(i));
      m_sections[i].page->setObjectName(key + ".page." + QString::number(i));
    }
    setCurrentIndex(std::clamp(settings.value(m_key, 0).toInt(), -1, static_cast<int>(m_sections.size()) - 1));
  }

 private:
  struct Section { QToolButton* header; QWidget* page; };
  std::vector<Section> m_sections;
  QVBoxLayout* m_layout;
  QString m_key;
  int m_current = -1;
  void setCurrentIndex(int index) {
    m_current = index;
    for (int i = 0; i < static_cast<int>(m_sections.size()); ++i) {
      const bool open = i == index;
      m_sections[i].header->setChecked(open);
      m_sections[i].header->setArrowType(open ? Qt::DownArrow : Qt::RightArrow);
      m_sections[i].page->setVisible(open);
    }
    if (!m_key.isEmpty()) QSettings().setValue(m_key, index);
    updateGeometry();
  }
};

QWidget* addSection(CollapsibleSections* sections, const QString& title) {
  return sections->addSection(title);
}

void addNote(QWidget* parent, const QString& text) {
  auto* note = new QLabel(text, parent);
  note->setWordWrap(true);
  note->setTextFormat(Qt::PlainText);
  static_cast<QVBoxLayout*>(parent->layout())->addWidget(note);
}

struct ConstraintForm { QGroupBox* box; QFormLayout* form; };
ConstraintForm addConstraint(QWidget* parent, const QString& title, QSettings& settings,
    const QString& key, bool& enabled, SolverParametersWidget* owner, Changed changed) {
  const QString fullKey = settingKey(key);
  enabled = settings.value(fullKey, enabled).toBool();
  auto* box = new QGroupBox(title, parent);
  box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  box->setObjectName(key);
  box->setCheckable(true);
  box->setChecked(enabled);
  auto* form = makeForm(box);
  static_cast<QVBoxLayout*>(parent->layout())->addWidget(box);
  QObject::connect(box, &QGroupBox::toggled, owner,
      [&enabled, fullKey, owner, changed](bool v) {
        enabled = v;
        QSettings().setValue(fullKey, v);
        (owner->*changed)();
      });
  return {box, form};
}

void migrateSettings(QSettings& settings) {
  const QString oldFallback = settingKey("rehWaw.belowTipFallback");
  if (settings.contains(oldFallback)) {
    for (const auto* key : {"rehWaw.oneDotBelowTip", "rehWaw.twoDotsBelowTip"})
      if (!settings.contains(settingKey(key))) settings.setValue(settingKey(key), settings.value(oldFallback));
    settings.remove(oldFallback);
  }
  const QString paddingKey = settingKey("broadphasePadding");
  bool hasLegacyPadding = false;
  for (const auto* key : {"minGapBody", "minGapMark", "maxShiftBodyX", "maxShiftBodyY", "maxShiftMark"})
    hasLegacyPadding |= settings.contains(settingKey(key));
  if (!settings.contains(paddingKey) && hasLegacyPadding) {
    const double gap = std::max(settings.value(settingKey("minGapBody"), 10.0).toDouble(),
                                settings.value(settingKey("minGapMark"), 100.0).toDouble());
    const double shift = std::max({settings.value(settingKey("maxShiftBodyX"), 15.0).toDouble(),
        settings.value(settingKey("maxShiftBodyY"), 4.0).toDouble(),
        settings.value(settingKey("maxShiftMark"), 20.0).toDouble()});
    settings.setValue(paddingKey, gap + 2.0 * shift);
  }
  for (const auto* key : {"minGapBody", "minGapMark", "maxShiftBodyX", "maxShiftBodyY", "maxShiftMark",
      "smoothStrength", "attachStrength", "sepOvershoot", "toggle.returnToAnchor", "compliance.returnToAnchor"})
    settings.remove(settingKey(key));
}
}  // namespace

SolverParametersWidget::SolverParametersWidget(digitalkhatt::layout::OptParams& params, QWidget* parent)
    : QWidget(parent), m_params(params) {
  m_debounce = new QTimer(this);
  m_debounce->setSingleShot(true);
  connect(m_debounce, &QTimer::timeout, this, &SolverParametersWidget::parametersChanged);
  QSettings settings;
  migrateSettings(settings);
  auto* main = new QVBoxLayout(this);
  main->setContentsMargins(0, 0, 0, 0);
  auto* tabs = new QTabWidget(this);
  main->addWidget(tabs);
  auto& toggles = m_params.toggles;
  auto& comp = m_params.compliance;
  const auto changed = &SolverParametersWidget::scheduleChanged;
  const QString units = tr(" units");
  const QString percent = tr(" %");
  const QString softness = tr("XPBD compliance: 0 is rigid; larger values allow more deviation from this constraint.");
  auto compliance = [&](QFormLayout* form, const QString& label, const QString& key,
                        double& value, double maxV = 2.0, double step = 0.01, int decimals = 3) {
    addDoubleRow(form, label, settings, key, value, 0.0, maxV, step, decimals, this, changed, softness);
  };

  auto* placement = addTab(tabs, tr("Placement"));
  auto* solver = new QGroupBox(tr("Solver"), placement);
  auto* solverForm = makeForm(solver);
  addIntRow(solverForm, tr("Maximum iterations"), settings, "maxIters", m_params.maxIters,
      1, 200, this, changed, tr("Upper limit on the number of XPBD iterations per page."));
  addDoubleRow(solverForm, tr("Convergence tolerance"), settings, "tolCollision", m_params.tolCollision,
      0.0, 50.0, 0.1, 2, this, changed,
      tr("Early exit requires small applied corrections and acceptable remaining hard residuals."), units);
  static_cast<QVBoxLayout*>(placement->layout())->addWidget(solver);
  auto* constraints = new CollapsibleSections(placement);
  static_cast<QVBoxLayout*>(placement->layout())->addWidget(constraints);

  auto* collisionPage = addSection(constraints, tr("Collision avoidance"));
  auto gaps = addConstraint(collisionPage, tr("Resolve gaps and collisions"), settings,
      "toggle.genericGapConstraint", toggles.genericGapConstraint, this, changed);
  addDoubleRow(gaps.form, tr("Same-word gap"), settings, "sameWordGap", m_params.sameWordGap,
      0.0, 1000.0, 5.0, 1, this, changed,
      tr("Generic target clearance within one word. Dedicated stacks and the 40 / 10-unit attachment exceptions use their own gaps. Zero requests no extra clearance."), units);
  addDoubleRow(gaps.form, tr("Different-word gap"), settings, "differentWordGap", m_params.differentWordGap,
      0.0, 1000.0, 5.0, 1, this, changed,
      tr("Target clearance between glyphs of different words, including different lines. Word identity follows spaces in the glyph sequence, not mark positions. XPBD compliance can leave a residual."), units);
  auto* gapRules = new QLabel(tr("Attachment exceptions: 40 units for selected marks on their alef, and 10 for the kasra / iqlab meem pair. Base glyphs remain fixed; gap forces act on marks."), gaps.box);
  gapRules->setWordWrap(true);
  gaps.form->addRow(gapRules);
  addNote(collisionPage, tr("Save Collision clearance is a separate reporting setting in the Reports tab."));

  auto* association = addSection(constraints, tr("Base association"));
  auto order = addConstraint(association, tr("Horizontal order"), settings,
      "toggle.horizontalOrder", toggles.horizontalOrder, this, changed);
  addDoubleRow(order.form, tr("Own mark exposed"), settings, "horizontalOrderSelfKeep", m_params.horizontalOrderSelfKeep,
      0.0, 100.0, 1.0, 2, this, changed,
      tr("Minimum proportion of each mark's width exposed on its own owner's side."), percent, 100.0);
  addDoubleRow(order.form, tr("Neighbor exposure threshold"), settings, "horizontalOrderCrossKeep", m_params.horizontalOrderCrossKeep,
      0.0, 100.0, 1.0, 2, this, changed,
      tr("The exposed portion must also reach this proportion of the neighboring mark's width."), percent, 100.0);
  compliance(order.form, tr("Compliance"), "compliance.horizontalOrder", comp.horizontalOrder);
  auto vicinity = addConstraint(association, tr("Stay near base"), settings,
      "toggle.baseVicinity", toggles.baseVicinity, this, changed);
  compliance(vicinity.form, tr("Left compliance"), "compliance.baseVicinityLeft", comp.baseVicinityLeft);
  compliance(vicinity.form, tr("Right compliance"), "compliance.baseVicinityRight", comp.baseVicinityRight);
  auto side = addConstraint(association, tr("Stay above / below"), settings,
      "toggle.hardStayAboveBelow", toggles.hardStayAboveBelow, this, changed);
  compliance(side.form, tr("Above compliance"), "compliance.hardStayAbove", comp.hardStayAbove, 0.01, 0.0000001, 8);
  compliance(side.form, tr("Below compliance"), "compliance.hardStayBelow", comp.hardStayBelow, 0.01, 0.0000001, 8);

  auto* stacking = addSection(constraints, tr("Stacking and harakat"));
  auto stack = addConstraint(stacking, tr("Stack order"), settings,
      "toggle.stackOrder", toggles.stackOrder, this, changed);
  compliance(stack.form, tr("Gap compliance"), "compliance.stackOrderGap", comp.stackOrderGap);
  compliance(stack.form, tr("Horizontal compliance"), "compliance.stackOrderXAlign", comp.stackOrderXAlign);
  auto lane = addConstraint(stacking, tr("Harakat height band"), settings,
      "toggle.ylane", toggles.ylane, this, changed);
  compliance(lane.form, tr("Compliance"), "compliance.ylane", comp.ylane);

  auto* waqf = addSection(constraints, tr("Waqf placement"));
  auto waqfPlacement = addConstraint(waqf, tr("Position waqf marks"), settings,
      "toggle.waqfPlacement", toggles.waqfPlacement, this, changed);
  compliance(waqfPlacement.form, tr("Minimum-height compliance"), "compliance.waqfMin", comp.waqfMin);
  compliance(waqfPlacement.form, tr("Preferred-height compliance"), "compliance.waqfTarget", comp.waqfTarget, 10.0);
  compliance(waqfPlacement.form, tr("Maximum-height compliance"), "compliance.waqfMax", comp.waqfMax);
  compliance(waqfPlacement.form, tr("Horizontal compliance"), "compliance.waqfXAlign", comp.waqfXAlign);
  addDoubleRow(waqfPlacement.form, tr("Horizontal alignment band"), settings, "waqfHorizontalAlignmentBandPercent",
      m_params.waqfHorizontalAlignmentBandPercent, 0.0, 1000.0, 5.0, 1, this, changed,
      tr("No horizontal alignment force within this distance on either side of the target, as a percentage of waqf width."), percent);
  auto escape = addConstraint(waqf, tr("Left / down escape when squeezed"), settings,
      "toggle.waqfEscape", toggles.waqfEscape, this, changed);
  addDoubleRow(escape.form, tr("Trigger clearance"), settings, "waqfEscapeMinGap", m_params.waqfEscapeMinGap,
      0.0, 1000.0, 1.0, 2, this, changed,
      tr("Escape starts below this estimated clearance when progress stalls, or immediately when an earlier-line contact opposes contact with the owning top stack."), units);
  addDoubleRow(escape.form, tr("Left step"), settings, "waqfEscapeStepPercent", m_params.waqfEscapeStepPercent,
      0.0, 100.0, 1.0, 2, this, changed, tr("Target advance per iteration, as a percentage of waqf width."), percent);
  addDoubleRow(escape.form, tr("Down step"), settings, "waqfEscapeDownStepPercent", m_params.waqfEscapeDownStepPercent,
      0.0, 100.0, 1.0, 2, this, changed, tr("Target advance per iteration, as a percentage of waqf height."), percent);
  addDoubleRow(escape.form, tr("Maximum left target offset"), settings, "waqfEscapeMaxLeftPercent", m_params.waqfEscapeMaxLeftPercent,
      0.0, 1000.0, 5.0, 2, this, changed, tr("Cap on the escape target's left offset from the owning base, as a percentage of waqf width."), percent);
  escape.box->setToolTip(tr("A soft escape preference for a stalled contact or squeeze against an earlier line. Requires waqf placement and collision avoidance to be enabled."));
  const auto updateEscape = [escape, waqfPlacement, gaps]() {
    escape.box->setEnabled(waqfPlacement.box->isChecked() && gaps.box->isChecked());
  };
  connect(waqfPlacement.box, &QGroupBox::toggled, this, updateEscape);
  connect(gaps.box, &QGroupBox::toggled, this, updateEscape);
  updateEscape();

  auto* special = addSection(constraints, tr("Special placements"));
  auto behPlacement = addConstraint(special, tr("Reh / Waw followed by initial Beh"), settings,
      "toggle.rehWawBehPlacement", toggles.rehWawBehPlacement, this, changed);
  auto& beh = m_params.rehWaw;
  auto* behNote = new QLabel(tr("Initial positioning runs once before XPBD. Centered dots stay fixed with their center at or right of the tip. Left dots align their center with the leftmost Reh/Waw tip. Left/below dots remain movable; ordinary stacking aligns same-owner kasra."), behPlacement.box);
  behNote->setWordWrap(true);
  behPlacement.form->addRow(behNote);
  const auto fallbackTip = tr("A clear centered current gap is kept first. Used only when both that gap and the left slot fail. Checked: allow initial placement below Reh/Waw. Unchecked: keep the original input when neither slot fits.");
  addToggleRow(behPlacement.form, tr("One dot below tip"), settings,
      "rehWaw.oneDotBelowTip", beh.oneDotBelowTip, this, changed, fallbackTip);
  addToggleRow(behPlacement.form, tr("Two dots below tip"), settings,
      "rehWaw.twoDotsBelowTip", beh.twoDotsBelowTip, this, changed, fallbackTip);
  addDoubleRow(behPlacement.form, tr("Tip gap"), settings, "rehWaw.tipGap", beh.tipGap,
      0, 1000, 1, 2, this, changed, tr("Preferred compact spacing to the reference Reh/Waw tip."), units);
  addDoubleRow(behPlacement.form, tr("Dot–kasra gap"), settings, "rehWaw.dotKasraGap", beh.dotKasraGap,
      0, 1000, 1, 2, this, changed, tr("Vertical stacking gap for dots and kasra on the same Beh/Yeh."), units);
  addDoubleRow(behPlacement.form, tr("Compact ink clearance"), settings, "rehWaw.compactGap", beh.compactGap,
      0, 1000, 1, 2, this, changed, tr("Minimum outline clearance to the owning Behshape and reference Reh/Waw."), units);
  addDoubleRow(behPlacement.form, tr("Minimum left clearance"), settings, "rehWaw.leftGap", beh.leftGap,
      0, 1000, 1, 2, this, changed, tr("Minimum horizontal gap from a left-positioned dot's right edge to Reh/Waw's leftmost tip. The dot's center is initially level with that tip."), units);
  addDoubleRow(behPlacement.form, tr("Preferred in-gap clearance"), settings, "rehWaw.preferredGap", beh.preferredGap,
      0, 1000, 1, 2, this, changed, tr("Aim for this much clearance to both Behshape and Reh/Waw within the current gap. If it cannot fit, use the roomiest safe position there. This target does not force below-tip placement; Compact ink clearance remains the minimum."), units);
  addDoubleRow(behPlacement.form, tr("Other ink clearance"), settings, "rehWaw.neighborGap", beh.neighborGap,
      0, 1000, 1, 2, this, changed, tr("Minimum outline clearance to other letters and marks. Normal same-word/different-word gaps remain preferences."), units);
  addDoubleRow(behPlacement.form, tr("Allowed overhang"), settings, "rehWaw.overhangPercent", beh.overhangPercent,
      0, 100, 1, 1, this, changed, tr("Maximum left-slot overhang beyond Behshape, as a percentage of dot width."), percent);
  addDoubleRow(behPlacement.form, tr("Overhang cap"), settings, "rehWaw.overhangLimit", beh.overhangLimit,
      0, 1000, 1, 2, this, changed, tr("Absolute cap on the allowed left-slot overhang."), units);
  compliance(behPlacement.form, tr("Kasra horizontal compliance"), "compliance.rehWawKasraFollow", comp.rehWawKasraFollow);
  auto pair = addConstraint(special, tr("Kasra / iqlab meem pair"), settings,
      "toggle.matchMarkPosition", toggles.matchMarkPosition, this, changed);
  compliance(pair.form, tr("Compliance"), "compliance.matchMarkPosition", comp.matchMarkPosition);
  auto bowl = addConstraint(special, tr("Bowl-mark clusters"), settings,
      "toggle.bowlCluster", toggles.bowlCluster, this, changed);
  compliance(bowl.form, tr("Compliance"), "compliance.bowlCluster", comp.bowlCluster);
  auto squeeze = addConstraint(special, tr("Center squeezed marks"), settings,
      "toggle.squeezeCenter", toggles.squeezeCenter, this, changed);
  compliance(squeeze.form, tr("Compliance"), "compliance.squeezeCenter", comp.squeezeCenter);
  constraints->restore(settings, "ui.placementSection");

  auto* reports = addTab(tabs, tr("Reports"));
  auto* collectBox = new QGroupBox(tr("Collection"), reports);
  auto* collectForm = makeForm(collectBox);
  addToggleRow(collectForm, tr("Collect placement violations"), settings, "toggle.reportViolations",
      toggles.reportViolations, this, changed, tr("Enable violation reporting when generating the Mushaf."));
  static_cast<QVBoxLayout*>(reports->layout())->addWidget(collectBox);
  addNote(reports, tr("These settings select and interpret report findings; they do not change placement forces."));
  auto* reportSections = new CollapsibleSections(reports);
  static_cast<QVBoxLayout*>(reports->layout())->addWidget(reportSections);
  auto* selection = addSection(reportSections, tr("Finding selection"));
  auto* selectionFormWidget = new QWidget(selection);
  auto* selectionForm = makeForm(selectionFormWidget);
  static_cast<QVBoxLayout*>(selection->layout())->addWidget(selectionFormWidget);
  addToggleRow(selectionForm, tr("Only new / worsened findings"), settings, "reportOnlyChanged",
      m_params.reportOnlyChanged, this, changed, tr("Structural findings remain included."));
  addDoubleRow(selectionForm, tr("Minimum excess severity"), settings, "minViolationSeverity", m_params.minViolationSeverity,
      0.0, 10000.0, 1.0, 3, this, changed, tr("Exclude non-structural findings below this severity after allowed slack."), units);
  addIntRow(selectionForm, tr("Maximum findings"), settings, "reportMaxFindings", m_params.reportMaxFindings,
      0, 1000000, this, changed, tr("Limit for the whole report, after filtering and sorting."), tr("Unlimited"));
  const QString sortKey = settingKey("reportSort");
  m_params.reportSort = settings.value(sortKey, QString::fromStdString(m_params.reportSort)).toString().toStdString();
  if (m_params.reportSort != "severity" && m_params.reportSort != "priority") m_params.reportSort = "severity";
  auto* sort = new QComboBox(selectionFormWidget);
  sort->addItem(tr("Largest excess severity"), "severity");
  sort->addItem(tr("Review priority, then severity"), "priority");
  sort->setCurrentIndex(sort->findData(QString::fromStdString(m_params.reportSort)));
  sort->setToolTip(tr("Critical structural findings are always sorted first."));
  selectionForm->addRow(tr("Sort order"), sort);
  connect(sort, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, sortKey, sort](int) {
    m_params.reportSort = sort->currentData().toString().toStdString();
    QSettings().setValue(sortKey, QString::fromStdString(m_params.reportSort));
    scheduleChanged();
  });

  auto* diagnostics = addSection(reportSections, tr("Diagnostic types"));
  auto gapReport = addConstraint(diagnostics, tr("Collision / gap findings"), settings,
      "toggle.reportGenericGap", toggles.reportGenericGap, this, changed);
  auto* saveCollision = addToggleRow(gapReport.form, tr("Use Save Collision check"), settings,
      "reportGenericGapCollisionsOnly", m_params.reportGenericGapCollisionsOnly, this, changed,
      tr("Use filled-ink geometry and the clearance below. When unchecked, report solver-proxy gap residuals instead."));
  auto* clearance = addDoubleRow(gapReport.form, tr("Save Collision clearance"), settings,
      "collisionReportMinGap", m_params.collisionReportMinGap, 0.0, 1000.0, 1.0, 2, this, changed,
      tr("Reporting threshold for the Save Collision ink check."), units);
  gapReport.form->labelForField(clearance)->setEnabled(m_params.reportGenericGapCollisionsOnly);
  clearance->setEnabled(m_params.reportGenericGapCollisionsOnly);
  connect(saveCollision, &QCheckBox::toggled, this, [gapReport, clearance](bool checked) {
    gapReport.form->labelForField(clearance)->setEnabled(checked);
    clearance->setEnabled(checked);
  });
  auto* extra = new QWidget(diagnostics);
  auto* extraForm = makeForm(extra);
  static_cast<QVBoxLayout*>(diagnostics->layout())->addWidget(extra);
  addToggleRow(extraForm, tr("Placement audit"), settings, "toggle.reportPlacementAudit", toggles.reportPlacementAudit,
      this, changed, tr("Read-only checks of mark side, classification and base association."));
  addToggleRow(extraForm, tr("Soft target residuals"), settings, "toggle.reportSoftResiduals", toggles.reportSoftResiduals,
      this, changed, tr("Include residuals of placement preferences such as alignment and desired height."));
  addToggleRow(extraForm, tr("Waqf height-bound residuals"), settings, "toggle.reportWaqfBounds", toggles.reportWaqfBounds,
      this, changed, tr("Include residuals of the solver's waqf floor and ceiling constraints."));

  auto* allowances = addSection(reportSections, tr("Reporting allowances"));
  auto* allowanceWidget = new QWidget(allowances);
  auto* allowanceForm = makeForm(allowanceWidget);
  static_cast<QVBoxLayout*>(allowances->layout())->addWidget(allowanceWidget);
  addDoubleRow(allowanceForm, tr("Base vicinity: marks"), settings, "baseVicinityMarkTolerancePercent", m_params.baseVicinityMarkTolerancePercent,
      0.0, 100.0, 1.0, 2, this, changed, tr("Allowed overflow beyond the vicinity boundary, as a percentage of mark width."), percent);
  addDoubleRow(allowanceForm, tr("Base vicinity: dots"), settings, "baseVicinityDotTolerancePercent", m_params.baseVicinityDotTolerancePercent,
      0.0, 100.0, 1.0, 2, this, changed, tr("Allowed overflow beyond the vicinity boundary, as a percentage of dot width."), percent);
  addDoubleRow(allowanceForm, tr("Waqf: left drift"), settings, "waqfLeftDriftTolerancePercent", m_params.waqfLeftDriftTolerancePercent,
      0.0, 1000.0, 5.0, 2, this, changed, tr("Allowed left drift, as a percentage of waqf width."), percent);
  addDoubleRow(allowanceForm, tr("Waqf: right drift"), settings, "waqfRightDriftTolerancePercent", m_params.waqfRightDriftTolerancePercent,
      0.0, 1000.0, 5.0, 2, this, changed, tr("Allowed right drift, as a percentage of waqf width."), percent);
  addDoubleRow(allowanceForm, tr("Waqf: previous-line margin"), settings, "waqfPreviousLineMarginPercent", m_params.waqfPreviousLineMarginPercent,
      0.0, 100.0, 1.0, 2, this, changed, tr("Required margin below the previous baseline, as a percentage of line spacing."), percent);
  addDoubleRow(allowanceForm, tr("Proxy gap slack multiplier"), settings, "complianceResidualMargin", m_params.complianceResidualMargin,
      0.0, 100.0, 0.05, 2, this, changed,
      tr("Multiplier on expected compliance slack for solver-proxy gap findings. Actual intersections remain reportable; Save Collision mode does not use this allowance."));
  reportSections->restore(settings, "ui.reportSection");

  auto* advanced = addTab(tabs, tr("Advanced"));
  auto* search = new QGroupBox(tr("Collision candidate search"), advanced);
  auto* searchForm = makeForm(search);
  addDoubleRow(searchForm, tr("Bounding-box padding"), settings, "broadphasePadding", m_params.broadphasePadding,
      0.0, 10000.0, 1.0, 1, this, changed,
      tr("Minimum expansion of each glyph's bounding box for contact search. The effective padding is also at least half the largest target gap."), units);
  static_cast<QVBoxLayout*>(advanced->layout())->addWidget(search);
  addNote(advanced, tr("Search coverage expands automatically for configured gap targets. Extra padding increases candidate checks. The default minimum padding is 140 units."));
  static_cast<QVBoxLayout*>(advanced->layout())->addStretch(1);
  const QString tabKey = settingKey("ui.currentTab");
  tabs->setCurrentIndex(std::clamp(settings.value(tabKey, 0).toInt(), 0, tabs->count() - 1));
  connect(tabs, &QTabWidget::currentChanged, this, [tabKey](int index) { QSettings().setValue(tabKey, index); });
}

void SolverParametersWidget::scheduleChanged() {
  m_debounce->start(kDebounceMs);
}
