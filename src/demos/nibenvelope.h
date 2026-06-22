#pragma once

#include <QtGui>
#include <QtWidgets>
#include "metafont.h"
#include "nibenvelope.h"
#include "font.hpp"
#include <optional>

namespace nibenvelope {
struct CubicSegment {
  QPointF p0;
  QPointF c1;
  QPointF c2;
  QPointF p1;
};
enum class HandleKind {
  OutHandle,
  InHandle
};
enum class KnotContinuity {
  Smooth,
  Corner
};
struct Knot {
  QPointF pos;
  QPainterPath pen;

  std::optional<double> inDirDeg;
  std::optional<double> outDirDeg;

  double leftTension = 1.0;
  double rightTension = 1.0;

  double penAngle = 0;  // in degree
  KnotContinuity continuity = KnotContinuity::Smooth;
};
class ControlHandleItem;
struct ControlHandleGraphics {
  int segmentIndex = -1;
  int knotIndex = -1;
  HandleKind kind;

  QGraphicsLineItem* line = nullptr;
  ControlHandleItem* handle = nullptr;
};

class StrokeModel : public QObject {
  Q_OBJECT

 public:
  explicit StrokeModel(QObject* parent = nullptr) : QObject(parent) {}
  QVector<Knot> knots;
  QPainterPath defaultPen;
 signals:
  void changed();
};
class KnotItem : public QObject,
                 public QGraphicsEllipseItem {
  Q_OBJECT

 public:
  KnotItem(int index, QGraphicsItem* parent = nullptr);
  int index() const { return m_index; }

 signals:

  void moved(int index, QPointF);

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

 private:
  int m_index;
};
class AngleHandle : public QObject, public QGraphicsLineItem {
  Q_OBJECT

 public:
  AngleHandle(int knotIndex, qreal angleDeg, QGraphicsItem* parent = nullptr)
      : QGraphicsLineItem(parent),
        m_index(knotIndex),
        m_angle(angleDeg) {
    setPen(QPen(Qt::darkGreen, 2));
    setFlag(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::LeftButton);
    updateLine();
  }

 signals:
  void angleChanged(int index, qreal angleRad);

 protected:
  void mouseMoveEvent(QGraphicsSceneMouseEvent* ev) override {
    QPointF p = ev->pos();
    m_angle = qRadiansToDegrees(std::atan2(p.y(), p.x()));
    updateLine();
    emit angleChanged(m_index, m_angle);
  }

 private:
  void updateLine() {
    constexpr qreal L = 55.0;

    qreal a = qDegreesToRadians(m_angle);

    setLine(0, 0, L * std::cos(a), L * std::sin(a));
  }

 private:
  int m_index;
  qreal m_angle;
};
class KnobItem : public QObject,
                 public QGraphicsEllipseItem {
  Q_OBJECT

 public:
  explicit KnobItem(QGraphicsItem* parent = nullptr)
      : QObject(),
        QGraphicsEllipseItem(parent) {
    setFlag(ItemIsSelectable);
  }

 signals:
  void moved(QPointF);

 protected:
  QVariant itemChange(
      GraphicsItemChange change,
      const QVariant& value) override {
    if (change == ItemPositionHasChanged)
      emit moved(value.toPointF());

    return QGraphicsEllipseItem::itemChange(
        change,
        value);
  }
};
class PenAngleHandleItem : public QObject, public QGraphicsItem {
  Q_OBJECT

 public:
  PenAngleHandleItem(int knotIndex,
                     double angleDeg,
                     QGraphicsItem* parent = nullptr)
      : QGraphicsItem(parent),
        m_knotIndex(knotIndex),
        m_angleDeg(angleDeg) {
    setAcceptedMouseButtons(Qt::NoButton);

    m_line = new QGraphicsLineItem(this);
    m_line->setPen(QPen(Qt::darkGreen, 1.5));

    m_knob = new KnobItem(this);
    connect(m_knob, &KnobItem::moved, this, &PenAngleHandleItem::knobMoved);
    m_knob->setRect(-5, -5, 10, 10);
    m_knob->setBrush(Qt::darkGreen);
    m_knob->setPen(QPen(Qt::black, 1));
    m_knob->setAcceptedMouseButtons(Qt::LeftButton);
    m_knob->setFlag(QGraphicsItem::ItemIsMovable);
    m_knob->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
    m_knob->setZValue(30);
    m_line->setZValue(0);

    updateGeometry();
  }

  QRectF boundingRect() const override {
    return QRectF(-80, -80, 160, 160);
  }

  void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override {
  }

  void setAngleDeg(double a) {
    m_angleDeg = a;
    updateGeometry();
  }

 signals:
  void angleChanged(int knotIndex, double angleDeg);

 private:
  void knobMoved(QPointF localPos) {
    double a =
        qRadiansToDegrees(std::atan2(localPos.y(), localPos.x()));

    m_angleDeg = a;

    updateGeometry();

    emit angleChanged(m_knotIndex, m_angleDeg);
  }

  void updateGeometry() {
    constexpr double L = 55.0;

    double a = qDegreesToRadians(m_angleDeg);

    QPointF end(
        L * std::cos(a),
        L * std::sin(a));

    m_line->setLine(QLineF(QPointF(0, 0), end));

    {
      QSignalBlocker blocker(m_knob);
      m_knob->setPos(end);
    }
  }

 private:
  int m_knotIndex = -1;
  double m_angleDeg = 0.0;

  QGraphicsLineItem* m_line = nullptr;
  KnobItem* m_knob = nullptr;
};
class EnvelopeItem : public QGraphicsPathItem {
 public:
  void setEnvelope(mp_edge_object* edge);
};

class StrokeGraphicsView : public QGraphicsView {
  Q_OBJECT

 public:
  explicit StrokeGraphicsView(QGraphicsScene* scene, QWidget* parent = nullptr)
      : QGraphicsView(scene, parent) {}

 signals:
  void shiftClicked(QPointF scenePos);

 protected:
  void mousePressEvent(QMouseEvent* ev) override {
    if (ev->button() == Qt::LeftButton &&
        ev->modifiers() & Qt::ShiftModifier) {
      QPointF p = mapToScene(ev->pos());
      emit shiftClicked(p);
      return;
    }

    QGraphicsView::mousePressEvent(ev);
  }
};
class ControlHandleItem : public QObject, public QGraphicsEllipseItem {
  Q_OBJECT

 public:
  ControlHandleItem(int knotIndex, HandleKind kind, QGraphicsItem* parent = nullptr)
      : QGraphicsEllipseItem(parent),
        m_knotIndex(knotIndex),
        m_kind(kind) {
    setRect(-4, -4, 8, 8);
    setBrush(Qt::yellow);
    setPen(QPen(Qt::black, 1));

    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
    setZValue(20);
  }

 signals:
  void moved(int knotIndex, HandleKind kind, QPointF scenePos);

 protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override {
    if (change == ItemPositionHasChanged)
      emit moved(m_knotIndex, m_kind, scenePos());

    return QGraphicsEllipseItem::itemChange(change, value);
  }

 private:
  int m_knotIndex;
  HandleKind m_kind;
};
class PenStrokeEditor : public QWidget {
  Q_OBJECT

 public:
  PenStrokeEditor(Font* font, QWidget* parent = nullptr);

 private slots:
  void knotMoved(int index, QPointF p);
  void penAngleChanged(int index, qreal angleDeg);
  void rebuild(bool rebuildHandles);
  void addKnotAt(QPointF scenePos);
  void removeSelectedKnot();
  void controlHandleMoved(int knotIndex, HandleKind kind, QPointF scenePos);

 protected:
  void keyPressEvent(QKeyEvent* ev) override;

 private:
  MPGlyphInfo render(const StrokeModel& model);
  void rebuildSceneItems(bool rebuildHandles);
  void clearControlHandles();
  void rebuildControlHandles();
  void updateControlHandles();
  void clearDirection(int knotIndex, HandleKind kind);
  void updateRightTensionFromHandle(int knotIndex, double handleLength);
  void updateLeftTensionFromHandle(int knotIndex, double handleLength);

 private:
  QGraphicsView* m_view;
  QGraphicsScene* m_scene;

  StrokeModel m_model;

  QGraphicsPathItem* m_trajectoryItem = nullptr;
  EnvelopeItem* m_envelopeItem = nullptr;
  QVector<QGraphicsPathItem*> m_penItems;
  Font* m_font;
  QVector<KnotItem*> m_knotItems;
  QVector<ControlHandleGraphics> m_controlHandleGraphics;
  std::vector<CubicSegment> m_segments;
};

}  // namespace nibenvelope
