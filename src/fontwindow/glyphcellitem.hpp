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
#ifndef GLYPHCELLITEM_H
#define GLYPHCELLITEM_H
#include <QGraphicsItem>
#include "glyph.hpp"
#include "glyphcellheaderitem.hpp"

# include "mplibps.h"

class GlyphCellItem : public QGraphicsObject {	
	Q_OBJECT

public:
	GlyphCellItem(QGraphicsItem * parent = Q_NULLPTR);
	~GlyphCellItem();

	enum { Type = UserType + 1 };

	int type() const;

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

	void setGlyph(Glyph* glyph);
	Glyph* getGlyph();

public slots:
	void glyphChanged(QString name);

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
	void dropEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
	void setPath();
	
	Glyph* glyph;
	QGraphicsPathItem* path = NULL;
	GlyphCellHeaderItem* header;
	
};
#endif // GLYPHCELLITEM_H