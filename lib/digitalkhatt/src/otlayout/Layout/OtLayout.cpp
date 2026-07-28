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

#include "hb-ot-hmtx-table.hh"
#include "hb-ot-layout-gsub-table.hh"
#undef max
#include "Lookup.h"
#include "OtLayout.h"
#include "MPFont.h"

#include "Subtable.h"
#include "to_opentype.h"
#include "hb-ot-cmap-table.hh"
#include "hb-ot-post-table.hh"
#include "hb-ot.h"
#include "GlazeJson.h"
#include <iostream>
#include "FeaParser/driver.h"
#include "FeaParser/feaast.h"
#include "GlyphVis.h"
#include "digitalkhatt/core/ByteBuffer.h"
#include "digitalkhatt/core/Regex16.h"
#include "automedina/automedina.h"

#include <cfenv>
#include <filesystem>
#include <fstream>
#include <limits>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string_view>

#include "metafont.h"


namespace {
digitalkhatt::TextString utf8ToUtf16(std::string_view input) {
  digitalkhatt::TextString result;
  result.reserve(input.size());
  for (std::size_t i = 0; i < input.size();) {
    const auto first = static_cast<unsigned char>(input[i]);
    char32_t codepoint = 0;
    std::size_t length = 0;
    if (first < 0x80) {
      codepoint = first;
      length = 1;
    } else if ((first & 0xe0) == 0xc0) {
      codepoint = first & 0x1f;
      length = 2;
    } else if ((first & 0xf0) == 0xe0) {
      codepoint = first & 0x0f;
      length = 3;
    } else if ((first & 0xf8) == 0xf0) {
      codepoint = first & 0x07;
      length = 4;
    } else {
      codepoint = 0xfffd;
      length = 1;
    }
    if (i + length > input.size()) {
      codepoint = 0xfffd;
      length = 1;
    } else {
      for (std::size_t j = 1; j < length; ++j) {
        const auto continuation = static_cast<unsigned char>(input[i + j]);
        if ((continuation & 0xc0) != 0x80) {
          codepoint = 0xfffd;
          length = j;
          break;
        }
        codepoint = (codepoint << 6) | (continuation & 0x3f);
      }
    }
    if (codepoint <= 0xffff) {
      result.push_back(static_cast<char16_t>(codepoint));
    } else if (codepoint <= 0x10ffff) {
      codepoint -= 0x10000;
      result.push_back(static_cast<char16_t>(0xd800 + (codepoint >> 10)));
      result.push_back(static_cast<char16_t>(0xdc00 + (codepoint & 0x3ff)));
    } else {
      result.push_back(u'\ufffd');
    }
    i += length;
  }
  return result;
}

std::vector<digitalkhatt::TextString> splitLines(digitalkhatt::TextView text) {
  std::vector<digitalkhatt::TextString> lines;
  for (std::size_t start = 0; start <= text.size();) {
    const auto end = text.find(u'\n', start);
    const auto count = end == digitalkhatt::TextView::npos ? text.size() - start : end - start;
    if (count != 0) lines.emplace_back(text.substr(start, count));
    if (end == digitalkhatt::TextView::npos) break;
    start = end + 1;
  }
  return lines;
}
}  // namespace

int OtLayout::SCALEBY = 0;
double OtLayout::EMSCALE = 1;
int OtLayout::MINSPACEWIDTH = 0;
int OtLayout::SPACEWIDTH = 75;
int OtLayout::MAXSPACEWIDTH = 100;

std::pair<int, int> OtLayout::getDeltaSetEntry(DefaultDelta delta, int subregionIndex) {
  return toOpenType->getDeltaSetEntry(delta, subregionIndex);
}

float OtLayout::normalToParameter(unsigned int code, float tatweel, bool left) {
  if (!useNormAxisValues || tatweel == 0.0) return tatweel;

  const auto& name = glyphNamePerCode.at(code);
  if (tatweel < -1) {
    std::cout.precision(17);
    std::cout << "min tatweel " << std::fixed << tatweel << " error for glyph " << name << '\n';
    tatweel = -1;
  } else if (tatweel > 1) {
    std::cout.precision(17);
    std::cout << "max tatweel " << std::fixed << tatweel << " error for glyph " << name << '\n';
    tatweel = 1;
  }

  const auto limitsIt = expandableGlyphs.find(name);
  if (limitsIt == expandableGlyphs.end()) {
    std::cout << "No expandable glyph " << name << '\n';
    return tatweel;
  }

  double min = left ? limitsIt->second.minLeft : limitsIt->second.minRight;
  double max = left ? limitsIt->second.maxLeft : limitsIt->second.maxRight;
  if (toOpenType->isUniformAxis()) {
    min = left ? toOpenType->axisLimits.minLeft : toOpenType->axisLimits.minRight;
    max = left ? toOpenType->axisLimits.maxLeft : toOpenType->axisLimits.maxRight;
  }
  return tatweel < 0 ? -tatweel * min : tatweel * max;
}


#include "hb-ot-name-table.hh"

static hb_blob_t* harfbuzzGetTables(hb_face_t* face, hb_tag_t tag, void* userData) {
  OtLayout* layout = reinterpret_cast<OtLayout*>(userData);

  digitalkhatt::ByteBuffer data;

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
      break;
    case HB_TAG('n', 'a', 'm', 'e'):
      data = layout->toOpenType->name();
      break;
    case HB_TAG('f', 'v', 'a', 'r'):
      data = layout->toOpenType->fvar();
      break;
    case HB_TAG('H', 'V', 'A', 'R'):
      data = layout->toOpenType->HVAR();
      break;
    case HB_TAG('J', 'T', 'S', 'T'):
      data = layout->JTST();
      break;
    case HB_TAG('h', 'm', 't', 'x'):
      data = layout->toOpenType->hmtx();
      break;
    case HB_TAG('h', 'h', 'e', 'a'):
      data = layout->toOpenType->hhea();
      break;
    case HB_TAG('p', 'o', 's', 't'):
      data = layout->toOpenType->post();
      break;
  }

  return hb_blob_create(reinterpret_cast<const char*>(data.data()), data.size(),
                        HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
}

static unsigned int
getNominalGlyphs(hb_font_t* font HB_UNUSED,
                 void* font_data,
                 unsigned int count,
                 const hb_codepoint_t* first_unicode,
                 unsigned int unicode_stride,
                 hb_codepoint_t* first_glyph,
                 unsigned int glyph_stride,
                 void* user_data HB_UNUSED) {
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
                void* user_data) {
  /*
      OtLayout* layoutg = reinterpret_cast<OtLayout*>(font_data);

      auto& table = *layoutg->face->table.name;

      auto test = table.get_name(0);

      unsigned int text_size = 1000;
      char text[1000];

      hb_ot_name_get_utf8(layoutg->face,9, 0, &text_size, text);*/

  // auto tt = QString(text);
  OtLayout* layoutg = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layoutg->face;

  return ot_face->table.cmap->get_nominal_glyph(unicode, glyph);

  *glyph = unicode;

  return true;
}

static hb_position_t floatToHarfBuzzPosition(double value) {
  return static_cast<hb_position_t>(value * (1 << OtLayout::SCALEBY));
}

static hb_position_t getGlyphHorizontalAdvance(hb_font_t* hbFont, void* fontData, hb_codepoint_t glyph, GlyphParameters parameters, void* userData) {
  OtLayout* layout = reinterpret_cast<OtLayout*>(fontData);

  if (!layout->glyphNamePerCode.contains(glyph)) {
    // std::cout << "Glyph " << glyph << " not found" << std::endl;
    return 0;
  }

  if (layout->glyphGlobalClasses[glyph] == OtLayout::MarkGlyph) {
    return 0;
  } else {
    const auto& name = layout->glyphNamePerCode[glyph];

    GlyphVis* pglyph = &layout->glyphs[name];

    if (parameters.lefttatweel != 0 || parameters.righttatweel != 0) {
      pglyph = layout->getAlternate(pglyph->charcode, parameters);
    }

    const double metricWidth =
        layout->quantizeGlyphAdvances ? toInt(pglyph->width) : pglyph->width;
    auto xadvance = hbFont->em_scale_x(metricWidth);

    double advance = metricWidth;
    // return advance; // floatToHarfBuzzPosition(advance);
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

hb_position_t OtLayout::gethHorizontalAdvance(hb_font_t* hbFont, hb_codepoint_t glyph, GlyphParameters parameters, void* userData) {
  return getGlyphHorizontalAdvance(hbFont, this, glyph, parameters, userData);
}

static void
hb_ot_get_glyph_h_advances(hb_font_t* font, void* font_data,
                           unsigned count,
                           const hb_codepoint_t* first_glyph,
                           unsigned glyph_stride,
                           hb_position_t* first_advance,
                           unsigned advance_stride,
                           void* user_data HB_UNUSED) {
  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layout->face;

  // const hb_ot_face_t* ot_face = (const hb_ot_face_t*)font_data;
  // const OT::hmtx_accelerator_t& hmtx = *ot_face->hmtx;

  // const OT::hmtx_accelerator_t& hmtx = ot_face->table.hmtx;

  // return ot_face->table.cmap->

  int coords[2];

  auto glyphs = (hb_glyph_info_t*)(first_glyph);
  auto positions = (hb_glyph_position_t*)(first_advance);
  for (unsigned int i = 0; i < count; i++) {
    // double leftTatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].lefttatweel, true);
    // double righttatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].righttatweel, false);

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
    } else {
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
                                        void* user_data) {
  // TODO:hacking
  auto glyphs = (hb_glyph_info_t*)(first_glyph);
  auto positions = (hb_glyph_position_t*)(first_advance);

  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  for (unsigned int i = 0; i < count; i++) {
    GlyphParameters parameters;

    parameters.lefttatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].lefttatweel, true);
    parameters.righttatweel = layout->normalToParameter(glyphs[i].codepoint, glyphs[i].righttatweel, false);

    /*
    unsigned int num_coords = 0;

    const float* coords = hb_font_get_var_coords_design(font, &num_coords);
    if (num_coords == 1)
    {
      parameters.scalex = coords[0] /  100; // *65536.f;
      //std::cout << "coords[0]=" << coords[0] << ";";
    }*/

    positions[i].x_advance = getGlyphHorizontalAdvance(font, font_data, glyphs[i].codepoint, parameters, user_data);
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
      auto anchor = subtableTable->getEntry(context->glyph_id, {.lefttatweel = lefttatweel, .righttatweel = righttatweel});
      if (anchor) {
        *x = anchor->x();
        *y = anchor->y();

        return true;
      } else {
        *x = 0;
        *y = 0;
      }
    } else if (context->type == hb_cursive_anchor_context_t::exit) {
      auto anchor = subtableTable->getExit(context->glyph_id, {.lefttatweel = lefttatweel, .righttatweel = righttatweel});
      if (anchor) {
        *x = anchor->x();
        *y = anchor->y();

        return true;
      } else {
        *x = 0;
        *y = 0;
      }
    }

  } else if (lookupTable->type == Lookup::mark2base || lookupTable->type == Lookup::mark2mark) {
    MarkBaseSubtable* subtableTable = static_cast<MarkBaseSubtable*>(subtable);

    std::uint16_t classIndex = subtableTable->markCodes[context->glyph_id];

    const std::string& className = subtableTable->classNamebyIndex[classIndex];

    double lefttatweel = layout->normalToParameter(context->base_glyph_id, context->lefttatweel, true);
    double righttatweel = layout->normalToParameter(context->base_glyph_id, context->righttatweel, false);

    if (context->type == hb_cursive_anchor_context_t::base) {
      const auto& baseGlyphName = layout->glyphNamePerCode[context->base_glyph_id];

      GlyphVis& curr = layout->glyphs[baseGlyphName];

      auto anchor = subtableTable->getBaseAnchor(context->glyph_id, context->base_glyph_id, {.lefttatweel = lefttatweel, .righttatweel = righttatweel});
      if (anchor) {
        *x = anchor->x();
        *y = anchor->y();

        return true;
      } else {
        *x = 0;
        *y = 0;
      }

    } else if (context->type == hb_cursive_anchor_context_t::mark) {
      const auto& markGlyphName = layout->glyphNamePerCode[context->glyph_id];

      GlyphVis& curr = layout->glyphs[markGlyphName];

      auto anchor = subtableTable->getMarkAnchor(context->glyph_id, context->base_glyph_id, {.lefttatweel = lefttatweel, .righttatweel = righttatweel});
      if (anchor) {
        *x = anchor->x();
        *y = anchor->y();

        return true;
      } else {
        *x = 0;
        *y = 0;
      }
    }
  } else if (lookupTable->type == Lookup::pairadjustment && context->type == hb_cursive_anchor_context_t::pair) {
    PairAdjustmentSubtable* subtableTable = static_cast<PairAdjustmentSubtable*>(subtable);
    subtableTable->getPairValue(context);
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

    char buf[64];

    hb_font_get_glyph_name(font, curr_info.codepoint, buf, sizeof(buf));

    char prevName[64];
    hb_font_get_glyph_name(font, prev_info.codepoint, prevName, sizeof(prevName));
    if (std::string_view(prevName) == "behshape.medi.expa") {
      curr_info.lefttatweel = (std::min)(prev_info.lefttatweel, 1.5);
    } else if (std::string_view(prevName).find(".expa") != std::string_view::npos) {
      curr_info.lefttatweel = 1.5;

    } else {
      // curr_info.lefttatweel = 0.07 + 0.1 * prev_info.lefttatweel;
    }

    auto& curr_glyph = *layout->getGlyph(curr_info.codepoint);

  } else if (lookupTable->name.starts_with("expa.")) {
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
        layout->justificationContext.Expansions.insert({buffer->idx, expa});
        layout->justificationContext.totalWeight += expa.weight;
      }
    }

    return false;

  } else if (lookupTable->name.contains("test")) {
    auto buffer = context->buffer;

    auto& curr_info = buffer->cur();

    auto name = layout->glyphNamePerCode[curr_info.codepoint];

    // JustificationContext::GlyphsToExtend.append(buffer->idx);

    if (name == "behshape.medi") {
      curr_info.lefttatweel = 3;
      curr_info.righttatweel = 2;
    }

  } else {
    auto buffer = context->buffer;

    auto& curr_info = buffer->cur();

    auto name = layout->glyphNamePerCode[curr_info.codepoint];

    // JustificationContext::GlyphsToExtend.append(buffer->idx);
    // JustificationContext::Substitutes.append(context->substitute);

    auto subtable = lookupTable->subtables.at(context->subtable_index);

    if (lookupTable->type == Lookup::single) {
      SingleSubtable* subtableTable = static_cast<SingleSubtable*>(subtable);
      if (subtableTable->format == 10) {
        SingleSubtableWithExpansion* tatweelSubtable = static_cast<SingleSubtableWithExpansion*>(subtableTable);
        auto expa = tatweelSubtable->expansion.at(curr_info.codepoint);
        // layout->justificationContext.Expansions.insert({ buffer->idx, tatweelSubtable->expansion.value(curr_info.codepoint) });
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
  } else {
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
                     void* user_data HB_UNUSED) {
  OtLayout* layout = reinterpret_cast<OtLayout*>(font_data);

  auto ot_face = layout->face;

  if (ot_face->table.post->get_glyph_name(glyph, name, size)) return true;

  return false;
}
static hb_font_funcs_t* getFontFunctions(hb_font_t* font, bool otVar) {
  static hb_font_funcs_t* harfbuzzCoreTextFontFuncs = 0;

  // auto& ffunctions = hb_font_get_font_funcs(*font);

  if (!harfbuzzCoreTextFontFuncs) {
    harfbuzzCoreTextFontFuncs = hb_font_funcs_create();
    hb_font_funcs_set_nominal_glyph_func(harfbuzzCoreTextFontFuncs, getNominalGlyph, NULL, NULL);
    // hb_font_funcs_set_nominal_glyphs_func(harfbuzzCoreTextFontFuncs, getNominalGlyphs, NULL, NULL);
    // hb_font_funcs_set_glyph_h_advance_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalAdvance, 0, 0);
    if (!otVar) {
      hb_font_funcs_set_glyph_h_advances_func(harfbuzzCoreTextFontFuncs, get_glyph_h_advances_custom, 0, 0);
    } else {
      hb_font_funcs_set_glyph_h_advances_func(harfbuzzCoreTextFontFuncs, hb_ot_get_glyph_h_advances, nullptr, nullptr);
    }

    hb_font_funcs_set_cursive_anchor_func(harfbuzzCoreTextFontFuncs, get_cursive_anchor, 0, 0);
    hb_font_funcs_set_substitution_func(harfbuzzCoreTextFontFuncs, get_substitution, 0, 0);
    hb_font_funcs_set_apply_lookup_func(harfbuzzCoreTextFontFuncs, apply_lookup, 0, 0);

    // hb_font_funcs_set_glyph_h_origin_func(harfbuzzCoreTextFontFuncs, getGlyphHorizontalOrigin, 0, 0);
    // hb_font_funcs_set_glyph_extents_func(harfbuzzCoreTextFontFuncs, getGlyphExtents, 0, 0);

    // hb_font_funcs_set_glyph_name_func(harfbuzzCoreTextFontFuncs, hb_ot_get_glyph_name, nullptr, nullptr);

    hb_font_funcs_make_immutable(harfbuzzCoreTextFontFuncs);
  }
  return harfbuzzCoreTextFontFuncs;
}

Point AnchorCalc::getAdjustment(Automedina& y, MarkBaseSubtable& subtable,
                                GlyphVis* curr, const std::string& className,
                                Point adjust, GlyphParameters parameters,
                                GlyphVis** poriginalglyph) {
  GlyphVis* originalglyph = curr;

  Point adjustoriginal;

  if (curr->expanded) {
    if (curr->name != "alternatechar" && (!curr->originalglyph.empty() && (curr->charlt != 0 || curr->charrt != 0))) {
      adjustoriginal = subtable.classes[className].baseparameters[curr->originalglyph];
    }

    originalglyph = &y.glyphs[curr->originalglyph];
    if (curr->leftAnchor) {
      double xshift = curr->matrix.xpart - originalglyph->matrix.xpart;
      double yshift = curr->matrix.ypart - originalglyph->matrix.ypart;

      adjustoriginal += Point(xshift, yshift);
    } else if (curr->rightAnchor && !(curr->originalglyph.find("fina") != std::string::npos && curr->originalglyph.find("expa") != std::string::npos)) {
    } else {
      originalglyph = curr;
    }
  }

  *poriginalglyph = originalglyph;

  return adjustoriginal;
}

GlyphVis* OtLayout::getGlyph(const std::string& name, GlyphParameters parameters) {
  GlyphVis* pglyph = &this->glyphs[name];

  if (parameters.lefttatweel != 0 || parameters.righttatweel != 0 || parameters.scalex != 0) {
    pglyph = getAlternate(pglyph->charcode, parameters);
  }

  return pglyph;
}

GlyphVis* OtLayout::getGlyph(int code, GlyphParameters parameters) {
  if (glyphNamePerCode.contains(code)) {
    return getAlternate(code, parameters);
  }

  return nullptr;
}

GlyphVis* OtLayout::getGlyph(int code) {
  GlyphVis* curr = nullptr;

  if (glyphNamePerCode.contains(code)) {
    const auto& baseGlyphName = glyphNamePerCode[code];

    curr = &glyphs[baseGlyphName];
  }

  return curr;
}

digitalkhatt::ByteBuffer OtLayout::getGDEF() {
  if (!gdef_array.empty() && !dirty) {
    return gdef_array;
  }

  gdef_array.clear();

  constexpr std::uint16_t gdefHeaderSize = 18;
  const auto markGlyphSetCount = markGlyphSets.size();

  struct ClassRange {
    std::uint16_t first;
    std::uint16_t last;
    std::uint16_t glyphClass;
  };
  std::vector<ClassRange> classRanges;
  for (const auto& [glyphCode, glyphClass] : glyphGlobalClasses) {
    if (!classRanges.empty() &&
        glyphCode == static_cast<std::uint16_t>(classRanges.back().last + 1) &&
        glyphClass == classRanges.back().glyphClass) {
      classRanges.back().last = glyphCode;
    } else {
      classRanges.push_back({glyphCode, glyphCode, glyphClass});
    }
  }
  if (classRanges.size() > std::numeric_limits<std::uint16_t>::max()) {
    throw std::runtime_error("GDEF ClassDef has too many ranges");
  }

  digitalkhatt::ByteBuffer glyphClassDef;
  glyphClassDef.writeU16(2);  // ClassDef format 2
  glyphClassDef.writeU16(static_cast<std::uint16_t>(classRanges.size()));
  for (const auto& range : classRanges) {
    glyphClassDef.writeU16(range.first);
    glyphClassDef.writeU16(range.last);
    glyphClassDef.writeU16(range.glyphClass);
  }

  /*
   * Put the small MarkGlyphSetsDef header before the potentially large
   * ClassDef and coverage payloads. Its coverage offsets are Offset32, while
   * the GDEF header offsets to MarkGlyphSetsDef and ClassDef are Offset16.
   */
  digitalkhatt::ByteBuffer markGlyphSetsHeader;
  digitalkhatt::ByteBuffer coverageTables;
  if (markGlyphSetCount >
      std::numeric_limits<std::uint16_t>::max()) {
    throw std::runtime_error("GDEF has too many mark glyph sets");
  }
  if (markGlyphSetCount != 0) {
    markGlyphSetsHeader.writeU16(1);
    markGlyphSetsHeader.writeU16(
        static_cast<std::uint16_t>(markGlyphSetCount));
    std::uint32_t coverageOffset =
        4 + 4 * markGlyphSetCount + glyphClassDef.size();
    for (auto markGlyphSet : markGlyphSets) {
      std::sort(markGlyphSet.begin(), markGlyphSet.end());
      markGlyphSet.erase(
          std::unique(markGlyphSet.begin(), markGlyphSet.end()),
          markGlyphSet.end());
      if (markGlyphSet.size() >
          std::numeric_limits<std::uint16_t>::max()) {
        throw std::runtime_error("GDEF mark glyph set is too large");
      }
      markGlyphSetsHeader.writeU32(coverageOffset);
      coverageTables.writeU16(1);
      coverageTables.writeU16(
          static_cast<std::uint16_t>(markGlyphSet.size()));
      for (auto glyphCode : markGlyphSet)
        coverageTables.writeU16(glyphCode);
      coverageOffset += 4 + 2 * markGlyphSet.size();
    }
  }

  const std::uint16_t markGlyphSetsDefOffset =
      markGlyphSetCount == 0 ? 0 : gdefHeaderSize;
  const std::uint32_t glyphClassDefOffset32 =
      gdefHeaderSize + markGlyphSetsHeader.size();
  if (glyphClassDefOffset32 >
      std::numeric_limits<std::uint16_t>::max()) {
    throw std::runtime_error("GDEF glyphClassDefOffset exceeds Offset16");
  }
  const auto glyphClassDefOffset =
      static_cast<std::uint16_t>(glyphClassDefOffset32);

  uint32_t itemVarStoreOffset = 0;
  auto itemVariationStore = toOpenType->getGDEFItemVariationStore();
  if (!itemVariationStore.empty())
    itemVarStoreOffset = gdefHeaderSize + markGlyphSetsHeader.size() +
                         glyphClassDef.size() + coverageTables.size();
  digitalkhatt::ByteBuffer gdef;
  gdef.writeU16(1);                       // majorVersion
  gdef.writeU16(3);                       // minorVersion
  gdef.writeU16(glyphClassDefOffset);     // glyphClassDefOffset
  gdef.writeU16(0);                       // attachListOffset
  gdef.writeU16(0);                       // ligCaretListOffset
  gdef.writeU16(0);                       // markAttachClassDefOffset
  gdef.writeU16(markGlyphSetsDefOffset);  // markGlyphSetsDefOffset
  gdef.writeU32(itemVarStoreOffset);      // itemVarStoreOffset
  gdef.append(markGlyphSetsHeader);
  gdef.append(glyphClassDef);
  gdef.append(coverageTables);
  gdef.append(itemVariationStore);
  gdef_array = std::move(gdef);
  return gdef_array;
}
digitalkhatt::ByteBuffer OtLayout::getGSUB() {
  if (!gsub_array.empty() && !dirty) {
    return gsub_array;
  }

  gsub_array = getGSUBorGPOS(true, gsublookups, allGsubFeatures, gsublookupsIndexByName);

  return gsub_array;
}
digitalkhatt::ByteBuffer OtLayout::getGPOS() {
  if (!gpos_array.empty() && !dirty) {
    return gpos_array;
  }

  gpos_array = getGSUBorGPOS(false, gposlookups, allGposFeatures, gposlookupsIndexByName);

  const auto green = gposlookupsIndexByName.find("green");
  tajweedcolorindex = green == gposlookupsIndexByName.end() ? 0xFFFF : green->second;

  return gpos_array;
}
digitalkhatt::ByteBuffer OtLayout::getFeatureList(
    const std::map<std::string, std::set<std::uint16_t>>& allFeatures) {
  std::uint16_t featureCount = allFeatures.size();
  digitalkhatt::ByteBuffer featureList;
  digitalkhatt::ByteBuffer features;
  featureList.writeU16(featureCount);  // featureCount
  uint16_t featureOffset = 2 + 6 * featureCount;
  for (const auto& [featureName, lookupIndexes] : allFeatures) {
    for (int tagIndex = 0; tagIndex < 4; ++tagIndex)
      featureList.writeU8(featureName.at(tagIndex));          // featureTag
    featureList.writeU16(featureOffset);                      // featureOffset
    features.writeU16(0);                                    // featureParams
    features.writeU16(lookupIndexes.size());                  // lookupIndexCount
    for (auto lookupIndex : lookupIndexes) features.writeU16(lookupIndex);
    featureOffset += 4 + 2 * lookupIndexes.size();
  }
  featureList.append(features);
  return featureList;
}
digitalkhatt::ByteBuffer OtLayout::getScriptList(int featureCount) {
  digitalkhatt::ByteBuffer scriptList;
  scriptList.writeU16(1);  // scriptCount
  for (char byte : std::string_view("arab", 4)) scriptList.writeU8(byte);  // scriptTag
  scriptList.writeU16(8);   // scriptOffset
  scriptList.writeU16(10);  // defaultLangSys
  scriptList.writeU16(1);   // langSysCount
  for (char byte : std::string_view("ARA ", 4)) scriptList.writeU8(byte);  // langSysTag
  scriptList.writeU16(10);       // langSysOffset (2 + 2 + 4 + 2)
  scriptList.writeU16(0);        // lookupOrder
  scriptList.writeU16(0xFFFF);   // requiredFeatureIndex
  scriptList.writeU16(featureCount);  // featureIndexCount
  for (uint16_t index = 0; index < featureCount; ++index)
    scriptList.writeU16(index);
  return scriptList;
}

digitalkhatt::ByteBuffer OtLayout::getGSUBorGPOS(bool isgsub, std::vector<Lookup*>& lookups, std::map<std::string, std::set<std::uint16_t>>& allFeatures,
                                   std::map<std::string, int>& lookupsIndexByName) {
  allFeatures.clear();
  lookupsIndexByName.clear();
  lookups.clear();

  for (auto lookup : this->lookups) {
    if (!disabledLookups.contains(lookup->name) && (extended || (lookup->type != Lookup::fsmgsub))) {
      if (isgsub == lookup->isGsubLookup()) {
        std::uint16_t lookupIndex = lookups.size();

        lookupsIndexByName[lookup->name] = lookupIndex;
        lookups.push_back(lookup);
      }
    }
  }

  for (const auto& [featureName, featureLookups] : this->allFeatures) {
    for (auto lookup : featureLookups) {
      const auto found = lookupsIndexByName.find(lookup->name);
      int lookupIndex = found == lookupsIndexByName.end() ? -1 : found->second;
      if (lookupIndex != -1) {
        allFeatures[featureName].insert(lookupIndex);
      }
    }
  }

  auto scriptList = getScriptList(allFeatures.size());
  auto featureList = getFeatureList(allFeatures);


  const std::uint16_t scriptListOffset = 10;
  const std::uint16_t featureListOffset = scriptListOffset + scriptList.size();
  const std::uint16_t lookupListOffset = featureListOffset + featureList.size();
  const std::uint16_t lookupCount = lookups.size();
  std::vector<std::vector<digitalkhatt::ByteBuffer>> serializedLookups;
  serializedLookups.reserve(lookups.size());
  std::uint32_t lookupListtotalSize = 2 + 2 * lookupCount;
  for (auto* lookup : lookups) {
    std::vector<digitalkhatt::ByteBuffer> serializedSubtables;
    for (auto* subtable : lookup->getSubtables(extended)) {
      auto parts = !extended && subtable->isConvertible()
                       ? subtable->getConvertedOpenTypeTables()
                       : subtable->getOpenTypeTables(extended);
      for (const auto& part : parts) {
        if (part.size() > std::numeric_limits<std::uint16_t>::max()) {
          throw std::runtime_error(
              "OpenType subtable exceeds Offset16 after serialization: " +
              lookup->name + "/" + subtable->name);
        }
      }
      serializedSubtables.insert(
          serializedSubtables.end(),
          std::make_move_iterator(parts.begin()),
          std::make_move_iterator(parts.end()));
    }
    const auto subtableCount = serializedSubtables.size();
    if (subtableCount > std::numeric_limits<std::uint16_t>::max()) {
      throw std::runtime_error("Too many physical subtables in lookup " +
                               lookup->name);
    }
    lookupListtotalSize += 6 + 2 * subtableCount;
    if (lookup->markGlyphSetIndex != Lookup::NoMarkGlyphSet) lookupListtotalSize += 2;
    lookupListtotalSize += 8 * subtableCount;
    const std::uint64_t compactLookupSize =
        6 + 2 * subtableCount +
        (lookup->markGlyphSetIndex != Lookup::NoMarkGlyphSet ? 2 : 0) +
        8 * subtableCount;
    if (compactLookupSize > std::numeric_limits<std::uint16_t>::max()) {
      throw std::runtime_error(
          "Extension wrappers exceed Offset16 in lookup " + lookup->name);
    }
    serializedLookups.push_back(std::move(serializedSubtables));
  }
  const std::uint16_t extensiontype =
      isgsub ? Lookup::extensiongsub : Lookup::extensiongpos;

  digitalkhatt::ByteBuffer root;
  root.writeU16(1);                  // majorVersion
  root.writeU16(0);                  // minorVersion
  root.writeU16(scriptListOffset);   // scriptListOffset
  root.writeU16(featureListOffset);  // featureListOffset
  root.writeU16(lookupListOffset);   // lookupListOffset
  root.append(scriptList);
  root.append(featureList);

  digitalkhatt::ByteBuffer lookupList;
  digitalkhatt::ByteBuffer lookupsData;
  digitalkhatt::ByteBuffer subtablesData;
  lookupList.writeU16(lookupCount);  // lookupCount
  std::uint32_t lookupOffset = 2 + 2 * lookupCount;
  uint32_t subtablesDataOffset = lookupListtotalSize;
  for (std::size_t lookupIndex = 0; lookupIndex < lookups.size();
       ++lookupIndex) {
    auto* lookup = lookups[lookupIndex];
    const auto& lookupSubtables = serializedLookups[lookupIndex];
    digitalkhatt::ByteBuffer lookupTable;
    digitalkhatt::ByteBuffer extensions;
    lookupTable.writeU16(extensiontype);          // lookupType
    lookupTable.writeU16(lookup->flags);          // lookupFlag
    lookupTable.writeU16(lookupSubtables.size()); // subTableCount
    uint16_t extensionOffset = 6 + 2 * lookupSubtables.size();
    if (lookup->markGlyphSetIndex != Lookup::NoMarkGlyphSet) extensionOffset += 2;
    for (const auto& subtableBytes : lookupSubtables) {
      lookupTable.writeU16(extensionOffset);
      extensions.writeU16(1);  // extension format
      extensions.writeU16(static_cast<std::uint16_t>(lookup->type));  // extensionLookupType
      extensions.writeU32(subtablesDataOffset -
                             (lookupOffset + extensionOffset));
      subtablesData.append(subtableBytes);
      subtablesDataOffset += subtableBytes.size();
      extensionOffset += 8;
    }
    if (lookup->markGlyphSetIndex != Lookup::NoMarkGlyphSet)
      lookupTable.writeU16(lookup->markGlyphSetIndex);
    lookupTable.append(extensions);
    if (lookupOffset > std::numeric_limits<std::uint16_t>::max()) {
      throw std::runtime_error(
          "LookupList Offset16 overflow before lookup " + lookup->name);
    }
    lookupList.writeU16(static_cast<std::uint16_t>(lookupOffset));
    lookupOffset += lookupTable.size();
    lookupsData.append(lookupTable);
  }
  lookupList.append(lookupsData);
  lookupList.append(subtablesData);
  root.append(lookupList);
  return root;
}
OtLayout::OtLayout(MPFont* font, bool extended, bool generateVariableOpenType)
    : fsmDriver{*this}, justTable{this}, font{font},
      isOTVar{generateVariableOpenType} {

  this->extended = extended;
  face = hb_face_create_for_tables(harfbuzzGetTables, this, 0);

  dirty = true;

  const std::filesystem::path fontPath = font->projectFile();
#ifdef NDEBUG
  constexpr std::string_view debugPostfix = "";
#else
  constexpr std::string_view debugPostfix = "d";
#endif
  auto ff =
      (fontPath.parent_path() /
       (std::string{SLPREFIX} + fontPath.stem().string() +
        std::string{debugPostfix} + SLEXT))
          .string();
#ifdef __EMSCRIPTEN__
  // Load-time linked side modules are registered by the name stored in the
  // main module's dylink.0 section (e.g. "libmadina.wasm"). MEMFS may turn
  // projectFile() into an absolute path, but passing that absolute path to
  // dlopen would miss the already-loaded entry and fetch the module again.
  ff = std::filesystem::path{ff}.filename().string();
#endif
  dlhandle slhandle = dlopen(ff.c_str(), 0);
  if (!slhandle) {
    std::cout << "could not load the dynamic library " << ff << std::endl;
    throw std::runtime_error("could not load the dynamic library");
  } else {
    typedef Automedina* (*f_funci)(OtLayout* layout, MPFont* font, bool extended);
    f_funci funci = (f_funci)dlsym(slhandle, "font_create");
    if (!funci) {
      std::cout << "could not locate the function" << std::endl;
      throw std::runtime_error("could not locate the function");
    }
    automedina = funci(this, font, extended);
    if (!automedina) {
      throw std::runtime_error("font_create failed for " + ff);
    }
  }
  nuqta();

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

mp_graphic_object* OtLayout::copyEdgeBody(mp_graphic_object* source) const {
  return font->copyBody(source);
}

void OtLayout::setDisabled(Lookup* lookup) {
  disabledLookups.insert(lookup->name);
}

void OtLayout::setLookupDisabled(Lookup* lookup, bool disabled) {
  setLookupDisabled(lookup->name, disabled);
}

void OtLayout::setLookupDisabled(std::string lookupName, bool disabled) {
  if (disabled) {
    disabledLookups.insert(std::move(lookupName));
  } else {
    disabledLookups.erase(lookupName);
  }
}

void OtLayout::generateSubstEquivGlyphs() {
  if (!extended && substEquivGlyphs.size() == 0) {
    automedina->generateSubstEquivGlyphs();
  }
}

void OtLayout::generateSubstEquivGlyphsLegacy() {
  for (auto* lookup : lookups) {
    if (disabledLookups.contains(lookup->name) || !lookup->isGsubLookup() ||
        lookup->type == Lookup::SubType::fsmgsub)
      continue;
    for (auto* subtable : lookup->getSubtables(extended))
      subtable->generateSubstEquivGlyphs();
  }
}

void OtLayout::clearAlternates() {
  for (auto& glyph : tempGlyphs) {
    for (auto& path : glyph.second) {
      delete path.second;
    }
    // glyph.second.clear();
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

CalcAnchor OtLayout::getanchorCalcFunctions(const std::string& functionName, Subtable* subtable) {
  return automedina->getanchorCalcFunctions(functionName, subtable);
}
CursiveAnchorFunc OtLayout::getCursiveFunctions(const std::string& functionName, Subtable* subtable) {
  return automedina->getCursiveFunctions(functionName, subtable);
}
PairAdjustFunc OtLayout::getPairAdjustFunction(std::string functionName, Subtable* subtable) {
  return automedina->getPairAdjustFunction(functionName, subtable);
}
void OtLayout::addLookup(Lookup* lookup) {
  if (lookup->type == Lookup::none) {
    throw "Lookup Type not defined";
  }

  if (lookup->name.empty()) {
    throw "Lookup name not defined";
  }

  if (!lookup->feature.empty() && lookup->feature != "inherited") {
    allFeatures[lookup->feature].insert(lookup);
  }

  std::uint16_t lookupIndex = lookups.size();

  lookupsIndexByName[lookup->name] = lookupIndex;
  lookups.push_back(lookup);
}

void OtLayout::loadLookupFile(std::string fileName) {
  std::string absoluteFileName;

  std::filesystem::path p1 = fileName;

  if (p1.is_relative()) {
    std::filesystem::path p2 = font->projectDirectory();
    p2 /= p1;
    absoluteFileName = p2.string();
  } else {
    absoluteFileName = std::move(fileName);
  }

  parseFeatureFile(absoluteFileName);

  const auto parametersFileName =
      font->projectDirectory() /
      "parameters.json";

  std::ifstream parametersStream(parametersFileName, std::ios::binary);

  if (parametersStream) {
    std::string buffer{std::istreambuf_iterator<char>{parametersStream}, {}};
    ParameterJsonObject parameters;
    if (glz::read_json(parameters, buffer)) {
      std::cout << "Problem reading file." << absoluteFileName;
    } else {
      readParameters(parameters);
    }

    parametersStream.close();
  }

  // addGlyphs();
}

void OtLayout::parseFeatureFile(std::string fileName) {
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
  // automedina->cvxxfeatures.clear();
  allFeatures.clear();
  // Do not clear disabledLookups here. GenerateFile reparses the feature file
  // twice, recreating every Lookup object; the name-based disabled state must
  // remain in effect across those reparses.
  tables.clear();
  // nojustalternatePaths.clear();

  feayy::FeaContext context{this};

  feayy::Driver driver(context);
  if (!driver.parse_file(fileName)) {
    std::cout << "Error in parsing " << fileName << std::endl;
  };

  context.populateFeatures();

  if (face != nullptr) {
    hb_face_destroy(face);
    face = nullptr;
  }
}
bool OtLayout::parseCppLookup(const std::string& lookupName) {
  Lookup* newlookup = automedina->getLookup(lookupName);
  if (newlookup) {
    addLookup(newlookup);
    return true;
  }
  return false;
}
void OtLayout::saveParameters(ParameterJsonObject& json) const {
  for (auto lookup : lookups) {
    if (!lookup->isGsubLookup()) {
      ParameterJsonObject lookupObject;
      lookup->saveParameters(lookupObject);
      if (!lookupObject.empty()) {
        json[lookup->name] = std::move(lookupObject);
      }
    }
  }
}
void OtLayout::readParameters(const ParameterJsonObject& json) {
  for (auto lookup : lookups) {
    if (!lookup->isGsubLookup()) {
      const auto found = json.find(lookup->name);
      if (found == json.end() || !found->second.is_object()) continue;
      lookup->readParameters(found->second.get_object());
    }
  }
}
void OtLayout::addClass(std::string name, std::unordered_set<std::string> set) {
  if (automedina->classes.contains(name)) {
    if (name == "haslefttatweel" && automedina->extended == false) {
      return;
    }
    // throw "Class " + name + " Already exists";
  }
  automedina->classes[name] = std::move(set);
}
hb_font_t* OtLayout::createFont(double emScale, bool newFace) {
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
  const int scale = emScale * upem;  // // (1 << OtLayout::SCALEBY) * static_cast<int>(size);
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

std::uint16_t OtLayout::addMarkSet(std::vector<std::uint16_t> list) {
  std::uint16_t index = markGlyphSets.size();

  markGlyphSets.push_back(std::move(list));

  return index;
}
std::uint16_t OtLayout::addMarkSet(const std::vector<std::string>& list) {
  std::vector<std::uint16_t> codeList;
  for (const auto& glyphName : list) {
    if (auto found = glyphCodePerName.find(glyphName); found != glyphCodePerName.end()) {
      std::uint16_t glyphcode = found->second;
      codeList.push_back(glyphcode);
    } else {
      std::cout << "addMarkSet : Glyph Name '" << glyphName << "' does not exist.\n";
    }
  }

  return addMarkSet(codeList);
}
std::unordered_set<std::uint16_t> OtLayout::classtoUnicode(const std::string& className) {
  return automedina->classtoUnicode(className);
}

std::unordered_set<std::uint16_t> OtLayout::getSubsts(int charCode) {
  std::unordered_set<std::uint16_t> set;
  auto addedGlyphs = substEquivGlyphs.find(charCode);
  if (addedGlyphs != substEquivGlyphs.end()) {
    for (auto& addedGlyph : addedGlyphs->second) {
      set.insert(addedGlyph.second->charcode);
    }
  }
  return set;
}

std::unordered_set<std::uint16_t> OtLayout::regexptoUnicode(const std::string& regexp) {
  return automedina->regexptoUnicode(regexp);
}

double OtLayout::nuqta() {
  if (_nuqta == -1) {
    _nuqta = font->numericVariable("nuqta");
  }

  return _nuqta;
}

void OtLayout::applyJustFeature(hb_buffer_t* buffer, bool& needgpos, double& diff, const std::string& feature, hb_font_t* shapefont, double nuqta, double emScale) {
  if (!this->allGsubFeatures.contains(feature))
    return;

  const unsigned int table_index = 0u;
  buffer->reverse();

  unsigned int glyph_count;

  hb_buffer_t* copy_buffer = nullptr;
  copy_buffer = hb_buffer_create();
  hb_buffer_append(copy_buffer, buffer, 0, -1);

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(copy_buffer, &glyph_count);

  OT::hb_ot_apply_context_t c(table_index, shapefont, buffer);
  c.set_recurse_func(OT::SubstLookup::template dispatch_recurse_func<
                     OT::hb_ot_apply_context_t>);
  std::vector<std::uint16_t> list(this->allGsubFeatures[feature].begin(),
                                  this->allGsubFeatures[feature].end());
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
        break;
        ;
      }

      totalWeight = 0;

      std::map<int, GlyphExpansion> affectedIndexes;

      bool insideGroup = false;
      hb_position_t oldWidth = 0;
      hb_position_t newWidth = 0;
      GlyphExpansion groupExpa{};
      groupExpa.weight = 0;
      std::vector<int> group;
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

        group.push_back(i);
        oldWidth += glyph_pos[index].x_advance;
        if (glyph_info[index].codepoint == justificationContext.Substitutes[i]) {
          newWidth += glyph_pos[index].x_advance;
        } else {
          newWidth += getGlyphHorizontalAdvance(shapefont, this, justificationContext.Substitutes[i], {.lefttatweel = glyph_pos[index].lefttatweel, .righttatweel = glyph_pos[index].righttatweel}, nullptr);  // substitute.width* emScale;
        }

        groupExpa.weight += expa.weight;
        groupExpa.MinLeftTatweel += expa.MinLeftTatweel;
        groupExpa.MaxLeftTatweel += expa.MaxLeftTatweel;
        groupExpa.MinRightTatweel += expa.MinRightTatweel;
        groupExpa.MaxRightTatweel += expa.MaxRightTatweel;

        if (expa.startEndLig == StartEndLig::Start) {
          insideGroup = true;
          continue;
        } else if (insideGroup && expa.startEndLig != StartEndLig::End && expa.startEndLig != StartEndLig::EndKashida) {
          continue;
        }

        int widthDiff = newWidth - oldWidth;

        auto tatweel = expaUnit * groupExpa.weight + remainingWidth;

        if (groupExpa.weight == 0) goto next;

        if ((stretch && widthDiff > tatweel) || (!stretch && widthDiff < tatweel)) {
          totalWeight += groupExpa.weight;
          remainingWidth = tatweel;
        } else {
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
                } else {
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
                } else {
                  expa.weight = 0;
                }
              } else {
                expa.weight = 0;
              }
            } else {
              expa.MinLeftTatweel = expa.MinLeftTatweel < 0 ? expa.MinLeftTatweel : 0;
              expa.MinRightTatweel = expa.MinRightTatweel < 0 ? expa.MinRightTatweel : 0;

              auto MinTatweel = expa.MinLeftTatweel + expa.MinRightTatweel;

              if (MinTatweel != 0) {
                auto minShrink = MinTatweel * nuqta;

                if (tatweel < minShrink) {
                  remainingWidth = tatweel - minShrink;
                  tatweel = minShrink;
                } else {
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
                } else {
                  expa.weight = 0;
                }
              } else {
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

void OtLayout::applyJustFeature_old(hb_buffer_t* buffer, bool& needgpos, double& diff, const std::string& feature, hb_font_t* shapefont, double nuqta, double emScale) {
  if (!this->allGsubFeatures.contains(feature))
    return;

  const unsigned int table_index = 0u;
  buffer->reverse();

  unsigned int glyph_count;

  hb_buffer_t* copy_buffer = nullptr;
  copy_buffer = hb_buffer_create();
  hb_buffer_append(copy_buffer, buffer, 0, -1);

  hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(copy_buffer, &glyph_count);

  OT::hb_ot_apply_context_t c(table_index, shapefont, buffer);
  c.set_recurse_func(OT::SubstLookup::template dispatch_recurse_func<
                     OT::hb_ot_apply_context_t>);
  std::vector<std::uint16_t> list(this->allGsubFeatures[feature].begin(),
                                  this->allGsubFeatures[feature].end());
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
        // double tatweel = diff / JustificationContext::GlyphsToExtend.count() / nuqta;

        std::map<int, GlyphExpansion> affectedIndexes;

        bool insideGroup = false;
        hb_position_t oldWidth = 0;
        hb_position_t newWidth = 0;
        std::vector<int> group;

        for (int i = 0; i < justificationContext.GlyphsToExtend.size(); i++) {
          int index = justificationContext.GlyphsToExtend[i];  // glyph_count - 1 - JustificationContext::GlyphsToExtend[i];
          GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[justificationContext.Substitutes[i]]];

          GlyphExpansion& expa = justificationContext.Expansions[index];

          group.push_back(i);
          oldWidth += glyph_pos[index].x_advance;
          if (glyph_info[index].codepoint == justificationContext.Substitutes[i]) {
            newWidth += glyph_pos[index].x_advance;
          } else {
            double leftTatweel = glyph_pos[index].lefttatweel + expa.MinLeftTatweel > 0 ? expa.MinLeftTatweel : 0;
            double rightTatweel = glyph_pos[index].righttatweel + expa.MinRightTatweel > 0 ? expa.MinRightTatweel : 0;
            newWidth += getGlyphHorizontalAdvance(shapefont, this, justificationContext.Substitutes[i], {.lefttatweel = leftTatweel, .righttatweel = rightTatweel}, nullptr);  // substitute.width* emScale;
          }

          if (expa.startEndLig == StartEndLig::Start) {
            insideGroup = true;
            continue;
          } else if (insideGroup && expa.startEndLig != StartEndLig::End && expa.startEndLig != StartEndLig::EndKashida) {
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

                if (stretch && expa.MaxLeftTatweel <= 0 && expa.MaxRightTatweel <= 0 || !stretch && expa.MinLeftTatweel >= 0 && expa.MinRightTatweel >= 0)
                  continue;

                affectedIndexes.insert_or_assign(i, expa);
              } else {
                // GlyphVis& glyph = this->glyphs[this->glyphNamePerCode[glyph_info[index].codepoint]];
                // GlyphVis& substitute = this->glyphs[this->glyphNamePerCode[JustificationContext::Substitutes[i]]];

                // auto minStretch = (substitute.width - glyph.width) * emScale; // +JustificationContext::Expansions[index].MinLeftTatweel * nuqta;

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
                    affectedIndexes.insert_or_assign(i, expa);
                  }
                } else if (!stretch) {
                  if (expa.MinLeftTatweel < 0 || expa.MinRightTatweel < 0) {
                    affectedIndexes.insert_or_assign(i, expa);
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

          std::map<int, GlyphExpansion>::iterator i;
          std::map<int, GlyphExpansion> newaffectedIndexes;
          for (i = affectedIndexes.begin(); i != affectedIndexes.end(); ++i) {
            int index = justificationContext.GlyphsToExtend[i->first];

            auto expa = i->second;

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
                newaffectedIndexes.insert_or_assign(i->first, expa);
              }

            } else {
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
                newaffectedIndexes.insert_or_assign(i->first, expa);
              }
            }
          }
          affectedIndexes = newaffectedIndexes;
        }
      }
    } else {
      continue;
    }
  }
  buffer->reverse();
  if (copy_buffer)
    hb_buffer_destroy(copy_buffer);
}

void OtLayout::jutifyLine_old(hb_font_t* shapefont, hb_buffer_t* text_buffer, int lineWidth, double emScale, bool tajweedColor) {
  const int minSpace = OtLayout::MINSPACEWIDTH * emScale;
  const int defaultSpace = OtLayout::SPACEWIDTH * emScale;
  double nuqta = this->nuqta() * emScale;

  auto copy_buffer_properties = [](hb_buffer_t* dst, hb_buffer_t* src) {
    hb_segment_properties_t props;
    hb_buffer_get_segment_properties(src, &props);
    hb_buffer_set_segment_properties(dst, &props);
    hb_buffer_set_flags(dst, hb_buffer_get_flags(src));
    hb_buffer_set_cluster_level(dst, hb_buffer_get_cluster_level(src));
  };

  auto copyBuffer = [&](hb_buffer_t* des_buffer, hb_buffer_t* source_buffer) {
    hb_buffer_clear_contents(des_buffer);
    copy_buffer_properties(des_buffer, source_buffer);
    hb_buffer_append(des_buffer, source_buffer, 0, -1);
  };

  hb_feature_t color_fea{HB_TAG('t', 'j', 'w', 'd'), 0, 0, (unsigned int)-1};
  if (tajweedColor) {
    color_fea.value = 1;
  }

  hb_feature_t gpos_features[] = {
      {HB_TAG('i', 'n', 'i', 't'), 0, 0, (unsigned int)-1},
      {HB_TAG('m', 'e', 'd', 'i'), 0, 0, (unsigned int)-1},
      {HB_TAG('f', 'i', 'n', 'a'), 0, 0, (unsigned int)-1},
      {HB_TAG('r', 'l', 'i', 'g'), 0, 0, (unsigned int)-1},
      {HB_TAG('l', 'i', 'g', 'a'), 0, 0, (unsigned int)-1},
      {HB_TAG('c', 'a', 'l', 't'), 0, 0, (unsigned int)-1},
      {HB_TAG('s', 'c', 'h', 'm'), 1, 0, (unsigned int)-1},
      {HB_TAG('s', 'h', 'r', '1'), 0, 0, (unsigned int)-1},
      color_fea};

  int num_gpos_features = sizeof(gpos_features) / sizeof(*gpos_features);

  unsigned int glyph_count;

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
      std::vector<std::uint32_t> spaces;
      int currentlineWidth = 0;

      for (int i = glyph_count - 1; i >= 0; i--) {
        if (glyph_info[i].codepoint == 32) {
          glyph_pos[i].x_advance = minSpace;
          spaces.push_back(i);
        } else {
          currentlineWidth += glyph_pos[i].x_advance;
        }
      }

      double diff = (double)lineWidth - currentlineWidth - spaces.size() * (double)defaultSpace;

      bool needgpos = false;
      if (diff > 0) {
        applyJustFeature(buffer, needgpos, diff, "sch1", shapefont, nuqta, emScale);
      }
      // shrink
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
        } else {
          applyJustFeature(buffer, needgpos, diff, "shr2", shapefont, nuqta, emScale);
        }
      }

      if (needgpos) {
        buffer->reverse();
#ifndef HB_NO_JUSTIFICATION
        buffer->justContext = nullptr;
#endif
        gpos_features[7].value = schr1applied ? 1 : 0;
        ;

        hb_shape(shapefont, buffer, gpos_features, num_gpos_features);
        // hb_shape(shapefont, buffer, nullptr, 0);
      }
    }
    JustificationInProgress = false;
  }
  copyBuffer(text_buffer, buffer);

  hb_buffer_destroy(buffer);
}

void OtLayout::jutifyLine(hb_font_t* shapefont, hb_buffer_t* text_buffer, int lineWidth, bool tajweedColor) {
#ifndef HB_NO_JUSTIFICATION
  if (applyJustification && lineWidth != 0) {
    hb_buffer_set_justify(text_buffer, lineWidth);
    // text_buffer->justifyLine = true;
    // text_buffer->lineWidth = lineWidth;
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
  } else {
    features[1].value = 0;
  }
  features[1].start = 0;
  features[1].end = -1;

  JustificationInProgress = applyJustification;  // true;
  hb_shape(shapefont, text_buffer, features, 2);
  /*
  if (tajweedColor) {
    hb_feature_t color_fea{ HB_TAG('t', 'j', 'w', 'd'),0,0,(unsigned int)-1 };
    color_fea.value = 1;

    hb_shape(shapefont, text_buffer, &color_fea, 1);
  }
  else {
    hb_shape(shapefont, text_buffer, nullptr, 0);
  }*/
  JustificationInProgress = false;
}

std::vector<LineLayoutInfo> OtLayout::justifyPage(double emScale, int pageWidth, const std::vector<LineToJustify>& lines, bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level,
                                                  JustOption justOption, std::string mushafLayout) {
  auto justType = justOption.justType;
  auto justStyle = justOption.justStyle;

  if (justType == JustType::Madina || justType == JustType::IndoPak || justType == JustType::Experimental || justType == JustType::Experimental2) {
    return justifyPageUsingFeatures(emScale, pageWidth, lines, newFace, tajweedColor, cluster_level, justOption, mushafLayout);
  }

  std::vector<LineLayoutInfo> page;

  hb_buffer_t* buffer = buffer = hb_buffer_create();
  hb_font_t* shapefont = this->createFont(emScale, newFace);

  int currentyPos = TopSpace << OtLayout::SCALEBY;

  hb_segment_properties_t savedprops;
  savedprops.direction = HB_DIRECTION_RTL;
  savedprops.script = HB_SCRIPT_ARABIC;
  savedprops.language = hb_language_from_string("ar", strlen("ar"));

  savedprops.reserved1 = 0;
  savedprops.reserved2 = 0;

  auto initializeBuffer = [&](hb_buffer_t* buffer, hb_segment_properties_t* savedprops, const LineToJustify& line) {
    hb_buffer_clear_contents(buffer);
    hb_buffer_set_segment_properties(buffer, savedprops);
    hb_buffer_set_cluster_level(buffer, cluster_level);
    auto newLine = line.text;  // QString("\n") + line + QString("\n");
    auto lineLength = static_cast<int>(newLine.size());
    hb_buffer_add_utf16(buffer, reinterpret_cast<const uint16_t*>(newLine.c_str()), lineLength, 0, lineLength);
  };

  hb_font_t* currentFont = nullptr;

  for (auto& line : lines) {
    bool first = true;
    bool overfull = false;
    currentFont = shapefont;
    double fontSize = emScale;
    auto lineWidth = line.width;
    auto justification = line.lineJustification;

    unsigned int glyph_count;

    while (first || overfull) {
      first = false;

      initializeBuffer(buffer, &savedprops, line);
#ifndef HB_NO_JUSTIFICATION
      buffer->useCallback = !useNormAxisValues;
#endif
      if (justType == JustType::HarfBuzz || justType == JustType::None) {
        jutifyLine(currentFont, buffer, lineWidth, tajweedColor);
      } else {
        jutifyLine_old(currentFont, buffer, lineWidth, fontSize, tajweedColor);
      }

      hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
      hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
      std::vector<std::uint32_t> spaces;
      int currentlineWidth = 0;
      int spaceWidth = 0;

      LineLayoutInfo lineLayout;

      for (int i = glyph_count - 1; i >= 0; i--) {
        GlyphLayoutInfo glyphLayout;

        glyphLayout.codepoint = glyph_info[i].codepoint;
        glyphLayout.lefttatweel = normalToParameter(glyph_info[i].codepoint, glyph_info[i].lefttatweel, true);     // glyph_info[i].lefttatweel;
        glyphLayout.righttatweel = normalToParameter(glyph_info[i].codepoint, glyph_info[i].righttatweel, false);  // glyph_info[i].righttatweel;
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
          spaces.push_back(lineLayout.glyphs.size());
          spaceWidth += glyphLayout.x_advance;
        }

        lineLayout.glyphs.push_back(glyphLayout);
      }

      lineLayout.currentLineWidth = currentlineWidth;
      lineLayout.desiredLineWidth = lineWidth;

      lineLayout.overfull = lineWidth != 0 ? currentlineWidth - lineWidth : 0;

      const int minSpace = OtLayout::MINSPACEWIDTH * fontSize;

      if (lineWidth != 0 && spaces.size() != 0 && applyJustification) {
        if (lineLayout.overfull < 0) {
          double spaceAdded = -lineLayout.overfull / spaces.size();
          for (auto index : spaces) {
            lineLayout.glyphs[index].x_advance += spaceAdded;
            // lineLayout.overfull += spaceAdded;
          }
          lineLayout.overfull = 0;
          currentlineWidth = lineWidth;
        } else if (lineLayout.overfull > 0) {
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
      } else {
        lineLayout.xstartposition = (pageWidth - currentlineWidth) / 2;
      }

      lineLayout.ystartposition = currentyPos;
      lineLayout.fontSize = fontSize;

      if (overfull) {
        hb_font_destroy(currentFont);
        overfull = false;
      } else if (justStyle == JustStyle::FontSize) {
        if (lineLayout.overfull > 0) {
          double ratio = (double)lineWidth / currentlineWidth;
          if (ratio > 0.01) {
            fontSize = emScale * ratio;
            currentFont = this->createFont(fontSize, false);
            overfull = true;
            continue;
          }
        }
      } else if (justStyle == JustStyle::XScale) {
        // if (lineLayout.overfull > 0) {
        double ratio = (double)lineWidth / currentlineWidth;
        lineLayout.xscale = ratio;
        //}
      }
      currentyPos = currentyPos + (InterLineSpacing << OtLayout::SCALEBY);

      lineLayout.type = line.lineType;

      page.push_back(lineLayout);
    }
  }

  hb_font_destroy(shapefont);
  hb_buffer_destroy(buffer);

  return page;
}

std::vector<LineLayoutInfo> OtLayout::justifyPage(double emScale, int lineWidth, int pageWidth, std::vector<std::string> lines, LineJustification justification,
                                                  bool newFace, bool tajweedColor, hb_buffer_cluster_level_t cluster_level, JustOption justOption, std::string mushafLayoutType) {
  std::vector<LineToJustify> newLines;

  for (auto& line : lines) {
    newLines.push_back({utf8ToUtf16(line), lineWidth, justification, LineType::Line});
  }
  return justifyPage(emScale, pageWidth, newLines, newFace, tajweedColor, cluster_level, justOption, mushafLayoutType);
}

OriginalPageList OtLayout::pageBreak(double emScale, int lineWidth,
                                      bool pageFinishbyaVerse,
                                      digitalkhatt::TextString text,
                                      int nbPages) {
  std::unordered_set<int> forcedBreaks;
  constexpr std::u16string_view suraWord = u"سُورَةُ";
  constexpr std::u16string_view bism = u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
  constexpr std::u16string_view alternateBism =
      u"بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  std::size_t lineStart = 0;
  while (lineStart <= text.size()) {
    const auto lineEnd = text.find(u'\n', lineStart);
    const auto end = lineEnd == digitalkhatt::TextString::npos
                         ? text.size()
                         : lineEnd;
    const std::u16string_view line{text.data() + lineStart, end - lineStart};
    if (line.starts_with(suraWord) || line == bism || line == alternateBism) {
      forcedBreaks.insert(static_cast<int>(end));
      if (lineStart > 0) forcedBreaks.insert(static_cast<int>(lineStart - 1));
    }
    if (lineEnd == digitalkhatt::TextString::npos) break;
    lineStart = lineEnd + 1;
  }

  std::replace(text.begin(), text.end(), u'\n', u' ');
  const digitalkhatt::TextString bismWithSpace =
      digitalkhatt::TextString{bism} + u' ';
  const digitalkhatt::TextString bismWithBreak =
      digitalkhatt::TextString{bism} + u'\n';
  for (auto pos = text.find(bismWithSpace);
       pos != digitalkhatt::TextString::npos;
       pos = text.find(bismWithSpace, pos + bismWithBreak.size())) {
    text.replace(pos, bismWithSpace.size(), bismWithBreak);
  }

  return pageBreak(emScale, lineWidth, pageFinishbyaVerse, std::move(text),
                   std::move(forcedBreaks), nbPages);
}

OriginalPageList OtLayout::pageBreak(
    double emScale, int lineWidth, bool pageFinishbyaVerse,
    digitalkhatt::TextString text, std::unordered_set<int> forcedBreaks,
    int nbPages) {
  typedef long ParaWidth;

  struct Candidate {
    size_t index;                // index int the text buffer
    int prev = -1;               // index to previous break
    int totalWidth = 0;          // width of text until this point, if we decide to break here
    double totalDemerits = 0.0;  // best demerits found for this break (index) and lineNumber
    size_t lineNumber = 0;       // only updated for non-constant line widths
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

  hb_buffer_add_utf16(buffer,
                      reinterpret_cast<const std::uint16_t*>(text.data()),
                      text.size(), 0, text.size());

  hb_shape(font, buffer, NULL, 0);

  unsigned int glyph_count;

  const int spaceWidth = 100 * emScale;
  const int maxStretch = 100 * emScale;
  const int maxShrink = 50 * emScale;

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
    } else if (candidates[a].pageNumber > candidates[b].pageNumber) {
      return false;
    } else {
      return candidates[a].lineNumber < candidates[b].lineNumber;
    }
  };

  for (int i = glyph_count - 1; i >= 0; i--) {
    if (glyph_info[i].codepoint != 10 && glyph_info[i].codepoint != 0x20) {
      totalWidth += glyph_pos[i].x_advance;
      continue;
    }

    double penalty = 0;

    // check nextglyph equal aya and set penalty
    if (i != 0 && glyphNamePerCode[glyph_info[i - 1].codepoint].find("aya") != std::string::npos) {
      // avoid break
      penalty = 500;
    }
    // check previous glyph eual aya and set penalty
    else if (i != glyph_count - 1 && glyphNamePerCode[glyph_info[i + 1].codepoint].find("aya") != std::string::npos) {
      // prefer break
      penalty = -1;
    }

    const bool forcedBreak = forcedBreaks.contains(glyph_info[i].cluster);

    totalSpaces++;

    std::unordered_map<int, Candidate> potcandidates;

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
        if (width < lineWidth) {  // short line
          adjRatio = (lineWidth - width) / (nbSpaces * maxStretch);
        } else if (width > lineWidth) {  // a long line
          adjRatio = (lineWidth - width) / (nbSpaces * maxShrink);
        }

        if (adjRatio < -1) {
          activeit = actives.erase(activeit);
          continue;
        }
      } else {
        int nbSpaces = totalSpaces - active->totalSpaces - 1;
        auto wordsWidth = (totalWidth - active->totalWidth);
        ParaWidth width = wordsWidth + nbSpaces * (spaceWidth);
        // ParaWidth width = wordsWidth + nbSpaces * (200 * emScale);
        double maxLineStretch = 0.05 * wordsWidth + nbSpaces * maxStretch;
        double maxLineShrink = 0.01 * wordsWidth + nbSpaces * maxShrink;
        if (width < lineWidth) {  // short line
          adjRatio = (lineWidth - width) / (maxLineStretch);
        } else if (width > lineWidth) {  // a long line
          adjRatio = (lineWidth - width) / (maxLineShrink);
          if (adjRatio < -1) {
            // adjRatio = lineWidth - width;
          }
        }
        // std::cout << "lineWidth=" << lineWidth << ", maxLineShrink=" << maxLineShrink << ", width=" << width << ", adjRatio=" << adjRatio << std::endl;
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
      } else {
        demerits = (1 + std::pow(badness, 2) - std::pow(penalty, 2));
      }

#if defined(FE_OVERFLOW) && defined(FE_UNDERFLOW)
      // Not all libc's implement these fenv.h exceptions (e.g. Emscripten's,
      // since wasm has no hardware FP exception-flag support); skip the
      // check where they're unavailable rather than fail to compile.
      if ((bool)std::fetestexcept(FE_OVERFLOW) || (bool)std::fetestexcept(FE_UNDERFLOW)) {
        throw "Error";
      }
#endif

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

#if defined(FE_OVERFLOW) && defined(FE_UNDERFLOW)
      // Not all libc's implement these fenv.h exceptions (e.g. Emscripten's,
      // since wasm has no hardware FP exception-flag support); skip the
      // check where they're unavailable rather than fail to compile.
      if ((bool)std::fetestexcept(FE_OVERFLOW) || (bool)std::fetestexcept(FE_UNDERFLOW)) {
        throw "Error";
      }
#endif

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

    for (auto& [key, cand] : potcandidates) {
      actives.push_back(candidates.size());
      candidates.push_back(cand);
    }
    std::sort(actives.begin(), actives.end(), sortActives);
  }

  Candidate* bestCandidate = nullptr;

  double best = DEMERITS_INFTY;
  for (auto activenum : actives) {
    auto active = &candidates.at(activenum);
    // qDebug() << "index : " << active->index << ", lineNumber : " << active->lineNumber << "Total demerits : " << active->totalDemerits;
    if (active->index == 0 && active->totalDemerits < best && active->lineNumber == 15 && (!(nbPages > 0) || active->pageNumber == nbPages)) {
      best = active->totalDemerits;
      bestCandidate = active;
    }
  }

  if (bestCandidate == nullptr) {
    return {};
  }

  OriginalPage originalPage;
  OriginalPageList originalPages;

  auto cand = bestCandidate;
  int currentpageNumber = bestCandidate->pageNumber;

  // std::cout << std::fixed << "lineWidth=" << lineWidth << ",spaceWidth=" << spaceWidth << ",maxStretch=" << maxStretch << ",maxShrink=" << maxShrink << std::endl;

  while (cand->prev != -1) {
    auto prev = &candidates.at(cand->prev);

    int beginIndex = prev->index - 1;
    int endIndex = cand->index + 1;

    const auto start = glyph_info[prev->index - 1].cluster;
    const auto length = glyph_info[cand->index].cluster - start;
    digitalkhatt::TextString originalLine = text.substr(start, length);

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

    // originalLine.append(text.mid(currentcluster, glyph_info[endIndex - 1].cluster - currentcluster));

    if (cand->pageNumber == currentpageNumber) {
      originalPage.insert(originalPage.begin(), std::move(originalLine));
    } else {
      currentpageNumber--;
      originalPages.insert(originalPages.begin(), std::move(originalPage));
      originalPage.clear();
      originalPage.push_back(std::move(originalLine));
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

  originalPages.insert(originalPages.begin(), std::move(originalPage));

  return originalPages;
}

LayoutPages OtLayout::pageBreak(std::vector<digitalkhatt::TextString> textPages, double emScale, int lineWidth, bool pageFinishbyaVerse, int lastPage, hb_buffer_cluster_level_t cluster_level) {
  bool use20_604_Format = true;

  bool isQurancomplex = false;

  typedef double ParaWidth;

  struct Candidate {
    size_t index;          // index int the text buffer
    int prev;              // index to previous break
    ParaWidth totalWidth;  // width of text until this point, if we decide to break here
    double totalDemerits;  // best demerits found for this break (index) and lineNumber
    size_t lineNumber;     // only updated for non-constant line widths
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

  digitalkhatt::TextString quran;

  for (int i = 2; i < lastPage; i++) {

    quran.append(textPages[i] + u"\n");
  }

  // quran = quran.replace(QRegularExpression("\\s*" + QString("۞") + "\\s*"), QString("۞") + " ");

  std::unordered_set<int> lineBreaks;
  std::unordered_set<int> suraLines;
  std::unordered_set<int> bismLines;

  constexpr digitalkhatt::TextView bism = u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";

  constexpr digitalkhatt::TextView surapattern =
      u"(?m)^(سُورَةُ .*|بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ|بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ)$";

  std::vector<digitalkhatt::TextString> suraNames;

  digitalkhatt::Regex16 suraRe(surapattern);
  for (int offset = 0; offset <= static_cast<int>(quran.size());) {
    auto match = suraRe.match(quran, offset);
    if (!match.hasMatch()) break;
    const int startOffset = match.start();
    const int endOffset = match.end();
    const auto captured = quran.substr(startOffset, endOffset - startOffset);
    lineBreaks.insert(endOffset);
    lineBreaks.insert(startOffset - 1);

    if (captured.starts_with(u"سُ")) {
      suraLines.insert(startOffset);
      suraNames.push_back(captured);
    } else {
      bismLines.insert(startOffset);
    }
    offset = endOffset > offset ? endOffset : offset + 1;
  }

  std::ranges::replace(quran, u'\n', u' ');
  quran = digitalkhatt::replaceAll(quran, digitalkhatt::TextString{bism} + u' ',
                                    digitalkhatt::TextString{bism} + u'\n');

  // Mark sajda rules
  std::unordered_set<int> beginsajdas;
  std::unordered_set<int> endsajdas;

  // QString gg = //"يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا|يَسْجُدُ لَهُۥ|وَخَرَّ رَاكِعٗا|أَلَّا يَسْجُدُوا۟ لِلَّهِ|وَٱسْجُدُوا۟ لِلَّهِ|فَٱسْجُدُوا۟ لِلَّهِ|يَسْجُدُونَ|وَلِلَّهِ يَسْجُدُ|خَرُّوا۟ سُجَّدٗا";
  // QString sajdapatterns = QString("(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدٗا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعٗا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدٗا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)");
  constexpr digitalkhatt::TextView sajdapatterns = u"(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)";
  digitalkhatt::Regex16 sajdaRe(sajdapatterns);
  for (int offset = 0; offset <= static_cast<int>(quran.size());) {
    auto match = sajdaRe.match(quran, offset);
    if (!match.hasMatch()) break;
    const int captureIndex = match.lastCapturedIndex();
    int startOffset = match.start(captureIndex);
    int endOffset = match.end(captureIndex) - 1;

    // int tt = match.lastCapturedIndex();

    beginsajdas.insert(startOffset);

    while (endOffset >= 0) {
      const auto category = hb_unicode_general_category(hb_unicode_funcs_get_default(), quran[endOffset]);
      if (category != HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK &&
          category != HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK &&
          category != HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK) break;
      --endOffset;
    }

    endsajdas.insert(endOffset);
    offset = match.end() > offset ? match.end() : offset + 1;
  }

  hb_buffer_add_utf16(buffer, reinterpret_cast<const std::uint16_t*>(quran.data()),
                      static_cast<int>(quran.size()), 0, static_cast<int>(quran.size()));

  hb_shape(font, buffer, NULL, 0);

  unsigned int glyph_count;

  const int spaceWidth = 100 * emScale;
  const int maxStretch = 100 * emScale;
  const int maxShrink = 50 * emScale;

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

    std::unordered_map<int, int> potcandidates;

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
        if (width < lineWidth) {  // short line
          adjRatio = (lineWidth - width) / (spaces * maxStretch);
        } else if (width > lineWidth) {  // a long line
          adjRatio = (lineWidth - width) / (spaces * maxShrink);
        }

        if (adjRatio < -1) {
          activeit = actives.erase(activeit);
          continue;
        }
      } else {
        int spaces = totalSpaces - active->totalSpaces;
        ParaWidth width = (totalWidth - active->totalWidth) + spaces * spaceWidth;
        double maxLineStretch = 0.05 * lineWidth;
        double maxLineShrink = 0.02 * lineWidth;
        if (width < lineWidth) {  // short line
          adjRatio = (lineWidth - width) / (maxLineStretch);
        } else if (width > lineWidth) {  // a long line
          adjRatio = (lineWidth - width) / (maxLineShrink);
        }

        if (adjRatio < -100) {
          activeit = actives.erase(activeit);
          continue;
        }
      }

      // if (adjRatio < 160) {
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

    for (const auto& [key, cand] : potcandidates) {
      actives.push_back(cand);
    }

    std::sort(actives.begin(), actives.end(), [&candidates](const int& a, const int& b) {
      if (candidates[a].pageNumber < candidates[b].pageNumber) {
        return true;
      } else if (candidates[a].pageNumber > candidates[b].pageNumber) {
        return false;
      } else {
        return candidates[a].lineNumber < candidates[b].lineNumber;
      }
    });
  }

  Candidate* bestCandidate = nullptr;

  double best = DEMERITS_INFTY;
  for (auto activenum : actives) {
    auto active = &candidates.at(activenum);
    // qDebug() << "index : " << active->index << ", lineNumber : " << active->lineNumber << "Total demerits : " << active->totalDemerits;
    if (active->index == 0 && active->totalDemerits < best && active->lineNumber == 15 && (!use20_604_Format || active->pageNumber == 18)) {
      best = active->totalDemerits;
      bestCandidate = active;
    }
  }

  if (bestCandidate == nullptr) {
    // QMessageBox msgBox;
    // msgBox.setText("No feasable solution. Try to change the scale.");
    // msgBox.exec();
    return {};
  }

  LayoutPage currentPage;
  LayoutPageList pages;
  OriginalPage originalPage;
  OriginalPageList originalPages;
  std::vector<digitalkhatt::TextString> suraNamebyPage;

  auto cand = bestCandidate;
  int currentpageNumber = bestCandidate->pageNumber;

  int nbbeginsajda = 0;
  int nbendsajda = 0;

  int lastLinePos = (OtLayout::TopSpace + OtLayout::InterLineSpacing * 14) << OtLayout::SCALEBY;

  int currentyPos = lastLinePos;

  int suraIndex = static_cast<int>(suraNames.size());
  digitalkhatt::TextString currentSuraName;
  digitalkhatt::TextString firstSuraInCurrentage;

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
      if (!firstSuraInCurrentage.empty()) {
        suraNamebyPage.insert(suraNamebyPage.begin(), firstSuraInCurrentage);

        if (suraIndex - 1 >= 0) {
          currentSuraName = suraNames[suraIndex - 1];
        } else {
          currentSuraName = u"سُورَةُ البَقَرَةِ";
        }

      } else {
        suraNamebyPage.insert(suraNamebyPage.begin(), currentSuraName);
      }

      firstSuraInCurrentage.clear();
    }

    if (suraLines.contains(glyph_info[beginIndex].cluster)) {
      spaceaverage = (int)spaceWidth;
      currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
      lineLayout.type = LineType::Sura;

      firstSuraInCurrentage = suraNames[--suraIndex];
    } else if (bismLines.contains(glyph_info[beginIndex].cluster)) {
      spaceaverage = (int)spaceWidth;
      currentxPos = (lineWidth - (totalWidth + totalSpaces * spaceaverage)) / 2;
      lineLayout.type = LineType::Bism;
    }

    spaceaverage = spaceaverage;
    currentxPos = currentxPos;

    digitalkhatt::TextString originalLine;

    int currentcluster = glyph_info[beginIndex].cluster;
    int currentnewcluster = 0;

    for (int i = beginIndex; i >= endIndex; i--) {
      GlyphLayoutInfo glyphLayout;

      const auto& glyphName = this->glyphNamePerCode[glyph_info[i].codepoint];

      if (glyph_info[i].cluster != currentcluster) {
        int clusternb = glyph_info[i].cluster - currentcluster;
        originalLine.append(quran.substr(currentcluster, clusternb));
        currentcluster = glyph_info[i].cluster;
        currentnewcluster += clusternb;
      }

      glyphLayout.codepoint = glyph_info[i].codepoint;
      glyphLayout.cluster = currentnewcluster;  // glyph_info[i].cluster;
      glyphLayout.x_advance = glyph_pos[i].x_advance;
      glyphLayout.y_advance = glyph_pos[i].y_advance;
      glyphLayout.x_offset = glyph_pos[i].x_offset;
      glyphLayout.y_offset = glyph_pos[i].y_offset;
      glyphLayout.lookup_index = glyph_pos[i].lookup_index;
      glyphLayout.color = glyph_pos[i].lookup_index >= this->tajweedcolorindex ? glyph_pos[i].base_codepoint : 0;
      glyphLayout.subtable_index = glyph_pos[i].subtable_index;
      glyphLayout.base_codepoint = glyph_pos[i].base_codepoint;
      // Todo Optimize
      glyphLayout.beginsajda = false;
      glyphLayout.endsajda = false;

      if (beginsajdas.contains(glyph_info[i].cluster)) {
        glyphLayout.beginsajda = true;
        nbbeginsajda++;
        beginsajdas.erase(glyph_info[i].cluster);

      } else if (endsajdas.contains(glyph_info[i].cluster)) {
        glyphLayout.endsajda = true;
        nbendsajda++;
        endsajdas.erase(glyph_info[i].cluster);
      }

      if (glyphLayout.codepoint == 32 || glyphLayout.codepoint == 10) {
        glyphLayout.x_advance = spaceaverage;
        glyphLayout.codepoint = 32;
      }

      lineLayout.glyphs.push_back(glyphLayout);
    }

    originalLine.append(quran.substr(currentcluster, glyph_info[endIndex - 1].cluster - currentcluster));

    lineLayout.xstartposition = currentxPos;

    if (cand->pageNumber == currentpageNumber) {
      lineLayout.ystartposition = currentyPos;
      currentPage.insert(currentPage.begin(), lineLayout);
      originalPage.insert(originalPage.begin(), originalLine);
    } else {
      currentyPos = lastLinePos;
      lineLayout.ystartposition = currentyPos;
      currentpageNumber--;

      pages.insert(pages.begin(), currentPage);
      originalPages.insert(originalPages.begin(), originalPage);
      currentPage = LayoutPage();
      originalPage.clear();
      originalPage.push_back(originalLine);
      currentPage.push_back(lineLayout);
    }

    currentyPos -= OtLayout::InterLineSpacing << OtLayout::SCALEBY;
    cand = &candidates.at(cand->prev);
  }

  if (nbbeginsajda != 15) {
    std::cerr << "nbbeginsajda problems?\n";
  }
  if (nbendsajda != 15) {
    std::cerr << "nbendsajda problems?\n";
  }

  pages.insert(pages.begin(), currentPage);
  originalPages.insert(originalPages.begin(), originalPage);
  suraNamebyPage.insert(suraNamebyPage.begin(), currentSuraName);

  // First & second pages : Al fatiha &  Al Bakara

  for (int pageNumber = 1; pageNumber >= 0; pageNumber--) {
    const auto text = textPages[pageNumber];
    const auto lines = splitLines(text);

    int beginsura = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 3)) << OtLayout::SCALEBY;

    int pageWidth = lineWidth;
    int newLineWidth = 0;

    LayoutPage page;

    for (int lineIndex = 0; lineIndex < static_cast<int>(lines.size()); lineIndex++) {
      if (lineIndex > 0) {
        double diameter = pageWidth * 1;  // 0.9;
        if (pageNumber == 0) {
          diameter = pageWidth * 1;  // 0.9;
        }

        int index = lineIndex - 1;
        // index = index % 4;
        //  22.5 = 180 / 8
        double degree = lineIndex * 22.5 * M_PI / 180;
        newLineWidth = diameter * std::sin(degree);
        // std::cout << "lineIndex=" << lineIndex << ", lineWidth=" << lineWidth << std::endl;
      } else {
        newLineWidth = 0;
      }

      std::vector<LineToJustify> lineToJustify{{lines[lineIndex], newLineWidth,
                                                LineJustification::Center, LineType::Line}};
      auto lineResult = this->justifyPage(emScale, pageWidth, lineToJustify,
                                          false, true, cluster_level, {}, {})[0];

      if (lineIndex == 0) {
        lineResult.type = LineType::Sura;
        lineResult.ystartposition = (OtLayout::TopSpace + (OtLayout::InterLineSpacing * 1)) << OtLayout::SCALEBY;
      } else {
        lineResult.ystartposition = beginsura;
        beginsura += OtLayout::InterLineSpacing << OtLayout::SCALEBY;
      }

      page.push_back(lineResult);
    }
    pages.insert(pages.begin(), page);
    OriginalPage originalLines;
    originalLines.reserve(lines.size());
    for (const auto& line : lines) originalLines.push_back(line);
    originalPages.insert(originalPages.begin(), std::move(originalLines));

    if (pageNumber == 1) {
      suraNamebyPage.insert(suraNamebyPage.begin(), currentSuraName);
    } else {
      suraNamebyPage.insert(suraNamebyPage.begin(), u"سُورَةُ الفَاتِحَةِ");
    }
  }

  // Last pages

  currentSuraName = suraNamebyPage.back();

  for (int pageNumber = lastPage; pageNumber < 604; pageNumber++) {
    const auto text = textPages[pageNumber];
    const auto lines = splitLines(text);
    std::vector<LineToJustify> linesToJustify;
    linesToJustify.reserve(lines.size());
    for (const auto& line : lines) {
      linesToJustify.push_back({line, lineWidth, LineJustification::Center, LineType::Line});
    }
    auto page = this->justifyPage(emScale, lineWidth, linesToJustify, false, true,
                                  cluster_level, {}, {});

    bool containsBeginSura = false;

    for (int lineIndex = 0; lineIndex < lines.size(); lineIndex++) {
      auto match = suraRe.match(lines[lineIndex]);
      if (match.hasMatch()) {
        const auto captured = lines[lineIndex].substr(match.start(), match.end() - match.start());
        if (captured.starts_with(u"سُ")) {
          page[lineIndex].type = LineType::Sura;
          if (!containsBeginSura) {
            containsBeginSura = true;
            currentSuraName = captured;
          }
        } else {
          page[lineIndex].type = LineType::Bism;
        }
      }
    }

    suraNamebyPage.push_back(currentSuraName);
    pages.emplace_back(page.begin(), page.end());
    OriginalPage originalLines;
    originalLines.reserve(lines.size());
    for (const auto& line : lines) originalLines.push_back(line);
    originalPages.push_back(std::move(originalLines));
  }

  delete font;
  hb_buffer_destroy(buffer);

  return {pages, originalPages, suraNamebyPage};
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
      tt.insert({parameters, tryfind1->second});
    }
    return tryfind1->second;
  }

  auto glyph = this->getGlyph(glyphCode);

  if (glyph == nullptr) {
    throw std::runtime_error{"Glyph  not found."};
  }

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
    } else if (parameters.lefttatweel > expnadable->second.maxLeft) {
      parameters.lefttatweel = expnadable->second.maxLeft;
    }
    if (parameters.righttatweel < expnadable->second.minRight) {
      parameters.righttatweel = expnadable->second.minRight;
    } else if (parameters.righttatweel > expnadable->second.maxRight) {
      parameters.righttatweel = expnadable->second.maxRight;
    }
    GlyphParameters nullpar;
    if (nullpar == parameters && !generateNewGlyph) {
      return glyph;
    }
  } else if (parameters.scalex == 0) {
    // std::cout << "No parameter is set for glyph " << glyph->name.toStdString() << std::endl;
    return glyph;
  }

  cachedGlyphs = !generateNewGlyph ? &tempGlyphs[glyphCode] : &addedGlyphs[glyphCode];

  auto tryfind2 = cachedGlyphs->find(parameters);

  if (tryfind2 != cachedGlyphs->end()) {
    if (addToEquivSubst) {
      auto& tt = substEquivGlyphs[glyphCode];
      tt.insert({parameters, tryfind2->second});
    }
    return tryfind2->second;
  }

  auto addedGlyphFind = automedina->addedGlyphs.find(glyph->name);
  if (addedGlyphFind != automedina->addedGlyphs.end()) {
    font->generateAlternate(glyph->name, parameters.lefttatweel,
                            parameters.righttatweel, parameters.third,
                            parameters.fourth, parameters.fifth,
                            parameters.scalex, addedGlyphFind->second,
                            AlternatelastCode);
  } else if (!font->hasGlyph(glyph->name)) {
    // std::cout << glyph->name.toStdString() << " is auto generated. It dows not exist in the original font" <<  std::endl;
    return glyph;
  } else {
    font->generateAlternate(glyph->name, parameters.lefttatweel,
                            parameters.righttatweel, parameters.third,
                            parameters.fourth, parameters.fifth,
                            parameters.scalex, {}, AlternatelastCode);
  }

  mp_edge_object* edge = font->edge(AlternatelastCode);

  if (edge == nullptr) {
    throw "Error";
  }

  GlyphVis* newglyph = nullptr;

  if (!generateNewGlyph) {
    newglyph = new GlyphVis{this, edge};
    newglyph->expanded = true;
  } else {
    // Add glyph to font
    std::uint16_t charcode = glyphNamePerCode.empty() ? 0 : glyphNamePerCode.rbegin()->first + 1;

    const std::string name = std::format("{}.added_{}", glyph->name, charcode);

    GlyphVis& temp = glyphs.insert_or_assign(name, GlyphVis(this, edge)).first->second;

    newglyph = &temp;

    newglyph->charcode = charcode;
    newglyph->name = name;
    newglyph->expanded = true;
    newglyph->isAlternate = true;
    newglyph->originalglyph = glyph->name;

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

    for (const auto& [anchorKey, anchor] : newglyph->anchors) {
      auto anchorName = anchorKey.name;

      switch (anchor.type) {
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

  cachedGlyphs->insert({parameters, newglyph});

  if (addToEquivSubst) {
    auto& tt = substEquivGlyphs[glyphCode];
    tt.insert({parameters, newglyph});
  }

  return newglyph;
}
digitalkhatt::ByteBuffer OtLayout::getCmap() {
  struct Segemnt {
    uint16_t startCode;
    uint16_t endCode;
    int16_t idDelta;
  };

  std::vector<Segemnt> segements;

  Segemnt currentSegment{};

  auto i = unicodeToGlyphCode.cbegin();
  while (i != unicodeToGlyphCode.cend()) {
    auto [unicode, glyphId] = *i;
    if (unicode >= 10) {
      if (currentSegment.endCode + 1 == unicode && unicode + currentSegment.idDelta == glyphId) {
        currentSegment.endCode = unicode;
      } else {
        if (currentSegment.startCode != 0) {
          segements.push_back(currentSegment);
        }
        currentSegment = {unicode, unicode, (int16_t)(glyphId - unicode)};
      }
    }

    ++i;
  }

  if (currentSegment.startCode != 0) {
    segements.push_back(currentSegment);
  }

  segements.push_back({0xFFFF, 0xFFFF, 1});

  uint16_t segCount = segements.size();

  constexpr int nbEncoding = 2;
  digitalkhatt::ByteBuffer data;
  data.writeU16(0);           // version
  data.writeU16(nbEncoding);  // numTables
  // encodingRecords[0]
  data.writeU16(0);                       // platformID
  data.writeU16(3);                       // encodingID
  data.writeU32(4 + 8 * nbEncoding);      // subtable offset
  // encodingRecords[1]
  data.writeU16(3);                       // platformID
  data.writeU16(1);                       // encodingID
  data.writeU32(4 + 8 * nbEncoding);      // subtable offset
  digitalkhatt::ByteBuffer subtable;
  subtable.writeU16(4);  // format
  const auto lengthPosition = subtable.size();
  subtable.writeU16(0);             // length, patched below
  subtable.writeU16(0);             // language
  subtable.writeU16(segCount * 2);  // segCountX2
  const auto searchRange = exp2(floor(log2(segCount)));
  subtable.writeU16(2 * searchRange);               // searchRange
  subtable.writeU16(log2(searchRange));             // entrySelector
  subtable.writeU16(2 * (segCount - searchRange));  // rangeShift
  for (const auto& segment : segements) subtable.writeU16(segment.endCode);
  subtable.writeU16(0);  // reservedPad
  for (const auto& segment : segements) subtable.writeU16(segment.startCode);
  for (const auto& segment : segements) subtable.writeI16(segment.idDelta);
  // idRangeOffset[segCount]
  for ([[maybe_unused]] const auto& segment : segements)
    subtable.writeU16(0);  // idRangeOffset
  subtable.patchU16(lengthPosition, subtable.size());
  data.append(subtable);

  return data;
}
digitalkhatt::ByteBuffer OtLayout::JTST() {
  return justTable.getOpenTypeTable();
}

digitalkhatt::ByteBuffer Just::getOpenTypeTable() {
  digitalkhatt::ByteBuffer afterGsub;
  afterGsub.writeU16(lastGsubLookups.size());  // lookupCount
  for (auto* lookup : lastGsubLookups) {
    const auto found = layout->gsublookupsIndexByName.find(lookup->name);
    if (found != layout->gsublookupsIndexByName.end()) {
      const auto index = found->second;
      if (index != -1) afterGsub.writeU16(index);
    }
  }
  auto buildSteps = [&](const auto& steps) {
    digitalkhatt::ByteBuffer offsets;
    digitalkhatt::ByteBuffer stepData;
    uint16_t currentOffset = 2 + 2 * steps.size();
    for (const auto& step : steps) {
      std::vector<int> lookupIndexes;
      for (auto* lookup : step.lookups) {
        const auto& indexes = step.gsub ? layout->gsublookupsIndexByName
                                        : layout->gposlookupsIndexByName;
        const auto found = indexes.find(lookup->name);
        if (found != indexes.end()) {
          const auto index = found->second;
          if (index != -1) lookupIndexes.push_back(index);
        }
      }
      offsets.writeU16(currentOffset);             // stepOffset
      stepData.writeU32(step.gsub);                // isGsub
      stepData.writeU16(lookupIndexes.size());     // lookupCount
      for (auto index : lookupIndexes) stepData.writeU16(index);
      currentOffset += 6 + 2 * lookupIndexes.size();
    }
    offsets.append(stepData);
    return offsets;
  };
  auto stretchStepsTable = buildSteps(stretchSteps);
  auto shrinkStepsTable = buildSteps(shrinkSteps);
  digitalkhatt::ByteBuffer data;
  data.writeU16(1);   // majorVersion
  data.writeU16(0);   // minorVersion
  data.writeU16(10);  // stretchStepsOffset
  data.writeU16(12 + stretchStepsTable.size());  // shrinkStepsOffset
  data.writeU16(14 + stretchStepsTable.size() + shrinkStepsTable.size());  // afterGsubOffset
  data.writeU16(stretchSteps.size());  // stretchStepsCount
  data.append(stretchStepsTable);
  data.writeU16(shrinkSteps.size());  // shrinkStepsCount
  data.append(shrinkStepsTable);
  data.append(afterGsub);

  return data;
}

const digitalkhatt::layout::ClassMap& OtLayout::glyphClasses() const {
  return automedina->classes;
}

std::unordered_set<std::uint16_t> OtLayout::classToUnicode(const std::string& className) {
  return automedina->classtoUnicode(className);
}

std::map<std::uint16_t, std::vector<ExtendedGlyph>>& OtLayout::resetCvxxFeatures() {
  automedina->cvxxfeatures.clear();
  return automedina->cvxxfeatures.emplace_back();
}
