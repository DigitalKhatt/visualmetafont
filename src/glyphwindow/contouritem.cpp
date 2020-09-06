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
#include "tensiondirectionitem.hpp"



ContourItem::~ContourItem() {

}

ContourItem::ContourItem(QGraphicsItem * parent)
	: QGraphicsObject(parent)
{
	penWidth = 1;
	rotationAngle = 0;
	penColor = Qt::black;

	QMatrix m;
	m.scale(1, -1);

	setMatrix(m);

	path = NULL;
	parampoints = NULL;
	labels = NULL;

}

void ContourItem::setFillEnabled(bool enable)
{
	this->enableFill = enable;

	if (this->path) {
		if(enable)
			this->path->setBrush(Qt::black);
		else
			this->path->setBrush(QBrush{});
	}
}

void ContourItem::setFillGradient(const QColor &color1, const QColor &color2)
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

void ContourItem::setPenColor(const QColor &color)
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

void ContourItem::paint(QPainter *painter,
	const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);


}
void ContourItem::generateedge(mp_edge_object *  h, bool newelement) {

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
		QMapIterator<QString, Glyph::Param> i(glyph->params);

		QPointF translate;
		auto edge = glyph->getEdge();
		if(edge != nullptr){
				 translate =  QPointF(edge->xpart, edge->ypart);
		}
		while (i.hasNext()) {
			i.next();		
			Glyph::Param param = i.value();
			/*
			if (param.type == Glyph::point && !param.isInControllePath) {
				QByteArray propname = param.name.toLatin1();
				QVariant val = glyph->property(propname);
				if (QVariant::PointF == val.type()) {
					QPointF point = val.toPointF() + translate;
					QGraphicsItem *  pair;
					if (newelement) {
						pair = new PairItem(param, glyph, this->parampoints);
					}
					else {
						pair = this->parampoints->childItems()[childnb++];
					}

					pair->setPos(point.x(), point.y());
					//pair->ensureVisible(-1, -1, 2, 2, 0, 0);
				}
			}
			else */
			if (param.type == Glyph::expression && !param.isInControllePath){
				 auto value =  glyph->expressions.value(param.name);
				 if(value->type() == QVariant::PointF){
					 QPointF point = value->value().toPointF() + translate;
					 QGraphicsItem *  pair;
					 if (newelement) {
						 pair = new PairItem(param, glyph, this->parampoints);
					 }
					 else {
						 pair = this->parampoints->childItems()[childnb++];
					 }

					 pair->setPos(point.x(), point.y());
				 }

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
					mp_fill_object * fill = (mp_fill_object *)body;
					QString prescript = fill->pre_script;
					/*
					if (prescript.contains("begin component")) {
						while (body = body->next) {
							fill = (mp_fill_object *)body;
							QString postscipt = fill->post_script;
						}
					}				*/					
					//if (numsubpath > glyph->controlledPaths.count() - 1) {
						QPainterPath subpath = mp_dump_solved_path(fill->path_p, numsubpath++, newelement);
						localpath.addPath(subpath);
					//}
					
					break;
				}
				case mp_text_code: {
					mp_text_object * text = (mp_text_object *)body;
					QGraphicsSimpleTextItem * textitem = new QGraphicsSimpleTextItem(labels);
					textitem->setFlags(QGraphicsItem::ItemIgnoresTransformations);
					textitem->setPos(text->tx, text->ty);
					textitem->setText(text->text_p);
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
QPainterPath ContourItem::mp_dump_solved_path(mp_gr_knot h, int numsubpath, bool newelement) {
	mp_gr_knot p, q;
	QPainterPath path;
	if (h == NULL) return path;

	int numpoint = 0;

	path.moveTo(h->x_coord, h->y_coord);
	p = h;
	do {	

		if (p->data.types.right_type == mp_endpoint) {
			break;
		}

		if (newelement) {
			knotControlledItems[numsubpath][numpoint] = new KnotControlledItem(numsubpath, numpoint, p, glyph, this->path);
			
		}
		else {
			KnotControlledItem* item = knotControlledItems.value(numsubpath).value(numpoint);
			if(item != nullptr)
				item->setPositions(p);
			
			
		}

		q = p->next;

		
		path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

		numpoint++;

		p = q;
		
	} while (p != h);

	/*
	if (h->data.types.left_type != mp_endpoint) {
		//	path.closeSubpath();
		if (newelement) {
			new KnotControlledItem(numsubpath, numpoint, p, glyph, this->path);
		}
		else {
			KnotControlledItem* item = (KnotControlledItem*)this->path->childItems()[numele++];
			item->setPositions(p);
		}
	}*/
	

	return path;

}
//QPainterPath ContourItem::mp_dump_solved_path(mp_gr_knot h, int & numele, int numsubpath, bool newelement) {
//	mp_gr_knot p, q;
//	QPainterPath path;
//	if (h == NULL) return path;
//
//	QPen penline(Qt::gray);
//	penline.setWidth(0);
//	penline.setCosmetic(true);
//
//
//	QGraphicsItem* pknot;
//	QGraphicsItem* qknot;
//
//	int numpoint = 0;
//
//	if (newelement) {
//		pknot = new KnotItem(KnotItem::InCurve, this->path);
//	}
//	else {
//		pknot = this->path->childItems()[numele++];
//	}
//	pknot->setPos(h->x_coord, h->y_coord);
//
//	path.moveTo(h->x_coord, h->y_coord);
//	p = h;
//	do {
//
//		QString rightkey = QString(numsubpath) + "_" + QString(numpoint);
//		numpoint++;
//		QString leftkey = QString(numsubpath) + "_" + QString(numpoint);
//
//		q = p->next;
//
//		path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);
//
//		if (newelement) {
//			qknot = new KnotItem(KnotItem::InCurve, this->path);
//			qknot->setPos(q->x_coord, q->y_coord);
//			if (glyph->ldirections[leftkey].type == Glyph::direction || glyph->ltensions[leftkey].type == Glyph::tension) {
//
//				TensionDirectionItem* pp = new TensionDirectionItem(glyph->ldirections[leftkey], glyph->ltensions[leftkey], glyph, qknot);
//				pp->calculatePosition(-QLineF(q->left_x, q->left_y, q->x_coord, q->y_coord).angle());
//
//				QGraphicsLineItem * line = new QGraphicsLineItem(qknot);
//				line->setPen(penline);
//				line->setLine(0, 0, pp->pos().x(), pp->pos().y());
//			}
//			if (glyph->rdirections[rightkey].type == Glyph::direction || glyph->rtensions[rightkey].type == Glyph::tension) {
//
//				TensionDirectionItem* pp = new TensionDirectionItem(glyph->rdirections[rightkey], glyph->rtensions[rightkey], glyph, pknot);
//				pp->calculatePosition(-QLineF(p->right_x, p->right_y, p->x_coord, p->y_coord).angle());
//
//				QGraphicsLineItem * line = new QGraphicsLineItem(pknot);
//				line->setPen(penline);
//				line->setLine(0, 0, pp->pos().x(), pp->pos().y());
//
//			}
//		}
//		else {
//			qknot = this->path->childItems()[numele++];
//			qknot->setPos(q->x_coord, q->y_coord);
//			if (glyph->ldirections[leftkey].type == Glyph::direction || glyph->ltensions[leftkey].type == Glyph::tension) {
//
//				TensionDirectionItem* pp = (TensionDirectionItem*)qknot->childItems()[0];
//				pp->calculatePosition(-QLineF(q->left_x, q->left_y, q->x_coord, q->y_coord).angle());
//
//				QGraphicsLineItem * line = (QGraphicsLineItem *)qknot->childItems()[1];
//				line->setLine(0, 0, pp->pos().x(), pp->pos().y());
//			}
//			if (glyph->rdirections[rightkey].type == Glyph::direction || glyph->rtensions[rightkey].type == Glyph::tension) {
//
//				int index = 0;
//				if (glyph->ldirections[rightkey].type == Glyph::direction || glyph->ltensions[rightkey].type == Glyph::tension) {
//					index = 2;
//				}
//
//				TensionDirectionItem* pp = (TensionDirectionItem*)pknot->childItems()[index];
//				pp->calculatePosition(-QLineF(p->right_x, p->right_y, p->x_coord, p->y_coord).angle());
//
//				QGraphicsLineItem * line = (QGraphicsLineItem *)pknot->childItems()[index + 1];
//				line->setLine(0, 0, pp->pos().x(), pp->pos().y());
//
//			}
//		}
//
//		/*
//		QGraphicsItem* knot;
//		if (newelement)
//			knot = new KnotItem(KnotItem::RightControl, this->path);
//		else
//			knot = this->path->childItems()[numele++];
//
//		knot->setPos(p->right_x, p->right_y);
//
//		if (newelement)
//			knot = new KnotItem(KnotItem::LeftControl, this->path);
//		else
//			knot = this->path->childItems()[numele++];
//
//		knot->setPos(q->left_x, q->left_y);
//
//		QGraphicsLineItem * line;
//		if (newelement) {
//			line = new QGraphicsLineItem(this->path);
//			line->setPen(penline);
//			//line->setZValue(-2);
//		}
//		else
//			line = (QGraphicsLineItem *)this->path->childItems()[numele++];
//
//		line->setLine(p->x_coord, p->y_coord, p->right_x, p->right_y);
//
//		if (newelement) {
//			line = new QGraphicsLineItem(this->path);
//			line->setPen(penline);
//			//line->setZValue(-2);
//		}
//		else
//			line = (QGraphicsLineItem *)this->path->childItems()[numele++];
//
//		line->setLine(q->x_coord, q->y_coord, q->left_x, q->left_y);
//		*/
//
//		p = q;
//		pknot = qknot;
//	} while (p != h);
//
//	if (h->data.types.left_type != mp_endpoint)
//		path.closeSubpath();
//
//	return path;
//
//}
