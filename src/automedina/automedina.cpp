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

#include "automedina.h"
#include "qstring.h"
#include "Subtable.h"
#include "Lookup.h"
#include "font.hpp"
#include "GlyphVis.h"
#include <algorithm>
#include "qregularexpression.h"
#include "defaultmarkpositions.h"

extern "C"
{
#include "AddedFiles\newmp.h"
}

extern "C"
{
# include "mplib.h"
# include "mplibps.h"
# include "mplibsvg.h"
}



AnchorCalc* Automedina::getanchorCalcFunctions(QString functionName, Subtable * subtable) {
	if (functionName == "defaultmarkabovemark") {
		return new Defaultmarkabovemark(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaultopmarkanchor") {
		return new Defaultopmarkanchor(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaultmarkbelowmark") {
		return new Defaultmarkbelowmark(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaullowmarkanchor") {
		return new Defaullowmarkanchor(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaultwaqfmarkabovemark") {
		return new Defaulwaqftmarkabovemark(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaultbaseanchorforlow") {
		return new Defaulbaseanchorforlow(*this, *(MarkBaseSubtable*)(subtable));
	}
	else if (functionName == "defaulbaseanchorfortop") {
		return new Defaulbaseanchorfortop(*this, *(MarkBaseSubtable*)(subtable));
	}

	else if (functionName == "joinedsmalllettersbaseanchor") {
		return new Joinedsmalllettersbaseanchor(*this, *(MarkBaseSubtable*)(subtable));
	}

	return nullptr;
}

Automedina::Automedina(OtLayout* layout, MP mp) :glyphs{ layout->glyphs }, m_layout{ layout }, mp{mp}{
	
	//m_metafont = layout->m_font;
	classes["marks"] = {
		"onedotup",
		"onedotdown",
		"twodotsup",
		"twodotsdown",
		"three_dots",
		"fathatanidgham",
		"kasratanidgham",
		"dammatanidgham",
		"fatha",
		"damma",
		"kasra",
		"shadda",
		"headkhah",
		"sukun",
		"dammatan",
		"maddahabove",
		"fathatan",
		"kasratan",
		"smallalef",
		"smallalef.replacement",
		"smallalef.joined",
		"meemiqlab",
		"smallhighyeh",
		"smallhighwaw",
		"wasla",
		"hamzaabove",
		"hamzaabove.lamalef",
		"hamzaabove.joined",
		"hamzabelow",
		"smallhighroundedzero",
		"rectangularzero",
		"smallhighseen",
		"smalllowseen",
		"smallhighnoon",
		"waqf.meem",
		"waqf.lam",
		"waqf.qaf",
		"waqf.jeem",
		"waqf.sad",
		"waqf.smallhighthreedots",
		"roundedfilledhigh",
		"roundedfilledlow",
		"space.ii"
	};

	classes["topmarks"] = {
		//"onedotup",
		//"twodotsup",
		//"three_dots",
		"fathatanidgham",
		"dammatanidgham",
		"fatha",
		"damma",
		"shadda",
		"headkhah",
		"sukun",
		"dammatan",
		"maddahabove",
		"fathatan",
		"smallalef",
		"smallalef.replacement",
		"smallalef.joined",
		"meemiqlab",
		"smallhighyeh",
		"smallhighwaw",
		"wasla",
		"hamzaabove",
		"hamzaabove.joined",
		//"hamzaabove.lamalef",
		"smallhighroundedzero",
		"rectangularzero",
		"smallhighseen",
		"smallhighnoon",
		"roundedfilledhigh",
		"hamzaabove.joined",
	};

	classes["lowmarks"] = {
		//"onedotdown",
		//"twodotsdown",
		"kasratanidgham",
		"kasra",
		"kasratan",
		"hamzabelow",
		"smalllowseen",
		"roundedfilledlow"
	};

	classes["kasras"] = {
		"kasratanidgham",
		"kasra",
		"kasratan"
	};

	classes["waqfmarks"] = {
		"waqf.meem",
		"waqf.lam",
		"waqf.qaf",
		"waqf.jeem",
		"waqf.sad",
		"waqf.smallhighthreedots"
	};

	classes["dotmarks"] = {
		"onedotup",
		"onedotdown",
		"twodotsup",
		"twodotsdown",
		"three_dots"
	};

	classes["topdotmarks"] = {
		"onedotup",
		"twodotsup",
		"three_dots"
	};

	classes["downdotmarks"] = {
		"onedotdown",
		"twodotsdown"
	};

	classes["digits"] = {
		"zeroindic",
		"oneindic",
		"twoindic",
		"threeindic",
		"fourindic",
		"fiveindic",
		"sixindic",
		"sevenindic",
		"eightindic",
		"nineindic"
	};

	initchar = {
		"behshape" ,
		//"teh" ,
		//"tehmarbuta" ,
		//"theh" ,
		//"jeem" ,
		"hah" ,
		//"khah" ,
		//"dal" ,
		//"thal" ,
		//"reh" ,
		//"zain" ,
		"seen" ,
		//"sheen" ,
		"sad" ,
		//"dad" ,
		"tah" ,
		//"zah" ,
		"ain" ,
		//"ghain" ,
		"fehshape" ,
		//"qaf" ,
		"kaf" ,
		"lam" ,
		"meem" ,
		//"noon" ,
		"heh" ,
		//"waw" ,
		//"yeh" ,
		//"yehwithoutdots" ,
		//"alefmaksura"
	};

	medichar = {
		//"alef" ,
		"behshape" ,
		//"teh" ,
		//"tehmarbuta" ,
		//"theh" ,
		//"jeem" ,
		"hah" ,
		//"khah" ,
		//"dal" ,
		//"thal" ,
		//"reh" ,
		//"zain" ,
		"seen" ,
		//"sheen" ,
		"sad" ,
		//"dad" ,
		"tah" ,
		//"zah" ,
		"ain" ,
		//"ghain" ,
		"fehshape" ,
		//"qaf" ,
		"kaf" ,
		"lam" ,
		"meem" ,
		//"noon" ,
		"heh" ,
		//"waw" ,
		//"yeh" ,
		//"yehwithoutdots" ,
		//"alefmaksura"
	};

	addchars();

	generateGlyphs();

	//setAnchorCalcFunctions();


}

void Automedina::generateGlyphs() {

	mp_run_data* _mp_results = mp_rundata(mp);
	mp_edge_object * edges = _mp_results->edges;

	glyphs.clear();

	while (edges) {

		GlyphVis& glyph = *glyphs.insert(edges->charname, GlyphVis(m_layout, edges));

		m_layout->glyphNamePerCode[glyph.charcode] = glyph.name;
		m_layout->glyphCodePerName[glyph.name] = glyph.charcode;

		//if (glyph.name.contains("space")) {

		//}
		//else 
		if (!classes["marks"].contains(glyph.name)) {
			classes["bases"].insert(glyph.name);
			m_layout->glyphGlobalClasses[glyph.charcode] = OtLayout::BaseGlyph;
		}
		else {
			m_layout->glyphGlobalClasses[glyph.charcode] = OtLayout::MarkGlyph;
		}

		//uint totalAnchors = getTotalAnchors(mp, glyph.charcode);

		if (edges->numAnchors > 10) {
			throw "error";
		}

		for (int i = 0; i < edges->numAnchors; i++) {
			AnchorPoint anchor = edges->anchors[i];
			if (anchor.anchorName) {
				switch (anchor.type)
				{
				case 1:
					markAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
					break;
				case 2:
					entryAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
					break;
				case 3:
					exitAnchors[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
				case 4:
					entryAnchorsRTL[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
					break;
				case 5:
					exitAnchorsRTL[anchor.anchorName][glyph.charcode] = QPoint(anchor.x, anchor.y);
				default:
					break;
				}
			}
		}

		edges = edges->next;
	}

	auto& ayaGlyph = glyphs["endofaya"];

	int ayacharcode = AyaNumberCode;

	for (int i = 1; i <= 286; i++) {

		QString name = QString("aya%1").arg(i);

		// Amine very temporary for test
		auto& glyph = *glyphs.insert(name, ayaGlyph);

		glyph.name = name;
		glyph.charcode = ayacharcode++;
#ifndef DIGITALKHATT_WEBLIB
		glyph.refresh(glyphs);
#endif

		m_layout->glyphNamePerCode.insert(glyph.charcode, glyph.name);
		m_layout->glyphCodePerName.insert(glyph.name, glyph.charcode);

		m_layout->glyphGlobalClasses.insert(glyph.charcode, OtLayout::LigatureGlyph);

	}

	m_layout->glyphs = glyphs;

	
}

Lookup* Automedina::getLookup(QString lookupName) {

	if (lookupName == "defaultmarkpositioncpp") {
		return defaultmarkposition();
	}
	else if (lookupName == "defaultwaqfmarktobase") {
		return defaultwaqfmarktobase();
	}
	else if (lookupName == "forsmalllalef") {
		return forsmalllalef();
	}
	else if (lookupName == "forhamza") {
		return forhamza();
	}
	else if (lookupName == "forheh") {
		return forheh();
	}
	else if (lookupName == "forwaw") {
		return forwaw();
	}
	else if (lookupName == "leftrightcursive") {
		return leftrightcursive();
	}
	else if (lookupName == "rehwawcursive") {
		return rehwawcursive();
	}
	else if (lookupName == "ligaturecursive") {
		return ligaturecursive();
	}
	else if (lookupName == "defaultdotmarks") {
		return defaultdotmarks();
	}
	else if (lookupName == "pointmarks") {
		return pointmarks();
	}
	else if (lookupName == "defaultwaqfmarkabovemarkprecise") {
		return defaultwaqfmarkabovemarkprecise();
	}
	else if (lookupName == "defaultmarkdotmarks") {
		return defaultmarkdotmarks();
	}
	else if (lookupName == "defaultmkmk") {
		return defaultmkmk();
	}
	else if (lookupName == "ayanumbers") {
		return ayanumbers();
	}
	else if (lookupName == "shrinkstretchlt") {
		return shrinkstretchlt();
	}
	else if (lookupName == "lowmarkafterwawandreh") {
		return lowmarkafterwawandreh();
	}
	else if (lookupName == "tajweedcolorcpp") {
		return tajweedcolorcpp();
	}
	else if (lookupName == "forsmallhighwaw") {
		return forsmallhighwaw();
	}



	return NULL;
}

Lookup * Automedina::rehwawcursive() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "rehwawcursive";
	lookup->feature = "curs";
	lookup->type = Lookup::cursive;
	lookup->flags = Lookup::Flags::IgnoreMarks;

	class CustomCursiveSubtable : public CursiveSubtable {
	public:
		CustomCursiveSubtable(Lookup* lookup) : CursiveSubtable(lookup) {}

		virtual QPoint calculateEntry(GlyphVis* originalglyph, GlyphVis* extendedglyph, QPoint defaultEntry) {

			QPoint entry = entryParameters[originalglyph->charcode];

			entry += QPoint(extendedglyph->width, 0);

			return entry;

		}

	};

	CursiveSubtable* rehwawisol = new CustomCursiveSubtable(lookup);
	lookup->subtables.append(rehwawisol);
	rehwawisol->name = "rehwawisol";

	CursiveSubtable* rehwawfina = new CustomCursiveSubtable(lookup);
	lookup->subtables.append(rehwawfina);
	rehwawfina->name = "rehwawfina";

	CursiveSubtable* rehfinaafterbehshape = new CursiveSubtable(lookup);
	lookup->subtables.append(rehfinaafterbehshape);
	rehfinaafterbehshape->name = "rehfinaafterbehshape";
	rehfinaafterbehshape->anchors[glyphs["reh.fina.afterbehshape"].charcode].exit = QPoint(0, 0);

	CursiveSubtable* rehfinaafterseen = new CursiveSubtable(lookup);
	lookup->subtables.append(rehfinaafterseen);
	rehfinaafterseen->name = "rehfinaafterseen";
	rehfinaafterseen->anchors[glyphs["reh.fina.afterseen"].charcode].exit = QPoint(0, 0);




	auto glyphcodes = m_layout->classtoUnicode("reh.isol|waw.isol");

	for (auto glyphcode : glyphcodes) {
		rehwawisol->anchors[glyphcode].exit = QPoint(0, 0);
	}

	glyphcodes = m_layout->classtoUnicode("reh.fina$|waw.fina$");

	for (auto glyphcode : glyphcodes) {
		rehwawfina->anchors[glyphcode].exit = QPoint(0, 0);
	}



	glyphcodes = m_layout->classtoUnicode("isol|init"); //"((?<!reh|waw)[.]isol)|init"

	for (auto glyphcode : glyphcodes) {
		QString glyphName = m_layout->glyphNamePerCode[glyphcode];
		auto& glyph = glyphs[glyphName];
		bool includeGlyph = true;
		for (auto anchors : exitAnchorsRTL) {
			if (anchors.contains(glyphcode)) {
				includeGlyph = false;
				break;
			}
		}

		if (includeGlyph) {
			rehwawisol->anchors[glyphcode].entry = QPoint(glyph.width, 0);
			rehwawfina->anchors[glyphcode].entry = QPoint(glyph.width, 0);
			rehfinaafterbehshape->anchors[glyphcode].entry = QPoint(glyph.width, 0);
			rehfinaafterseen->anchors[glyphcode].entry = QPoint(glyph.width, 0);
		}

	}

	return lookup;

}
Lookup * Automedina::leftrightcursive() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "leftrightcursive";
	lookup->feature = "curs";
	lookup->type = Lookup::cursive;
	lookup->flags = 0;
	lookup->flags = lookup->flags | Lookup::Flags::RightToLeft | Lookup::Flags::IgnoreMarks;

	struct Impl : CursiveSubtable
	{
		Impl(Lookup* lookup) : CursiveSubtable{ lookup } {}

		virtual optional<QPoint> getEntry(quint16 glyph_id, double lefttatweel, double righttatweel) override {

			if (lefttatweel == 0 && righttatweel == 0) {
				return CursiveSubtable::getEntry(glyph_id, 0, 0);
			}

			auto glyphName = this->getNameFromCode(glyph_id);

			GlyphVis* glyph = &this->m_layout->glyphs[glyphName];

			GlyphParameters parameters{};

			parameters.lefttatweel = lefttatweel;
			parameters.righttatweel = righttatweel;

			glyph = glyph->getAlternate(parameters);

			return glyph->rightAnchor;

		}

		virtual optional<QPoint> getExit(quint16 glyph_id, double lefttatweel, double righttatweel) override {

			if (lefttatweel == 0 && righttatweel == 0) {
				return CursiveSubtable::getExit(glyph_id, 0, 0);
			}

			auto glyphName = this->getNameFromCode(glyph_id);

			GlyphVis* glyph = &this->m_layout->glyphs[glyphName];

			GlyphParameters parameters{};

			parameters.lefttatweel = lefttatweel;
			parameters.righttatweel = righttatweel;

			glyph = glyph->getAlternate(parameters);

			return glyph->leftAnchor;

		}
	};


	CursiveSubtable* newsubtable = new Impl(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "leftrightcursive";

	for (auto& glyph : glyphs) {
		if (glyph.leftAnchor || glyph.rightAnchor) {
			newsubtable->anchors[glyph.charcode].entry = glyph.rightAnchor;
			newsubtable->anchors[glyph.charcode].exit = glyph.leftAnchor;
		}
	}

	/*
	CursiveSubtable* leftrightcursivenonjoining = new CursiveSubtable(lookup);
	lookup->subtables.append(leftrightcursivenonjoining);

	leftrightcursivenonjoining->name = "leftrightcursivenonjoining";

	for (auto& firstglyph : glyphs) {
		if (firstglyph.name.contains("isol")) {
			for (auto& secondglyph : glyphs) {
				if (secondglyph.name.contains("init")) {
					leftrightcursivenonjoining->anchors[firstglyph.charcode].exit = QPoint{ 0,0 };
					leftrightcursivenonjoining->anchors[secondglyph.charcode].entry = QPoint{ (int)secondglyph.width,0 };
				}
			}

		}
	}*/

	return lookup;

}
Lookup * Automedina::ligaturecursive() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "ligaturecursive";
	lookup->feature = "curs";
	lookup->type = Lookup::cursive;
	lookup->flags = 0;
	//lookup->flags = lookup->flags | Lookup::Flags::RightToLeft | Lookup::Flags::IgnoreMarks;
	lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;


	for (auto it = entryAnchors.constBegin(); it != entryAnchors.constEnd(); ++it) {
		QString cursiveName = it.key();
		auto entries = it.value();
		auto exits = exitAnchors[cursiveName];

		CursiveSubtable* newsubtable = new CursiveSubtable(lookup);
		lookup->subtables.append(newsubtable);
		newsubtable->name = "ligaturecursive";

		for (auto anchor = entries.constBegin(); anchor != entries.constEnd(); ++anchor) {
			newsubtable->anchors[anchor.key()].entry = anchor.value();
		}

		for (auto anchor = exits.constBegin(); anchor != exits.constEnd(); ++anchor) {
			newsubtable->anchors[anchor.key()].exit = anchor.value();
		}
	}

	m_layout->addLookup(lookup);

	lookup = new Lookup(m_layout);
	lookup->name = "ligaturecursiveRTL";
	lookup->feature = "curs";
	lookup->type = Lookup::cursive;
	lookup->flags = 0;
	lookup->flags = lookup->flags | Lookup::Flags::RightToLeft | Lookup::Flags::IgnoreMarks;

	for (auto it = entryAnchorsRTL.constBegin(); it != entryAnchorsRTL.constEnd(); ++it) {
		QString cursiveName = it.key();
		auto entries = it.value();
		auto exits = exitAnchorsRTL[cursiveName];

		CursiveSubtable* newsubtable = new CursiveSubtable(lookup);
		lookup->subtables.append(newsubtable);
		newsubtable->name = "ligaturecursiveRTL";

		for (auto anchor = entries.constBegin(); anchor != entries.constEnd(); ++anchor) {
			newsubtable->anchors[anchor.key()].entry = anchor.value();
		}

		for (auto anchor = exits.constBegin(); anchor != exits.constEnd(); ++anchor) {
			newsubtable->anchors[anchor.key()].exit = anchor.value();
		}
	}


	return lookup;



	/*
	int leftbearing = 100;

	if (glyphs["alefmaksura.fina.ii"]->anchors.contains("yehcursive1")) {
		//newsubtable->anchors[m_layout->glyphCodePerName["alefmaksura.fina.ii"]].entry = QPoint(400 + leftbearing - 24, 62);
		newsubtable->anchors[m_layout->glyphCodePerName["alefmaksura.fina.ii"]].entry = glyphs["alefmaksura.fina.ii"]->anchors["yehcursive1"];
	}

	newsubtable->anchors[m_layout->glyphCodePerName["yeh.fina.ii"]].entry = glyphs["yeh.fina.ii"]->anchors["yehcursive1"];

	if (glyphs["lam.medi.beforeyeh"]->anchors.contains("yehcursive1")) {
		//newsubtable->anchors[m_layout->glyphCodePerName["lam.medi.beforeyeh"]].exit = QPoint(2, -79);
		newsubtable->anchors[m_layout->glyphCodePerName["lam.medi.beforeyeh"]].exit = glyphs["lam.medi.beforeyeh"]->anchors["yehcursive1"];
	}

	if (glyphs["feh.init.yeh"]->anchors.contains("yehcursive1")) {
		newsubtable->anchors[m_layout->glyphCodePerName["feh.init.yeh"]].exit = glyphs["feh.init.yeh"]->anchors["yehcursive1"];
	}*/



}
Lookup * Automedina::defaultmarkposition() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultmarkposition";
	lookup->feature = "mark";
	lookup->type = Lookup::mark2base;
	lookup->flags = 0;

	auto topmarks = classes["topmarks"];

	topmarks.remove("smallalef");
	topmarks.remove("smallalef.joined");
	topmarks.remove("smallalef.replacement");
	topmarks.remove("smallhighyeh");
	topmarks.remove("smallhighwaw");
	topmarks.remove("smallhighnoon");
	topmarks.remove("roundedfilledhigh");
	topmarks.remove("hamzaabove");
	topmarks.remove("hamzaabove.joined");
	topmarks.remove("wasla");
	topmarks.remove("maddahabove");
	topmarks.remove("smallhighseen");
	topmarks.remove("shadda");

	auto lowmarks = classes["lowmarks"];
	lowmarks.remove("hamzabelow");
	lowmarks.remove("smalllowseen");


	//meem.fina.afterkaf

	MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "meemfinaafterkaf";
	newsubtable->base = { "meem.fina.afterkaf" };
	newsubtable->classes["sukun"].mark = { "sukun" };
	newsubtable->classes["sukun"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["sukun"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//tah
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "tah";
	newsubtable->base = { "^tah" };

	newsubtable->classes["fathadamma"].mark = { "fatha", "damma","shadda", "sukun" };
	newsubtable->classes["fathadamma"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["fathadamma"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	newsubtable->classes["fathatandammatan"].mark = { "fathatan", "dammatan" };
	newsubtable->classes["fathatandammatan"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["fathatandammatan"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	newsubtable->classes["idgham"].mark = { "fathatanidgham", "dammatanidgham" };
	newsubtable->classes["idgham"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["idgham"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);


	//default
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "defaultmarkposition";
	newsubtable->base = { "bases" };
	newsubtable->classes["topmarks"].mark = topmarks;
	newsubtable->classes["topmarks"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["topmarks"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);


	newsubtable->classes["lowmarks"].mark = lowmarks;
	newsubtable->classes["lowmarks"].basefunction = new Defaulbaseanchorforlow(*this, *newsubtable);
	newsubtable->classes["lowmarks"].markfunction = new Defaullowmarkanchor(*this, *newsubtable);

	/*
	newsubtable->classes["shadda"].mark = { "shadda" };
	newsubtable->classes["shadda"].basefunction = new Defaulbaseanchorforsmallalef(*this, *newsubtable);
	newsubtable->classes["shadda"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);*/

	newsubtable->classes["smallletters"].mark = { "smallalef.joined","smallhighwaw","hamzaabove.joined" };
	newsubtable->classes["smallletters"].basefunction = new Defaulbaseanchorforsmallalef(*this, *newsubtable);
	newsubtable->classes["smallletters"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//default
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);
	newsubtable->name = "smallhighyeh";
	newsubtable->base = { "bases" }; // TODO minimize

	newsubtable->classes["smallhighyeh"].mark = { "smallhighyeh" };
	newsubtable->classes["smallhighyeh"].basefunction = new Defaulbaseanchorforsmallalef(*this, *newsubtable);
	newsubtable->classes["smallhighyeh"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//shadda

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "shadda";
	newsubtable->base = { "bases" };
	newsubtable->classes["shadda"].mark = { "shadda" };
	newsubtable->classes["shadda"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["shadda"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//maddahabove

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "maddahabove";
	newsubtable->base = { "bases" };
	newsubtable->classes["maddahabove"].mark = { "maddahabove" };
	newsubtable->classes["maddahabove"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["maddahabove"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//hamzaabove

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "hamzaabove";
	newsubtable->base = { "alef|waw|yehshape|behshape" };
	newsubtable->classes["hamzaabove"].mark = { "hamzaabove" };
	newsubtable->classes["hamzaabove"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["hamzaabove"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//smallalef.replacement

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "smallalefreplacement";
	newsubtable->base = { "alef|waw|yehshape|behshape" };
	newsubtable->classes["smallalefreplacement"].mark = { "smallalef.replacement" };
	newsubtable->classes["smallalefreplacement"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["smallalefreplacement"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//roundedfilledhigh

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "roundedfilledhigh";
	newsubtable->base = { "alef[.]isol.*|meem[.]init.*" };
	newsubtable->classes["roundedfilledhigh"].mark = { "roundedfilledhigh" };
	newsubtable->classes["roundedfilledhigh"].basefunction = new Defaulbaseanchorforsmallalef(*this, *newsubtable);
	newsubtable->classes["roundedfilledhigh"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//smallhighnoon
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "smallhighnoon";
	newsubtable->base = { "behshape[.]init.*" };
	newsubtable->classes["smallhighnoon"].mark = { "smallhighnoon" };
	newsubtable->classes["smallhighnoon"].basefunction = new Defaulbaseanchorforsmallalef(*this, *newsubtable);
	newsubtable->classes["smallhighnoon"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//smallhighseen
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "smallhighseen";
	newsubtable->base = { "sad[.]medi|^alef.fina|^heh.fina|^lam.fina|^noon.fina" };
	newsubtable->classes["smallhighseen"].mark = { "smallhighseen" };
	newsubtable->classes["smallhighseen"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["smallhighseen"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//hamzaabove.lamalef
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "hamzaabove.lamalef";
	newsubtable->base = { "lam.init.lamalef","^lam.medi.laf" };
	newsubtable->classes["hamzaabove"].mark = { "hamzaabove.lamalef" };
	newsubtable->classes["hamzaabove"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["hamzaabove"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//hamzabelow
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "hamzabelow";
	newsubtable->base = { "^alef[.]" };
	newsubtable->classes["hamzabelow"].mark = { "hamzabelow" };
	newsubtable->classes["hamzabelow"].basefunction = nullptr;//;new Defaulbaseanchorforlow(*this, *newsubtable);
	newsubtable->classes["hamzabelow"].markfunction = nullptr; // new Defaullowmarkanchor(*this, *newsubtable);

	//wasla
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "wasla";
	newsubtable->base = { "^alef[.]" };
	newsubtable->classes["wasla"].mark = { "wasla" };
	newsubtable->classes["wasla"].basefunction = new Defaulbaseanchorfortop(*this, *newsubtable);
	newsubtable->classes["wasla"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	//smalllowseen
	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "smalllowseen";
	newsubtable->base = { "^sad[.]medi" };
	newsubtable->classes["smalllowseen"].mark = { "smalllowseen" };
	newsubtable->classes["smalllowseen"].basefunction = new Defaulbaseanchorforlow(*this, *newsubtable);
	newsubtable->classes["smalllowseen"].markfunction = new Defaullowmarkanchor(*this, *newsubtable);


	return lookup;



}
Lookup * Automedina::defaultwaqfmarktobase() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultwaqfmarktobase";
	lookup->feature = "mark";
	lookup->type = Lookup::mark2base;


	auto basefunction = [this](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis& curr = glyphs[glyphName];

		int height = std::max((int)curr.height + spacebasetotopmark, minwaqfhigh);
		int width = 0; // curr.bbox.llx;


		width = width + adjust.x();
		height = height + adjust.y();

		return QPoint(width, height);
	};

	auto markfunction = [this](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis& curr = glyphs[glyphName];

		int height = 0;
		int width = 0;


		width = width + adjust.x();
		height = height + adjust.y();

		return QPoint(width, height);
	};

	MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "defaultwaqfmarktobase";
	newsubtable->base = { "isol|fina|smallwaw|smallyeh" };


	newsubtable->classes["waqfmarks"].mark = { "waqfmarks" };
	newsubtable->classes["waqfmarks"].basefunction = MakeAnchorCalc(basefunction);
	newsubtable->classes["waqfmarks"].markfunction = MakeAnchorCalc(markfunction);

	return lookup;
}
Lookup * Automedina::defaultdotmarks() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultdotmarks";
	lookup->feature = "mark";
	lookup->type = Lookup::mark2base;
	lookup->flags = 0;


	MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);

	/*classes["topdotmarks"] = {
		"onedotup",
		"twodotsup",
		"three_dots"
	};

	classes["downdotmarks"] = {
		"onedotdown",
		"twodotsdown"
	};*/

	newsubtable->name = "onedotup";
	newsubtable->base = { "^behshape|^hah|^fehshape|^dal|^reh|^sad|^tah|^ain" };
	newsubtable->classes["onedotup"].mark = { "onedotup" };
	newsubtable->classes["onedotup"].basefunction = new Defaulbaseanchorfortopdots(*this, *newsubtable);
	newsubtable->classes["onedotup"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);
	newsubtable->name = "twodotsup";
	newsubtable->base = { "^behshape|^fehshape|^heh" };
	newsubtable->classes["twodotsup"].mark = { "twodotsup" };
	newsubtable->classes["twodotsup"].basefunction = new Defaulbaseanchorfortopdots(*this, *newsubtable);
	newsubtable->classes["twodotsup"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);
	newsubtable->name = "three_dots";
	newsubtable->base = { "^behshape|^seen" };
	newsubtable->classes["three_dots"].mark = { "three_dots" };
	newsubtable->classes["three_dots"].basefunction = new Defaulbaseanchorfortopdots(*this, *newsubtable);
	newsubtable->classes["three_dots"].markfunction = new Defaultopmarkanchor(*this, *newsubtable);

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);
	newsubtable->name = "onedotdown";
	newsubtable->base = { "^behshape|^hah" };
	newsubtable->classes["onedotdown"].mark = { "onedotdown" };
	newsubtable->classes["onedotdown"].basefunction = new Defaulbaseanchorforlowdots(*this, *newsubtable);
	newsubtable->classes["onedotdown"].markfunction = new Defaullowmarkanchor(*this, *newsubtable);

	newsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(newsubtable);
	newsubtable->name = "twodotsdown";
	newsubtable->base = { "^behshape" };
	newsubtable->classes["twodotsdown"].mark = { "twodotsdown" };
	newsubtable->classes["twodotsdown"].basefunction = new Defaulbaseanchorforlowdots(*this, *newsubtable);
	newsubtable->classes["twodotsdown"].markfunction = new Defaullowmarkanchor(*this, *newsubtable);

	return lookup;


	//std::function<QPoint(QString, QString, QPoint)> basefunction;
	/*
	auto basetopfunction = [this, newsubtable](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis* curr = glyphs[glyphName];

		int height;
		int width;

		if (!curr->originalglyph.isEmpty() && (curr->charlt != 0 || curr->charrt != 0)) {
			QPoint adjustoriginal;
			if (newsubtable->classes[className].baseparameters.contains(curr->originalglyph)) {
				adjustoriginal = newsubtable->classes[className].baseparameters[curr->originalglyph];
			}

			//QPoint originalAnchor = basefunction(curr->originalglyph, className, adjustoriginal);
			GlyphVis* original = glyphs[curr->originalglyph];
			double xshift = curr->matrix.xpart - original->matrix.xpart;
			double yshift = curr->matrix.ypart - original->matrix.ypart;

			width = original->width * 0.5 + xshift + adjustoriginal.x();
			height = (int)original->height + 80 + yshift + adjustoriginal.y();

		}
		else {
			width = curr->width * 0.5;
			height = (int)curr->height + 80;
		}


		width = width + adjust.x();
		height = height + adjust.y();

		return QPoint(width, height);
	};

	auto basedownfunction = [this, newsubtable](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis* curr = glyphs[glyphName];

		int height;
		int width;

		if (!curr->originalglyph.isEmpty() && (curr->charlt != 0 || curr->charrt != 0)) {
			QPoint adjustoriginal;
			if (newsubtable->classes[className].baseparameters.contains(curr->originalglyph)) {
				adjustoriginal = newsubtable->classes[className].baseparameters[curr->originalglyph];
			}

			//QPoint originalAnchor = basefunction(curr->originalglyph, className, adjustoriginal);
			GlyphVis* original = glyphs[curr->originalglyph];
			double xshift = curr->matrix.xpart - original->matrix.xpart;
			double yshift = -curr->matrix.ypart + original->matrix.ypart;

			width = original->width * 0.5 + xshift + adjustoriginal.x();
			height = (int)-original->depth + 50 + yshift - adjustoriginal.y();

		}
		else {
			height = (int)-curr->depth + 50;
			width = curr->width * 0.5;
		}


		width = width + adjust.x();
		height = height - adjust.y();

		return QPoint(width, -height);
	};*/


}
Lookup* Automedina::defaultmkmk() {

	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultmkmk";
	lookup->feature = "mkmk";
	lookup->type = Lookup::mark2mark;
	lookup->flags = 0;

	MarkBaseSubtable* subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "defaultmkmktop";
	subtable->base = { "hamzaabove","hamzaabove.joined","hamzaabove.lamalef", "shadda","smallalef","smallalef.joined","smallalef.replacement","smallhighseen", "smallhighwaw","smallhighyeh" };

	subtable->classes["topmarks"].mark = { "topmarks" };
	subtable->classes["topmarks"].basefunction = new Defaultmarkabovemark(*this, *subtable);
	subtable->classes["topmarks"].markfunction = new Defaultopmarkanchor(*this, *subtable);

	subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "defaultmkmkbottom";
	subtable->base = { "hamzabelow","hamzaabove.joined","smallhighyeh" };

	subtable->classes["lowmarks"].mark = { "lowmarks" };
	subtable->classes["lowmarks"].basefunction = new Defaulbaseanchorforlow(*this, *subtable);
	subtable->classes["lowmarks"].markfunction = new Defaullowmarkanchor(*this, *subtable);

	subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "defaultmkmkmeemiqlab";
	subtable->base = { "fatha","damma" };// "kasra"

	subtable->classes["meemiqlab"].mark = { "meemiqlab" };
	subtable->classes["meemiqlab"].basefunction = new Defaultmarkabovemark(*this, *subtable);
	subtable->classes["meemiqlab"].markfunction = new Defaultopmarkanchor(*this, *subtable);

	subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "smallhighseenwaqf.qaf";
	subtable->base = { "smallhighseen" };

	subtable->classes["waqf.qaf"].mark = { "waqf.qaf" };
	subtable->classes["waqf.qaf"].basefunction = new Defaultmarkabovemark(*this, *subtable);
	subtable->classes["waqf.qaf"].markfunction = new Defaultopmarkanchor(*this, *subtable);


	// hamzaabove.joined
	subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "hamzaabovejoined";
	subtable->base = { "maddahabove" };

	subtable->classes["hamzaabove.joined"].mark = { "hamzaabove.joined" };
	subtable->classes["hamzaabove.joined"].basefunction = nullptr; //;new Defaultmarkabovemark(*this, *smallhighseen);
	subtable->classes["hamzaabove.joined"].markfunction = nullptr; // new Defaultopmarkanchor(*this, *smallhighseen);


	m_layout->addLookup(lookup);

	// hamzaabove.joined

	lookup = new Lookup(m_layout);
	lookup->name = "smallalefjoined";
	lookup->feature = "mkmk";
	lookup->type = Lookup::mark2mark;
	lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef.joined"].charcode , (quint16)glyphs["hamzaabove.joined"].charcode });
	lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;

	subtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(subtable);

	subtable->name = "smallalefjoined";
	subtable->base = { "hamzaabove.joined" };

	subtable->classes["smallalef.joined"].mark = { "smallalef.joined" };
	//subtable->classes["smallalef.joined"].baseanchors = { { "smallalef.joined", 1 } }
	subtable->classes["smallalef.joined"].markanchors = { { "smallalef.joined", QPoint(200,0) } };

	return lookup;

}
Lookup* Automedina::defaultmarkdotmarks() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultmarkdotmarkstop";
	lookup->feature = "mkmk";
	lookup->type = Lookup::mark2mark;
	lookup->flags = 0;


	MarkBaseSubtable* topsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(topsubtable);

	topsubtable->name = "defaultmarkdotmarkstop";
	topsubtable->base = { "topdotmarks" };

	auto basetopfunction = [this, topsubtable](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis& curr = glyphs[glyphName];


		int width = curr.width * 0.5;
		int height = (int)curr.height + 80;



		width = width + adjust.x();
		height = height + adjust.y();

		return QPoint(width, height);
	};

	auto topmarks = classes["topmarks"];
	topmarks.remove("shadda");

	topsubtable->classes["topmarks"].mark = topmarks;
	topsubtable->classes["topmarks"].basefunction = MakeAnchorCalc(basetopfunction);
	topsubtable->classes["topmarks"].markfunction = new Defaultopmarkanchor(*this, *topsubtable);

	topsubtable->classes["shadda"].mark = { "shadda" };
	topsubtable->classes["shadda"].basefunction = MakeAnchorCalc(basetopfunction);
	topsubtable->classes["shadda"].markfunction = new Defaultopmarkanchor(*this, *topsubtable);

	m_layout->addLookup(lookup);

	lookup = new Lookup(m_layout);
	lookup->name = "defaultmarkdotmarksbottom";
	lookup->feature = "mkmk";
	lookup->type = Lookup::mark2mark;
	lookup->flags = 0;
	lookup->setGlyphSet({ "downdotmarks","lowmarks" });

	MarkBaseSubtable* bottomsubtable = new MarkBaseSubtable(lookup);
	lookup->subtables.append(bottomsubtable);

	bottomsubtable->name = "defaultmarkdotmarksbottom";
	bottomsubtable->base = { "downdotmarks" };

	auto basedownfunction = [this, bottomsubtable](QString glyphName, QString className, QPoint adjust) -> QPoint {
		GlyphVis& curr = glyphs[glyphName];

		int depth = -(int)curr.depth + 50;
		int width = curr.width * 0.5;


		width = width + adjust.x();
		depth = depth - adjust.y();

		return QPoint(width, -depth);
	};

	bottomsubtable->classes["lowmarks"].mark = { "lowmarks" };
	bottomsubtable->classes["lowmarks"].basefunction = MakeAnchorCalc(basedownfunction);
	bottomsubtable->classes["lowmarks"].markfunction = new Defaullowmarkanchor(*this, *bottomsubtable);


	return lookup;

}
Lookup * Automedina::defaultwaqfmarkabovemarkprecise() {
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "defaultwaqfmarkabovemarkprecise";
	lookup->feature = "mark";
	lookup->type = Lookup::chainingpos;
	lookup->flags = 0;

	QSet<quint16> waqfmarks = classtoUnicode("waqfmarks");
	QSet<quint16> bases = classtoUnicode("bases");

	for (auto topmark : classes["topmarks"]) {

		QString sublookupName = topmark;

		Lookup * sublookup = new Lookup(m_layout);
		sublookup->name = lookup->name + "." + sublookupName;
		sublookup->feature = "";
		sublookup->type = Lookup::mark2base;
		sublookup->flags = 0;

		m_layout->addLookup(sublookup);

		MarkBaseSubtable* marksubtable = new MarkBaseSubtable(sublookup);
		sublookup->subtables.append(marksubtable);

		marksubtable->name = sublookup->name;
		marksubtable->base = { "bases" };


		marksubtable->classes["waqfmarks"].mark = { "waqfmarks" };
		marksubtable->classes["waqfmarks"].basefunction = new Defaulbaseanchorfortop(*this, *marksubtable);
		marksubtable->classes["waqfmarks"].markfunction = new Defaultopmarkanchor(*this, *marksubtable);


		ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);



		newsubtable->name = "topmarks_" + topmark;

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.backtrack.append(bases);
		newsubtable->compiledRule.backtrack.append({ (quint16)glyphs[topmark].charcode });
		newsubtable->compiledRule.input.append(waqfmarks);

		newsubtable->compiledRule.lookupRecords.append({ 0,sublookupName });
	}



	return lookup;

}
Lookup * Automedina::lowmarkafterwawandreh() {

	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "lowmarkafterwawandreh";
	lookup->feature = "mkmk";
	lookup->type = Lookup::chainingpos;

	lookup->markGlyphSetIndex = m_layout->addMarkSet({ "kasra", "kasratan", "onedotdown", "twodotsdown", "kasratanidgham" });
	lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;

	for (QString finaisol : {"fina.afterbehshape", "fina.afterseen", "fina", "isol" }) {
		for (QString kastradot : { "1", "2", "beforehah" }) {

			Lookup * sublookup = new Lookup(m_layout);
			sublookup->name = lookup->name + "." + finaisol + kastradot;
			sublookup->feature = "";
			sublookup->markGlyphSetIndex = m_layout->addMarkSet({ "kasra", "kasratan", "onedotdown", "twodotsdown", "kasratanidgham" });
			sublookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;

			MarkBaseSubtable* marksubtable = new MarkBaseSubtable(sublookup);
			sublookup->subtables.append(marksubtable);

			marksubtable->name = sublookup->name;
			if (kastradot == "1") {
				sublookup->type = Lookup::mark2base;
				marksubtable->base = { "[.]init|[.]isol" };

				marksubtable->classes["lowmarks"].mark = { "kasra", "onedotdown", "twodotsdown" };
				marksubtable->classes["lowmarks"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
				marksubtable->classes["lowmarks"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);

				marksubtable->classes["kasratan"].mark = { "kasratan",  "kasratanidgham" };
				marksubtable->classes["kasratan"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
				marksubtable->classes["kasratan"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);

			}
			else if (kastradot == "2") {
				//sublookup->type = Lookup::mark2mark;
				//marksubtable->base = { "onedotdown", "twodotsdown" };

				sublookup->type = Lookup::mark2base;
				marksubtable->base = { "(behshape|hah)[.](init|isol)" };

				marksubtable->classes["kasra"].mark = { "kasra", };
				marksubtable->classes["kasra"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
				marksubtable->classes["kasra"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);

				marksubtable->classes["kasratan"].mark = { "kasratan", "kasratanidgham" };
				marksubtable->classes["kasratan"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
				marksubtable->classes["kasratan"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);


			}
			else if (kastradot == "beforehah") {
				sublookup->type = Lookup::mark2base;
				marksubtable->base = { "reh|waw" };

				marksubtable->classes["kasra"].mark = { "kasra" };
				marksubtable->classes["kasra"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
				marksubtable->classes["kasra"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);
			}

			m_layout->addLookup(sublookup);
		}
		// displac
		/*
		if (finaisol == "isol") {
			Lookup * sublookup = new Lookup(m_layout);
			sublookup->name = lookup->name + "." + finaisol + "displac";
			sublookup->feature = "";
			sublookup->is_gsub = false;
			sublookup->local = true;
			sublookup->type = Lookup::singleadjustment;

			m_layout->addLookup(sublookup, true);

			SingleAdjustmentSubtable* singleadjsubtable = new SingleAdjustmentSubtable(sublookup);
			sublookup->subtables.append(singleadjsubtable);

			singleadjsubtable->name = sublookup->name;

			auto applied = classtoUnicode("(waw|reh)[.]" + finaisol);
			for (auto glyph : applied) {
				singleadjsubtable->singlePos[glyph] = { 0,0,0,0 };
			}
		}*/

		/*
		//rehwawcursive
		Lookup * sublookup = new Lookup(m_layout);
		sublookup->name = lookup->name + "." + finaisol + "rehwawcursive";
		sublookup->feature = "";
		sublookup->type = Lookup::cursive;
		sublookup->is_gsub = false;
		sublookup->local = true;
		sublookup->flags = Lookup::Flags::IgnoreMarks;

		m_layout->addLookup(sublookup, true);

		CursiveSubtable* cursivesubtable = new CursiveSubtable(lookup);
		sublookup->subtables.append(cursivesubtable);
		cursivesubtable->name = "cursivesubtable";

		auto& glyphcodes = m_layout->classtoUnicode("(waw|reh)[.]" + finaisol);

		for (auto glyphcode : glyphcodes) {
			cursivesubtable->anchors[glyphcode].exit = QPoint(0, 0);
		}

		glyphcodes = m_layout->classtoUnicode("isol|init"); //"((?<!reh|waw)[.]isol)|init"

		for (auto glyphcode : glyphcodes) {
			QString glyphName = m_layout->glyphNamePerCode[glyphcode];
			auto glyph = glyphs[glyphName];
			bool includeGlyph = true;
			for (auto anchors : exitAnchorsRTL) {
				if (anchors.contains(glyphcode)) {
					includeGlyph = false;
					break;
				}
			}

			if (includeGlyph) {
				cursivesubtable->anchors[glyphcode].entry = QPoint(glyph.width, 0);
			}

		}*/

		// beforehah
		ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->name = "chaining_" + finaisol;

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.input.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });
		newsubtable->compiledRule.input.append(classtoUnicode("hah[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["onedotdown"].charcode,(quint16)glyphs["twodotsdown"].charcode });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });

		//if (finaisol == "isol") {
		//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		//}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "beforehah" });
		newsubtable->compiledRule.lookupRecords.append({ 3,lookup->name + "." + finaisol + "1" });
		newsubtable->compiledRule.lookupRecords.append({ 4,lookup->name + "." + finaisol + "2" });


		//
		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->name = "chaining_" + finaisol;

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.backtrack.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.backtrack.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });
		newsubtable->compiledRule.input.append(classtoUnicode("[.]init|[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["onedotdown"].charcode,(quint16)glyphs["twodotsdown"].charcode });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });

		if (finaisol == "isol") {
			//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });		
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "1" });
		newsubtable->compiledRule.lookupRecords.append({ 2,lookup->name + "." + finaisol + "2" });

		//
		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.backtrack.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.input.append(classtoUnicode("[.]init|[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["onedotdown"].charcode,(quint16)glyphs["twodotsdown"].charcode });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });

		if (finaisol == "isol") {
			//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });		
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "1" });
		newsubtable->compiledRule.lookupRecords.append({ 2,lookup->name + "." + finaisol + "2" });

		// beforehah
		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.input.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });
		newsubtable->compiledRule.input.append(classtoUnicode("hah[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode,(quint16)glyphs["onedotdown"].charcode ,(quint16)glyphs["twodotsdown"].charcode });

		if (finaisol == "isol") {
			//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "beforehah" });
		newsubtable->compiledRule.lookupRecords.append({ 3,lookup->name + "." + finaisol + "1" });

		//
		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();
		newsubtable->compiledRule.backtrack.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.backtrack.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });
		newsubtable->compiledRule.input.append(classtoUnicode("[.]init|[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode,(quint16)glyphs["onedotdown"].charcode ,(quint16)glyphs["twodotsdown"].charcode });

		if (finaisol == "isol") {
			//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });		
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "1" });

		//
		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();
		newsubtable->compiledRule.backtrack.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.input.append(classtoUnicode("[.]init|[.]isol"));
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode, (quint16)glyphs["onedotdown"].charcode ,(quint16)glyphs["twodotsdown"].charcode });

		if (finaisol == "isol") {
			//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });		
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "1" });

		newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();
		newsubtable->compiledRule.input.append({ classtoUnicode("(waw|reh)[.]" + finaisol) });
		newsubtable->compiledRule.input.append({ (quint16)glyphs["kasra"].charcode,(quint16)glyphs["kasratan"].charcode,(quint16)glyphs["kasratanidgham"].charcode });
		newsubtable->compiledRule.lookahead.append(classtoUnicode("hah[.]isol|ain[.]isol"));

		if (finaisol == "isol") {
			//	newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "displac" });
		}
		//newsubtable->compiledRule.lookupRecords.append({ 0,lookup->name + "." + finaisol + "rehwawcursive" });		
		newsubtable->compiledRule.lookupRecords.append({ 1,lookup->name + "." + finaisol + "beforehah" });

	}
	//}

	return lookup;


}
Lookup * Automedina::tajweedcolorcpp() {

	Lookup * single = new Lookup(m_layout);
	single->name = "tajweedcolor.green";
	single->feature = "";
	single->type = Lookup::singleadjustment;
	m_layout->addLookup(single);

	ValueRecord green{ 99, 200, 77, 0 };
	ValueRecord gray{ 200, 200, 200, 0 };
	ValueRecord lkalkala{ 200, 200, 200, 0 };

	SingleAdjustmentSubtable* newsubtable = new SingleAdjustmentSubtable(single, 3);
	single->subtables.append(newsubtable);
	newsubtable->name = single->name;
	for (auto className : { "bases" ,"marks" }) {
		auto  unicodes = m_layout->classtoUnicode(className);
		for (auto unicode : unicodes) {

			newsubtable->singlePos[unicode] = green;
		}
	}

	single = new Lookup(m_layout);
	single->name = "tajweedcolor.lgray";
	single->feature = "";
	single->type = Lookup::singleadjustment;
	m_layout->addLookup(single);

	newsubtable = new SingleAdjustmentSubtable(single, 3);
	single->subtables.append(newsubtable);
	newsubtable->name = single->name;
	for (auto className : { "^meem|^behshape|onedotup|^noon" ,"marks" }) {
		auto  unicodes = m_layout->classtoUnicode(className);
		for (auto unicode : unicodes) {

			newsubtable->singlePos[unicode] = gray;
		}
	}

	single = new Lookup(m_layout);
	single->name = "tajweedcolor.lkalkala";
	single->feature = "";
	single->type = Lookup::singleadjustment;
	m_layout->addLookup(single);

	newsubtable = new SingleAdjustmentSubtable(single, 3);
	single->subtables.append(newsubtable);
	newsubtable->name = single->name;
	for (auto className : { "^tah|^behshape|^dal|^hah|^kaf|^fehshape" ,"marks" }) {
		auto  unicodes = m_layout->classtoUnicode(className);
		for (auto unicode : unicodes) {
			newsubtable->singlePos[unicode] = lkalkala;
		}
	}


	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "tajweedcolor";
	lookup->feature = "mkmk";
	lookup->type = Lookup::chainingpos;
	lookup->flags = 0;

	//tajweedcolor_meemiqlab1
	ChainingSubtable* chainingsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(chainingsubtable);

	chainingsubtable->name = "tajweedcolor_meemiqlab1";

	chainingsubtable->compiledRule = ChainingSubtable::CompiledRule();

	chainingsubtable->compiledRule.input.append(classtoUnicode("^noon"));
	chainingsubtable->compiledRule.input.append(classtoUnicode("meemiqlab"));

	chainingsubtable->compiledRule.lookupRecords.append({ 0,"lgray" });
	chainingsubtable->compiledRule.lookupRecords.append({ 1,"green" });

	//tajweedcolor_meemiqlab2
	chainingsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(chainingsubtable);

	chainingsubtable->name = "tajweedcolor_meemiqlab2";

	chainingsubtable->compiledRule = ChainingSubtable::CompiledRule();

	chainingsubtable->compiledRule.input.append(classtoUnicode("^behshape"));
	chainingsubtable->compiledRule.input.append(classtoUnicode("onedotup"));
	chainingsubtable->compiledRule.input.append(classtoUnicode("meemiqlab"));

	chainingsubtable->compiledRule.lookupRecords.append({ 0,"lgray" });
	chainingsubtable->compiledRule.lookupRecords.append({ 1,"lgray" });
	chainingsubtable->compiledRule.lookupRecords.append({ 2,"green" });

	//tajweedcolor_meemnoon
	chainingsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(chainingsubtable);

	chainingsubtable->name = "tajweedcolor_meemnoon";

	chainingsubtable->compiledRule = ChainingSubtable::CompiledRule();

	chainingsubtable->compiledRule.input.append(classtoUnicode("^meem|^noon"));
	chainingsubtable->compiledRule.input.append(classtoUnicode("shadda"));
	chainingsubtable->compiledRule.input.append(classtoUnicode("marks"));

	chainingsubtable->compiledRule.lookupRecords.append({ 0,"green" });
	chainingsubtable->compiledRule.lookupRecords.append({ 1,"green" });
	chainingsubtable->compiledRule.lookupRecords.append({ 2,"green" });


	return lookup;


}
Lookup * Automedina::pointmarks() {

	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "pointmarks";
	lookup->feature = "mark";
	lookup->type = Lookup::chainingpos;
	lookup->flags = 0;
	//lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode });
	//lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;

	for (auto pointmark : classes["dotmarks"]) {

		QString sublookupName = pointmark;

		Lookup * sublookup = new Lookup(m_layout);
		sublookup->name = lookup->name + "." + sublookupName;
		sublookup->feature = "";
		sublookup->type = Lookup::mark2base;
		sublookup->flags = 0;

		m_layout->addLookup(sublookup);

		MarkBaseSubtable* marksubtable = new MarkBaseSubtable(sublookup);
		sublookup->subtables.append(marksubtable);

		marksubtable->name = sublookup->name;
		marksubtable->base = { "bases" };

		marksubtable->classes["topmarks"].mark = { "topmarks" };
		marksubtable->classes["topmarks"].basefunction = new Defaulbaseanchorfortop(*this, *marksubtable);
		marksubtable->classes["topmarks"].markfunction = new Defaultopmarkanchor(*this, *marksubtable);


		marksubtable->classes["lowmarks"].mark = { "lowmarks" };
		marksubtable->classes["lowmarks"].basefunction = new Defaulbaseanchorforlow(*this, *marksubtable);
		marksubtable->classes["lowmarks"].markfunction = new Defaullowmarkanchor(*this, *marksubtable);


		ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
		lookup->subtables.append(newsubtable);



		newsubtable->name = "pointmarks_" + pointmark;

		newsubtable->compiledRule = ChainingSubtable::CompiledRule();

		newsubtable->compiledRule.backtrack.append({ classtoUnicode("bases") });
		newsubtable->compiledRule.input.append({ (quint16)glyphs[pointmark].charcode });
		newsubtable->compiledRule.input.append(classtoUnicode("marks"));

		newsubtable->compiledRule.lookupRecords.append({ 1,sublookupName });
	}



	return lookup;


}

Lookup * Automedina::ayanumbers() {

	// ligature 
	Lookup * ligature = new Lookup(m_layout);
	ligature->name = "ayanumbers.l1";
	ligature->feature = "";
	ligature->type = Lookup::ligature;
	m_layout->addLookup(ligature);

	LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
	ligature->subtables.append(ligaturesubtable);
	ligaturesubtable->name = ligature->name;

	for (quint16 i = 286; i > 99; i--) {
		quint16 code = m_layout->glyphCodePerName[QString("aya%1").arg(i)];
		if (i < 100) {
			int onesdigit = i % 10;
			int tensdigit = i / 10;
			ligaturesubtable->ligatures.append({ code, { (quint16)(1632 + tensdigit),(quint16)(1632 + onesdigit) } });
		}
		else {
			int onesdigit = i % 10;
			int tensdigit = (i / 10) % 10;
			int hundredsdigit = i / 100;
			ligaturesubtable->ligatures.append({ code, { (quint16)(1632 + hundredsdigit),(quint16)(1632 + tensdigit),(quint16)(1632 + onesdigit) } });
		}
	}

	// ligature 
	ligature = new Lookup(m_layout);
	ligature->name = "ayanumbers.l2";
	ligature->feature = "";
	ligature->type = Lookup::ligature;
	m_layout->addLookup(ligature);

	ligaturesubtable = new LigatureSubtable(ligature);
	ligature->subtables.append(ligaturesubtable);
	ligaturesubtable->name = ligature->name;

	for (quint16 i = 99; i > 9; i--) {
		quint16 code = m_layout->glyphCodePerName[QString("aya%1").arg(i)];
		if (i < 100) {
			int onesdigit = i % 10;
			int tensdigit = i / 10;
			ligaturesubtable->ligatures.append({ code,{ (quint16)(1632 + tensdigit),(quint16)(1632 + onesdigit) } });
		}
		else {
			int onesdigit = i % 10;
			int tensdigit = (i / 10) % 10;
			int hundredsdigit = i / 100;
			ligaturesubtable->ligatures.append({ code,{ (quint16)(1632 + hundredsdigit),(quint16)(1632 + tensdigit),(quint16)(1632 + onesdigit) } });
		}
	}

	// Single substitution
	Lookup * single = new Lookup(m_layout);
	single->name = "ayanumbers.l3";
	single->feature = "";
	single->type = Lookup::single;
	m_layout->addLookup(single);

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	for (int i = 1; i < 10; i++) {
		singlesubtable->subst[(quint16)(1632 + i)] = glyphs[QString("aya%1").arg(i)].charcode;
	}

	//main lokkup



	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "ayanumbers";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->flags = 0;
	//lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode });
	//lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;

	auto digitySet = classtoUnicode("digits");


	ChainingSubtable* subtable = new ChainingSubtable(lookup);
	lookup->subtables.append(subtable);
	subtable->name = "ayanumbers3digits";
	subtable->compiledRule = ChainingSubtable::CompiledRule();
	subtable->compiledRule.input = { digitySet,digitySet,digitySet };
	subtable->compiledRule.lookupRecords.append({ 0,"l1" });

	subtable = new ChainingSubtable(lookup);
	lookup->subtables.append(subtable);
	subtable->name = "ayanumbers2digits";
	subtable->compiledRule = ChainingSubtable::CompiledRule();
	subtable->compiledRule.input = { digitySet,digitySet };
	subtable->compiledRule.lookupRecords.append({ 0,"l2" });

	subtable = new ChainingSubtable(lookup);
	lookup->subtables.append(subtable);
	subtable->name = "ayanumbers1digit";
	subtable->compiledRule = ChainingSubtable::CompiledRule();
	subtable->compiledRule.input = { digitySet };
	subtable->compiledRule.lookupRecords.append({ 0,"l3" });

	return lookup;

}
Lookup * Automedina::forheh() {

	Lookup * single = new Lookup(m_layout);
	single->name = "forheh.l1";
	single->feature = "";
	single->type = Lookup::single;
	m_layout->addLookup(single);

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;


	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + 2) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + 2) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}


	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "forheh";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->flags = 0;
	//lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode });
	lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;

	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forheh";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	//newsubtable->compiledRule.input.append({ (quint16)glyphs["heh.medi"].charcode,(quint16)glyphs["heh.medi.forsmalllalef"].charcode });
	newsubtable->compiledRule.lookahead.append(classtoUnicode("^heh.medi")); //  { (quint16)glyphs["heh.medi"].charcode });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });

	return lookup;

}
Lookup * Automedina::forhamza() {

	Lookup * single = new Lookup(m_layout);
	single->name = "forhamza.l1";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	int tatweel = 2;

	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}

	// ligature 
	Lookup * ligature = new Lookup(m_layout);
	ligature->name = "forhamza.l2";
	ligature->feature = "";
	ligature->type = Lookup::ligature;
	//ligature->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["hamzaabove"].charcode,(quint16)glyphs["smallhighyeh"].charcode,(quint16)glyphs["smallhighwaw"].charcode ,(quint16)glyphs["smallhighnoon"].charcode });
	//ligature->flags = ligature->flags | Lookup::Flags::UseMarkFilteringSet;
	m_layout->addLookup(ligature);

	LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
	ligature->subtables.append(ligaturesubtable);
	ligaturesubtable->name = ligature->name;

	ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove"].charcode,{ (quint16)glyphs["hamzaabove"].charcode  , 0x200D } });
	ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove.joined"].charcode,{ 0x200D,(quint16)glyphs["hamzaabove"].charcode } });
	ligaturesubtable->ligatures.append({ (quint16)glyphs["hamzaabove.joined"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["hamzaabove"].charcode } });
	ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighyeh"].charcode,{ 0x200D,(quint16)glyphs["smallhighyeh"].charcode } });
	ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighyeh"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighyeh"].charcode } });
	//ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ 0x200D,(quint16)glyphs["smallhighwaw"].charcode } });
	//ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighwaw"].charcode } });

	ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighnoon"].charcode,{ 0x200D,(quint16)glyphs["smallhighnoon"].charcode } });
	ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighnoon"].charcode,{ (quint16)glyphs["tatweel"].charcode,(quint16)glyphs["smallhighnoon"].charcode } });


	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "forhamza";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->markGlyphSetIndex = m_layout->addMarkSet({
		//(quint16)glyphs["smallhighwaw"].charcode,
		(quint16)glyphs["hamzaabove"].charcode,
		(quint16)glyphs["smallhighyeh"].charcode,
		(quint16)glyphs["smallhighnoon"].charcode,
		(quint16)glyphs["roundedfilledhigh"].charcode
		});
	lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;
	//lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;

	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forhamza";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ 0x200D ,  (quint16)glyphs["tatweel"].charcode });
	newsubtable->compiledRule.input.append({ (quint16)glyphs["hamzaabove"].charcode,(quint16)glyphs["smallhighyeh"].charcode, (quint16)glyphs["smallhighnoon"].charcode });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
	newsubtable->compiledRule.lookupRecords.append({ 1,"l2" });


	//roundedfilledhigh
	newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "roundedfilledhigh";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ (quint16)glyphs["roundedfilledhigh"].charcode, });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });


	return lookup;

}
Lookup* Automedina::shrinkstretchlt() {



	Lookup* lookup;
	int count = 1;
	for (float i = -0.1; i >= -0.7; i = i - 0.1) {
		lookup = shrinkstretchlt(i, QString("shr%1").arg(count));
		m_layout->addLookup(lookup);
		count++;
	}

	return nullptr;

}
Lookup * Automedina::shrinkstretchlt(float lt, QString featureName) {



	//m_layout->addLookup(forwaw(), false);

	QString lookupName;

	if (lt < 0) {
		lookupName = QString("minuslt_%1").arg(lt * -100);
	}
	else {
		lookupName = QString("pluslt_%1").arg(lt * -100);
	}

	Lookup * single = new Lookup(m_layout);
	single->name = lookupName + ".l1";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	for (auto& glyph : glyphs) {
		//QRegularExpression reg2("beginchar\\((.*?),(.*?),(.*?),(.*?)\\);");
		QRegularExpression regname("(.*)[.](minuslt|pluslt)_(.*)");
		QRegularExpressionMatch match = regname.match(glyph.name);
		if (match.hasMatch()) {
			QString name = match.captured(1);
			QString plusminus = match.captured(2);
			int value = match.captured(3).toInt();
		}
		else if (classes["haslefttatweel"].contains(glyph.name)) {
			if (lt < 0) {
				QString destName = QStringLiteral("%1.minuslt_%2").arg(glyph.name).arg((int)(lt * -100));
				if (glyphs.contains(destName)) {
					singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
				}
			}
			else {
				QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)(lt * 100));
				if (glyphs.contains(destName)) {
					singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
				}
			}

		}
		/*
		if (match.hasMatch()) {
			int w1 = match.captured(1).toInt();
			double w2 = match.captured(2).toDouble();

		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.minuslt_%2").arg(glyph.name).arg((int)(lt * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt - shrink) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}*/
	}


	Lookup * lookup = new Lookup(m_layout);
	lookup->name = lookupName;
	lookup->feature = featureName;
	lookup->type = Lookup::chainingsub;
	lookup->flags = 0;


	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = lookupName;

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });

	return lookup;

}
Lookup * Automedina::forsmallhighwaw() {

	Lookup * single = new Lookup(m_layout);
	single->name = "forsmallhighwaw.l1";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	float tatweel = 1;

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}

	// ligature 
	Lookup * ligature = new Lookup(m_layout);
	ligature->name = "forsmallhighwaw.l2";
	ligature->feature = "";
	ligature->type = Lookup::ligature;
	m_layout->addLookup(ligature);

	LigatureSubtable* ligaturesubtable = new LigatureSubtable(ligature);
	ligature->subtables.append(ligaturesubtable);
	ligaturesubtable->name = ligature->name;

	ligaturesubtable->ligatures.append({ (quint16)glyphs["smallhighwaw"].charcode,{ 0x034F,(quint16)glyphs["smallhighwaw"].charcode } });

	//main lookup
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "forsmallhighwaw";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->markGlyphSetIndex = m_layout->addMarkSet(QList{ (quint16)glyphs["smallhighwaw"].charcode });
	lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;



	//forsmallalefwithmaddah
	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "subtable1";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ 0x034F });
	newsubtable->compiledRule.input.append({ (quint16)glyphs["smallhighwaw"].charcode });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
	newsubtable->compiledRule.lookupRecords.append({ 1,"l2" });



	return lookup;

}
Lookup * Automedina::forsmalllalef() {

	Lookup * single = new Lookup(m_layout);
	single->name = "forsmallalef.l1";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	float tatweel = 1;

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}

	singlesubtable->subst[glyphs["smallalef"].charcode] = glyphs["smallalef.joined"].charcode;

	//followed by maddahabove
	single = new Lookup(m_layout);
	single->name = "forsmallalef.l2";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	tatweel = 2;

	singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}

	//main lookup
	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "forsmallalef";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->flags = 0;
	lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode , (quint16)glyphs["maddahabove"].charcode });
	lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;



	//forsmallalefwithmaddah
	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forsmallalefwithmaddah";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ (quint16)glyphs["smallalef"].charcode });
	newsubtable->compiledRule.input.append({ (quint16)glyphs["maddahabove"].charcode });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l2" });
	newsubtable->compiledRule.lookupRecords.append({ 1,"l1" });

	//forsmallalef
	newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forsmallalef";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ (quint16)glyphs["smallalef"].charcode });


	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
	newsubtable->compiledRule.lookupRecords.append({ 1,"l1" });

	//forsmallalef
	newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forsmallalef";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ (quint16)glyphs["maddahabove"].charcode });
	newsubtable->compiledRule.input.append({ (quint16)glyphs["smallalef"].charcode });


	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });
	newsubtable->compiledRule.lookupRecords.append({ 2,"l1" });



	return lookup;

}
QSet<quint16> Automedina::regexptoUnicode(QString regexp) {

	QSet<quint16> unicodes;

	QRegularExpression re(regexp);
	for (auto& glyph : m_layout->glyphs) {
		if (re.match(glyph.name).hasMatch()) {
			unicodes.insert(glyph.charcode);
		}
	}

	return unicodes;
}
QSet<quint16> Automedina::classtoUnicode(QString className) {



	if (cachedClasstoUnicode.contains(className)) {
		return cachedClasstoUnicode[className];
	}

	QSet<quint16> unicodes;

	if (!classes.contains(className)) {
		if (m_layout->glyphs.contains(className)) {
			auto& glyph = m_layout->glyphs[className];
			unicodes.insert(glyph.charcode);
		}
		else {
			bool ok;
			quint16 uniode = className.toUInt(&ok, 16);
			if (!ok) {
				QRegularExpression re(className);
				for (auto& glyph : m_layout->glyphs) {
					if (re.match(glyph.name).hasMatch()) {
						unicodes.insert(glyph.charcode);
					}
				}
			}
			else {
				unicodes.insert(uniode);
			}

		}
	}
	else {

		for (auto name : classes[className]) {
			unicodes.unite(classtoUnicode(name));
		}
	}

	cachedClasstoUnicode[className] = unicodes;
	return unicodes;
}

Lookup * Automedina::forwaw() {

	Lookup * single = new Lookup(m_layout);
	single->name = "forwaw.l1";
	single->feature = "";
	single->type = Lookup::single;

	m_layout->addLookup(single);

	SingleSubtable* singlesubtable = new SingleSubtable(single);
	single->subtables.append(singlesubtable);
	singlesubtable->name = single->name;

	float tatweel = 1;

	for (auto& glyph : glyphs) {
		if (classes["haslefttatweel"].contains(glyph.name)) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.name).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
		else if (classes["haslefttatweel"].contains(glyph.originalglyph) && glyph.name.contains("pluslt")) {
			QString destName = QStringLiteral("%1.pluslt_%2").arg(glyph.originalglyph).arg((int)((glyph.charlt + tatweel) * 100));
			if (glyphs.contains(destName)) {
				singlesubtable->subst[glyphs[glyph.name].charcode] = glyphs[destName].charcode;
			}
		}
	}


	Lookup * lookup = new Lookup(m_layout);
	lookup->name = "forwaw";
	lookup->feature = "rlig";
	lookup->type = Lookup::chainingsub;
	lookup->flags = 0;
	//lookup->markGlyphSetIndex = m_layout->addMarkSet({ (quint16)glyphs["smallalef"].charcode });
	//lookup->flags = lookup->flags | Lookup::Flags::UseMarkFilteringSet;
	lookup->flags = lookup->flags | Lookup::Flags::IgnoreMarks;

	ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
	lookup->subtables.append(newsubtable);

	newsubtable->name = "forwaw";

	newsubtable->compiledRule = ChainingSubtable::CompiledRule();

	newsubtable->compiledRule.input.append(singlesubtable->subst.keys().toSet());
	newsubtable->compiledRule.input.append({ (quint16)glyphs["waw.fina"].charcode });

	newsubtable->compiledRule.lookupRecords.append({ 0,"l1" });

	return lookup;
}

QSet<QString> Automedina::classtoGlyphName(QString className) {

	QSet<QString> names;
	//TODO use classtoUnicode
	if (!classes.contains(className)) {
		if (m_layout->glyphs.contains(className)) {
			auto& glyph = m_layout->glyphs[className];
			names.insert(glyph.name);
		}
		else {
			QRegularExpression re(className);
			for (auto& glyph : m_layout->glyphs) {
				if (re.match(glyph.name).hasMatch()) {
					names.insert(glyph.name);
				}
			}
		}
	}
	else {

		for (auto name : classes[className]) {
			names.unite(classtoGlyphName(name));
		}
	}

	return names;

}
void Automedina::addchars() {

	/*
	for (auto key : classes["haslefttatweel"]) {
		QString name = QStringLiteral("%1.beforeheh").arg(key);
		addchar(key, -1, 200 / 100.0, {}, {}, {}, {}, {}, name, 2);
	}*/
	/*
	for (auto key : classes["haslefttatweel"]) {
		for (float i = 100; i <= 500; i = i + 100) {
			QString name = QStringLiteral("%1.pluslt_%2").arg(key).arg((int)(i * 1));
			addchar(key, -1, i / 100.0, {}, {}, {}, {}, {}, name, 2);
		}

		
		// for (float i = 10; i <= 200; i = i + 10) {
		//	QString name = QStringLiteral("%1.minuslt_%2").arg(key).arg((int)(i * 1));
		//	addchar(key, -1, -i / 100.0, {}, {}, {}, {}, {}, name, 2);
		//}

		QString metapostcode = QString("vardef %1.pluslt_[]_(expr lt,rt) = %1_(lt,rt);enddef;").arg(key);

		QByteArray commandBytes = metapostcode.toLatin1();

		int status = mp_execute(mp, commandBytes.data(), commandBytes.size());
		if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
			mp_run_data * results = mp_rundata(mp);
			QString ret(results->term_out.data);
			ret.trimmed();
			mp_finish(mp);
			throw "Could not initialize MetaPost library instance!\n" + ret;
		}
	}*/

	//behshape.init.minuslt_110
	//addchar("lam.medi", -1, {}, -0.2, {}, {}, {}, {}, "lam.medi.afterhah", 2);
	addchar("behshape.init", -1, -110 / 100.0, {}, {}, {}, {}, {}, "behshape.init.minuslt_110", 2);



}
void Automedina::addchar(QString macroname,
	int charcode,
	double lefttatweel,
	double righttatweel,
	optional<double> leftextratio,
	optional<double> rightextratio,
	optional<double> left_tatweeltension,
	optional<double> right_tatweeltension,
	QString newname,
	optional<int> which_in_baseline) {

	QString metapostcode = QString("beginchar(%1,%2,-1,-1,-1);").arg(newname).arg(charcode);

	if (leftextratio) {
		metapostcode = metapostcode + QString("interim left_verticalextratio := %1 ;").arg(*leftextratio);
	}
	if (rightextratio) {
		metapostcode = metapostcode + QString("interim right_verticalextratio := %1 ;").arg(*rightextratio);
	}
	if (left_tatweeltension) {
		metapostcode = metapostcode + QString("interim left_tatweeltension := %1 ;").arg(*left_tatweeltension);
	}
	if (right_tatweeltension) {
		metapostcode = metapostcode + QString("interim right_tatweeltension := %1 ;").arg(*right_tatweeltension);
	}
	if (which_in_baseline) {
		metapostcode = metapostcode + QString("interim which_in_baseline := %1 ;").arg(*which_in_baseline);
	}

	QString tst =  QString("%1").arg(lefttatweel);
	QString tst2 = QString("%1").arg(righttatweel); // metapostcode + QString("%1").number(0.10764366596638640, 'g', 6);

	metapostcode = metapostcode + QString("%1_(%2,%3);endchar;").arg(macroname).arg(lefttatweel).arg(righttatweel);

	

	QByteArray commandBytes = metapostcode.toLatin1();

	int status = mp_execute(mp, commandBytes.data(), commandBytes.size());
	if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
		mp_run_data * results = mp_rundata(mp);
		QString ret(results->term_out.data);
		ret.trimmed();
		mp_finish(mp);
		throw "Could not initialize MetaPost library instance!\n" + ret;
	}




}


