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



#include "hb-ot-layout-gsub-table.hh"
#include "hb-ot-hmtx-table.hh"
#undef max
#include "OtLayout.h"
#include "Lookup.h"
#include "Subtable.h"
#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonarray.h"
#include "hb-ot.h"
#include "hb-ot-cmap-table.hh"
#include "hb-ot-post-table.hh"
#include <qmap.h>
#include <qset.h>
//#include <QFile>
//#include <QTextStream>
//#include "QJSValueIterator"
#include <iostream>
#ifndef DIGITALKHATT_WEBLIB
#include <QDebug>
#endif
#include "automedina/automedina.h"
#include "QByteArrayOperator.h"
#include "GlyphVis.h"
#include "FeaParser/driver.h"
#include "FeaParser/feaast.h"
#include "qiodevice.h"
//#include "hb-ot-layout-gsubgpos.hh"

#include "qregularexpression.h"

#include "qurantext/quran.h"
#include <limits>

#include <QtCore/qmath.h>
#include <fstream>
#include "metafont.h"
#include "automedina/digitalkhatt.h"
#include "automedina/oldmadina.h"
#include "automedina/indopak.h"

#include <cfenv>


int OtLayout::SCALEBY = 0;
double OtLayout::EMSCALE = 1;
int OtLayout::MINSPACEWIDTH = 0;
int OtLayout::SPACEWIDTH = 75;
int OtLayout::MAXSPACEWIDTH = 100;

QDataStream& operator<<(QDataStream& s, const QSet<quint16>& v)
{
  for (QSet<quint16>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

QDataStream& operator<<(QDataStream& s, const QSet<quint32>& v)
{
  for (QSet<quint32>::const_iterator it = v.begin(); it != v.end(); ++it)
    s << *it;
  return s;
}

QDataStream& operator<<(QDataStream& stream, const SuraLocation& location) {
  stream << location.name << location.pageNumber << location.x << location.y;
  return stream;
}
QDataStream& operator>>(QDataStream& stream, SuraLocation& location) {
  return stream >> location.name >> location.pageNumber >> location.x >> location.y;
}

#include "hb-ot-name-table.hh"

static hb_blob_t* harfbuzzGetTables(hb_face_t* face, hb_tag_t tag, void* userData)
{
  OtLayout* layout = reinterpret_cast<OtLayout*>(userData);

  QByteArray data;
  hb_memory_mode_t mode = HB_MEMORY_MODE_READONLY;

  switch (tag) {
  case HB_OT_TAG_GSUB:
    data = layout->getGSUB();
    break;
  case HB_OT_TAG_GPOS:
    data = layout->getGPOS();
    break;
  case HB_OT_TAG_GDEF:
    data = layout->getGDEF();
    break;
  case HB_TAG('c', 'm', 'a', 'p'):
    data = layout->getCmap();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case HB_TAG('n', 'a', 'm', 'e'):
    data = layout->toOpenType->name();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case HB_TAG('f', 'v', 'a', 'r'):
    data = layout->toOpenType->fvar();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case HB_TAG('H', 'V', 'A', 'R'):
    data = layout->toOpenType->HVAR();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case HB_TAG('J', 'T', 'S', 'T'):
    data = layout->JTST();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case  HB_TAG('h', 'm', 't', 'x'):
    data = layout->toOpenType->hmtx();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case  HB_TAG('h', 'h', 'e', 'a'):
    data = layout->toOpenType->hhea();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  case  HB_TAG('p', 'o', 's', 't'):
    data = layout->toOpenType->post();
    mode = HB_MEMORY_MODE_DUPLICATE;
    break;
  }

  return hb_blob_create(data.constData(), data.size(), mode, NULL, NULL);

}

static unsigned int
getNominalGlyphs(hb_font_t* font HB_UNUSED,
  void* font_data,
  unsigned int count,
  const hb_codepoint_t* first_unicode,
  unsigned int unicode_stride,
  hb_codepoint_t* first_glyph,
  unsigned int glyph_stride,
  void* user_data HB_UNUSED)
{
  OtLayout* layoutgg = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layoutgg->face;

  return ot_face->table.cmap->get_nominal_glyphs(count,
    first_unicode, unicode_stride,
    first_glyph, glyph_stride);
}
static hb_bool_t
getNominalGlyph(hb_font_t* font,
  void* font_data,
  hb_codepoint_t unicode,
  hb_codepoint_t* glyph,
  void* user_data)
{
  /*
      OtLayout* layoutg = reinterpret_cast<OtLayout*>(font_data);

      auto& table = *layoutg->face->table.name;

      auto test = table.get_name(0);

      uint text_size = 1000;
      char text[1000];

      hb_ot_name_get_utf8(layoutg->face,9, 0, &text_size, text);*/


      //auto tt = QString(text);
  OtLayout* layoutg = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layoutg->face;

  return ot_face->table.cmap->get_nominal_glyph(unicode, glyph);

  *glyph = unicode;

  return true;




}

static hb_position_t floatToHarfBuzzPosition(double value)
{
  return static_cast<hb_position_t>(value * (1 << OtLayout::SCALEBY));
}

static hb_position_t getGlyphHorizontalAdvance(hb_font_t* hbFont, void* fontData, hb_codepoint_t glyph, double lefttatweel, double righttatweel, void* userData)
{
  OtLayout* layout = reinterpret_cast<OtLayout*>(fontData);

  if (!layout->glyphNamePerCode.contains(glyph)) {
    //std::cout << "Glyph " << glyph << " not found" << std::endl;
    return 0;
  }

  if (layout->glyphGlobalClasses[glyph] == OtLayout::MarkGlyph) {
    return 0;
  }
  else {
    QString name = layout->glyphNamePerCode[glyph];

    GlyphVis* pglyph = &layout->glyphs[name];

    if (lefttatweel != 0 || righttatweel != 0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      pglyph = layout->getAlternate(pglyph->charcode, parameters);
      
    }

    auto xadvance = hbFont->em_scale_x(pglyph->width);

    double advance = pglyph->width;
    //return advance; // floatToHarfBuzzPosition(advance);
    int upem = 1000;
    int xscale, yscale;
    hb_font_get_scale(hbFont, &xscale, &yscale);
    int64_t scaled = advance * xscale;
    scaled += scaled >= 0 ? upem / 2 : -upem / 2; /* Round. */
    auto gg = (hb_position_t)(scaled / upem);

    return xadvance;

    return gg;
  }

}

hb_position_t OtLayout::gethHorizontalAdvance(hb_font_t* hbFont, hb_codepoint_t glyph, double lefttatweel, double righttatweel, void* userData) {
  return getGlyphHorizontalAdvance(hbFont, this, glyph, lefttatweel, righttatweel, userData);
}

static void
hb_ot_get_glyph_h_advances(hb_font_t* font, void* font_data,
  unsigned count,
  const hb_codepoint_t* first_glyph,
  unsigned glyph_stride,
  hb_position_t* first_advance,
  unsigned advance_stride,
  void* user_data HB_UNUSED)
{

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layout->face;

  //const hb_ot_face_t* ot_face = (const hb_ot_face_t*)font_data;
  //const OT::hmtx_accelerator_t& hmtx = *ot_face->hmtx;

  //const OT::hmtx_accelerator_t& hmtx = ot_face->table.hmtx;

  //return ot_face->table.cmap->

  int coords[2];

  auto glyphs = (hb_glyph_info_t*)(first_glyph);
  auto positions = (hb_glyph_position_t*)(first_advance);
  for (unsigned int i = 0; i < count; i++)
  {



    //double leftTatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].lefttatweel, true);
    //double righttatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].righttatweel, false);

    /*
    *first_advance = font->em_scale_x(ot_face->table.hmtx->get_advance(*first_glyph, font));
    first_glyph = &StructAtOffsetUnaligned<hb_codepoint_t>(first_glyph, glyph_stride);
    first_advance = &StructAtOffsetUnaligned<hb_position_t>(first_advance, advance_stride);*/
    if (glyphs[i].lefttatweel != 0.0 || glyphs[i].righttatweel != 0.0) {
      coords[0] = roundf(glyphs[i].lefttatweel * 16384.f);
      coords[1] = roundf(glyphs[i].righttatweel * 16384.f);
      font->num_coords = 2;
      font->coords = &coords[0];
      positions[i].x_advance = font->em_scale_x(ot_face->table.hmtx->get_advance_with_var_unscaled(glyphs[i].codepoint, font));
      font->num_coords = 0;
      font->coords = nullptr;
    }
    else {
      positions[i].x_advance = font->em_scale_x(ot_face->table.hmtx->get_advance_with_var_unscaled(glyphs[i].codepoint, font));
    }


  }
}

static void get_glyph_h_advances_custom(hb_font_t* font, void* font_data,
  unsigned count,
  const hb_codepoint_t* first_glyph,
  unsigned glyph_stride,
  hb_position_t* first_advance,
  unsigned advance_stride,
  void* user_data)
{
  //TODO:hacking
  auto glyphs = (hb_glyph_info_t*)(first_glyph);
  auto positions = (hb_glyph_position_t*)(first_advance);

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);


  for (unsigned int i = 0; i < count; i++)
  {
    //TEST
    /*
          QString name = layout->glyphNamePerCode[glyphs[i].codepoint];
          if (name == "behshape.init") {
              glyphs[i].lefttatweel = 12;
          }else if (name == "lam.medi") {
              glyphs[i].lefttatweel = 6;
              glyphs[i].righttatweel = 6;
          }*/

          //positions[i].x_advance = getGlyphHorizontalAdvance(font, font_data, glyphs[i].codepoint, glyphs[i].lefttatweel, glyphs[i].righttatweel, user_data);

    double leftTatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].lefttatweel, true);
    double righttatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].righttatweel, false);

    positions[i].x_advance = getGlyphHorizontalAdvance(font, font_data, glyphs[i].codepoint, leftTatweel, righttatweel, user_data);


  }

}

static hb_bool_t get_cursive_anchor(hb_font_t* font, void* font_data,
  hb_cursive_anchor_context_t* context,
  hb_position_t* x,
  hb_position_t* y,
  void* user_data) {

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  Lookup* lookupTable = layout->gposlookups.at(context->lookup_index);

  auto subtable = lookupTable->subtables.at(context->subtable_index);

  if (lookupTable->type == Lookup::cursive) {
    CursiveSubtable* subtableTable = static_cast<CursiveSubtable*>(subtable);

    double lefttatweel = layout->normalToParameter(context->glyph_id, context->lefttatweel, true);
    double righttatweel = layout->normalToParameter(context->glyph_id, context->righttatweel, false);

    if (context->type == hb_cursive_anchor_context_t::entry) {
      auto anchor = subtableTable->getEntry(context->glyph_id, lefttatweel, righttatweel);
      if (anchor) {

        *x = anchor->x();
        *y = anchor->y();

        return true;
      }
      else {
        *x = 0;
        *y = 0;
      }
    }
    else if (context->type == hb_cursive_anchor_context_t::exit) {
      auto anchor = subtableTable->getExit(context->glyph_id, lefttatweel, righttatweel);
      if (anchor) {

        *x = anchor->x();
        *y = anchor->y();

        return true;
      }
      else {
        *x = 0;
        *y = 0;
      }
    }

  }
  else if (lookupTable->type == Lookup::mark2base || lookupTable->type == Lookup::mark2mark) {

    MarkBaseSubtable* subtableTable = static_cast<MarkBaseSubtable*>(subtable);

    quint16 classIndex = subtableTable->markCodes[context->glyph_id];

    QString className = subtableTable->classNamebyIndex[classIndex];

    double lefttatweel = layout->normalToParameter(context->base_glyph_id, context->lefttatweel, true);
    double righttatweel = layout->normalToParameter(context->base_glyph_id, context->righttatweel, false);

    if (context->type == hb_cursive_anchor_context_t::base) {

      QString baseGlyphName = layout->glyphNamePerCode[context->base_glyph_id];

      GlyphVis& curr = layout->glyphs[baseGlyphName];

      auto anchor = subtableTable->getBaseAnchor(context->glyph_id, context->base_glyph_id, lefttatweel, righttatweel);
      if (anchor) {

        *x = anchor->x();
        *y = anchor->y();

        return true;
      }
      else {
        *x = 0;
        *y = 0;
      }

    }
    else if (context->type == hb_cursive_anchor_context_t::mark) {
      QString markGlyphName = layout->glyphNamePerCode[context->glyph_id];


      GlyphVis& curr = layout->glyphs[markGlyphName];

      auto anchor = subtableTable->getMarkAnchor(context->glyph_id, context->base_glyph_id, lefttatweel, righttatweel);
      if (anchor) {

        *x = anchor->x();
        *y = anchor->y();

        return true;
      }
      else {
        *x = 0;
        *y = 0;
      }


    }



  }

  return false;

}

static hb_bool_t get_substitution(hb_font_t* font, void* font_data,
  hb_substitution_context_t* context, void* user_data) {

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  Lookup* lookupTable = layout->gsublookups.at(context->lookup_index);

  if (lookupTable->name == "markexpansion.l1") {
    auto buffer = context->buffer;
    unsigned int glyph_count;

    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

    unsigned int prevIndex = context->curr - 1;

    while (prevIndex >= 0 && glyph_info[prevIndex].var1.u16[0] & HB_OT_LAYOUT_GLYPH_PROPS_MARK) prevIndex--;

    if (prevIndex < 0) return false;

    auto& curr_info = glyph_info[context->curr];

    auto& prev_info = glyph_info[prevIndex];

    char			buf[64];

    hb_font_get_glyph_name(font, curr_info.codepoint, buf, sizeof(buf));

    char			prevName[64];
    hb_font_get_glyph_name(font, prev_info.codepoint, prevName, sizeof(prevName));
    if (QString(prevName) == "behshape.medi.expa") {
      curr_info.lefttatweel = (std::min)(prev_info.lefttatweel, 1.5);
    }
    else if (QString(prevName).contains(".expa")) {
      curr_info.lefttatweel = 1.5;

    }
    else {
      //curr_info.lefttatweel = 0.07 + 0.1 * prev_info.lefttatweel;
    }

    auto& curr_glyph = *layout->getGlyph(curr_info.codepoint);



  }
  else if (lookupTable->name.startsWith("expa.")) {

    auto buffer = context->buffer;

    auto& curr_info = buffer->cur();

    auto name = layout->glyphNamePerCode[curr_info.codepoint];

    layout->justificationContext.GlyphsToExtend.push_back(buffer->idx);
    layout->justificationContext.Substitutes.push_back(context->substitute);

    auto subtable = lookupTable->subtables.at(context->subtable_index);

    if (lookupTable->type == Lookup::single) {
      SingleSubtable* subtableTable = static_cast<SingleSubtable*>(subtable);
      if (subtableTable->format == 10) {
        SingleSubtableWithExpansion* tatweelSubtable = static_cast<SingleSubtableWithExpansion*>(subtableTable);
        auto& expa = tatweelSubtable->expansion[curr_info.codepoint];
        layout->justificationContext.Expansions.insert({ buffer->idx, expa });
        layout->justificationContext.totalWeight += expa.weight;
      }
    }




    return false;

  }
  else if (lookupTable->name.contains("test")) {

    auto buffer = context->buffer;

    auto& curr_info = buffer->cur();

    auto name = layout->glyphNamePerCode[curr_info.codepoint];

    //JustificationContext::GlyphsToExtend.append(buffer->idx);

    if (name == "behshape.medi") {
      curr_info.lefttatweel = 3;
      curr_info.righttatweel = 2;
    }

  }
  else {
    auto buffer = context->buffer;

    auto& curr_info = buffer->cur();

    auto name = layout->glyphNamePerCode[curr_info.codepoint];

    //JustificationContext::GlyphsToExtend.append(buffer->idx);
    //JustificationContext::Substitutes.append(context->substitute);

    auto subtable = lookupTable->subtables.at(context->subtable_index);

    if (lookupTable->type == Lookup::single) {
      SingleSubtable* subtableTable = static_cast<SingleSubtable*>(subtable);
      if (subtableTable->format == 10) {
        SingleSubtableWithExpansion* tatweelSubtable = static_cast<SingleSubtableWithExpansion*>(subtableTable);
        auto expa = tatweelSubtable->expansion.value(curr_info.codepoint);
        //layout->justificationContext.Expansions.insert({ buffer->idx, tatweelSubtable->expansion.value(curr_info.codepoint) });
        curr_info.lefttatweel += expa.MaxLeftTatweel;
        curr_info.righttatweel += expa.MaxRightTatweel;
      }
    }
  }

  return true;
}

static hb_bool_t apply_lookup(hb_font_t* font, void* font_data,
  OT::hb_ot_apply_context_t* c, void* user_data) {

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  Lookup* lookupTable = nullptr;

  if (c->table_index == 0) {
    lookupTable = layout->gsublookups.at(c->lookup_index);
  }
  else {
    lookupTable = layout->gposlookups.at(c->lookup_index);
  }

  auto subtable = lookupTable->subtables.at(c->subtable_index);

  if (lookupTable->type == Lookup::fsmgsub || lookupTable->type == Lookup::fsmgpos) {
    FSMSubtable* subtableTable = static_cast<FSMSubtable*>(subtable);
    layout->executeFSM(*subtableTable, c);
  }

  return true;

}
static hb_bool_t
hb_ot_get_glyph_name(hb_font_t* font HB_UNUSED,
  void* font_data,
  hb_codepoint_t glyph,
  char* name, unsigned int size,
  void* user_data HB_UNUSED)
{
  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layout->face;

  if (ot_face->table.post->get_glyph_name(glyph, name, size)) return true;

  return false;
}
static hb_font_funcs_t* getFontFunctions(hb_font_t* font, bool otVar)
{
  static hb_font_funcs_t* harfbuzzCoreTextFontFuncs = 0;

  //auto& ffunctions = hb_font_get_font_funcs(*font);

  if (!harfbuzzCoreTextFontFuncs) {
    harfbuzzCoreTextFontFuncs = hb_font_funcs_create();
    hb_font_funcs_set_nominal_glyph_func(harfbuzzCoreTextFontFuncs, getNominalGlyph, NULL, NULL);
    //hb_font_funcs_set_nominal_glyphs_func(harfbuzzCoreTextFontFuncs, getNominalGlyphs, NULL, NULL);
    //hb_font_funcs_set_glyph_h_advance_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalAdvance, 0, 0);
    if (!otVar) {
      hb_font_funcs_set_glyph_h_advances_func(harfbuzzCoreTextFontFuncs, get_glyph_h_advances_custom, 0, 0);
    }
    else {
      hb_font_funcs_set_glyph_h_advances_func(harfbuzzCoreTextFontFuncs, hb_ot_get_glyph_h_advances, nullptr, nullptr);

    }

    hb_font_funcs_set_cursive_anchor_func(harfbuzzCoreTextFontFuncs, get_cursive_anchor, 0, 0);
    hb_font_funcs_set_substitution_func(harfbuzzCoreTextFontFuncs, get_substitution, 0, 0);
    hb_font_funcs_set_apply_lookup_func(harfbuzzCoreTextFontFuncs, apply_lookup, 0, 0);

    //hb_font_funcs_set_glyph_h_origin_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalOrigin, 0, 0);
    //hb_font_funcs_set_glyph_extents_func(harfbuzzCoreTextFontFuncs, getGlyphExtents, 0, 0);

    //hb_font_funcs_set_glyph_name_func(harfbuzzCoreTextFontFuncs, hb_ot_get_glyph_name, nullptr, nullptr);

    hb_font_funcs_make_immutable(harfbuzzCoreTextFontFuncs);

  }
  return harfbuzzCoreTextFontFuncs;
}

QPoint AnchorCalc::getAdjustment(Automedina& y, MarkBaseSubtable& subtable, GlyphVis* curr, QString className, QPoint adjust, double lefttatweel, double righttatweel, GlyphVis** poriginalglyph) {

  GlyphVis* originalglyph = curr;

  QPoint adjustoriginal;

  if (curr->expanded) {
    if (curr->name != "alternatechar" && (!curr->originalglyph.isEmpty() && (curr->charlt != 0 || curr->charrt != 0))) {
      adjustoriginal = subtable.classes[className].baseparameters[curr->originalglyph];
    }

    originalglyph = &y.glyphs[curr->originalglyph];
    if (curr->leftAnchor) {
      double xshift = curr->matrix.xpart - originalglyph->matrix.xpart;
      double yshift = curr->matrix.ypart - originalglyph->matrix.ypart;

      adjustoriginal += QPoint(xshift, yshift);
    }
    else if (curr->rightAnchor && !(curr->originalglyph.contains("fina") && curr->originalglyph.contains("expa"))) {

    }
    else {
      originalglyph = curr;
    }
  }

  *poriginalglyph = originalglyph;

  return adjustoriginal;

}

GlyphVis* OtLayout::getGlyph(QString name, double lefttatweel, double righttatweel) {
  GlyphVis* pglyph = &this->glyphs[name];

  if (lefttatweel != 0 || righttatweel != 0) {
    GlyphParameters parameters{};

    parameters.lefttatweel = lefttatweel;
    parameters.righttatweel = righttatweel;

    pglyph = getAlternate(pglyph->charcode, parameters);
  }

  return pglyph;
}

GlyphVis* OtLayout::getGlyph(int code, double lefttatweel, double righttatweel) {

  if (glyphNamePerCode.contains(code)) {
    if (lefttatweel != 0 || righttatweel != 0) {
      GlyphParameters parameters{};

      parameters.lefttatweel = lefttatweel;
      parameters.righttatweel = righttatweel;

      return getAlternate(code, parameters);
    }
    else {
      return getGlyph(glyphNamePerCode[code], lefttatweel, righttatweel);
    }
  }

  return nullptr;
}

GlyphVis* OtLayout::getGlyph(int code) {

  GlyphVis* curr = nullptr;

  if (glyphNamePerCode.contains(code)) {
    QString baseGlyphName = glyphNamePerCode[code];

    curr = &glyphs[baseGlyphName];
  }

  return curr;
}

QByteArray OtLayout::getGDEF() {
  if (!gdef_array.isEmpty() && !dirty) {
    return gdef_array;
  }

  gdef_array.clear();

  quint16 markGlyphSetsDefOffset = 0;
  quint16 glyphClassDefOffset = 18;

  quint16 glyphCount = glyphGlobalClasses.size();
  quint16 markGlyphSetCount = markGlyphSets.size();

  //if (markGlyphSetCount > 0) {
  markGlyphSetsDefOffset = glyphClassDefOffset + 2 + 2 + glyphCount * 6;
  //}

  uint32_t itemVarStoreOffset = 0;

  QByteArray MarkGlyphSetsTable;
  QByteArray coverageTables;

  MarkGlyphSetsTable << (quint16)1 << markGlyphSetCount;

  quint32 CoverageOffsets = 2 + 2 + 4 * markGlyphSetCount;

  for (auto MarkGlyphSet : markGlyphSets) {
    MarkGlyphSetsTable << CoverageOffsets;
    std::sort(MarkGlyphSet.begin(), MarkGlyphSet.end());
    quint16 MarkGlyphSetCount = MarkGlyphSet.size();
    coverageTables << (quint16)1 << MarkGlyphSetCount << MarkGlyphSet;
    CoverageOffsets += 2 + 2 + 2 * MarkGlyphSetCount;
  }

  MarkGlyphSetsTable.append(coverageTables);

  QByteArray itemVariationStore = toOpenType->getGDEFItemVariationStore();
  if (itemVariationStore.size() != 0) {
    itemVarStoreOffset = markGlyphSetsDefOffset + MarkGlyphSetsTable.size();
  }

  gdef_array << (quint16)1 << (quint16)3 << glyphClassDefOffset << (quint16)0 << (quint16)0 << (quint16)0 << markGlyphSetsDefOffset << itemVarStoreOffset;

  gdef_array << (quint16)2;

  gdef_array << glyphCount;

  for (auto i = glyphGlobalClasses.constBegin(); i != glyphGlobalClasses.constEnd(); ++i) {
    quint16 code = i.key();
    quint16 classValue = i.value();
    gdef_array << code;
    gdef_array << code;
    gdef_array << classValue;
  }

  gdef_array.append(MarkGlyphSetsTable);
  gdef_array.append(itemVariationStore);

  return gdef_array;

}
QByteArray OtLayout::getGSUB() {
  if (!gsub_array.isEmpty() && !dirty) {
    return gsub_array;
  }

  gsub_array = getGSUBorGPOS(true, gsublookups, allGsubFeatures, gsublookupsIndexByName);

  return gsub_array;
}
QByteArray OtLayout::getGPOS() {
  if (!gpos_array.isEmpty() && !dirty) {
    return gpos_array;
  }


  gpos_array = getGSUBorGPOS(false, gposlookups, allGposFeatures, gposlookupsIndexByName);

  tajweedcolorindex = gposlookupsIndexByName.value("green", 0xFFFF);

  return gpos_array;
}
QByteArray OtLayout::getFeatureList(QMap<QString, QSet<quint16>> allFeatures) {

  QByteArray featureList_array;
  QByteArray features_array;
  QDataStream featureList_stream(&featureList_array, QIODevice::WriteOnly);
  QDataStream features_stream(&features_array, QIODevice::WriteOnly);

  quint16 featureCount = allFeatures.size();
  quint16 beginoffset = 2 + 6 * featureCount;

  featureList_stream << featureCount;

  QMapIterator<QString, QSet<quint16>> it(allFeatures);
  while (it.hasNext()) {
    it.next();
    featureList_stream.writeRawData(it.key().toLatin1(), 4); // scriptTag
    featureList_stream << beginoffset;

    quint16 lookupIndexCount = it.value().count();

    features_stream << (quint16)0; //featureParams
    features_stream << lookupIndexCount;

    features_stream << it.value();

    /*
          QSetIterator<quint16> i(it.value());
          while (i.hasNext()) {
          features_stream << i.next();
          }*/

    beginoffset += 2 + 2 + 2 * lookupIndexCount;

  }

  featureList_array.append(features_array);

  return featureList_array;
}
QByteArray OtLayout::getScriptList(int featureCount) {
  // script list

  QByteArray scriptList_array;
  QDataStream scriptList_stream(&scriptList_array, QIODevice::WriteOnly);

  //ScriptList table
  scriptList_stream << (quint16)1; // scriptCount
  scriptList_stream.writeRawData("arab", 4); // scriptTag
  scriptList_stream << (quint16)8; // scriptOffset

  // arab script table
  scriptList_stream << (quint16)10; // defaultLangSys
  scriptList_stream << (quint16)1; // langSysCount
  scriptList_stream.writeRawData("ARA ", 4); // langSysTag
  scriptList_stream << (quint16)10; // langSysOffset (2 + 2 + 4 + 2)

  // LangSys table

  scriptList_stream << (quint16)0; // lookupOrder
  scriptList_stream << (quint16)0xFFFF; // lookupOrder
  scriptList_stream << (quint16)featureCount; // featureIndexCount

  for (int i = 0; i < featureCount; i++) {
    scriptList_stream << (quint16)i;
  }

  return scriptList_array;
}

QByteArray OtLayout::getGSUBorGPOS(bool isgsub, QVector<Lookup*>& lookups, QMap<QString, QSet<quint16>>& allFeatures,
  QMap<QString, int>& lookupsIndexByName) {

  allFeatures.clear();
  lookupsIndexByName.clear();
  lookups.clear();

  for (auto lookup : this->lookups) {
    if (!disabledLookups.contains(lookup) && (extended || (lookup->type != Lookup::fsmgsub))) {
      if (isgsub == lookup->isGsubLookup()) {
        quint16 lookupIndex = lookups.size();

        lookupsIndexByName[lookup->name] = lookupIndex;
        lookups.append(lookup);
      }
    }
  }

  for (auto featureName : this->allFeatures.keys()) {
    for (auto lookup : this->allFeatures.value(featureName)) {
      int lookupIndex = lookupsIndexByName.value(lookup->name, -1);
      if (lookupIndex != -1) {
        allFeatures[featureName].insert(lookupIndex);
      }
    }
  }

  QByteArray scriptList = getScriptList(allFeatures.count());
  QByteArray featureList = getFeatureList(allFeatures);


  QByteArray root;

  quint16 scriptListOffset = 10;
  quint16 featureListOffset = scriptListOffset + scriptList.size();
  quint16 lookupListOffset = featureListOffset + featureList.size();

  root << (quint16)1 << (quint16)0 << scriptListOffset << featureListOffset << lookupListOffset;

  root.append(scriptList);
  root.append(featureList);

  quint16 lookupCount = lookups.size();

  quint32 lookupListtotalSize = 2 + 2 * lookupCount;

  int nbSubtablekt01expa1 = 0;

  for (int i = 0; i < lookups.size(); ++i) {

    Lookup* lookup = lookups.at(i);

    auto subtables = lookup->getSubtables(extended);

    quint32 nb_subtables = subtables.size();

    if (lookup->name == "kt01.expa.1") {
      nbSubtablekt01expa1 = nb_subtables;
    }

    auto expaLookup = lookup->name == "kt02.expa.1" || lookup->name == "kt03.expa.1" || lookup->name == "kt04.expa.1" || lookup->name == "kt05.expa.1";

    if (expaLookup) {
      nb_subtables = nbSubtablekt01expa1;
    }

    //lookup header
    lookupListtotalSize += 2 + 2 + 2 + 2 * nb_subtables;

    if (lookup->markGlyphSetIndex != -1) {
      lookupListtotalSize += 2;
    }

    //extension subtables
    lookupListtotalSize += 8 * nb_subtables;

  }

  QByteArray lookupList;
  QByteArray lookups_array;
  lookupList << lookupCount;

  quint16 beginoffset = 2 + 2 * lookupCount;
  quint16 extensiontype = isgsub ? Lookup::extensiongsub : Lookup::extensiongpos;
  QByteArray subtablesArray;

  quint32 subtablesOffset = lookupListtotalSize;

  std::vector<quint32> subtablesOffsetkt01expa1;

  for (int i = 0; i < lookups.size(); ++i) {

    Lookup* lookup = lookups.at(i);

    auto expaLookup = lookup->name == "kt02.expa.1" || lookup->name == "kt03.expa.1" || lookup->name == "kt04.expa.1" || lookup->name == "kt05.expa.1";

    auto subtables = lookup->getSubtables(extended);

    quint32 nb_subtables = subtables.size();

    if (expaLookup) {
      nb_subtables = nbSubtablekt01expa1;
    }

    QByteArray lookupArray;

    lookupArray << extensiontype;
    lookupArray << lookup->flags;
    lookupArray << (quint16)nb_subtables;

    quint16 debutsequence = 2 + 2 + 2 + 2 * nb_subtables;

    if (lookup->markGlyphSetIndex != -1) {
      debutsequence += 2;
    }

    QByteArray exttables_array;


    for (int i = 0; i < nb_subtables; ++i) {



      lookupArray << debutsequence;


      quint16 lookuptype = (quint16)lookup->type;

      if (lookup->name == "kt01.expa.1") {
        subtablesOffsetkt01expa1.push_back(subtablesOffset);
      }

      if (expaLookup) {
        exttables_array << (quint16)1 << lookuptype << (quint32)(subtablesOffsetkt01expa1[i] - (beginoffset + debutsequence));
      }
      else {

        exttables_array << (quint16)1 << lookuptype << (quint32)(subtablesOffset - (beginoffset + debutsequence));

        QByteArray subtableArray;

        auto& subtable = subtables.at(i);

        if (!extended && subtable->isConvertible()) {
          subtableArray = subtable->getConvertedOpenTypeTable();
        }
        else {
          subtableArray = subtable->getOptOpenTypeTable(extended);
        }




        if (lookup->type != Lookup::fsmgsub && subtableArray.size() > 0xFFFF) {
          std::cout << lookup->name.toStdString() << " : Subtable " << subtable->name.toStdString() << " exceeds the limit of 64K" << std::endl;
          //throw std::runtime_error{ "Subtable exeeded the limit of 64K" };
        }



        subtablesArray.append(subtableArray);

        subtablesOffset += subtableArray.size();
      }



      debutsequence += 8;
    }

    if (lookup->markGlyphSetIndex != -1) {
      lookupArray << lookup->markGlyphSetIndex;
    }

    lookupArray.append(exttables_array);
    lookups_array.append(lookupArray);

    lookupList << beginoffset;

    beginoffset += lookupArray.size();

  }

  lookupList.append(lookups_array);
  lookupList.append(subtablesArray);


  root.append(lookupList);

  return root;

}
/*
QByteArray OtLayout::getGSUBorGPOS(bool isgsub) {

  QByteArray scriptList = getScriptList(isgsub);
  QByteArray featureList = getFeatureList(isgsub);


  QByteArray root;

  quint16 scriptListOffset = 10;
  quint16 featureListOffset = scriptListOffset + scriptList.size();
  quint16 lookupListOffset = featureListOffset + featureList.size();

  root << (quint16)1 << (quint16)0 << scriptListOffset << featureListOffset << lookupListOffset;

  root.append(scriptList);
  root.append(featureList);

  //lookupList


  QVector<Lookup*> lookups;

  if (isgsub) {
    lookups = gsublookups;
  }
  else {
    lookups = gposlookups;
  }

  quint16 lookupCount = lookups.size();

  QByteArray lookupList;
  QByteArray lookups_array;


  quint16 beginoffset = 2 + 2 * lookupCount;

  lookupList << lookupCount;

  for (int i = 0; i < lookups.size(); ++i) {

    QByteArray temp = lookups.at(i)->getOpenTypeTable();

    lookupList << beginoffset;
    lookups_array.append(temp);

    beginoffset += temp.size();
  }

  lookupList.append(lookups_array);


  root.append(lookupList);

  return root;

}*/

#if DIGITALKHATT_WEBLIB
OtLayout::OtLayout(MP mp, bool extended) : fsmDriver{ *this }, justTable{ this } {
#else
OtLayout::OtLayout(MP mp, bool extended, QObject * parent) :QObject(parent), fsmDriver{ *this }, justTable{ this } {
#endif

  this->extended = extended;
  face = hb_face_create_for_tables(harfbuzzGetTables, this, 0);

  dirty = true;

  this->mp = mp;

  if (strcmp(mp->job_name, "digitalkhatt") == 0) {
    automedina = new digitalkhatt(this, mp, extended);
  }
  else if (strcmp(mp->job_name, "oldmadina") == 0) {
    automedina = new OldMadina(this, mp, extended);
  }
  else if (strcmp(mp->job_name, "indopak") == 0) {
    automedina = new IndoPak(this, mp, extended);
  }
  else {
    throw new std::runtime_error("invalid font");
  }



  nuqta();

  /*
  if (!extended) {

    loadLookupFile("automedina.fea");

    QSet<QString> newhaslefttatweel = automedina->classes["haslefttatweel"];

    for (auto name : automedina->classes["haslefttatweel"]) {

      auto code = glyphCodePerName.value(name);
      GlyphParameters parameters{};

      auto tatweels = { 1.0,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0,6.5 };

      for (auto tatweel : tatweels) {
        parameters.lefttatweel = tatweel;
        parameters.righttatweel = 0;

        GlyphVis* glyph = getAlternate(code, parameters);
        if (tatweel <= 4) {
          newhaslefttatweel.insert(glyph->name);
        }

      }
    }

    automedina->classes["haslefttatweel"] = newhaslefttatweel;


  }*/

  toOpenType = new ToOpenType(this);

  toOpenType->populateGlyphs();

}
OtLayout::~OtLayout() {
  for (auto lookup : lookups) {
    delete lookup;
  }
  clearAlternates();

  delete face;
  delete automedina;
  delete toOpenType;
}

void OtLayout::generateSubstEquivGlyphs() {


  if (!extended && substEquivGlyphs.size() == 0) {

    automedina->generateSubstEquivGlyphs();
    for (auto lookup : lookups) {
      if (!disabledLookups.contains(lookup)) {
        if (lookup->isGsubLookup() && lookup->type != Lookup::SubType::fsmgsub) {
          auto subtables = lookup->getSubtables(extended);
          for (auto subtable : subtables) {
            subtable->generateSubstEquivGlyphs();
          }
        }
      }
    }
  }
}

void OtLayout::clearAlternates() {
  for (auto& glyph : tempGlyphs) {
    for (auto& path : glyph.second) {
      delete path.second;
    }
    //glyph.second.clear();
  }
  /*
  for (auto& glyph : nojustalternatePaths) {
    for (auto& path : glyph.second) {
      delete path.second;
    }
    //glyph.second.clear();
  }*/

  tempGlyphs.clear();

}

CalcAnchor OtLayout::getanchorCalcFunctions(QString functionName, Subtable * subtable) {
  return automedina->getanchorCalcFunctions(functionName, subtable);
}
/*
void OtLayout::prepareJSENgine() {

  evaluateImport();

  QJSValue descriptions = myEngine.newObject();
  myEngine.globalObject().setProperty("desc", descriptions);
  QJSValue classes = myEngine.globalObject().property("classes");
  QJSValue marksClass = classes.property("marks");
  QJSValue basesClass = myEngine.newObject();
  classes.setProperty("bases", basesClass);

  for (int i = 0; i < m_font->glyphs.length(); i++) {

    Glyph* curr = m_font->glyphs[i];

    Glyph::ComputedValues & values = curr->getComputedValues();

    QJSValue item = myEngine.newObject();

    item.setProperty("width", values.width);
    item.setProperty("height", values.height);
    item.setProperty("depth", values.depth);
    item.setProperty("charcode", values.charcode);
    QJSValue bbox = myEngine.newObject();

    bbox.setProperty("llx", values.bbox.llx);
    bbox.setProperty("lly", values.bbox.lly);
    bbox.setProperty("urx", values.bbox.urx);
    bbox.setProperty("ury", values.bbox.ury);

    item.setProperty("boundingbox", bbox);
    item.setProperty("name", curr->name());


    if (values.leftAnchor.has_value()) {
      QJSValue leftAnchor = myEngine.newObject();
      leftAnchor.setProperty("x", values.leftAnchor.value().x());
      leftAnchor.setProperty("y", values.leftAnchor.value().y());
      item.setProperty("leftanchor", leftAnchor);
    }

    if (values.rightAnchor.has_value()) {
      QJSValue rightAnchor = myEngine.newObject();
      rightAnchor.setProperty("x", values.rightAnchor.value().x());
      rightAnchor.setProperty("y", values.rightAnchor.value().y());
      item.setProperty("rightanchor", rightAnchor);
    }

    descriptions.setProperty(curr->name(), item);

    if (!marksClass.hasProperty(curr->name())) {
      basesClass.setProperty(curr->name(), true);
      glyphGlobalClasses[curr->name()] = BaseGlyph;
    }
    else {
      glyphGlobalClasses[curr->name()] = MarkGlyph;
    }
  }


  //QJSValueIterator it(myEngine.globalObject().property("desc"));
  //while (it.hasNext()) {
  //	it.next();
    //std::cout << it.name().toLatin1().data() << ": " << it.value().toString().toLatin1().data() << "\n";
    //ebug() << it.name() << ": " << it.value().toString();
  //}



}

void OtLayout::evaluateImport() {
  QString fileName = import;
  QFile scriptFile(fileName);
  if (!scriptFile.open(QIODevice::ReadOnly)) {
    return;
  }
  // handle error
  QTextStream stream(&scriptFile);
  QString contents = stream.readAll();
  scriptFile.close();
  QJSValue result = myEngine.evaluate(contents, fileName);

  if (result.isError()) {
    qDebug()
      << "Uncaught exception at line"
      << result.property("lineNumber").toInt()
      << ":" << result.toString();
    //printf("Uncaught exception at line %d :\n%s\n", result.property("lineNumber").toInt(), result.toString().toLatin1().constData());
  }

}*/
void OtLayout::addLookup(Lookup * lookup) {
  if (lookup->type == Lookup::none) {
    throw "Lookup Type not defined";
  }

  if (lookup->name.isEmpty()) {
    throw "Lookup name not defined";
  }

  if (!lookup->feature.isEmpty()) {
    allFeatures[lookup->feature].insert(lookup);
  }

  quint16 lookupIndex = lookups.size();

  lookupsIndexByName[lookup->name] = lookupIndex;
  lookups.append(lookup);


}

void OtLayout::loadLookupFile(std::string fileName) {

  parseFeatureFile(fileName);

  std::ifstream parametersStream("parameters.json", std::ios::binary);

  if (parametersStream) {
    // get length of file:
    parametersStream.seekg(0, parametersStream.end);
    int length = parametersStream.tellg();
    parametersStream.seekg(0, parametersStream.beg);

    char* buffer = new char[length];

    parametersStream.read(buffer, length);

    if (!parametersStream) {
      std::cout << "Problem reading file." << fileName;
    }
    else {
      QJsonDocument mDocument = QJsonDocument::fromJson(QByteArray::fromRawData(buffer, length));
      this->readParameters(mDocument.object());

    }

    parametersStream.close();
    delete[] buffer;

  }

  //addGlyphs();
}

void OtLayout::parseFeatureFile(std::string fileName)
{

  for (auto lookup : lookups) {
    delete lookup;
  }
  lookups.clear();
  lookupsIndexByName.clear();
  gsublookups.clear();
  gposlookups.clear();
  markGlyphSets.clear();
  allGposFeatures.clear();
  allGsubFeatures.clear();
  gsublookupsIndexByName.clear();
  gposlookupsIndexByName.clear();
  automedina->cachedClasstoUnicode.clear();
  //automedina->cvxxfeatures.clear();
  allFeatures.clear();
  disabledLookups.clear();
  tables.clear();
  //nojustalternatePaths.clear();



  feayy::FeaContext context{ this };

  feayy::Driver driver(context);
  if (!driver.parse_file(fileName)) {
    std::cout << "Error in parsing " << fileName << endl;
  };

  context.populateFeatures();

  if (face != nullptr) {
    hb_face_destroy(face);
    face = nullptr;
  }

}
void OtLayout::parseCppLookup(QString lookupName) {

  Lookup* newlookup = automedina->getLookup(lookupName);
  if (newlookup) {
    addLookup(newlookup);
  }
}
void OtLayout::saveParameters(QJsonObject & json) const {
  for (auto lookup : lookups) {
    if (!lookup->isGsubLookup()) {
      QJsonObject lookupObject;
      lookup->saveParameters(lookupObject);
      if (!lookupObject.isEmpty()) {
        json[lookup->name] = lookupObject;
      }
    }

  }
}
void OtLayout::readParameters(const QJsonObject & json) {
  for (auto lookup : lookups) {
    if (!lookup->isGsubLookup()) {
      if (!json[lookup->name].toObject().isEmpty()) {
        lookup->readParameters(json[lookup->name].toObject());
      }
    }
  }
}
void OtLayout::addClass(QString name, QSet<QString> set) {
  if (automedina->classes.contains(name)) {
    if (name == "haslefttatweel" && automedina->extended == false) {
      return;
    }
    //throw "Class " + name + " Already exists";
  }
  automedina->classes[name] = set;
}
hb_font_t* OtLayout::createFont(double emScale, bool newFace)
{
  int upem = 1000;

  if (newFace || face == nullptr) {
    if (face != nullptr) {
      hb_face_destroy(face);
      face = nullptr;
    }


    face = hb_face_create_for_tables(harfbuzzGetTables, this, 0);
    hb_face_set_upem(face, upem);
  }

  hb_font_t* font = hb_font_create(face);
  hb_font_set_ppem(font, upem, upem);
  const int scale = emScale * upem; // // (1 << OtLayout::SCALEBY) * static_cast<int>(size);
  hb_font_set_scale(font, scale, scale);

  hb_font_t* subfont = hb_font_create_sub_font(font);

  /*

    hb_font_funcs_t * ffunctions = hb_font_funcs_create();
  hb_font_funcs_set_nominal_glyph_func(ffunctions, func, user_data, destroy);
  hb_font_set_funcs(subfont, ffunctions, font_data, destroy);
  hb_font_funcs_destroy(ffunctions);*/

  hb_font_set_funcs(subfont, getFontFunctions(font, useNormAxisValues), this, 0);


  return subfont;
}


quint16 OtLayout::addMarkSet(QList<quint16> list) {

  quint16 index = markGlyphSets.size();

  markGlyphSets.append(list);

  return index;
}
quint16 OtLayout::addMarkSet(QVector<QString> list) {
  QList<quint16> codeList;
  for (auto glyphName : list) {
    if (quint16 glyphcode = glyphCodePerName.value(glyphName, 0)) {
      codeList.append(glyphcode);
    }
    else {
      std::cout << "addMarkSet : Glyph Name '" << glyphName.toStdString() << "' does not exist.\n";
    }
  }

  return addMarkSet(codeList);

}
QSet<quint16> OtLayout::classtoUnicode(QString className) {
  return automedina->classtoUnicode(className);
}

QSet<quint16> OtLayout::getSubsts(int charCode) {
  QSet<quint16> set;
  auto addedGlyphs = substEquivGlyphs.find(charCode);
  if (addedGlyphs != substEquivGlyphs.end()) {
    for (auto& addedGlyph : addedGlyphs->second) {
      set.insert(addedGlyph.second->charcode);
    }
  }
  return set;
}

QSet<quint16> OtLayout::regexptoUnicode(QString regexp) {
  return automedina->regexptoUnicode(regexp);
}

QSet<QString> OtLayout::classtoGlyphName(QString className) {

  return automedina->classtoGlyphName(className);
  /*
      QSet<QString> names;

      QJSValue classes = myEngine.globalObject().property("classes");

      if (!classes.hasOwnProperty(className)) {
          Glyph* glyph = m_font->glyphperName[className];
          if (glyph) {
              names.insert(glyph->name());
          }
      }
      else {
          QJSValue classObject = classes.property(className);

          QJSValueIterator it(classObject);
          while (it.hasNext()) {
              it.next();
              names.unite(classtoGlyphName(it.name()));
          }
      }

      return names;*/
}

double OtLayout::nuqta() {
  if (_nuqta == -1) {
    _nuqta = getNumericVariable("nuqta");
  }

  return _nuqta;
}

double OtLayout::getNumericVariable(QString name) {
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

#ifndef DIGITALKHATT_WEBLIB
void OtLayout::setParameter(quint16 glyphCode, quint32 lookup, quint32 subtableIndex, quint16 markCode, quint16 baseCode, QPoint displacement, Qt::KeyboardModifiers modifiers) {

  bool shift = false;
  bool ctrl = false;
  bool alt = false;

  if (modifiers & Qt::ShiftModifier) {
    shift = true;
  }

  if (modifiers & Qt::ControlModifier) {
    ctrl = true;
  }

  if (modifiers & Qt::AltModifier) {
    alt = true;
  }

  Lookup* lookupTable = gposlookups.at(lookup);

  auto subtable = lookupTable->subtables.at(subtableIndex);

  if (lookupTable->type == Lookup::singleadjustment) {
    SingleAdjustmentSubtable* subtableTable = static_cast<SingleAdjustmentSubtable*>(subtable);
    QString glyphName = glyphNamePerCode[markCode];

    ValueRecord prev = subtableTable->parameters[markCode];

    ValueRecord newvalue{ (qint16)(prev.xPlacement + displacement.x()),(qint16)(prev.yPlacement + displacement.y()),prev.xAdvance,(qint16)0 };

    if (shift) {
      newvalue.xAdvance += displacement.x();
    }
    else if (alt) {
      newvalue.xAdvance -= displacement.x();
    }

    subtableTable->parameters[markCode] = newvalue;

    qDebug() << QString("Changing single adjust anchor %1.%2.%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue.xPlacement << newvalue.yPlacement << newvalue.xAdvance;

    subtableTable->isDirty = true;

    emit parameterChanged();
  }
  else if (lookupTable->type == Lookup::mark2base || lookupTable->type == Lookup::mark2mark) {

    MarkBaseSubtable* subtableTable = static_cast<MarkBaseSubtable*>(subtable);

    quint16 classIndex = subtableTable->markCodes[markCode];

    QString className = subtableTable->classNamebyIndex[classIndex];



    if (!shift) {

      QString baseGlyphName = glyphNamePerCode[baseCode];

      GlyphVis& curr = glyphs[baseGlyphName];

      if (ctrl && !curr.originalglyph.isEmpty() && (curr.charlt != 0 || curr.charrt != 0)) {
        baseGlyphName = curr.originalglyph;
      }


      QPoint prev = subtableTable->classes[className].baseparameters[baseGlyphName];

      QPoint newvalue = prev + displacement;

      subtableTable->classes[className].baseparameters[baseGlyphName] = newvalue;

      qDebug() << QString("Changing base anchor %1::%2::%3::%4 : (%5,%6)").arg(lookupTable->name, subtable->name, className, baseGlyphName, QString::number(newvalue.x()), QString::number(newvalue.y()));

    }
    else {
      QString markGlyphName = glyphNamePerCode[markCode];
      QPoint prev = subtableTable->classes[className].markparameters[markGlyphName];

      QPoint newvalue = prev - displacement;

      subtableTable->classes[className].markparameters[markGlyphName] = prev - displacement;

      qDebug() << QString("Changing mark anchor %1::%2::%3::%4 : (%5,%6)").arg(lookupTable->name, subtable->name, className, markGlyphName, QString::number(newvalue.x()), QString::number(newvalue.y()));
    }
    subtableTable->isDirty = true;



    //qDebug() << "prev : " << prev << "new" << subtableTable->classes[className].baseparameters[baseGlyphName];



    emit parameterChanged();

  }
  else if (lookupTable->type == Lookup::cursive) {
    CursiveSubtable* subtableTable = static_cast<CursiveSubtable*>(subtable);

    QString glyphName = glyphNamePerCode[glyphCode];

    QString baseGlyphName = glyphNamePerCode[baseCode];

    GlyphVis& curr = glyphs[glyphName];

    Lookup* lookup = subtableTable->getLookup();

    //if (lookup->flags & Lookup::RightToLeft) {
    //	QPoint newvalue = subtableTable->exitParameters[glyphCode] + displacement;
    //	subtableTable->exitParameters[glyphCode] = newvalue;

    //    qDebug() << QString("Changing cursive exit anchor %1::%2::%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue;
    //}
    //else {
    if (!shift) {
      QPoint newvalue = subtableTable->entryParameters[glyphCode] - displacement;
      subtableTable->entryParameters[glyphCode] = newvalue;

      qDebug() << QString("Changing cursive entry anchor %1::%2::%3 :").arg(lookupTable->name, subtable->name, glyphName) << newvalue;
    }
    else {
      QPoint newvalue = subtableTable->exitParameters[baseCode] + displacement;
      subtableTable->exitParameters[baseCode] = newvalue;

      qDebug() << QString("Changing cursive exit anchor %1::%2::%3 :").arg(lookupTable->name, subtable->name, baseGlyphName) << newvalue;
    }
    //}

    subtableTable->isDirty = true;

    emit parameterChanged();


  }
}
#endif

void OtLayout::applyJustFeature(hb_buffer_t * buffer, bool& needgpos, double& diff, QString feature, hb_font_t * shapefont, double nuqta, double emScale) {

  if (!this->allGsubFeatures.contains(feature))
    return;

  const unsigned int table_index = 0u;
  buffer->reverse();

  uint glyph_count;

  hb_buffer_t* copy_buffer = nullptr;
  copy_buffer = hb_buffer_create();
  hb_buffer_append(copy_buffer, buffer, 0, -1);

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(copy_buffer, &glyph_count);

  OT::hb_ot_apply_context_t c(table_index, shapefont, buffer);
  c.set_recurse_func(OT::SubstLookup::template dispatch_recurse_func<
    OT::hb_ot_apply_context_t>);
  auto list = this->allGsubFeatures[feature].values();
  std::sort(list.begin(), list.end());

  bool stretch = diff > 0;

  /*
  for (auto& table : tables) {
    if (table->name == "just") {
      for (auto& subtable : table->subtables) {
        if (auto dd = dynamic_cast<FSMSubtable*>(subtable)) {
          fsmDriver.executeFSM(*dd, buffer);
        }
      }
    }
  }*/

  for (auto lookup_index : list) {

    if (!((stretch && diff > 0) || (!stretch && diff < 0))) break;

    c.set_lookup_index(lookup_index);
    c.set_lookup_mask(2);
    c.set_auto_zwj(1);
    c.set_auto_zwnj(1);

    needgpos = true;
    justificationContext.clear();

    /*
    OT::JustificationContext justContext{ shapefont };
    buffer->justContext = &justContext;*/


    hb_ot_layout_substitute_lookup(&c,
      shapefont->face->table.GSUB->table->get_lookup(lookup_index),
      *shapefont->face->table.GSUB->accels[lookup_index].get_acquire());

    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

    int totalWeight = justificationContext.totalWeight;

    bool remaining = true;

    double remainingWidth = -1;

    while (totalWeight != 0 && remaining && remainingWidth != 0.0) {

      double expaUnit = diff / totalWeight;
      if (expaUnit == 0.0) {
        diff = 0.0;
        break;;
      }

      totalWeight = 0;

      QMap<int, GlyphExpansion > affectedIndexes;

      bool insideGroup = false;
      hb_position_t oldWidth = 0;
      hb_position_t newWidth = 0;
      GlyphExpansion groupExpa{};
      groupExpa.weight = 0;
      QVector<int> group;
      remaining = false;
      remainingWidth = 0.0;


      for (int i = 0; i < justificationContext.GlyphsToExtend.size(); i++) {

        int index = justificationContext.GlyphsToExtend[i];
        GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[justificationContext.Substitutes[i]]];

        GlyphExpansion& expa = justificationContext.Expansions[index];

        if (expa.stretchIsAbsolute) {
          expa.MaxLeftTatweel = expa.MaxLeftTatweel - glyph_info[index].lefttatweel;
          expa.MaxRightTatweel = expa.MaxRightTatweel - glyph_info[index].righttatweel;
          expa.stretchIsAbsolute = false;
        }

        if (expa.shrinkIsAbsolute) {
          expa.MinLeftTatweel = expa.MinLeftTatweel - glyph_info[index].lefttatweel;
          expa.MinRightTatweel = expa.MinRightTatweel - glyph_info[index].righttatweel;
          expa.shrinkIsAbsolute = false;
        }

        group.append(i);
        oldWidth += glyph_pos[index].x_advance;
        if (glyph_info[index].codepoint == justificationContext.Substitutes[i]) {
          newWidth += glyph_pos[index].x_advance;
        }
        else {
          newWidth += getGlyphHorizontalAdvance(shapefont, this, justificationContext.Substitutes[i], glyph_pos[index].lefttatweel, glyph_pos[index].righttatweel, nullptr); // substitute.width* emScale;
        }

        groupExpa.weight += expa.weight;
        groupExpa.MinLeftTatweel += expa.MinLeftTatweel;
        groupExpa.MaxLeftTatweel += expa.MaxLeftTatweel;
        groupExpa.MinRightTatweel += expa.MinRightTatweel;
        groupExpa.MaxRightTatweel += expa.MaxRightTatweel;


        if (expa.startEndLig == StartEndLig::Start) {
          insideGroup = true;
          continue;
        }
        else if (insideGroup && expa.startEndLig != StartEndLig::End && expa.startEndLig != StartEndLig::EndKashida) {
          continue;
        }

        int widthDiff = newWidth - oldWidth;

        auto tatweel = expaUnit * groupExpa.weight + remainingWidth;

        if (groupExpa.weight == 0) goto next;


        if ((stretch && widthDiff > tatweel) || (!stretch && widthDiff < tatweel)) {
          totalWeight += groupExpa.weight;
          remainingWidth = tatweel;
        }
        else {

          diff -= widthDiff;
          tatweel -= widthDiff;

          for (int i : group) {

            int index = justificationContext.GlyphsToExtend[i];
            GlyphExpansion& expa = justificationContext.Expansions[index];

            glyph_info[index].codepoint = justificationContext.Substitutes[i];

            if (stretch) {
              expa.MaxLeftTatweel = expa.MaxLeftTatweel > 0 ? expa.MaxLeftTatweel : 0;
              expa.MaxRightTatweel = expa.MaxRightTatweel > 0 ? expa.MaxRightTatweel : 0;

              auto maxTatweel = expa.MaxLeftTatweel + expa.MaxRightTatweel;
              if (maxTatweel != 0) {
                auto maxStretch = maxTatweel * nuqta;

                if (tatweel > maxStretch) {
                  remainingWidth = tatweel - maxStretch;
                  tatweel = maxStretch;
                }
                else {
                  remainingWidth = 0;
                }

                double leftTatweel = (tatweel * (expa.MaxLeftTatweel / maxTatweel)) / nuqta;
                double rightTatweel = (tatweel * (expa.MaxRightTatweel / maxTatweel)) / nuqta;

                glyph_info[index].lefttatweel += leftTatweel;
                glyph_info[index].righttatweel += rightTatweel;

                diff -= tatweel;

                if (maxStretch > tatweel) {
                  expa.MaxLeftTatweel -= leftTatweel;
                  expa.MaxRightTatweel -= rightTatweel;
                  remaining = true;
                  totalWeight += expa.weight;
                }
                else {
                  expa.weight = 0;
                }
              }
              else {
                expa.weight = 0;
              }
            }
            else {
              expa.MinLeftTatweel = expa.MinLeftTatweel < 0 ? expa.MinLeftTatweel : 0;
              expa.MinRightTatweel = expa.MinRightTatweel < 0 ? expa.MinRightTatweel : 0;

              auto MinTatweel = expa.MinLeftTatweel + expa.MinRightTatweel;

              if (MinTatweel != 0) {
                auto minShrink = MinTatweel * nuqta;

                if (tatweel < minShrink) {
                  remainingWidth = tatweel - minShrink;
                  tatweel = minShrink;
                }
                else {
                  remainingWidth = 0;
                }

                double leftTatweel = (tatweel * (expa.MinLeftTatweel / MinTatweel)) / nuqta;
                double rightTatweel = (tatweel * (expa.MinRightTatweel / MinTatweel)) / nuqta;

                glyph_info[index].lefttatweel += leftTatweel;
                glyph_info[index].righttatweel += rightTatweel;

                diff -= tatweel;

                if (tatweel > minShrink) {
                  expa.MinLeftTatweel -= leftTatweel;
                  expa.MinRightTatweel -= rightTatweel;
                  remaining = true;
                  totalWeight += expa.weight;
                }
                else {
                  expa.weight = 0;
                }
              }
              else {
                expa.weight = 0;
              }
            }
          }
        }

      next:
        insideGroup = false;
        oldWidth = 0.0;
        newWidth = 0.0;
        groupExpa = {};
        groupExpa.weight = 0;
        group.clear();
      }
    }
  }
  buffer->reverse();
  if (copy_buffer)
    hb_buffer_destroy(copy_buffer);

}

void OtLayout::applyJustFeature_old(hb_buffer_t * buffer, bool& needgpos, double& diff, QString feature, hb_font_t * shapefont, double nuqta, double emScale) {

  if (!this->allGsubFeatures.contains(feature))
    return;

  const unsigned int table_index = 0u;
  buffer->reverse();

  uint glyph_count;

  hb_buffer_t* copy_buffer = nullptr;
  copy_buffer = hb_buffer_create();
  hb_buffer_append(copy_buffer, buffer, 0, -1);

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(copy_buffer, &glyph_count);

  OT::hb_ot_apply_context_t c(table_index, shapefont, buffer);
  c.set_recurse_func(OT::SubstLookup::template dispatch_recurse_func<
    OT::hb_ot_apply_context_t>);
  auto list = this->allGsubFeatures[feature].values();
  std::sort(list.begin(), list.end());
  bool stretch = diff > 0;

  /*
  for (auto& table : tables) {
    if (table->name == "just") {
      for (auto& subtable : table->subtables) {
        if (auto dd = dynamic_cast<FSMSubtable*>(subtable)) {
          fsmDriver.executeFSM(*dd, buffer);
        }
      }
    }
  }*/

  for (auto lookup_index : list) {
    if ((stretch && diff > 0) || (!stretch && diff < 0)) {
      c.set_lookup_index(lookup_index);
      c.set_lookup_mask(2);
      c.set_auto_zwj(1);
      c.set_auto_zwnj(1);

      needgpos = true;
      justificationContext.clear();

      hb_ot_layout_substitute_lookup(&c,
        shapefont->face->table.GSUB->table->get_lookup(lookup_index),
        *shapefont->face->table.GSUB->accels[lookup_index].get_acquire());

      hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);

      if (justificationContext.GlyphsToExtend.size() != 0) {

        //double tatweel = diff / JustificationContext::GlyphsToExtend.count() / nuqta;

        QMap<int, GlyphExpansion > affectedIndexes;

        bool insideGroup = false;
        hb_position_t oldWidth = 0;
        hb_position_t newWidth = 0;
        QVector<int> group;

        for (int i = 0; i < justificationContext.GlyphsToExtend.size(); i++) {

          int index = justificationContext.GlyphsToExtend[i]; // glyph_count - 1 - JustificationContext::GlyphsToExtend[i];
          GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[justificationContext.Substitutes[i]]];

          GlyphExpansion& expa = justificationContext.Expansions[index];

          group.append(i);
          oldWidth += glyph_pos[index].x_advance;
          if (glyph_info[index].codepoint == justificationContext.Substitutes[i]) {
            newWidth += glyph_pos[index].x_advance;
          }
          else {
            double leftTatweel = glyph_pos[index].lefttatweel + expa.MinLeftTatweel > 0 ? expa.MinLeftTatweel : 0;
            double rightTatweel = glyph_pos[index].righttatweel + expa.MinRightTatweel > 0 ? expa.MinRightTatweel : 0;
            newWidth += getGlyphHorizontalAdvance(shapefont, this, justificationContext.Substitutes[i], leftTatweel, rightTatweel, nullptr); // substitute.width* emScale;
          }


          if (expa.startEndLig == StartEndLig::Start) {
            insideGroup = true;
            continue;
          }
          else if (insideGroup && expa.startEndLig != StartEndLig::End && expa.startEndLig != StartEndLig::EndKashida) {
            continue;
          }

          auto expansion = newWidth - oldWidth;

          if ((stretch && expansion <= diff && expansion >= 0) || (!stretch && expansion >= diff && expansion <= 0)) {

            diff -= expansion;

            for (int i : group) {

              int index = justificationContext.GlyphsToExtend[i];
              GlyphExpansion& expa = justificationContext.Expansions[index];

              if (glyph_info[index].codepoint == justificationContext.Substitutes[i]) {

                expa.MaxLeftTatweel = expa.MaxLeftTatweel - glyph_info[index].lefttatweel;
                expa.MaxRightTatweel = expa.MaxRightTatweel - glyph_info[index].righttatweel;

                if (stretch && expa.MaxLeftTatweel <= 0 && expa.MaxRightTatweel <= 0
                  || !stretch && expa.MinLeftTatweel >= 0 && expa.MinRightTatweel >= 0)
                  continue;

                affectedIndexes.insert(i, expa);
              }
              else {
                //GlyphVis& glyph = this->glyphs[this->glyphNamePerCode[glyph_info[index].codepoint]];
                //GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[JustificationContext::Substitutes[i]]];

                //auto minStretch = (substitute.width - glyph.width) * emScale; // +JustificationContext::Expansions[index].MinLeftTatweel * nuqta;

                glyph_info[index].codepoint = justificationContext.Substitutes[i];
                if (expa.MinLeftTatweel > 0) {
                  glyph_info[index].lefttatweel += expa.MinLeftTatweel;
                  expa.MinLeftTatweel = 0;
                }

                if (expa.MinRightTatweel > 0) {
                  glyph_info[index].righttatweel += expa.MinRightTatweel;
                  expa.MinRightTatweel = 0;
                }

                if (stretch) {

                  if (expa.MaxLeftTatweel > 0 || expa.MaxRightTatweel > 0) {
                    affectedIndexes.insert(i, expa);
                  }
                }
                else if (!stretch) {

                  if (expa.MinLeftTatweel < 0 || expa.MinRightTatweel < 0) {
                    affectedIndexes.insert(i, expa);
                  }
                }
              }
            }

          }

          insideGroup = false;
          oldWidth = 0.0;
          newWidth = 0.0;
          group.clear();
        }

        while (affectedIndexes.size() != 0) {
          double meanTatweel = diff / (affectedIndexes.size());

          if (meanTatweel == 0.0) break;

          QMap<int, GlyphExpansion >::iterator i;
          QMap<int, GlyphExpansion > newaffectedIndexes;
          for (i = affectedIndexes.begin(); i != affectedIndexes.end(); ++i) {

            int index = justificationContext.GlyphsToExtend[i.key()]; // glyph_count - 1 - JustificationContext::GlyphsToExtend[i];

            auto expa = i.value();

            if (stretch) {
              expa.MaxLeftTatweel = expa.MaxLeftTatweel > 0 ? expa.MaxLeftTatweel : 0;
              expa.MaxRightTatweel = expa.MaxRightTatweel > 0 ? expa.MaxRightTatweel : 0;

              auto MaxTatweel = expa.MaxLeftTatweel + expa.MaxRightTatweel;
              auto maxStretch = MaxTatweel * nuqta;


              auto tatweel = meanTatweel;

              if (meanTatweel > maxStretch) {
                tatweel = maxStretch;
              }

              diff -= tatweel;

              double leftTatweel = (tatweel * (expa.MaxLeftTatweel / MaxTatweel)) / nuqta;
              double rightTatweel = (tatweel * (expa.MaxRightTatweel / MaxTatweel)) / nuqta;

              glyph_info[index].lefttatweel += leftTatweel;
              glyph_info[index].righttatweel += rightTatweel;

              if (meanTatweel < maxStretch && diff > 0) {
                expa.MaxLeftTatweel -= leftTatweel;
                expa.MaxRightTatweel -= rightTatweel;
                newaffectedIndexes.insert(i.key(), expa);
              }

            }
            else {
              expa.MinLeftTatweel = expa.MinLeftTatweel < 0 ? expa.MinLeftTatweel : 0;
              expa.MinRightTatweel = expa.MinRightTatweel < 0 ? expa.MinRightTatweel : 0;

              auto MinTatweel = expa.MinLeftTatweel + expa.MinRightTatweel;
              auto minShrink = MinTatweel * nuqta;


              auto tatweel = meanTatweel;

              if (meanTatweel < minShrink) {
                tatweel = minShrink;
              }

              diff -= tatweel;

              double leftTatweel = (tatweel * (expa.MinLeftTatweel / MinTatweel)) / nuqta;
              double rightTatweel = (tatweel * (expa.MinRightTatweel / MinTatweel)) / nuqta;

              glyph_info[index].lefttatweel += leftTatweel;
              glyph_info[index].righttatweel += rightTatweel;

              if (meanTatweel > minShrink && diff < 0) {
                expa.MinLeftTatweel -= leftTatweel;
                expa.MinRightTatweel -= rightTatweel;
                newaffectedIndexes.insert(i.key(), expa);
              }
            }
          }
          affectedIndexes = newaffectedIndexes;
        }
      }
    }
    else {
      continue;
    }
  }
  buffer->reverse();
  if (copy_buffer)
    hb_buffer_destroy(copy_buffer);

}

void OtLayout::jutifyLine_old(hb_font_t * shapefont, hb_buffer_t * text_buffer, int lineWidth, double emScale, bool tajweedColor) {

  const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
  const int  defaultSpace = OtLayout::SPACEWIDTH * emScale;
  double nuqta = this->nuqta() * emScale;

  auto  copy_buffer_properties = [](hb_buffer_t* dst, hb_buffer_t* src)
  {
    hb_segment_properties_t props;
    hb_buffer_get_segment_properties(src, &props);
    hb_buffer_set_segment_properties(dst, &props);
    hb_buffer_set_flags(dst, hb_buffer_get_flags(src));
    hb_buffer_set_cluster_level(dst, hb_buffer_get_cluster_level(src));
  };


  auto copyBuffer = [&](hb_buffer_t* des_buffer, hb_buffer_t* source_buffer)
  {
    hb_buffer_clear_contents(des_buffer);
    copy_buffer_properties(des_buffer, source_buffer);
    hb_buffer_append(des_buffer, source_buffer, 0, -1);
  };

  hb_feature_t color_fea{ HB_TAG('t', 'j', 'w', 'd'),0,0,(uint)-1 };
  if (tajweedColor) {
    color_fea.value = 1;
  }

  hb_feature_t gpos_features[] = {
    {HB_TAG('i', 'n', 'i', 't'),0,0,(uint)-1},
    {HB_TAG('m', 'e', 'd', 'i'),0,0,(uint)-1},
    {HB_TAG('f', 'i', 'n', 'a'),0,0,(uint)-1},
    {HB_TAG('r', 'l', 'i', 'g'),0,0,(uint)-1},
    {HB_TAG('l', 'i', 'g', 'a'),0,0,(uint)-1},
    {HB_TAG('c', 'a', 'l', 't'),0,0,(uint)-1},
    {HB_TAG('s', 'c', 'h', 'm'),1,0,(uint)-1},
    {HB_TAG('s', 'h', 'r', '1'),0,0,(uint)-1},
    color_fea
  };

  int num_gpos_features = sizeof(gpos_features) / sizeof(*gpos_features);

  uint glyph_count;

  hb_segment_properties_t savedprops;

  hb_buffer_get_segment_properties(text_buffer, &savedprops);


  hb_buffer_t* buffer = hb_buffer_create();
  copyBuffer(buffer, text_buffer);

  /*buffer->justifyLine = true;
  buffer->lineWidth = lineWidth;*/

  hb_shape(shapefont, buffer, &color_fea, 1);


  if (applyJustification && lineWidth != 0) {

    JustificationInProgress = true;
    bool continueJustification = true;
    bool schr1applied = false;
    while (continueJustification) {

      continueJustification = false;

      hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
      hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
      QVector<quint32> spaces;
      int currentlineWidth = 0;


      for (int i = glyph_count - 1; i >= 0; i--) {
        if (glyph_info[i].codepoint == 32) {
          glyph_pos[i].x_advance = minSpace;
          spaces.append(i);
        }
        else {
          currentlineWidth += glyph_pos[i].x_advance;
        }
      }

      double diff = (double)lineWidth - currentlineWidth - spaces.size() * (double)defaultSpace;

      bool needgpos = false;
      if (diff > 0) {
        applyJustFeature(buffer, needgpos, diff, "sch1", shapefont, nuqta, emScale);
      }
      //shrink
      else {

        if (!schr1applied) {
          hb_feature_t festures2[2];
          festures2[0].tag = HB_TAG('s', 'h', 'r', '1');
          festures2[0].value = 1;
          festures2[0].start = 0;
          festures2[0].end = -1;

          festures2[1] = color_fea;

          copyBuffer(buffer, text_buffer);

          hb_shape(shapefont, buffer, festures2, 2);

          continueJustification = true;
          schr1applied = true;
        }
        else {
          applyJustFeature(buffer, needgpos, diff, "shr2", shapefont, nuqta, emScale);
        }
      }

      if (needgpos) {
        buffer->reverse();
#ifndef HB_NO_JUSTIFICATION
        buffer->justContext = nullptr;
#endif
        gpos_features[7].value = schr1applied ? 1 : 0;;

        hb_shape(shapefont, buffer, gpos_features, num_gpos_features);
        //hb_shape(shapefont, buffer, nullptr, 0);
      }
    }
    JustificationInProgress = false;
  }
  copyBuffer(text_buffer, buffer);

  hb_buffer_destroy(buffer);
}

void OtLayout::jutifyLine(hb_font_t * shapefont, hb_buffer_t * text_buffer, int lineWidth, bool tajweedColor) {

#ifndef HB_NO_JUSTIFICATION
  if (applyJustification && lineWidth != 0) {
    hb_buffer_set_justify(text_buffer, lineWidth);
    //text_buffer->justifyLine = true;    
    //text_buffer->lineWidth = lineWidth;
  }

  text_buffer->justContext = nullptr;
#endif
  hb_feature_t features[2];

  features[0].tag = HB_TAG('s', 'h', 'r', '1');
  features[0].value = 10;
  features[0].start = -1;
  features[0].end = 0;

  features[1].tag = HB_TAG('t', 'j', 'w', 'd');
  if (tajweedColor) {
    features[1].value = 1;
  }
  else {
    features[1].value = 0;
  }
  features[1].start = 0;
  features[1].end = -1;

  JustificationInProgress = applyJustification; // true;
  hb_shape(shapefont, text_buffer, features, 2);
  /*
  if (tajweedColor) {
    hb_feature_t color_fea{ HB_TAG('t', 'j', 'w', 'd'),0,0,(uint)-1 };
    color_fea.value = 1;

    hb_shape(shapefont, text_buffer, &color_fea, 1);
  }
  else {
    hb_shape(shapefont, text_buffer, nullptr, 0);
  }*/
  JustificationInProgress = false;

}

QList<LineLayoutInfo> OtLayout::justifyPage(double emScale, int lineWidth, int pageWidth, QStringList lines, LineJustification justification,
  bool newFace, bool tajweedColor, bool changeSize, hb_buffer_cluster_level_t  cluster_level, JustType justType) {


  if (justType == JustType::Features) {
    return justifyPageUsingFeatures(emScale, lineWidth, pageWidth, lines, justification, newFace, tajweedColor, changeSize, cluster_level);
  }

  QList<LineLayoutInfo> page;

  hb_buffer_t* buffer = buffer = hb_buffer_create();
  hb_font_t* shapefont = this->createFont(emScale, newFace);

  int currentyPos = TopSpace << OtLayout::SCALEBY;

  hb_segment_properties_t savedprops;
  savedprops.direction = HB_DIRECTION_RTL;
  savedprops.script = HB_SCRIPT_ARABIC;
  savedprops.language = hb_language_from_string("ar", strlen("ar"));

  savedprops.reserved1 = 0;
  savedprops.reserved2 = 0;

  auto initializeBuffer = [&](hb_buffer_t* buffer, hb_segment_properties_t* savedprops, QString& line)
  {
    hb_buffer_clear_contents(buffer);
    hb_buffer_set_segment_properties(buffer, savedprops);
    hb_buffer_set_cluster_level(buffer, cluster_level);
    auto newLine = line; //QString("\n") + line + QString("\n");
    auto lineLength = newLine.length();
    hb_buffer_add_utf16(buffer, newLine.utf16(), lineLength, 0, lineLength);
  };

  hb_font_t* currentFont = nullptr;

  for (auto& line : lines) {

    bool first = true;
    bool overfull = false;
    currentFont = shapefont;
    double fontSize = emScale;

    uint glyph_count;

    while (first || overfull)
    {

      first = false;

      initializeBuffer(buffer, &savedprops, line);
#ifndef HB_NO_JUSTIFICATION
      buffer->useCallback = !useNormAxisValues;
#endif
      if (justType == JustType::HarfBuzz || justType == JustType::None) {
        jutifyLine(currentFont, buffer, lineWidth, tajweedColor);
      }
      else {
        jutifyLine_old(currentFont, buffer, lineWidth, fontSize, tajweedColor);
      }

      hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
      hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
      QVector<quint32> spaces;
      int currentlineWidth = 0;
      int spaceWidth = 0;

      LineLayoutInfo lineLayout;

      for (int i = glyph_count - 1; i >= 0; i--) {

        GlyphLayoutInfo glyphLayout;

        glyphLayout.codepoint = glyph_info[i].codepoint;
        glyphLayout.lefttatweel = normalToParameter(glyph_info[i].codepoint, glyph_info[i].lefttatweel, true); //glyph_info[i].lefttatweel;
        glyphLayout.righttatweel = normalToParameter(glyph_info[i].codepoint, glyph_info[i].righttatweel, false); // glyph_info[i].righttatweel;
        glyphLayout.cluster = glyph_info[i].cluster;
        glyphLayout.x_advance = glyph_pos[i].x_advance;
        glyphLayout.y_advance = glyph_pos[i].y_advance;
        glyphLayout.x_offset = glyph_pos[i].x_offset;
        glyphLayout.y_offset = glyph_pos[i].y_offset;
        glyphLayout.lookup_index = glyph_pos[i].lookup_index;
        glyphLayout.color = glyph_pos[i].lookup_index >= this->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
        glyphLayout.subtable_index = glyph_pos[i].subtable_index;
        glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;

        glyphLayout.beginsajda = false;
        glyphLayout.endsajda = false;

        currentlineWidth += glyphLayout.x_advance;

        if (glyphLayout.codepoint == 32) {
          spaces.append(lineLayout.glyphs.size());
          spaceWidth += glyphLayout.x_advance;

        }

        lineLayout.glyphs.push_back(glyphLayout);

      }

      lineLayout.overfull = lineWidth != 0 ? currentlineWidth - lineWidth : 0;

      const int minSpace = OtLayout::MINSPACEWIDTH * fontSize;


      if (lineWidth != 0 && spaces.size() != 0 && applyJustification) {
        if (lineLayout.overfull < 0) {

          double spaceAdded = -lineLayout.overfull / spaces.size();
          for (auto index : spaces) {
            lineLayout.glyphs[index].x_advance += spaceAdded;
            //lineLayout.overfull += spaceAdded;
          }
          lineLayout.overfull = 0;
          currentlineWidth = lineWidth;
        }
        else if (lineLayout.overfull > 0) {
          /*double spaceRemoved = lineLayout.overfull / spaces.size();
          for (auto index : spaces) {
            auto newSpace = lineLayout.glyphs[index].x_advance - spaceRemoved;
            if (newSpace > minSpace) {
             lineLayout.glyphs[index].x_advance = newSpace;
              lineLayout.overfull -= spaceRemoved;
            }
          }*/
        }
      }

      if (justification == LineJustification::Distribute) {
        lineLayout.xstartposition = 0;
      }
      else {
        lineLayout.xstartposition = (pageWidth - currentlineWidth) / 2;
      }

      lineLayout.ystartposition = currentyPos;
      lineLayout.fontSize = fontSize;

      if (overfull) {
        hb_font_destroy(currentFont);
        overfull = false;
      }
      else if (changeSize) {
        if (lineLayout.overfull > 0) {
          double ratio = (double)lineWidth / currentlineWidth;
          if (ratio > 0.01) {
            fontSize = emScale * ratio;
            currentFont = this->createFont(fontSize, false);
            overfull = true;
            continue;
          }
        }
      }

      currentyPos = currentyPos + (InterLineSpacing << OtLayout::SCALEBY);

      page.append(lineLayout);
    }
  }

  hb_font_destroy(shapefont);
  hb_buffer_destroy(buffer);

  return page;

}

QList<QStringList> OtLayout::pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, QString text, int nbPages) {

  QSet<int> forcedBreaks;

  QString suraWord = "سُورَةُ";
  QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  QString surapattern = "^("
    + suraWord + " .*|"
    + bism
    + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + ")$";

  QList<QString> suraNames;

  QRegularExpression suraRe(surapattern, QRegularExpression::MultilineOption);
  QRegularExpressionMatchIterator i = suraRe.globalMatch(text);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    int startOffset = match.capturedStart(); // startOffset == 6
    int endOffset = match.capturedEnd(); // endOffset == 9
    forcedBreaks.insert(endOffset);
    forcedBreaks.insert(startOffset - 1);
  }

  text = text.replace(char(10), char(32));
  text = text.replace(bism + char(32), bism + char(10));


  return pageBreak(emScale, lineWidth, pageFinishbyaVerse, text, forcedBreaks, nbPages);

}
QList<QStringList> OtLayout::pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, QString text, QSet<int> forcedBreaks, int nbPages) {

  typedef long ParaWidth;

  struct Candidate {
    size_t index;  // index int the text buffer
    int prev = -1;  // index to previous break
    int totalWidth = 0;  // width of text until this point, if we decide to break here
    double  totalDemerits = 0.0;  // best demerits found for this break (index) and lineNumber
    size_t lineNumber = 0;  // only updated for non-constant line widths
    size_t pageNumber = 1;
    size_t totalSpaces = 0;  // preceding space count after breaking
  };


  constexpr double DEMERITS_INFTY = std::numeric_limits<double>::max();

  hb_buffer_t* buffer = buffer = hb_buffer_create();

  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  buffer->useCallback = !useNormAxisValues;


  hb_font_t* font = this->createFont(emScale);

  hb_buffer_add_utf16(buffer, text.utf16(), text.size(), 0, text.size());

  hb_shape(font, buffer, NULL, 0);


  uint glyph_count;

  const int spaceWidth = 100 * emScale;
  const int maxStretch = 100 * emScale;
  const int maxShrink = 50 * emScale;

  //ParaWidth lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);



  ParaWidth totalWidth = 0;
  int totalSpaces = 0;
  std::vector<int> actives;
  std::vector<Candidate> candidates;

  candidates.push_back({});
  Candidate* initCand = &candidates.back();

  initCand->pageNumber = 1;
  initCand->lineNumber = 0;
  initCand->index = glyph_count;
  initCand->prev = -1;

  actives.push_back(0);

  auto sortActives = [&candidates](const int& a, const int& b) {
    if (candidates[a].pageNumber < candidates[b].pageNumber) {
      return true;
    }
    else if (candidates[a].pageNumber > candidates[b].pageNumber) {

      return false;
    }
    else {
      return candidates[a].lineNumber < candidates[b].lineNumber;
    }

  };

  for (int i = glyph_count - 1; i >= 0; i--) {

    if (glyph_info[i].codepoint != 10 && glyph_info[i].codepoint != 0x20) {
      totalWidth += glyph_pos[i].x_advance;
      continue;
    }

    double penalty = 0;

    //check nextglyph equal aya and set penalty
    if (i != 0 && glyphNamePerCode[glyph_info[i - 1].codepoint].contains("aya")) {
      // avoid break
      penalty = 500;
    }
    //check previous glyph eual aya and set penalty
    else if (i != glyph_count - 1 && glyphNamePerCode[glyph_info[i + 1].codepoint].contains("aya")) {
      // prefer break
      penalty = -1;
    }

    auto forcedBreak = forcedBreaks.contains(glyph_info[i].cluster);

    totalSpaces++;

    QHash<int, Candidate> potcandidates;

    auto activeit = actives.begin();
    while (activeit != actives.end()) {

      auto active = &candidates.at(*activeit);

      // must terminate a page with end of aya
      // A page has always 15 lines
      if (pageFinishbyaVerse) {
        if (active->lineNumber == 14 && !(glyph_info[i + 1].codepoint >= Automedina::AyaNumberCode && glyph_info[i + 1].codepoint <= Automedina::AyaNumberCode + 286)) {
          activeit++;
          continue;
        }
      }

      // calculate adjustment ratio
      double adjRatio = 0.0;
      if (nbPages <= 0) {
        int nbSpaces = totalSpaces - active->totalSpaces - 1;
        ParaWidth width = (totalWidth - active->totalWidth) + nbSpaces * spaceWidth;
        if (width < lineWidth) { // short line
          adjRatio = (lineWidth - width) / (nbSpaces * maxStretch);
        }
        else if (width > lineWidth) { // a long line
          adjRatio = (lineWidth - width) / (nbSpaces * maxShrink);
        }

        if (adjRatio < -1) {
          activeit = actives.erase(activeit);
          continue;
        }
      }
      else {
        int nbSpaces = totalSpaces - active->totalSpaces - 1;
        auto wordsWidth = (totalWidth - active->totalWidth);
        ParaWidth width = wordsWidth + nbSpaces * (spaceWidth);
        //ParaWidth width = wordsWidth + nbSpaces * (200 * emScale);
        double maxLineStretch = 0.05 * wordsWidth + nbSpaces * maxStretch;
        double maxLineShrink = 0.01 * wordsWidth + nbSpaces * maxShrink;
        if (width < lineWidth) { // short line
          adjRatio = (lineWidth - width) / (maxLineStretch);
        }
        else if (width > lineWidth) { // a long line
          adjRatio = (lineWidth - width) / (maxLineShrink);
          if (adjRatio < -1) {
            //adjRatio = lineWidth - width;
          }
        }
        //std::cout << "lineWidth=" << lineWidth << ", maxLineShrink=" << maxLineShrink << ", width=" << width << ", adjRatio=" << adjRatio << std::endl;
        /*
        if (adjRatio < -100) {
          activeit = actives.erase(activeit);
          continue;
        }*/
      }

      std::feclearexcept(FE_ALL_EXCEPT);

      double demerits = 0;
      double badness = 100 * std::pow(std::abs(adjRatio), 3);
      if (penalty >= 0) {
        demerits = (1 + std::pow(badness + penalty, 2));
      }
      else {
        demerits = (1 + std::pow(badness, 2) - std::pow(penalty, 2));
      }

      if ((bool)std::fetestexcept(FE_OVERFLOW) || (bool)std::fetestexcept(FE_UNDERFLOW)) {
        throw "Error";
      }

      double totalDemerits = active->totalDemerits + demerits;

      /*
                qDebug() << "index : " << active->index
                    << ", lineNumber : " << active->lineNumber
                    << ", pageNumber : " << active->pageNumber
                    << ", totalSpaces : " << active->totalSpaces
                    << ", totalWidth : " << active->totalWidth
                    << ",active->totalDemerits : " << active->totalDemerits
                    << ",demerits : " << demerits
                    << ",totalDemerits : " << totalDemerits
                    << ",currentIndex : " << i;*/


      if ((bool)std::fetestexcept(FE_OVERFLOW) || (bool)std::fetestexcept(FE_UNDERFLOW)) {
        throw "Error";
      }

      int lineNumber = active->lineNumber + 1;
      int pageNumber = active->pageNumber;

      if (lineNumber == 16) {
        lineNumber = 1;
        pageNumber = pageNumber + 1;
      }

      int key = lineNumber;

      if (nbPages > 0) {
        key = (pageNumber - 1) * 15 + lineNumber;
      }


      if (!potcandidates.contains(key) || potcandidates[key].totalDemerits > totalDemerits) {


        potcandidates[key] = {};

        Candidate* cand = &potcandidates[key];


        cand->lineNumber = lineNumber;
        cand->pageNumber = pageNumber;

        cand->index = i;
        cand->totalSpaces = totalSpaces;
        cand->totalWidth = totalWidth;
        cand->totalDemerits = totalDemerits;
        cand->prev = *activeit;


      }

      if (nbPages > 0 && nbPages != 1) {
        if (potcandidates.size() > 30 * 15) break;
      }

      activeit++;
    }

    if (forcedBreak) {
      actives.clear();
    }

    for (auto& cand : potcandidates) {
      actives.push_back(candidates.size());
      candidates.push_back(cand);
    }
    std::sort(actives.begin(), actives.end(), sortActives);
  }

  Candidate* bestCandidate = nullptr;

  double best = DEMERITS_INFTY;
  for (auto activenum : actives) {
    auto active = &candidates.at(activenum);
    //qDebug() << "index : " << active->index << ", lineNumber : " << active->lineNumber << "Total demerits : " << active->totalDemerits;
    if (active->index == 0 && active->totalDemerits < best && active->lineNumber == 15 && (!(nbPages > 0) || active->pageNumber == nbPages)) {
      best = active->totalDemerits;
      bestCandidate = active;
    }

  }

  if (bestCandidate == nullptr) {
    return {};
  }


  QStringList originalPage;
  QList<QStringList> originalPages;


  auto cand = bestCandidate;
  int currentpageNumber = bestCandidate->pageNumber;

  //std::cout << std::fixed << "lineWidth=" << lineWidth << ",spaceWidth=" << spaceWidth << ",maxStretch=" << maxStretch << ",maxShrink=" << maxShrink << std::endl;

  while (cand->prev != -1) {

    auto prev = &candidates.at(cand->prev);

    int beginIndex = prev->index - 1;
    int endIndex = cand->index + 1;

    QString originalLine;

    originalLine.append(text.mid(glyph_info[prev->index - 1].cluster, glyph_info[cand->index].cluster - glyph_info[prev->index - 1].cluster));

    /*
    int currentcluster = glyph_info[beginIndex].cluster;
    int currentnewcluster = 0;

    for (int i = beginIndex; i >= endIndex; i--) {

      if (glyph_info[i].cluster != currentcluster) {
        int clusternb = glyph_info[i].cluster - currentcluster;
        originalLine.append(text.mid(currentcluster, clusternb));
        currentcluster = glyph_info[i].cluster;
        currentnewcluster += clusternb;
      }

    }*/

    //originalLine.append(text.mid(currentcluster, glyph_info[endIndex - 1].cluster - currentcluster));

    if (cand->pageNumber == currentpageNumber) {
      originalPage.prepend(originalLine);
    }
    else {
      currentpageNumber--;
      originalPages.prepend(originalPage);
      originalPage.clear();
      originalPage.append(originalLine);

    }

    /*
    int nbSpaces = cand->totalSpaces - prev->totalSpaces - 1;
    auto wordsWidth = (cand->totalWidth - prev->totalWidth);
    ParaWidth width = wordsWidth + nbSpaces * (spaceWidth);

    std::cout << std::fixed << "pageNumber=" << cand->pageNumber << ",lineNumber=" << cand->lineNumber << ",demerits=" << cand->totalDemerits - prev->totalDemerits
      << ",totalDemerits=" << cand->totalDemerits
      << ",nbSpaces=" << nbSpaces
      << ",wordsWidth=" << cand->totalWidth - prev->totalWidth
      << ",currLineWidth=" << width
      << std::endl;*/


    cand = prev;
  }


  originalPages.prepend(originalPage);

  return originalPages;

}

LayoutPages OtLayout::pageBreak(double emScale, int lineWidth, bool pageFinishbyaVerse, int lastPage, hb_buffer_cluster_level_t  cluster_level) {

  bool use20_604_Format = true;

  bool isQurancomplex = false;

  typedef double ParaWidth;

  struct Candidate {
    size_t index;  // index int the text buffer
    int prev;  // index to previous break
    ParaWidth totalWidth;  // width of text until this point, if we decide to break here
    double  totalDemerits;  // best demerits found for this break (index) and lineNumber
    size_t lineNumber;  // only updated for non-constant line widths
    size_t pageNumber;
    size_t totalSpaces;  // preceding space count after breaking
  };


  const double DEMERITS_INFTY = std::numeric_limits<double>::max();

  hb_buffer_t* buffer = buffer = hb_buffer_create();

  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", strlen("ar")));
  hb_buffer_set_cluster_level(buffer, cluster_level);

  hb_font_t* font = this->createFont(emScale);

  QString quran;

  //for (int i = 2; i < lastPage; i++) {
  for (int i = 581; i < 600; i++) {
    //const char * text = qurantext[i];
    const char* tt;

    if (isQurancomplex) {
      tt = quranComplex[i] + 1;
    }
    else {
      tt = qurantext[i] + 1;
    }

    //unsigned int text_len = strlen(tt);

    quran.append(quran.fromUtf8(tt));

    //hb_buffer_add_utf8(buffer, tt, text_len, 0, text_len);

  }




  //quran = quran.replace(QRegularExpression("\\s*" + QString("۞") + "\\s*"), QString("۞") + " ");  

  QSet<int> lineBreaks;
  QSet<int> suraLines;
  QSet<int> bismLines;

  QString suraWord = "سُورَةُ";
  QString bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  QString surapattern = "^("
    + suraWord + " .*|"
    + bism
    + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"
    + ")$";

  QList<QString> suraNames;

  QRegularExpression suraRe(surapattern, QRegularExpression::MultilineOption);
  QRegularExpressionMatchIterator i = suraRe.globalMatch(quran);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    int startOffset = match.capturedStart(); // startOffset == 6
    int endOffset = match.capturedEnd(); // endOffset == 9
    lineBreaks.insert(endOffset);
    lineBreaks.insert(startOffset - 1);

    if (match.captured(0).startsWith("سُ")) {
      suraLines.insert(startOffset);
      suraNames.append(match.captured(0));
    }
    else {
      bismLines.insert(startOffset);
    }
  }

  quran = quran.replace(char(10), char(32));
  quran = quran.replace(bism + char(32), bism + char(10));


  //Mark sajda rules
  QSet<int> beginsajdas;
  QSet<int> endsajdas;



  //QString gg = //"يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا|يَسْجُدُ لَهُۥ|وَخَرَّ رَاكِعٗا|أَلَّا يَسْجُدُوا۟ لِلَّهِ|وَٱسْجُدُوا۟ لِلَّهِ|فَٱسْجُدُوا۟ لِلَّهِ|يَسْجُدُونَ|وَلِلَّهِ يَسْجُدُ|خَرُّوا۟ سُجَّدٗا";
  //QString sajdapatterns = QString("(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدٗا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعٗا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)");
  QString sajdapatterns = "(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)"; // sajdapatterns.replace("\u0657", "\u08F0").replace("\u065E", "\u08F1").replace("\u0656", "\u08F2");
  auto sajdaRe = QRegularExpression(sajdapatterns, QRegularExpression::MultilineOption);
  i = sajdaRe.globalMatch(quran);


  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    int startOffset = match.capturedStart(match.lastCapturedIndex()); // startOffset == 6
    int endOffset = match.capturedEnd(match.lastCapturedIndex()) - 1; // endOffset == 9
    QString c0 = match.captured(0);
    QString captured = match.captured(match.lastCapturedIndex());

    //int tt = match.lastCapturedIndex();


    beginsajdas.insert(startOffset);

    while (this->glyphGlobalClasses[quran[endOffset].unicode()] == OtLayout::MarkGlyph)
      endOffset--;

    endsajdas.insert(endOffset);
  }

  hb_buffer_add_utf16(buffer, quran.utf16(), -1, 0, -1);

  hb_shape(font, buffer, NULL, 0);

  uint glyph_count;

  const int spaceWidth = 100 * emScale;
  const int maxStretch = 100 * emScale;
  const int maxShrink = 50 * emScale;

  //ParaWidth lineWidth = (17000 - (2 * 400)) << OtLayout::SCALEBY;

  hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);



  ParaWidth totalWidth = 0;
  int totalSpaces = 0;
  std::vector<int> actives;
  std::vector<Candidate> candidates;

  candidates.push_back({});
  Candidate* initCand = &candidates.back();

  initCand->pageNumber = 1;
  initCand->lineNumber = 0;
  initCand->index = glyph_count;
  initCand->prev = -1;

  actives.push_back(0);

  for (int i = glyph_count - 1; i >= 0; i--) {

    if (glyph_info[i].codepoint != 10 && glyph_info[i].codepoint != 0x20) {
      totalWidth += glyph_pos[i].x_advance;
      continue;
    }

    totalSpaces++;

    QHash<int, int> potcandidates;

    auto activeit = actives.begin();
    while (activeit != actives.end()) {

      auto active = &candidates.at(*activeit);

      // must terminate a page with end of aya
      // A page has always 15 lines
      if (pageFinishbyaVerse) {
        if (active->lineNumber == 14 && !(glyph_info[i + 1].codepoint >= Automedina::AyaNumberCode && glyph_info[i + 1].codepoint <= Automedina::AyaNumberCode + 286)) {
          activeit++;
          continue;
        }
      }

      // calculate adjustment ratio
      double adjRatio = 0.0;
      if (!use20_604_Format) {
        int spaces = totalSpaces - active->totalSpaces;
        ParaWidth width = (totalWidth - active->totalWidth) + spaces * spaceWidth;
        if (width < lineWidth) { // short line
          adjRatio = (lineWidth - width) / (spaces * maxStretch);
        }
        else if (width > lineWidth) { // a long line
          adjRatio = (lineWidth - width) / (spaces * maxShrink);
        }

        if (adjRatio < -1) {
          activeit = actives.erase(activeit);
          continue;
        }
      }
      else {
        int spaces = totalSpaces - active->totalSpaces;
        ParaWidth width = (totalWidth - active->totalWidth) + spaces * spaceWidth;
        double maxLineStretch = 0.05 * lineWidth;
        double maxLineShrink = 0.02 * lineWidth;
        if (width < lineWidth) { // short line
          adjRatio = (lineWidth - width) / (maxLineStretch);
        }
        else if (width > lineWidth) { // a long line
          adjRatio = (lineWidth - width) / (maxLineShrink);
        }

        if (adjRatio < -100) {
          activeit = actives.erase(activeit);
          continue;
        }
      }


      //if (adjRatio < 160) {
      double demerits = (1 + 100 * std::pow(std::abs(adjRatio), 3));

      double totalDemerits = active->totalDemerits + demerits;

      /*
                qDebug() << "index : " << active->index
                    << ", lineNumber : " << active->lineNumber
                    << ", pageNumber : " << active->pageNumber
                    << ", totalSpaces : " << active->totalSpaces
                    << ", totalWidth : " << active->totalWidth
                    << ",active->totalDemerits : " << active->totalDemerits
                    << ",demerits : " << demerits
                    << ",totalDemerits : " << totalDemerits
                    << ",currentIndex : " << i;*/

      int lineNumber = active->lineNumber + 1;
      int pageNumber = active->pageNumber;

      if (lineNumber == 16) {
        lineNumber = 1;
        pageNumber = pageNumber + 1;
      }

      int key = lineNumber;

      if (use20_604_Format) {
        key = (pageNumber - 1) * 15 + lineNumber;
      }


      if (!potcandidates.contains(key) || candidates.at(potcandidates[key]).totalDemerits > totalDemerits) {


        potcandidates[key] = candidates.size();

        candidates.push_back({});
        Candidate* cand = &candidates.back();


        cand->lineNumber = lineNumber;
        cand->pageNumber = pageNumber;

        cand->index = i;
        cand->totalSpaces = totalSpaces;
        cand->totalWidth = totalWidth;
        cand->totalDemerits = totalDemerits;
        cand->prev = *activeit;


      }

      if (use20_604_Format) {
        if (potcandidates.size() > 30 * 15) break;
      }

      activeit++;
    }

    if (lineBreaks.contains(glyph_info[i].cluster)) {
      actives.clear();
    }

    for (auto cand : potcandidates) {
      actives.push_back(cand);
    }

    std::sort(actives.begin(), actives.end(), [&candidates](const int& a, const int& b) {
      if (candidates[a].pageNumber < candidates[b].pageNumber) {
        return true;
      }
      else if (candidates[a].pageNumber > candidates[b].pageNumber) {

        return false;
      }
      else {
        return candidates[a].lineNumber < candidates[b].lineNumber;
      }

    });

  }

  Candidate* bestCandidate = nullptr;

  double best = DEMERITS_INFTY;
  for (auto activenum : actives) {
    auto active = &candidates.at(activenum);
    //qDebug() << "index : " << active->index << ", lineNumber : " << active->lineNumber << "Total demerits : " << active->totalDemerits;
    if (active->index == 0 && active->totalDemerits < best && active->lineNumber == 15 && (!use20_604_Format || active->pageNumber == 18)) {
      best = active->totalDemerits;
      bestCandidate = active;
    }

  }

  if (bestCandidate == nullptr) {
    //QMessageBox msgBox;
    //msgBox.setText("No feasable solution. Try to change the scale.");
    //msgBox.exec();
    return {};
  }

  QList<LineLayoutInfo> currentPage;
  QList<QList<LineLayoutInfo>> pages;
  QStringList originalPage;
  QList<QStringList> originalPages;
  QList<QString> suraNamebyPage;


  auto cand = bestCandidate;
  int currentpageNumber = bestCandidate->pageNumber;

  int nbbeginsajda = 0;
  int nbendsajda = 0;

  int lastLinePos = (OtLayout::TopSpace + OtLayout::InterLineSpacing * 14) << OtLayout::SCALEBY;

  int currentyPos = lastLinePos;

  int suraIndex = suraNames.length();
  QString currentSuraName;
  QString firstSuraInCurrentage;


  while (cand->prev != -1) {

    auto prev = &candidates.at(cand->prev);

    int beginIndex = prev->index - 1;
    int endIndex = cand->index + 1;
    int totalSpaces = cand->totalSpaces - prev->totalSpaces - 1;
    int totalWidth = cand->totalWidth - prev->totalWidth;

    int minSpaceWidth = spaceWidth - maxStretch;

    int spaceaverage = minSpaceWidth;

    if (totalSpaces != 0) {
      spaceaverage = (lineWidth - totalWidth) / totalSpaces;
      if (spaceaverage < minSpaceWidth) {
        spaceaverage = minSpaceWidth;
      }
    }


    LineLayoutInfo lineLayout;

    lineLayout.type = LineType::Line;

    int currentxPos = 0;

    if (cand->pageNumber != currentpageNumber) {
      if (!firstSuraInCurrentage.isEmpty()) {
        suraNamebyPage.prepend(firstSuraInCurrentage);

        if (suraIndex - 1 >= 0) {
          currentSuraName = suraNames[suraIndex - 1];
        }
        else {
          currentSuraName = "سُورَةُ البَقَرَةِ";
        }

      }
      else {
        suraNamebyPage.prepend(currentSuraName);
      }

      firstSuraInCurrentage = "";
    }



    if (suraLines.contains(glyph_info[beginIndex].cluster)) {
      spaceaverage = (int)spaceWidth;
      currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
      lineLayout.type = LineType::Sura;

      firstSuraInCurrentage = suraNames[--suraIndex];
    }
    else if (bismLines.contains(glyph_info[beginIndex].cluster)) {
      spaceaverage = (int)spaceWidth;
      currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
      lineLayout.type = LineType::Bism;
    }

    spaceaverage = spaceaverage;
    currentxPos = currentxPos;

    QString originalLine;

    int currentcluster = glyph_info[beginIndex].cluster;
    int currentnewcluster = 0;

    for (int i = beginIndex; i >= endIndex; i--) {

      GlyphLayoutInfo glyphLayout;


      QString glyphName = this->glyphNamePerCode[glyph_info[i].codepoint];

      if (glyph_info[i].cluster != currentcluster) {
        int clusternb = glyph_info[i].cluster - currentcluster;
        originalLine.append(quran.mid(currentcluster, clusternb));
        currentcluster = glyph_info[i].cluster;
        currentnewcluster += clusternb;
      }

      glyphLayout.codepoint = glyph_info[i].codepoint;
      glyphLayout.cluster = currentnewcluster; // glyph_info[i].cluster;
      glyphLayout.x_advance = glyph_pos[i].x_advance;
      glyphLayout.y_advance = glyph_pos[i].y_advance;
      glyphLayout.x_offset = glyph_pos[i].x_offset;
      glyphLayout.y_offset = glyph_pos[i].y_offset;
      glyphLayout.lookup_index = glyph_pos[i].lookup_index;
      glyphLayout.color = glyph_pos[i].lookup_index >= this->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
      glyphLayout.subtable_index = glyph_pos[i].subtable_index;
      glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;
      //Todo Optimize
      glyphLayout.beginsajda = false;
      glyphLayout.endsajda = false;

      if (beginsajdas.contains(glyph_info[i].cluster)) {
        glyphLayout.beginsajda = true;
        nbbeginsajda++;
        beginsajdas.remove(glyph_info[i].cluster);

      }
      else if (endsajdas.contains(glyph_info[i].cluster)) {
        glyphLayout.endsajda = true;
        nbendsajda++;
        endsajdas.remove(glyph_info[i].cluster);
      }


      if (glyphLayout.codepoint == 32 || glyphLayout.codepoint == 10) {
        glyphLayout.x_advance = spaceaverage;
        glyphLayout.codepoint = 32;
      }

      lineLayout.glyphs.push_back(glyphLayout);

    }

    originalLine.append(quran.mid(currentcluster, glyph_info[endIndex - 1].cluster - currentcluster));


    lineLayout.xstartposition = currentxPos;



    if (cand->pageNumber == currentpageNumber) {
      lineLayout.ystartposition = currentyPos;
      currentPage.prepend(lineLayout);
      originalPage.prepend(originalLine);
    }
    else {
      currentyPos = lastLinePos;
      lineLayout.ystartposition = currentyPos;
      currentpageNumber--;

      pages.prepend(currentPage);
      originalPages.prepend(originalPage);
      currentPage = QList<LineLayoutInfo>();
      originalPage.clear();
      originalPage.append(originalLine);
      currentPage.append(lineLayout);

    }

    currentyPos -= OtLayout::InterLineSpacing << OtLayout::SCALEBY;
    cand = &candidates.at(cand->prev);


  }

  if (nbbeginsajda != 15) {
    qDebug() << "nbbeginsajda problems?";
  }
  if (nbendsajda != 15) {
    qDebug() << "nbendsajda problems?";
  }

  pages.prepend(currentPage);
  originalPages.prepend(originalPage);
  suraNamebyPage.prepend(currentSuraName);

  // First & second pages : Al fatiha &  Al Bakara


  for (int pageNumber = 1; pageNumber >= 0; pageNumber--) {

    QString textt = QString::fromUtf8(qurantext[pageNumber] + 1);

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    int beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;

    int pageWidth = lineWidth;
    int newLineWidth = 0;

    QList<LineLayoutInfo> page;

    for (int lineIndex = 0; lineIndex < lines.length(); lineIndex++) {

      if (lineIndex > 0) {
        double diameter = pageWidth * 1; // 0.9;
        if (pageNumber == 0) {
          diameter = pageWidth * 1; // 0.9;
        }

        int index = lineIndex - 1;
        //index = index % 4;
        // 22.5 = 180 / 8
        double degree = lineIndex * 22.5 * M_PI / 180;
        newLineWidth = diameter * std::sin(degree);
        //std::cout << "lineIndex=" << lineIndex << ", lineWidth=" << lineWidth << std::endl;
      }
      else {
        newLineWidth = 0;
      }

      auto lineResult = this->justifyPage(emScale, newLineWidth, pageWidth, QStringList{ lines[lineIndex] }, LineJustification::Center, false, true)[0];


      if (lineIndex == 0) {
        lineResult.type = LineType::Sura;
        lineResult.ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
      }
      else {
        lineResult.ystartposition = beginsura;
        beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;
      }

      page.append(lineResult);
    }
    pages.prepend(page);
    originalPages.prepend(lines);

    if (pageNumber == 1) {
      suraNamebyPage.prepend(currentSuraName);
    }
    else {
      suraNamebyPage.prepend("سُورَةُ الفَاتِحَةِ");
    }
  }

  // Last pages

  currentSuraName = suraNamebyPage.last();

  for (int pageNumber = lastPage; pageNumber < 604; pageNumber++) {
    QString textt = QString::fromUtf8(qurantext[pageNumber] + 1);

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    auto page = this->justifyPage(emScale, lineWidth, lineWidth, lines, LineJustification::Center, false, true);

    bool containsBeginSura = false;

    for (int lineIndex = 0; lineIndex < lines.size(); lineIndex++) {

      auto match = suraRe.match(lines[lineIndex]);
      if (match.hasMatch()) {
        if (match.captured(0).startsWith("سُ")) {
          page[lineIndex].type = LineType::Sura;
          if (!containsBeginSura) {
            containsBeginSura = true;
            currentSuraName = match.captured(0);
          }
        }
        else {
          page[lineIndex].type = LineType::Bism;
        }
      }
    }

    suraNamebyPage.append(currentSuraName);
    pages.append(page);
    originalPages.append(lines);
  }

  delete font;
  hb_buffer_destroy(buffer);

  //Compare text

  /*
      QFile file("qurantinputtext.txt");
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      QTextStream out(&file);   // we will serialize the data into the file
      out.setCodec("UTF-8");


      QString newquran;

      for (auto page : originalPages) {
          for (auto line : page) {
              if (newquran.isEmpty()) {
                  newquran = line;
              }
              else {
                  newquran = newquran + " " + line;
              }
              //out << line.replace("\u06E5","").replace("\u06E6", "") << "\n";   // serialize a string
              out << line << "\n";   // serialize a string

          }
      }
      newquran = newquran + " ";

      int index = newquran.compare(quran);
      file.close();*/

  return { pages, originalPages, suraNamebyPage };

}
int OtLayout::AlternatelastCode = 0xF0000;
std::unordered_map<GlyphParameters, GlyphVis*>& OtLayout::getSubstEquivGlyphs(int glyphCode) {
  return substEquivGlyphs[glyphCode];
}
GlyphVis* OtLayout::getAlternate(int glyphCode, GlyphParameters parameters, bool generateNewGlyph, bool addToEquivSubst) {

  if (addToEquivSubst) {
    auto find = substEquivGlyphs.find(glyphCode);
    if (find != substEquivGlyphs.end()) {
      auto find2 = find->second.find(parameters);
      if (find2 != find->second.end()) {
        return find2->second;
      }
    }
  }

  auto cachedGlyphs = !generateNewGlyph ? &tempGlyphs[glyphCode] : &addedGlyphs[glyphCode];

  auto tryfind1 = cachedGlyphs->find(parameters);

  if (tryfind1 != cachedGlyphs->end()) {
    if (addToEquivSubst) {
      auto& tt = substEquivGlyphs[glyphCode];
      tt.insert({ parameters, tryfind1->second });
    }
    return tryfind1->second;
  }

  auto glyph = this->getGlyph(glyphCode);

  if (glyph->isAlternate) {
    auto originalGlyph = glyph->originalglyph;
    parameters.lefttatweel += glyph->charlt;
    parameters.righttatweel += glyph->charrt;

    glyph = &glyphs[originalGlyph];

    glyphCode = glyph->charcode;
  }

  auto expnadable = expandableGlyphs.find(glyph->name);

  if (expnadable != expandableGlyphs.end()) {
    if (parameters.lefttatweel < expnadable->second.minLeft) {
      parameters.lefttatweel = expnadable->second.minLeft;
    }
    else if (parameters.lefttatweel > expnadable->second.maxLeft) {
      parameters.lefttatweel = expnadable->second.maxLeft;
    }
    if (parameters.righttatweel < expnadable->second.minRight) {
      parameters.righttatweel = expnadable->second.minRight;
    }
    else if (parameters.righttatweel > expnadable->second.maxRight) {
      parameters.righttatweel = expnadable->second.maxRight;
    }
  }
  else {
    std::cout << glyph->name.toStdString() << " is not expandable" << std::endl;
    return glyph;

    /*
    if (parameters.lefttatweel < -0.5) {
      parameters.lefttatweel = -0.5;
    }
    else if (parameters.lefttatweel > 20) {
      parameters.lefttatweel = 20;
    }
    if (parameters.righttatweel < -0.5) {
      parameters.righttatweel = -0.5;
    }
    else if (parameters.righttatweel > 20) {
      parameters.righttatweel = 20;
    }*/
  }

  cachedGlyphs = !generateNewGlyph ? &tempGlyphs[glyphCode] : &addedGlyphs[glyphCode];

  auto tryfind2 = cachedGlyphs->find(parameters);

  if (tryfind2 != cachedGlyphs->end()) {
    if (addToEquivSubst) {
      auto& tt = substEquivGlyphs[glyphCode];
      tt.insert({ parameters, tryfind2->second });
    }
    return tryfind2->second;
  }

  //generate glyph
  automedina->addchar(glyph->name, AlternatelastCode, parameters.lefttatweel, parameters.righttatweel, parameters.leftextratio, parameters.rightextratio,
    parameters.left_tatweeltension, parameters.right_tatweeltension, "alternatechar", parameters.which_in_baseline);

  mp_run_data* _mp_results = mp_rundata(automedina->mp);


  mp_edge_object* p = _mp_results->edges;

  mp_edge_object* edge = nullptr;
  while (p) {
    if (p->charcode == AlternatelastCode) {
      edge = p;
      break;
    }
    p = p->next;
  }

  if (edge == nullptr) {
    throw "Error";
  }

  GlyphVis* newglyph = nullptr;

  if (!generateNewGlyph) {
    newglyph = new	GlyphVis{ this, edge, true };
    newglyph->expanded = true;
  }
  else {
    //Add glyph to font
    quint16 charcode = glyphNamePerCode.keys().last() + 1;
    QString name = QString("%1.added_%2").arg(glyph->name).arg(charcode);

    GlyphVis& temp = *glyphs.insert(name, GlyphVis(this, edge, true));

    newglyph = &temp;

    newglyph->charcode = charcode;
    newglyph->name = name;
    newglyph->expanded = true;
    newglyph->isAlternate = true;

    glyphNamePerCode[newglyph->charcode] = newglyph->name;
    glyphCodePerName[newglyph->name] = newglyph->charcode;



    if (glyphGlobalClasses.contains(glyphCode)) {
      glyphGlobalClasses[newglyph->charcode] = glyphGlobalClasses[glyphCode];

      /*
      for (auto& pclass : automedina->classes) {
        if (pclass.contains(glyph->name)) {

          pclass.insert(newglyph->name);
        }

      }*/
    }

    QMap<GlyphVis::AnchorKey, GlyphVisAnchor>::iterator i;
    for (i = newglyph->anchors.begin(); i != newglyph->anchors.end(); ++i) {
      auto anchor = i.value();
      auto anchorKey = i.key();
      auto anchorName = anchorKey.name;

      switch (i.value().type)
      {
      case 1:
        automedina->markAnchors[anchorName][newglyph->charcode] = anchor.anchor;
        break;
      case 2:
        automedina->entryAnchors[anchorName][newglyph->charcode] = anchor.anchor;
        break;
      case 3:
        automedina->exitAnchors[anchorName][newglyph->charcode] = anchor.anchor;
        break;
      case 4:
        automedina->entryAnchorsRTL[anchorName][newglyph->charcode] = anchor.anchor;
        break;
      case 5:
        automedina->exitAnchorsRTL[anchorName][newglyph->charcode] = anchor.anchor;
        break;
      default:
        break;
      }
    }
  }

  cachedGlyphs->insert({ parameters, newglyph });

  if (addToEquivSubst) {
    auto& tt = substEquivGlyphs[glyphCode];
    tt.insert({ parameters, newglyph });
  }

  return newglyph;


}
QByteArray OtLayout::getCmap() {

  struct Segemnt {
    uint16_t startCode;
    uint16_t endCode;
    int16_t idDelta;
  };

  QVector<Segemnt> segements;

  Segemnt currentSegment{};

  QMap<uint16_t, uint16_t>::const_iterator i = unicodeToGlyphCode.constBegin();
  while (i != unicodeToGlyphCode.constEnd()) {
    auto unicode = i.key();
    auto glyphId = i.value();
    if (unicode >= 10) {
      if (currentSegment.endCode + 1 == unicode && unicode + currentSegment.idDelta == glyphId) {
        currentSegment.endCode = unicode;
      }
      else {
        if (currentSegment.startCode != 0) {
          segements.append(currentSegment);
        }
        currentSegment = { unicode,unicode,(int16_t)(glyphId - unicode) };
      }
    }

    ++i;
  }

  if (currentSegment.startCode != 0) {
    segements.append(currentSegment);
  }

  segements.append({ 0xFFFF,0xFFFF,1 });

  uint16_t segCount = segements.size();


  QByteArray data;

  int nbEncoding = 2;

  data << (uint16_t)0; // version
  data << (uint16_t)nbEncoding; // numTables
  // encodingRecords[0]
  data << (uint16_t)0; // platformID
  data << (uint16_t)3; // encodingID
  data << (uint32_t)(2 + 2 + (2 + 2 + 4) * nbEncoding); // offset
  // encodingRecords[1]
  data << (uint16_t)3; // platformID
  data << (uint16_t)1; // encodingID
  data << (uint32_t)(2 + 2 + (2 + 2 + 4) * nbEncoding); // offset

  QByteArray subtable;

  subtable << (uint16_t)4; // format

  int lengthPos = subtable.size();

  subtable << (uint16_t)0; // length
  subtable << (uint16_t)0; // language
  subtable << (uint16_t)(segCount * 2); // 2 × segCount

  auto range = exp2(floor(log2(segCount)));
  subtable << (uint16_t)(2 * range); // searchRange
  subtable << (uint16_t)log2(range);//entrySelector
  subtable << (uint16_t)(2 * (segCount - range));//rangeShift

  for (auto& seg : segements) {
    subtable << seg.endCode;
  }

  subtable << (uint16_t)0; // reservedPad

  for (auto& seg : segements) {
    subtable << seg.startCode;
  }
  for (auto& seg : segements) {
    subtable << seg.idDelta;
  }
  //idRangeOffset[segCount]
  for (auto& seg : segements) {
    subtable << (uint16_t)0; //idRangeOffset
  }

  QByteArray length;

  length << (uint16_t)subtable.size();
  subtable.replace(lengthPos, 2, length);

  data.append(subtable);

  return data;
}
QByteArray OtLayout::JTST() {
  return justTable.getOpenTypeTable();
}

QByteArray Just::getOpenTypeTable() {


  QByteArray stretchStepsData;
  QByteArray shrinkStepsData;

  QByteArray stretchStepOffsets;
  QByteArray shrinkStepOffsets;
  QByteArray afterGsubArray;

  afterGsubArray << (uint16_t)this->lastGsubLookups.size();

  for (auto lookup : this->lastGsubLookups) {
    int index = -1;
    if (layout->gsublookupsIndexByName.contains(lookup->name)) {
      index = layout->gsublookupsIndexByName.value(lookup->name, -1);
    }
    if (index != -1) {
      afterGsubArray << (uint16_t)index;
    }
  }


  int stretchStepsCount = 0;
  int shrinkStepsCount = 0;

  for (int i = 0; i < 2; i++) {
    auto& StepsData = i == 0 ? stretchStepsData : shrinkStepsData;
    auto& Steps = i == 0 ? stretchSteps : shrinkSteps;
    auto& stepsCount = i == 0 ? stretchStepsCount : shrinkStepsCount;
    auto& stepOffsets = i == 0 ? stretchStepOffsets : shrinkStepOffsets;
    uint16_t currentOffset = 2 + 2 * Steps.size();

    for (auto& step : Steps) {
      std::vector<int> lookups;
      for (auto lookup : step.lookups) {
        int index = -1;
        if (step.gsub && layout->gsublookupsIndexByName.contains(lookup->name)) {
          index = layout->gsublookupsIndexByName.value(lookup->name, -1);
        }
        if (!step.gsub && layout->gposlookupsIndexByName.contains(lookup->name)) {
          index = layout->gposlookupsIndexByName.value(lookup->name, -1);
        }
        if (index != -1) {
          lookups.push_back(index);
        }
      }
      QByteArray StepData;
      //if (lookups.size() != 0) {
      stepsCount++;
      StepData << (uint32_t)(step.gsub);
      StepData << (uint16_t)lookups.size();
      for (auto i : lookups) {
        StepData << (uint16_t)i;
      }
      stepOffsets << (uint16_t)currentOffset;
      currentOffset += StepData.size();
      StepsData.append(StepData);
      //}
    }
  }

  QByteArray data;

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (uint16_t)10; // stretchSteps Offset
  data << (uint16_t)(10 + 2 + stretchStepOffsets.size() + stretchStepsData.size());
  data << (uint16_t)(10 + 2 + stretchStepOffsets.size() + stretchStepsData.size() + 2 + shrinkStepOffsets.size() + shrinkStepsData.size());
  data << (uint16_t)stretchStepsCount;
  data.append(stretchStepOffsets);
  data.append(stretchStepsData);
  data << (uint16_t)shrinkStepsCount;
  data.append(shrinkStepOffsets);
  data.append(shrinkStepsData);
  data.append(afterGsubArray);

  return data;

}
