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
  _mp_options->job_name = (char*)"VisualMetaFont";
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

  mp->job_name = strdup(job_name.c_str());

  m_fontName = mp->job_name;

  QTextStream in(&file);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString code = in.readAll();

  QRegularExpression re("((?:beginchar|defchar)(.*?)(?:enddefchar|endchar);)", QRegularExpression::DotMatchesEverythingOption);
  QRegularExpressionMatchIterator i = re.globalMatch(code);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    QString source = match.captured(1);
    Glyph* glyph = new Glyph(source, this);
    glyphs.append(glyph);
    //glyphperUnicode[glyph->unicode()] = glyph;
  }

  QFileInfo fileInfo(fileName);

  m_path = fileInfo.absoluteFilePath();

  file.close();

  return true;
}
double Font::lineHeight() {

  double lineheight = getNumericVariable("lineheight");

  return lineheight;
}
double Font::getNumericVariable(QString name) {

  double x = 0;
  auto ba = name.toStdString();
  auto ret = getMPNumVariable(mp, (char*)ba.c_str(), &x);


  return x;
}
QString Font::familyName() {

  char* name = nullptr;
  QString ret;

  auto found = getMPStringVariable(mp, "nametable familyName", &name);

  if (found) {
    ret = QString::fromUtf8(name);
  }

  return ret;

}
bool Font::getPairVariable(QString name, QPointF& point) {
  QPointF value;
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
    point = QPointF(x, y);
    return true;

  }
  else {
    QByteArray ba = name.toLocal8Bit();
    char* c_str2 = ba.data();
    double x, y;
    if (!getMPPairVariable(mp, c_str2, &x, &y)) {
      return false;
    }
    point = QPointF(x, y);
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
QString Font::fontName()
{
  return m_fontName;
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

  mp->history = mp_spotless;
  QByteArray commandBytes = command.toLatin1();
  int status = mp_execute(mp, (char*)commandBytes.constData(), commandBytes.size());
  mp_run_data* results = mp_rundata(mp);
  QString ret(results->term_out.data);
  ret.trimmed();
  if (status == mp_error_message_issued || status == mp_fatal_error_stop) {
    std::cout << ret.toStdString() << std::endl;
    throw ret;
  }

  return ret;

}
mp_edge_object* Font::getEdges() {
  mp_run_data* _mp_results = mp_rundata(mp);
  mp_edge_object* edges = _mp_results->edges;
  return edges;
};

mp_edge_object* Font::getEdge(int charCode) {

  mp_edge_object* edge = nullptr;

  mp_edge_object* p = getEdges();

  while (p) {
    if (p->charcode == charCode) {
      edge = p;
      break;
    }
    p = p->next;
  }

  return edge;
}

void Font::generateAlternate(QString macroname, GlyphParameters params) {

  QString metapostString = QString("save params;params0:=%1;params1:=%2;params3:=%3;params4:=%4;params5:=%5;")
    .arg(params.lefttatweel)
    .arg(params.righttatweel)
    .arg(params.third)
    .arg(params.fourth)
    .arg(params.fifth);


  metapostString = metapostString + QString("generateAlternate(%1$,params);").arg(macroname);

  executeMetaPost(metapostString);

}
mp_graphic_object* Font::copyEdgeBody(mp_graphic_object* body) {
  mp_graphic_object* result = nullptr;

  auto copypath = [this](mp_gr_knot knot)
  {
    mp_gr_knot p, current, ret;

    ret = nullptr;

    if (knot == nullptr) return ret;

    ret = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data)); //new mp_gr_knot_data();

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


      mp_gr_knot tmp = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data)); // new mp_gr_knot_data();

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






  mp_graphic_object* currObject = nullptr;

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {

        mp_fill_object* fillobject = (mp_fill_object*)body;
        mp_gr_knot newpath = copypath(fillobject->path_p);

        mp_fill_object* nextObject = (mp_fill_object*)mp_new_graphic_object(mp, mp_fill_code); // new mp_fill_object;
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
          result = currObject;
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


  return result;

}

QString Font::getLog()
{
  mp_run_data* results = mp_rundata(mp);
  QString ret(results->term_out.data);
  return ret.trimmed();
}

