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
#include <QGraphicsView>
#include "glyph.hpp"
#include "contouritem.hpp"
#include "glyphscene.hpp"
#include "imageitem.hpp"
#include "guidesitem.hpp"
#include "componentitem.hpp"

class GlyphView : public QGraphicsView {
	Q_OBJECT

public:
	GlyphView(Glyph *glyph);
	~GlyphView();

	//void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
	void setGlyph(Glyph * glyph);	
	GlyphScene * getScene();
	
	

public slots:	
	void zoomIn();
	void zoomOut();



protected:
	void scaleView(qreal scaleFactor);
	void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
	
	GlyphScene * glyphscene;
	Glyph *m_glyph;


	
	
};
