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

#pragma once
#ifndef FONTSCENE_H
#define FONTSCENE_H
#include <QGraphicsScene>
#include "glyph.hpp"

class FontScene : public QGraphicsScene {
	Q_OBJECT

public:
	FontScene(QObject * parent = Q_NULLPTR);
	~FontScene();

	void repositionItems();

signals:
	void glyphActivated(Glyph *item);

protected:
	void drawForeground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) Q_DECL_OVERRIDE;

private:
	int nbcols;
	int nbrows;
	bool isDoubleClick = false;
};
#endif // FONTSCENE_H