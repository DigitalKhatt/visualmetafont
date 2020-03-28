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

#include "glyphcellheaderitem.hpp"

GlyphCellHeaderItem::GlyphCellHeaderItem(QGraphicsItem * parent) : QGraphicsItem(parent) {
	
}

GlyphCellHeaderItem::~GlyphCellHeaderItem() {
	
}
QRectF GlyphCellHeaderItem::boundingRect() const{
	QRectF rect;
	QGraphicsScene* scene = this->scene();
	if (scene) {
		QGraphicsView *view = scene->views().first();
		qreal height = view->transform().inverted().mapRect(QRectF(0, 0, 15, 15)).height();
		rect = QRectF(0, 0, 1000, height);
	}
	return rect;
}
void GlyphCellHeaderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
	QGraphicsScene* scene = this->scene();
	if (scene) {

		
		QFont font = painter->font();
		//int fontsize = font.pixelSize();
		//font.setPointSize(12);
		QFontMetrics fm(font);		
		int fontsize = fm.height();
		painter->setFont(font);

		
		

		QGraphicsView *view = scene->views().first();
		qreal height = view->transform().inverted().mapRect(QRectF(0, 0, fontsize, fontsize)).height();
		QRectF rect(0, 0, 1000, height);

		QColor selectionColor(Qt::lightGray);
		selectionColor.setAlphaF(1);
		
		painter->fillRect(rect, selectionColor);


		qreal factor = 1 / (view->transform().mapRect(QRectF(0, 0, 1, 1)).width());

		painter->scale(factor, factor);
		QRectF textRect(50 / factor, 0, 900 / factor, height / factor);

		painter->drawText(textRect, Qt::AlignLeft | Qt::TextSingleLine, text);// , QTextOption(Qt::AlignCenter | Qt::TextSingleLine));
	}


}

void GlyphCellHeaderItem::setText(QString text) {
	this->text = text;
}

