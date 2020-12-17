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
#include "qhash.h"
#include "qset.h"
#include "qmap.h"
#include <optional>
#include "qpoint.h"
#include "OtLayout.h"

class LayoutWindow;

class Automedina {
	friend class OtLayout;
	friend class AnchorCalc;
	friend class LayoutWindow;
	friend class GlyphVis;
	friend class ToOpenType;

public : 
	//static const quint16 AyaNumberCode = 65200;
	static const quint16 AyaNumberCode = 0xE000;
	const int markheigh = 500;
	const int markdepth = 80;
	const int spacemkmk = 100;
	const int spacebasetotopmark = 100;
	const int shaddamarkheight = markheigh - 250;
	const int spacebasetobottommark = 50;
	const int minwaqfhigh = 900;

public:
	Automedina(OtLayout * layout,MP mp, bool extended);
	Lookup* getLookup(QString lookupName);	
	QSet<quint16> classtoUnicode(QString className);
	QSet<quint16> regexptoUnicode(QString regexp);
	QSet<QString> classtoGlyphName(QString className);

	QHash<QString, GlyphVis>& glyphs;
	CalcAnchor  getanchorCalcFunctions(QString functionName, Subtable * subtable);

private:
	OtLayout * m_layout;
	

	Lookup * defaultmarkposition();
	Lookup * defaultwaqfmarktobase();
	Lookup * forsmalllalef();
	Lookup * forsmallhighwaw();
	Lookup * forhamza();
	Lookup * forheh();	
	Lookup * forwaw();
	Lookup * leftrightcursive();
	Lookup * ligaturecursive();
	Lookup * pointmarks();
	Lookup * defaultwaqfmarkabovemarkprecise();
	Lookup * defaultdotmarks();
	Lookup* defaultmarkdotmarks();
	Lookup* defaultmkmk();
	Lookup* ayanumbers();
	Lookup* ayanumberskern();
	Lookup * rehwawcursive();
	Lookup * lowmarkafterwawandreh();
	Lookup* tajweedcolorcpp();
	//Justification
	Lookup* shrinkstretchlt(float lt, QString featureName);
	Lookup* shrinkstretchlt();
	QHash<QString, QSet<QString>> classes;
	//void setAnchorCalcFunctions();
	void addchar(QString macroname,
		int charcode,
		double lefttatweel,
		double righttatweel,
		std::optional<double> leftextratio,
		std::optional<double> rightextratio,
		std::optional<double> left_tatweeltension,
		std::optional<double> right_tatweeltension,
		QString newname,
		std::optional<int> which_in_baseline);
	void addchars();
	void generateGlyphs();
	QMap<QString, QSet<quint16>> cachedClasstoUnicode;
	//QMap<QString, AnchorCalc*> anchorCalcFunctions;
	

	QSet<QString> initchar;
	QSet<QString> medichar;
	
	MP mp;

	QMap <QString, QMap<quint16, QPoint>> markAnchors;
	QMap <QString, QMap<quint16, QPoint>> entryAnchors;
	QMap <QString, QMap<quint16, QPoint>> exitAnchors;
	QMap <QString, QMap<quint16, QPoint>> entryAnchorsRTL;
	QMap <QString, QMap<quint16, QPoint>> exitAnchorsRTL;

	 bool extended;

};
