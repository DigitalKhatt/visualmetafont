#include "LookupPatternEditor.h"

#include <QtWidgets>
#include <algorithm>
#include <functional>
#include <tuple>

#include "GlyphRenderingQt.h"
#include "GlyphVis.h"
#include "LookupCaseGallery.h"
#include "OtLayout.h"
using namespace digitalkhatt::layout;
namespace {
class OffsetCommand final : public QUndoCommand {
  OtLayout& layout_;
  LookupAnchorChanges* changes_;
  AnchorTarget target_;
  std::optional<Point> before_, after_;
  quint64 gesture_;

 public:
  OffsetCommand(OtLayout& layout, LookupAnchorChanges* changes, AnchorTarget target,
                std::optional<Point> after, quint64 gesture = 0)
      : layout_(layout),
        changes_(changes),
        target_(std::move(target)),
        before_(anchorAdjustment(layout_, target_)),
        after_(after),
        gesture_(gesture) {
    setText(
        QObject::tr("Adjust %1 / %2")
            .arg(QString::fromStdString(target_.glyph), QString::fromStdString(target_.markClass)));
  }
  int id() const override { return gesture_ ? 0x4b484154 : -1; }
  bool mergeWith(const QUndoCommand* other) override {
    auto* next = dynamic_cast<const OffsetCommand*>(other);
    if (!next || !gesture_ || gesture_ != next->gesture_ || !(target_ == next->target_))
      return false;
    after_ = next->after_;
    return true;
  }
  void apply(std::optional<Point> value) {
    setAnchorAdjustment(layout_, target_, value);
    emit changes_->anchorChanged(QString::fromStdString(target_.glyph));
  }
  void redo() override { apply(after_); }
  void undo() override { apply(before_); }
};
struct GestureHandlers {
  std::function<bool()> begin;
  std::function<void(Point)> move;
  std::function<void()> end;
};
// The scene items stay alive throughout a gesture. Updated shaping drives their
// geometry; rebuilding the scene from a mouseMoveEvent would delete the grabber.
template <class Base>
class DragItem final : public Base {
  QPointF start_;
  bool active_ = false;
  GestureHandlers handlers_;

 public:
  explicit DragItem(GestureHandlers handlers) : handlers_(std::move(handlers)) {
    this->setAcceptedMouseButtons(Qt::LeftButton);
    this->setCursor(Qt::SizeAllCursor);
  }

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* e) override {
    start_ = e->scenePos();
    active_ = handlers_.begin();
    if (active_)
      e->accept();
    else
      e->ignore();
  }
  void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override {
    if (!active_) {
      e->ignore();
      return;
    }
    const auto delta = e->scenePos() - start_;
    handlers_.move(Point{qRound(delta.x()), qRound(delta.y())});
    e->accept();
  }
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override {
    if (!active_) {
      e->ignore();
      return;
    }
    const auto delta = e->scenePos() - start_;
    handlers_.move(Point{qRound(delta.x()), qRound(delta.y())});
    active_ = false;
    handlers_.end();
    e->accept();
  }
};
QString pointText(Point p) { return QString("(%1, %2)").arg(p.x()).arg(p.y()); }
}  // namespace
LookupPatternEditor::LookupPatternEditor(OtLayout& layout, const QString& lookup, QUndoStack* undo,
                                         QWidget* parent)
    : QWidget(parent), layout_(layout), lookup_(lookup.toStdString()), undo_(undo) {
  setObjectName("lookupPatternEditor");
  changes_ =
      undo_->findChild<LookupAnchorChanges*>("lookupAnchorChanges", Qt::FindDirectChildrenOnly);
  if (!changes_) {
    changes_ = new LookupAnchorChanges(undo_);
    changes_->setObjectName("lookupAnchorChanges");
  }
  connect(changes_, &LookupAnchorChanges::anchorChanged, this, &LookupPatternEditor::anchorChanged);
  auto* outer = new QVBoxLayout(this);
  outer->setContentsMargins(5, 5, 5, 5);
  auto* header = new QHBoxLayout;
  rules_ = new QComboBox(this);
  rules_->setObjectName("rulePatternSelector");
  rules_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  rules_->setMinimumContentsLength(18);
  header->addWidget(rules_, 1);
  header->addWidget(new QLabel(tr("Cases")));
  caseLimit_ = new QComboBox;
  caseLimit_->setObjectName("patternCaseLimit");
  caseLimit_->addItem("1000", qulonglong(1000));
  caseLimit_->addItem("10000", qulonglong(10000));
  caseLimit_->addItem(tr("All"), qulonglong(0));
  header->addWidget(caseLimit_);
  auto* editSelected = new QToolButton;
  editSelected->setText(tr("Edit selected"));
  editSelected->setCheckable(true);
  editSelected->setObjectName("editSelectedPattern");
  header->addWidget(editSelected);
  auto* maximize = new QPushButton(tr("Maximize"));
  maximize->setObjectName("maximizeLookupEditor");
  header->addWidget(maximize);
  connect(maximize, &QPushButton::clicked, this, [this, maximize] {
    if (window()->isMaximized()) {
      window()->showNormal();
      maximize->setText(tr("Maximize"));
    } else {
      window()->showMaximized();
      maximize->setText(tr("Restore"));
    }
  });
  auto* save = new QPushButton(tr("Save adjustments"));
  save->setObjectName("saveLookupAdjustments");
  header->addWidget(save);
  outer->addLayout(header);
  auto* split = new QSplitter(this);
  outer->addWidget(split, 1);
  auto* listPane = new QWidget;
  auto* listLayout = new QVBoxLayout(listPane);
  listLayout->setContentsMargins(0, 0, 0, 0);
  cases_ = new LookupCaseGallery;
  listLayout->addWidget(cases_, 1);
  auto* paging = new QHBoxLayout;
  auto* previous = new QPushButton(tr("Previous"));
  previous->setObjectName("previousPatternPage");
  auto* next = new QPushButton(tr("Next"));
  next->setObjectName("nextPatternPage");
  count_ = new QLabel;
  paging->addWidget(previous);
  paging->addWidget(next);
  listLayout->addLayout(paging);
  listLayout->addWidget(count_);
  split->addWidget(listPane);
  auto* detail = new QWidget;
  auto* detailLayout = new QVBoxLayout(detail);
  detailLayout->setContentsMargins(2, 0, 0, 0);
  auto* targets = new QHBoxLayout;
  attachment_ = new QComboBox;
  attachment_->setObjectName("patternAttachment");
  targets->addWidget(attachment_, 1);
  detailLayout->addLayout(targets);
  auto* parameters = new QHBoxLayout;
  variationRole_ = new QComboBox;
  variationRole_->setObjectName("patternVariationGlyph");
  variationRole_->addItems({tr("Stretch base"), tr("Stretch mark")});
  parameters->addWidget(variationRole_);
  auto spin = [&](QString label, QString name, double minimum, double maximum, double value) {
    parameters->addWidget(new QLabel(label));
    auto* s = new QDoubleSpinBox;
    s->setObjectName(name);
    s->setRange(minimum, maximum);
    s->setDecimals(2);
    s->setSingleStep(.25);
    s->setValue(value);
    parameters->addWidget(s);
    return s;
  };
  left_ = spin(tr("Left"), "patternLeft", -2, 60, 0);
  right_ = spin(tr("Right"), "patternRight", -2, 60, 0);
  detailLayout->addLayout(parameters);
  auto* options = new QHBoxLayout;
  preceding_ = new QCheckBox(tr("Earlier positioning"));
  preceding_->setObjectName("patternPreceding");
  preceding_->setChecked(true);
  compare_ = new QCheckBox(tr("Compare variations"));
  compare_->setObjectName("comparePatternVariations");
  options->addWidget(preceding_);
  options->addWidget(compare_);
  auto* defaults = new QPushButton(tr("Default parameters"));
  options->addWidget(defaults);
  detailLayout->addLayout(options);
  scene_ = new QGraphicsScene(this);
  scene_->setObjectName("lookupPatternScene");
  view_ = new QGraphicsView(scene_);
  view_->setObjectName("lookupPatternView");
  view_->setRenderHints(QPainter::Antialiasing);
  view_->setMinimumSize(320, 260);
  view_->setDragMode(QGraphicsView::ScrollHandDrag);
  view_->setBackgroundBrush(Qt::white);
  detailLayout->addWidget(view_, 1);
  auto adjustmentRow = [&](QString label, QString xName, QString yName, QString resetName,
                           AnchorRole role) {
    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel(label));
    auto* x = new QSpinBox;
    auto* y = new QSpinBox;
    x->setObjectName(xName);
    y->setObjectName(yName);
    for (auto* spin : {x, y}) {
      spin->setRange(-10000, 10000);
      row->addWidget(spin);
    }
    auto* reset = new QPushButton(tr("Reset"));
    reset->setObjectName(resetName);
    row->addWidget(reset);
    auto* edit = new QPushButton(tr("Open glyph…"));
    row->addWidget(edit);
    detailLayout->addLayout(row);
    const auto apply = [this, role, x, y] { applyOffset(role, Point{x->value(), y->value()}); };
    connect(x, &QSpinBox::editingFinished, this, apply);
    connect(y, &QSpinBox::editingFinished, this, apply);
    connect(reset, &QPushButton::clicked, this, [this, role] { applyOffset(role, std::nullopt); });
    connect(edit, &QPushButton::clicked, this, [this, role] {
      if (auto t = target(role)) emit glyphEditRequested(QString::fromStdString(t->glyph));
    });
    return std::pair{x, y};
  };
  std::tie(offsetX_, offsetY_) = adjustmentRow(tr("Base X / Y"), "anchorOffsetX", "anchorOffsetY",
                                               "resetAnchorAdjustment", AnchorRole::Base);
  std::tie(markOffsetX_, markOffsetY_) =
      adjustmentRow(tr("Mark X / Y"), "markAnchorOffsetX", "markAnchorOffsetY",
                    "resetMarkAnchorAdjustment", AnchorRole::Mark);
  auto* hint = new QLabel(tr(
      "Drag the mark to adjust its base anchor. Drag the orange dot to adjust the mark anchor."));
  hint->setWordWrap(true);
  detailLayout->addWidget(hint);
  auto* history = new QHBoxLayout;
  auto* undoButton = new QToolButton;
  undoButton->setDefaultAction(undo_->createUndoAction(this, tr("Undo")));
  auto* redoButton = new QToolButton;
  redoButton->setDefaultAction(undo_->createRedoAction(this, tr("Redo")));
  history->addWidget(undoButton);
  history->addWidget(redoButton);
  status_ = new QLabel;
  status_->setObjectName("patternAppliedStatus");
  status_->setWordWrap(true);
  history->addWidget(status_, 1);
  detailLayout->addLayout(history);
  source_ = new QLabel;
  source_->setObjectName("anchorResolutionSource");
  source_->setWordWrap(true);
  source_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  detailLayout->addWidget(source_);
  split->addWidget(detail);
  detail->hide();
  connect(editSelected, &QToolButton::toggled, detail, &QWidget::setVisible);
  split->setStretchFactor(0, 2);
  split->setStretchFactor(1, 1);
  split->setSizes({760, 600});
  patterns_ = lookupPatterns(layout_, lookup_);
  rules_->addItem(tr("All groups"));
  for (const auto& p : patterns_)
    rules_->addItem(
        QString::fromStdString(p.subtable + (p.markClass.empty() ? "" : " / " + p.markClass)));
  connect(rules_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
    page_ = 0;
    loadCases();
  });
  connect(caseLimit_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
    page_ = 0;
    loadCases();
  });
  connect(cases_, &LookupCaseGallery::currentChanged, this, [this] { renderPreview(); });
  connect(previous, &QPushButton::clicked, this, [this] {
    if (page_) {
      --page_;
      loadCases();
    }
  });
  connect(next, &QPushButton::clicked, this, [this] {
    if (rules_->currentIndex() >= 0 && totalCases() > 0 &&
        page_ < ((totalCases() - 1) / pageSize())) {
      ++page_;
      loadCases();
    }
  });
  for (auto* s : {left_, right_})
    connect(s, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [this] { refreshPreview(); });
  connect(attachment_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this] { renderPreview(); });
  connect(variationRole_, qOverload<int>(&QComboBox::currentIndexChanged), this,
          [this] { refreshPreview(); });
  connect(preceding_, &QCheckBox::toggled, this, [this] { refreshPreview(); });
  connect(compare_, &QCheckBox::toggled, this, [this] { renderPreview(); });
  connect(defaults, &QPushButton::clicked, this, [this] {
    QSignalBlocker a(left_), b(right_);
    left_->setValue(0);
    right_->setValue(0);
    refreshPreview();
  });
  connect(save, &QPushButton::clicked, this, &LookupPatternEditor::saveRequested);
  connect(this, &LookupPatternEditor::draggingChanged, save, &QPushButton::setDisabled);
  loadCases();
}
quint64 LookupPatternEditor::totalCases() const {
  if (rules_->currentIndex() > 0) return patterns_.at(rules_->currentIndex() - 1).combinations();
  quint64 total = 0;
  for (const auto& p : patterns_) {
    const auto n = p.combinations();
    if (n > std::numeric_limits<quint64>::max() - total) return std::numeric_limits<quint64>::max();
    total += n;
  }
  return total;
}
std::pair<const LookupPattern*, quint64> LookupPatternEditor::patternCase(quint64 index) const {
  if (rules_->currentIndex() > 0) return {&patterns_.at(rules_->currentIndex() - 1), index};
  for (const auto& p : patterns_) {
    const auto n = p.combinations();
    if (index < n) return {&p, index};
    index -= n;
  }
  throw std::out_of_range("Pattern case");
}
quint64 LookupPatternEditor::pageSize() const {
  const auto limit = caseLimit_->currentData().toULongLong();
  return limit ? limit : std::max<quint64>(1, totalCases());
}
QString LookupPatternEditor::caseText(quint64 index) const {
  if (rules_->currentIndex() < 0 || index >= cases_->count()) return {};
  QStringList names;
  const auto [pattern, local] = patternCase(page_ * pageSize() + index);
  for (auto code : pattern->sequence(local))
    names << QString::fromStdString(layout_.glyphNamePerCode.at(code));
  return names.join(" · ");
}
void LookupPatternEditor::loadCases() {
  if (patterns_.empty() || rules_->currentIndex() < 0) {
    cases_->setCases(0, {});
    findChild<QPushButton*>("previousPatternPage")->setEnabled(false);
    findChild<QPushButton*>("nextPatternPage")->setEnabled(false);
    count_->setText(tr("No supported positioning rules"));
    status_->setText(tr("Mark attachment and contextual positioning lookups are supported."));
    return;
  }
  const auto total = totalCases();
  const auto begin = page_ * pageSize();
  const auto count = std::min<quint64>(pageSize(), total - begin);
  count_->setText(tr("%1–%2 of %3 cases")
                      .arg(qulonglong(begin + 1))
                      .arg(qulonglong(begin + count))
                      .arg(qulonglong(total)));
  cases_->setCases(count, [this](quint64 index) { return renderCase(index); });
  findChild<QPushButton*>("previousPatternPage")->setEnabled(page_ > 0);
  findChild<QPushButton*>("nextPatternPage")->setEnabled(count < total - begin);
}
LookupCaseThumbnail LookupPatternEditor::renderCase(quint64 index) {
  LookupCaseThumbnail result;
  result.label = caseText(index);
  const auto [pattern, local] = patternCase(page_ * pageSize() + index);
  const auto& p = *pattern;
  const auto sequence = p.sequence(local);
  for (auto code : sequence) {
    result.glyphDependencies.insert(QString::fromStdString(layout_.glyphNamePerCode.at(code)));
    result.glyphDependencies.insert(
        QString::fromStdString(layout_.resolveGlyphInstance(code).sourceGlyph->name));
  }
  std::vector<GlyphParameters> parameters(sequence.size());
  auto preview =
      previewLookupPattern(layout_, lookup_, sequence, parameters, preceding_->isChecked());
  if (!currentParameters().isDefault() && !preview.attachments.empty()) {
    const auto& a = preview.attachments.front();
    parameters[variationRole_->currentIndex() == 0 ? a.base : a.mark] = currentParameters();
    preview = previewLookupPattern(layout_, lookup_, sequence, parameters, preceding_->isChecked());
  }
  result.path.setFillRule(Qt::WindingFill);
  for (const auto& g : preview.glyphs) {
    auto path = digitalkhatt::qt::pathForGlyph(*layout_.getGlyph(g.code, g.parameters));
    path.translate(g.x, g.y);
    result.path.addPath(path);
  }
  result.applied = std::find(preview.appliedSubtables.begin(), preview.appliedSubtables.end(),
                             p.subtableIndex) != preview.appliedSubtables.end();
  if (!result.applied) result.label += tr(" · rule not applied");
  return result;
}
GlyphParameters LookupPatternEditor::currentParameters() const {
  GlyphParameters p;
  p.lefttatweel = left_->value();
  p.righttatweel = right_->value();
  return p;
}
LookupPatternEditor::~LookupPatternEditor() {
  if (drag_ && drag_->macro) undo_->endMacro();
}
void LookupPatternEditor::refreshPreview() {
  cases_->invalidatePreviews();
  if (drag_)
    updateScenePreview();
  else
    renderPreview();
}
std::optional<AnchorTarget> LookupPatternEditor::target(AnchorRole role) const {
  const int index = attachment_->currentIndex();
  if (index < 0 || std::size_t(index) >= preview_.attachments.size()) return std::nullopt;
  const auto& a = preview_.attachments[index];
  const auto glyphIndex = role == AnchorRole::Base ? a.base : a.mark;
  if (glyphIndex >= preview_.glyphs.size()) return std::nullopt;
  return AnchorTarget{a.lookup, a.markClass,
                      layout_.glyphNamePerCode.at(preview_.glyphs[glyphIndex].code), a.subtable,
                      role};
}
void LookupPatternEditor::anchorChanged(const QString& glyph) {
  cases_->invalidateGlyph(glyph);
  const bool affected = std::any_of(sequence_.begin(), sequence_.end(), [&](auto code) {
    return QString::fromStdString(layout_.glyphNamePerCode.at(code)) == glyph ||
           QString::fromStdString(layout_.resolveGlyphInstance(code).sourceGlyph->name) == glyph;
  });
  if (!affected) return;
  try {
    if (drag_)
      updateScenePreview();
    else
      renderPreview(false);
  } catch (const std::exception& e) {
    status_->setText(QString::fromUtf8(e.what()));
  }
}
void LookupPatternEditor::renderPreview(bool fitView) {
  if (rendering_ || drag_ || rules_->currentIndex() < 0 ||
      cases_->currentIndex() >= cases_->count())
    return;
  rendering_ = true;
  struct Guard {
    bool& value;
    ~Guard() { value = false; }
  } guard{rendering_};
  const auto oldCenter = view_->mapToScene(view_->viewport()->rect().center());
  const auto oldTransform = view_->transform();
  try {
    const auto [pattern, local] = patternCase(page_ * pageSize() + cases_->currentIndex());
    const auto& p = *pattern;
    sequence_ = p.sequence(local);
    std::vector<GlyphParameters> parameters(sequence_.size());
    auto initial =
        previewLookupPattern(layout_, lookup_, sequence_, parameters, preceding_->isChecked());
    const int selected = std::clamp(attachment_->currentIndex(), 0,
                                    std::max(0, int(initial.attachments.size()) - 1));
    if (!initial.attachments.empty()) {
      const auto& a = initial.attachments[selected];
      parameters[variationRole_->currentIndex() == 0 ? a.base : a.mark] = currentParameters();
    }
    preview_ =
        previewLookupPattern(layout_, lookup_, sequence_, parameters, preceding_->isChecked());
    {
      QSignalBlocker blocker(attachment_);
      attachment_->clear();
      for (const auto& a : preview_.attachments)
        attachment_->addItem(QString::fromStdString(
            layout_.glyphNamePerCode.at(preview_.glyphs[a.base].code) + " → " +
            layout_.glyphNamePerCode.at(preview_.glyphs[a.mark].code) + " / " + a.markClass));
      if (attachment_->count())
        attachment_->setCurrentIndex(std::min(selected, attachment_->count() - 1));
    }
    editable_ = preview_.selectedLookupApplied &&
                std::find(preview_.appliedSubtables.begin(), preview_.appliedSubtables.end(),
                          p.subtableIndex) != preview_.appliedSubtables.end();
    status_->setText(editable_ ? tr("Selected rule applied · glyph-level witness")
                               : tr("Selected rule did not apply to this witness · preview only"));
    sceneRows_.clear();
    scene_->clear();
    baseRing_ = markDot_ = baseDefault_ = markDefault_ = dragGhost_ = nullptr;
    baseGuide_ = markGuide_ = dragGuide_ = nullptr;
    const auto handlers = [this](AnchorRole role) {
      return GestureHandlers{[this, role] { return beginDrag(role); },
                             [this](Point delta) { dragBy(delta); }, [this] { endDrag(); }};
    };
    auto draw = [&](const PatternPreview& sample, const std::vector<GlyphParameters>& params,
                    double shift, bool interactive) {
      SceneRow row{params, shift, {}};
      QRectF bounds;
      for (std::size_t i = 0; i < sample.glyphs.size(); ++i) {
        const auto& g = sample.glyphs[i];
        auto& glyph = layout_.glyphs.at(layout_.glyphNamePerCode.at(g.code));
        QGraphicsPathItem* item = nullptr;
        const bool attached = interactive && editable_ && attachment_->currentIndex() >= 0 &&
                              i == preview_.attachments[attachment_->currentIndex()].mark;
        if (attached) {
          item = new DragItem<QGraphicsPathItem>(handlers(AnchorRole::Base));
          scene_->addItem(item);
          item->setData(0, "lookupMarkBody");
          item->setToolTip(tr("Drag mark: adjust the base anchor"));
          item->setZValue(1);
        } else {
          item = scene_->addPath({});
          item->setAcceptedMouseButtons(Qt::NoButton);
          item->setToolTip(QString::fromStdString(glyph.name));
        }
        item->setPath(digitalkhatt::qt::pathForGlyph(*glyph.getAlternate(g.parameters)));
        item->setPen(Qt::NoPen);
        item->setBrush(Qt::black);
        item->setPos(g.x, g.y + shift);
        row.items.push_back(item);
        bounds |= item->sceneBoundingRect();
      }
      sceneRows_.push_back(std::move(row));
      return bounds;
    };
    const auto mainBounds = draw(preview_, parameters, 0, true);
    if (target(AnchorRole::Base)) {
      baseGuide_ = scene_->addLine({}, QPen(QColor("#367bb5"), 1, Qt::DashLine));
      markGuide_ = scene_->addLine({}, QPen(QColor("#bd671c"), 1, Qt::DashLine));
      auto point = [&](QColor color, int radius) {
        auto* item = scene_->addEllipse(-radius, -radius, 2 * radius, 2 * radius, QPen(color, 1.5),
                                        Qt::NoBrush);
        item->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        item->setAcceptedMouseButtons(Qt::NoButton);
        item->setZValue(9);
        return item;
      };
      baseDefault_ = point(QColor("#8fa2b3"), 4);
      markDefault_ = point(QColor("#b19a87"), 3);
      baseDefault_->setToolTip(tr("Calculated base point"));
      markDefault_->setToolTip(tr("Calculated mark point"));
      baseRing_ = point(QColor("#1677c8"), 8);
      baseRing_->setToolTip(tr("Base anchor"));
      if (editable_) {
        markDot_ = new DragItem<QGraphicsEllipseItem>(handlers(AnchorRole::Mark));
        scene_->addItem(markDot_);
        markDot_->setData(0, "lookupAnchorHandle");
      } else
        markDot_ = scene_->addEllipse({});
      markDot_->setRect(-4, -4, 8, 8);
      markDot_->setBrush(QColor("#c96f1c"));
      markDot_->setPen(QPen(Qt::white, 1));
      markDot_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      markDot_->setZValue(10);
      markDot_->setToolTip(
          tr("Drag dot: adjust the mark anchor; the mark repositions against the fixed base"));
      dragGuide_ = scene_->addLine({}, QPen(QColor("#c96f1c"), 1, Qt::DashLine));
      dragGuide_->setZValue(11);
      dragGuide_->hide();
      dragGhost_ = point(QColor("#c96f1c"), 5);
      dragGhost_->setZValue(12);
      dragGhost_->hide();
    }
    if (compare_->isChecked() && target(AnchorRole::Base)) {
      const auto& a = preview_.attachments[attachment_->currentIndex()];
      const auto index = variationRole_->currentIndex() == 0 ? a.base : a.mark;
      auto caption = [&](QString text, const QRectF& bounds) {
        auto* label = scene_->addSimpleText(text);
        label->setBrush(QColor("#55616e"));
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        label->setPos(bounds.left(), bounds.top() - 100);
      };
      caption(tr("Current parameters"), mainBounds);
      double bottom = mainBounds.top() - 400;
      for (double left : {-1., 4.}) {
        auto pp = parameters;
        pp[index].lefttatweel = left;
        try {
          const auto sample =
              previewLookupPattern(layout_, lookup_, sequence_, pp, preceding_->isChecked());
          double top = 0;
          for (const auto& g : sample.glyphs)
            top = std::max(top, g.y + layout_.getGlyph(g.code, g.parameters)->height);
          const auto bounds = draw(sample, pp, bottom - top, false);
          caption(tr("Left extension %1").arg(left), bounds);
          bottom = bounds.top() - 400;
        } catch (const std::exception& e) {
          status_->setText(status_->text() +
                           tr(" · Variation unavailable: %1")
                               .arg(QString::fromUtf8(e.what()).section('\n', 0, 0)));
        }
      }
    }
    updateAnchorItems();
    updateTargetControls();
    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-200, -300, 200, 200));
    if (fitView) {
      view_->resetTransform();
      view_->scale(1, -1);
      view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
    } else {
      view_->setTransform(oldTransform);
      view_->centerOn(oldCenter);
    }
  } catch (const std::exception& e) {
    sceneRows_.clear();
    scene_->clear();
    baseRing_ = markDot_ = baseDefault_ = markDefault_ = dragGhost_ = nullptr;
    baseGuide_ = markGuide_ = dragGuide_ = nullptr;
    preview_ = {};
    editable_ = false;
    {
      QSignalBlocker blocker(attachment_);
      attachment_->clear();
    }
    source_->clear();
    status_->setText(tr("Cannot preview: %1").arg(QString::fromUtf8(e.what())));
    updateTargetControls();
  }
}
void LookupPatternEditor::updateScenePreview() {
  for (std::size_t rowIndex = 0; rowIndex < sceneRows_.size(); ++rowIndex) {
    auto& row = sceneRows_[rowIndex];
    auto sample =
        previewLookupPattern(layout_, lookup_, sequence_, row.parameters, preceding_->isChecked());
    if (sample.glyphs.size() != row.items.size())
      throw std::runtime_error("Preview glyph count changed during dragging");
    for (std::size_t i = 0; i < sample.glyphs.size(); ++i) {
      const auto& g = sample.glyphs[i];
      row.items[i]->setPath(
          digitalkhatt::qt::pathForGlyph(*layout_.getGlyph(g.code, g.parameters)));
      row.items[i]->setPos(g.x, g.y + row.shift);
    }
    if (rowIndex == 0) preview_ = std::move(sample);
  }
  updateAnchorItems();
  updateTargetControls();
}
void LookupPatternEditor::updateAnchorItems() {
  const auto base = target(AnchorRole::Base), mark = target(AnchorRole::Mark);
  if (!base || !mark || !baseRing_) {
    source_->setText(tr("No editable mark attachment in this case."));
    return;
  }
  const auto& a = preview_.attachments[attachment_->currentIndex()];
  const auto& b = preview_.glyphs[a.base];
  const auto& m = preview_.glyphs[a.mark];
  const auto bi = describeAnchor(layout_, *base, b.parameters),
             mi = describeAnchor(layout_, *mark, m.parameters);
  const auto bp = QPointF(b.x, b.y) + QPointF(bi.resolved.x(), bi.resolved.y());
  const auto mp = QPointF(m.x, m.y) + QPointF(mi.resolved.x(), mi.resolved.y());
  baseRing_->setPos(bp);
  markDot_->setPos(mp);
  baseDefault_->setPos(QPointF(b.x, b.y) + QPointF(bi.calculated.x(), bi.calculated.y()));
  markDefault_->setPos(QPointF(m.x, m.y) + QPointF(mi.calculated.x(), mi.calculated.y()));
  baseGuide_->setLine(QLineF(baseDefault_->pos(), bp));
  markGuide_->setLine(QLineF(markDefault_->pos(), mp));
  source_->setText(tr("Base: %1\nCalculated %2 · adjustment %3 · resolved %4\nMark: %5\nCalculated "
                      "%6 · adjustment %7 · resolved %8")
                       .arg(QString::fromStdString(bi.source), pointText(bi.calculated),
                            pointText(bi.adjustment), pointText(bi.resolved),
                            QString::fromStdString(mi.source), pointText(mi.calculated),
                            pointText(mi.adjustment), pointText(mi.resolved)));
}
void LookupPatternEditor::updateTargetControls() {
  const QSignalBlocker a(offsetX_), b(offsetY_), c(markOffsetX_), d(markOffsetY_);
  const auto base = target(AnchorRole::Base), mark = target(AnchorRole::Mark);
  const auto bv = base ? anchorAdjustment(layout_, *base).value_or(Point{}) : Point{};
  const auto mv = mark ? anchorAdjustment(layout_, *mark).value_or(Point{}) : Point{};
  offsetX_->setValue(bv.x());
  offsetY_->setValue(bv.y());
  markOffsetX_->setValue(mv.x());
  markOffsetY_->setValue(mv.y());
  for (auto* spin : {offsetX_, offsetY_, markOffsetX_, markOffsetY_})
    spin->setEnabled(editable_ && !drag_);
}
void LookupPatternEditor::applyOffset(AnchorRole role, std::optional<Point> value) {
  if (!editable_ || drag_) return;
  if (auto t = target(role)) {
    if (anchorAdjustment(layout_, *t) != value)
      undo_->push(new OffsetCommand(layout_, changes_, *t, value));
  }
}
bool LookupPatternEditor::beginDrag(AnchorRole role) {
  if (!editable_ || drag_) return false;
  auto t = target(role);
  if (!t) return false;
  const auto& a = preview_.attachments[attachment_->currentIndex()];
  const auto index = role == AnchorRole::Base ? a.base : a.mark;
  static quint64 nextGesture = 1;
  drag_ = DragState{*t,
                    preview_.glyphs[index].parameters,
                    describeAnchor(layout_, *t, preview_.glyphs[index].parameters).resolved,
                    role == AnchorRole::Base ? baseRing_->scenePos() : markDot_->scenePos(),
                    nextGesture++,
                    false};
  for (auto* widget : {rules_, caseLimit_, attachment_, variationRole_}) widget->setEnabled(false);
  for (auto* spin : {left_, right_}) spin->setEnabled(false);
  preceding_->setEnabled(false);
  compare_->setEnabled(false);
  cases_->setEnabled(false);
  updateTargetControls();
  emit draggingChanged(true);
  return true;
}
void LookupPatternEditor::dragBy(Point delta) {
  if (!drag_) return;
  try {
    const auto at = describeAnchor(layout_, drag_->target, drag_->parameters).resolved;
    const auto value = adjustmentForMove(layout_, drag_->target, drag_->parameters,
                                         drag_->initialAnchor + delta - at);
    if (anchorAdjustment(layout_, drag_->target) != std::optional<Point>{value}) {
      if (!drag_->macro) {
        undo_->beginMacro(drag_->target.role == AnchorRole::Base ? tr("Move mark using base anchor")
                                                                 : tr("Move mark anchor"));
        drag_->macro = true;
      }
      undo_->push(new OffsetCommand(layout_, changes_, drag_->target, value, drag_->gesture));
    }
    if (drag_->target.role == AnchorRole::Mark && dragGhost_) {
      const auto cursor = drag_->initialSceneAnchor + QPointF(delta.x(), delta.y());
      dragGhost_->setPos(cursor);
      dragGuide_->setLine(QLineF(baseRing_->scenePos(), cursor));
      dragGhost_->show();
      dragGuide_->show();
    }
  } catch (const std::exception& e) {
    status_->setText(QString::fromUtf8(e.what()));
  }
}
void LookupPatternEditor::endDrag() {
  if (!drag_) return;
  const bool macro = drag_->macro;
  drag_.reset();
  if (macro) undo_->endMacro();
  for (auto* widget : {rules_, caseLimit_, attachment_, variationRole_}) widget->setEnabled(true);
  for (auto* spin : {left_, right_}) spin->setEnabled(true);
  preceding_->setEnabled(true);
  compare_->setEnabled(true);
  cases_->setEnabled(true);
  updateTargetControls();
  emit draggingChanged(false);
  // Defer deleting the released graphics item until its event handler returns.
  QTimer::singleShot(0, this, [this] {
    if (!drag_) renderPreview(false);
  });
}
