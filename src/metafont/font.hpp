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
#ifndef FONT_H
#define FONT_H
#include <QObject>
#include <QVector>
#include <QHash>




class OtLayout;
class GlyphVis;
class Glyph;
typedef struct MP_instance* MP;
struct mp_edge_object;

class Font : public QObject {
	Q_OBJECT

public:
	friend OtLayout;
	friend GlyphVis;
	Font(QObject* parent = Q_NULLPTR);
	~Font();
	bool loadFile(const QString& fileName);
	bool saveFile();
	QVector<Glyph*> glyphs;
	QHash<QString, Glyph*> glyphperName;
	QString filePath();
	double lineHeight();
	double getNumericVariable(QString name);
	Glyph* getGlyph(uint code);	
	QString path() {
		return m_path;
	}

	//protected:
	MP mp;


private:
	QString m_path;
};
#endif // FONT_H
