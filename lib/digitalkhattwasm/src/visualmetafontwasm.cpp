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



#include "metafont.h"


#include "quranshaper.h"

using namespace emscripten;

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
	register_vector<LineLayoutInfo>("VectorLineLayoutInfo");
	register_vector<SuraLocation>("VectorSuraLocation");
	register_vector<digitalkhatt::TextString>("VectorString");


	value_object<LineLayoutInfo>("LineLayoutInfo")
		.field("glyphs", &LineLayoutInfo::glyphs)
		.field("xstartposition", &LineLayoutInfo::xstartposition)
		.field("ystartposition", &LineLayoutInfo::ystartposition)
		.field("type", &LineLayoutInfo::type)
		.field("overfull", &LineLayoutInfo::overfull)
    .field("fontSize", &LineLayoutInfo::fontSize)
		;

	value_object<SuraLocation>("SuraLocation")
		.field("name", &SuraLocation::name)
		.field("pageNumber", &SuraLocation::pageNumber)
		.field("x", &SuraLocation::x)
		.field("y", &SuraLocation::y)
		;

  class_<PageResult>("PageResult")
    .property("page", &PageResult::page)
    .property("originalPage", &PageResult::originalPage);

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
    .function("getTexNbPages", &QuranShaper::getTexNbPages)
		.function("shapeText", &QuranShaper::shapeText)
		;

}
