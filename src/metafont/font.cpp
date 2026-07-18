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

#include "font.hpp"

#include <filesystem>

#include "glyph.hpp"
#include "hb.hh"
#include "metafont.h"
#include "qapplication.h"
#include "qdebug.h"
#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qregularexpression.h"
#include "qtextstream.h"

namespace fs = std::filesystem;

Font::Font(QObject* parent) : QObject(parent) {
}
bool Font::loadFile(const QString& fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    return false;
  }

  QFile rsmfplain(":/metafont/mfplain.mp");
  QFile rsmpost(":/metafont/mpost.mp");
  QFile rsvmf(":/metafont/vmf.mp");
  QString initMF = "MPGUI:=1;";
  if (!rsmfplain.open(QIODevice::ReadOnly)) {
    qDebug() << "mfplain.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsmfplain.readAll());
  }

  if (!rsmpost.open(QIODevice::ReadOnly)) {
    qDebug() << "mpost.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsmpost.readAll());
  }

  if (!rsvmf.open(QIODevice::ReadOnly)) {
    qDebug() << "vmf.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsvmf.readAll());
  }

  initMF.append(file.readAll());

  fs::path p1 = fileName.toStdString();

  auto parentPath = p1.parent_path();
  m_currentDir = QString::fromStdString(parentPath.string());
  m_mpFont.initialize(initMF.toLocal8Bit().toStdString(), p1);
  m_fontName = QString::fromStdString(m_mpFont.fontName());

  QString glyphsPath = QString::fromStdString(p1.parent_path().append("glyphs.mp").string());

  QFile glyphsFile(glyphsPath);

  if (!glyphsFile.open(QFile::ReadOnly | QFile::Text)) {
    return false;
  }

  QTextStream in(&glyphsFile);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString code = in.readAll();

  QRegularExpression re("((?:beginchar|defchar)(.*?)(?:enddefchar|endchar);)", QRegularExpression::DotMatchesEverythingOption);
  QRegularExpressionMatchIterator i = re.globalMatch(code);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    QString source = match.captured(1);
    Glyph* glyph = new Glyph(source, this);
    glyphs.append(glyph);
    m_mpFont.registerGlyphSource(glyph->name().toStdString(),
                                 glyph->source().toStdString(),
                                 glyph->beginMacroName().toStdString(),
                                 glyph->unicode());
    // glyphperUnicode[glyph->unicode()] = glyph;
  }

  QFileInfo fileInfo(fileName);

  m_path = fileInfo.absoluteFilePath();

  file.close();

  return true;
}
double Font::lineHeight() {
  double lineheight = getInternalNumericVariable("lineheight");

  return lineheight;
}
double Font::getNumericVariable(QString name) {
  return m_mpFont.numericVariable(name.toStdString());
}
bool Font::getBoolVariable(QString name) {
  return m_mpFont.boolVariable(name.toStdString());
}
double Font::getInternalNumericVariable(QString name) {
  return m_mpFont.internalNumericVariable(name.toStdString());
}

QString Font::familyName() {
  return QString::fromStdString(familyNameStd());
}

std::string Font::familyNameStd() {
  return m_mpFont.familyName();
}
QString Font::copyright() {
  return QString::fromStdString(copyrightStd());
}

std::string Font::copyrightStd() {
  return m_mpFont.copyright();
}
bool Font::getPairVariable(QString name, QPointF& point) {
  if (name[0] == 'z' && (name.size() == 1 || name[1].isDigit() || name[1] == '.')) {
    double x, y = 0;
    name[0] = 'x';
    x = m_mpFont.numericVariable(name.toStdString());
    name[0] = 'y';
    y = m_mpFont.numericVariable(name.toStdString());
    point = QPointF(x, y);
    return true;

  } else {
    double x, y;
    if (!m_mpFont.pairVariable(name.toStdString(), x, y)) {
      return false;
    }
    point = QPointF(x, y);
    return true;
  }
  return false;
}
bool Font::saveUnicodes() {
  if (m_path.isEmpty()) return false;

  QFileInfo fileInfo(m_path);

  QString unicodefileName = fileInfo.absolutePath() + "/output/" + fileInfo.baseName() + "_unicodes.lua";

  QFile unicodesfile(unicodefileName);

  if (!unicodesfile.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QTextStream outunicode(&unicodesfile);

  outunicode << fileInfo.baseName() << ".unicodes = {\n";

  for (int i = 0; i < glyphs.length(); i++) {
    outunicode << "  [\"" << glyphs[i]->name() << "\"] = " << glyphs[i]->charcode() << ",\n";
  }

  outunicode << "}";

  unicodesfile.close();

  return true;
}
bool Font::saveFile() {
  if (!m_path.isEmpty()) {
    QFileInfo fileInfo(m_path);
    QFile file(fileInfo.path() + "/glyphs.mp");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return false;

    QTextStream out(&file);

    for (int i = 0; i < glyphs.length(); i++) {
      out << glyphs[i]->source();
    }

    file.close();

    return true;
  }

  return false;
}

Font::~Font() {
}

QString Font::filePath() {
  return m_path;
}
std::string Font::filePathStd() const {
  return m_path.toStdString();
}
QString Font::fontName() {
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

void Font::synchronizeGlyphSources() {
  m_mpFont.clearGlyphSources();
  for (auto* glyph : glyphs) {
    m_mpFont.registerGlyphSource(glyph->name().toStdString(),
                                 glyph->source().toStdString(),
                                 glyph->beginMacroName().toStdString(),
                                 glyph->unicode());
  }
}

MPFont& Font::mpFont() {
  synchronizeGlyphSources();
  return m_mpFont;
}

std::string Font::executeMetaPost(std::string command) {
  std::vector<std::string> names;
  names.reserve(pictureNames.size());
  for (const auto& name : pictureNames) names.push_back(name.toStdString());
  m_mpFont.setControlledPictureNames(std::move(names));
  return m_mpFont.execute(command);
}
std::vector<mp_edge_object*> Font::getEdges() const {
  return m_mpFont.edges();
};

mp_edge_object* Font::getEdge(int charCode) {
  return m_mpFont.edge(charCode);
}

MPGlyphInfo Font::getMPGlyphInfo(int charCode) {
  MPGlyphInfo info;

  auto mpInfo = m_mpFont.glyphInfo(charCode);
  info.currentPicture = mpInfo.currentPicture;
  for (const auto& [name, picture] : mpInfo.controlledPictures)
    info.controlledPictures.insert(QString::fromStdString(name), picture);

  return info;
}

bool Font::hasGlyph(std::string_view glyphName) const {
  return m_mpFont.hasGlyph(glyphName);
}
mp_graphic_object* Font::copyEdgeBody(mp_graphic_object* body) {
  return m_mpFont.copyBody(body);
/* Legacy implementation retained temporarily for reference.
#if 0
  mp_graphic_object* result = nullptr;

  auto copypath = [this](mp_gr_knot knot) {
    mp_gr_knot p, current, ret;

    ret = nullptr;

    if (knot == nullptr) return ret;

    ret = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data));  // new mp_gr_knot_data();

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
      mp_gr_knot tmp = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data));  // new mp_gr_knot_data();

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
      switch (body->type) {
        case mp_fill_code: {
          mp_fill_object* fillobject = (mp_fill_object*)body;
          mp_gr_knot newpath = copypath(fillobject->path_p);

          mp_fill_object* nextObject = (mp_fill_object*)mp_new_graphic_object(mp, mp_fill_code);
          nextObject->type = mp_fill_code;
          nextObject->path_p = newpath;
          nextObject->next = nullptr;
          nextObject->pre_script = fillobject->pre_script != nullptr
                                       ? xstrdup(fillobject->pre_script)
                                       : nullptr;
          nextObject->post_script = fillobject->post_script != nullptr
                                        ? xstrdup(fillobject->post_script)
                                        : nullptr;
          nextObject->pen_p = nullptr;
          nextObject->htap_p = nullptr;

          if (fillobject->color_model == mp_rgb_model) {
            nextObject->color_model = mp_rgb_model;
            nextObject->color = fillobject->color;
          }

          if (currObject == nullptr) {
            currObject = (mp_graphic_object*)nextObject;
            result = currObject;
          } else {
            currObject->next = (mp_graphic_object*)nextObject;
            currObject = currObject->next;
          }

          break;
        }
        case mp_stroked_code: {
          mp_stroked_object* fillobject = (mp_stroked_object*)body;
          mp_gr_knot newpath = copypath(fillobject->path_p);

          mp_stroked_object* nextObject = (mp_stroked_object*)mp_new_graphic_object(mp, mp_stroked_code);  // new mp_fill_object;
          nextObject->type = mp_stroked_code;
          nextObject->path_p = newpath;
          nextObject->next = nullptr;
          nextObject->pre_script = fillobject->pre_script != nullptr
                                       ? xstrdup(fillobject->pre_script)
                                       : nullptr;
          nextObject->post_script = fillobject->post_script != nullptr
                                        ? xstrdup(fillobject->post_script)
                                        : nullptr;
          nextObject->pen_p = nullptr;
          // nextObject->htap_p = nullptr;

          if (fillobject->color_model == mp_rgb_model) {
            nextObject->color_model = mp_rgb_model;
            nextObject->color = fillobject->color;
          }

          if (currObject == nullptr) {
            currObject = (mp_graphic_object*)nextObject;
            result = currObject;
          } else {
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
#endif */
}

QString Font::getLog() {
  return QString::fromStdString(m_mpFont.log()).trimmed();
}

/* Axes are owned and parsed by MPFont.
void Font::readAxes() {
  axes.clear();

  auto numAxes = getInternalNumericVariable("number_of_axes");

  for (int i = 0; i < numAxes; i++) {
    VarAxis axis;
    char* value;
    auto varname = std::format("axes {} name", i);
    auto found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.name = value;
    }
    varname = std::format("axes {} tag", i);
    found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.axisTag = hb_tag_from_string(value, 4);
    }

    varname = std::format("axes {} equivExpr", i);
    found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.equivExpr = value;
    }
    double dbValue = 0.0;

    varname = std::format("axes {} minValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.minValue = dbValue;
    }

    varname = std::format("axes {} defaultValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.defaultValue = dbValue;
    }

    varname = std::format("axes {} maxValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.maxValue = dbValue;
    }

    axes.append(axis);
  }
} */
