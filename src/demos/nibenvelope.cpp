#include "nibenvelope.h"

static QPainterPath mp_dump_solved_path(mp_gr_knot h, std::vector<nibenvelope::CubicSegment>& cubicSegments) {
  mp_gr_knot p, q;
  QPainterPath path;
  // path.setFillRule(Qt::OddEvenFill);
  if (h == NULL) return path;

  path.moveTo(h->x_coord, h->y_coord);
  p = h;
  do {
    q = p->next;
    if (q != h || h->data.types.left_type != mp_endpoint) {
      path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);
      cubicSegments.push_back({QPointF(p->x_coord, p->y_coord), QPointF(p->right_x, p->right_y), QPointF(q->left_x, q->left_y), QPointF(q->x_coord, q->y_coord)});
    }
    p = q;
  } while (p != h);
  /*if (h->data.types.left_type != mp_endpoint)
    path.closeSubpath();*/

  return path;
}
static QPainterPath getPath(mp_edge_object* h, std::vector<std::vector<nibenvelope::CubicSegment>>& cubicSegments) {
  QPainterPath localpath;

  // localpath.setFillRule(Qt::WindingFill);

  if (h) {
    mp_graphic_object* body = h->body;

    if (body) {
      do {
        switch (body->type) {
          case mp_stroked_code:
          case mp_fill_code: {
            cubicSegments.push_back({});
            QPainterPath subpath = mp_dump_solved_path(((mp_fill_object*)body)->path_p, cubicSegments.back());
            localpath.addPath(subpath);

            break;
          }
          default:
            break;
        }

      } while (body = body->next);
    }
  }

  return localpath;
}
static double normalizeDeg(double a) {
  while (a <= -180.0) a += 360.0;
  while (a > 180.0) a -= 360.0;
  return a;
}

static double oppositeDeg(double a) {
  return normalizeDeg(a + 180.0);
}
static double clampTension(double t) {
  return std::clamp(t, 0.75, 10.0);
}

namespace nibenvelope {

KnotItem::KnotItem(
    int index,
    QGraphicsItem* parent)
    : QGraphicsEllipseItem(parent),
      m_index(index) {
  setRect(-5, -5, 10, 10);

  setBrush(Qt::red);

  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setFlag(ItemIsSelectable);
}

QVariant KnotItem::itemChange(GraphicsItemChange change, const QVariant& value) {
  if (change == ItemPositionHasChanged) {
    emit moved(m_index, value.toPointF());
  }
  return QGraphicsEllipseItem::itemChange(change, value);
}

static QPainterPath createRoundedRectPen(qreal w, qreal h, qreal r) {
  QPainterPath p;
  p.addRoundedRect(QRectF(-w / 2.0, -h / 2.0, w, h), r, r);
  return p;
}
PenStrokeEditor::PenStrokeEditor(Font* font, QWidget* parent) : QWidget(parent), m_model(new StrokeModel(this)), m_font(font) {
  m_scene = new QGraphicsScene(this);
  // m_scene->setSceneRect(0, 0, 1000, 700);

  m_view = new StrokeGraphicsView(m_scene, this);
  m_view->setRenderHint(QPainter::Antialiasing);
  m_view->setDragMode(QGraphicsView::RubberBandDrag);
  m_view->scale(1.0, -1.0);

  connect(static_cast<StrokeGraphicsView*>(m_view), &StrokeGraphicsView::shiftClicked, this, &PenStrokeEditor::addKnotAt);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_view);

  // --------------------------------------------------
  // 1. Fixed pen: rounded rectangle
  // --------------------------------------------------

  m_model.defaultPen = createRoundedRectPen(
      92.0,  // width
      20.0,  // height
      16.0   // corner radius
  );

  const qreal penAngle = 70.0;
  m_model.knots = {
      {
          .pos = QPointF(700, 350),
          .pen = m_model.defaultPen,
          .penAngle = penAngle,
      }};

  /*m_model.knots = {
      {
          .pos = QPointF(150, 350),
          .pen = m_model.defaultPen,
          .penAngle = penAngle,
      },
      {
          .pos = QPointF(400, 200),
          .pen = m_model.defaultPen,
          .penAngle = penAngle,
      },
      {
          .pos = QPointF(700, 350),
          .pen = m_model.defaultPen,
          .penAngle = penAngle,
      }};*/

  m_trajectoryItem = new QGraphicsPathItem();
  m_trajectoryItem->setPen(QPen(Qt::blue, 2));
  m_trajectoryItem->setZValue(0);
  m_scene->addItem(m_trajectoryItem);

  m_envelopeItem = new EnvelopeItem();
  QColor color(Qt::blue);
  color.setAlphaF(0.2);
  m_envelopeItem->setBrush(color);
  m_envelopeItem->setPen(QPen(Qt::blue, 2));
  m_envelopeItem->setZValue(0);
  m_scene->addItem(m_envelopeItem);

  connect(&m_model, &StrokeModel::changed, [this] { PenStrokeEditor::rebuild(true); });

  rebuildSceneItems(true);
}
void PenStrokeEditor::knotMoved(int index, QPointF p) {
  if (index < 0 || index >= m_model.knots.size())
    return;

  m_model.knots[index].pos = p;
  rebuild(false);
}

void PenStrokeEditor::penAngleChanged(int index, qreal angleDeg) {
  if (index < 0 || index >= m_model.knots.size())
    return;

  m_model.knots[index].penAngle = angleDeg;
  emit m_model.changed();
}
void PenStrokeEditor::rebuild(bool rebuildHandles) {
  auto glyphInfo = render(m_model);

  m_envelopeItem->setPos(0, 0);

  m_envelopeItem->setEnvelope(glyphInfo.currentPicture);

  QPainterPath trajectoryPath;
  m_segments = {};

  auto it = glyphInfo.controlledPictures.find("trajectoryPic");

  if (it != glyphInfo.controlledPictures.end()) {
    std::vector<std::vector<nibenvelope::CubicSegment>> cubicSegments;
    trajectoryPath = getPath(it.value(), cubicSegments);
    if (cubicSegments.size() > 0) {
      m_segments = cubicSegments[0];
    }
  }

  if (rebuildHandles)
    rebuildControlHandles();
  else
    updateControlHandles();

  trajectoryPath.setFillRule(Qt::WindingFill);

  m_trajectoryItem->setPath(trajectoryPath);

  // Draw pen at each knot, rotated by knot.penAngle

  for (int i = 0; i < m_model.knots.size(); ++i) {
    const Knot& k = m_model.knots[i];
    QTransform t;
    t.translate(k.pos.x(), k.pos.y());
    t.rotate(k.penAngle);
    m_penItems[i]->setPath(t.map(k.pen));
  }
}

void EnvelopeItem::setEnvelope(mp_edge_object* edge) {
  std::vector<std::vector<nibenvelope::CubicSegment>> cubicSegments;
  auto originalPath = getPath(edge, cubicSegments);
  setPath(originalPath);
}
static QString painterPathToMetaPost(const QPainterPath& path) {
  QString mp;

  bool first = true;

  int i = 0;

  QPointF movePoint;

  while (i < path.elementCount()) {
    QPainterPath::Element e = path.elementAt(i);

    if (e.type == QPainterPath::MoveToElement) {
      if (!first)
        mp += "\n;\n";

      mp += QString("(%1,%2)").arg(e.x).arg(e.y);

      movePoint = QPointF(e.x, e.y);

      first = false;
      ++i;
    } else if (e.type == QPainterPath::LineToElement) {
      mp += QString("\n-- (%1,%2)")
                .arg(e.x)
                .arg(e.y);

      ++i;
    } else if (e.type == QPainterPath::CurveToElement) {
      if (i + 2 >= path.elementCount())
        break;

      auto c1 = path.elementAt(i);
      auto c2 = path.elementAt(i + 1);
      auto p3 = path.elementAt(i + 2);

      i += 3;

      QPointF currentPoint(p3.x, p3.y);

      if (i == path.elementCount() && currentPoint == movePoint) {
        mp += QString(
                  "\n.. controls (%1,%2) and (%3,%4) .. cycle")
                  .arg(c1.x)
                  .arg(c1.y)
                  .arg(c2.x)
                  .arg(c2.y);
      } else {
        mp += QString(
                  "\n.. controls (%1,%2) and (%3,%4) .. (%5,%6)")
                  .arg(c1.x)
                  .arg(c1.y)
                  .arg(c2.x)
                  .arg(c2.y)
                  .arg(p3.x)
                  .arg(p3.y);
      }

    } else {
      ++i;
    }
  }

  mp += ";\n";

  return mp;
}
MPGlyphInfo PenStrokeEditor::render(const StrokeModel& model) {
  MPGlyphInfo glyphInfo;

  QString source;

  source = "beginchar(alternatechar,983040,0,-1,-1);\n";
  source += "save trajectory,result,penpath;\n";
  source += "path trajectory,result,penpath[];\n";
  if (model.knots.size() > 1) {
    source += "trajectory := ";

    QString penpaths;

    for (int i = 0; i < model.knots.size(); i++) {
      auto const& k = model.knots[i];

      if (i > 0) {
        const Knot& prev = m_model.knots[i - 1];
        if (prev.rightTension != 1.0 || k.leftTension != 1.0) {
          source += " .. ";
          if (qFuzzyCompare(prev.rightTension, k.leftTension)) {
            source += QString("tension %1 .. ").arg(prev.rightTension);
          } else {
            source += QString("tension %1 and %2 .. ").arg(prev.rightTension).arg(k.leftTension);
          }
        } else {
          source += " ... ";
        }
      }
      if (k.inDirDeg && i != 0)
        source += QString("{dir %1}").arg(*k.inDirDeg);

      source += QString("(%1,%2)").arg(k.pos.x()).arg(k.pos.y());

      if (k.outDirDeg && (i == 0 || k.outDirDeg != k.inDirDeg))
        source += QString("{dir %1}").arg(*k.outDirDeg);

      // source += QString("{dir %1}").arg(k.outDir * 180 / M_PI);

      QString penPath = QString("penpath[%1] := ").arg(i);

      penPath += painterPathToMetaPost(k.pen);
      penpaths += penPath;
    }

    source += ";\n";
    source += penpaths;

    source += "pen_stroke(\n";
    for (int i = 0; i < model.knots.size(); i++) {
      auto const& k = model.knots[i];
      QString nib = QString("nib(penpath[%1] rotated %2)(%1);\n").arg(i).arg(k.penAngle);
      source += nib;
    }

    source += ")(trajectory)(result);\n";
    source += "fill result;\n";
    source += "save trajectoryPic; picture trajectoryPic; trajectoryPic := image(draw trajectory withpen nullpen);\n";
  }

  source += "endchar;\n";

  qDebug() << source;

  m_font->pictureNames.append("trajectoryPic");

  try {
    m_font->executeMetaPost(source);
    m_font->pictureNames.clear();
  } catch (const std::exception& e) {
    m_font->pictureNames.clear();
  }

  glyphInfo = m_font->getMPGlyphInfo(983040);

  return glyphInfo;
}
void PenStrokeEditor::addKnotAt(QPointF scenePos) {
  int insertIndex = m_model.knots.size();

  for (QGraphicsItem* item : m_scene->selectedItems()) {
    auto* knotItem = dynamic_cast<KnotItem*>(item);
    if (knotItem) {
      insertIndex = knotItem->index() + 1;
      break;
    }
  }

  Knot k;
  k.pos = scenePos;
  k.leftTension = 1.0;
  k.rightTension = 1.0;
  k.penAngle = 70.0;
  k.pen = m_model.defaultPen;

  m_model.knots.insert(insertIndex, k);

  rebuildSceneItems(true);
  emit m_model.changed();
}
void PenStrokeEditor::keyPressEvent(QKeyEvent* ev) {
  if (ev->key() == Qt::Key_Delete ||
      ev->key() == Qt::Key_Backspace) {
    removeSelectedKnot();
    return;
  }

  QWidget::keyPressEvent(ev);
}
void PenStrokeEditor::removeSelectedKnot() {
  int removeIndex = -1;

  for (QGraphicsItem* item : m_scene->selectedItems()) {
    auto* knotItem = dynamic_cast<KnotItem*>(item);
    if (knotItem) {
      removeIndex = knotItem->index();
      break;
    }
  }

  if (removeIndex < 0)
    return;

  if (m_model.knots.size() <= 2)
    return;  // keep at least 2 knots

  m_model.knots.removeAt(removeIndex);

  rebuildSceneItems(true);
  emit m_model.changed();
}
void PenStrokeEditor::rebuildSceneItems(bool rebuildHandles) {
  for (auto* item : m_knotItems)
    m_scene->removeItem(item);

  for (auto* item : m_penItems)
    m_scene->removeItem(item);

  m_knotItems.clear();
  m_penItems.clear();

  for (int i = 0; i < m_model.knots.size(); ++i) {
    auto* penItem = new QGraphicsPathItem;
    penItem->setPen(QPen(Qt::darkGreen, 1));
    penItem->setBrush(QBrush(QColor(0, 180, 0, 60)));
    penItem->setZValue(1);

    m_penItems.append(penItem);
    m_scene->addItem(penItem);
  }

  for (int i = 0; i < m_model.knots.size(); ++i) {
    const Knot& k = m_model.knots[i];

    auto* knotItem = new KnotItem(i);
    knotItem->setPos(k.pos);
    knotItem->setZValue(20);
    knotItem->setFlag(QGraphicsItem::ItemIsSelectable);
    m_scene->addItem(knotItem);

    m_knotItems.append(knotItem);

    connect(knotItem, &KnotItem::moved, this, &PenStrokeEditor::knotMoved);

    auto* angleHandle = new PenAngleHandleItem(i, k.penAngle, knotItem);
    angleHandle->setZValue(10);

    connect(angleHandle, &PenAngleHandleItem::angleChanged, this, &PenStrokeEditor::penAngleChanged);
  }

  rebuild(rebuildHandles);
}
void PenStrokeEditor::clearControlHandles() {
  for (auto& h : m_controlHandleGraphics) {
    delete h.handle;
    delete h.line;
  }

  m_controlHandleGraphics.clear();
}
void PenStrokeEditor::rebuildControlHandles() {
  clearControlHandles();

  for (int i = 0; i < m_segments.size(); ++i) {
    const CubicSegment& s = m_segments[i];

    {
      auto* line = m_scene->addLine(QLineF(s.p0, s.c1), QPen(Qt::gray, 1, Qt::DashLine));

      auto* handle = new ControlHandleItem(i, HandleKind::OutHandle);
      handle->setPos(s.c1);
      m_scene->addItem(handle);

      connect(handle, &ControlHandleItem::moved, this, &PenStrokeEditor::controlHandleMoved);

      m_controlHandleGraphics.append({i, i, HandleKind::OutHandle, line, handle});
    }

    {
      auto* line = m_scene->addLine(QLineF(s.p1, s.c2), QPen(Qt::gray, 1, Qt::DashLine));

      auto* handle = new ControlHandleItem(i + 1, HandleKind::InHandle);
      handle->setPos(s.c2);
      m_scene->addItem(handle);

      connect(handle, &ControlHandleItem::moved, this, &PenStrokeEditor::controlHandleMoved);

      m_controlHandleGraphics.append({i, i + 1, HandleKind::InHandle, line, handle});
    }
  }
}
void PenStrokeEditor::updateControlHandles() {
  if (m_controlHandleGraphics.size() != m_segments.size() * 2) {
    rebuildControlHandles();
    return;
  }

  for (ControlHandleGraphics& g : m_controlHandleGraphics) {
    if (g.segmentIndex < 0 || g.segmentIndex >= m_segments.size())
      continue;

    const CubicSegment& s = m_segments[g.segmentIndex];

    g.handle->blockSignals(true);

    if (g.kind == HandleKind::OutHandle) {
      g.line->setLine(QLineF(s.p0, s.c1));

      // if (!g.handle->isUnderMouse())
      if (g.handle->pos() != s.c1)
        g.handle->setPos(s.c1);
    } else {
      g.line->setLine(QLineF(s.p1, s.c2));

      // if (!g.handle->isUnderMouse())
      if (g.handle->pos() != s.c2)
        g.handle->setPos(s.c2);
    }
    g.handle->blockSignals(false);
  }
}
void PenStrokeEditor::controlHandleMoved(int knotIndex, HandleKind kind, QPointF scenePos) {
  if (knotIndex < 0 || knotIndex >= m_model.knots.size())
    return;

  Knot& k = m_model.knots[knotIndex];

  const bool control = QApplication::keyboardModifiers() & Qt::ControlModifier;
  const bool alt = QApplication::keyboardModifiers() & Qt::AltModifier;

  if (control) {
    k.continuity = KnotContinuity::Corner;
  } else if (alt) {
    k.continuity = KnotContinuity::Smooth;
  }

  QPointF v;
  if (kind == HandleKind::OutHandle)
    v = scenePos - k.pos;
  else
    v = k.pos - scenePos;

  double len = std::hypot(v.x(), v.y());

  if (len < 1e-6)
    return;

  double dirDeg = normalizeDeg(qRadiansToDegrees(std::atan2(v.y(), v.x())));

  if (kind == HandleKind::OutHandle) {
    k.outDirDeg = dirDeg;
    if (k.continuity == KnotContinuity::Smooth)
      k.inDirDeg = dirDeg;
    // updateRightTensionFromHandle(knotIndex, len);

  } else {
    k.inDirDeg = dirDeg;
    if (k.continuity == KnotContinuity::Smooth)
      k.outDirDeg = dirDeg;
    //  updateLeftTensionFromHandle(knotIndex, len);
  }

  rebuild(false);
}
void PenStrokeEditor::updateRightTensionFromHandle(int knotIndex, double handleLength) {
  if (knotIndex < 0 || knotIndex + 1 >= m_model.knots.size())
    return;

  const QPointF p0 = m_model.knots[knotIndex].pos;
  const QPointF p1 = m_model.knots[knotIndex + 1].pos;

  double chord = QLineF(p0, p1).length();

  if (chord < 1e-6 || handleLength < 1e-6)
    return;

  double t = chord / (3.0 * handleLength);

  m_model.knots[knotIndex].rightTension = clampTension(t);
}
void PenStrokeEditor::updateLeftTensionFromHandle(int knotIndex, double handleLength) {
  if (knotIndex <= 0 || knotIndex >= m_model.knots.size())
    return;

  const QPointF p0 = m_model.knots[knotIndex - 1].pos;
  const QPointF p1 = m_model.knots[knotIndex].pos;

  double chord = QLineF(p0, p1).length();

  if (chord < 1e-6 || handleLength < 1e-6)
    return;

  double t = chord / (3.0 * handleLength);

  m_model.knots[knotIndex].leftTension = clampTension(t);
}
}  // namespace nibenvelope
