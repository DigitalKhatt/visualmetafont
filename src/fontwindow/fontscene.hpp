/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
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
	int nbrows = 1;
	bool isDoubleClick = false;
};
#endif // FONTSCENE_H