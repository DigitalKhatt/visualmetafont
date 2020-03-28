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

#include "KnotControlledItem.hpp"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsView>
#include "glyph.hpp"
#include "tensiondirectionitem.hpp"
#include <QMenu>


KnotControlledItem::KnotControlledItem(int numsubpath, int numpoint, mp_gr_knot knot, Glyph* glyph, QGraphicsItem* parent)
	: QGraphicsObject(parent) {

	this->m_numsubpath = numsubpath;
	this->m_numpoint = numpoint;
	this->m_knot = knot;
	this->m_glyph = glyph;
	left = NULL;
	right = NULL;
	lefttd = NULL;
	righttd = NULL;

	QPen penline(Qt::gray);
	penline.setWidth(0);
	penline.setCosmetic(true);

	setFlags(ItemIsMovable | ItemHasNoContents);


	QString rightkey = QString(numsubpath) + "_" + QString(numpoint);
	QString leftkey = QString(numsubpath) + "_" + QString(numpoint);

	setFiltersChildEvents(true);

	incurve = new KnotItem(KnotItem::InCurve, this);

	if (m_glyph->controlledPaths.contains(numsubpath) && m_glyph->controlledPaths[numsubpath].contains(numpoint)) {
		m_glyphknot = m_glyph->controlledPaths[numsubpath][numpoint];
		if (m_glyphknot->isConstant || m_glyph->params.contains(m_glyphknot->paramName)) {
			incurve->setFlags(ItemIsMovable | ItemIsSelectable);
		}

		leftline = new QGraphicsLineItem(this);
		leftline->setPen(penline);
		leftline->setZValue(-1);
		left = new KnotItem(KnotItem::LeftControl, this);

		if (m_glyphknot->leftValue.isDirConstant || m_glyphknot->leftValue.isControlConstant || m_glyph->params.contains(m_glyphknot->leftValue.controlValue)) { // || m_glyph->params.contains(m_glyphknot->leftValue.value)) {

			left->setFlags(ItemIsMovable | ItemIsSelectable);

		}

		rightline = new QGraphicsLineItem(this);
		rightline->setPen(penline);
		rightline->setZValue(-1);
		right = new KnotItem(KnotItem::RightControl, this);
		if (m_glyphknot->rightValue.isDirConstant || m_glyphknot->rightValue.isControlConstant || m_glyph->params.contains(m_glyphknot->rightValue.controlValue)) {// || m_glyph->params.contains(m_glyphknot->rightValue.value)) {

			right->setFlags(ItemIsMovable | ItemIsSelectable);

		}
	}





	//left = new KnotItem(KnotItem::LeftControl, this);
	//right = new KnotItem(KnotItem::RightControl, this);

	if (m_glyph->ldirections[leftkey].type == Glyph::direction || m_glyph->ltensions[leftkey].type == Glyph::tension) {

		lefttd = new TensionDirectionItem(m_glyph->ldirections[leftkey], m_glyph->ltensions[leftkey], glyph, incurve);

		leftline = new QGraphicsLineItem(incurve);
		leftline->setPen(penline);
	}
	if (m_glyph->rdirections[rightkey].type == Glyph::direction || m_glyph->rtensions[rightkey].type == Glyph::tension) {

		righttd = new TensionDirectionItem(m_glyph->rdirections[rightkey], m_glyph->rtensions[rightkey], m_glyph, incurve);


		rightline = new QGraphicsLineItem(incurve);
		rightline->setPen(penline);

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
	if (lefttd) {
		lefttd->calculatePosition(-QLineF(m_knot->left_x, m_knot->left_y, m_knot->x_coord, m_knot->y_coord).angle());
		leftline->setLine(0, 0, lefttd->pos().x(), lefttd->pos().y());
	}
	if (righttd) {
		righttd->calculatePosition(-QLineF(m_knot->right_x, m_knot->right_y, m_knot->x_coord, m_knot->y_coord).angle());
		rightline->setLine(0, 0, righttd->pos().x(), righttd->pos().y());
	}

}
QRectF KnotControlledItem::boundingRect() const
{
	return QRectF();
}
void KnotControlledItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{


}
bool KnotControlledItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event) {
	if (!isEnabled()) {
		return false;
	}
	if (event->type() == QEvent::GraphicsSceneMouseMove) {

		QGraphicsSceneMouseEvent* me = static_cast<QGraphicsSceneMouseEvent*>(event);

		Qt::KeyboardModifiers modifiers = me->modifiers();

		bool shift = false;
		bool ctrl = false;

		if (modifiers & Qt::ShiftModifier) {
			shift = true;
		}
		if (modifiers & Qt::ControlModifier) {
			ctrl = true;
		}



		//QPointF currentParentPos = watched->mapToParent(watched->mapFromScene(me->scenePos()));
		//QPointF currentParentPos = watched->mapToParent(me->pos());
		//QPointF diff = currentParentPos - watched->pos();
		QPointF diff = me->pos() - me->lastPos();
		QPointF pair = incurve->mapFromScene(me->scenePos());
		QLineF line(QPointF(), pair);

		if (watched == incurve) {

			if (m_glyphknot->isConstant) {
				m_glyphknot->x = m_glyphknot->x + diff.x();
				m_glyphknot->y = m_glyphknot->y + diff.y();
			}
			else {
				QVariant val = m_glyph->property(m_glyphknot->paramName.toLatin1());
				if (QMetaType::QPointF == val.type()) {
					QPointF point = val.toPointF();
					point.setX(point.x() + diff.x());
					point.setY(point.y() + diff.y());
					m_glyph->setProperty(m_glyphknot->paramName.toLatin1(), point);
				}

			}

			if (m_glyphknot->leftValue.isControlConstant && m_glyphknot->leftValue.type == Glyph::mpgui_explicit) {
				m_glyphknot->leftValue.x = m_glyphknot->leftValue.x + diff.x();
				m_glyphknot->leftValue.y = m_glyphknot->leftValue.y + diff.y();
			}

			if (m_glyphknot->rightValue.isControlConstant && m_glyphknot->rightValue.type == Glyph::mpgui_explicit) {
				m_glyphknot->rightValue.x = m_glyphknot->rightValue.x + diff.x();
				m_glyphknot->rightValue.y = m_glyphknot->rightValue.y + diff.y();
			}

			//m_glyph->setWidth(m_glyph->width());

			return true;
		}
		else if (watched == left) {

			return updateControlPoint(true, diff, shift, ctrl, line);

			/*
			if (incurve && !incurve->isSelected()) {

				if (m_glyphknot->leftValue.isControlConstant && m_glyphknot->leftValue.type == Glyph::mpgui_explicit) {

					m_glyphknot->leftValue.x = m_glyphknot->leftValue.x + diff.x();
					m_glyphknot->leftValue.y = m_glyphknot->leftValue.y + diff.y();

					if (m_glyphknot->leftValue.isEqualBefore) {
						Glyph::Knot* knot = m_glyph->controlledPaths[m_numsubpath][m_numpoint - 1];
						knot->rightValue.x = m_glyphknot->leftValue.x;
						knot->rightValue.y = m_glyphknot->leftValue.y;
					}
				}

				if (m_glyphknot->leftValue.isDirConstant && m_glyphknot->leftValue.type == Glyph::mpgui_given) {
					if (!shift) {

						double angle = -line.angle() + 180;
						m_glyphknot->leftValue.x = angle; // < 0 ? 360 + angle : angle;
					}
				}

				if (m_glyphknot->leftValue.type != Glyph::mpgui_explicit && !ctrl) {
					QPointF currentpos = incurve->mapFromScene(left->scenePos());
					QLineF currentline(QPointF(), currentpos);
					double ang = currentline.angleTo(line);
					double cos = qCos(qDegreesToRadians(ang));
					double length = line.length() * cos;
					double newtension;
					double oldtension = 1;
					bool canmodify = true;
					if (m_glyphknot->leftValue.isControlConstant) {
						oldtension = m_glyphknot->leftValue.y;
					}
					else {
						QVariant val = m_glyph->property(m_glyphknot->leftValue.controlValue.toLatin1());
						if (QMetaType::Double == val.type()) {
							oldtension = val.toDouble();
						}
						else {
							canmodify = false;
						}
					}
					if (canmodify) {
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
						if (m_glyphknot->leftValue.isControlConstant) {
							m_glyphknot->leftValue.y = newtension;
						}
						else {
							m_glyph->setProperty(m_glyphknot->leftValue.controlValue.toLatin1(), newtension);
						}

					}


				}

				//m_glyph->setWidth(m_glyph->width());
			}

			return true;*/

		}
		else if (watched == right) {

			return updateControlPoint(false, diff, shift, ctrl, line);
			/*
			if (incurve && !incurve->isSelected()) {

				if (m_glyphknot->rightValue.isControlConstant && m_glyphknot->rightValue.type == Glyph::mpgui_explicit) {
					m_glyphknot->rightValue.x = m_glyphknot->rightValue.x + diff.x();
					m_glyphknot->rightValue.y = m_glyphknot->rightValue.y + diff.y();
				}

				if (m_glyphknot->rightValue.isDirConstant && m_glyphknot->rightValue.type == Glyph::mpgui_given) {
					if (!shift) {
						QPointF pair = incurve->mapFromScene(me->scenePos());
						QLineF line(pair, QPointF());
						double angle = -line.angle() + 180;
						m_glyphknot->rightValue.x = angle; // < 0 ? 360 + angle : angle;
					}
				}

				if (m_glyphknot->rightValue.isControlConstant && m_glyphknot->rightValue.type != Glyph::mpgui_explicit && !ctrl) {
					QPointF currentpos = incurve->mapFromScene(right->scenePos());
					QLineF currentline(QPointF(), currentpos);
					double ang = currentline.angleTo(line);
					double cos = qCos(qDegreesToRadians(ang));
					double length = line.length() * cos;
					if (length < 0) {
						m_glyphknot->rightValue.y = 10000;
					}
					else {
						m_glyphknot->rightValue.y = (m_glyphknot->rightValue.y / length) * currentline.length();
						if (m_glyphknot->rightValue.y < 0.75) {
							m_glyphknot->rightValue.y = 0.75;
						}
						if (m_glyphknot->rightValue.y > 10000) {
							m_glyphknot->rightValue.y = 10000;
						}
					}
				}

				//m_glyph->setWidth(m_glyph->width());
			}*/



			return true;
		}

	}
	else if (event->type() == QEvent::GraphicsSceneContextMenu) {
		QGraphicsSceneContextMenuEvent* me = static_cast<QGraphicsSceneContextMenuEvent*>(event);

		if (watched == incurve) {
			QMenu menu;
			//QAction * scaleAct = new QAction("&Scale", this);	
			//connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

			menu.addAction("Left");
			menu.addAction("Right");
			menu.addAction("Up");
			menu.addAction("Down");
			menu.addAction("dir");
			menu.addAction("dir-dir");
			QAction* a = menu.exec(me->screenPos());
			if (a != NULL) {
				if (a->text() == "Left") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "left";
					m_glyphknot->leftValue.isDirConstant = false;
					m_glyphknot->rightValue.type = Glyph::mpgui_open;
					m_glyph->setWidth(m_glyph->width());
				}
				else if (a->text() == "Right") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "right";
					m_glyphknot->leftValue.isDirConstant = false;
					m_glyphknot->rightValue.type = Glyph::mpgui_open;
					m_glyph->setWidth(m_glyph->width());
				}
				else if (a->text() == "Up") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "up";
					m_glyphknot->leftValue.isDirConstant = false;
					m_glyphknot->rightValue.type = Glyph::mpgui_open;
					m_glyph->setWidth(m_glyph->width());
				}
				else if (a->text() == "Down") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "down";
					m_glyphknot->leftValue.isDirConstant = false;
					m_glyphknot->rightValue.type = Glyph::mpgui_open;
					m_glyph->setWidth(m_glyph->width());
				}
				else if (a->text() == "dir") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "";
					m_glyphknot->leftValue.isDirConstant = true;
					m_glyphknot->leftValue.x = 45;
					m_glyphknot->rightValue.type = Glyph::mpgui_open;
					m_glyph->setWidth(m_glyph->width());
				}
				else if (a->text() == "dir-dir") {
					m_glyphknot->leftValue.type = Glyph::mpgui_given;
					m_glyphknot->leftValue.value = "";
					m_glyphknot->leftValue.isDirConstant = true;
					m_glyphknot->leftValue.x = 45;
					m_glyphknot->rightValue.type = Glyph::mpgui_given;
					m_glyphknot->rightValue.value = "";
					m_glyphknot->rightValue.isDirConstant = true;
					m_glyphknot->rightValue.x = -45;
					m_glyph->setWidth(m_glyph->width());
				}
			}

			return true;
		}
	}

	return QGraphicsItem::sceneEventFilter(watched, event);
}

bool KnotControlledItem::updateControlPoint(bool leftControl, QPointF diff, bool shift, bool ctrl, QLineF line) {

	Glyph::KnotEntryExit& controlValue = leftControl ? m_glyphknot->leftValue : m_glyphknot->rightValue;
	KnotItem* item = leftControl ? left : right;

	bool canmodify = false;

	if (incurve && !incurve->isSelected()) {

		canmodify = true;

		if (controlValue.isControlConstant && controlValue.type == Glyph::mpgui_explicit) {

			controlValue.x = controlValue.x + diff.x();
			controlValue.y = controlValue.y + diff.y();

			if (controlValue.isEqualBefore && leftControl) {
				Glyph::Knot* knot = m_glyph->controlledPaths[m_numsubpath][m_numpoint - 1];
				knot->rightValue.x = m_glyphknot->leftValue.x;
				knot->rightValue.y = m_glyphknot->leftValue.y;
			}
		}

		if (controlValue.isDirConstant && controlValue.type == Glyph::mpgui_given) {
			if (!shift) {				
				controlValue.x = -line.angle() + (leftControl ? 180 : 0);
			}
		}

		if (controlValue.type != Glyph::mpgui_explicit && !ctrl) {
			QPointF currentpos = incurve->mapFromScene(item->scenePos());
			QLineF currentline(QPointF(), currentpos);
			double ang = currentline.angleTo(line);
			double cos = qCos(qDegreesToRadians(ang));
			double length = line.length() * cos;
			double newtension;
			double oldtension = 1;
			
			if (controlValue.isControlConstant) {
				oldtension = controlValue.y;
			}
			else {
				QVariant val = m_glyph->property(controlValue.controlValue.toLatin1());
				if (QMetaType::Double == val.type()) {
					oldtension = val.toDouble();
				}
				else {
					canmodify = false;
				}
			}
			if (canmodify) {
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
				if (controlValue.isControlConstant) {
					controlValue.y = newtension;
				}
				else {
					m_glyph->setProperty(controlValue.controlValue.toLatin1(), newtension);
				}

			}


		}

		return canmodify;

		//m_glyph->setWidth(m_glyph->width());
	}
}
