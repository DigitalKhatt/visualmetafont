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

#include "GenerateLayout.h"
#include <qhash.h>
#include "Layout/GlyphVis.h"
#include "qfile.h"
#include "qtextstream.h"
#include "automedina/automedina.h"

#include "qdebug.h"
#include "LayoutWindow.h"
#include "hb.hh"
#include "qregularexpression.h"
#include "qjsonobject.h"
#include "qjsondocument.h"
#include "qjsonarray.h"

extern "C"
{
#include "mplibps.h"
}

static double round_up(double value, int decimal_places = -1) {
  if (decimal_places < 0) {
    return value;
  }
  const double multiplier = std::pow(10.0, decimal_places);
  return std::ceil(value * multiplier) / multiplier;
}


GenerateLayout::GenerateLayout(OtLayout* otlayout, LayoutPages& layoutPages) :m_otlayout(otlayout), layoutPages{ layoutPages }
{
}

GenerateLayout::~GenerateLayout()
{
}
void GenerateLayout::generateGlyphs(QJsonObject& glyphsObject) {
  for (auto& glyph : m_otlayout->glyphs) {
    QJsonObject glyphObject;

    glyphObject["name"] = glyph.name;

    QJsonArray bbox;

    bbox.append(glyph.bbox.llx);
    bbox.append(glyph.bbox.lly);
    bbox.append(glyph.bbox.urx);
    bbox.append(glyph.bbox.ury);

    glyphObject["bbox"] = bbox;

    QJsonArray pathArray;

    edgetoHTML5Path(glyph.copiedPath, pathArray);

    glyphObject["default"] = pathArray;

    const auto& ff = m_otlayout->expandableGlyphs.find(glyph.name);

    if (ff != m_otlayout->expandableGlyphs.end()) {

      QJsonArray limitsArray;

      auto& jj = ff->second;

      limitsArray.append(jj.minLeft);
      limitsArray.append(jj.maxLeft);
      limitsArray.append(jj.minRight);
      limitsArray.append(jj.maxRight);

      glyphObject["limits"] = limitsArray;

      if (jj.minLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.minLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        QJsonArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphObject["minLeft"] = pathArray;
      }

      if (jj.maxLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.maxLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        QJsonArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphObject["maxLeft"] = pathArray;
      }

      if (jj.minRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.minRight;

        auto alternate = glyph.getAlternate(parameters);

        QJsonArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphObject["minRight"] = pathArray;
      }

      if (jj.maxRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.maxRight;

        auto alternate = glyph.getAlternate(parameters);

        QJsonArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphObject["maxRight"] = pathArray;
      }


    }

    glyphsObject[QString("%1").arg(glyph.charcode)] = glyphObject;
  }
}
void GenerateLayout::generatePages(QJsonArray& pagesArray, int lineWidth, int scale) {
  for (auto& page : layoutPages.pages) {

    QJsonObject pageObject;

    QJsonArray linesArray;

    for (auto& line : page) {
      QJsonObject lineObject;
      QJsonArray glyphsArray;
      for (auto& glyph : line.glyphs) {
        QJsonObject glyphObject;


        /*
         QJsonArray measures;

        measures.append(round_up(glyph.codepoint));
        measures.append(round_up(glyph.x_advance));
        measures.append(round_up(glyph.x_offset));
        measures.append(round_up(glyph.y_offset));
        measures.append(round_up((int)glyph.color));
        measures.append(round_up(glyph.lefttatweel));
        measures.append(round_up(glyph.righttatweel));

        glyphObject["m"] = measures;*/
        


        if (glyph.x_advance != 0) {
          glyphObject["x_advance"] = round_up(glyph.x_advance);
        }
        if (glyph.x_offset != 0) {
          glyphObject["x_offset"] = round_up(glyph.x_offset);
        }

        if (glyph.y_offset != 0) {
          glyphObject["y_offset"] = round_up(glyph.y_offset);
        }

        if (glyph.color != 0) {
          glyphObject["color"] = (int)glyph.color;
        }

        if (glyph.lefttatweel != 0) {
          glyphObject["lefttatweel"] = round_up(glyph.lefttatweel);
        }

        if (glyph.righttatweel != 0) {
          glyphObject["righttatweel"] = round_up(glyph.righttatweel);
        }

        glyphObject["codepoint"] = glyph.codepoint;


        if (glyph.beginsajda) {
          glyphObject["beginsajda"] = true;
        }
        if (glyph.endsajda) {
          glyphObject["endsajda"] = true;
        }

        glyphsArray.append(glyphObject);

      }

      lineObject["glyphs"] = glyphsArray;
      lineObject["type"] = (int)line.type;
      lineObject["x"] = round_up(line.xstartposition);
      lineObject["y"] = round_up(line.ystartposition);

      linesArray.append(lineObject);
    }

    pageObject["lines"] = linesArray;
    pagesArray.append(pageObject);

  }

}
void GenerateLayout::generateLayout(int lineWidth, int scale) {
  bool Json = true;

  QFile saveFile(Json
    ? QStringLiteral("quran.json")
    : QStringLiteral("quran.dat"));

  if (!saveFile.open(QIODevice::WriteOnly)) {
    qWarning("Couldn't open save file.");
    return;
  }

  QJsonObject quranObject;
  QJsonArray pagesObject;
  QJsonObject glyphsObject;
  QJsonArray suraLocationsArray;

  generateGlyphs(glyphsObject);
  generatePages(pagesObject, lineWidth, scale);
  generateSuraLocations(suraLocationsArray);

  quranObject["glyphs"] = glyphsObject;
  quranObject["pages"] = pagesObject;
  quranObject["suras"] = suraLocationsArray;


  saveFile.write(Json
    ? QJsonDocument(quranObject).toJson(QJsonDocument::JsonFormat::Compact)
    : QCborValue::fromJsonValue(quranObject).toCbor());
}
void GenerateLayout::generateSuraLocations(QJsonArray& surasArray) {


  int height = OtLayout::TopSpace << OtLayout::SCALEBY;

  int suraNumber = 1;

  for (int pageIndex = 0; pageIndex < layoutPages.pages.size(); pageIndex++) {
    auto& page = layoutPages.pages.at(pageIndex);
    for (int lineIndex = 0; lineIndex < page.size(); lineIndex++) {
      auto& line = page.at(lineIndex);
      if (line.type == LineType::Sura) {
        int y = (line.ystartposition - 3 * height / 5) * 72. / (4800 << OtLayout::SCALEBY);
        SuraLocation location{ QString("%1 ( %2 )").arg(layoutPages.originalPages.at(pageIndex).at(lineIndex)).arg(suraNumber++)
          ,pageIndex,0, y };
        QJsonObject sura;

        sura["name"] = QString("%1 ( %2 )").arg(layoutPages.originalPages.at(pageIndex).at(lineIndex)).arg(suraNumber++);
        sura["pageNumber"] = pageIndex;
        sura["x"] = 0;
        sura["y"] = y;

        surasArray.append(sura);
      }
    }
  }
}

void GenerateLayout::filltoHTML5Path(mp_gr_knot h, QJsonArray& pathArray)
{

  if (!h) return;
  mp_gr_knot p, q;

  QJsonArray start;

  start.append(h->x_coord);
  start.append(h->y_coord);

  pathArray.append(start);

  p = h;
  do {
    q = p->next;
    QJsonArray cubic;

    cubic.append(p->right_x);
    cubic.append(p->right_y);
    cubic.append(q->left_x);
    cubic.append(q->left_y);
    cubic.append(q->x_coord);
    cubic.append(q->y_coord);

    pathArray.append(cubic);

    p = q;
  } while (p != h);

}
void GenerateLayout::edgetoHTML5Path(mp_graphic_object* body, QJsonArray& pathsArray) {


  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {

        QJsonObject pathObject;

        auto fillobject = (mp_fill_object*)body;

        QJsonArray pathArray;

        filltoHTML5Path(fillobject->path_p, pathArray);

        pathObject["path"] = pathArray;



        if (fillobject->color_model == mp_rgb_model) {

          QJsonArray rgbArray;

          rgbArray.append(fillobject->color.a_val * 255);
          rgbArray.append(fillobject->color.b_val * 255);
          rgbArray.append(fillobject->color.c_val * 255);

          pathObject["color"] = rgbArray;

        }

        pathsArray.append(pathObject);


        break;
      }
      default:
        break;
      }

    } while (body = body->next);
  }
}
