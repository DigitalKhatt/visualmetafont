#include "GenerateLayout.h"
#include <QDir>
#include <QSaveFile>
#include <stdexcept>
#include <bit>
#include <cmath>
#include <limits>
#include <map>
#include <vector>
#include <qhash.h>
#include "Layout/GlyphVis.h"
#include "font.hpp"
#include "Layout/QByteArrayOperator.h"
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
#include <fstream>

#include "qfileinfo.h"

extern "C"
{
#include "mplibps.h"
}
static void filltoHTML5Path(mp_gr_knot h, QByteArray& pathData)
{

  if (!h) return;
  mp_gr_knot p, q;

  

  pathData << (int32_t)getFixed(h->x_coord);
  pathData << (int32_t)getFixed(h->y_coord);

  QByteArray data;

  int nbCubic = 0;

  p = h;
  do {
    q = p->next;

    nbCubic++;

    data << (int32_t)getFixed(p->right_x);
    data << (int32_t)getFixed(p->right_y);
    data << (int32_t)getFixed(q->left_x);
    data << (int32_t)getFixed(q->left_y);
    data << (int32_t)getFixed(q->x_coord);
    data << (int32_t)getFixed(q->y_coord);

    p = q;
  } while (p != h);

  if(nbCubic > 255){
    throw "Error";
  }

  pathData << (uint8_t)nbCubic;
  pathData.append(data);


}
static void edgetoHTML5Path(mp_graphic_object* body, QByteArray& pathsArray) {

  int nbPaths = 0;

  QByteArray data;

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {

        nbPaths++;

        QJsonObject pathObject;

        auto fillobject = (mp_fill_object*)body;        

        filltoHTML5Path(fillobject->path_p, data);       

        break;
      }
      default:
        break;
      }

    } while (body = body->next);
  }

  if(nbPaths > 255){
    throw "ERROR";
  }

  pathsArray << (uint8_t)nbPaths;
  pathsArray.append(data);
}


static void generateBinaryGlyphs(OtLayout& layout, QByteArray& data) {
  data << (uint16_t)layout.glyphs.size();    
  
  for (auto& [name, glyph] : layout.glyphs) {

    

    QByteArray glyphData;    

    glyphData << (uint32_t)glyph.charcode;

    QByteArray defaultArray;

    edgetoHTML5Path(glyph.copiedPath, defaultArray);

    glyphData.append(defaultArray);    

    const auto& ff = layout.expandableGlyphs.find(glyph.name);

    if (ff != layout.expandableGlyphs.end()) {            

      auto& jj = ff->second;

      glyphData << (uint32_t)getFixed(jj.minLeft);
      glyphData << (uint32_t)getFixed(jj.maxLeft);
      glyphData << (uint32_t)getFixed(jj.minRight);
      glyphData << (uint32_t)getFixed(jj.maxRight);
      

      if (jj.minLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.minLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        QByteArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphData.append(pathArray);
        
      }

      if (jj.maxLeft != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = jj.maxLeft;
        parameters.righttatweel = 0.0;

        auto alternate = glyph.getAlternate(parameters);

        QByteArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphData.append(pathArray);
      }

      if (jj.minRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.minRight;

        auto alternate = glyph.getAlternate(parameters);

        QByteArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphData.append(pathArray);
      }

      if (jj.maxRight != 0) {
        GlyphParameters parameters{};

        parameters.lefttatweel = 0.0;
        parameters.righttatweel = jj.maxRight;

        auto alternate = glyph.getAlternate(parameters);

        QByteArray pathArray;

        edgetoHTML5Path(alternate->copiedPath, pathArray);
        glyphData.append(pathArray);
      }


    }else{
      glyphData << (uint32_t)0;
      glyphData << (uint32_t)0;
      glyphData << (uint32_t)0;
      glyphData << (uint32_t)0;
    }

    const bool hasKafBodyAxis = GenerateLayoutSupport::hasKafBodyAxis(glyph.name);
    const double minThird = hasKafBodyAxis ? GenerateLayoutSupport::KafBodyMin : 0.0;
    const double maxThird = hasKafBodyAxis ? GenerateLayoutSupport::KafBodyMax : 0.0;
    glyphData << (uint32_t)getFixed(minThird);
    glyphData << (uint32_t)getFixed(maxThird);

    if (minThird != 0.0) {
      GlyphParameters parameters{};
      parameters.third = minThird;
      auto alternate = glyph.getAlternate(parameters);

      QByteArray pathArray;
      edgetoHTML5Path(alternate->copiedPath, pathArray);
      glyphData.append(pathArray);
    }

    if (maxThird != 0.0) {
      GlyphParameters parameters{};
      parameters.third = maxThird;
      auto alternate = glyph.getAlternate(parameters);

      QByteArray pathArray;
      edgetoHTML5Path(alternate->copiedPath, pathArray);
      glyphData.append(pathArray);
    }

    data.append(glyphData);    
  }
}
// Each v4 entry represents one actual GlyphVis instance. Retain all parameter
// values, including additional axes, without the v3 fixed-point quantization.
using WholeGlyphKey = std::pair<uint16_t, std::vector<double>>;
struct WholeGlyphVariant {
  const GlyphLayoutInfo* placement;
  uint16_t id = 0;
};
using WholeGlyphVariants = std::map<WholeGlyphKey, WholeGlyphVariant>;

static WholeGlyphKey wholeGlyphKey(const GlyphLayoutInfo& glyph) {
  if (glyph.codepoint < 0 || glyph.codepoint > 65535) {
    throw std::runtime_error("Glyph code cannot fit the layout binary");
  }
  WholeGlyphKey key{static_cast<uint16_t>(glyph.codepoint), {}};
  key.second.reserve(glyph.parameters.size());
  for (digitalkhatt::GlyphAxisId axis = 0; axis < glyph.parameters.size(); ++axis) {
    const auto value = glyph.parameters.value(axis);
    if (!std::isfinite(value)) throw std::runtime_error("Non-finite glyph parameter");
    key.second.push_back(value);
  }
  return key;
}

static void appendFloat(QByteArray& data, double coordinate) {
  static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);
  const auto value = static_cast<float>(coordinate);
  if (!std::isfinite(value)) throw std::runtime_error("Non-finite glyph outline coordinate");
  data << std::bit_cast<uint32_t>(value);
}

static void generateWholeGlyphOutline(const GlyphVis& glyph, QByteArray& data) {
  QByteArray contours;
  unsigned count = 0;
  for (auto* object = glyph.mpPath(); object; object = object->next) {
    if (object->type != mp_fill_code) continue;
    auto* start = reinterpret_cast<mp_fill_object*>(object)->path_p;
    if (!start) continue;
    if (++count > 255) throw std::runtime_error("Too many glyph contours");
    appendFloat(contours, start->x_coord);
    appendFloat(contours, start->y_coord);
    QByteArray cubics;
    unsigned cubicCount = 0;
    auto* p = start;
    do {
      if (++cubicCount > 255) throw std::runtime_error("Too many glyph cubics");
      auto* q = p->next;
      appendFloat(cubics, p->right_x);
      appendFloat(cubics, p->right_y);
      appendFloat(cubics, q->left_x);
      appendFloat(cubics, q->left_y);
      appendFloat(cubics, q->x_coord);
      appendFloat(cubics, q->y_coord);
      p = q;
    } while (p != start);
    contours << static_cast<uint8_t>(cubicCount);
    contours.append(cubics);
  }
  data << static_cast<uint8_t>(count);
  data.append(contours);
}

static WholeGlyphVariants generateWholeGlyphs(OtLayout& layout, const LayoutPages& pages,
                                              QByteArray& data) {
  WholeGlyphVariants variants;
  for (const auto& page : pages.pages) {
    for (const auto& line : page) {
      if (line.type == LineType::Sura) continue;
      for (const auto& glyph : line.glyphs) {
        variants.try_emplace(wholeGlyphKey(glyph), WholeGlyphVariant{&glyph});
      }
    }
  }
  if (variants.empty() || variants.size() > 65535) {
    throw std::runtime_error("Whole-glyph variant count cannot fit uint16 IDs");
  }
  data << static_cast<uint16_t>(variants.size());
  unsigned nextId = 1;
  for (auto& [key, variant] : variants) {
    variant.id = static_cast<uint16_t>(nextId++);
    // Resolve the same parameterized GlyphVis used for the PDF, including any
    // parameters already embedded in a materialized glyph. No master blending.
    const auto* glyph = layout.getGlyph(*variant.placement);
    if (!glyph) throw std::runtime_error("Missing GlyphVis for whole-glyph export");
    data << key.first;
    generateWholeGlyphOutline(*glyph, data);
  }
  return variants;
}

// TODO use enums
static QMap<int, int> colorToTajweedEnum = {
  {0x00A650FF,1}, {0x006694FF,2}, {0xB4B4B4FF,3},
  {0x00ADEFFF,4}, {0xC38A08FF,5}, {0xF47216FF,6},
  {0xEC008CFF,7}, {0x8C0000FF,8}
};
static void generateBinaryPages(LayoutPages& layoutPages, QByteArray& data, int lineWidth, int scale,
                                const WholeGlyphVariants* variants = nullptr) {

  data << (uint16_t)layoutPages.pages.size();
  int surahIndex = 0;
  

  for (int pageIndex = 0; pageIndex < layoutPages.pages.size(); pageIndex++) {
    auto& page = layoutPages.pages[pageIndex];
    auto& pageText = layoutPages.originalPages[pageIndex];
    data << (uint8_t)page.size();
    for (int lineIndex = 0; lineIndex < page.size(); lineIndex++) {    
      auto& line = page[lineIndex];
      auto& lineText = pageText[lineIndex];
      if(line.glyphs.size() > 255) {
        throw "ERROR";
      }
      data << (uint8_t)line.glyphs.size();
      for (auto& glyph : line.glyphs) {      

        QByteArray glyphdata;
        int glyphMask = 0;
        if(glyph.x_advance != 0){
          glyphdata << (int16_t)glyph.x_advance;
          glyphMask = glyphMask | 0b1;
        }
        if(glyph.x_offset != 0){
          glyphdata << (int16_t)glyph.x_offset;
          glyphMask = glyphMask | 0b10;
        }
        if(glyph.y_offset != 0){
          glyphdata << (int16_t)glyph.y_offset;
          glyphMask = glyphMask | 0b100;
        }
        if(glyph.color != 0){
          glyphdata << (uint8_t)colorToTajweedEnum[glyph.color];
          glyphMask = glyphMask | 0b1000;
        }
        if(!variants && glyph.parameters.lefttatweel != 0){
          glyphdata << (uint32_t)getFixed(glyph.parameters.lefttatweel);
          glyphMask = glyphMask | 0b10000;
        }
        if(!variants && glyph.parameters.righttatweel != 0){
          glyphdata << (uint32_t)getFixed(glyph.parameters.righttatweel);
          glyphMask = glyphMask | 0b100000;
        }
        if(!variants && glyph.parameters.third != 0){
          glyphdata << (uint32_t)getFixed(glyph.parameters.third);
          glyphMask = glyphMask | 0b1000000;
        }
        if(glyph.cluster > 255){
          throw "Error";
        }
        
        const uint16_t code = variants
            ? (line.type == LineType::Sura ? 0 : variants->at(wholeGlyphKey(glyph)).id)
            : static_cast<uint16_t>(glyph.codepoint);
        data << code;
        data << (uint8_t)glyph.cluster;
        data << (uint8_t)glyphMask;        
        data.append(glyphdata);
       

      }
      data << (uint8_t)line.type;
      data << (int16_t)line.xstartposition;
      data << (uint32_t)getFixed(line.xscale);
      data << (uint32_t)getFixed(line.fontSize);
      data << (uint16_t)(lineText.size());
      if((int)line.type == 1){
        data << (uint8_t)surahIndex++;      
      }      
    }

  }
}

void GenerateLayout::generateLayoutBinary(int lineWidth, int scale, BinaryFormat format) {
  const QFileInfo fileInfo(QString::fromStdString(m_otlayout->font->projectFile().string()));
  const auto outputDirectory = fileInfo.path() + "/output";
  if (!QDir().mkpath(outputDirectory)) {
    throw std::runtime_error(("Could not create " + outputDirectory).toStdString());
  }
  const auto baseName = outputDirectory + "/" + fileInfo.completeBaseName() + "Layout";

  // Generate both buffers before replacing any output. V4 reads the live
  // parameterized GlyphVis paths; v3 retains its interpolation masters/axes.
  QByteArray interpolated, wholeGlyphs;
  if (format != BinaryFormat::WholeGlyphs) {
    interpolated.append("DKLY", 4);
    interpolated << (uint16_t)3;
    generateBinaryGlyphs(*m_otlayout, interpolated);
    generateBinaryPages(layoutPages, interpolated, lineWidth, scale);
  }
  if (format != BinaryFormat::Interpolated) {
    wholeGlyphs.append("DKLY", 4);
    wholeGlyphs << (uint16_t)4;
    const auto variants = generateWholeGlyphs(*m_otlayout, layoutPages, wholeGlyphs);
    generateBinaryPages(layoutPages, wholeGlyphs, lineWidth, scale, &variants);
  }
  const auto save = [](const QString& name, const char* bytes, qint64 size) {
    QSaveFile file(name);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes, size) != size || !file.commit()) {
      throw std::runtime_error(("Could not save " + name + ": " + file.errorString()).toStdString());
    }
  };
  if (format != BinaryFormat::WholeGlyphs) save(baseName + ".bin", interpolated.constData(), interpolated.size());
  if (format != BinaryFormat::Interpolated) {
    save(baseName + ".variants.bin", wholeGlyphs.constData(), wholeGlyphs.size());
  }
}
