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

extern "C"
{
# include "mplib.h"
# include "mpmp.h"
#include "mplibps.h"
}

#include "hb-ot-layout-gsub-table.hh"
#undef max
#include "OtLayout.h"
#include "Lookup.h"
#include "Subtable.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonarray.h"
#include "hb-ot.h"
#include <qmap.h>
#include <qset.h>
//#include <QFile>
//#include <QTextStream>
//#include "QJSValueIterator"
#include <iostream>
#ifndef DIGITALKHATT_WEBLIB
#include <QDebug>
#endif
#include "automedina\automedina.h"
#include "QByteArrayOperator.h"
#include "GlyphVis.h"
#include "FeaParser\driver.h"
#include "FeaParser\feaast.h"
//#include "hb-ot-layout-gsubgpos.hh"

#include "qregularexpression.h"

#include "QuranText/quran.h"
#include <limits>
#include <QtCore\qmath.h>





int OtLayout::SCALEBY = 8;
double OtLayout::EMSCALE = 1;
int OtLayout::MINSPACEWIDTH = 0;
int OtLayout::SPACEWIDTH = 75;
int OtLayout::MAXSPACEWIDTH = 100;

QDataStream& operator<<(QDataStream& s, const QSet<quint16>& v)
{
	for (QSet<quint16>::const_iterator it = v.begin(); it != v.end(); ++it)
		s << *it;
	return s;
}

QDataStream& operator<<(QDataStream& s, const QSet<quint32>& v)
{
	for (QSet<quint32>::const_iterator it = v.begin(); it != v.end(); ++it)
		s << *it;
	return s;
}

QDataStream& operator<<(QDataStream& stream, const SuraLocation& location) {
	stream << location.name << location.pageNumber << location.x << location.y;
	return stream;
}
QDataStream& operator>>(QDataStream& stream, SuraLocation& location) {
	return stream >> location.name >> location.pageNumber >> location.x >> location.y;
}

static hb_blob_t* harfbuzzGetTables(hb_face_t* face, hb_tag_t tag, void* userData)
{
	OtLayout* layout = reinterpret_cast<OtLayout*>(userData);

	QByteArray data;

	switch (tag) {
	case HB_OT_TAG_GSUB:
		data = layout->getGSUB();
		break;
	case HB_OT_TAG_GPOS:
		data = layout->getGPOS();
		break;
	case HB_OT_TAG_GDEF:
		data = layout->getGDEF();
		break;
	}

	return hb_blob_create(data.constData(), data.size(), HB_MEMORY_MODE_READONLY, NULL, NULL);

}

static hb_bool_t
getNominalGlyph(hb_font_t* font,
	void* font_data,
	hb_codepoint_t unicode,
	hb_codepoint_t* glyph,
	void* user_data)
{

	*glyph = unicode;

	return true;

	OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

	if (unicode == 2291) {
		*glyph = layout->glyphCodePerName[QString("smallhighwaw")];
	}
	else {
		*glyph = unicode;
	}

	return true;



	for (auto& curr : layout->glyphs) {
		int charcode = curr.charcode;
		if (charcode == 2291) {
			*glyph = layout->glyphCodePerName[QString("smallhighwaw")];
			return true;
		}
		else if (charcode >= 0x0600 && charcode <= 0x06FF) {
			*glyph = unicode;
			return true;
		}
	}

	return false;
}

static hb_position_t floatToHarfBuzzPosition(double value)
{
	return static_cast<hb_position_t>(value * (1 << OtLayout::SCALEBY));
}

static hb_position_t getGlyphHorizontalAdvance(hb_font_t* hbFont, void* fontData, hb_codepoint_t glyph, double lefttatweel, double righttatweel, void* userData)
{
	OtLayout* layout = reinterpret_cast<OtLayout*>(fontData);

	if (!layout->glyphNamePerCode.contains(glyph)) return 0;

	if (layout->glyphGlobalClasses[glyph] == OtLayout::MarkGlyph) {
		return 0;
	}
	else {
		QString name = layout->glyphNamePerCode[glyph];

		GlyphVis* pglyph = &layout->glyphs[name];

		if (lefttatweel != 0 || righttatweel != 0) {
			GlyphParameters parameters{};

			parameters.lefttatweel = lefttatweel;
			parameters.righttatweel = righttatweel;

			pglyph = layout->getAlternate(pglyph->charcode, parameters);
		}


		double advance = pglyph->width;
		//return advance; // floatToHarfBuzzPosition(advance);
		int upem = 1000;
		int xscale, yscale;
		hb_font_get_scale(hbFont, &xscale, &yscale);
		int64_t scaled = advance * xscale;
		scaled += scaled >= 0 ? upem / 2 : -upem / 2; /* Round. */
		auto gg = (hb_position_t)(scaled / upem);
		return gg;
	}

}

static void get_glyph_h_advances(hb_font_t* font, void* font_data,
	unsigned count,
	const hb_codepoint_t* first_glyph,
	unsigned glyph_stride,
	hb_position_t* first_advance,
	unsigned advance_stride,
	void* user_data)
{
	//TODO:hacking
	auto glyphs = (hb_glyph_info_t*)(first_glyph);
	auto positions = (hb_glyph_position_t*)(first_advance);

	OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);


	for (unsigned int i = 0; i < count; i++)
	{
		//TEST
		/*
		QString name = layout->glyphNamePerCode[glyphs[i].codepoint];
		if (name == "behshape.init") {
			glyphs[i].lefttatweel = 12;
		}else if (name == "lam.medi") {
			glyphs[i].lefttatweel = 6;
			glyphs[i].righttatweel = 6;
		}*/

		positions[i].x_advance = getGlyphHorizontalAdvance(font, font_data, glyphs[i].codepoint, glyphs[i].lefttatweel, glyphs[i].righttatweel, user_data);
	}

}

static hb_bool_t get_cursive_anchor(hb_font_t* font, void* font_data,
	hb_cursive_anchor_context_t* context,
	hb_position_t* x,
	hb_position_t* y,
	void* user_data) {

	OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

	Lookup* lookupTable = layout->gposlookups.at(context->lookup_index);

	auto subtable = lookupTable->subtables.at(context->subtable_index);

	if (lookupTable->type == Lookup::cursive) {
		CursiveSubtable* subtableTable = static_cast<CursiveSubtable*>(subtable);

		if (context->type == hb_cursive_anchor_context_t::entry) {
			auto anchor = subtableTable->getEntry(context->glyph_id, context->lefttatweel, context->righttatweel);
			if (anchor) {

				*x = anchor->x();
				*y = anchor->y();

				return true;
			}
			else {
				*x = 0;
				*y = 0;
			}
		}
		else if (context->type == hb_cursive_anchor_context_t::exit) {
			auto anchor = subtableTable->getExit(context->glyph_id, context->lefttatweel, context->righttatweel);
			if (anchor) {

				*x = anchor->x();
				*y = anchor->y();

				return true;
			}
			else {
				*x = 0;
				*y = 0;
			}
		}

	}
	else if (lookupTable->type == Lookup::mark2base || lookupTable->type == Lookup::mark2mark) {

		MarkBaseSubtable* subtableTable = static_cast<MarkBaseSubtable*>(subtable);

		quint16 classIndex = subtableTable->markCodes[context->glyph_id];

		QString className = subtableTable->classNamebyIndex[classIndex];



		if (context->type == hb_cursive_anchor_context_t::base) {

			QString baseGlyphName = layout->glyphNamePerCode[context->base_glyph_id];

			GlyphVis& curr = layout->glyphs[baseGlyphName];

			auto anchor = subtableTable->getBaseAnchor(context->glyph_id, context->base_glyph_id, context->lefttatweel, context->righttatweel);
			if (anchor) {

				*x = anchor->x();
				*y = anchor->y();

				return true;
			}
			else {
				*x = 0;
				*y = 0;
			}

		}
		else if (context->type == hb_cursive_anchor_context_t::mark) {
			QString markGlyphName = layout->glyphNamePerCode[context->glyph_id];


			GlyphVis& curr = layout->glyphs[markGlyphName];

			auto anchor = subtableTable->getMarkAnchor(context->glyph_id, context->base_glyph_id, context->lefttatweel, context->righttatweel);
			if (anchor) {

				*x = anchor->x();
				*y = anchor->y();

				return true;
			}
			else {
				*x = 0;
				*y = 0;
			}


		}



	}

	return false;

}

static hb_bool_t get_substitution(hb_font_t* font, void* font_data,
	hb_substitution_context_t* context, void* user_data) {

	OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

	Lookup* lookupTable = layout->gsublookups.at(context->ot_context->lookup_index);

	if (lookupTable->name == "markexpansion.l1") {
		auto buffer = context->ot_context->buffer;

		auto& curr_info = buffer->cur();
		auto& prev_info = buffer->cur(-1);

		auto& curr_glyph = *layout->getGlyph(curr_info.codepoint);
		auto& prev_glyph = *layout->getGlyph(prev_info.codepoint);


		curr_info.lefttatweel = prev_info.lefttatweel / 2;

		/*


		auto& curr_glyph = *layout->getGlyph(curr_info.codepoint);
		auto& prev_glyph = *layout->getGlyph(prev_info.codepoint);

		if (prev_glyph.name == "noon.isol.expa" && prev_info.lefttatweel != 0) {
			curr_info.lefttatweel = prev_info.lefttatweel / 2;
		}*/


	}
	else if (lookupTable->name.startsWith("expa.")) {

		auto buffer = context->ot_context->buffer;

		auto& curr_info = buffer->cur();

		auto name = layout->glyphNamePerCode[curr_info.codepoint];

		JustificationContext::GlyphsToExtend.append(buffer->idx);
		JustificationContext::Substitutes.append(context->substitute);

		auto subtable = lookupTable->subtables.at(context->ot_context->subtable_index);

		if (lookupTable->type == Lookup::single) {
			SingleSubtable* subtableTable = static_cast<SingleSubtable*>(subtable);
			if (subtableTable->format == 10) {
				SingleSubtableWithExpansion* tatweelSubtable = static_cast<SingleSubtableWithExpansion*>(subtableTable);
				JustificationContext::Expansions.insert(buffer->idx, tatweelSubtable->expansion.value(curr_info.codepoint));
			}
		}




		return false;

	}
	else if (lookupTable->name.contains("test")) {

		auto buffer = context->ot_context->buffer;

		auto& curr_info = buffer->cur();

		auto name = layout->glyphNamePerCode[curr_info.codepoint];

		//JustificationContext::GlyphsToExtend.append(buffer->idx);

		if (name == "behshape.medi") {
			curr_info.lefttatweel = 3;
			curr_info.righttatweel = 2;
		}

	}

	return true;
}

static hb_font_funcs_t* getFontFunctions()
{
	static hb_font_funcs_t* harfbuzzCoreTextFontFuncs = 0;

	if (!harfbuzzCoreTextFontFuncs) {
		harfbuzzCoreTextFontFuncs = hb_font_funcs_create();
		hb_font_funcs_set_nominal_glyph_func(harfbuzzCoreTextFontFuncs, getNominalGlyph, NULL, NULL);
		//hb_font_funcs_set_glyph_h_advance_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalAdvance, 0, 0);
		hb_font_funcs_set_glyph_h_advances_func(harfbuzzCoreTextFontFuncs, get_glyph_h_advances, 0, 0);
		hb_font_funcs_set_cursive_anchor_func(harfbuzzCoreTextFontFuncs, get_cursive_anchor, 0, 0);
		hb_font_funcs_set_substitution_func(harfbuzzCoreTextFontFuncs, get_substitution, 0, 0);

		//hb_font_funcs_set_glyph_h_origin_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalOrigin, 0, 0);
		//hb_font_funcs_set_glyph_extents_func(harfbuzzCoreTextFontFuncs, getGlyphExtents, 0, 0);

		hb_font_funcs_make_immutable(harfbuzzCoreTextFontFuncs);

	}
	return harfbuzzCoreTextFontFuncs;
}

QPoint AnchorCalc::getAdjustment(Automedina& y, MarkBaseSubtable& subtable, GlyphVis* curr, QString className, QPoint adjust, double lefttatweel, double righttatweel, GlyphVis** poriginalglyph) {

	GlyphVis* originalglyph = curr;

	QPoint adjustoriginal;

	if (curr->name != "alternatechar" && (!curr->originalglyph.isEmpty() && (curr->charlt != 0 || curr->charrt != 0))) {
		originalglyph = &y.glyphs[curr->originalglyph];
		adjustoriginal = subtable.classes[className].baseparameters[curr->originalglyph];

		if (originalglyph != curr) {
			if (curr->leftAnchor) {
				double xshift = curr->matrix.xpart - originalglyph->matrix.xpart;
				double yshift = curr->matrix.ypart - originalglyph->matrix.ypart;

				adjustoriginal += QPoint(xshift, yshift);
			}
			else {
				//adjustoriginal += QPoint(curr->width - originalglyph->width, curr->height - originalglyph->height);
			}

		}
	}

	if (curr->name == "alternatechar") {
		originalglyph = &y.glyphs[curr->originalglyph];
		//adjustoriginal = subtable.classes[className].baseparameters[curr->originalglyph];
		if (curr->leftAnchor) {
			double xshift = curr->matrix.xpart - originalglyph->matrix.xpart;
			double yshift = curr->matrix.ypart - originalglyph->matrix.ypart;

			adjustoriginal += QPoint(xshift, yshift);
		}
		else {
			originalglyph = curr;
		}
	}

	*poriginalglyph = originalglyph;

	return adjustoriginal;

}

GlyphVis* OtLayout::getGlyph(QString name, double lefttatweel, double righttatweel) {
	GlyphVis* pglyph = &this->glyphs[name];

	if (lefttatweel != 0 || righttatweel != 0) {
		GlyphParameters parameters{};

		parameters.lefttatweel = lefttatweel;
		parameters.righttatweel = righttatweel;

		pglyph = getAlternate(pglyph->charcode, parameters);
	}

	return pglyph;
}

GlyphVis* OtLayout::getGlyph(int code, double lefttatweel, double righttatweel) {

	if (glyphNamePerCode.contains(code)) {
		if (lefttatweel != 0 || righttatweel != 0) {
			GlyphParameters parameters{};

			parameters.lefttatweel = lefttatweel;
			parameters.righttatweel = righttatweel;

			return getAlternate(code, parameters);
		}
		else {
			return getGlyph(glyphNamePerCode[code], lefttatweel, righttatweel);
		}
	}

	return nullptr;
}

GlyphVis* OtLayout::getGlyph(int code) {

	GlyphVis* curr = nullptr;

	if (glyphNamePerCode.contains(code)) {
		QString baseGlyphName = glyphNamePerCode[code];

		curr = &glyphs[baseGlyphName];
	}

	return curr;
}

QByteArray OtLayout::getGDEF() {
	if (!gdef_array.isEmpty() && !dirty) {
		return gdef_array;
	}

	gdef_array.clear();

	quint16 markGlyphSetsDefOffset = 0;
	quint16 glyphClassDefOffset = 14;

	quint16 glyphCount = glyphGlobalClasses.size();
	quint16 markGlyphSetCount = markGlyphSets.size();

	if (markGlyphSetCount > 0) {
		markGlyphSetsDefOffset = glyphClassDefOffset + 2 + 2 + glyphCount * 6;
	}

	gdef_array << (quint16)1 << (quint16)2 << glyphClassDefOffset << (quint16)0 << (quint16)0 << (quint16)0 << markGlyphSetsDefOffset;

	gdef_array << (quint16)2;

	gdef_array << glyphCount;

	for (auto i = glyphGlobalClasses.constBegin(); i != glyphGlobalClasses.constEnd(); ++i) {
		quint16 code = i.key();
		quint16 classValue = i.value();
		gdef_array << code;
		gdef_array << code;
		gdef_array << classValue;
	}

	QByteArray MarkGlyphSetsTable;
	QByteArray coverageTables;



	MarkGlyphSetsTable << (quint16)1 << markGlyphSetCount;

	quint32 CoverageOffsets = 2 + 2 + 4 * markGlyphSetCount;

	for (auto MarkGlyphSet : markGlyphSets) {
		MarkGlyphSetsTable << CoverageOffsets;
		std::sort(MarkGlyphSet.begin(), MarkGlyphSet.end());
		quint16 MarkGlyphSetCount = MarkGlyphSet.size();
		coverageTables << (quint16)1 << MarkGlyphSetCount << MarkGlyphSet;
		CoverageOffsets += 2 + 2 + 2 * MarkGlyphSetCount;
	}

	MarkGlyphSetsTable.append(coverageTables);

	gdef_array.append(MarkGlyphSetsTable);

	return gdef_array;

}
QByteArray OtLayout::getGSUB() {
	if (!gsub_array.isEmpty() && !dirty) {
		return gsub_array;
	}
	gsub_array = getGSUBorGPOS(true, gsublookups, allGsubFeatures, gsublookupsIndexByName);

	return gsub_array;
}
QByteArray OtLayout::getGPOS() {
	if (!gpos_array.isEmpty() && !dirty) {
		return gpos_array;
	}

	gpos_array = getGSUBorGPOS(false, gposlookups, allGposFeatures, gposlookupsIndexByName);

	tajweedcolorindex = gposlookupsIndexByName.value("green", 0xFFFF);

	return gpos_array;
}
QByteArray OtLayout::getFeatureList(QMap<QString, QSet<quint16>> allFeatures) {

	QByteArray featureList_array;
	QByteArray features_array;
	QDataStream featureList_stream(&featureList_array, QIODevice::WriteOnly);
	QDataStream features_stream(&features_array, QIODevice::WriteOnly);

	quint16 featureCount = allFeatures.size();
	quint16 beginoffset = 2 + 6 * featureCount;

	featureList_stream << featureCount;

	QMapIterator<QString, QSet<quint16>> it(allFeatures);
	while (it.hasNext()) {
		it.next();
		featureList_stream.writeRawData(it.key().toLatin1(), 4); // scriptTag
		featureList_stream << beginoffset;

		quint16 lookupIndexCount = it.value().count();

		features_stream << (quint16)0; //featureParams
		features_stream << lookupIndexCount;

		features_stream << it.value();

		/*
		QSetIterator<quint16> i(it.value());
		while (i.hasNext()) {
		features_stream << i.next();
		}*/

		beginoffset += 2 + 2 + 2 * lookupIndexCount;

	}

	featureList_array.append(features_array);

	return featureList_array;
}
QByteArray OtLayout::getScriptList(int featureCount) {
	// script list

	QByteArray scriptList_array;
	QDataStream scriptList_stream(&scriptList_array, QIODevice::WriteOnly);

	//ScriptList table
	scriptList_stream << (quint16)1; // scriptCount
	scriptList_stream.writeRawData("arab", 4); // scriptTag
	scriptList_stream << (quint16)8; // scriptOffset

									 // arab script table
	scriptList_stream << (quint16)0; // defaultLangSys
	scriptList_stream << (quint16)1; // langSysCount
	scriptList_stream.writeRawData("ARA ", 4); // langSysTag
	scriptList_stream << (quint16)10; // langSysOffset (2 + 2 + 4 + 2)

										// LangSys table

	scriptList_stream << (quint16)0; // lookupOrder
	scriptList_stream << (quint16)0xFFFF; // lookupOrder
	scriptList_stream << (quint16)featureCount; // featureIndexCount

	for (int i = 0; i < featureCount; i++) {
		scriptList_stream << (quint16)i;
	}

	return scriptList_array;
}

QByteArray OtLayout::getGSUBorGPOS(bool isgsub, QVector<Lookup*>& lookups, QMap<QString, QSet<quint16>>& allFeatures,
	QMap<QString, int>& lookupsIndexByName) {

	allFeatures.clear();
	lookupsIndexByName.clear();
	lookups.clear();

	for (auto lookup : this->lookups) {
		if (!disabledLookups.contains(lookup)) {
			if (isgsub == lookup->isGsubLookup()) {
				quint16 lookupIndex = lookups.size();

				lookupsIndexByName[lookup->name] = lookupIndex;
				lookups.append(lookup);
			}
		}
	}

	for (auto featureName : this->allFeatures.keys()) {
		for (auto lookup : this->allFeatures.value(featureName)) {
			int lookupIndex = lookupsIndexByName.value(lookup->name, -1);
			if (lookupIndex != -1) {
				allFeatures[featureName].insert(lookupIndex);
			}
		}
	}

	QByteArray scriptList = getScriptList(allFeatures.count());
	QByteArray featureList = getFeatureList(allFeatures);


	QByteArray root;

	quint16 scriptListOffset = 10;
	quint16 featureListOffset = scriptListOffset + scriptList.size();
	quint16 lookupListOffset = featureListOffset + featureList.size();

	root << (quint16)1 << (quint16)0 << scriptListOffset << featureListOffset << lookupListOffset;

	root.append(scriptList);
	root.append(featureList);



	quint16 lookupCount = lookups.size();

	quint32 lookupListtotalSize = 2 + 2 * lookupCount;
	for (int i = 0; i < lookups.size(); ++i) {

		Lookup* lookup = lookups.at(i);

		quint32 nb_subtables = lookup->subtables.size();

		//lookup header
		lookupListtotalSize += 2 + 2 + 2 + 2 * nb_subtables;

		if (lookup->markGlyphSetIndex != -1) {
			lookupListtotalSize += 2;
		}

		//extension subtables
		lookupListtotalSize += 8 * nb_subtables;

	}

	QByteArray lookupList;
	QByteArray lookups_array;
	lookupList << lookupCount;

	quint16 beginoffset = 2 + 2 * lookupCount;
	quint16 extensiontype = isgsub ? Lookup::extensiongsub : (Lookup::extensiongpos - 10);
	QByteArray subtablesArray;

	quint32 subtablesOffset = lookupListtotalSize;

	for (int i = 0; i < lookups.size(); ++i) {

		Lookup* lookup = lookups.at(i);

		quint32 nb_subtables = lookup->subtables.size();

		QByteArray lookupArray;

		lookupArray << extensiontype;
		lookupArray << lookup->flags;
		lookupArray << (quint16)nb_subtables;

		quint16 debutsequence = 2 + 2 + 2 + 2 * nb_subtables;

		if (lookup->markGlyphSetIndex != -1) {
			debutsequence += 2;
		}
		//QByteArray temp = lookup->getSubtables();
		QByteArray exttables_array;

		for (int i = 0; i < nb_subtables; ++i) {

			lookupArray << debutsequence;

			exttables_array << (quint16)1 << (quint16)(lookup->type % 10) << (quint32)(subtablesOffset - (beginoffset + debutsequence));

			QByteArray subtableArray = lookup->subtables.at(i)->getOptOpenTypeTable();


			if (subtableArray.size() > 0xFFFF) {
				throw std::runtime_error{ "Subtable exeeded the limit of 64K" };
			}

			// std::cout << lookup->name.toLatin1().data() << " --- " << lookup->subtables.at(i)->name.toLatin1().data() << " : " << subtableArray.size() << "\n";

			subtablesArray.append(subtableArray);

			subtablesOffset += subtableArray.size();

			debutsequence += 8;
		}

		if (lookup->markGlyphSetIndex != -1) {
			lookupArray << lookup->markGlyphSetIndex;
		}

		lookupArray.append(exttables_array);
		lookups_array.append(lookupArray);

		lookupList << beginoffset;

		beginoffset += lookupArray.size();

	}

	lookupList.append(lookups_array);
	lookupList.append(subtablesArray);


	root.append(lookupList);

	return root;

}
/*
QByteArray OtLayout::getGSUBorGPOS(bool isgsub) {

	QByteArray scriptList = getScriptList(isgsub);
	QByteArray featureList = getFeatureList(isgsub);


	QByteArray root;

	quint16 scriptListOffset = 10;
	quint16 featureListOffset = scriptListOffset + scriptList.size();
	quint16 lookupListOffset = featureListOffset + featureList.size();

	root << (quint16)1 << (quint16)0 << scriptListOffset << featureListOffset << lookupListOffset;

	root.append(scriptList);
	root.append(featureList);

	//lookupList


	QVector<Lookup*> lookups;

	if (isgsub) {
		lookups = gsublookups;
	}
	else {
		lookups = gposlookups;
	}

	quint16 lookupCount = lookups.size();

	QByteArray lookupList;
	QByteArray lookups_array;


	quint16 beginoffset = 2 + 2 * lookupCount;

	lookupList << lookupCount;

	for (int i = 0; i < lookups.size(); ++i) {

		QByteArray temp = lookups.at(i)->getOpenTypeTable();

		lookupList << beginoffset;
		lookups_array.append(temp);

		beginoffset += temp.size();
	}

	lookupList.append(lookups_array);


	root.append(lookupList);

	return root;

}*/

#if DIGITALKHATT_WEBLIB
OtLayout::OtLayout(MP mp) {
#else
OtLayout::OtLayout(MP mp, QObject * parent) :QObject(parent) {
#endif

	face = hb_face_create_for_tables(harfbuzzGetTables, this, 0);

	dirty = true;

	this->mp = mp;

	automedina = new Automedina(this, mp);

	nuqta();

	//TEST performance
	/*
	GlyphVis* originalglyph = &glyphs["kaf.fina.expa"];
	int num = 0;

	for (double ii = 0x0.00001p-1; ii <= 3; ii += 0x0.00001p-1) {

		GlyphParameters parameters{};

		parameters.lefttatweel = ii;
		parameters.righttatweel = 0;

		auto& test = originalglyph->getAlternate(parameters);
		num++;

	}*/


}
OtLayout::~OtLayout() {
	for (auto lookup : lookups) {
		delete lookup;
	}
	clearAlternates();

	delete face;
	delete automedina;
}

void OtLayout::clearAlternates() {
	for (auto& glyph : alternatePaths) {
		for (auto& path : glyph.second) {
			delete path.second;
		}
		//glyph.second.clear();
	}
	alternatePaths.clear();
}

AnchorCalc* OtLayout::getanchorCalcFunctions(QString functionName, Subtable * subtable) {
	return automedina->getanchorCalcFunctions(functionName, subtable);
}
/*
void OtLayout::prepareJSENgine() {

	evaluateImport();

	QJSValue descriptions = myEngine.newObject();
	myEngine.globalObject().setProperty("desc", descriptions);
	QJSValue classes = myEngine.globalObject().property("classes");
	QJSValue marksClass = classes.property("marks");
	QJSValue basesClass = myEngine.newObject();
	classes.setProperty("bases", basesClass);

	for (int i = 0; i < m_font->glyphs.length(); i++) {

		Glyph* curr = m_font->glyphs[i];

		Glyph::ComputedValues & values = curr->getComputedValues();

		QJSValue item = myEngine.newObject();

		item.setProperty("width", values.width);
		item.setProperty("height", values.height);
		item.setProperty("depth", values.depth);
		item.setProperty("charcode", values.charcode);
		QJSValue bbox = myEngine.newObject();

		bbox.setProperty("llx", values.bbox.llx);
		bbox.setProperty("lly", values.bbox.lly);
		bbox.setProperty("urx", values.bbox.urx);
		bbox.setProperty("ury", values.bbox.ury);

		item.setProperty("boundingbox", bbox);
		item.setProperty("name", curr->name());


		if (values.leftAnchor.has_value()) {
			QJSValue leftAnchor = myEngine.newObject();
			leftAnchor.setProperty("x", values.leftAnchor.value().x());
			leftAnchor.setProperty("y", values.leftAnchor.value().y());
			item.setProperty("leftanchor", leftAnchor);
		}

		if (values.rightAnchor.has_value()) {
			QJSValue rightAnchor = myEngine.newObject();
			rightAnchor.setProperty("x", values.rightAnchor.value().x());
			rightAnchor.setProperty("y", values.rightAnchor.value().y());
			item.setProperty("rightanchor", rightAnchor);
		}

		descriptions.setProperty(curr->name(), item);

		if (!marksClass.hasProperty(curr->name())) {
			basesClass.setProperty(curr->name(), true);
			glyphGlobalClasses[curr->name()] = BaseGlyph;
		}
		else {
			glyphGlobalClasses[curr->name()] = MarkGlyph;
		}
	}


	//QJSValueIterator it(myEngine.globalObject().property("desc"));
	//while (it.hasNext()) {
	//	it.next();
		//std::cout << it.name().toLatin1().data() << ": " << it.value().toString().toLatin1().data() << "\n";
		//ebug() << it.name() << ": " << it.value().toString();
	//}



}

void OtLayout::evaluateImport() {
	QString fileName = import;
	QFile scriptFile(fileName);
	if (!scriptFile.open(QIODevice::ReadOnly)) {
		return;
	}
	// handle error
	QTextStream stream(&scriptFile);
	QString contents = stream.readAll();
	scriptFile.close();
	QJSValue result = myEngine.evaluate(contents, fileName);

	if (result.isError()) {
		qDebug()
			<< "Uncaught exception at line"
			<< result.property("lineNumber").toInt()
			<< ":" << result.toString();
		//printf("Uncaught exception at line %d :\n%s\n", result.property("lineNumber").toInt(), result.toString().toLatin1().constData());
	}

}*/
void OtLayout::addLookup(Lookup * lookup) {
	if (lookup->type == Lookup::none) {
		throw "Lookup Type not defined";
	}

	if (lookup->name.isEmpty()) {
		throw "Lookup name not defined";
	}

	if (!lookup->feature.isEmpty()) {
		allFeatures[lookup->feature].insert(lookup);
	}

	quint16 lookupIndex = lookups.size();

	lookupsIndexByName[lookup->name] = lookupIndex;
	lookups.append(lookup);


}

void OtLayout::readJson(const QJsonObject & json)
{

	for (auto lookup : lookups) {
		delete lookup;
	}
	lookups.clear();
	lookupsIndexByName.clear();
	gsublookups.clear();
	gposlookups.clear();
	markGlyphSets.clear();
	allGposFeatures.clear();
	allGsubFeatures.clear();
	gsublookupsIndexByName.clear();
	gposlookupsIndexByName.clear();
	automedina->cachedClasstoUnicode.clear();
	allFeatures.clear();
	disabledLookups.clear();

	//import = json["import"].toString();

	QJsonObject lookupsObject = json["lookups"].toObject();

	feayy::FeaContext context{ this, &lookupsObject };

	if (json.contains("featurefile")) {
		QString featurefile = json["featurefile"].toString();
		if (!featurefile.isEmpty()) {
			feayy::Driver driver(context);
			if (!driver.parse_file(featurefile.toStdString())) {
				std::cout << "Error in parsing " << featurefile.toStdString() << endl;
			};
		}

	}

	//anchorCalcFunctions = automedina->anchorCalcFunctions;
	/*
	for (int gi = 0; gi < 2; gi++) {

		QString table = gi ? "gpos" : "gsub";

		QJsonArray lookupsjson = json[table].toArray();
		for (QJsonArray::iterator it = lookupsjson.begin(); it != lookupsjson.end(); it++) {

			QString lookupName = it->toString();

			QJsonObject jsonsubtable = lookupsObject[lookupName].toObject();

			if (jsonsubtable.isEmpty()) {

				Lookup* lookup = automedina->getLookup(lookupName);
				if (lookup)
					addLookup(lookup);
				else {
					context.useLookup(lookupName);
				}

			}
			else {
				QJsonObject lookupsObject = jsonsubtable["lookups"].toObject();
				for (int index = 0; index < lookupsObject.size(); ++index) {
					QString innerlookupName = lookupsObject.keys()[index];
					QJsonObject lookupObject = lookupsObject[innerlookupName].toObject();
					Lookup* lookup = new Lookup(this);
					lookup->readJson(lookupObject);
					lookup->name = lookupName + "." + innerlookupName;
					lookup->feature = "";
					addLookup(lookup);
				}

				Lookup* lookup = new Lookup(this);


				lookup->name = lookupName;
				lookup->feature = jsonsubtable["feature"].toString();

				lookup->readJson(jsonsubtable);

				addLookup(lookup);

			}
		}
	}*/

	context.populateFeatures();

	if (face != nullptr) {
		hb_face_destroy(face);
		face = nullptr;
	}

	/*
	for (auto iter = allFeatures.constBegin(); iter != allFeatures.constEnd(); ++iter) {
		std::cout << "feature " << iter.key().toStdString() << " { " << endl;
		for (auto lookup : iter.value()) {
			std::cout << '\t' << "lookup " << lookup->name.toStdString() << ";" << endl;
		}

		std::cout << "} " << iter.key().toStdString() << ";" << endl << endl;
	}*/
}
void OtLayout::parseCppJsonLookup(QString lookupName, const QJsonObject & lookups) {

	Lookup* newlookup = automedina->getLookup(lookupName);
	if (newlookup) {
		addLookup(newlookup);
	}
	else {
		QJsonObject jsonsubtable = lookups[lookupName].toObject();

		if (jsonsubtable.isEmpty())
			return;

		QJsonObject lookupsObject = jsonsubtable["lookups"].toObject();
		for (int index = 0; index < lookupsObject.size(); ++index) {
			QString innerlookupName = lookupsObject.keys()[index];
			QJsonObject lookupObject = lookupsObject[innerlookupName].toObject();
			Lookup* lookup = new Lookup(this);
			lookup->readJson(lookupObject);
			lookup->name = lookupName + "." + innerlookupName;
			lookup->feature = "";
			addLookup(lookup);
		}

		Lookup* lookup = new Lookup(this);


		lookup->name = lookupName;
		lookup->feature = jsonsubtable["feature"].toString();

		lookup->readJson(jsonsubtable);

		addLookup(lookup);

	}



}
void OtLayout::saveParameters(QJsonObject & json) const {
	for (auto lookup : lookups) {
		if (!lookup->isGsubLookup()) {
			QJsonObject lookupObject;
			lookup->saveParameters(lookupObject);
			if (!lookupObject.isEmpty()) {
				json[lookup->name] = lookupObject;
			}
		}

	}
}
void OtLayout::readParameters(const QJsonObject & json) {
	for (auto lookup : lookups) {
		if (!lookup->isGsubLookup()) {
			if (!json[lookup->name].toObject().isEmpty()) {
				lookup->readParameters(json[lookup->name].toObject());
			}
		}
	}
}
void OtLayout::addClass(QString name, QSet<QString> set) {
	if (automedina->classes.contains(name)) {
		//throw "Class " + name + " Already exists";
	}
	automedina->classes[name] = set;
}
hb_font_t* OtLayout::createFont(int emScale, bool newFace)
{
	int upem = 1000;

	if (newFace || face == nullptr) {
		if (face != nullptr) {
			hb_face_destroy(face);
			face = nullptr;
		}


		face = hb_face_create_for_tables(harfbuzzGetTables, this, 0);
		hb_face_set_upem(face, upem);
	}

	hb_font_t* font = hb_font_create(face);
	hb_font_set_funcs(font, getFontFunctions(), this, 0);
	hb_font_set_ppem(font, upem, upem);
	const int scale = emScale * upem; // // (1 << OtLayout::SCALEBY) * static_cast<int>(size);
	hb_font_set_scale(font, scale, scale);
	return font;
}


quint16 OtLayout::addMarkSet(QList<quint16> list) {

	quint16 index = markGlyphSets.size();

	markGlyphSets.append(list);

	return index;
}
quint16 OtLayout::addMarkSet(QVector<QString> list) {
	QList<quint16> codeList;
	for (auto glyphName : list) {
		if (quint16 glyphcode = glyphCodePerName.value(glyphName, 0)) {
			codeList.append(glyphcode);
		}
		else {
			std::cout << "addMarkSet : Glyph Name '" << glyphName.toStdString() << "' does not exist.\n";
		}
	}

	return addMarkSet(codeList);

}
QSet<quint16> OtLayout::classtoUnicode(QString className) {
	return automedina->classtoUnicode(className);
}

QSet<quint16> OtLayout::regexptoUnicode(QString regexp) {
	return automedina->regexptoUnicode(regexp);
}

QSet<QString> OtLayout::classtoGlyphName(QString className) {

	return automedina->classtoGlyphName(className);
	/*
	QSet<QString> names;

	QJSValue classes = myEngine.globalObject().property("classes");

	if (!classes.hasOwnProperty(className)) {
		Glyph* glyph = m_font->glyphperName[className];
		if (glyph) {
			names.insert(glyph->name());
		}
	}
	else {
		QJSValue classObject = classes.property(className);

		QJSValueIterator it(classObject);
		while (it.hasNext()) {
			it.next();
			names.unite(classtoGlyphName(it.name()));
		}
	}

	return names;*/
}

double OtLayout::nuqta() {
	if (_nuqta == -1) {
		_nuqta = getNumericVariable("nuqta");
	}

	return _nuqta;
}

double OtLayout::getNumericVariable(QString name) {
	double value;
	QString command("show " + name + ";");

	QByteArray commandBytes = command.toLatin1();
	mp->history = mp_spotless;
	int status = mp_execute(mp, (char*)commandBytes.constData(), commandBytes.size());
	mp_run_data* results = mp_rundata(mp);
	QString ret(results->term_out.data);
	ret = ret.trimmed();
	if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
		mp_finish(mp);
		throw "Could not get " + name + " !\n" + ret;
	}
	else {
		value = ret.mid(3).toDouble();
	}

	return value;
}

#ifndef DIGITALKHATT_WEBLIB
void OtLayout::setParameter(quint16 glyphCode, quint32 lookup, quint32 subtableIndex, quint16 markCode, quint16 baseCode, QPoint displacement, Qt::KeyboardModifiers modifiers) {

	bool shift = false;
	bool ctrl = false;
	bool alt = false;

	if (modifiers & Qt::ShiftModifier) {
		shift = true;
	}

	if (modifiers & Qt::ControlModifier) {
		ctrl = true;
	}

	if (modifiers & Qt::AltModifier) {
		alt = true;
	}

	Lookup* lookupTable = gposlookups.at(lookup);

	auto subtable = lookupTable->subtables.at(subtableIndex);

	if (lookupTable->type == Lookup::singleadjustment) {
		SingleAdjustmentSubtable* subtableTable = static_cast<SingleAdjustmentSubtable*>(subtable);
		QString glyphName = glyphNamePerCode[markCode];

		ValueRecord prev = subtableTable->parameters[markCode];

		ValueRecord newvalue{ (qint16)(prev.xPlacement + displacement.x()),(qint16)(prev.yPlacement + displacement.y()),prev.xAdvance,(qint16)0 };

		if (shift) {
			newvalue.xAdvance += displacement.x();
		}
		else if (alt) {
			newvalue.xAdvance -= displacement.x();
		}

		subtableTable->parameters[markCode] = newvalue;

		qDebug() << QString("Changing single adjust anchor %1.%2.%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue.xPlacement << newvalue.yPlacement << newvalue.xAdvance;

		subtableTable->isDirty = true;

		emit parameterChanged();
	}
	else if (lookupTable->type == Lookup::mark2base || lookupTable->type == Lookup::mark2mark) {

		MarkBaseSubtable* subtableTable = static_cast<MarkBaseSubtable*>(subtable);

		quint16 classIndex = subtableTable->markCodes[markCode];

		QString className = subtableTable->classNamebyIndex[classIndex];



		if (!shift) {

			QString baseGlyphName = glyphNamePerCode[baseCode];

			GlyphVis& curr = glyphs[baseGlyphName];

			if (ctrl && !curr.originalglyph.isEmpty() && (curr.charlt != 0 || curr.charrt != 0)) {
				baseGlyphName = curr.originalglyph;
			}


			QPoint prev = subtableTable->classes[className].baseparameters[baseGlyphName];

			QPoint newvalue = prev + displacement;

			subtableTable->classes[className].baseparameters[baseGlyphName] = newvalue;

			qDebug() << QString("Changing base anchor %1::%2::%3::%4 : (%5,%6)").arg(lookupTable->name, subtable->name, className, baseGlyphName, QString::number(newvalue.x()), QString::number(newvalue.y()));

		}
		else {
			QString markGlyphName = glyphNamePerCode[markCode];
			QPoint prev = subtableTable->classes[className].markparameters[markGlyphName];

			QPoint newvalue = prev - displacement;

			subtableTable->classes[className].markparameters[markGlyphName] = prev - displacement;

			qDebug() << QString("Changing mark anchor %1::%2::%3::%4 : (%5,%6)").arg(lookupTable->name, subtable->name, className, markGlyphName, QString::number(newvalue.x()), QString::number(newvalue.y()));
		}
		subtableTable->isDirty = true;



		//qDebug() << "prev : " << prev << "new" << subtableTable->classes[className].baseparameters[baseGlyphName];



		emit parameterChanged();

	}
	else if (lookupTable->type == Lookup::cursive) {
		CursiveSubtable* subtableTable = static_cast<CursiveSubtable*>(subtable);

		QString glyphName = glyphNamePerCode[glyphCode];

		QString baseGlyphName = glyphNamePerCode[baseCode];

		GlyphVis& curr = glyphs[glyphName];

		Lookup* lookup = subtableTable->getLookup();

		if (lookup->flags & Lookup::RightToLeft) {
			QPoint newvalue = subtableTable->exitParameters[glyphCode] + displacement;
			subtableTable->exitParameters[glyphCode] = newvalue;

			qDebug() << QString("Changing cursive exit anchor %1.%2.%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue;
		}
		else {
			if (!shift) {
				QPoint newvalue = subtableTable->entryParameters[glyphCode] - displacement;
				subtableTable->entryParameters[glyphCode] = newvalue;

				qDebug() << QString("Changing cursive entry anchor %1.%2.%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue;
			}
			else {
				QPoint newvalue = subtableTable->exitParameters[baseCode] + displacement;
				subtableTable->exitParameters[baseCode] = newvalue;

				qDebug() << QString("Changing cursive exit anchor %1.%2.%3 :").arg(lookupTable->name, subtable->name, baseGlyphName) << newvalue;
			}
		}

		subtableTable->isDirty = true;

		emit parameterChanged();


	}




}
#endif

void OtLayout::applyJustLookup(hb_buffer_t * buffer, bool& needgpos, double& diff, QString feature, hb_font_t * shapefont, double nuqta, int emScale) {

	if (!this->allGsubFeatures.contains(feature))
		return;

	const unsigned int table_index = 0u;
	buffer->reverse();

	uint glyph_count;

	hb_buffer_t* copy_buffer = nullptr;
	copy_buffer = hb_buffer_create();
	hb_buffer_append(copy_buffer, buffer, 0, -1);

	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(copy_buffer, &glyph_count);

	OT::hb_ot_apply_context_t c(table_index, shapefont, buffer);
	c.set_recurse_func(OT::SubstLookup::apply_recurse_func);
	auto list = this->allGsubFeatures[feature].toList();
	qSort(list);
	bool stretch = diff > 0;
	for (auto lookup_index : list) {
		if ((stretch && diff > 0) || (!stretch && diff < 0)) {
			c.set_lookup_index(lookup_index);
			c.set_lookup_mask(2);
			c.set_auto_zwj(1);
			c.set_auto_zwnj(1);

			needgpos = true;
			JustificationContext::clear();

			hb_ot_layout_substitute_lookup(&c,
				shapefont->face->table.GSUB->table->get_lookup(lookup_index),
				shapefont->face->table.GSUB->accels[lookup_index]);

			hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

			if (JustificationContext::GlyphsToExtend.count() != 0) {

				//double tatweel = diff / JustificationContext::GlyphsToExtend.count() / nuqta;

				QMap<int, GlyphExpansion > affectedIndexes;

				bool insideGroup = false;
				hb_position_t oldWidth = 0;
				hb_position_t newWidth = 0;
				QVector<int> group;

				for (int i = 0; i < JustificationContext::GlyphsToExtend.count(); i++) {

					int index = JustificationContext::GlyphsToExtend[i]; // glyph_count - 1 - JustificationContext::GlyphsToExtend[i];
					GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[JustificationContext::Substitutes[i]]];

					GlyphExpansion& expa = JustificationContext::Expansions[index];

					group.append(i);
					oldWidth += glyph_pos[index].x_advance;
					if (glyph_info[index].codepoint == JustificationContext::Substitutes[i]) {
						newWidth += glyph_pos[index].x_advance;
					}
					else {
						double leftTatweel = glyph_pos[index].lefttatweel + expa.MinLeftTatweel > 0 ? expa.MinLeftTatweel : 0;
						double rightTatweel = glyph_pos[index].righttatweel + expa.MinRightTatweel > 0 ? expa.MinRightTatweel : 0;
						newWidth += getGlyphHorizontalAdvance(shapefont, this, JustificationContext::Substitutes[i], leftTatweel, rightTatweel, nullptr); // substitute.width* emScale;
					}


					if (expa.startEndLig == StartEndLig::Start) {
						insideGroup = true;
						continue;
					}
					else if (insideGroup && expa.startEndLig != StartEndLig::End) {
						continue;
					}

					auto expansion = newWidth - oldWidth;

					if ((stretch && expansion <= diff && expansion >= 0) || (!stretch && expansion >= diff && expansion <= 0)) {

						diff -= expansion;

						for (int i : group) {

							int index = JustificationContext::GlyphsToExtend[i];
							GlyphExpansion& expa = JustificationContext::Expansions[index];

							if (glyph_info[index].codepoint == JustificationContext::Substitutes[i]) {

								expa.MaxLeftTatweel = expa.MaxLeftTatweel - glyph_info[index].lefttatweel;
								expa.MaxRightTatweel = expa.MaxRightTatweel - glyph_info[index].righttatweel;

								if (stretch && expa.MaxLeftTatweel <= 0 && expa.MaxRightTatweel <= 0
									|| !stretch && expa.MinLeftTatweel >= 0 && expa.MinRightTatweel >= 0)
									continue;

								affectedIndexes.insert(i, expa);
							}
							else {
								//GlyphVis& glyph = this->glyphs[this->glyphNamePerCode[glyph_info[index].codepoint]];
								//GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[JustificationContext::Substitutes[i]]];

								//auto minStretch = (substitute.width - glyph.width) * emScale; // +JustificationContext::Expansions[index].MinLeftTatweel * nuqta;					

								glyph_info[index].codepoint = JustificationContext::Substitutes[i];
								if (expa.MinLeftTatweel > 0) {
									glyph_info[index].lefttatweel += expa.MinLeftTatweel;
									expa.MinLeftTatweel = 0;
								}

								if (expa.MinRightTatweel > 0) {
									glyph_info[index].righttatweel += expa.MinRightTatweel;
									expa.MinRightTatweel = 0;
								}

								if (stretch) {

									if (expa.MaxLeftTatweel > 0 || expa.MaxRightTatweel > 0) {
										affectedIndexes.insert(i, expa);
									}
								}
								else if (!stretch) {

									if (expa.MinLeftTatweel < 0 || expa.MinRightTatweel < 0) {
										affectedIndexes.insert(i, expa);
									}
								}
							}
						}

					}

					insideGroup = false;
					oldWidth = 0.0;
					newWidth = 0.0;
					group.clear();
				}

				while (affectedIndexes.size() != 0) {
					double meanTatweel = diff / (affectedIndexes.size());

					if (meanTatweel == 0.0) break;

					QMap<int, GlyphExpansion >::iterator i;
					QMap<int, GlyphExpansion > newaffectedIndexes;
					for (i = affectedIndexes.begin(); i != affectedIndexes.end(); ++i) {

						int index = JustificationContext::GlyphsToExtend[i.key()]; // glyph_count - 1 - JustificationContext::GlyphsToExtend[i];

						auto expa = i.value();

						if (stretch) {
							expa.MaxLeftTatweel = expa.MaxLeftTatweel > 0 ? expa.MaxLeftTatweel : 0;
							expa.MaxRightTatweel = expa.MaxRightTatweel > 0 ? expa.MaxRightTatweel : 0;

							auto MaxTatweel = expa.MaxLeftTatweel + expa.MaxRightTatweel;
							auto maxStretch = MaxTatweel * nuqta;


							auto tatweel = meanTatweel;

							if (meanTatweel > maxStretch) {
								tatweel = maxStretch;
							}

							diff -= tatweel;

							double leftTatweel = (tatweel * (expa.MaxLeftTatweel / MaxTatweel)) / nuqta;
							double rightTatweel = (tatweel * (expa.MaxRightTatweel / MaxTatweel)) / nuqta;

							glyph_info[index].lefttatweel += leftTatweel;
							glyph_info[index].righttatweel += rightTatweel;

							if (meanTatweel < maxStretch && diff > 0) {
								expa.MaxLeftTatweel -= leftTatweel;
								expa.MaxRightTatweel -= rightTatweel;
								newaffectedIndexes.insert(i.key(), expa);
							}

						}
						else {
							expa.MinLeftTatweel = expa.MinLeftTatweel < 0 ? expa.MinLeftTatweel : 0;
							expa.MinRightTatweel = expa.MinRightTatweel < 0 ? expa.MinRightTatweel : 0;

							auto MinTatweel = expa.MinLeftTatweel + expa.MinRightTatweel;
							auto minShrink = MinTatweel * nuqta;


							auto tatweel = meanTatweel;

							if (meanTatweel < minShrink) {
								tatweel = minShrink;
							}

							diff -= tatweel;

							double leftTatweel = (tatweel * (expa.MinLeftTatweel / MinTatweel)) / nuqta;
							double rightTatweel = (tatweel * (expa.MinRightTatweel / MinTatweel)) / nuqta;

							glyph_info[index].lefttatweel += leftTatweel;
							glyph_info[index].righttatweel += rightTatweel;

							if (meanTatweel > minShrink&& diff < 0) {
								expa.MinLeftTatweel -= leftTatweel;
								expa.MinRightTatweel -= rightTatweel;
								newaffectedIndexes.insert(i.key(), expa);
							}
						}
					}
					affectedIndexes = newaffectedIndexes;
				}
			}
		}
		else {
			continue;
		}
	}
	buffer->reverse();
	if (copy_buffer)
		hb_buffer_destroy(copy_buffer);

}

QList<LineLayoutInfo> OtLayout::justifyPage(int emScale, int lineWidth, int pageWidth, QStringList lines, LineJustification justification, bool newFace, bool tajweedColor) {

	QList<LineLayoutInfo> page;

	hb_buffer_t* buffer = buffer = hb_buffer_create();
	hb_font_t* shapefont = this->createFont(emScale, newFace);


	const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
	const int  defaultSpace = OtLayout::SPACEWIDTH * emScale;
	double nuqta = this->nuqta() * emScale;

	int currentyPos = TopSpace << OtLayout::SCALEBY;

	/*
	hb_feature_t features[] = {
		{HB_TAG('c', 'c', 'm', 'p'),0,0,(uint)-1},
		{HB_TAG('i', 'n', 'i', 't'),0,0,(uint)-1},
		{HB_TAG('m', 'e', 'd', 'i'),0,0,(uint)-1},
		{HB_TAG('f', 'i', 'n', 'a'),0,0,(uint)-1},
		{HB_TAG('r', 'l', 'i', 'g'),0,0,(uint)-1},
		{HB_TAG('l', 'i', 'g', 'a'),0,0,(uint)-1},
		{HB_TAG('c', 'a', 'l', 't'),0,0,(uint)-1},
		{HB_TAG('c', 'u', 'r', 's'),0,0,(uint)-1},
		{HB_TAG('k', 'e', 'r', 'n'),0,0,(uint)-1},
		{HB_TAG('m', 'a', 'r', 'k'),0,0,(uint)-1},
		{HB_TAG('m', 'k', 'm', 'k'),0,0,(uint)-1},
		{HB_TAG('s', 'c', 'h', '1'),0,0,(uint)-1},
	};*/

	//int num_features = sizeof(features) / sizeof(*features);

	hb_segment_properties_t savedprops;
	savedprops.direction = HB_DIRECTION_RTL;
	savedprops.script = HB_SCRIPT_ARABIC;
	savedprops.language = hb_language_from_string("ar", strlen("ar"));
	savedprops.reserved1 = 0;
	savedprops.reserved2 = 0;

	hb_feature_t color_fea{ HB_TAG('r', 'c', 'l', 't'),0,0,(uint)-1 };
	if (tajweedColor) {
		color_fea.value = 1;
	}

	hb_feature_t gpos_features[] = {
		{HB_TAG('i', 'n', 'i', 't'),0,0,(uint)-1},
		{HB_TAG('m', 'e', 'd', 'i'),0,0,(uint)-1},
		{HB_TAG('f', 'i', 'n', 'a'),0,0,(uint)-1},
		{HB_TAG('r', 'l', 'i', 'g'),0,0,(uint)-1},
		{HB_TAG('l', 'i', 'g', 'a'),0,0,(uint)-1},
		{HB_TAG('c', 'a', 'l', 't'),0,0,(uint)-1},
		{HB_TAG('s', 'c', 'h', 'm'),1,0,(uint)-1},
		color_fea
	};

	int num_gpos_features = sizeof(gpos_features) / sizeof(*gpos_features);





	auto initializeBuffer = [&](hb_buffer_t* buffer, hb_segment_properties_t* savedprops, QString& line)
	{
		hb_buffer_clear_contents(buffer);
		hb_buffer_set_segment_properties(buffer, savedprops);
		hb_buffer_add_utf16(buffer, line.utf16(), -1, 0, -1);
	};

	for (auto& line : lines) {

		initializeBuffer(buffer, &savedprops, line);
		//hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
		// hb_buffer_set_message_func(buffer, setMessage, this, NULL);			

		uint glyph_count;

		/*
		hb_shape_plan_t* shape_plan = hb_shape_plan_create_cached2(shapefont->face, &buffer->props, nullptr, 0, shapefont->coords, shapefont->num_coords, nullptr);

		hb_bool_t res = hb_shape_plan_execute(shape_plan, shapefont, buffer, nullptr, 0);
		hb_shape_plan_destroy(shape_plan);
		if (res)
			buffer->content_type = HB_BUFFER_CONTENT_TYPE_GLYPHS;*/

		hb_shape(shapefont, buffer, &color_fea, 1);

		if (applyJustification && lineWidth != 0) {
			JustificationInProgress = true;
			bool continueJustification = true;
			bool schr1applied = false;
			while (continueJustification) {

				continueJustification = false;

				hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
				hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
				QVector<quint32> spaces;
				int currentlineWidth = 0;
				int lastGlyph;


				for (int i = glyph_count - 1; i >= 0; i--) {
					//auto name = this->glyphs[this->glyphNamePerCode[glyph_info[i].codepoint]];
					if (glyph_info[i].codepoint == 32) {
						glyph_pos[i].x_advance = minSpace;
						spaces.append(i);
					}
					else {
						currentlineWidth += glyph_pos[i].x_advance;
						if (this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::BaseGlyph || this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::LigatureGlyph) {
							lastGlyph = glyph_info[i].codepoint;
						}

					}
				}

				GlyphVis& llglyph = this->glyphs[this->glyphNamePerCode[lastGlyph]];

				if (!llglyph.isAyaNumber()) {
					currentlineWidth = currentlineWidth - this->glyphs[this->glyphNamePerCode[lastGlyph]].bbox.llx;
				}


				double diff = (double)lineWidth - currentlineWidth - spaces.size() * defaultSpace;

				bool needgpos = false;
				if (diff > 0) {
					applyJustLookup(buffer, needgpos, diff, "sch1", shapefont, nuqta, emScale);
				}
				//shrink
				else {

					if (!schr1applied) {
						hb_feature_t festures2[2];
						festures2[0].tag = HB_TAG('s', 'h', 'r', '1');
						festures2[0].value = 1;
						festures2[0].start = 0;
						festures2[0].end = -1;

						festures2[1] = color_fea;


						//buffer->reverse();
						initializeBuffer(buffer, &savedprops, line);

						hb_shape(shapefont, buffer, festures2, 2);

						continueJustification = true;
						schr1applied = true;
					}
					else {
						applyJustLookup(buffer, needgpos, diff, "shr2", shapefont, nuqta, emScale);
					}
				}

				if (needgpos) {
					buffer->reverse();
					hb_shape(shapefont, buffer, gpos_features, num_gpos_features);
					//hb_shape(shapefont, buffer, nullptr, 0);
				}
			}
			JustificationInProgress = false;
		}


		hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
		hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
		QVector<quint32> spaces;
		int currentlineWidth = 0;

		int lastGlyph;
		LineLayoutInfo lineLayout;

		for (int i = glyph_count - 1; i >= 0; i--) {

			GlyphLayoutInfo glyphLayout;

			//if (glyph_info[i].lefttatweel != 0) {
			//	int t = 0;
			//}

			glyphLayout.codepoint = glyph_info[i].codepoint;
			glyphLayout.lefttatweel = glyph_info[i].lefttatweel;
			glyphLayout.righttatweel = glyph_info[i].righttatweel;
			glyphLayout.cluster = glyph_info[i].cluster;
			glyphLayout.x_advance = glyph_pos[i].x_advance;
			glyphLayout.y_advance = glyph_pos[i].y_advance;
			glyphLayout.x_offset = glyph_pos[i].x_offset;
			glyphLayout.y_offset = glyph_pos[i].y_offset;
			glyphLayout.lookup_index = glyph_pos[i].lookup_index;
			glyphLayout.color = glyph_pos[i].lookup_index >= this->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
			glyphLayout.subtable_index = glyph_pos[i].subtable_index;
			glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;

			glyphLayout.beginsajda = false;
			glyphLayout.endsajda = false;

			if (glyphLayout.codepoint == 32) {
				if (lineWidth != 0) {
					glyphLayout.x_advance = minSpace;
					spaces.append(lineLayout.glyphs.size());
				}
				else {
					glyphLayout.x_advance = defaultSpace;
					currentlineWidth += glyphLayout.x_advance;
				}
							
			}
			else {
				currentlineWidth += glyphLayout.x_advance;
				if (this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::BaseGlyph || this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::LigatureGlyph) {
					lastGlyph = glyph_info[i].codepoint;
				}
			}

			lineLayout.glyphs.push_back(glyphLayout);

		}

		GlyphVis& llglyph = this->glyphs[this->glyphNamePerCode[lastGlyph]];

		if (!llglyph.isAyaNumber()) {
			currentlineWidth = currentlineWidth - llglyph.bbox.llx;
		}



		lineLayout.underfull = 0;

		double spaceaverage = 0;
		if (lineWidth != 0) {
			
			if (spaces.size() != 0) {
				spaceaverage = (lineWidth - currentlineWidth) / spaces.size();
			}

			if (spaceaverage > minSpace) {
				for (auto index : spaces) {
					lineLayout.glyphs[index].x_advance = spaceaverage;
				}
			}
			else {
				lineLayout.underfull = (minSpace - spaceaverage) * spaces.size();
				spaceaverage = minSpace;
			}
		}

		


		if (justification == LineJustification::Distribute) {
			lineLayout.xstartposition = 0;
		}
		else {
			//lineLayout.xstartposition = -(lineWidth - (currentlineWidth + spaces.size() * defaultSpace)) / 2;
			lineLayout.xstartposition = (pageWidth - (currentlineWidth + spaces.size() * spaceaverage)) / 2;
			//lineLayout.xstartposition = 0;
		}

		lineLayout.ystartposition = currentyPos;

		page.append(lineLayout);

		currentyPos = currentyPos + (InterLineSpacing << OtLayout::SCALEBY);
	}

	hb_font_destroy(shapefont);
	hb_buffer_destroy(buffer);

	return page;

}

LayoutPages OtLayout::pageBreak(int emScale, int lineWidth, bool pageFinishbyaVerse) {


	bool isQurancomplex = false;

	typedef double ParaWidth;

	struct Candidate {
		size_t index;  // index int the text buffer
		int prev;  // index to previous break		
		ParaWidth totalWidth;  // width of text until this point, if we decide to break here		
		double  totalDemerits;  // best demerits found for this break (index) and lineNumber
		size_t lineNumber;  // only updated for non-constant line widths
		size_t pageNumber;
		size_t totalSpaces;  // preceding space count after breaking		
	};


	const double DEMERITS_INFTY = std::numeric_limits<double>::max();

	hb_buffer_t* buffer = buffer = hb_buffer_create();

	hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
	hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
	hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));
	//hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

	hb_font_t* font = this->createFont(emScale);

	QString quran;

	for (int i = 2; i < 604; i++) {
		//for (int i = 581; i < 604; i++) {
			//for (int i = 1; i < 2; i++) {
				//const char * text = qurantext[i];
		const char* tt;

		if (isQurancomplex) {
			tt = quranComplex[i] + 1;
		}
		else {
			tt = qurantext[i] + 1;
		}

		//unsigned int text_len = strlen(tt);

		quran.append(quran.fromUtf8(tt));

		//hb_buffer_add_utf8(buffer, tt, text_len, 0, text_len);

	}

	quran = quran.replace(QRegularExpression("\\s*" + QString("۞") + "\\s*"), QString("۞") + " ");

	QSet<int> lineBreaks;
	QSet<int> suraLines;
	QSet<int> bismLines;

	QString suraWord = "سُورَةُ";
	QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

	QString surapattern = "^("
		+ suraWord + " .*|"
		+ bism
		+ "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
		+ ")$";

	QList<QString> suraNames;

	QRegularExpression re(surapattern, QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator i = re.globalMatch(quran);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		int startOffset = match.capturedStart(); // startOffset == 6
		int endOffset = match.capturedEnd(); // endOffset == 9
		lineBreaks.insert(endOffset);
		lineBreaks.insert(startOffset - 1);

		if (match.captured(0).startsWith("سُ")) {
			suraLines.insert(startOffset);
			suraNames.append(match.captured(0));
		}
		else {
			bismLines.insert(startOffset);
		}
	}

	quran = quran.replace(10, 32);
	quran = quran.replace(bism + char(32), bism + char(10));


	//Mark sajda rules
	QSet<int> beginsajdas;
	QSet<int> endsajdas;



	//QString gg = //"يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا|يَسْجُدُ لَهُۥ|وَخَرَّ رَاكِعٗا|أَلَّا يَسْجُدُوا۟ لِلَّهِ|وَٱسْجُدُوا۟ لِلَّهِ|فَٱسْجُدُوا۟ لِلَّهِ|يَسْجُدُونَ|وَلِلَّهِ يَسْجُدُ|خَرُّوا۟ سُجَّدٗا";
	//QString sajdapatterns = QString("(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدٗا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعٗا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)");
	QString sajdapatterns = "(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)"; // sajdapatterns.replace("\u0657", "\u08F0").replace("\u065E", "\u08F1").replace("\u0656", "\u08F2");
	re = QRegularExpression(sajdapatterns, QRegularExpression::MultilineOption);
	i = re.globalMatch(quran);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
		int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9
		QString c0 = match.captured(0);
		QString captured = match.captured(match.lastCapturedIndex());

		//int tt = match.lastCapturedIndex();


		beginsajdas.insert(startOffset);

		while (this->glyphGlobalClasses[quran[endOffset].unicode()] == OtLayout::MarkGlyph)
			endOffset--;

		endsajdas.insert(endOffset);
	}

	hb_buffer_add_utf16(buffer, quran.utf16(), -1, 0, -1);

	hb_shape(font, buffer, NULL, 0);

	uint glyph_count;

	const int minSpaceWidth = 50 * emScale;
	const int spaceWidth = 75 * emScale;
	const int maxSpaceWidth = 100 * emScale;

	//ParaWidth lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);



	ParaWidth totalWidth = 0;
	int totalSpaces = 0;
	std::vector<int> actives;
	std::vector<Candidate> candidates;

	candidates.push_back({});
	Candidate* initCand = &candidates.back();

	initCand->pageNumber = 1;
	initCand->index = glyph_count;
	initCand->prev = -1;

	actives.push_back(0);

	for (int i = glyph_count - 1; i >= 0; i--) {

		if (glyph_info[i].codepoint != 10 && glyph_info[i].codepoint != 0x20) {
			totalWidth += glyph_pos[i].x_advance;
			continue;
		}

		totalSpaces++;

		QHash<int, int> potcandidates;

		auto activeit = actives.begin();
		while (activeit != actives.end()) {

			auto active = &candidates.at(*activeit);

			// must terminate a page with end of aya
			// A page has always 15 lines
			if (pageFinishbyaVerse) {
				if (active->lineNumber == 14 && !(glyph_info[i + 1].codepoint >= Automedina::AyaNumberCode && glyph_info[i + 1].codepoint <= Automedina::AyaNumberCode + 286)) {
					activeit++;
					continue;
				}
			}

			// calculate adjustment ratio
			double adjRatio = 0.0;
			int spaces = totalSpaces - active->totalSpaces;
			ParaWidth width = (totalWidth - active->totalWidth) + spaces * spaceWidth;
			if (width < lineWidth) {
				adjRatio = (lineWidth - width) / (spaces * maxSpaceWidth);
			}
			else if (width > lineWidth) {
				adjRatio = (lineWidth - width) / (spaces * minSpaceWidth);
			}

			if (adjRatio < -1) {
				activeit = actives.erase(activeit);
				continue;
			}

			double demerits = (1 + 100 * std::pow(std::abs(adjRatio), 3));

			double totalDemerits = active->totalDemerits + demerits;

			/*
			qDebug() << "index : " << active->index
				<< ", lineNumber : " << active->lineNumber
				<< ", pageNumber : " << active->pageNumber
				<< ", totalSpaces : " << active->totalSpaces
				<< ", totalWidth : " << active->totalWidth
				<< ",active->totalDemerits : " << active->totalDemerits
				<< ",demerits : " << demerits
				<< ",totalDemerits : " << totalDemerits
				<< ",currentIndex : " << i;*/

			int lineNumber = active->lineNumber + 1;
			int pageNumber = active->pageNumber;

			if (lineNumber == 16) {
				lineNumber = 1;
				pageNumber = pageNumber + 1;
			}

			//int key = (pageNumber - 1)* 15 + lineNumber;
			int key = lineNumber;

			if (potcandidates.contains(key)) {
				auto& candidate = candidates.at(potcandidates[key]);
				if (candidate.pageNumber != pageNumber && candidate.totalDemerits > totalDemerits) {
					throw "Should not";
				}
			}
			

			if (!potcandidates.contains(key) || candidates.at(potcandidates[key]).totalDemerits > totalDemerits) {


				potcandidates[key] = candidates.size();

				candidates.push_back({});
				Candidate* cand = &candidates.back();


				cand->lineNumber = lineNumber;
				cand->pageNumber = pageNumber;

				cand->index = i;
				cand->totalSpaces = totalSpaces;
				cand->totalWidth = totalWidth;
				cand->totalDemerits = totalDemerits;
				cand->prev = *activeit;


			}

			activeit++;
		}

		if (lineBreaks.contains(glyph_info[i].cluster)) {
			actives.clear();
		}

		for (auto cand : potcandidates) {
			actives.push_back(cand);
		}

	}

	Candidate* bestCandidate = nullptr;

	double best = DEMERITS_INFTY;
	for (auto activenum : actives) {
		auto active = &candidates.at(activenum);
		//qDebug() << "index : " << active->index << ", lineNumber : " << active->lineNumber << "Total demerits : " << active->totalDemerits;		
		if (active->index == 0 && active->totalDemerits < best && active->lineNumber == 15) {
			best = active->totalDemerits;
			bestCandidate = active;
		}

	}

	if (bestCandidate == nullptr) {
		//QMessageBox msgBox;
		//msgBox.setText("No feasable solution. Try to change the scale.");
		//msgBox.exec();
		return {};
	}

	QList<LineLayoutInfo> currentPage;
	QList<QList<LineLayoutInfo>> pages;
	QStringList originalPage;
	QList<QStringList> originalPages;
	QList<QString> suraNamebyPage;


	auto cand = bestCandidate;
	int currentpageNumber = bestCandidate->pageNumber;

	int nbbeginsajda = 0;
	int nbendsajda = 0;

	int lastLinePos = (OtLayout::TopSpace + OtLayout::InterLineSpacing * 14) << OtLayout::SCALEBY;

	int currentyPos = lastLinePos;

	int suraIndex = suraNames.length();
	QString currentSuraName;
	QString firstSuraInCurrentage;


	while (cand->prev != -1) {

		auto prev = &candidates.at(cand->prev);

		int beginIndex = prev->index - 1;
		int endIndex = cand->index + 1;
		int totalSpaces = cand->totalSpaces - prev->totalSpaces - 1;
		int totalWidth = cand->totalWidth - prev->totalWidth;

		int spaceaverage = (lineWidth - totalWidth) / totalSpaces;
		if (spaceaverage < minSpaceWidth) {
			spaceaverage = minSpaceWidth;
		}

		LineLayoutInfo lineLayout;

		lineLayout.type = LineType::Line;

		int currentxPos = 0;

		if (cand->pageNumber != currentpageNumber) {
			if (!firstSuraInCurrentage.isEmpty()) {
				suraNamebyPage.prepend(firstSuraInCurrentage);

				if (suraIndex - 1 >= 0) {
					currentSuraName = suraNames[suraIndex - 1];
				}
				else {
					currentSuraName = "سُورَةُ البَقَرَةِ";
				}

			}
			else {
				suraNamebyPage.prepend(currentSuraName);
			}

			firstSuraInCurrentage = "";
		}



		if (suraLines.contains(glyph_info[beginIndex].cluster)) {
			spaceaverage = (int)spaceWidth;
			currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
			lineLayout.type = LineType::Sura;

			firstSuraInCurrentage = suraNames[--suraIndex];
		}
		else if (bismLines.contains(glyph_info[beginIndex].cluster)) {
			spaceaverage = (int)spaceWidth;
			currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
			lineLayout.type = LineType::Bism;
		}

		spaceaverage = spaceaverage;
		currentxPos = currentxPos;

		QString originalLine;

		int currentcluster = glyph_info[beginIndex].cluster;
		int currentnewcluster = 0;

		for (int i = beginIndex; i >= endIndex; i--) {

			GlyphLayoutInfo glyphLayout;


			QString glyphName = this->glyphNamePerCode[glyph_info[i].codepoint];

			if (glyph_info[i].cluster != currentcluster) {
				int clusternb = glyph_info[i].cluster - currentcluster;
				originalLine.append(quran.mid(currentcluster, clusternb));
				currentcluster = glyph_info[i].cluster;
				currentnewcluster += clusternb;
			}

			glyphLayout.codepoint = glyph_info[i].codepoint;
			glyphLayout.cluster = currentnewcluster; // glyph_info[i].cluster;
			glyphLayout.x_advance = glyph_pos[i].x_advance;
			glyphLayout.y_advance = glyph_pos[i].y_advance;
			glyphLayout.x_offset = glyph_pos[i].x_offset;
			glyphLayout.y_offset = glyph_pos[i].y_offset;
			glyphLayout.lookup_index = glyph_pos[i].lookup_index;
			glyphLayout.color = glyph_pos[i].lookup_index >= this->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
			glyphLayout.subtable_index = glyph_pos[i].subtable_index;
			glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;
			//Todo Optimize 
			glyphLayout.beginsajda = false;
			glyphLayout.endsajda = false;

			if (beginsajdas.contains(glyph_info[i].cluster)) {
				glyphLayout.beginsajda = true;
				nbbeginsajda++;
				beginsajdas.remove(glyph_info[i].cluster);

			}
			else if (endsajdas.contains(glyph_info[i].cluster)) {
				glyphLayout.endsajda = true;
				nbendsajda++;
				endsajdas.remove(glyph_info[i].cluster);
			}


			if (glyphLayout.codepoint == 32 || glyphLayout.codepoint == 10) {
				glyphLayout.x_advance = spaceaverage;
				glyphLayout.codepoint = 32;
			}
			else {
				//currentlineWidth += glyphLayout.x_advance;
				if (this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::BaseGlyph || this->glyphGlobalClasses[glyph_info[i].codepoint] == OtLayout::LigatureGlyph) {
					//	lastGlyph = glyph_info[i].codepoint;
				}
			}

			lineLayout.glyphs.push_back(glyphLayout);

		}

		originalLine.append(quran.mid(currentcluster, glyph_info[endIndex - 1].cluster - currentcluster));


		lineLayout.xstartposition = currentxPos;



		if (cand->pageNumber == currentpageNumber) {
			lineLayout.ystartposition = currentyPos;
			currentPage.prepend(lineLayout);
			originalPage.prepend(originalLine);
		}
		else {
			currentyPos = lastLinePos;
			lineLayout.ystartposition = currentyPos;
			currentpageNumber--;

			pages.prepend(currentPage);
			originalPages.prepend(originalPage);
			currentPage = QList<LineLayoutInfo>();
			originalPage.clear();
			originalPage.append(originalLine);
			currentPage.append(lineLayout);

		}

		currentyPos -= OtLayout::InterLineSpacing << OtLayout::SCALEBY;
		cand = &candidates.at(cand->prev);


	}

	if (nbbeginsajda != 15) {
		qDebug() << "nbbeginsajda problems?";
	}
	if (nbendsajda != 15) {
		qDebug() << "nbendsajda problems?";
	}

	pages.prepend(currentPage);
	originalPages.prepend(originalPage);
	suraNamebyPage.prepend(currentSuraName);

	// First & second pages : Al fatiha &  Al Bakara


	for (int pageNumber = 1; pageNumber >= 0; pageNumber--) {

		QString textt = QString::fromUtf8(qurantext[pageNumber] + 1);

		auto lines = textt.split(10, QString::SkipEmptyParts);

		int beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;

		int pageWidth = lineWidth;
		int newLineWidth = 0;

		QList<LineLayoutInfo> page;

		for (int lineIndex = 0; lineIndex < lines.length(); lineIndex++) {
			
			if (lineIndex > 0) {
				double diameter = pageWidth * 1; // 0.9;
				if (pageNumber == 0) {
					diameter = pageWidth * 1; // 0.9;
				}

				int index = lineIndex - 1;
				//index = index % 4;
				// 22.5 = 180 / 8
				double degree = lineIndex * 22.5 * M_PI / 180;
				newLineWidth = diameter * std::sin(degree);
				//std::cout << "lineIndex=" << lineIndex << ", lineWidth=" << lineWidth << std::endl;
			}
			else {
				newLineWidth = 0;
			}

			auto lineResult = this->justifyPage(emScale, newLineWidth, pageWidth, QStringList{ lines[lineIndex] }, LineJustification::Center, false, true)[0];
			

			if (lineIndex == 0) {
				lineResult.type = LineType::Sura;
				lineResult.ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
			}
			else {
				lineResult.ystartposition = beginsura;
				beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;
			}

			page.append(lineResult);
		}
		pages.prepend(page);
		originalPages.prepend(lines);

		if (pageNumber == 1) {
			suraNamebyPage.prepend(currentSuraName);
		}
		else {
			suraNamebyPage.prepend("سُورَةُ الفَاتِحَةِ");
		}
	}

	delete font;
	hb_buffer_destroy(buffer);

	//Compare text

	/*
	QFile file("qurantinputtext.txt");
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);   // we will serialize the data into the file
	out.setCodec("UTF-8");


	QString newquran;

	for (auto page : originalPages) {
		for (auto line : page) {
			if (newquran.isEmpty()) {
				newquran = line;
			}
			else {
				newquran = newquran + " " + line;
			}
			//out << line.replace("\u06E5","").replace("\u06E6", "") << "\n";   // serialize a string
			out << line << "\n";   // serialize a string

		}
	}
	newquran = newquran + " ";

	int index = newquran.compare(quran);
	file.close();*/

	return { pages, originalPages, suraNamebyPage };

}
int OtLayout::AlternatelastCode = 0xF0000;
GlyphVis* OtLayout::getAlternate(int glyphCode, GlyphParameters parameters) {



	auto& paths = JustificationInProgress ? alternatePaths[glyphCode] : nojustalternatePaths[glyphCode];

	if (paths.find(parameters) != paths.end()) {
		return paths.at(parameters);
	}
	else {

		auto glyph = this->getGlyph(glyphCode);

		automedina->addchar(glyph->name, AlternatelastCode, parameters.lefttatweel, parameters.righttatweel, parameters.leftextratio, parameters.rightextratio,
			parameters.left_tatweeltension, parameters.right_tatweeltension, "alternatechar", parameters.which_in_baseline);

		mp_run_data* _mp_results = mp_rundata(automedina->mp);
		mp_edge_object* edges = _mp_results->edges;

		mp_edge_object* p = _mp_results->edges;

		mp_edge_object* edge = nullptr;
		while (p) {
			if (p->charcode == AlternatelastCode) {
				edge = p;
				break;
			}
			p = p->next;
		}

		if (edge == nullptr) {
			throw "Error";
		}

		GlyphVis* newglyph = new	GlyphVis{ this, edge, true };

		//newglyph.name = this->name;

		paths.insert({ parameters, newglyph });

		return paths.at(parameters);
	}


	return nullptr;
}