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

#include "fontscene.hpp"
#include "glyphcellitem.hpp"
#include "glyphwindow.h"
#include <QtWidgets>

FontScene::FontScene(QObject * parent) : QGraphicsScene(parent) {
	nbcols = 10;
}

FontScene::~FontScene() {
	
}

void FontScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) {

	QGraphicsScene::mouseDoubleClickEvent(mouseEvent);

	isDoubleClick = true;
}
void FontScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {

	if (isDoubleClick) {
		QList<QGraphicsItem*> list = items(mouseEvent->scenePos());	

		if (!list.isEmpty()) {
			GlyphCellItem *cell = qgraphicsitem_cast<GlyphCellItem *>(list.first());

			if (cell) {
				//emit glyphActivated(cell->getGlyph());
				GlyphWindow* glyphwindow = new GlyphWindow(cell->getGlyph());
				//glyphwindow->setWindowFlags(Qt::WindowStaysOnTopHint);
				//glyphwindow->setAttribute(Qt::WA_NativeWindow);
				glyphwindow->showMaximized();


			}
		}
	}
	QGraphicsScene::mouseReleaseEvent(mouseEvent);

	isDoubleClick = false;
	
}

void FontScene::repositionItems()
{

	auto vs = views();

	if (vs.length() == 0)  return;

	QGraphicsView* view = vs[0];

	double viewportwidth = view->viewport()->width();

	qreal factor = view->transform().mapRect(QRectF(0, 0, 1, 1)).width();	


	nbcols = viewportwidth / (1000 * factor);

	//factor = viewportwidth / (1000 * nbcols);

	//view->resetTransform();

	//view->scale(factor, factor);

	int col = 0;
	int row = 0;

	QList<QGraphicsItem *> cells = this->items(Qt::AscendingOrder);
	int length = cells.length();

	for (int i = 0; i < length; i++) {
		QGraphicsItem* item = cells[i];

		GlyphCellItem * cell = qgraphicsitem_cast<GlyphCellItem *>(item);
		if (cell) {
			cell->setPos(col * 1000, row * 1000);
			col++;
			if (col == nbcols) {
				col = 0;
				row++;
			}
		}
	}

	nbrows = row + 1;

	setSceneRect(0, 0, nbcols * 1000, nbrows * 1000);
}
void FontScene::drawForeground(QPainter *painter, const QRectF &rect)
{

	painter->setPen(QPen(Qt::darkGray, 5));
	QVarLengthArray<QLineF, 100> lines;

	QRectF test = this->sceneRect();

	double width = rect.width();
	double height = rect.height();	

	for (int i = 0; i <= nbcols; i++) {
		lines.append(QLineF(1000*i, 0, 1000*i, 1000* nbrows));
	}

	for (int i = 0; i <= nbrows; i++) {
		lines.append(QLineF(0, 1000*i, 1000*nbcols, 1000 * i));
	}

	

	painter->drawLines(lines.data(), lines.size());
}
