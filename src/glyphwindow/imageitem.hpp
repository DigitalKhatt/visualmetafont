/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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