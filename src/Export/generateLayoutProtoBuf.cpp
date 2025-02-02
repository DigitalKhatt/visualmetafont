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
#include "protobuf/quran.pb.h"
#include <fstream>

#include "qfileinfo.h"

extern "C"
{
#include "mplibps.h"
}

using namespace protobuf;

static protobuf::PathElem* filltoHTML5Path(mp_gr_knot h, protobuf::Path& pathStream)
{

  if (!h) return nullptr;
  mp_gr_knot p, q;



  QJsonArray start;

  auto elem = pathStream.add_elems();

  elem->add_points(h->x_coord);
  elem->add_points(h->y_coord);

  p = h;
  do {
    q = p->next;


    elem = pathStream.add_elems();

    elem->add_points(p->right_x);
    elem->add_points(p->right_y);
    elem->add_points(q->left_x);
    elem->add_points(q->left_y);
    elem->add_points(q->x_coord);
    elem->add_points(q->y_coord);

    p = q;
  } while (p != h);

  return elem;

}

static void edgetoHTML5Path(mp_graphic_object* body, protobuf::Glyph& glyph, decltype  (&protobuf::Glyph::add_default_) func, bool isColored) {


  protobuf::Path* pathStream = nullptr;  

  if (!isColored) {
    pathStream = (glyph.*func)();
  }

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {

        QJsonObject pathObject;

        auto fillobject = (mp_fill_object*)body;

        if (isColored) {
          pathStream = (glyph.*func)();
        }

        auto pathElem = filltoHTML5Path(fillobject->path_p, *pathStream);

        if (isColored) {
          pathStream->add_color(fillobject->color.a_val * 255);
          pathStream->add_color(fillobject->color.b_val * 255);
          pathStream->add_color(fillobject->color.c_val * 255);
        }

        break;
      }
      default:
        break;
      }

    } while (body = body->next);
  }
}

void GenerateLayout::generateLayoutProtoBuf(int lineWidth, int scale) {

  LayOut layout;
  protobuf::Font font;

  auto& glyphs = *font.mutable_glyphs();

  for (auto& glyph : m_otlayout->glyphs) {
    protobuf::Glyph glyphProto;

    bool isColored = glyph.name.contains("aya");

    if (isColored) {
      //glyphProto.set_name(glyph.name.toStdString());
    }
    

    glyphProto.add_bbox(glyph.bbox.llx);
    glyphProto.add_bbox(glyph.bbox.lly);
    glyphProto.add_bbox(glyph.bbox.urx);
    glyphProto.add_bbox(glyph.bbox.ury);

    ::edgetoHTML5Path(glyph.copiedPath, glyphProto, &protobuf::Glyph::add_default_, isColored);

    const auto& ff = m_otlayout->expandableGlyphs.find(glyph.name);

    if (ff != m_otlayout->expandableGlyphs.end()) {


      auto& jj = ff->second;

      glyphProto.add_limits(jj.minLeft);
      glyphProto.add_limits(jj.maxLeft);
      glyphProto.add_limits(jj.minRight);
      glyphProto.add_limits(jj.maxRight);

      if (jj.minLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.minLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        ::edgetoHTML5Path(alternate->copiedPath, glyphProto, &protobuf::Glyph::add_minleft, isColored);
      }

      if (jj.maxLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.maxLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        ::edgetoHTML5Path(alternate->copiedPath, glyphProto, &protobuf::Glyph::add_maxleft, isColored);
      }

      if (jj.minRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.minRight;

        auto alternate = glyph.getAlternate(parameters);

        ::edgetoHTML5Path(alternate->copiedPath, glyphProto, &protobuf::Glyph::add_minright, isColored);
      }

      if (jj.maxRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.maxRight;

        auto alternate = glyph.getAlternate(parameters);        

        ::edgetoHTML5Path(alternate->copiedPath, glyphProto, &protobuf::Glyph::add_maxright, isColored);
      }


    }

    glyphs[glyph.charcode] = glyphProto;
  }

  for (int pageIndex = 0; pageIndex < layoutPages.pages.size(); pageIndex++) {

    auto& page = layoutPages.pages[pageIndex];
    auto& pagetext = layoutPages.originalPages[pageIndex];

    auto pageStream = layout.add_pages();


    for (int lineIndex = 0; lineIndex < page.size(); lineIndex++) {
      auto& line = page[lineIndex];
      auto lineStream = pageStream->add_lines();
      lineStream->set_type((int)line.type);
      lineStream->set_x(line.xstartposition);
      lineStream->set_y(line.ystartposition);
      lineStream->set_text(pagetext[lineIndex].toStdString());
      for (auto& glyph : line.glyphs) {
        auto glphStream = lineStream->add_glyphs();


        glphStream->set_codepoint(glyph.codepoint);
        glphStream->set_cluster(glyph.cluster);

        if (glyph.x_advance != 0) {
          glphStream->set_x_advance(glyph.x_advance);
        }
        if (glyph.x_offset != 0) {
          glphStream->set_x_offset(glyph.x_offset);
        }

        if (glyph.y_offset != 0) {
          glphStream->set_y_offset(glyph.y_offset);
        }

        if (glyph.color != 0) {
          glphStream->set_color(glyph.color);
        }

        if (glyph.lefttatweel != 0) {
          glphStream->set_lefttatweel(glyph.lefttatweel);
        }

        if (glyph.righttatweel != 0) {
          glphStream->set_righttatweel(glyph.righttatweel);
        }


        if (glyph.beginsajda) {
          glphStream->set_beginsajda(glyph.beginsajda);
        }
        if (glyph.endsajda) {
          glphStream->set_endsajda(glyph.endsajda);
        }

      }
    }

  }

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

        auto sura = layout.add_suras();

        sura->set_name(QString("%1 ( %2 )").arg(layoutPages.originalPages.at(pageIndex).at(lineIndex)).arg(suraNumber++).toStdString());
        sura->set_page_number(pageIndex);
        sura->set_x(0);
        sura->set_y(y);

      }
    }
  }

  
  auto path = this->m_otlayout->font->filePath();
  QFileInfo fileInfo = QFileInfo(path);



  std::fstream layoutOutput(fileInfo.path().toStdString() + "/output/layout.protobuf", std::ios::out | std::ios::trunc | std::ios::binary);
  if (!layout.SerializeToOstream(&layoutOutput)) {
    throw std::runtime_error("ERROR");
  }

  std::fstream fontOutput(fileInfo.path().toStdString() + "/output/font.protobuf", std::ios::out | std::ios::trunc | std::ios::binary);
  if (!font.SerializeToOstream(&fontOutput)) {
    throw std::runtime_error("ERROR");
  }

}
