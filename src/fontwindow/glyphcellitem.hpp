/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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
	QGraphicsPathItem* path = nullptr;	
	GlyphCellHeaderItem* header;
	QPicture picture;
	QTransform m;
};
#endif // GLYPHCELLITEM_H
