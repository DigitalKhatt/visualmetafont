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

#include "knotcontrolleditem.hpp"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsView>
#include "glyph.hpp"
#include "font.hpp"
#include <QMenu>
#include <iostream>
#include "glyphscene.hpp"
#include "qmessagebox.h"
#include <commands.h>


KnotControlledItem::KnotControlledItem(int numsubpath, int numpoint, mp_gr_knot knot, int numpointpath, Glyph* glyph, QGraphicsItem* parent)
  : QGraphicsObject(parent) {

  this->m_numsubpath = numsubpath;
  this->m_numpoint = numpoint;
  this->m_numpointpath = numpointpath;
  this->m_knot = knot;
  this->m_glyph = glyph;
  this->m_isInControlledPath = m_glyph->controlledPaths.contains(numsubpath) && m_glyph->controlledPaths[numsubpath].contains(numpoint);
  left = NULL;
  right = NULL;

  QPen penline(Qt::gray);
  penline.setWidth(0);
  penline.setCosmetic(true);

  setFlags(ItemIsMovable | ItemHasNoContents);


  QString rightkey = QString::number(numsubpath) + "_" + QString::number(numpoint);
  QString leftkey = QString::number(numsubpath) + "_" + QString::number(numpoint);

  setFiltersChildEvents(true);

  incurve = new KnotItem(KnotItem::InCurve, this);


  if (m_isInControlledPath) {
    m_glyphknot = m_glyph->controlledPaths[numsubpath][numpoint];
    if (m_glyphknot->expr->containsConstant() || m_glyphknot->expr->containsParam()) {
      incurve->setFlags(ItemIsMovable | ItemIsSelectable);
    }

    leftline = new QGraphicsLineItem(this);
    leftline->setPen(penline);
    leftline->setZValue(-1);
    left = new KnotItem(KnotItem::LeftControl, this);

    bool isControlEqualBefore = m_glyphknot->leftValue.type == Glyph::mpgui_explicit && m_glyphknot->leftValue.isEqualBefore;
    if (isControlEqualBefore) {
      left->setVisible(false);
    }
    else if (m_glyphknot->leftValue.isControllable()) {
      left->setFlags(ItemIsMovable | ItemIsSelectable);
    }

    rightline = new QGraphicsLineItem(this);
    rightline->setPen(penline);
    rightline->setZValue(-1);
    right = new KnotItem(KnotItem::RightControl, this);
    if (m_glyphknot->rightValue.isControllable()) {
      right->setFlags(ItemIsMovable | ItemIsSelectable);
    }
    //incurve->setFlags(ItemIsMovable | ItemIsSelectable);
    //left->setFlags(ItemIsMovable | ItemIsSelectable);
    //right->setFlags(ItemIsMovable | ItemIsSelectable);
  }
  else {
    m_glyphknot = nullptr;
  }


  setPositions(knot);

}

KnotControlledItem::~KnotControlledItem() {

}

void KnotControlledItem::setPositions(mp_gr_knot knot) {
  this->m_knot = knot;

  if (incurve) {
    incurve->setPos(m_knot->x_coord, m_knot->y_coord);
  }
  if (left) {
    //left->setPos(m_knot->left_x - m_knot->x_coord, m_knot->left_y - m_knot->y_coord);
    left->setPos(m_knot->left_x, m_knot->left_y);
    leftline->setLine(m_knot->x_coord, m_knot->y_coord, left->pos().x(), left->pos().y());
  }

  if (right) {
    //right->setPos(m_knot->right_x - m_knot->x_coord, m_knot->right_y - m_knot->y_coord);
    right->setPos(m_knot->right_x, m_knot->right_y);
    rightline->setLine(m_knot->x_coord, m_knot->y_coord, right->pos().x(), right->pos().y());
  }
  

}
QRectF KnotControlledItem::boundingRect() const
{
  return QRectF();
}
void KnotControlledItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{


}
bool KnotControlledItem::moveMinMaxDeltas(QGraphicsItem* watched, QGraphicsSceneMouseEvent* event) {

  bool alt = event->modifiers() & Qt::AltModifier;

  if (!alt) {
    return false;
  }

  auto scene = (GlyphScene*)watched->scene();

  auto F1 = scene->currentPressdKeys.contains(Qt::Key_F1);
  auto F2 = scene->currentPressdKeys.contains(Qt::Key_F2);
  auto F3 = scene->currentPressdKeys.contains(Qt::Key_F3);

  if (!F1 && !F2 && !F3) {
    return false;
  }

  QPointF diff = event->pos() - event->lastPos();

  Qt::KeyboardModifiers modifiers = event->modifiers();

  auto paramName = F1 ? "defdeltas" : F2 ? "leftdeltas" : "rightdeltas";
  auto index = watched == incurve ? 0 : watched == left ? 1 : 2;

  auto adjustPoint = [this, paramName](int index, QPointF diff) {
    auto propName = QString("%1[%2][%3][%4]").arg(paramName).arg(m_numsubpath).arg(m_numpointpath).arg(index);
    QByteArray ba = propName.toLocal8Bit();
    QVariant val = m_glyph->property(ba.data());
    if (!val.isValid()) {
      auto point = QPointF(0, 0);
      m_glyph->setParameter(propName, new PairPathPointExp(point), false);
      auto& param = m_glyph->params[propName];
      param.isInControllePath = true;
      val = point;
    }
    if (QVariant::PointF == val.type()) {
      QPointF point = val.toPointF();
      point.setX(point.x() + diff.x());
      point.setY(point.y() + diff.y());
      m_glyph->setProperty(ba.data(), point);
    }
  };
  if (index == 0) {
    adjustPoint(0, diff);
    adjustPoint(1, diff);
    adjustPoint(2, diff);
  }
  else {
    //bool shift = modifiers & Qt::ShiftModifier;
    bool ctrl = modifiers & Qt::ControlModifier;
    //preserve slope
    if (!ctrl) {
      QGraphicsItem* other = index == 1 ? right : left;
      QPointF leftPoint{ m_knot->left_x,m_knot->left_y };
      QPointF rightPoint{ m_knot->right_x,m_knot->right_y };
      QPointF inCurvePoint{ m_knot->x_coord,m_knot->y_coord };
      //currentPoint = currentPoint + diff;
      QPointF currControl = index == 1 ? leftPoint : rightPoint;
      currControl = currControl + diff;
      QPointF otherControl = index == 1 ? rightPoint : leftPoint;
      auto a = std::atan2(inCurvePoint.y() - currControl.y(), inCurvePoint.x() - currControl.x());
      auto d = QLineF(inCurvePoint, otherControl).length();
      QPointF newPoint(inCurvePoint.x() + d * std::cos(a), inCurvePoint.y() + d * std::sin(a));
      QPointF otherDiff = newPoint - otherControl;
      auto otherIndex = index == 1 ? 2 : 1;
      adjustPoint(otherIndex, otherDiff);
    }
    adjustPoint(index, diff);
  }

  return true;

}
bool KnotControlledItem::updateControlledPoint(MFExpr* expr, int position, QPointF diff) {
  bool ret = false;
  if (expr->isConstant(position)) {
    expr->setConstantValue(position, expr->constantValue(position).toPoint() + diff.toPoint());
    ret = true;
  }
  else {
    auto parmName = expr->paramName(position);
    if (m_glyph->dependents.contains(parmName)) {
      auto param = m_glyph->dependents.value(parmName);
      parmName = param->name;
    }
    if (!parmName.isEmpty()) {
      auto latinName = parmName.toLatin1();
      auto& param = m_glyph->params[parmName];
      if (param.expr->isConstant(position)) {
        QPointF pair = param.expr->constantValue(position).toPoint() + diff.toPoint();
        param.expr->setConstantValue(position, pair);
        m_glyph->setProperty(parmName.toLatin1(), pair);
        ret = true;
      }
      /*
      QVariant val = m_glyph->property(latinName);
      if (QVariant::PointF == val.type()) {
        QPointF point = val.toPointF();
        m_glyph->setProperty(latinName, point + diff);
        ret = true;
      }*/
    }
  }
  return ret;
}
bool KnotControlledItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event) {
  if (!isEnabled()) {
    return false;
  }
  if (event->type() == QEvent::GraphicsSceneMouseMove) {

    auto scene = (GlyphScene*)watched->scene();

    QGraphicsSceneMouseEvent* me = static_cast<QGraphicsSceneMouseEvent*>(event);


    Qt::KeyboardModifiers modifiers = me->modifiers();

    bool shift = modifiers & Qt::ShiftModifier;
    bool ctrl = modifiers & Qt::ControlModifier;

    QPointF diff = me->pos() - me->lastPos();

    if (moveMinMaxDeltas(watched, me)) {
      return true;
    }

    if (watched == incurve) {

      int expIndex = scene->getControlledPosition(me);

      updateControlledPoint(m_glyphknot->expr.get(), expIndex, diff);

      if (m_glyphknot->leftValue.type == Glyph::mpgui_explicit) {

        //if (m_glyphknot->leftValue.isEqualBefore) {
        //  Glyph::Knot* knot = m_glyph->controlledPaths[m_numsubpath][m_numpoint - 1];
          //knot->rightValue.dirExpr = m_glyphknot->leftValue.dirExpr->clone();
        //  updateControlledPoint(knot->rightValue.dirExpr.get(), expIndex, diff);
        //}
        //else {
        updateControlledPoint(m_glyphknot->leftValue.dirExpr.get(), expIndex, diff);
        //}
      }

      if (m_glyphknot->rightValue.type == Glyph::mpgui_explicit) {
        updateControlledPoint(m_glyphknot->rightValue.dirExpr.get(), expIndex, diff);
      }

      return true;
    }
    else if (watched == left || watched == right) {
      return updateControlPoint(me, watched == left, diff, shift, ctrl);
    }
  }
  else if (event->type() == QEvent::GraphicsSceneContextMenu) {
    QGraphicsSceneContextMenuEvent* me = static_cast<QGraphicsSceneContextMenuEvent*>(event);

    QMenu menu;
    QMenu* lksubMenu = nullptr;
    QMenu* rksubMenu = nullptr;
    QMenu* axes = nullptr;
    if (watched == incurve) {

      //QAction * scaleAct = new QAction("&Scale", this);	
      //connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

      menu.addAction("Left");
      menu.addAction("Right");
      menu.addAction("Up");
      menu.addAction("Down");
      menu.addAction("dir");
      menu.addAction("dir-dir");
      //menu.addAction("Left Kashida");      
      //menu.addAction("Right Kashida");

      /** add submenu */
      /*lksubMenu = menu.addMenu(tr("Left Kashida"));
      lksubMenu->addAction("0");
      lksubMenu->addAction("1");
      lksubMenu->addAction("2");*/

      /*rksubMenu = menu.addMenu(tr("Right Kashida"));
      rksubMenu->addAction("0");
      rksubMenu->addAction("1");
      rksubMenu->addAction("2");*/

      /*axes = menu.addMenu(tr("Axes"));
      axes->addAction("Add Left");
      axes->addAction("Add Right");
      axes->addAction("Remove Blend");*/
      menu.addAction("Add Left 1");
      menu.addAction("Add Right 1");
      menu.addAction("Add Left 2");
      menu.addAction("Add Right 2");
      menu.addAction("Remove Blend");
    }
    else if (watched == left || watched == right) {
      QMenu* axes = menu.addMenu(tr("Axes"));
      axes->addAction("Add Left Tension");
      axes->addAction("Add Left Dir");
      axes->addAction("Add Right Tension");
      axes->addAction("Add Right Dir");
      axes->addAction("Remove Blend From Tension");
      axes->addAction("Remove Blend From Dir");
    }

    QAction* a = menu.exec(me->screenPos());
    if (a != NULL) {
      if (a->parentWidget() == lksubMenu) {
        int knots = a->text().toInt();
        addKashida(true, knots);
      }
      else if (a->parentWidget() == rksubMenu) {
        int knots = a->text().toInt();
        addKashida(false, knots);
      }
      else if (a->text() == "Add Left 1") {
        AddRemoveBlend(true, false, 0);
      }
      else if (a->text() == "Add Right 1") {
        AddRemoveBlend(false, false, 0);
      }
      else if (a->text() == "Add Left 2") {
        AddRemoveBlend(true, false, 3);
      }
      else if (a->text() == "Add Right 2") {
        AddRemoveBlend(false, false, 3);
      }
      else if (a->text() == "Remove Blend") {
        AddRemoveBlend(false, true, 0);
      }
      else if (a->text() == "Add Left Dir") {
        AddRemoveBlend(true, false, 1);
      }
      else if (a->text() == "Add Right Dir") {
        AddRemoveBlend(false, false, 1);
      }
      else if (a->text() == "Add Left Tension") {
        AddRemoveBlend(true, false, 2);
      }
      else if (a->text() == "Add Right Tension") {
        AddRemoveBlend(false, false, 2);
      }
      else if (a->text() == "Remove Blend From Dir") {
        AddRemoveBlend(false, true, 1);
      }
      else if (a->text() == "Remove Blend From Tension") {
        AddRemoveBlend(false, true, 2);
      }
      else if (a->text() == "Left") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        m_glyphknot->leftValue.dirExpr = std::make_unique<VarMFExpr>("left", false);
        m_glyphknot->rightValue.type = Glyph::mpgui_open;
        m_glyph->setWidth(m_glyph->width());
      }
      else if (a->text() == "Right") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        m_glyphknot->leftValue.dirExpr = std::make_unique<VarMFExpr>("right", false);
        m_glyphknot->rightValue.type = Glyph::mpgui_open;
        m_glyph->setWidth(m_glyph->width());
      }
      else if (a->text() == "Up") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        m_glyphknot->leftValue.dirExpr = std::make_unique<VarMFExpr>("up", false);
        m_glyphknot->rightValue.type = Glyph::mpgui_open;
        m_glyph->setWidth(m_glyph->width());
      }
      else if (a->text() == "Down") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        m_glyphknot->leftValue.dirExpr = std::make_unique<VarMFExpr>("down", false);
        m_glyphknot->rightValue.type = Glyph::mpgui_open;
        m_glyph->setWidth(m_glyph->width());
      }
      else if (a->text() == "dir") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        auto numExp = new LitPathNumericExp(45);
        m_glyphknot->leftValue.dirExpr = std::make_unique<DirPathPointExp>(numExp);


        m_glyphknot->rightValue.type = Glyph::mpgui_open;
        m_glyph->setWidth(m_glyph->width());
      }
      else if (a->text() == "dir-dir") {
        m_glyphknot->leftValue.type = Glyph::mpgui_given;
        auto numExp = new LitPathNumericExp(45);
        m_glyphknot->leftValue.dirExpr = std::make_unique<DirPathPointExp>(numExp);

        m_glyphknot->rightValue.type = Glyph::mpgui_given;
        numExp = new LitPathNumericExp(-45);
        m_glyphknot->rightValue.dirExpr = std::make_unique<DirPathPointExp>(numExp);

        m_glyph->setWidth(m_glyph->width());
      }
    }

    return true;
  }

  return QGraphicsItem::sceneEventFilter(watched, event);
}

bool KnotControlledItem::AddRemoveBlend(bool left, bool remove, int type) {

  auto scene = (GlyphScene*)this->scene();

  auto selectItems = scene->selectedItems();

  auto oldSource = m_glyph->source();

  bool change = false;

  for (auto item : selectItems) {
    auto knotItem = dynamic_cast<KnotItem*>(item);
    if (!knotItem) continue;

    auto parentItem = (KnotControlledItem*)knotItem->parentItem();

    auto knot = parentItem->m_glyphknot;

    if (knotItem == parentItem->incurve) {
      if (remove) {
        auto expr = dynamic_cast<FunctionMFExp*>(parentItem->m_glyphknot->expr.get());
        if (expr) {
          auto expr2 = expr->getFirst()->clone();
          parentItem->m_glyphknot->expr = std::unique_ptr<MFExpr>(expr2.release());
          change = true;
        }
      }
      else {
        QString bendFuncName = left ? (type == 0 ? "bldmvi" : "bldmv") : (type == 0 ? "brdmvi" : "brdmv");

        if (type == 0) {
          parentItem->m_glyphknot->expr = std::make_unique<FunctionMFExp>(bendFuncName,
            parentItem->m_glyphknot->expr.release(),
            new PairPathPointExp(QPointF{ 0,0 })
          );
        }
        else {

          std::vector<MFExpr*> params{ parentItem->m_glyphknot->expr.release(),
            new PairPathPointExp(QPointF{ 0,0 }),
            new PairPathPointExp(QPointF{ 0,0 }) };
          parentItem->m_glyphknot->expr = std::make_unique<FunctionMFExp>(bendFuncName, params);
        }

        change = true;
      }
    }
    else if (knotItem == parentItem->left || knotItem == parentItem->right) {

      auto& knotentryexit = knotItem == parentItem->left ? parentItem->m_glyphknot->leftValue : parentItem->m_glyphknot->rightValue;

      if (remove) {
        if (type == 0 || type == 1) {
          auto expr = dynamic_cast<FunctionMFExp*>(knotentryexit.dirExpr.get());
          if (expr) {
            auto expr2 = expr->getFirst()->clone();
            knotentryexit.dirExpr = std::unique_ptr<MFExpr>(expr2.release());
            change = true;
          }
          else {
            auto expdir = dynamic_cast<DirPathPointExp*>(knotentryexit.dirExpr.get());
            if (expdir) {
              auto expr2 = expdir->getVal();
              expr = dynamic_cast<FunctionMFExp*>(expr2);
              if (expr) {
                expdir->setVal(expr->getFirst()->clone().release());
                change = true;
              }

            }
          }
        }
        if (type == 0 || type == 2) {
          auto expr = dynamic_cast<FunctionMFExp*>(knotentryexit.tensionExpr.get());
          if (expr) {
            auto expr2 = expr->getFirst()->clone();
            knotentryexit.tensionExpr = std::unique_ptr<MFExpr>(expr2.release());
            change = true;
          }
        }
      }
      else {
        QString bendFuncName = left ? "bldmvi" : "brdmvi";

        if (type == 0 || type == 1) {
          if (knotentryexit.dirExpr) {
            auto dirExpr = dynamic_cast<DirPathPointExp*>(knotentryexit.dirExpr.get());
            if (dirExpr) {
              auto expr2 = dirExpr->getVal()->clone().release();
              dirExpr->setVal(new FunctionMFExp(bendFuncName,
                expr2,
                new LitPathNumericExp(0)
              ));
              change = true;
            }
            else {
              knotentryexit.dirExpr = std::make_unique<FunctionMFExp>(bendFuncName,
                knotentryexit.dirExpr.release(),
                new PairPathPointExp(QPointF{ 0,0 })
              );
              change = true;
            }
          }
          else {
            knotentryexit.dirExpr = std::make_unique<DirPathPointExp>(new FunctionMFExp(bendFuncName, new LitPathNumericExp(25), new LitPathNumericExp(0)));
            knotentryexit.type = Glyph::mpgui_given;
            change = true;
          }
        }
        if (type == 0 || type == 2) {
          if (knotentryexit.tensionExpr) {

            knotentryexit.tensionExpr = std::make_unique<FunctionMFExp>(bendFuncName,
              knotentryexit.tensionExpr.release(),
              new LitPathNumericExp(0)
            );
            change = true;
          }
        }
      }
    }

  }

  if (change) {
    m_glyph->isDirty = true;

    auto newSource = m_glyph->source();

    GlyphSourceChangeCommand* command = new GlyphSourceChangeCommand(m_glyph, "Source Changed", oldSource, newSource, true);
    m_glyph->undoStack()->push(command);


  }

  return true;

}
bool KnotControlledItem::addKashida(bool left, int nbKnots) {

  auto scene = (GlyphScene*)this->scene();

  auto selectItems = scene->selectedItems();

  if (selectItems.size() != 2) {
    QMessageBox msgBox;
    msgBox.setText("Two incurve points should be selected.");
    return msgBox.exec();
  }

  auto topPoint = dynamic_cast<KnotItem*>(selectItems.at(0));
  auto bottomPoint = dynamic_cast<KnotItem*>(selectItems.at(1));

  if (!topPoint || !bottomPoint || topPoint->knottype != KnotItem::InCurve || bottomPoint->knottype != KnotItem::InCurve) {
    QMessageBox msgBox;
    msgBox.setText("Two incurve points should be selected.");
    return msgBox.exec();
  }

  auto parentTop = (KnotControlledItem*)topPoint->parentItem();
  auto parentBottom = (KnotControlledItem*)bottomPoint->parentItem();

  if (parentTop->m_numsubpath != parentBottom->m_numsubpath || !parentTop->m_isInControlledPath || !parentBottom->m_isInControlledPath) {
    QMessageBox msgBox;
    msgBox.setText("The two points should belong to the same path.");
    return msgBox.exec();
  }

  if (topPoint->scenePos().y() * -1 < bottomPoint->scenePos().y() * -1) {
    auto temp = topPoint;
    auto tempParent = parentTop;
    topPoint = bottomPoint;
    bottomPoint = temp;
    parentTop = parentBottom;
    parentBottom = tempParent;
  }

  //TODO : Detect orientation. For now we suppose clockwise orientation

  KnotItem* first, * second;


  if (left) {
    first = bottomPoint;
    second = topPoint;
  }
  else {
    first = topPoint;
    second = bottomPoint;
  }

  KnotControlledItem* parentFirst, * parentSecond;

  parentFirst = (KnotControlledItem*)first->parentItem();
  parentSecond = (KnotControlledItem*)second->parentItem();

  auto firstKnot = m_glyph->controlledPaths.value(parentFirst->m_numsubpath).value(parentFirst->m_numpoint);
  auto secondKnot = m_glyph->controlledPaths.value(parentSecond->m_numsubpath).value(parentSecond->m_numpoint);

  auto bottomKnot = left ? firstKnot : secondKnot;
  auto topKnot = left ? secondKnot : firstKnot;

  auto controlledPath = m_glyph->controlledPaths[parentFirst->m_numsubpath];
  auto lastKey = controlledPath.lastKey();

  auto startPoint = parentFirst->m_numpoint < parentSecond->m_numpoint ? controlledPath.firstKey() : parentSecond->m_numpoint;
  auto firstPoint = parentFirst->m_numpoint;
  auto secondPoint = parentFirst->m_numpoint < parentSecond->m_numpoint ? parentSecond->m_numpoint : lastKey + parentSecond->m_numpoint;


  //firstKnot->expr->constantValue

  m_glyph->blockSignals(true);
  auto oldSource = m_glyph->source();


  QMap<int, Glyph::Knot*>  newcontrolledPaths;
  int keyoffset = 0;
  bool secondPointDone = false;
  for (auto i = controlledPath.begin(); i != controlledPath.end(); ++i) {
    if (i.key() < startPoint) {
      delete i.value();
      continue;
    }

    if (i.key() == startPoint && parentFirst->m_numpoint >= parentSecond->m_numpoint) {
      continue;
    }

    if (i.key() <= firstPoint) {
      newcontrolledPaths.insert(i.key(), controlledPath.value(i.key()));
      continue;
    }

    if (i.key() < secondPoint) {
      if (i.key() < lastKey) {
        delete i.value();
        continue;
      }
    }

    if (!secondPointDone && (i.key() == secondPoint || i.key() == lastKey)) {

      auto edge = m_glyph->getEdge();
      QPointF matrix;
      if (edge) {
        matrix = { edge->xpart,edge->ypart };
      }

      double nuqta = m_glyph->font->getNumericVariable("nuqta");

      QString vertRatioVarName = left ? "leftVerticalRatio" : "rightVerticalRatio";
      auto dirVar1Value = left ? 25 : 155;
      auto vertRatioValue = left ? 6 : 6;
      auto minValue = left ? QPointF{ -5, -5 } : QPointF{ 5, -5 };
      // TODO : get value from font
      QPointF joinvector{ 16,88 };
      auto firstPointRatio = 1.0 / 3;
      auto secondPointRatio = 2.0 / 3;
      auto maxLength = (1 + 6);

      m_glyph->setParameter(QString(vertRatioVarName), new LitPathNumericExp(vertRatioValue), false, true);


      int maxTatweel = 20;
      int minTatweel = -1;
      double verticalRatio = left ? 12 : 6;
      double maxDeltaWidth = nuqta * (maxTatweel - minTatweel);
      auto maxDelta = left ? QPointF{ -maxDeltaWidth,-maxDeltaWidth / verticalRatio } : QPointF{ maxDeltaWidth,-maxDeltaWidth / verticalRatio };
      QPointF origin{ parentBottom->m_knot->x_coord  ,parentBottom->m_knot->y_coord };
      origin = origin - matrix;
      QString bendFuncName = left ? "bldmvi" : "brdmvi";

      /*
      firstKnot->leftValue.type = Glyph::mpgui_given;

      firstKnot->leftValue.dirExpr = std::make_unique<ScalarMultiMFExp>(
        new DirPathPointExp(
          new FunctionMFExp(bendFuncName,
            new LitPathNumericExp(dirVar1Value),
            new LitPathNumericExp(0))), MFExprOperator::MINUS
        );*/

      firstKnot->expr = std::make_unique<FunctionMFExp>(bendFuncName,
        firstKnot->expr.release(),
        new PairPathPointExp(QPointF{ 0,0 })
      );

      if (nbKnots == 0) {
        firstKnot->rightValue = {};
        firstKnot->rightValue.macrovalue = left ? "" : "link";
        firstKnot->rightValue.jointtype = left ? Glyph::path_join_tension : Glyph::path_join_macro;
      }
      else {
        firstKnot->rightValue = {};
        firstKnot->rightValue.jointtype = Glyph::path_join_tension;

        auto z_r1 = new Glyph::Knot();
        z_r1->leftValue = {};
        z_r1->leftValue.jointtype = Glyph::path_join_tension;

        if (nbKnots == 1) {
          z_r1->rightValue = {};
          z_r1->rightValue.macrovalue = left ? "" : "link";
          z_r1->rightValue.jointtype = left ? Glyph::path_join_tension : Glyph::path_join_macro;
        }
        else {
          z_r1->rightValue = {};
          z_r1->rightValue.jointtype = Glyph::path_join_tension;
        }

        z_r1->expr = std::make_unique<FunctionMFExp>(bendFuncName,
          new PairPathPointExp(origin + firstPointRatio * minValue + (left ? QPointF() : joinvector)),
          //new LitPointPathPointExp({ firstPointRatio * maxDelta.x(),firstPointRatio * maxDelta.y() })
          new PairPathPointExp(QPointF{ 0,0 })
        );

        keyoffset++;

        newcontrolledPaths.insert(i.key() + keyoffset, z_r1);

        if (nbKnots > 1) {

          auto z_r2 = new Glyph::Knot();
          z_r2->leftValue = {};
          z_r2->leftValue.type = Glyph::mpgui_open;
          z_r2->leftValue.jointtype = Glyph::path_join_tension;

          z_r2->rightValue = {};
          z_r2->rightValue.type = Glyph::mpgui_curl;
          z_r2->rightValue.macrovalue = left ? "" : "link";
          z_r2->rightValue.jointtype = left ? Glyph::path_join_tension : Glyph::path_join_macro;

          z_r2->expr = std::make_unique<FunctionMFExp>(bendFuncName,
            new PairPathPointExp(origin + secondPointRatio * minValue + (left ? QPointF() : joinvector)),
            //new LitPointPathPointExp({ secondPointRatio * maxDelta.x(),secondPointRatio * maxDelta.y() })
            new PairPathPointExp(QPointF{ 0,0 })
          );

          keyoffset++;

          newcontrolledPaths.insert(i.key() + keyoffset, z_r2);

        }
      }

      auto z_r3 = new Glyph::Knot();
      z_r3->expr = std::make_unique<FunctionMFExp>(bendFuncName,
        new PairPathPointExp(origin + minValue),
        new BinOpMFExp(new VarMFExpr("nuqta", false), MFExprOperator::TIMES,
          new PairPathPointExp(new LitPathNumericExp(left ? -maxLength : maxLength), new BinOpMFExp(new LitPathNumericExp(-maxLength), MFExprOperator::OVER, new VarMFExpr(vertRatioVarName, true))))
      );

      if (!left) {
        z_r3->leftValue = {};
        z_r3->leftValue.macrovalue = "link";
        z_r3->leftValue.jointtype = Glyph::path_join_macro;

        z_r3->rightValue = {};
        z_r3->rightValue.jointtype = Glyph::path_join_tension;
      }
      else {
        z_r3->rightValue = {};
        z_r3->rightValue.macrovalue = "link";
        z_r3->rightValue.jointtype = Glyph::path_join_macro;

        z_r3->leftValue = {};
        z_r3->leftValue.jointtype = Glyph::path_join_tension;
      }

      keyoffset += 3;

      newcontrolledPaths.insert(i.key() + keyoffset, z_r3);

      if (nbKnots > 1) {

        auto z_r2_bottom = new Glyph::Knot();
        z_r2_bottom->leftValue = {};
        z_r2_bottom->leftValue.jointtype = Glyph::path_join_tension;

        z_r2_bottom->rightValue = {};
        z_r2_bottom->rightValue.jointtype = Glyph::path_join_tension;

        z_r2_bottom->expr = std::make_unique<FunctionMFExp>(bendFuncName,
          new PairPathPointExp(origin + secondPointRatio * minValue + (left ? joinvector : QPointF())),
          //new LitPointPathPointExp({ secondPointRatio * maxDelta.x(),secondPointRatio * maxDelta.y() })
          new PairPathPointExp(QPointF{ 0,0 })
        );

        keyoffset++;
        newcontrolledPaths.insert(i.key() + keyoffset, z_r2_bottom);
      }

      if (nbKnots > 0) {

        auto z_r1_bottom = new Glyph::Knot();
        z_r1_bottom->leftValue = {};
        z_r1_bottom->leftValue.jointtype = Glyph::path_join_tension;

        z_r1_bottom->rightValue = {};
        z_r1_bottom->rightValue.jointtype = Glyph::path_join_tension;

        z_r1_bottom->expr = std::make_unique<FunctionMFExp>(bendFuncName,
          new PairPathPointExp(origin + firstPointRatio * minValue + (left ? joinvector : QPointF())),
          //new LitPointPathPointExp({ firstPointRatio * maxDelta.x(),firstPointRatio * maxDelta.y() })
          new PairPathPointExp(QPointF{ 0,0 })
        );

        keyoffset++;
        newcontrolledPaths.insert(i.key() + keyoffset, z_r1_bottom);
      }

      if (!left) {
        /*
        secondKnot->leftValue.type = Glyph::mpgui_given;

        secondKnot->leftValue.dirExpr = std::make_unique<DirPathPointExp>(
          new FunctionMFExp(bendFuncName,
            new LitPathNumericExp(dirVar1Value),
            new LitPathNumericExp(0))
          );*/

        secondKnot->leftValue.jointtype = Glyph::path_join_tension;
        secondKnot->leftValue.tensionExpr = nullptr;
      }
      else {

        /*
        secondKnot->rightValue.type = Glyph::mpgui_given;

        secondKnot->rightValue.dirExpr = std::make_unique<DirPathPointExp>(
          new FunctionMFExp(bendFuncName,
            new LitPathNumericExp(dirVar1Value),
            new LitPathNumericExp(0))
          );*/

        secondKnot->leftValue.jointtype = Glyph::path_join_tension;
        secondKnot->leftValue.tensionExpr = nullptr;
      }




      secondKnot->expr = std::make_unique<FunctionMFExp>(bendFuncName,
        secondKnot->expr.release(),
        new PairPathPointExp(QPointF{ 0,0 })
      );


      keyoffset++;

      newcontrolledPaths.insert(i.key() + keyoffset, secondKnot);



      if (i.key() == lastKey) {
        newcontrolledPaths.insert(i.key() + ++keyoffset, newcontrolledPaths.first());
      }

      secondPointDone = true;

      continue;
    }

    newcontrolledPaths.insert(i.key() + keyoffset, controlledPath.value(i.key()));
  }

  m_glyph->blockSignals(false);



  m_glyph->controlledPaths[parentFirst->m_numsubpath] = newcontrolledPaths;

  m_glyph->isDirty = true;

  auto newSource = m_glyph->source();

  GlyphSourceChangeCommand* command = new GlyphSourceChangeCommand(m_glyph, "Source Changed", oldSource, newSource, true);
  m_glyph->undoStack()->push(command);



  return true;


}

bool KnotControlledItem::updateControlPoint(QGraphicsSceneMouseEvent* event, bool leftControl, QPointF diff, bool shift, bool ctrl) {

  
  if (!incurve || incurve->isSelected()) {
    return false;
  }

  auto scene = (GlyphScene*)incurve->scene();

  Glyph::KnotEntryExit& controlValue = leftControl ? m_glyphknot->leftValue : m_glyphknot->rightValue;
  KnotItem* item = leftControl ? left : right;

  bool canmodify = false;

  QLineF line(QPointF(), incurve->mapFromScene(event->scenePos()));
  QLineF currentline(QPointF(), incurve->mapFromScene(item->scenePos()));
  double ang = currentline.angleTo(line);
  double radianAngle = qDegreesToRadians(ang);
  int expIndex = scene->getControlledPosition(event);

  //Controls
  if (controlValue.type == Glyph::mpgui_explicit) {

    updateControlledPoint(controlValue.dirExpr.get(), expIndex, diff);

    /*
    if (controlValue.isEqualBefore && leftControl) {
      Glyph::Knot* knot = m_glyph->controlledPaths[m_numsubpath][m_numpoint - 1];
      knot->rightValue.dirExpr = m_glyphknot->leftValue.dirExpr->clone();
    }*/

    return true;
  }

  //Direction
  if (!shift && controlValue.type == Glyph::mpgui_given) {
    //updateControlledPoint(controlValue.dirExpr.get(), expIndex, -diff);
    //canmodify = true;
    
    auto expr = controlValue.dirExpr.get();

    double deltaAng = line.angleTo(currentline);
    double radianDeltaAngle = qDegreesToRadians(deltaAng);
    // Vector having angle equal deltaAng
    auto vectorAngle = QPointF(qCos(radianDeltaAngle), qSin(radianDeltaAngle));
    if (expr->isConstant(expIndex)) {
      auto oldValue = expr->constantValue(expIndex).toPointF();
      //complex multiplication = rotation
      auto newValue = QPointF(oldValue.x() * vectorAngle.x() - oldValue.y() * vectorAngle.y(), oldValue.x() * vectorAngle.y() + oldValue.y() * vectorAngle.x());
      expr->setConstantValue(expIndex, newValue);
      canmodify = true;
    }
    else {
      auto parmName = expr->paramName(expIndex);
      if (!parmName.isEmpty()) {
        auto latinName = parmName.toLatin1();
        QVariant val = m_glyph->property(latinName);
        if (QVariant::PointF == val.type()) {
          QPointF oldValue = val.toPointF();
          auto newValue = QPointF(oldValue.x() * vectorAngle.x() - oldValue.y() * vectorAngle.y(), oldValue.x() * vectorAngle.y() + oldValue.y() * vectorAngle.x());
          m_glyph->setProperty(latinName, newValue);
          canmodify = true;
        }
        else if (QVariant::Double == val.type()) {
          double angleDegree = val.toDouble();
          m_glyph->setProperty(latinName, std::fmod(deltaAng + angleDegree, 360));
          canmodify = true;
        }
      }
    }
  }

  //Tensions
  auto expr = controlValue.tensionExpr.get();
  if (!ctrl && expr) {

    QByteArray latinName;


    double oldtension = 1;
    bool found = false;

    if (expr->isConstant(expIndex)) {
      oldtension = expr->constantValue(expIndex).toDouble();
      found = true;
    }
    else {
      auto parmName = expr->paramName(expIndex);
      if (!parmName.isEmpty()) {
        latinName = parmName.toLatin1();
        QVariant val = m_glyph->property(latinName);
        if (QVariant::Double == val.type()) {
          oldtension = val.toDouble();
          found = true;
        }
      }
    }

    if (found) {
      canmodify = true;
      double cos = qCos(radianAngle);
      double length = line.length() * cos;
      double newtension;

      if (length < 0) {
        newtension = 10000;
      }
      else {
        newtension = (oldtension / length) * currentline.length();
        if (newtension < 0.75) {
          newtension = 0.75;
        }
        if (newtension > 10000) {
          newtension = 10000;
        }
      }
      if (!latinName.isEmpty()) {
        m_glyph->setProperty(latinName, newtension);
      }
      else {
        expr->setConstantValue(expIndex, newtension);
      }

    }

    return canmodify;
  }
}
