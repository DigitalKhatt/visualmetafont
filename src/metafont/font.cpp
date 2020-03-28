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
# include "mplibps.h"
# include "mplibsvg.h"
}

#include "qfile.h"
#include "font.hpp"
#include "glyph.hpp"

#include "hb.hh"
#include "mpmp.h"
#include "qtextstream.h"
#include "qregularexpression.h"
#include "qapplication.h"
#include "qfileinfo.h"

Font::Font(QObject * parent) : QObject(parent) {
	MP_options * _mp_options = mp_options();
	//MP_options _mp_options;
	_mp_options->noninteractive = 1;
	_mp_options->command_line = NULL;
	_mp_options->ini_version = true;
	_mp_options->math_mode = mp_math_double_mode;
	//_mp_options->interaction = mp_nonstop_mode;
	//_mp_options->mem_name = "plain";
	//_mp_options->mem_name = "automedina";
	//_mp_options -> main_memory = 1000000;

	mp = mp_initialize(_mp_options);

	if (!mp) exit(EXIT_FAILURE);

	if (!mp) throw "Could not initialize MetaPost library instance!";

	QByteArray commandBytes = "MPGUI:=1;input mpguifont.mp;";

	int status = mp_execute(mp, commandBytes.data(), commandBytes.size());
	if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
		mp_run_data * results = mp_rundata(mp);
		QString ret(results->term_out.data);
		ret = ret.trimmed();
		mp_finish(mp);
		throw "Could not initialize MetaPost library instance!\n" + ret;
	}
	
}
bool Font::loadFile(const QString &fileName) {
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return false;
	}

	QTextStream in(&file);
	QApplication::setOverrideCursor(Qt::WaitCursor);

	QString code = in.readAll();

	QRegularExpression re("((?:beginchar|defchar)(.*?)(?:enddefchar|endchar);)", QRegularExpression::DotMatchesEverythingOption);
	QRegularExpressionMatchIterator i = re.globalMatch(code);
	while (i.hasNext()) {
		QRegularExpressionMatch match = i.next();
		QString source = match.captured(1);
		Glyph* glyph = new Glyph(source, mp, this);
		glyphs.append(glyph);
		//glyphperUnicode[glyph->unicode()] = glyph;
	}

	QFileInfo fileInfo(fileName);

	m_path = fileInfo.absoluteFilePath();
	
	file.close();

	return true;
}
double Font::lineHeight() {


	double lineheight;
	QString command("show lineheight;");

	QByteArray commandBytes = command.toLatin1();
	mp->history = mp_spotless;
	int status = mp_execute(mp, (char *)commandBytes.constData(), commandBytes.size());
	mp_run_data * results = mp_rundata(mp);
	QString ret(results->term_out.data);
	ret = ret.trimmed();
	if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
		mp_finish(mp);
		throw "Could not get lineHeight !\n" + ret;
	}
	else {
		lineheight = ret.mid(3).toDouble();
	}

	return lineheight;
}
double Font::getNumericVariable(QString name) {
	double value;
	QString command("show " + name + ";");

	QByteArray commandBytes = command.toLatin1();
	mp->history = mp_spotless;
	int status = mp_execute(mp, (char *)commandBytes.constData(), commandBytes.size());
	mp_run_data * results = mp_rundata(mp);
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
bool Font::saveFile() {
	if (!m_path.isEmpty()) {

		QFileInfo fileInfo(m_path);
		QFile file(m_path);

		QString unicodefileName = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_unicodes.lua";

		QFile unicodesfile(unicodefileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return false;

		if (!unicodesfile.open(QIODevice::WriteOnly | QIODevice::Text))
			return false;

		QTextStream out(&file);
		QTextStream outunicode(&unicodesfile);

		outunicode << fileInfo.baseName() << ".unicodes = {\n";

		for (int i = 0; i < glyphs.length(); i++) {
			out << glyphs[i]->source();
			outunicode << "  [\"" << glyphs[i]->name() << "\"] = " << glyphs[i]->charcode() << ",\n";
		}

		outunicode << "}";


		file.close();
		unicodesfile.close();

		/*
		QString anchorfileName = "saveAnchors(\"" + fileInfo.baseName() + "\"," + "\"" + fileInfo.baseName() + "_anchors.lua" + "\");";

		QByteArray commandBytes = anchorfileName.toLatin1();

		int status = mp_execute(mp, commandBytes.data(), commandBytes.size());
		if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
			mp_run_data * results = mp_rundata(mp);
			QString ret(results->term_out.data);
			ret = ret.trimmed();
			mp_finish(mp);
			throw "Error when executing saveAnchors()!\n" + ret;
		}*/

		return true;

	}

	return false;
}

Font::~Font() {
	mp_finish(mp);
}

QString Font::filePath() {
	return m_path;
}
Glyph* Font::getGlyph(uint charcode) {
	
	for (QVector<Glyph*>::const_iterator it = glyphs.begin(); it != glyphs.end(); ++it) {

		Glyph* cur = *it;
		if (cur->charcode() == (int)charcode) {
			return cur;
		}
	}

	return NULL;
		
}



