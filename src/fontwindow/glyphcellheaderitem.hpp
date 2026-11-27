/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#pragma once
#ifndef GLYPHCELLHEADERITEM_H
#define GLYPHCELLHEADERITEM_H
#include <QGraphicsItem>
#include <QtWidgets>

class GlyphCellHeaderItem : public QGraphicsItem {	

public:
	GlyphCellHeaderItem(QGraphicsItem * parent = Q_NULLPTR);
	~GlyphCellHeaderItem();

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;

	void setText(QString text);

private:
	QString text;
	
};
#endif // GLYPHCELLHEADERITEM_H