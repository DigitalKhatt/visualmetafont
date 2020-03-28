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

// quick_example.cpp


#include <iostream>
#include <sstream>
extern "C"
{
#include <mplib.h>
#include <mplibps.h>
}

#include "quranshaper.h"
//#include <QtWidgets/QApplication>
//#include "qplugin.h"

#if  defined  EMSCRIPTEN
using namespace emscripten;
#else
//For debugging
int main(int argc, char** argv) {


	//Q_IMPORT_PLUGIN(QWasmIntegrationPlugin)
	//	QApplication app(argc, argv);

	//return app.exec();

	QuranShaper shaper;

	std::cout << "begin" << "\n";

	auto alternate = shaper.layout->getGlyph(57506, 0.25, 0.52);
	shaper.clearAlternates();

	alternate = shaper.layout->getGlyph(57506, 0.253, 0.523);
	shaper.clearAlternates();

	for (int i = 0; i < 100; i++) {
		auto result = shaper.shapePage(i, 0.8, true,0,true,true);
		shaper.clearAlternates();
	}

	std::cout << "fin" << "\n";

}
#endif


#if defined  EMSCRIPTEN

EMSCRIPTEN_BINDINGS(my_module) {


	enum_<LineType>("LineType")
		.value("Line", LineType::Line)
		.value("Sura", LineType::Sura)
		.value("Bism", LineType::Bism);

	value_object<GlyphLayoutInfo>("GlyphLayoutInfo")
		.field("advance", &GlyphLayoutInfo::advance)
		.field("x_offset", &GlyphLayoutInfo::x_offset)
		.field("y_offset", &GlyphLayoutInfo::y_offset)
		.field("x_advance", &GlyphLayoutInfo::x_advance)
		.field("y_advance", &GlyphLayoutInfo::y_advance)
		.field("codepoint", &GlyphLayoutInfo::codepoint)
		.field("cluster", &GlyphLayoutInfo::cluster)
		.field("lookup_index", &GlyphLayoutInfo::lookup_index)
		.field("subtable_index", &GlyphLayoutInfo::subtable_index)
		.field("lefttatweel", &GlyphLayoutInfo::lefttatweel)
		.field("righttatweel", &GlyphLayoutInfo::righttatweel)
		.field("base_codepoint", &GlyphLayoutInfo::base_codepoint)
		.field("beginsajda", &GlyphLayoutInfo::beginsajda)
		.field("endsajda", &GlyphLayoutInfo::endsajda)
		.field("color", &GlyphLayoutInfo::color)
		;

	register_vector<GlyphLayoutInfo>("VectorGlyphLayoutInfo");
	//register_vector<int>("VectorInt");


	value_object<LineLayoutInfo>("LineLayoutInfo")
		.field("glyphs", &LineLayoutInfo::glyphs)
		.field("xstartposition", &LineLayoutInfo::xstartposition)
		.field("ystartposition", &LineLayoutInfo::ystartposition)
		.field("type", &LineLayoutInfo::type)
		.field("underfull", &LineLayoutInfo::underfull)
		//.field("nbGlyph", optional_override([](LineLayoutInfo& line) {return line.glyphs.size(); }), nullptr)
		;

	value_object<SuraLocation>("SuraLocation")
		.field("name", &SuraLocation::name)
		.field("pageNumber", &SuraLocation::pageNumber)
		.field("x", &SuraLocation::x)
		.field("y", &SuraLocation::y)
		;

	class_<QList<SuraLocation>>("QListSuraLocation")
		.constructor<>()
		//.function("size", &QList<LineLayoutInfo>::size)
		.function("size", optional_override([](QList<SuraLocation>& list) {
		return list.size();
	}))
		.function("value", optional_override([](QList<SuraLocation>& list, int pos) {
		return list[pos];
	}))
		;

	class_<QList<LineLayoutInfo>>("QListLineLayoutInfo")
		.constructor<>()
		//.function("size", &QList<LineLayoutInfo>::size)
		.function("size", optional_override([](QList<LineLayoutInfo>& list) {
		return list.size();
	}))
		.function("value", optional_override([](QList<LineLayoutInfo>& list, int pos) {
		return list[pos];
	}))
		;
	/*
	class_<QList<QString>>("QListQString")
		.constructor<>()
		//.function("size", &QList<LineLayoutInfo>::size)
		.function("size", optional_override([](QList<QString>& list) {
		return list.size();
	}))
		.function("value", optional_override([](QList<QString>& list, int pos) {
		return list[pos];
	}))
		;*/

	class_<QChar>("QChar")
		.function("unicode", select_overload<ushort & ()>(&QChar::unicode))
		;

	class_<QString>("QString")
		.function("at", &QString::at)
		.function("unicode", optional_override([](QString& string, int pos) {
			return string[pos].unicode();
		}))
		.function("size", &QString::size)
		.function("toStdString", &QString::toStdString)
		;

	class_<QStringList>("QStringList")
		.constructor<>()
		//.function("size", &QList<LineLayoutInfo>::size)
		.function("size", optional_override([](QStringList& list) {
		return list.size();
	}))
		.function("get", optional_override([](QStringList& list, int pos) {
		return list[pos];
	}))
		;

	class_<PageResult>("PageResult")
		.property("page", &PageResult::page)
		.property("originalPage", &PageResult::originalPage)
		;

	class_<QuranShaper>("QuranShaper")
		.constructor<>()
		.function("initilizeMetapost", &QuranShaper::initilizeMetapost)		
		.function("executeMetapost", &QuranShaper::executeMetapost)		
		.function("initLayout", &QuranShaper::initLayout)
		.function("initLookup", &QuranShaper::initLookup)
		.function("shapePage", &QuranShaper::shapePage)
		.function("displayGlyph", &QuranShaper::displayGlyph)
		.function("clearAlternates", &QuranShaper::clearAlternates)
		.function("getGlyphName", &QuranShaper::getGlyphName)
		.function("getGlyphCode", &QuranShaper::getGlyphCode)		
		.function("drawPathByName", &QuranShaper::drawPath)
		.function("getSuraLocations", &QuranShaper::getSuraLocations)
		.function("shapeText", &QuranShaper::shapeText)
		

		;
}
#endif