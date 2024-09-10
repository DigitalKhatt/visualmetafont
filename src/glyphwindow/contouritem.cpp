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



ContourItem::~ContourItem() {

}

ContourItem::ContourItem(QGraphicsItem* parent)
  : QGraphicsObject(parent)
{
  penWidth = 1;
  rotationAngle = 0;
  penColor = Qt::black;

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
    if (enable)
      this->path->setBrush(Qt::black);
    else
      this->path->setBrush(QBrush{});
  }
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

void ContourItem::setGlyph(Glyph* glyph)
{
  this->glyph = glyph;


  generateedge(glyph->getEdge());
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

  if (h) {

    if (this->labels != NULL) {
      delete this->labels;
    }

    labels = new QGraphicsItemGroup(this);

    if (this->path == NULL)
      newelement = true;

    if (newelement) {
      if (this->path != NULL)
        delete this->path;

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

    for (auto& [name, param] : glyph->params) {
      if (!param.isInControllePath && param.value.type() == QVariant::PointF) {
        QPointF point = param.value.toPointF();
        QGraphicsItem* pair;
        if (newelement) {
          pair = new PairItem(param, glyph, this->parampoints);
        }
        else {
          pair = this->parampoints->childItems()[childnb++];
        }
        pair->setPos(point.x(), point.y());
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
            QPainterPath subpath = mp_dump_solved_path(fill, numsubpath++, newelement);
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
  }


}
QPainterPath ContourItem::mp_dump_solved_path(mp_fill_object* fill, int numsubpath, bool newelement) {
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

  path.moveTo(h->x_coord, h->y_coord);
  p = h;
  do {

    if (p->data.types.right_type == mp_endpoint) {
      break;
    }

    if (newelement) {
      knotControlledItems[numsubpath][numpoint] = new KnotControlledItem(numsubpath, numstaticpoint, p, numpoint, glyph, this->path);

    }
    else {
      KnotControlledItem* item = knotControlledItems.value(numsubpath).value(numpoint);
      if (item != nullptr)
        item->setPositions(p);
    }

    q = p->next;


    path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

    numpoint++;

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
