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

#include "ExportToHTML.h"
#include <qhash.h>
#include "Layout/GlyphVis.h"
#include "qfile.h"
#include "qtextstream.h"
#include "automedina/automedina.h"

#include "qdebug.h"
#include "LayoutWindow.h"
#include "hb.hh"
#include "qregularexpression.h"

extern "C"
{
#include "mplibps.h"
}


ExportToHTML::ExportToHTML(OtLayout* otlayout) :m_otlayout(otlayout)
{
}

ExportToHTML::~ExportToHTML()
{
}

void ExportToHTML::generateQuranPages(QList<QList<LineLayoutInfo>> pages, int lineWidth, QList<QStringList> originalText, int scale) {
  QFile file("pages.component.ts.html");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");

  for (int p = 0; p < originalText.length(); p++) {
    auto& pageText = originalText[p];
    auto& page = pages[p];
    out << "<div #page class='page";    
    if (p == 0 || p == 1) {
      out << " center";
    }
    else {
      out << " justify";
    }
    out << "' data-page-number='" << p + 1 << "'>" << '\n';
    out << "<div class='innerpage'>" << '\n';
    for (int l = 0; l < pageText.length(); l++) {
      auto& lineText = pageText[l];
      auto& line = page[l];
      out << "<div data-line-number='" << l + 1 << "'";      
      if (line.type == LineType::Bism) {
        out << " class = 'linebism";
      }
      else if (line.type == LineType::Sura) {
        out << " class = 'linesuran";
      }
      else {
        out << " class = 'line";
      }
      out << "'>" << '\n';
      QRegularExpression re("(\\d+)");
      re.setPatternOptions(QRegularExpression::UseUnicodePropertiesOption);

      out << lineText.replace(re, QString("۝") + "\\1") << '\n';
      out << "</div>" << '\n';
    }
    out << "</div>" << '\n';
    out << "</div>" << '\n';
  }
}

void ExportToHTML::generateQuranPagesOld(QList<QList<LineLayoutInfo>> pages, int lineWidth, QList<QStringList> originalText, int scale)
{

  QFile file("web/glyphs.js");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);   // we will serialize the data into the file
  out.setCodec("UTF-8");
  //out.setEncoding(QStringConverter::Utf8); 

  const int margin = 400 << OtLayout::SCALEBY;

  //qreal pixelSize = 1000 * scale;

  usedGlyphs.insert(m_otlayout->glyphs["endofaya"].charcode, usedGlyphs.size());

  out << "pages = [];\n";

  //for (int p = 0; p < pages.size(); p++) {
  for (int p = 0; p < 10; p++) {
    auto page = pages.at(p);
    auto originalPage = originalText.at(p);
    out << "pages[" << p << "] = function(ctx) {\n";

    QPoint beginsajda;
    QPoint endsajda;

    for (int l = 0; l < page.size(); l++) {
      auto line = page.at(l);
      auto originalLine = originalPage.at(l);

      hb_position_t currentxPos = (lineWidth)+margin - line.xstartposition;
      hb_position_t currentyPos = line.ystartposition;

      int end = -1;

      QPoint lastPos{ currentxPos ,currentyPos };
      //int MCID = 0;

      out << "\tctx.save();\n";

      if (line.type == LineType::Sura) {

        //int pageId = d_ep->pages.constLast();
        int height = (OtLayout::TopSpace << OtLayout::SCALEBY);


        int ayaFrameyPos = currentyPos - 3 * height / 5;



        out << "\tctx.drawImage(suraFrame," << (400 << OtLayout::SCALEBY) << "," << ayaFrameyPos << "," << lineWidth << "," << height << ");\n";

        //auto outlineDest = d_ep->pageMatrix().map(QPoint{ currentxPos , ayaFrameyPos });

        //MyQPdfEnginePrivate::Dest dest{ pageId,outlineDest.x() ,outlineDest.y() ,0 };

        //MyQPdfEnginePrivate::OutlineEntry outline{ originalLine, dest };

        //d_ep->outlines[d_ep->requestObject()] = outline;

        //*d_ep->currentPage << "ET\n";
        //*d_ep->currentPage << "q\n1 0 0 1 " << 0 << ayaFrameyPos << "cm\n";
        //writeRawPDFtoCurrentStream(QString("/Im%1  Do\nQ\n").arg(suraFrameObjectId));
        //addImagetoResources(suraFrameObjectId);
        //*d_ep->currentPage << "BT\n";
        //*d_ep->currentPage << "0.8 0 0 -0.8 " << lastPos.x() - (400 << OtLayout::SCALEBY) << lastPos.y() << "Tm\n";

        out << "\tctx.transform(" << 0.8 << ",0,0," << -0.8 << "," << (lastPos.x() - (400 << OtLayout::SCALEBY)) << "," << lastPos.y() << ");\n";

      }
      else if (line.type == LineType::Bism) {

        out << "\tctx.transform(" << 0.8 << ",0,0," << -0.8 << "," << (lastPos.x() - (600 << OtLayout::SCALEBY)) << "," << (lastPos.y() - (200 << OtLayout::SCALEBY)) << ");\n";

        //*d_ep->currentPage << "0.8 0 0 -0.8 " << lastPos.x() - (600 << OtLayout::SCALEBY) << lastPos.y() - (200 << OtLayout::SCALEBY) << "Tm\n";
      }
      else {
        //*d_ep->currentPage << "1 0 0 -1 " << lastPos.x() << lastPos.y() << "Tm\n";
        //out << "\tctx.translate(" << lastPos.x() << "," << lastPos.y() << ");\n";
        out << "\tctx.transform(" << 1 << ",0,0," << -1 << "," << lastPos.x() << "," << lastPos.y() << ");\n";
      }

      //*d_ep->currentPage << "/P << /MCID " << MCID << ">>\nBDC\n";

      for (int i = 0; i < line.glyphs.size(); i++) {

        if (!usedGlyphs.contains(line.glyphs[i].codepoint)) {
          usedGlyphs.insert(line.glyphs[i].codepoint, usedGlyphs.size());
        }
        if (i > end) {
          end = i;
          int currentcluster = line.glyphs[i].cluster;
          int endcluster = -1;
          do {
            while (end + 1 < line.glyphs.size() && line.glyphs[end + 1].cluster == currentcluster)
              ++end;

            if (end < line.glyphs.size() - 1) {
              endcluster = line.glyphs[end + 1].cluster;
              if (originalLine[endcluster].unicode() == 0x06E5 || originalLine[endcluster].unicode() == 0x06E6) {
                end++;
                currentcluster = endcluster;
                continue;
              }
            }
            else {
              endcluster = originalLine.size();
            }
            break;
          } while (true);

          /*
          *d_ep->currentPage << "/Span << /ActualText <FEFF";
          for (int ii = line.glyphs[i].cluster; ii < endcluster; ++ii) {
            ushort unicode = originalLine[ii].unicode();
            if (unicode == 10)
              unicode = 0x20;
            *d_ep->currentPage << MyQPdf::toHex(unicode, buf);
          }
          *d_ep->currentPage << "> >>\n"
            "BDC\n";*/

        }



        //set color
        if (line.glyphs[i].color) {
          int color = line.glyphs[i].color;
          //*d_ep->currentPage << ((color >> 24) & 0xff) / 255.0 << ((color >> 16) & 0xff) / 255.0 << ((color >> 8) & 0xff) / 255.0 << "scn\n";
          out << "\tctx.fillStyle = 'rgb(" << ((color >> 24) & 0xff) << "," << ((color >> 16) & 0xff) << "," << ((color >> 8) & 0xff) << ")';\n";
        }



        QPoint pos;
        currentxPos -= line.glyphs[i].x_advance;
        pos.setX(currentxPos + (line.glyphs[i].x_offset));
        pos.setY(currentyPos - (line.glyphs[i].y_offset));

        if (line.glyphs[i].beginsajda) {
          beginsajda = { lastPos.x(),currentyPos };
        }
        else if (line.glyphs[i].endsajda) {
          endsajda = { pos.x(),currentyPos };
        }

        QPoint diff = pos - lastPos;
        lastPos = pos;

        out << "\tctx.translate(" << diff.x() << "," << -diff.y() << ");\n";
        //draw glyph
        out << "\tctx.save();\n";
        out << "\tctx.scale(" << scale << "," << scale << ");\n";
        out << "\tglyphs['" << m_otlayout->glyphNamePerCode[line.glyphs[i].codepoint] << "'](ctx);\n";
        out << "\tctx.restore();\n";
        //}


        if (line.glyphs[i].color) {
          out << "\tctx.fillStyle = 'rgb(0,0,0)';\n";
        }

        if (i == end) {
          //*d_ep->currentPage << "EMC\n";
        }



      }
      //*d_ep->currentPage << "/Span << /ActualText <FEFF000A> >>\nBDC\nEMC\n";

      //*d_ep->currentPage << "EMC\n";

      out << "\tctx.restore();\n";


      if (!beginsajda.isNull() && !endsajda.isNull()) {

        if (beginsajda.y() != endsajda.y()) {
          qDebug() << "Sajda Rule not in the same line";
          //throw "Sajda Rule not in the same line";					
          //TODO check this 
          beginsajda.setX((lineWidth)+margin - line.xstartposition);
        }


        out << "\tctx.beginPath()\n;";
        out << "\tctx.moveTo(" << beginsajda.x() << "," << endsajda.y() - (1150 << OtLayout::SCALEBY) << ");\n";
        out << "\tctx.lineTo(" << endsajda.x() << "," << endsajda.y() - (1150 << OtLayout::SCALEBY) << ");\n";
        out << "\tctx.lineWidth = " << (40 << OtLayout::SCALEBY) << ";\n";
        out << "\tctx.stroke();\n";


        beginsajda = QPoint();
        endsajda = QPoint();
      }


    }
    out << "}\n";
  }

  auto glyphs = m_otlayout->glyphs;

  out << "glyphs = {};\n";

  for (auto& glyph : glyphs) {
    //if (usedGlyphs.contains(glyph.charcode)) {
    out << "glyphs['" << glyph.name << "'] = function(ctx) {\n";

    generateGlyph(glyph, out);

    out << "}\n";
    //}
  }

  file.close();
}

void ExportToHTML::edgetoHTML5Path(mp_graphic_object* body, QTextStream& out)
{


  if (body) {

    out << "\tctx.beginPath();\n";
    do {
      switch (body->type)
      {
      case mp_fill_code: {
        filltoHTML5Path(((mp_fill_object*)body)->path_p, out);


        break;
      }
      default:
        break;
      }

    } while (body = body->next);

    out << "\tctx.fill();\n";
  }

}

void ExportToHTML::filltoHTML5Path(mp_gr_knot h, QTextStream& out)
{
  mp_gr_knot p, q;

  out << "\tctx.moveTo(" << h->x_coord << "," << h->y_coord << ");\n";
  p = h;
  do {
    q = p->next;
    out << "\tctx.bezierCurveTo(" << p->right_x << "," << p->right_y << "," << q->left_x << "," << q->left_y << "," << q->x_coord << "," << q->y_coord << ");\n";

    p = q;
  } while (p != h);
  if (h->data.types.left_type != mp_endpoint)
    out << "\tctx.closePath()\n";


}
void ExportToHTML::generateGlyph(GlyphVis& glyph, QTextStream& out) {

  QByteArray steamDataByteArray;


  if (glyph.name == "endofaya") { //||  glyph->name == "rubelhizb" glyph->name == "placeofsajdah" ||
    getImageStream(glyph, out);
  }
  else if (glyph.charcode >= Automedina::AyaNumberCode && glyph.charcode <= Automedina::AyaNumberCode + 286) {
    int ayaNumber = (glyph.charcode - Automedina::AyaNumberCode) + 1;

    int digitheight = 120;

    out << "\tglyphs['endofaya'](ctx);\n";
    out << "\tctx.save();\n";

    if (ayaNumber < 10) {
      auto& onesglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + ayaNumber]];

      auto position = m_otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width) / 2;

      out << "\tctx.translate(" << position << "," << digitheight << ");\n";
      out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";

    }
    else if (ayaNumber < 100) {
      int onesdigit = ayaNumber % 10;
      int tensdigit = ayaNumber / 10;

      auto& onesglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + onesdigit]];
      auto& tensglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + tensdigit]];



      auto position = m_otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + 40) / 2;

      out << "\tctx.translate(" << position << "," << digitheight << ");\n";
      out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";

      out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 << ");\n";
      out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";

    }
    else {
      int onesdigit = ayaNumber % 10;
      int tensdigit = (ayaNumber / 10) % 10;
      int hundredsdigit = ayaNumber / 100;

      auto& onesglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + onesdigit]];
      auto& tensglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + tensdigit]];
      auto& hundredsglyph = m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + hundredsdigit]];

      auto position = m_otlayout->glyphs["endofaya"].width / 2 - (onesglyph.width + tensglyph.width + hundredsglyph.width + 80) / 2;

      out << "\tctx.translate(" << position << "," << digitheight << ");\n";
      out << "\tglyphs['" << hundredsglyph.name << "'](ctx);\n";

      out << "\tctx.translate(" << hundredsglyph.width + 40 << "," << 0 << ");\n";
      out << "\tglyphs['" << tensglyph.name << "'](ctx);\n";

      out << "\tctx.translate(" << tensglyph.width + 40 << "," << 0 << ");\n";
      out << "\tglyphs['" << onesglyph.name << "'](ctx);\n";

    }

    out << "\tctx.restore();\n";
  }
  else {
    edgetoHTML5Path(glyph.copiedPath, out);
  }
}
void ExportToHTML::getImageStream(GlyphVis& glyph, QTextStream& out) {

  if (glyph.m_edge) {
    mp_graphic_object* body = glyph.m_edge->body;
    if (body) {
      do {
        switch (body->type)
        {
        case mp_fill_code: {
          auto fillobject = (mp_fill_object*)body;
          out << "\tctx.beginPath();\n";
          filltoHTML5Path(fillobject->path_p, out);
          if (fillobject->color_model == mp_rgb_model) {
            //painter.setBrush(QColor(fillobject->color.a_val, fillobject->color.b_val, fillobject->color.c_val));
            out << "\tctx.fillStyle = 'rgb(" << fillobject->color.a_val * 255 << "," << fillobject->color.b_val * 255 << "," << fillobject->color.c_val * 255 << ")';\n";

          }
          out << "\tctx.fill();\n";
          out << "\tctx.fillStyle = 'rgb(0,0,0)';\n";

          break;
        }
        default:
          break;
        }

      } while (body = body->next);
    }
  }

}
