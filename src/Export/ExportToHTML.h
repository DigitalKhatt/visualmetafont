/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#ifndef EXPORTTOHTML_H
#define EXPORTTOHTML_H

#include "OtLayout.h"
#include "qtextstream.h"

struct mp_graphic_object;
typedef struct mp_gr_knot_data*mp_gr_knot;

class ExportToHTML
{
public:
	ExportToHTML(OtLayout* otlayout);	
	~ExportToHTML();
	void generateQuranPages(LayoutPageList pages, int lineWidth, OriginalPageList originalText, int scale);
	void generateQuranPagesOld(LayoutPageList pages, int lineWidth, OriginalPageList originalText, int scale);
  
	void edgetoHTML5Path(mp_graphic_object* body, QTextStream& out);
	void filltoHTML5Path(mp_gr_knot h, QTextStream& out);
	void generateGlyph(GlyphVis& glyph, QTextStream & out);
	void getImageStream(GlyphVis& glyph, QTextStream & out);

private:
	OtLayout * m_otlayout;
	QHash<int, int> usedGlyphs;
};

#endif
