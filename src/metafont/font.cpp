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

#include "qfile.h"
#include "font.hpp"
#include "glyph.hpp"



#include "qtextstream.h"
#include "qregularexpression.h"
#include "qapplication.h"
#include "qfileinfo.h"

#include "hb.hh"
#include "metafont.h"
#include <filesystem>

namespace fs = std::filesystem;

Font::Font(QObject* parent) : QObject(parent) {

}
bool Font::loadFile(const QString& fileName) {

  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    return false;
  }

  if (mp != nullptr) {
    mp_finish(mp);
  }

  MP_options* _mp_options = mp_options();
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

  fs::path p1 = fileName.toStdString();

  std::filesystem::current_path(p1.parent_path()); //setting path

  QByteArray commandBytes = "MPGUI:=1;input mpguifont.mp;";

  int status = mp_execute(mp, commandBytes.data(), commandBytes.size());
  if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
    mp_run_data* results = mp_rundata(mp);
    QString ret(results->term_out.data);
    ret = ret.trimmed();
    mp_finish(mp);
    throw "Could not initialize MetaPost library instance!\n" + ret;
  }

  if (mp->job_name != nullptr) {
    mp_xfree(mp->job_name);
  }

  auto path = fileName.toStdString();
  std::string base_filename = path.substr(path.find_last_of("/\\") + 1);
  std::string::size_type const p(base_filename.find_last_of('.'));
  std::string job_name = base_filename.substr(0, p);

  mp->job_name = xstrdup(job_name.c_str());

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
  int status = mp_execute(mp, (char*)commandBytes.constData(), commandBytes.size());
  mp_run_data* results = mp_rundata(mp);
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
bool Font::getPairVariable(QString name, QPointF& point) {
  QPointF value;
  /*

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
    QRegExp regexp("\\(([^,]+), ([^)]+)\\)");
    if (regexp.exactMatch(name)) {
      value.setX(regexp.capturedTexts()[1].toDouble());
      value.setY(regexp.capturedTexts()[2].toDouble());
    }
    else {
      //throw "Could not get " + name + " !\n" + ret;
    }
  }*/


  if (name[0] == 'z' && (name.size() == 1 || name[1].isDigit() || name[1] == '.')) {    
    double x, y = 0;
    name[0] = 'x';
    QByteArray ba = name.toLocal8Bit();
    char* c_str2 = ba.data();    
    if (!getMPNumVariable(mp, c_str2, &x)) {
      return false;
    }
    name[0] = 'y';
    ba = name.toLocal8Bit();
    c_str2 = ba.data();
    if (!getMPNumVariable(mp, c_str2, &y)) {
      return false;
    }
    point =  QPointF(x, y);
    return true;

  }
  else {
    QByteArray ba = name.toLocal8Bit();
    char* c_str2 = ba.data();
    double x, y;
    if (!getMPPairVariable(mp, c_str2, &x, &y)) {
      return false;
    }
    point =  QPointF(x, y);
    return true;
  }
  return false;
}
bool Font::saveFile() {
  if (!m_path.isEmpty()) {

    QFileInfo fileInfo(m_path);
    QFile file(m_path);

    QString unicodefileName = fileInfo.absolutePath() + "/output/" + fileInfo.baseName() + "_unicodes.lua";

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

    /*
    QMap<int, int> testMap;

    for (int i = 0; i < glyphs.length(); i++) {
      if (glyphs[i]->unicode() != -1) {
        testMap.insert(glyphs[i]->unicode(), glyphs[i]->unicode());
      }
    }

    for (int i = 0; i < glyphs.length(); i++) {
      if (glyphs[i]->unicode() != -1) {
        auto value = testMap[glyphs[i]->unicode()];
        outunicode << "\ncharProps[" << QString("0x%1").arg(value, 4, 16, QLatin1Char('0')).toUpper() << "] = {"<< glyphs[i]->getPath().toSubpathPolygons().count() << "}; // " << glyphs[i]->name();
      }

    }*/



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
  if (mp != nullptr) {
    mp_finish(mp);
  }
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
QString Font::executeMetaPost(QString command) {    

  QByteArray commandBytes = command.toLatin1();
  //mp->history = mp_spotless;
  int status = mp_execute(mp, (char*)commandBytes.constData(), commandBytes.size());
  mp_run_data* results = mp_rundata(mp);
  QString ret(results->term_out.data);
  ret.trimmed();
  if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
    throw "Could not get charcode !\n" + ret;
  }

  return ret;

}




