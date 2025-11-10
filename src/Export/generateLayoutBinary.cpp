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
  
  for (auto& glyph : layout.glyphs) {

    

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

    data.append(glyphData);    
  }
}
// TODO use enums
static QMap<int, int> colorToTajweedEnum = {
  {0x00A650FF,1}, {0x006694FF,2}, {0xB4B4B4FF,3},
  {0x00ADEFFF,4}, {0xC38A08FF,5}, {0xF47216FF,6},
  {0xEC008CFF,7}, {0x8C0000FF,8}
};
static void generateBinaryPages(LayoutPages& layoutPages, QByteArray& data, int lineWidth, int scale) {

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
        if(glyph.lefttatweel != 0){
          glyphdata << (uint32_t)getFixed(glyph.lefttatweel);
          glyphMask = glyphMask | 0b10000;
        }
        if(glyph.righttatweel != 0){
          glyphdata << (uint32_t)getFixed(glyph.righttatweel);
          glyphMask = glyphMask | 0b100000;
        }
        if(glyph.cluster > 255){
          throw "Error";
        }
        
        data << (uint16_t)glyph.codepoint;
        data << (uint8_t)glyph.cluster;
        data << (uint8_t)glyphMask;        
        data.append(glyphdata);
       

      }
      data << (uint8_t)line.type;
      data << (int16_t)line.xstartposition;
      data << (uint32_t)getFixed(line.xscale);
      data << (uint16_t)(lineText.size());
      if((int)line.type == 1){
        data << (uint8_t)surahIndex++;      
      }      
    }

  }
}

void GenerateLayout::generateLayoutBinary(int lineWidth, int scale) {
  auto path = m_otlayout->font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  auto fileName = fileInfo.path() + "/output/" + fileInfo.completeBaseName() + "Layout.bin";
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning("Couldn't open save file.");
    return;
  }

  QByteArray glyphsData;
  QByteArray pagesData;  

  generateBinaryGlyphs(*m_otlayout,glyphsData);

  generateBinaryPages(layoutPages,pagesData,lineWidth,scale);

  file.write(glyphsData);
  file.write(pagesData);


}
