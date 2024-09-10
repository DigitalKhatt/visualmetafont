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

#include "guidesitem.hpp"
#include "QPen"
#include "QPainter"
#include <QGraphicsScene>
#include <QGraphicsView>
#include "font.hpp"


GuidesItem::GuidesItem(Glyph* glyph, QGraphicsItem * parent) : QGraphicsObject(parent) {
	this->glyph = glyph;

	QTransform m;
	m.scale(1, -1);

	setTransform(m);

  lineHeight = glyph->font->lineHeight();
}

GuidesItem::~GuidesItem() {

}
QRectF GuidesItem::boundingRect() const
{
	QRectF rect;
	QGraphicsScene * scen = scene();
	if (scen) {

		auto views = scen->views();

		if (views.count() > 0) {

			QGraphicsView * view = scen->views().first();

			QPolygonF ress = view->mapToScene(view->rect());

			rect = ress.boundingRect();

			//-1000, -1700, 3000, 2500
			
			if (rect.x() > -1000) {
				rect.setX(-1000);
			}

			if (rect.y() > -1700) {
				rect.setY(-1700);
			}

			if (rect.width() < 3000) {
				rect.setWidth(3000);
			}

			if (rect.height() < 2500) {
				rect.setHeight(2500);
			}
		}

		//rect = scen->sceneRect();
	}

	return QRectF(rect.left(), -rect.bottom(), rect.width(), rect.height());
}
void GuidesItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{

	QGraphicsView * view = scene()->views().first();

	QPolygonF ress = view->mapToScene(view->rect());

	QPen pen = QPen(Qt::darkGray);
	pen.setWidth(0);


	QGraphicsScene * scen = scene();

	QRectF rect = ress.boundingRect(); // scen->sceneRect();

	//QRectF rect = boundingRect();

	painter->setPen(pen);

	painter->setRenderHint(QPainter::Antialiasing, false);

	double top = rect.top();
	double bottom = rect.bottom();

	painter->drawLine(rect.left(), 0, rect.right(), 0);
	painter->drawLine(0, -bottom, 0, -top);

	double width = glyph->getComputedValues().width;

	painter->drawLine(width, -bottom, width, -top);

	painter->drawLine(rect.left(), 800, rect.right(), 800);
	painter->drawLine(rect.left(), -200, rect.right(), -200);	
	painter->drawLine(rect.left(), lineHeight, rect.right(), lineHeight);



	QTransform t = painter->transform();
	qreal m11 = t.m11(), m22 = t.m22();
	painter->save(); // save painter state
	painter->setTransform(QTransform(1, t.m12(), t.m13(),
		t.m21(), 1, t.m23(),
		t.m31(), t.m32(), t.m33()));



	int margin = 2;
	int x = width;
	/*
	int y = 0;
	painter->drawText(x*m11 + margin, y*m22 + margin + 10, "BaseLine");
	y = 800 ;
	painter->drawText(x*m11 + margin, y*m22 - margin, "Ascender(800)");
	y = -200;
	painter->drawText(x*m11 + margin, y*m22 + margin + 10, "Descender(-200)");*/



	QRectF brect = ress.boundingRect();
	x = brect.left();

	painter->drawText(x*m11 + margin, 0 * m22 + margin + 10, "BaseLine");
	painter->drawText(x*m11 + margin, 800 * m22 - margin, "Ascender(800)");
	painter->drawText(x*m11 + margin, -200 * m22 + margin + 10, "Descender(-200)");



	painter->restore(); // restore painter state




}
