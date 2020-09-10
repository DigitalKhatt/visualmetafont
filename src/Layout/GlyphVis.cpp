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

#include "GlyphVis.h"
#include "font.hpp"

#ifndef DIGITALKHATT_WEBLIB
#include "qpainterpath.h"
#include "qpainter.h"
#include "qcolor.h"
#endif

#include "automedina/automedina.h"
#include <cmath>

extern "C"
{
#include "mplibps.h"
#include "mppsout.h"
}

GlyphVis::GlyphVis() {
	m_edge = nullptr;
	m_otLayout = nullptr;
	copiedPath = nullptr;
	isCopiedPath = false;
}

GlyphVis::~GlyphVis()
{
	if (copiedPath && isCopiedPath) {
		mp_graphic_object* p, *q;
		
		p = copiedPath;
		while (p != NULL) {
			q = p->next;
			mp_gr_toss_object(p);
			p = q;
		}
		
	}

}
GlyphVis::GlyphVis(const GlyphVis& other) {
	name = other.name;
	originalglyph = other.originalglyph;
	charcode = other.charcode;
	width = other.width;
	height = other.height;
	depth = other.depth;
	charlt = other.charlt;
	charrt = other.charrt;
	bbox = other.bbox;
	leftAnchor = other.leftAnchor;
	rightAnchor = other.rightAnchor;
#ifndef DIGITALKHATT_WEBLIB
	path = other.path;
	picture = other.picture;
#endif
	anchors = other.anchors;
	matrix = other.matrix;


	isdirty = other.isdirty;
	m_edge = other.m_edge;
	m_otLayout = other.m_otLayout;

	copiedPath = other.copiedPath;
	isCopiedPath = other.isCopiedPath;

	if (other.copiedPath && isCopiedPath) {
		//copyEdgeBody();		
	}

	//paths = other.paths;
}

GlyphVis* GlyphVis::getAlternate(GlyphParameters parameters) {
	return m_otLayout->getAlternate(charcode, parameters);
}

GlyphVis::GlyphVis(GlyphVis&& other)
{

	name = other.name;
	originalglyph = other.originalglyph;
	charcode = other.charcode;
	width = other.width;
	height = other.height;
	depth = other.depth;
	charlt = other.charlt;
	charrt = other.charrt;
	bbox = other.bbox;
	leftAnchor = other.leftAnchor;
	rightAnchor = other.rightAnchor;
#ifndef DIGITALKHATT_WEBLIB
	path = other.path;
	picture = other.picture;
#endif
	anchors = other.anchors;
	matrix = other.matrix;

	copiedPath = other.copiedPath;
	isCopiedPath = other.isCopiedPath;

	isdirty = other.isdirty;
	m_edge = other.m_edge;
	m_otLayout = other.m_otLayout;	

	other.copiedPath = nullptr;
#ifndef DIGITALKHATT_WEBLIB
	other.path = {};
#endif

}

GlyphVis& GlyphVis::operator=(const GlyphVis& other) {
	if (this == &other) return *this;

	if (copiedPath) {
		mp_graphic_object* p, * q;

		p = copiedPath;
		while (p != NULL) {
			q = p->next;
			mp_gr_toss_object(p);
			p = q;
		}
		
	}

	*this = other;

	return *this;
}

#ifndef DIGITALKHATT_WEBLIB
void GlyphVis::refresh(QHash<QString, GlyphVis>& glyphs)
{

	GlyphVis& endofaya = glyphs["endofaya"];
	auto ayaPicture = endofaya.picture;


	if (charcode >= Automedina::AyaNumberCode && charcode <= Automedina::AyaNumberCode + 286) {

		QPicture picture;
		QPainter painter;


		painter.begin(&picture);
		painter.drawPicture(0, 0, ayaPicture);
		painter.setBrush(Qt::black);
		int ayaNumber = (charcode - Automedina::AyaNumberCode) + 1;

		int digitheight = 120;

		if (ayaNumber < 10) {
			auto& onesglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + ayaNumber]];
			auto position = glyphs["endofaya"].width / 2 - (onesglyph.width) / 2;
			painter.translate(position, digitheight);

			painter.drawPath(onesglyph.path);
		}
		else if (ayaNumber < 100) {
			int onesdigit = ayaNumber % 10;
			int tensdigit = ayaNumber / 10;

			auto& onesglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + onesdigit]];
			auto& tensglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + tensdigit]];

			auto position = glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + 40) / 2;
			painter.translate(position, digitheight);
			painter.drawPath(tensglyph.path);
			painter.translate(tensglyph.width + 40, 0);
			painter.drawPath(onesglyph.path);

		}
		else {
			int onesdigit = ayaNumber % 10;
			int tensdigit = (ayaNumber / 10) % 10;
			int hundredsdigit = ayaNumber / 100;

			auto& onesglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + onesdigit]];
			auto& tensglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + tensdigit]];
			auto& hundredsglyph = glyphs[m_otLayout->glyphNamePerCode[1632 + hundredsdigit]];

			auto position = glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + hundredsglyph.width + 80) / 2;
			painter.translate(position, digitheight);
			painter.drawPath(hundredsglyph.path);
			painter.translate(hundredsglyph.width + 40, 0);
			painter.drawPath(tensglyph.path);
			painter.translate(tensglyph.width + 40, 0);
			painter.drawPath(onesglyph.path);

		}


		painter.resetMatrix();






		painter.end();

		this->picture = picture;
	}
}
#endif
bool GlyphVis::isAyaNumber()
{
	return (charcode >= Automedina::AyaNumberCode && charcode <= Automedina::AyaNumberCode + 286);
}
GlyphVis::GlyphVis(OtLayout* otLayout, mp_edge_object* edge, bool copyPath) {
	m_edge = edge;
	m_otLayout = otLayout;

	this->name = m_edge->charname;
	if (m_edge->originalglyph != "" && this->name != m_edge->originalglyph)
		originalglyph = m_edge->originalglyph;

	charcode = m_edge->charcode;
	width = m_edge->width;
	height = m_edge->height;
	depth = m_edge->depth;
	charlt = m_edge->charlt;
	charrt = m_edge->charrt;

	if(edge->body == nullptr ){
	  bbox.llx = 0;
	  bbox.lly = 0;
	  bbox.urx = 0;
	  bbox.ury = 0;
	}else{
	  bbox.llx = m_edge->minx;
	  bbox.lly = m_edge->miny;
	  bbox.urx = m_edge->maxx;
	  bbox.ury = m_edge->maxy;
	}

	if (!std::isnan(m_edge->xleftanchor)) {
		leftAnchor = QPoint(m_edge->xleftanchor, m_edge->yleftanchor);
	}
	if (!std::isnan(m_edge->xrightanchor)) {
		rightAnchor = QPoint(m_edge->xrightanchor, m_edge->yrightanchor);
	}

	//matrix = getMatrix(m_otLayout->mp, charcode);
	matrix = { m_edge->xpart,m_edge->ypart };
#ifndef DIGITALKHATT_WEBLIB
	if (name != "endofaya") {
		path = getPath(m_edge);
	}
	else {
		picture = getPicture(m_edge);
		path = getPath(m_edge);
	}
#endif
	if (copyPath) {
		isCopiedPath = true;
		this->copyEdgeBody();
	}
	else {
		isCopiedPath = false;
		this->copiedPath = m_edge != nullptr ? m_edge->body : nullptr;
	}

	
	for (int i = 0; i < m_edge->numAnchors; i++) {
		AnchorPoint anchor = m_edge->anchors[i];
		anchors.insert(anchor.anchorName, QPoint(anchor.x, anchor.y));
	}


}

bool GlyphVis::conatinsAnchor(QString name){
    return anchors.contains(name);
}

QPoint GlyphVis::getAnchor(QString name) {
	return anchors.value(name);

	/*
	if (value.isNull()) {
		for (int i = 0; i < m_edge->numAnchors; i++) {
			AnchorPoint anchor = m_edge->anchors[i];
			if (anchor.anchorName == name) {
				anchors[name] = QPoint(anchor.x, anchor.y);
				return anchors[name];
			}
		}
	}

	return value;*/
		
	
}

void GlyphVis::copyEdgeBody() {
	copiedPath = nullptr;

	mp_edge_object* h = edge();

	auto copypath = [](mp_gr_knot knot,OtLayout* layout)
	{
		mp_gr_knot p, current, ret;

		ret = nullptr;

		if (knot == nullptr) return ret;

		ret = (mp_gr_knot)mp_xmalloc(layout->mp, 1, sizeof(struct mp_gr_knot_data)); //new mp_gr_knot_data();

		ret->x_coord = knot->x_coord;
		ret->y_coord = knot->y_coord;
		ret->left_x = knot->left_x;
		ret->left_y = knot->left_y;
		ret->right_x = knot->right_x;
		ret->right_y = knot->right_y;
		ret->data.types.left_type = knot->data.types.left_type;
		ret->next = nullptr;

		current = ret;

		p = knot->next;
		while (p != knot) {

			
			mp_gr_knot tmp = (mp_gr_knot)mp_xmalloc(layout->mp, 1, sizeof(struct mp_gr_knot_data)); // new mp_gr_knot_data();

			tmp->left_x = p->left_x;
			tmp->left_y = p->left_y;
			tmp->x_coord = p->x_coord;
			tmp->y_coord = p->y_coord;
			tmp->right_x = p->right_x;
			tmp->right_y = p->right_y;
			tmp->data.types.left_type = p->data.types.left_type;

			

			current->next = tmp;
			current = tmp;

			p = p->next;
		}

		current->next = ret;

		return ret;
	};


	if (h) {


		mp_graphic_object* body = h->body;
		mp_graphic_object* currObject = nullptr;

		if (body) {
			do {
				switch (body->type)
				{
				case mp_fill_code: {

					mp_fill_object* fillobject = (mp_fill_object*)body;
					mp_gr_knot newpath = copypath(fillobject->path_p, m_otLayout);

					mp_fill_object* nextObject = (mp_fill_object * )mp_new_graphic_object(m_otLayout->mp, mp_fill_code); // new mp_fill_object;
					nextObject->type = mp_fill_code;
					nextObject->path_p = newpath;
					nextObject->next = nullptr;
					nextObject->pre_script = nullptr;
					nextObject->post_script = nullptr;
					nextObject->pen_p = nullptr;
					nextObject->htap_p = nullptr;

					if (fillobject->color_model == mp_rgb_model) {
						nextObject->color_model = mp_rgb_model;
						nextObject->color = fillobject->color;
					}

					if (currObject == nullptr) {
						currObject = (mp_graphic_object*)nextObject;
						copiedPath = currObject;
					}
					else {
						currObject->next = (mp_graphic_object*)nextObject;
						currObject = currObject->next;
					}


					break;
				}
				default:
					break;
				}

			} while (body = body->next);
		}
	}

}
#ifndef DIGITALKHATT_WEBLIB
QPainterPath GlyphVis::getPath(mp_edge_object* h) {

	QPainterPath localpath;

	localpath.setFillRule(Qt::WindingFill);


	if (h) {
		mp_graphic_object* body = h->body;


		if (body) {

			do {
				switch (body->type)
				{
				case mp_fill_code: {
					QPainterPath subpath = mp_dump_solved_path(((mp_fill_object*)body)->path_p);
					localpath.addPath(subpath);


					break;
				}
				default:
					break;
				}

			} while (body = body->next);
		}
	}

	return localpath;
}
QPicture GlyphVis::getPicture(mp_edge_object* h)
{

	QPicture pic;
	QPainter painter;

	painter.begin(&pic);

	if (h) {
		mp_graphic_object* body = h->body;


		if (body) {

			do {
				switch (body->type)
				{
				case mp_fill_code: {
					auto fillobject = (mp_fill_object*)body;
					QPainterPath subpath = mp_dump_solved_path(fillobject->path_p);
					if (fillobject->color_model == mp_rgb_model) {
						//painter.setBrush(QColor(fillobject->color.a_val, fillobject->color.b_val, fillobject->color.c_val));
						painter.fillPath(subpath, QColor(fillobject->color.a_val * 255, fillobject->color.b_val * 255, fillobject->color.c_val * 255));
					}
					else {
						int t = 5;
					}


					break;
				}
				default:
					break;
				}

			} while (body = body->next);
		}
	}

	painter.end();

	return pic;
}

QPainterPath GlyphVis::mp_dump_solved_path(mp_gr_knot h) {
	mp_gr_knot p, q;
	QPainterPath path;
	//path.setFillRule(Qt::OddEvenFill);
	if (h == NULL) return path;

	path.moveTo(h->x_coord, h->y_coord);
	p = h;
	do {
		q = p->next;
		path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

		p = q;
	} while (p != h);
	if (h->data.types.left_type != mp_endpoint)
		path.closeSubpath();

	return path;
}
#endif
