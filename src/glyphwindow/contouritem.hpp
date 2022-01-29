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

# include <QtCore>
# include <QtGui>



# include "mplibps.h"
#include "glyph.hpp"
#include "knotcontrolleditem.hpp"



class ContourItem : public QGraphicsObject {
	Q_OBJECT

	friend class GlyphScene;
public:
	ContourItem(QGraphicsItem * parent = Q_NULLPTR);
	~ContourItem();

	QRectF boundingRect() const Q_DECL_OVERRIDE;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) Q_DECL_OVERRIDE;
	
	
	void setFillEnabled(bool enable);
	void setFillGradient(const QColor &color1, const QColor &color2);
	void setPenWidth(int width);
	void setPenColor(const QColor &color);
	void setRotationAngle(int degrees);
	void setGlyph(Glyph* glyph);
	void generateedge(mp_edge_object* h, bool newelement = true);

private:

	
	QPainterPath mp_dump_solved_path(mp_fill_object* fill,int numsubpath, bool newelement = true);

	Glyph* glyph;
	QGraphicsPathItem* path;
	QGraphicsItem* parampoints;
	QGraphicsItemGroup* labels;
	QColor fillColor1;
	QColor fillColor2;
	int penWidth;
	QColor penColor;
	int rotationAngle;
	QMap<int, QMap<int, KnotControlledItem*> >  knotControlledItems;
	bool enableFill;
	
};
