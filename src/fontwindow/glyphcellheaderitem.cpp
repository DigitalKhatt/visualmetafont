/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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

