#pragma once
#include <QWidget>

#include "LookupCaseGallery.h"
#include "LookupPatterns.h"
class QComboBox;
class LookupCaseGallery;
class QGraphicsView;
class QGraphicsScene;
class QDoubleSpinBox;
class QSpinBox;
class QLabel;
class QCheckBox;
class QUndoStack;
class OtLayout;
class QGraphicsPathItem;
class QGraphicsEllipseItem;
class QGraphicsLineItem;
class LookupAnchorChanges : public QObject {
  Q_OBJECT
 public:
  using QObject::QObject;
 signals:
  void anchorChanged(QString glyph);
};
class LookupPatternEditor : public QWidget {
  Q_OBJECT
 public:
  LookupPatternEditor(OtLayout&, const QString& lookup, QUndoStack*, QWidget* parent = nullptr);
  ~LookupPatternEditor() override;
  void refreshPreview();
  QString caseText(quint64 index) const;
 signals:
  void saveRequested();
  void glyphEditRequested(QString name);
  void draggingChanged(bool active);

 private:
  void loadCases();
  quint64 pageSize() const;
  quint64 totalCases() const;
  std::pair<const digitalkhatt::layout::LookupPattern*, quint64> patternCase(quint64 index) const;
  LookupCaseThumbnail renderCase(quint64 index);
  void renderPreview(bool fitView = true);
  void updateScenePreview();
  void updateAnchorItems();
  void anchorChanged(const QString& glyph);
  bool beginDrag(digitalkhatt::layout::AnchorRole);
  void dragBy(Point delta);
  void endDrag();
  void updateTargetControls();
  std::optional<digitalkhatt::layout::AnchorTarget> target(digitalkhatt::layout::AnchorRole) const;
  GlyphParameters currentParameters() const;
  void applyOffset(digitalkhatt::layout::AnchorRole, std::optional<Point>);
  OtLayout& layout_;
  std::string lookup_;
  QUndoStack* undo_;
  std::vector<digitalkhatt::layout::LookupPattern> patterns_;
  digitalkhatt::layout::PatternPreview preview_;
  std::vector<std::uint16_t> sequence_;
  std::uint64_t page_ = 0;
  bool rendering_ = false, editable_ = false;
  LookupAnchorChanges* changes_;
  struct SceneRow {
    std::vector<GlyphParameters> parameters;
    double shift;
    std::vector<QGraphicsPathItem*> items;
  };
  std::vector<SceneRow> sceneRows_;
  QGraphicsEllipseItem *baseRing_ = nullptr, *markDot_ = nullptr, *baseDefault_ = nullptr,
                       *markDefault_ = nullptr, *dragGhost_ = nullptr;
  QGraphicsLineItem *baseGuide_ = nullptr, *markGuide_ = nullptr, *dragGuide_ = nullptr;
  struct DragState {
    digitalkhatt::layout::AnchorTarget target;
    GlyphParameters parameters;
    Point initialAnchor;
    QPointF initialSceneAnchor;
    quint64 gesture;
    bool macro = false;
  };
  std::optional<DragState> drag_;
  QComboBox *rules_, *attachment_, *variationRole_, *caseLimit_;
  LookupCaseGallery* cases_;
  QGraphicsView* view_;
  QGraphicsScene* scene_;
  QDoubleSpinBox *left_, *right_;
  QSpinBox *offsetX_, *offsetY_, *markOffsetX_, *markOffsetY_;
  QLabel *count_, *status_, *source_;
  QCheckBox *preceding_, *compare_;
};
