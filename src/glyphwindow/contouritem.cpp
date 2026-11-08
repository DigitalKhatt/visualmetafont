/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include "contouritem.hpp"
#include "pairitem.hpp"
#include "knotitem.hpp"
#include <iostream>
#include "font.hpp"
#include "anchorlabels.hpp"
#include "sourcelinks.hpp"
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <algorithm>
#include <cmath>

namespace {
class AnchorTextItem : public QGraphicsSimpleTextItem {
 public:
  explicit AnchorTextItem(QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent) {}
  QPainterPath shape() const override {
    QPainterPath result; result.addRect(boundingRect()); return result;
  }
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
    painter->save();
    auto background = data(SourceLinks::HighlightRole).toBool() || isSelected()
        ? QColor(255,210,120) : QApplication::palette().base().color();
    background.setAlpha(220);
    painter->setBrush(background);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boundingRect().adjusted(-2, -1, 2, 1), 2, 2);
    painter->restore();
    QGraphicsSimpleTextItem::paint(painter, option, widget);
  }
};
class AnchorMarkerItem : public QGraphicsPathItem {
 public:
  explicit AnchorMarkerItem(QGraphicsItem* parent) : QGraphicsPathItem(parent) {}
 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
    const auto views=scene()->views();
    const auto transform=views.isEmpty() ? QTransform() : views.first()->viewportTransform();
    for (auto* item:scene()->items(event->scenePos(),Qt::IntersectsItemShape,Qt::DescendingOrder,transform)) {
      if (item!=this && item->flags().testFlag(QGraphicsItem::ItemIsMovable) &&
          !item->data(SourceLinks::KeysRole).toStringList().isEmpty()) {
        // An editable handle at the same location must retain its drag gesture.
        event->ignore(); return;
      }
    }
    QGraphicsPathItem::mousePressEvent(event);
  }
};
QVector<mp_fill_object*> pictureContours(mp_edge_object* picture) {
  QVector<mp_fill_object*> result;
  for (auto* object=picture ? picture->body : nullptr;object;object=object->next) {
    if (object->type!=mp_fill_code && object->type!=mp_stroked_code) continue;
    auto* contour=reinterpret_cast<mp_fill_object*>(object);
    if (contour->pre_script && QString::fromUtf8(contour->pre_script)=="begintext") continue;
    result.append(contour);
  }
  return result;
}
bool sameCurve(mp_gr_knot first,mp_gr_knot second,const QTransform& transform={}) {
  if (!first || !second) return first==second;
  auto* a=first; auto* b=second;
  do {
    const auto near=[](const QPointF& a,const QPointF& b) {
      return qAbs(a.x()-b.x())<0.000001 && qAbs(a.y()-b.y())<0.000001;
    };
    if (a->data.types.left_type!=b->data.types.left_type || a->data.types.right_type!=b->data.types.right_type ||
        !near(transform.map(QPointF(a->x_coord,a->y_coord)),QPointF(b->x_coord,b->y_coord)) ||
        !near(transform.map(QPointF(a->left_x,a->left_y)),QPointF(b->left_x,b->left_y)) ||
        !near(transform.map(QPointF(a->right_x,a->right_y)),QPointF(b->right_x,b->right_y))) return false;
    a=a->next; b=b->next;
  } while (a && b && a!=first && b!=second);
  return a==first && b==second;
}
int knotCount(mp_gr_knot first) {
  if (!first) return 0;
  int count=0; auto* p=first;
  do { ++count; p=p->next; } while (p && p!=first);
  return count;
}
QVector<int> sourcePointIds(const Glyph* glyph,int path) {
  return glyph->authoredPointIds(path);
}
}



ContourItem::~ContourItem() {

}

ContourItem::ContourItem(QGraphicsItem* parent)
  : QGraphicsObject(parent)
{
  penWidth = 1;
  rotationAngle = 0;
  penColor = Qt::black;
  enableFill = false;

  QTransform m;
  m.scale(1, -1);

  setTransform(m);

  path = NULL;
  parampoints = NULL;
  labels = NULL;

}

void ContourItem::setFillEnabled(bool enable)
{
  this->enableFill = enable;

  if (this->path) {
    if (enable && outlineVisible_)
      this->path->setBrush(Qt::black);
    else
      this->path->setBrush(QBrush{});
  }
}
void ContourItem::setOutlineVisible(bool visible) {
  outlineVisible_=visible;
  if (!path) return;
  QPen pen(Qt::black); pen.setCosmetic(true);
  path->setPen(visible ? pen : QPen(Qt::NoPen));
  setFillEnabled(enableFill);
}

void ContourItem::setFillGradient(const QColor& color1, const QColor& color2)
{
  fillColor1 = color1;
  fillColor2 = color2;
  update();
}

void ContourItem::setPenWidth(int width)
{
  penWidth = width;
  update();
}

void ContourItem::setPenColor(const QColor& color)
{
  penColor = color;
  update();
}

void ContourItem::setRotationAngle(int degrees)
{
  rotationAngle = degrees;
  update();
}

void ContourItem::setGlyph(Glyph* glyph, const QString& pictureName)
{
  if (this->glyph) disconnect(this->glyph->font, nullptr, this, nullptr);
  this->glyph = glyph;
  pictureName_=pictureName;
  connect(glyph->font, &Font::anchorsVisibilityChanged, this, &ContourItem::setAnchorsVisible);
  generateedge(glyph->getEdge());
}
void ContourItem::setPictureName(const QString& name) {
  if (pictureName_==name) return;
  pictureName_=name;
  generateedge(glyph->getEdge(),true);
}
void ContourItem::setFinalPositions(bool enabled) {
  if (finalPositions_==enabled) return;
  finalPositions_=enabled;
  generateedge(glyph->getEdge(),true);
}
bool ContourItem::hasEditablePaths() const {
  for (auto path:sourceBindings_) if (path>=0) return true;
  return false;
}

void ContourItem::setAnchorDisplayState(const QSet<QString>& hidden, int labelMode,
                                        const QString& selected) {
  hiddenAnchorKeys_ = hidden;
  anchorLabelMode_ = labelMode;
  selectedAnchorKey_ = selected;
  setAnchorsVisible(glyph && glyph->font->anchorsVisible());
}
void ContourItem::setAnchorsVisible(bool visible) {
  if (anchorLabels) anchorLabels->setVisible(visible);
  QSet<QString> allNames, visibleNames;
  for (auto& anchor : anchorLabelItems) {
    allNames.insert(anchor.text->text());
    const bool show = visible && !hiddenAnchorKeys_.contains(anchor.key);
    const bool text = show && (anchorLabelMode_ == 0 ||
                               (anchorLabelMode_ == 1 && anchor.key == selectedAnchorKey_));
    anchor.marker->setVisible(show);
    anchor.text->setVisible(text);
    anchor.leader->setVisible(false);
    if (show) visibleNames.insert(anchor.text->text());
  }
  if (parampoints) {
    // Keep shared controls while any visible anchor needs them. If an association
    // comes from a macro rather than a local declaration, keep it conservatively.
    const auto known = glyph->anchorParameterNamesFor(allNames);
    const auto needed = glyph->anchorParameterNamesFor(visibleNames);
    for (auto* item : parampoints->childItems()) {
      auto* point = static_cast<PairItem*>(item);
      point->setVisible(!point->isAnchorParameter() || (visible && !visibleNames.isEmpty() &&
                                                        (!known.contains(point->parameterName()) ||
                                                         needed.contains(point->parameterName()))));
    }
  }
  if (visible) layoutAnchorLabels();
}
void ContourItem::layoutAnchorLabels() {
  if (anchorLabelItems.isEmpty() || !anchorLabels) return;
  const auto views = scene() ? scene()->views() : QList<QGraphicsView*>{};
  const auto transform = views.isEmpty()
                             ? anchorLabels->sceneTransform()
                             : anchorLabels->deviceTransform(views.first()->viewportTransform());
  bool invertible;
  const auto inverse = transform.inverted(&invertible);
  if (!invertible) return;
  QVector<QPointF> points;
  QVector<QSizeF> sizes;
  QVector<int> shown;
  for (int i = 0; i < anchorLabelItems.size(); ++i) {
    const auto& label = anchorLabelItems[i];
    if (label.text->isVisible()) {
      points.append(transform.map(label.point));
      sizes.append(label.text->boundingRect().size());
      shown.append(i);
    } else
      label.leader->hide();
  }
  QVector<QRectF> occupied;
  if (!views.isEmpty()) {
    for (const auto& anchor : anchorLabelItems)
      if (anchor.marker->isVisible())
        occupied.append(anchor.marker->deviceTransform(views.first()->viewportTransform())
                            .mapRect(anchor.marker->boundingRect())
                            .adjusted(-2, -2, 2, 2));
    if (parampoints)
      for (auto* item : parampoints->childItems())
        if (item->isVisible())
          occupied.append(item->deviceTransform(views.first()->viewportTransform())
                              .mapRect(item->boundingRect())
                              .adjusted(-2, -2, 2, 2));
    for (auto* item : scene()->items())
      if (item->type() == KnotItem::Type && item->isVisible())
        occupied.append(item->deviceTransform(views.first()->viewportTransform())
                            .mapRect(item->boundingRect())
                            .adjusted(-2, -2, 2, 2));
  }
  const auto rectangles = placeAnchorLabels(points, sizes, occupied);
  for (int i = 0; i < shown.size(); ++i) {
    auto& label = anchorLabelItems[shown[i]];
    label.text->setPos(inverse.map(rectangles[i].topLeft()));
    const auto target = inverse.map(QPointF(rectangles[i].left() - 2, rectangles[i].center().y()));
    label.leader->setLine(QLineF(label.point, target));
    label.leader->setVisible(qAbs(rectangles[i].top() - (points[i].y() - sizes[i].height() / 2)) >
                             1);
  }
}

QRectF ContourItem::boundingRect() const
{
  QRectF ret = QRectF();// (-1000, -1000, 2000, 2000);
  if (path != NULL)
    ret = path->boundingRect();

  return ret;
}

void ContourItem::paint(QPainter* painter,
  const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  Q_UNUSED(painter);
  Q_UNUSED(option);
  Q_UNUSED(widget);


}
void ContourItem::generateedge(mp_edge_object* h, bool newelement) {
  prepareGeometryChange();

  struct Binding { int path=-1; const mp_fill_object* geometry=nullptr; QTransform transform; };
  QVector<Binding> bindings;
  QMap<int,int> bindingIds;
  QTransform displayTransform=QTransform::fromScale(1,-1);
  if (h) {
    const auto info=glyph->font->getMPGlyphInfo(h->charcode);
    pictureNames_.clear();
    for (const auto& name:glyph->pictureNames)
      if (info.controlledPictures.contains(name) && !pictureNames_.contains(name)) pictureNames_.append(name);
    for (const auto& name:info.controlledPictures.keys())
      if (!pictureNames_.contains(name)) pictureNames_.append(name);
    const auto* finalPicture=h;
    double scale=1;
    for (const auto& axis:glyph->font->project.internalParameters()) if (axis.metaPostIndex==100) {
      const auto value=axis.metaPostValue(glyph->axis(axis.key));
      if (value!=0) scale=value/100;
    }
    const QTransform finalTransform(scale,0,0,1,finalPicture->xpart,finalPicture->ypart);
    if (!pictureName_.isEmpty() && !info.controlledPictures.contains(pictureName_)) {
      pictureName_.clear(); newelement=true;
    }
    // Named pictures are captured before endchar applies the glyph
    // origin and scale. Keep their outlines and controls in that same final
    // frame when final-position editing is enabled.
    if (finalPositions_ && !pictureName_.isEmpty())
      displayTransform=finalTransform * displayTransform;
    if (!pictureName_.isEmpty()) h=info.controlledPictures.value(pictureName_);
    auto rendered=pictureContours(h);
    // Legacy final-glyph editing keeps its existing association. Captured
    // pictures use explicit source ownership and verified evaluated geometry.
    if (glyph->pictureNames.isEmpty() && pictureName_.isEmpty()) {
      for (int i=0;i<rendered.size();++i) bindingIds[i]=i;
    } else {
      struct Candidate { int path; const mp_fill_object* geometry; QString picture; int ordinal; };
      QVector<Candidate> candidates;
      for (auto snapshot=info.controlledPictureSources.cbegin();snapshot!=info.controlledPictureSources.cend();++snapshot) {
        QVector<int> ids;
        for (auto i=glyph->controlledPaths.cbegin();i!=glyph->controlledPaths.cend();++i) {
          const auto definition=glyph->controlledPathNames.value(i.key());
          const auto owner=definition.pictureName=="currentpicture" ? QString() : definition.pictureName;
          if (owner==snapshot.key() && (definition.commandName=="fill" || definition.commandName=="fillc" || definition.commandName=="draw")) ids.append(i.key());
        }
        const auto curves=pictureContours(snapshot.value());
        if (curves.size()!=ids.size()) continue;
        for (int i=0;i<ids.size();++i)
          if (sourcePointIds(glyph,ids[i]).size()==knotCount(curves[i]->path_p)) candidates.append({ids[i],curves[i],snapshot.key(),i});
      }
      for (int i=0;i<rendered.size();++i) {
        Binding binding;
        bool ambiguous=false;
        for (const auto& candidate:candidates) {
          if (!pictureName_.isEmpty() && candidate.picture!=pictureName_) continue;
          // In the final view bind only the original authored contour prefix.
          // A coincident construction curve is not evidence that a generated
          // final contour came from that picture.
          if (pictureName_.isEmpty() && (!candidate.picture.isEmpty() || candidate.ordinal!=i)) continue;
          const auto transform=pictureName_.isEmpty() ? finalTransform : QTransform();
          if (!sameCurve(candidate.geometry->path_p,rendered[i]->path_p,transform)) continue;
          if (binding.path>=0 && binding.path!=candidate.path) { ambiguous=true; break; }
          binding={candidate.path,candidate.geometry,transform};
        }
        if (ambiguous) binding={};
        bindings.append(binding); bindingIds[i]=binding.path;
      }
    }
    if (bindingIds!=sourceBindings_) newelement=true;
    sourceBindings_=bindingIds;
  }
  setTransform(displayTransform);

  if (h) {

    if (this->labels != NULL) {
      delete this->labels;
    }

    labels = new QGraphicsItemGroup(this);
    delete anchorLabels;
    anchorLabelItems.clear();
    // A plain parent preserves independent label/marker selection. A group
    // delegates child selection to the entire group of anchors.
    anchorLabels = new QGraphicsPathItem(this);
    anchorLabels->setZValue(10);
    QSet<QString> anchorNames;
    for (int i = 0; i < h->numAnchors; ++i) {
      const auto& anchor = h->anchors[i];
      const auto name = QString::fromUtf8(anchor.anchorName);
      anchorNames.insert(name);
      const QPointF point(anchor.x, anchor.y);
      auto* text = new AnchorTextItem(anchorLabels);
      text->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      text->setAcceptedMouseButtons(Qt::LeftButton);
      text->setFlag(QGraphicsItem::ItemIsSelectable);
      text->setData(SourceLinks::KeysRole,QStringList{SourceLinks::anchorKey(name,anchor.type)});
      text->setText(name);
      text->setBrush(QApplication::palette().text());
      text->setToolTip(tr("Computed anchor: %1").arg(name));
      text->setData(0, "anchorLabel");
      auto* leader = new QGraphicsLineItem(anchorLabels);
      QPen leaderPen(QColor(110, 110, 120, 170));
      leaderPen.setCosmetic(true);
      leader->setPen(leaderPen);
      leader->setZValue(-1);
      leader->setAcceptedMouseButtons(Qt::NoButton);
      leader->setData(0, "anchorLeader");
      leader->setData(SourceLinks::KeysRole,
                      QStringList{SourceLinks::anchorKey(name, anchor.type)});
      auto* marker = new AnchorMarkerItem(anchorLabels);
      QPainterPath cross;
      cross.moveTo(-3, 0); cross.lineTo(3, 0);
      cross.moveTo(0, -3); cross.lineTo(0, 3);
      marker->setPath(cross);
      marker->setPen(leaderPen);
      marker->setPos(point);
      marker->setFlag(QGraphicsItem::ItemIgnoresTransformations);
      marker->setAcceptedMouseButtons(Qt::LeftButton);
      marker->setFlag(QGraphicsItem::ItemIsSelectable);
      marker->setData(SourceLinks::KeysRole,QStringList{SourceLinks::anchorKey(name,anchor.type)});
      marker->setData(0, "anchorMarker");
      anchorLabelItems.append(
          {point, text, leader, SourceLinks::anchorKey(name, anchor.type), marker, anchor.type});
    }
    std::sort(anchorLabelItems.begin(), anchorLabelItems.end(), [](const auto& a, const auto& b) {
      if (a.point.y() != b.point.y()) return a.point.y() > b.point.y();
      if (a.point.x() != b.point.x()) return a.point.x() < b.point.x();
      return a.text->text() < b.text->text();
    });

    if (this->path == NULL)
      newelement = true;

    if (newelement) {
      if (this->path != NULL)
        delete this->path;
      knotControlledItems.clear();

      if (this->parampoints != NULL)
        delete this->parampoints;

      this->path = new QGraphicsPathItem(this);
      this->path->setZValue(-1);
      //this->path->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);

      QPen pen;
      pen.setWidth(1);
      pen.setCosmetic(true);
      this->path->setPen(pen);

      this->parampoints = new QGraphicsPathItem(this);


    }


    int childnb = 0;


    QPointF translate;
    auto edge = glyph->getEdge();
    if (edge != nullptr) {
      translate = QPointF(edge->xpart, edge->ypart);
    }

    const auto anchorParameters = glyph->anchorParameterNames();
    for (auto& [name, param] : glyph->params) {
      if (pictureName_.isEmpty() && !param.isInControllePath && param.value.type() == QVariant::PointF) {
        QPointF point = param.value.toPointF();
        QGraphicsItem* pair;
        if (newelement) {
          pair = new PairItem(param, glyph, this->parampoints);
        }
        else {
          pair = this->parampoints->childItems()[childnb++];
        }
        pair->setPos(point.x(), point.y());
        static_cast<PairItem*>(pair)->setAnchorParameter(anchorParameters.contains(name));
      }
    }

    int numsubpath = 0;

    mp_graphic_object* body = h->body;
    QPainterPath localpath;

    if (body) {

      do {
        switch (body->type)
        {
        case mp_stroked_code:
        case mp_fill_code: {
          mp_fill_object* fill = (mp_fill_object*)body;
          QString prescript = fill->pre_script;

          if (prescript == "begintext") {
            if (anchorNames.contains(QString::fromUtf8(fill->post_script))) break;
            QGraphicsSimpleTextItem* textitem = new QGraphicsSimpleTextItem(labels);
            textitem->setFlags(QGraphicsItem::ItemIgnoresTransformations);
            auto tx = fill->path_p->x_coord;
            auto ty = fill->path_p->y_coord;
            textitem->setPos(tx, ty);
            textitem->setText(fill->post_script);
            QGraphicsEllipseItem* dot = new   QGraphicsEllipseItem(labels);
            dot->setRect(tx - 5, ty - 5, 10, 10);
          }
          else {
            const auto binding=numsubpath<bindings.size() ? bindings[numsubpath] : Binding{};
            const bool legacy=glyph->pictureNames.isEmpty() && pictureName_.isEmpty();
            QPainterPath subpath = mp_dump_solved_path(fill,numsubpath++,newelement,
                legacy ? -2 : binding.path,binding.geometry,binding.transform);
            localpath.addPath(subpath);
          }

          break;
        }
        default:
          break;
        }

      } while (body = body->next);
    }

    localpath.setFillRule(Qt::FillRule::WindingFill);

    path->setPath(localpath);
    setOutlineVisible(outlineVisible_);
    setAnchorsVisible(glyph->font->anchorsVisible());
    layoutAnchorLabels();
  }


}
QPainterPath ContourItem::mp_dump_solved_path(mp_fill_object* fill, int numsubpath, bool newelement,
    int sourcePath,const mp_fill_object* sourceGeometry,const QTransform& sourceTransform) {
  mp_gr_knot p, q;
  QPainterPath path;

  struct Variable {
    QString name;
    int totalPoint;
  };

  struct CurrentPoint {
    int index;
    int point;
  };

  std::vector<Variable> customLinks;

  QString post_script = fill->post_script;
  if (!post_script.isEmpty()) {
    auto split = post_script.split(';', Qt::SkipEmptyParts);
    for (auto value : split) {
      auto temp = value.split('=', Qt::SkipEmptyParts);
      if (temp.length() == 2) {
        customLinks.push_back({ temp[0],temp[1].toInt() });
      }
    }
  }

  CurrentPoint currentPoint{};

  auto h = fill->path_p;

  if (h == nullptr) return path;

  int numpoint = 0;
  int numstaticpoint = 0;
  int laststaticpoint = 0;
  const auto ids=sourcePointIds(glyph,sourcePath);
  auto* sourceKnot=sourceGeometry ? sourceGeometry->path_p : nullptr;

  path.moveTo(h->x_coord, h->y_coord);
  p = h;
  do {

    if (p->data.types.right_type == mp_endpoint && sourcePath==-2) {
      break;
    }

    if (sourcePath==-1) {
      // Generated or unverified curves have no editable source controls.
    }
    else if (newelement) {
      const int pathId=sourcePath==-2 ? numsubpath : sourcePath;
      const int pointId=sourceGeometry && numpoint<ids.size() ? ids[numpoint] : numstaticpoint;
      auto* item=new KnotControlledItem(pathId,pointId,sourceKnot ? sourceKnot : p,numpoint,glyph,this->path);
      if (sourceGeometry) item->setTransform(sourceTransform);
      knotControlledItems[numsubpath][numpoint]=item;

    }
    else {
      KnotControlledItem* item = knotControlledItems.value(numsubpath).value(numpoint);
      if (item != nullptr)
        item->setPositions(sourceKnot ? sourceKnot : p);
      if (item && sourceGeometry) item->setTransform(sourceTransform);
    }

    if (p->data.types.right_type == mp_endpoint) break;

    q = p->next;


    path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

    numpoint++;
    if (sourceKnot) sourceKnot=sourceKnot->next;

    if (!customLinks.empty()) {
      if (glyph->controlledPaths.contains(numsubpath) && glyph->controlledPaths[numsubpath].contains(laststaticpoint)) {
        auto m_glyphknot = glyph->controlledPaths[numsubpath][laststaticpoint];
        if (currentPoint.index < customLinks.size() && m_glyphknot->rightValue.macrovalue == customLinks[currentPoint.index].name) {
          if (currentPoint.point <= customLinks[currentPoint.index].totalPoint) {
            numstaticpoint = -1;
            currentPoint.point++;
          }
          else {
            numstaticpoint = laststaticpoint + 1;
            laststaticpoint = numstaticpoint;
            currentPoint.index++;
            currentPoint.point = 0;
          }
        }
        else {
          numstaticpoint++;
          laststaticpoint++;
        }
      }
    }
    else {
      numstaticpoint++;
      laststaticpoint++;
    }

    p = q;

  } while (p != h);


  return path;

}
