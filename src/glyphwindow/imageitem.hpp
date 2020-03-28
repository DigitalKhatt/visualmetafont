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
#ifndef IMAGEITEM_H
#define IMAGEITEM_H
#include <QGraphicsObject>
#include "glyph.hpp"

class ImageItem : public QGraphicsPixmapItem {	

public:
	ImageItem(Glyph * glyph, QGraphicsItem * parent = Q_NULLPTR);
	ImageItem(Glyph * glyph, const QPixmap &pixmap, QGraphicsItem *parent = Q_NULLPTR);
	~ImageItem();
	void setPath(QString path);
	QString path();

protected:
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;	
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;

private:
	QMenu *myContextMenu;
	Glyph * glyph;
	QString m_path;
	
};
#endif // IMAGEITEM_H