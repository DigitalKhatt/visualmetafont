/*
 * Copyright (c) 2020 Amine Anane. http: //digitalkhatt/license
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

#include "to_opentype.h"
#include "Lookup.h"
#include "Subtable.h"
#include "qfile.h"
#include <cmath>
#include "hb.h"
#include "qdatetime.h"
#include "OtLayout.h"
#include "GlyphVis.h"
#include "automedina/automedina.h"
#include <stdexcept>
#include <map>
#include "metafont.h";


ToOpenType::ToOpenType(OtLayout* layout) :ot_layout{ layout }
{
  setUniformAxis(true);

}
void ToOpenType::setUniformAxis(bool uniform) {
  uniformAxis = uniform;
  regionSubtables.clear();
  regions.clear();
  subRegions.clear();
  GDEFDeltaSets.clear();

  regions.push_back({ {0,getF2DOT14(1.0),getF2DOT14(1.0)},{0,0,0 } });
  regions.push_back({ {getF2DOT14(-1.0),getF2DOT14(-1.0),0},{0,0,0} });
  regions.push_back({ {0,0,0 },{0,getF2DOT14(1.0),getF2DOT14(1.0)} });
  regions.push_back({ {0,0,0 },{getF2DOT14(-1.0),getF2DOT14(-1.0),0} });
  subRegions.push_back({ 0,1,2,3 });
  if (!uniformAxis) {
    axisLimits.minLeft = -12;
    axisLimits.maxLeft = 12;
    axisLimits.minRight = -12;
    axisLimits.maxRight = 12;
    GDEFDeltaSets.resize(1);
  }
  else {
    axisLimits.minLeft = -20;
    axisLimits.maxLeft = 20;
    axisLimits.minRight = -20;
    axisLimits.maxRight = 20;

    for (auto& expandable : ot_layout->expandableGlyphs) {
      const auto& ff = regionSubtables.find(expandable.second);
      if (ff == regionSubtables.end()) {
        regionSubtables.insert({ expandable.second ,regionSubtables.size() + 1 });
      }
    }

    auto getRegionIndex = [&regions = regions](VariationRegion region) {
      int size = regions.size();
      for (int i = 0; i < size; i++) {
        if (regions[i] == region) return i;
      }

      regions.push_back(region);
      return size;
    };

    std::map<int, ValueLimits> orderedRegionSubtables;
    for (auto& subtable : regionSubtables) {
      orderedRegionSubtables.insert({ subtable.second,subtable.first });
    }

    for (auto& subtable : orderedRegionSubtables) {
      auto& limits = subtable.second;
      std::vector<int> subRegion;
      if (limits.maxLeft != 0.0) {
        if (limits.maxLeft > axisLimits.maxLeft) {
          throw new std::runtime_error("maxLeft exceeds axis Limit");
        }
        else if (limits.maxLeft == axisLimits.maxLeft) {
          subRegion.push_back(0);
        }
        else {
          int16_t normalLimit = getF2DOT14(limits.maxLeft / axisLimits.maxLeft);
          VariationRegion region1 = { {0,normalLimit,normalLimit},{0,0,0 } };
          subRegion.push_back(getRegionIndex(region1));
          VariationRegion region2 = { {(short)(normalLimit + 1),getF2DOT14(1),getF2DOT14(1)},{0,0,0  } };
          subRegion.push_back(getRegionIndex(region2));
          VariationRegion region3 = { {(short)(normalLimit + 1),(short)(normalLimit + 1),getF2DOT14(1)},{0,0,0 } };
          subRegion.push_back(getRegionIndex(region3));
        }
      }
      if (limits.minLeft != 0.0) {
        if (limits.minLeft < axisLimits.minLeft) {
          throw new std::runtime_error("minLeft exceeds axis Limit");
        }
        else if (limits.minLeft == axisLimits.minLeft) {
          subRegion.push_back(1);
        }
        else {
          int16_t normalLimit = getF2DOT14(-limits.minLeft / axisLimits.minLeft);
          VariationRegion region1 = { {normalLimit,normalLimit,0},{0,0,0 } };
          subRegion.push_back(getRegionIndex(region1));
          VariationRegion region2 = { {getF2DOT14(-1),(short)(normalLimit - 1),(short)(normalLimit - 1)},{0,0,0  } };
          subRegion.push_back(getRegionIndex(region2));
          VariationRegion region3 = { {getF2DOT14(-1),getF2DOT14(-1),(short)(normalLimit - 1)},{0,0,0  } };
          subRegion.push_back(getRegionIndex(region3));
        }
      }
      if (limits.maxRight != 0.0) {
        if (limits.maxRight > axisLimits.maxRight) {
          throw new std::runtime_error("maxRight exceeds axis Limit");
        }
        else if (limits.maxRight == axisLimits.maxRight) {
          subRegion.push_back(2);
        }
        else {
          int16_t normalLimit = getF2DOT14(limits.maxRight / axisLimits.maxRight);
          VariationRegion region1 = { {0,0,0  } , {0,normalLimit,normalLimit} };
          subRegion.push_back(getRegionIndex(region1));
          VariationRegion region2 = { {0,0,0  } , {(short)(normalLimit + 1),getF2DOT14(1),getF2DOT14(1)} };
          subRegion.push_back(getRegionIndex(region2));
          VariationRegion region3 = { {0,0,0  } , {(short)(normalLimit + 1),(short)(normalLimit + 1),getF2DOT14(1)} };
          subRegion.push_back(getRegionIndex(region3));
        }
      }
      if (limits.minRight != 0.0) {
        if (limits.minRight < axisLimits.minRight) {
          throw new std::runtime_error("minRight exceeds axis Limit");
        }
        else if (limits.minRight == axisLimits.minRight) {
          subRegion.push_back(3);
        }
        else {
          int16_t normalLimit = getF2DOT14(-limits.minRight / axisLimits.minRight);
          VariationRegion region1 = { {0,0,0  }, {normalLimit,normalLimit,0} };
          subRegion.push_back(getRegionIndex(region1));
          VariationRegion region2 = { {0,0,0 } , {getF2DOT14(-1),(short)(normalLimit - 1),(short)(normalLimit - 1)} };
          subRegion.push_back(getRegionIndex(region2));
          VariationRegion region3 = { {0,0,0 }, {getF2DOT14(-1),getF2DOT14(-1),(short)(normalLimit - 1)} };
          subRegion.push_back(getRegionIndex(region3));
        }
      }
      subRegions.push_back(subRegion);
    }
    GDEFDeltaSets.resize(subRegions.size());
  }
}

void ToOpenType::populateGlyphs() {
  for (auto& glyph : ot_layout->glyphs) {
    glyphs.insert(glyph.charcode, &glyph);
  }
}
void ToOpenType::initiliazeGlobals() {
  for (auto pglyph : glyphs) {
    auto& glyph = *pglyph;
    if (glyph.bbox.llx < globalValues.xMin) {
      globalValues.xMin = toInt(glyph.bbox.llx);
    }
    if (glyph.bbox.lly < globalValues.yMin) {
      globalValues.yMin = toInt(glyph.bbox.lly);
    }
    if (glyph.bbox.urx > globalValues.xMax) {
      globalValues.xMax = toInt(glyph.bbox.urx);
    }
    if (glyph.bbox.ury > globalValues.yMax) {
      globalValues.yMax = toInt(glyph.bbox.ury);
    }
    /*
    if(glyph.height> globalValues.ascender ){
      globalValues.ascender = toInt(glyph.height);
    }
    if(glyph.depth < globalValues.descender ){
      globalValues.descender = toInt(glyph.depth);
    }*/
    if (glyph.width > globalValues.advanceWidthMax) {
      globalValues.advanceWidthMax = toInt(glyph.width);
    }

    int rightSB = toInt(glyph.width) - toInt(glyph.bbox.urx);
    if (rightSB < globalValues.minRightSideBearing) {
      globalValues.minRightSideBearing = rightSB;
    }

  }

  globalValues.minLeftSideBearing = globalValues.xMin;
  globalValues.xMaxExtent = globalValues.xMax;

  globalValues.yStrikeoutSize = 20;

  globalValues.sTypoAscender = 1500;
  globalValues.sTypoDescender = -700;
  globalValues.sTypoLineGap = 200;

  globalValues.ascender = 1500;
  globalValues.descender = -700;
  globalValues.lineGap = 200;

  globalValues.major = 0;
  globalValues.minor = 1;
  globalValues.familyName = ot_layout->font->familyName(); // "DigitalKhatt Madina Quranic";
  globalValues.subFamilyName = "Regular";
  globalValues.Copyright = R"copyright(Copyright (c) 2020-2023 Amine Anane (https://github.com/DigitalKhatt))copyright";
  /*globalValues.License = R"license(This Font Software is licensed under the SIL Open Font License, Version 1.1.
This license is copied below, and is also available with a FAQ at:
http://scripts.sil.org/OFL


-----------------------------------------------------------
SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
-----------------------------------------------------------

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide
development of collaborative font projects, to support the font creation
efforts of academic and linguistic communities, and to provide a free and
open framework in which fonts may be shared and improved in partnership
with others.

The OFL allows the licensed fonts to be used, studied, modified and
redistributed freely as long as they are not sold by themselves. The
fonts, including any derivative works, can be bundled, embedded,
redistributed and/or sold with any software provided that any reserved
names are not used by derivative works. The fonts and derivatives,
however, cannot be released under any other type of license. The
requirement for fonts to remain under this license does not apply
to any document created using the fonts or their derivatives.

DEFINITIONS
"Font Software" refers to the set of files released by the Copyright
Holder(s) under this license and clearly marked as such. This may
include source files, build scripts and documentation.

"Reserved Font Name" refers to any names specified as such after the
copyright statement(s).

"Original Version" refers to the collection of Font Software components as
distributed by the Copyright Holder(s).

"Modified Version" refers to any derivative made by adding to, deleting,
or substituting -- in part or in whole -- any of the components of the
Original Version, by changing formats or by porting the Font Software to a
new environment.

"Author" refers to any designer, engineer, programmer, technical
writer or other person who contributed to the Font Software.

PERMISSION & CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining
a copy of the Font Software, to use, study, copy, merge, embed, modify,
redistribute, and sell modified and unmodified copies of the Font
Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components,
in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled,
redistributed and/or sold with any software, provided that each copy
contains the above copyright notice and this license. These can be
included either as stand-alone text files, human-readable headers or
in the appropriate machine-readable metadata fields within text or
binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font
Name(s) unless explicit written permission is granted by the corresponding
Copyright Holder. This restriction only applies to the primary font name as
presented to the users.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
Software shall not be used to promote, endorse or advertise any
Modified Version, except to acknowledge the contribution(s) of the
Copyright Holder(s) and the Author(s) or with their explicit written
permission.

5) The Font Software, modified or unmodified, in part or in whole,
must be distributed entirely under this license, and must not be
distributed under any other license. The requirement for fonts to
remain under this license does not apply to any document created
using the Font Software.

TERMINATION
This license becomes null and void if any of the above conditions are
not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
OTHER DEALINGS IN THE FONT SOFTWARE.)license";*/
}

void ToOpenType::setGIds() {


  QMap<quint16, quint16> newCodes;

  QMap<QString, quint16> glyphCodePerName;
  QMap<quint16, QString> glyphNamePerCode;
  QMap<quint16, quint16> unicodeToGlyphCode;
  QMap<quint16, OtLayout::GDEFClasses> glyphGlobalClasses;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> tempGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> addedGlyphs;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> substEquivGlyphs;


  if (!ot_layout->glyphs.contains("notdef")) {
    throw new std::runtime_error("notdef glyph not found");
  }

  if (!ot_layout->glyphs.contains("null")) {
    throw new std::runtime_error("null glyph not found");
  }

  newCodes.insert(ot_layout->glyphCodePerName.value("notdef"), 0);
  newCodes.insert(ot_layout->glyphCodePerName.value("null"), 1);
  uint16_t newCode = 2;

  for (auto code : ot_layout->glyphNamePerCode.keys()) {
    auto name = ot_layout->glyphNamePerCode.value(code);
    if (name.isEmpty()) continue;
    if (name != "notdef" && name != "null") {
      if (!ot_layout->glyphs.contains(name)) {
        throw new std::runtime_error(QString("Glyph name %1 not found").arg(name).toStdString());
      }
      newCodes.insert(code, newCode);
      auto glyph = &ot_layout->glyphs[name];
      glyph->charcode = newCode;
      newCode++;
    }
  }

  QMap<quint16, quint16>::const_iterator iter = newCodes.constBegin();
  while (iter != newCodes.constEnd()) {
    if (!ot_layout->glyphNamePerCode.contains(iter.key())) {
      throw new std::runtime_error(QString("Code %1 not found").arg(iter.key()).toStdString());
    }
    auto name = ot_layout->glyphNamePerCode.value(iter.key());
    glyphCodePerName.insert(name, iter.value());
    glyphNamePerCode.insert(iter.value(), name);

    iter++;
  }

  QMap<quint16, quint16>::const_iterator unicodeToGlyphCodeIter = ot_layout->unicodeToGlyphCode.constBegin();
  while (unicodeToGlyphCodeIter != ot_layout->unicodeToGlyphCode.constEnd()) {
    if (!newCodes.contains(unicodeToGlyphCodeIter.value())) {
      throw new std::runtime_error(QString("Code %1 not found").arg(unicodeToGlyphCodeIter.value()).toStdString());
    }
    auto unicode = unicodeToGlyphCodeIter.key();
    if (unicode < 0xE000 && unicode != 0 && unicode != 1) {
      unicodeToGlyphCode.insert(unicode, newCodes.value(unicodeToGlyphCodeIter.value()));
    }

    unicodeToGlyphCodeIter++;
  }

  QMap<quint16, OtLayout::GDEFClasses>::const_iterator glyphGlobalClassesIter = ot_layout->glyphGlobalClasses.constBegin();
  while (glyphGlobalClassesIter != ot_layout->glyphGlobalClasses.constEnd()) {
    if (!newCodes.contains(glyphGlobalClassesIter.key())) {
      throw new std::runtime_error(QString("Code %1 not found").arg(glyphGlobalClassesIter.key()).toStdString());
    }

    glyphGlobalClasses.insert(newCodes.value(glyphGlobalClassesIter.key()), glyphGlobalClassesIter.value());

    glyphGlobalClassesIter++;
  }


  for (std::pair<int, std::unordered_map<GlyphParameters, GlyphVis*>> element : ot_layout->tempGlyphs)
  {
    if (!newCodes.contains(element.first)) {
      throw new std::runtime_error(QString("Code %1 not found").arg(element.first).toStdString());
    }

    tempGlyphs.insert({ newCodes.value(element.first), element.second });
  }

  for (std::pair<int, std::unordered_map<GlyphParameters, GlyphVis*>> element : ot_layout->addedGlyphs)
  {
    if (!newCodes.contains(element.first)) {
      throw new std::runtime_error(QString("Code %1 not found").arg(element.first).toStdString());
    }

    addedGlyphs.insert({ newCodes.value(element.first), element.second });
  }

  for (std::pair<int, std::unordered_map<GlyphParameters, GlyphVis*>> element : ot_layout->substEquivGlyphs)
  {
    if (!newCodes.contains(element.first)) {
      throw new std::runtime_error(QString("Code %1 not found").arg(element.first).toStdString());
    }

    substEquivGlyphs.insert({ newCodes.value(element.first), element.second });
  }

  for (int i = 0; i <= 4; i++) {
    auto automedina = ot_layout->automedina;
    QMap <QString, QMap<quint16, QPoint>>& currentAnchors = i == 0 ? automedina->markAnchors : i == 1 ? automedina->entryAnchors : i == 2
      ? automedina->exitAnchors : i == 3 ? automedina->entryAnchorsRTL : automedina->exitAnchorsRTL;

    QMap <QString, QMap<quint16, QPoint>>  anchors;
    QMap <QString, QMap<quint16, QPoint>>::const_iterator anchorsIter = currentAnchors.constBegin();
    while (anchorsIter != currentAnchors.end()) {
      QMap<quint16, QPoint> map;
      QMap<quint16, QPoint>::ConstIterator mapIter = anchorsIter.value().constBegin();
      while (mapIter != anchorsIter.value().constEnd()) {
        if (!newCodes.contains(mapIter.key())) {
          throw new std::runtime_error(QString("Code %1 not found").arg(mapIter.key()).toStdString());
        }
        map.insert(newCodes.value(mapIter.key()), mapIter.value());
        mapIter++;
      }
      anchors.insert(anchorsIter.key(), map);
      anchorsIter++;
    }
    currentAnchors = anchors;
  }





  ot_layout->glyphCodePerName = glyphCodePerName;
  ot_layout->glyphNamePerCode = glyphNamePerCode;
  ot_layout->unicodeToGlyphCode = unicodeToGlyphCode;
  ot_layout->tempGlyphs = tempGlyphs;
  ot_layout->addedGlyphs = addedGlyphs;
  ot_layout->substEquivGlyphs = substEquivGlyphs;

  ot_layout->glyphGlobalClasses = glyphGlobalClasses;



  for (auto& cvlo : ot_layout->automedina->cvxxfeatures) {
    QMap<quint16, QVector<ExtendedGlyph>> newalternates;
    for (auto iter = cvlo.begin(); iter != cvlo.end(); ++iter) {
      auto oldid = iter.key();
      auto newid = newCodes[oldid];
      for (auto glyph : iter.value()) {
        ExtendedGlyph extendedGlyph = { newCodes[glyph.code],glyph.lefttatweel,glyph.righttatweel };
        newalternates[newid].append(extendedGlyph);
      }
    }
    cvlo = newalternates;
  }



}

bool ToOpenType::GenerateFile(QString fileName, std::string lokkupsFileName) {

  struct Table {
    QByteArray data;
    hb_tag_t tag;
    uint32_t checkSum = 0;
    uint32_t offset = 0;
    uint32_t length = 0;
  };

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
    return false;

  ot_layout->loadLookupFile(lokkupsFileName);

  ot_layout->substEquivGlyphs.clear();

  ot_layout->generateSubstEquivGlyphs();

  setGIds();

  ot_layout->loadLookupFile(lokkupsFileName);


  glyphs.clear();
  for (auto it = ot_layout->glyphCodePerName.keyValueBegin(); it != ot_layout->glyphCodePerName.keyValueEnd(); ++it) {
    glyphs.insert(it->second, &ot_layout->glyphs[it->first]);
  }

  initiliazeGlobals();

  QVector<Table> tables;
  QByteArray cffArray;

  generateComponents();

  //generate new color glyphs
  if (isCff2) {
    cffArray = cff2();
  }
  else {
    cffArray = cff();
  }


  tables.append({ head(),HB_TAG('h', 'e', 'a', 'd') });
  tables.append({ hhea(),HB_TAG('h', 'h', 'e', 'a') });
  tables.append({ maxp(),HB_TAG('m', 'a', 'x', 'p') });
  tables.append({ os2(),HB_TAG('O','S','/','2') });
  tables.append({ name(),HB_TAG('n','a','m','e') });
  tables.append({ cmap(),HB_TAG('c','m','a','p') });
  tables.append({ post(),HB_TAG('p','o','s','t') });
  if (isCff2) {
    tables.append({ cffArray,HB_TAG('C','F','F','2') });
  }
  else {
    tables.append({ cffArray,HB_TAG('C','F','F',' ') });
  }
  tables.append({ hmtx(),HB_TAG('h','m','t','x') });
  auto gposData = gpos();
  tables.append({ gdef(),HB_TAG('G','D','E','F') });

  tables.append({ gsub(),HB_TAG('G','S','U','B') });
  tables.append({ gposData,HB_TAG('G','P','O','S') });
  tables.append({ dsig(),HB_TAG('D','S','I','G') });
  if (isCff2) {
    tables.append({ fvar(),HB_TAG('f','v','a','r') });
    tables.append({ HVAR(),HB_TAG('H','V','A','R') });
    //tables.append({ MVAR(),HB_TAG('M','V','A','R') });
    tables.append({ STAT(),HB_TAG('S','T','A','T') });
    if (ot_layout->extended) {
      tables.append({ JTST(),HB_TAG('J', 'T', 'S', 'T') });
    }
  }

  if (!layers.isEmpty()) {
    QByteArray cpal;
    QByteArray colr;
    if (colrcpal(colr, cpal)) {
      tables.append({ colr,HB_TAG('C','O','L','R') });
      tables.append({ cpal,HB_TAG('C','P','A','L') });
    }
  }


  uint16_t numTables = tables.size();

  int offset = sizeof(uint32_t) + 4 * sizeof(uint16_t) + numTables * 4 * sizeof(uint32_t);

  for (int i = 0; i < numTables; i++) {

    tables[i].length = tables[i].data.size();
    uint32_t paddingLength = (tables[i].length + 3) & ~3;
    for (uint32_t pad = 0; pad < paddingLength - tables[i].length; pad++) {
      tables[i].data << (uint8_t)0;
    }
    tables[i].checkSum = calcTableChecksum((uint32_t*)tables[i].data.constData(), paddingLength);
    tables[i].offset = offset;
    offset += paddingLength;
  }

  QByteArray data;


  uint16_t entrySelector = floor(log2(numTables));
  uint16_t searchRange = exp2(entrySelector) * 16;

  data << 0x4F54544F;
  data << numTables;
  data << searchRange;
  data << entrySelector;
  data << (uint16_t)(numTables * 16 - searchRange);

  auto tagordered = tables;

  std::sort(tagordered.begin(), tagordered.end(), [](const Table& a, const Table& b) {return a.tag < b.tag; });

  for (int i = 0; i < numTables; i++) {
    auto& table = tagordered[i];
    data << (uint32_t)table.tag;
    data << (uint32_t)table.checkSum;
    data << (uint32_t)table.offset;
    data << (uint32_t)table.length;
  }

  int headPos = 0;

  for (int i = 0; i < numTables; i++) {
    auto& table = tables[i];
    if (tables[i].tag == HB_TAG('h', 'e', 'a', 'd')) {
      headPos = data.size();
    }
    data.append(table.data);
  }

  //checkSumAdjustment
  int32_t totalChecksum = calcTableChecksum((uint32_t*)data.constData(), data.size());
  totalChecksum = 0xB1B0AFBA - totalChecksum;

  QByteArray checksumArray;

  checksumArray << (uint32_t)totalChecksum;
  data.replace(headPos + 8, 4, checksumArray);

  file.write(data);

  return true;
}

uint32_t ToOpenType::calcTableChecksum(uint32_t* table, uint32_t length)
{
  uint32_t sum = 0L;
  uint32_t* endPtr = table + ((length + 3) & ~3) / sizeof(uint32_t);
  int n = 1;
  bool littleEnd = *(char*)&n == 1;

  while (table < endPtr) {
    auto value = *table;

    if (littleEnd) {
      value = 0 | ((value & 0x000000ff) << 24) | ((value & 0x0000ff00) << 8) | ((value & 0x00ff0000) >> 8) | ((value & 0xff000000) >> 24);
    }

    sum += value;
    table++;
  }

  return sum;
}

bool ToOpenType::colrcpal(QByteArray& colr, QByteArray& cpal) {

  if (layers.isEmpty()) return false;

  QMap<Color, uint16_t> colormap;
  QVector<Color> colors;

  QByteArray layerRecords;
  QByteArray baseGlyphRecords;
  uint16_t layerIndex = 0;

  QMapIterator<uint16_t, QVector<Layer>> layIter(this->layers);
  while (layIter.hasNext()) {
    layIter.next();
    auto& layers = layIter.value();
    if (layers.size() == 0) {
      throw new std::runtime_error("The number of layers cannot be 0");
    }
    baseGlyphRecords << (uint16_t)layIter.key();
    baseGlyphRecords << (uint16_t)layerIndex;
    baseGlyphRecords << (uint16_t)layers.size();
    for (auto& layer : layers) {
      layerRecords << (uint16_t)layer.gid;
      if (layer.color.foreground) {
        layerRecords << (uint16_t)0xFFFF;
      }
      else if (colormap.contains(layer.color)) {
        layerRecords << (uint16_t)colormap.value(layer.color);
      }
      else {
        layerRecords << (uint16_t)colors.size();
        colormap.insert(layer.color, colors.size());
        colors.append(layer.color);
      }
      layerIndex++;
    }

  }

  uint32_t baseGlyphRecordsOffset = 2 + 2 + 4 + 4 + 2;
  uint32_t layerRecordsOffset = baseGlyphRecordsOffset + baseGlyphRecords.size();

  colr << (uint16_t)0; // version
  colr << (uint16_t)this->layers.size(); // numBaseGlyphRecords
  colr << (uint32_t)baseGlyphRecordsOffset; // baseGlyphRecordsOffset
  colr << (uint32_t)layerRecordsOffset; // layerRecordsOffset
  colr << (uint16_t)layerIndex; // Number of Layer Records

  colr.append(baseGlyphRecords);
  colr.append(layerRecords);

  cpal << (uint16_t)0; // version
  cpal << (uint16_t)colors.size(); // numPaletteEntries
  cpal << (uint16_t)1; // numPalettes
  cpal << (uint16_t)colors.size(); // numColorRecords
  cpal << (uint32_t)(2 + 2 + 2 + 2 + 4 + 2); // offsetFirstColorRecord
  cpal << (uint16_t)0; // colorRecordIndices[numPalettes]

  for (auto& color : colors) {
    cpal << (uint8_t)color.blue;
    cpal << (uint8_t)color.green;
    cpal << (uint8_t)color.red;
    cpal << (uint8_t)color.alpha;
  }

  return true;

}

QByteArray ToOpenType::cmap() {
  return ot_layout->getCmap();
}
QByteArray ToOpenType::gdef() {
  return ot_layout->getGDEF();
}
QByteArray ToOpenType::gpos() {
  return ot_layout->getGPOS();

}
QByteArray ToOpenType::gsub() {
  return ot_layout->getGSUB();
}
QByteArray ToOpenType::head() {
  QByteArray data;

  QDateTime now = QDateTime::currentDateTimeUtc();
  qint64 secs = QDate(1904, 1, 1).startOfDay(Qt::UTC).secsTo(now);

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (uint32_t)((globalValues.major << 16) & globalValues.minor); // fontRevision
  data << (uint32_t)0; // See https://docs.microsoft.com/en-us/typography/opentype/spec/otff (Calculating Checksums)
  data << (uint32_t)0x5F0F3CF5; // magicNumber
  data << (uint16_t)0b0000000000000001; // flags
  data << (uint16_t)1000; // unitsPerEm
  data << (qint64)secs; // created
  data << (qint64)secs; // modified
  data << (int16_t)globalValues.xMin; // xMin
  data << (int16_t)globalValues.yMin; // yMin
  data << (int16_t)globalValues.xMax; // xMax
  data << (int16_t)globalValues.yMax; // yMax
  data << (uint16_t)0; // macStyle
  data << (uint16_t)8; // lowestRecPPEM
  data << (int16_t)2; // fontDirectionHint
  data << (int16_t)0; // indexToLocFormat
  data << (int16_t)0; // glyphDataFormat

  return data;
}
QByteArray ToOpenType::hhea() {
  QByteArray data;

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (int16_t)globalValues.ascender; // ascender
  data << (int16_t)globalValues.descender; // descender
  data << (int16_t)globalValues.lineGap; // lineGap
  data << (uint16_t)globalValues.advanceWidthMax; // advanceWidthMax
  data << (int16_t)globalValues.minLeftSideBearing; // minLeftSideBearing
  data << (int16_t)globalValues.minRightSideBearing; // minRightSideBearing
  data << (int16_t)globalValues.xMaxExtent; // xMaxExtent
  data << (int16_t)0; // caretSlopeRise
  data << (int16_t)0; // caretSlopeRun
  data << (int16_t)0; // caretOffset
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // metricDataFormat
  data << (uint16_t)(glyphs.lastKey() + 1); // numberOfHMetrics

  return data;
}
QByteArray ToOpenType::hmtx() {
  QByteArray data;

  auto& marks = ot_layout->automedina->classes.value("marks");



  for (int i = 0; i < glyphs.lastKey() + 1; i++) {
    if (glyphs.contains(i)) {
      auto glyph = glyphs.value(i);

      bool ismark = false;
      if (!glyph->originalglyph.isEmpty()) {
        ismark = marks.contains(glyph->originalglyph);
      }
      else {
        ismark = marks.contains(glyph->name);
      }
      if (ismark) {
        data << (uint16_t)0;
      }
      else {
        data << (uint16_t)toInt(glyph->width);
      }

      data << (int16_t)toInt(glyph->bbox.llx);
    }
    else {
      data << (uint16_t)0;
      data << (int16_t)0;
    }
  }

  return data;
}

QByteArray ToOpenType::maxp() {
  QByteArray data;

  data << (uint32_t)0x00005000;
  data << (uint16_t)(glyphs.lastKey() + 1);

  return data;
}
QByteArray ToOpenType::name() {

  struct Name {
    uint16_t nameID;
    QString str;
    uint16_t offset = 0;
    uint16_t length = 0;
  };

  auto streamString = [](QByteArray& data, QString str) {
    int l = str.length();
    const QChar* ub = str.unicode();

    while (l--) {
      data << (uint8_t)ub->row();
      data << (uint8_t)ub->cell();
      ub++;
    }

  };

  QVector<Name> names;


  names.append(Name{ 0,globalValues.Copyright });
  names.append(Name{ 1,globalValues.familyName });
  names.append(Name{ 2,globalValues.subFamilyName });
  names.append(Name{ 3,globalValues.fullName() + "V01" });
  names.append(Name{ 4,globalValues.fullName() });
  names.append(Name{ 5,"Version " + QString::number(globalValues.major) + "." + QString::number(globalValues.minor) });
  names.append(Name{ 6,globalValues.fullName().replace(" ","").mid(0,63) });
  //names.append(Name{ 7,"DigitalKhatt" });
  //names.append(Name{ 8,"DigitalKhatt" });
  names.append(Name{ 9,"Amine Anane" });
  names.append(Name{ 10,"This font is the OpenType version of the MetaFont-designed parametric font used in the DigitalKhatt Arabic typesetter which justifies text dynamically using curvilinear expansion of letters" });
  names.append(Name{ 11,"https://digitalkhatt.org/" });
  names.append(Name{ 12,"https://digitalkhatt.org/" });
  names.append(Name{ 13,globalValues.License });
  names.append(Name{ 14,"http://scripts.sil.org/OFL" });

  names.append(Name{ 16,globalValues.familyName });
  names.append(Name{ 17,globalValues.subFamilyName });
  names.append(Name{ 19,"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" });

  names.append(Name{ 21,globalValues.familyName });
  names.append(Name{ 22,globalValues.subFamilyName });

  names.append(Name{ LTATNameId,"Left Elongation" });
  names.append(Name{ RTATNameId,"Right Elongation" });

  QByteArray stringStorage;
  uint16_t offset = 0;
  for (auto& name : names) {
    name.offset = offset;
    name.length = name.str.length() * 2;
    streamString(stringStorage, name.str);
    offset += name.length;
  }

  QByteArray data;
  QByteArray nameRecords;

  int recordCount = 0;


  for (auto& name : names) {
    nameRecords << (uint16_t)0; // Platform
    nameRecords << (uint16_t)3; // encodingID
    nameRecords << (uint16_t)0; // languageID
    nameRecords << (uint16_t)name.nameID; // nameID
    nameRecords << (uint16_t)name.length; // length
    nameRecords << (uint16_t)name.offset; // offset

    recordCount++;


  }

  for (auto& name : names) {
    nameRecords << (uint16_t)3; // Platform
    nameRecords << (uint16_t)1; // encodingID
    nameRecords << (uint16_t)0x0409; // languageID
    nameRecords << (uint16_t)name.nameID; // nameID
    nameRecords << (uint16_t)name.length; // length
    nameRecords << (uint16_t)name.offset; // offset

    recordCount++;
  }

  data << (uint16_t)0; // format
  data << (uint16_t)recordCount; // count
  data << (uint16_t)(2 + 2 + 2 + recordCount * 12); // stringOffset

  data.append(nameRecords);
  data.append(stringStorage);

  return data;
}
QByteArray ToOpenType::os2() {
  QByteArray data;

  data << (uint16_t)0x0005; // version
  data << (int16_t)500; // xAvgCharWidth
  data << (uint16_t)400; // usWeightClass
  data << (uint16_t)5; // usWidthClass
  data << (uint16_t)0b0000000000001000; // fsType
  data << (int16_t)150; // ySubscriptXSize
  data << (int16_t)150; // ySubscriptYSize
  data << (int16_t)300; // ySubscriptXOffset
  data << (int16_t)100; // ySubscriptYOffset
  data << (int16_t)150; // ySuperscriptXSize
  data << (int16_t)150; // ySuperscriptYSize
  data << (int16_t)300; // ySuperscriptXOffset
  data << (int16_t)1000; // ySuperscriptYOffset
  data << (int16_t)globalValues.yStrikeoutSize; // yStrikeoutSize
  data << (int16_t)300;  //yStrikeoutPosition
  data << (int16_t)0;  //sFamilyClass
  //panose
  data << (uint8_t)0;  //bFamilyType
  data << (uint8_t)0;  //bSerifStyle
  data << (uint8_t)0;  //bWeight
  data << (uint8_t)0;  //bProportion
  data << (uint8_t)0;  //bContrast
  data << (uint8_t)0;  //bStrokeVariation
  data << (uint8_t)0;  //bArmStyle
  data << (uint8_t)0;  //bLetterform
  data << (uint8_t)0;  //bMidline
  data << (uint8_t)0;  //bXHeight


  data << (uint32_t)(1 << 13); //ulUnicodeRange1(Bits 0-31)
  data << (uint32_t)0; // ulUnicodeRange2(Bits 32-63)
  data << (uint32_t)0; //ulUnicodeRange3 (Bits 64-95)
  data << (uint32_t)0; //ulUnicodeRange4 (Bits 96-127)

  data << (uint32_t)0;  //achVendID
  data << (uint16_t)0b0000000011000000;  //fsSelection

  data << (uint16_t)10;  //usFirstCharIndex
  data << (uint16_t)0x08F3;  //usLastCharIndex

  data << (int16_t)globalValues.sTypoAscender; // sTypoAscender
  data << (int16_t)globalValues.sTypoDescender; // sTypoDescender
  data << (int16_t)globalValues.sTypoLineGap; // sTypoLineGap
  data << (uint16_t)globalValues.ascender; // usWinAscent
  data << (int16_t)-globalValues.descender; // usWinDescent

  data << (uint32_t)(1 << 6); // ulCodePageRange1 Bits 0-31
  data << (uint32_t)(1 << 19); // ulCodePageRange2 Bits 32-63

  data << (int16_t)400; // sxHeight
  data << (int16_t)1000; // sCapHeight
  data << (uint16_t)0; // usDefaultChar
  data << (uint16_t)0x0020; // usBreakChar

  data << (uint16_t)30; // usMaxContext

  data << (uint16_t)0; // usLowerOpticalPointSize
  data << (uint16_t)0xFFFF; // usUpperOpticalPointSize

  return data;
}
QByteArray ToOpenType::post() {
  QByteArray data;

  bool useGlyphName = isCff2 && ot_layout->extended;

  if (!useGlyphName) {
    data << (uint32_t)0x00030000; // version
  }
  else {
    data << (uint32_t)0x00020000; // version
  }


  data << (uint32_t)0; // italicAngle
  data << (int16_t)-200; // underlinePosition
  data << (int16_t)globalValues.yStrikeoutSize; // underlineThickness
  data << (uint32_t)0; // isFixedPitch
  data << (uint32_t)0; // minMemType42
  data << (uint32_t)0; // maxMemType42
  data << (uint32_t)0; // minMemType1
  data << (uint32_t)0; // maxMemType1

  if (useGlyphName) {
    int glyphCount = glyphs.lastKey() + 1;

    data << (int16_t)glyphCount;

    int index = 0;

    for (int i = 0; i < glyphCount; i++) {
      QByteArray glyphArray;

      if (glyphs.contains(i)) {
        data << (uint16_t)(index + 258);
        index++;
      }
      else {
        data << (uint16_t)(258);//  .notdef;
      }
    }

    for (auto& glyph : glyphs) {
      auto name = glyph->name.toLatin1();
      data << (uint8_t)name.size();
      data.append(name);
    }
  }

  return data;
}
void ToOpenType::dumpPath(GlyphVis& glyph, QByteArray& data, mp_fill_object* fill, double& currentx, double& currenty, PathLimits& pathLimits, GlyphVis** compGlyphVis) {



  if (fill->pre_script && strcmp(fill->pre_script, "begincomponent") == 0) {
    auto comp = QString(fill->post_script).split(",");
    QString name = comp[0];
    GlyphVis& compGlyph = ot_layout->glyphs[name];
    auto initPosX = comp[1].toDouble() + glyph.matrix.xpart;
    auto initPosY = comp[2].toDouble() + glyph.matrix.ypart;
    auto dx = initPosX - currentx;
    auto dy = initPosY - currenty;

    if (subrByGlyph.contains(compGlyph.charcode)) {
      auto subrByGlyphInfo = subrByGlyph.value(compGlyph.charcode);


      fixed_to_cff2(data, dx);
      fixed_to_cff2(data, dy);
      data << (uint8_t)21; // rmoveto;    
      int_to_cff2(data, subrByGlyphInfo.offset - subIndexBias);
      data << (uint8_t)10; // callsubr;

      currentx = initPosX + subrByGlyphInfo.lastx;
      currenty = initPosY + subrByGlyphInfo.lasty;

      *compGlyphVis = &compGlyph;

      return;
    }
  }

  auto path = fill->path_p;

  mp_gr_knot p, q;
  PathLimits pLimits, qLimits;

  if (path == nullptr) return;

  double defaultt = path->x_coord - currentx;
  fixed_to_cff2(data, defaultt);
  if (isCff2) {
    auto delta = pathLimits.x_coord() - pathLimits.currentx - defaultt;
    data.append(blend(delta, pathLimits.limits));
  }

  defaultt = path->y_coord - currenty;
  fixed_to_cff2(data, defaultt);
  if (isCff2) {
    auto delta = pathLimits.y_coord() - pathLimits.currenty - defaultt;
    data.append(blend(delta, pathLimits.limits));
  }

  data << (uint8_t)21; // rmoveto;

  p = path;
  pLimits = pathLimits;
  do {
    q = p->next;
    qLimits = pLimits.next();

    defaultt = p->right_x - p->x_coord;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto deltax = pLimits.right_x() - pLimits.x_coord() - defaultt;
      data.append(blend(deltax, pathLimits.limits));
    }

    defaultt = p->right_y - p->y_coord;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto deltay = pLimits.right_y() - pLimits.y_coord() - defaultt;
      data.append(blend(deltay, pathLimits.limits));
    }

    defaultt = q->left_x - p->right_x;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.left_x() - pLimits.right_x() - defaultt;
      data.append(blend(delta, pathLimits.limits));
    }

    defaultt = q->left_y - p->right_y;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.left_y() - pLimits.right_y() - defaultt;
      data.append(blend(delta, pathLimits.limits));
    }

    defaultt = q->x_coord - q->left_x;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.x_coord() - qLimits.left_x() - defaultt;
      data.append(blend(delta, pathLimits.limits));
    }

    defaultt = q->y_coord - q->left_y;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.y_coord() - qLimits.left_y() - defaultt;
      data.append(blend(delta, pathLimits.limits));
    }

    if (!isCff2) {
      data << (uint8_t)8; // rrcurveto;
    }

    p = q;
    pLimits = qLimits;
  } while (p != path);

  currentx = p->x_coord;
  currenty = p->y_coord;
  if (isCff2) {
    pathLimits.currentx = pLimits.x_coord();
    pathLimits.currenty = pLimits.y_coord();
  }


  if (isCff2) {
    data << (uint8_t)8; // rrcurveto;
  }

}
QByteArray ToOpenType::charString(GlyphVis& glyph, bool colored, bool iscff2, QVector<Layer>& layers, double& currentx, double& currenty, ToOpenType::ContourLimits contourLimits) {


  auto body = glyph.copiedPath;
  QByteArray data;

  Color ayablue = Color{ 255,240,220,255 };
  PathLimits pathlimits;
  pathlimits.limits = contourLimits.limits;

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {
        QByteArray layerArray;
        mp_fill_object* fillobject = (mp_fill_object*)body;

        if (!colored) {
          if (iscff2) {
            pathlimits.maxLeft = contourLimits.maxLeft != nullptr ? ((mp_fill_object*)contourLimits.maxLeft)->path_p : nullptr;
            pathlimits.minLeft = contourLimits.minLeft != nullptr ? ((mp_fill_object*)contourLimits.minLeft)->path_p : nullptr;
            pathlimits.maxRight = contourLimits.maxRight != nullptr ? ((mp_fill_object*)contourLimits.maxRight)->path_p : nullptr;
            pathlimits.minRight = contourLimits.minRight != nullptr ? ((mp_fill_object*)contourLimits.minRight)->path_p : nullptr;
          }
          GlyphVis* compGlyphVis = nullptr;
          dumpPath(glyph, layerArray, fillobject, currentx, currenty, pathlimits, &compGlyphVis);

          data.append(layerArray);

          if (compGlyphVis) {
            body = body->next;
            while (body->type != mp_stroked_code || ((mp_stroked_object*)body)->pre_script == nullptr || strcmp(((mp_stroked_object*)body)->pre_script, "endcomponent")) body = body->next;
          }
        }
        else {
          Layer layer;
          QByteArray newGlyphArray;
          double tempx = 0;
          double tempy = 0;
          PathLimits tempPathLimits;
          GlyphVis* compGlyphVis = nullptr;
          dumpPath(glyph, newGlyphArray, fillobject, tempx, tempy, tempPathLimits, &compGlyphVis);
          layer.charString = newGlyphArray;
          if (compGlyphVis) {
            body = body->next;
            while (body->type != mp_stroked_code || ((mp_stroked_object*)body)->pre_script == nullptr || strcmp(((mp_stroked_object*)body)->pre_script, "endcomponent")) body = body->next;
          }

          if (fillobject->color_model == mp_rgb_model) {
            layer.color.red = toInt(fillobject->color.a_val * 255);
            layer.color.green = toInt(fillobject->color.b_val * 255);
            layer.color.blue = toInt(fillobject->color.c_val * 255);
          }
          else if (fillobject->color_model == mp_no_model) {
            layer.color.foreground = true;
          }
          layers.append(layer);
        }

        break;
      }
      default:
        break;
      }
      body = body->next;
      if (iscff2) {
        contourLimits.maxLeft = contourLimits.maxLeft != nullptr ? contourLimits.maxLeft->next : nullptr;
        contourLimits.minLeft = contourLimits.minLeft != nullptr ? contourLimits.minLeft->next : nullptr;
        contourLimits.maxRight = contourLimits.maxRight != nullptr ? contourLimits.maxRight->next : nullptr;
        contourLimits.minRight = contourLimits.minRight != nullptr ? contourLimits.minRight->next : nullptr;
      }

    } while (body != nullptr);
  }

  return data;

}
QByteArray ToOpenType::charStrings(bool iscff2) {

  QByteArray objectData;


  QVector<uint> offsets;

  uint offset = 1;

  int glyphCount = glyphs.lastKey() + 1;

  std::map<int, QString> coloredglyphs;

  for (int i = 0; i < glyphCount; i++) {
    QByteArray glyphArray;

    if (glyphs.contains(i)) {
      auto& glyph = *glyphs.value(i);

      if (!glyph.coloredglyph.isEmpty()) {
        coloredglyphs.insert({ glyph.charcode,glyph.coloredglyph });
      }

      QByteArray glyphData;

      double currentx = 0.0;
      double currenty = 0.0;

      int regionSubtable = 0;

      if (subrByGlyph.contains(glyph.charcode)) {
        int_to_cff2(glyphData, subrByGlyph.value(glyph.charcode).offset - subIndexBias);
        glyphData << (uint8_t)10; // callsubr;
      }
      else {
        ContourLimits contourLimits;

        const auto& ff = ot_layout->expandableGlyphs.find(glyph.name);

        if (ff != ot_layout->expandableGlyphs.end()) {

          auto& jj = ff->second;
          contourLimits.limits = jj;

          if (uniformAxis) {
            regionSubtable = regionSubtables[jj];
            if (regionSubtable == 7) {
              int tt = 5;
            }
          }

          if (jj.maxLeft != 0) {
            GlyphParameters parameters{};

            parameters.lefttatweel = jj.maxLeft;
            parameters.righttatweel = 0.0;

            auto alternate = glyph.getAlternate(parameters);
            contourLimits.maxLeft = alternate->copiedPath;

          }
          if (jj.minLeft != 0) {
            GlyphParameters parameters{};

            parameters.lefttatweel = jj.minLeft;
            parameters.righttatweel = 0.0;

            auto alternate = glyph.getAlternate(parameters);
            contourLimits.minLeft = alternate->copiedPath;

          }
          if (jj.maxRight != 0) {
            GlyphParameters parameters{};

            parameters.lefttatweel = 0.0;
            parameters.righttatweel = jj.maxRight;

            auto alternate = glyph.getAlternate(parameters);
            contourLimits.maxRight = alternate->copiedPath;

          }
          if (jj.minRight != 0) {
            GlyphParameters parameters{};

            parameters.lefttatweel = 0.0;
            parameters.righttatweel = jj.minRight;

            auto alternate = glyph.getAlternate(parameters);
            contourLimits.minRight = alternate->copiedPath;

          }
        }
        QVector<Layer> layers;
        glyphData = charString(glyph, false, iscff2, layers, currentx, currenty, contourLimits);
      }

      if (glyphData.size() == 0) {
        if (!iscff2) {
          int_to_cff2(glyphArray, toInt(glyph.width)); // width
          glyphArray << (uint8_t)14; //  endchar
        }
      }
      else {
        if (iscff2) {
          if (regionSubtable != 0) {
            int_to_cff2(glyphArray, toInt(regionSubtable)); // regionSubtable
            glyphArray << (uint8_t)15; // vsindex;
          }
          glyphArray.append(glyphData);
        }
        else {
          int_to_cff2(glyphArray, toInt(glyph.width)); // width
          glyphArray.append(glyphData);
          glyphArray << (uint8_t)14; //  endchar
        }

      }


    }
    else if (!iscff2) {
      int_to_cff2(glyphArray, 0); // width
      glyphArray << (uint8_t)14; //  endchar
    }

    offsets.append(offset);
    offset += glyphArray.size();
    objectData.append(glyphArray);
  }

  for (auto coloredGlyp : coloredglyphs) {
    GlyphVis& glyph = ot_layout->glyphs[coloredGlyp.second];
    QVector<Layer> layers;

    QByteArray glyphData;

    double currentx = 0.0;
    double currenty = 0.0;


    if (subrByGlyph.contains(glyph.charcode)) {
      int_to_cff2(glyphData, subrByGlyph.value(glyph.charcode).offset - subIndexBias);
      glyphData << (uint8_t)10; // callsubr;
    }
    else {
      ContourLimits contourLimits;
      glyphData = charString(glyph, true, iscff2, layers, currentx, currenty, contourLimits);
    }

    if (layers.size() > 0) {
      this->layers.insert(coloredGlyp.first, layers);
    }
  }


  QMap<uint16_t, QVector<Layer>>::iterator layIter = this->layers.begin();
  while (layIter != this->layers.end()) {
    auto glyph = glyphs.value(layIter.key());
    for (auto& layer : layIter.value()) {



      auto  prevlayIter = layIter;
      while (prevlayIter != this->layers.constBegin()) {
        prevlayIter--;
        bool foudEqual = false;
        for (auto& prevlayer : prevlayIter.value()) {
          if (prevlayer.charString == layer.charString) {
            layer.gid = prevlayer.gid;
            foudEqual = true;
            break;
          }
        }
        if (foudEqual) break;

      }
      if (layer.gid == 0) {
        QByteArray glyphArray;
        layer.gid = glyphs.lastKey() + 1;
        glyphs.insert(layer.gid, glyph);

        if (iscff2) {
          glyphArray = layer.charString;
        }
        else {
          int_to_cff2(glyphArray, toInt(glyph->width)); // width
          glyphArray.append(layer.charString);
          glyphArray << (uint8_t)14; //  endchar
        }

        offsets.append(offset);
        offset += glyphArray.size();
        objectData.append(glyphArray);
      }

    }
    layIter++;

  }



  offsets.append(offset);

  uint maxOffset = offset;

  int offSize;

  if (maxOffset < exp2(8)) {
    offSize = 1;
  }
  else if (maxOffset < exp2(16)) {
    offSize = 2;
  }
  else if (maxOffset < exp2(24)) {
    offSize = 3;
  }
  else {
    offSize = 4;
  }

  QByteArray data;
  if (iscff2) {
    data << (uint32_t)(glyphs.lastKey() + 1);
  }
  else {
    data << (uint16_t)(glyphs.lastKey() + 1);
  }

  data << (uint8_t)offSize;

  for (auto offset : offsets) {
    if (offSize == 1) {
      data << (uint8_t)offset;
    }
    else if (offSize == 2) {
      data << (uint16_t)offset;
    }
    else if (offSize == 3) {
      data << uint8_t(offset >> 16) << uint16_t(offset);
    }
    else {
      data << (uint32_t)offset;
    }
  }

  data.append(objectData);


  return data;
}

QByteArray ToOpenType::cff() {

  QByteArray charStringsIndex = charStrings(false);

  // Private DICT Data
  QByteArray privateDict;

  int_to_cff2(privateDict, 200);
  privateDict << (uint8_t)20; // defaultWidthX

  QByteArray subrsOffset;
  int_to_cff2(subrsOffset, privateDict.size() + 2);

  if (subrsOffset.size() != 1) {
    throw new std::runtime_error("Change offset");
  }


  privateDict.append(subrsOffset);
  privateDict << (uint8_t)19; // Subrs Operator

  //privateDict.append(subrs);

  QByteArray charset;
  charset << (uint8_t)0; //format




  //String Index
  QByteArray stringIndex;
  stringIndex << (uint16_t)(6 + glyphs.lastKey()); //count
  stringIndex << (uint8_t)4; //offSize
  stringIndex << (uint32_t)1; // offset

  QByteArray stringIndexData;

  int lastSid = 391;

  QString version = QString::number(globalValues.major) + "." + QString::number(globalValues.minor);

  stringIndexData.append(version.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.fullName().toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.familyName.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.subFamilyName.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.Copyright.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.License.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  for (int i = 1; i <= glyphs.lastKey(); i++) {
    stringIndexData.append(QString("Glyph%1").arg(i).toLatin1());
    stringIndex << (uint32_t)(1 + stringIndexData.size());
    charset << (uint16_t)lastSid;
    lastSid++;
  }

  int lastOffset = 1 + stringIndexData.size();
  if (lastOffset > std::numeric_limits<uint32_t>::max()) {
    throw new std::runtime_error("Increase offset size");
  }

  stringIndex.append(stringIndexData);

  //Name index
  QByteArray nameIndex;


  nameIndex << (uint16_t)1; //count
  nameIndex << (uint8_t)1; //offSize
  nameIndex << (uint8_t)1; // offset



  QByteArray nameIndexData = QString("DigitalKhattQuranic").toLatin1();

  nameIndex << (uint8_t)(1 + nameIndexData.size()); //last offset

  nameIndex.append(nameIndexData);

  int offSize = 1;

  //Top DICT INDEX
  QByteArray topDictIndex;
  topDictIndex << (uint16_t)1; //count
  topDictIndex << (uint8_t)offSize; //offSize
  topDictIndex << (uint8_t)1; // offset

  QByteArray topDicData;

  //Version
  int_to_cff2(topDicData, 391);
  topDicData << (uint8_t)0;

  //FullName
  int_to_cff2(topDicData, 392);
  topDicData << (uint8_t)2;

  //FamilyName
  int_to_cff2(topDicData, 393);
  topDicData << (uint8_t)3;

  //Weight
  int_to_cff2(topDicData, 394);
  topDicData << (uint8_t)4;

  //Copyright
  int_to_cff2(topDicData, 395);
  topDicData << (uint8_t)12 << (uint8_t)0;

  //Notice
  int_to_cff2(topDicData, 396);
  topDicData << (uint8_t)1;

  //charset
  topDicData << (uint8_t)29;
  int offsetCharSetPos = topDictIndex.size() + topDicData.size() + offSize;
  topDicData << (int32_t)0 << (uint8_t)15;

  topDicData << (uint8_t)29;
  int offsetCharStringPos = topDictIndex.size() + topDicData.size() + offSize;
  topDicData << (int32_t)0 << (uint8_t)17;

  topDicData << (int8_t)29;
  int offsetPrivatePos = topDictIndex.size() + topDicData.size() + offSize;
  topDicData << (int32_t)0 << (int8_t)29 << (int32_t)0 << (uint8_t)18;

  lastOffset = topDicData.size() + 1;
  if (lastOffset > 255) {
    throw new std::runtime_error("Increase offset size");
  }
  topDictIndex << (uint8_t)lastOffset; // offset
  topDictIndex.append(topDicData);



  QByteArray globalSubrsIndex;
  globalSubrsIndex << (uint16_t)0;


  QByteArray data;

  //Header
  data << (uint8_t)1; // majorVersion
  data << (uint8_t)0; // minorVersion
  data << (uint8_t)4; // headerSize
  data << (uint8_t)4; // offSize

  int charSetOffset = data.size() + nameIndex.size() + topDictIndex.size() + stringIndex.size() + globalSubrsIndex.size();
  if (charSetOffset > std::numeric_limits<int32_t>::max()) {
    throw new std::runtime_error("Increase offset size");
  }
  QByteArray charSetOffsetArray;
  charSetOffsetArray << (int32_t)charSetOffset;
  topDictIndex.replace(offsetCharSetPos, charSetOffsetArray.size(), charSetOffsetArray);

  int offsetCharStrings = charSetOffset + charset.size();
  if (offsetCharStrings > std::numeric_limits<int32_t>::max()) {
    throw new std::runtime_error("Increase offset size");
  }
  QByteArray offsetCharString;
  offsetCharString << (int32_t)offsetCharStrings;
  topDictIndex.replace(offsetCharStringPos, offsetCharString.size(), offsetCharString);

  int offsetPrivate = offsetCharStrings + charStringsIndex.size();
  if (offsetPrivate > std::numeric_limits<int32_t>::max()) {
    throw new std::runtime_error("Increase offset size");
  }

  QByteArray offsetPrivateByte;
  offsetPrivateByte << (int32_t)privateDict.size() << (int8_t)29 << (int32_t)offsetPrivate;
  topDictIndex.replace(offsetPrivatePos, offsetPrivateByte.size(), offsetPrivateByte);


  data.append(nameIndex);
  data.append(topDictIndex);
  data.append(stringIndex);
  data.append(globalSubrsIndex);
  data.append(charset);
  data.append(charStringsIndex);
  data.append(privateDict);
  data.append(getSubrs());


  return data;

}
QByteArray ToOpenType::getPrivateDictCff2(int* size) {

  QByteArray privateDict;

  //int_to_cff2(privateDict, 0);
  //privateDict << 22; //vsindex

  QByteArray subrsOffset;
  int_to_cff2(subrsOffset, privateDict.size() + 2);

  if (subrsOffset.size() != 1) {
    throw new std::runtime_error("Change offset");
  }

  privateDict.append(subrsOffset);
  privateDict << (uint8_t)19; // Subrs Operator

  *size = privateDict.size();

  privateDict.append(getSubrs());

  return privateDict;
}
QByteArray ToOpenType::cff2() {


  QByteArray charStr = charStrings(true);



  //Font DICT index
  QByteArray fontDic;


  fontDic << (uint32_t)1; //count
  fontDic << (uint8_t)1; //offSize
  fontDic << (uint8_t)1; // offset
  fontDic << (uint8_t)12; // offset
  fontDic << (int8_t)29 << (int32_t)0 << (int8_t)29 << (int32_t)0 << (uint8_t)18; //Private DICT size and offset

  QByteArray topDict;
  topDict << (int8_t)29 << (int32_t)0 << (uint8_t)12 << (uint8_t)36; //FDArray operator  
  topDict << (int8_t)29 << (int32_t)0 << (uint8_t)17; // CharStrings operator
  topDict << (int8_t)29 << (int32_t)0 << (uint8_t)24; // vstore

  QByteArray globalSubrIndex;
  globalSubrIndex << (int32_t)0; // majorVersion

  QByteArray ret;

  ret << (uint8_t)2; // majorVersion
  ret << (uint8_t)0; // minorVersion
  ret << (uint8_t)5; // headerSize
  ret << (uint16_t)topDict.size(); // topDictLength
  int topDictOffset = ret.size();
  ret.append(topDict);
  ret.append(globalSubrIndex);
  int variationStoreOffset = ret.size();
  ret.append(CFF2VariationStore());
  int fontDicOffset = ret.size();
  ret.append(fontDic);
  int charStrOffset = ret.size();
  ret.append(charStr);
  int privateDictSize;
  ret.append(getPrivateDictCff2(&privateDictSize));

  QByteArray offsetData;
  offsetData << (int32_t)fontDicOffset;
  ret.replace(topDictOffset + 1, offsetData.size(), offsetData);

  offsetData.clear();
  offsetData << (int32_t)charStrOffset;
  ret.replace(topDictOffset + 8, offsetData.size(), offsetData);

  offsetData.clear();
  offsetData << (int32_t)variationStoreOffset;
  ret.replace(topDictOffset + 14, offsetData.size(), offsetData);

  offsetData.clear();
  offsetData << (int32_t)privateDictSize << (int8_t)29 << (int32_t)(charStrOffset + charStr.size());
  ret.replace(charStrOffset - 10, offsetData.size(), offsetData);


  return ret;
}
//https://learn.microsoft.com/en-us/typography/opentype/otspec190/cff2charstr
//Table 3 Operand Encoding
void ToOpenType::int_to_cff2(QByteArray& cff, int val) {

  if (val >= -107 && val <= 107)
    cff << (int8_t)(val + 139);
  else if (val >= 108 && val <= 1131) {
    val -= 108;
    cff << (int8_t)((val >> 8) + 247);
    cff << (int8_t)val;
  }
  else if (val >= -1131 && val <= -108) {
    val = -val;
    val -= 108;
    cff << (int8_t)((val >> 8) + 251);
    cff << (int8_t)(val);
  }
  else if (val >= -32768 && val < 32768) {
    cff << (int8_t)28;
    cff << (int16_t)val;
  }
  else {
    cff << (int8_t)29;
    cff << (int32_t)val;
  }
}
void ToOpenType::fixed_to_cff2(QByteArray& cff, double val) {
  /*
  int v = roundf(val * 65536.f);
  cff << (uint8_t)255;
  cff << (int32_t)v;*/

  double intpart;  
  if (std::modf(val, &intpart) == 0.0) {
    int_to_cff2(cff, val);
  }
  else {
    int v = roundf(val * 65536.f);
    cff << (uint8_t)255;
    cff << (int32_t)v;
  }
}

QByteArray ToOpenType::dsig() {
  QByteArray data;

  data << (uint32_t)1; //version
  data << (uint16_t)0; //numSignatures
  data << (uint16_t)1; //flags

  return data;
}

QByteArray ToOpenType::getSubrs() {

  uint maxOffset = subrs.size() + 1;
  int count = subrOffsets.size();

  if (count < 1240) {
    if (subIndexBias != 107) throw new std::runtime_error("Invalid bias");
  }
  else  if (count < 33900) {
    if (subIndexBias != 1131) throw new std::runtime_error("Invalid bias");
  }
  else if (subIndexBias != 32768) {
    throw new std::runtime_error("Invalid bias");
  }

  subrOffsets.append(maxOffset);

  int offSize;

  if (maxOffset < exp2(8)) {
    offSize = 1;
  }
  else if (maxOffset < exp2(16)) {
    offSize = 2;
  }
  else if (maxOffset < exp2(24)) {
    offSize = 3;
  }
  else {
    offSize = 4;
  }

  QByteArray data;
  if (isCff2) {
    data << (uint32_t)count;
  }
  else {
    data << (uint16_t)count;
  }

  data << (uint8_t)offSize;

  for (auto offset : subrOffsets) {
    if (offSize == 1) {
      data << (uint8_t)offset;
    }
    else if (offSize == 2) {
      data << (uint16_t)offset;
    }
    else if (offSize == 3) {
      data << uint8_t(offset >> 16) << uint16_t(offset);
    }
    else {
      data << (uint32_t)offset;
    }
  }

  data.append(subrs);


  return data;

}

void ToOpenType::generateComponents() {

  std::set<QString> components;

  for (auto& glyph : glyphs) {
    auto body = glyph->copiedPath;

    if (!body) continue;

    do {
      switch (body->type)
      {
      case mp_fill_code: {
        mp_fill_object* fillobject = (mp_fill_object*)body;
        if (fillobject->pre_script && strcmp(fillobject->pre_script, "begincomponent") == 0) {
          components.insert(QString(fillobject->post_script).split(",")[0]);
        }
      }
      }
      body = body->next;
    } while (body != nullptr);
  }

  for (auto& componentName : components) {
    GlyphVis& glyph = ot_layout->glyphs[componentName];
    double currentx = 0.0;
    double currenty = 0.0;

    if (glyph.getGlypfType() == GlyphType::GlyphTypeColored) continue;

    QVector<Layer> layers;
    QByteArray charStringArray = charString(glyph, false, this->isCff2, layers, currentx, currenty, ContourLimits{});
    if (!isCff2) {
      charStringArray << (uint8_t)11; // return
    }
    subrByGlyph.insert(glyph.charcode, { subrOffsets.size(),currentx,currenty });
    subrOffsets.append(subrs.size() + 1);
    subrs.append(charStringArray);
  }
}
QByteArray ToOpenType::fvar() {
  QByteArray data;

  int axisCount = 2;

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (uint16_t)16; // axesArrayOffset
  data << (uint16_t)2; // This field is permanently reserved. Set to 2.
  data << (uint16_t)axisCount; // axisCount
  data << (uint16_t)20; // axisSize
  data << (uint16_t)0; // instanceCount
  data << (uint16_t)12; // instanceSize : axisCount * sizeof(Fixed) + 4

  //VariationAxisRecord[0]
  data << (uint32_t)HB_TAG('L', 'T', 'A', 'T');
  data << getFixed(axisLimits.minLeft); // minValue
  data << getFixed(0); // defaultValue
  data << getFixed(axisLimits.maxLeft); // maxValue
  data << (uint16_t)0; // flags
  data << (uint16_t)LTATNameId; // axisNameID

  //VariationAxisRecord[1]
  data << (uint32_t)HB_TAG('R', 'T', 'A', 'T');
  data << getFixed(axisLimits.minRight); // minValue
  data << getFixed(0); // defaultValue
  data << getFixed(axisLimits.maxRight); // maxValue
  data << (uint16_t)0; // flags
  data << (uint16_t)RTATNameId; // axisNameID



  return data;
}
QByteArray ToOpenType::JTST() {
  return ot_layout->JTST();
}


QByteArray ToOpenType::STAT() {

  QByteArray data;

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)2; // minorVersion
  data << (uint16_t)8; // designAxisSize
  data << (uint16_t)2; // designAxisCount
  data << (uint32_t)20; // designAxesOffset
  data << (uint16_t)0; // axisValueCount
  data << (uint32_t)0; // offsetToAxisValueOffsets
  data << (uint16_t)2; // elidedFallbackNameID

  data << (uint32_t)HB_TAG('L', 'T', 'A', 'T'); //axisTag
  data << (uint16_t)LTATNameId; // axisNameID
  data << (uint16_t)0; // axisOrdering

  data << (uint32_t)HB_TAG('R', 'T', 'A', 'T'); //axisTag
  data << (uint16_t)RTATNameId; // axisNameID
  data << (uint16_t)1; // axisOrdering

  return data;

}



QByteArray ToOpenType::CFF2VariationStore() {

  QByteArray itemVariationStore;
  QByteArray ItemVariationDatas;

  int variationRegionListOffset = 2 /* format*/ + 4 /*variationRegionListOffset*/ + 2 /*itemVariationDataCount*/ + 4 * subRegions.size() /* itemVariationDataOffsets[itemVariationDataCount]*/;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)variationRegionListOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)(subRegions.size()); //itemVariationDataCount

  QByteArray variationRegionList = getVariationRegionList();

  int itemVariationDataOffsets = variationRegionListOffset + variationRegionList.size();

  for (auto& subRegion : subRegions) {

    itemVariationStore << (uint32_t)(itemVariationDataOffsets); //itemVariationDataOffsets[itemVariationDataCount]

    QByteArray ItemVariationData;
    //subtable
    ItemVariationData << (uint16_t)0; //itemCount
    ItemVariationData << (uint16_t)0; //shortDeltaCount
    ItemVariationData << (uint16_t)(subRegion.size()); //regionIndexCount
    for (auto index : subRegion) {
      ItemVariationData << (uint16_t)index; //regionIndexes
    }

    itemVariationDataOffsets += ItemVariationData.size();

    ItemVariationDatas.append(ItemVariationData);

  }

  itemVariationStore.append(variationRegionList);
  itemVariationStore.append(ItemVariationDatas);

  QByteArray varStore;

  varStore << (uint16_t)itemVariationStore.size();
  varStore.append(itemVariationStore);

  return varStore;

}

QByteArray ToOpenType::MVAR() {

  int valueRecordCount = 0;

  QByteArray itemVariationStore;
  QByteArray ValueRecordArray;
  QByteArray itemVarData;
  QByteArray deltaSets;

  ValueRecordArray << (uint32_t)HB_TAG('h', 'a', 's', 'c'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('h', 'd', 's', 'c'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('h', 'l', 'g', 'p'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('h', 'c', 'l', 'a'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('h', 'c', 'l', 'd'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('u', 'n', 'd', 's'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;

  ValueRecordArray << (uint32_t)HB_TAG('u', 'n', 'd', 'o'); // valueTag
  ValueRecordArray << (uint16_t)0; // deltaSetOuterIndex
  ValueRecordArray << (uint16_t)valueRecordCount++; // deltaSetInnerIndex
  deltaSets << (uint16_t)0 << (uint16_t)0 << (uint16_t)0 << (uint16_t)0;


  itemVarData << (uint16_t)valueRecordCount; //itemCount
  itemVarData << (uint16_t)4; //shortDeltaCount
  itemVarData << (uint16_t)4; //regionIndexCount
  for (int i = 0; i < 4; i++) {
    itemVarData << (uint16_t)i; //regionIndexes
  }
  itemVarData.append(deltaSets);


  auto itemVariationDataCount = 1;
  auto startOffset = 8 + 4 * itemVariationDataCount;
  QByteArray variationRegionList = getVariationRegionList();

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)startOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)itemVariationDataCount; //itemVariationDataCount
  itemVariationStore << (uint32_t)(startOffset + variationRegionList.size()); //itemVariationDataOffsets[0]  
  itemVariationStore.append(variationRegionList);
  itemVariationStore.append(itemVarData);


  //MVAR
  QByteArray data;

  data << (uint16_t)1; //majorVersion
  data << (uint16_t)0; //minorVersion
  data << (uint16_t)0; //reserved
  data << (uint16_t)8; //valueRecordSize
  data << (uint16_t)valueRecordCount; //valueRecordCount
  data << (uint16_t)(12 + ValueRecordArray.size()); //itemVariationStoreOffset
  data.append(ValueRecordArray);
  data.append(itemVariationStore);

  return data;
}

QByteArray ToOpenType::HVAR() {

  int glyphCount = glyphs.lastKey() + 1;

  std::vector<std::map<std::vector<int>, int>> deltaSets;
  std::map<int, std::pair<int, int>> advaceLookup;
  std::map<int, std::pair<int, int>> lsbLookup;

  deltaSets.resize(subRegions.size());

  std::vector<int> defaultValue;
  defaultValue.resize(4);

  deltaSets[0].insert({ defaultValue ,0 });

  auto defaultPos = std::make_pair(0, 0);

  for (int i = 0; i < glyphCount; i++) {

    if (glyphs.contains(i)) {

      auto& glyph = *glyphs.value(i);

      const auto& ff = ot_layout->expandableGlyphs.find(glyph.name);

      if (ff != ot_layout->expandableGlyphs.end()) {

        DefaultDelta advanceDelta;
        DefaultDelta lsbDelta;

        auto& jj = ff->second;
        if (jj.maxLeft != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = jj.maxLeft;
          parameters.righttatweel = 0.0;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.maxLeft = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.maxLeft = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

        }
        if (jj.minLeft != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = jj.minLeft;
          parameters.righttatweel = 0.0;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.minLeft = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.minLeft = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

        }
        if (jj.maxRight != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = 0.0;
          parameters.righttatweel = jj.maxRight;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.maxRight = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.maxRight = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

        }
        if (jj.minRight != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = 0.0;
          parameters.righttatweel = jj.minRight;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.minRight = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.minRight = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

        }

        auto advPos = getDeltaSetEntry(advanceDelta, jj, deltaSets);
        auto lsbPos = getDeltaSetEntry(lsbDelta, jj, deltaSets);

        advaceLookup.insert({ i,advPos });
        lsbLookup.insert({ i,lsbPos });
      }
    }
  }

  QByteArray itemVariationStore = getItemVariationStore(deltaSets);

  QByteArray advanceMapping;
  advanceMapping << (uint16_t)0x3F; //entryFormat : 2 byte for outer, 2 byte for inner
  advanceMapping << (uint16_t)glyphCount; //entryFormat : 1 byte for outer, 2 byte for inner
  for (int i = 0; i < glyphCount; i++) {
    const auto& ff = advaceLookup.find(i);
    if (ff == advaceLookup.end()) {
      advanceMapping << (uint16_t)defaultPos.first;
      advanceMapping << (uint16_t)defaultPos.second;
    }
    else {
      advanceMapping << (uint16_t)ff->second.first;
      advanceMapping << (uint16_t)ff->second.second;
    }

  }

  QByteArray lsbMapping;
  lsbMapping << (uint16_t)0x3F; //entryFormat : 2 byte for outer, 2 byte for inner
  lsbMapping << (uint16_t)glyphCount; //entryFormat : 1 byte for outer, 2 byte for inner
  for (int i = 0; i < glyphCount; i++) {
    const auto& ff = lsbLookup.find(i);
    if (ff == lsbLookup.end()) {
      lsbMapping << (uint16_t)defaultPos.first;
      lsbMapping << (uint16_t)defaultPos.second;
    }
    else {
      lsbMapping << (uint16_t)ff->second.first;
      lsbMapping << (uint16_t)ff->second.second;
    }
  }


  //hvar
  QByteArray data;

  data << (uint16_t)1; //majorVersion
  data << (uint16_t)0; //minorVersion
  data << (uint32_t)20; //itemVariationStoreOffset
  data << (uint32_t)(20 + itemVariationStore.size()); //advanceWidthMappingOffset
  data << (uint32_t)(20 + itemVariationStore.size() + advanceMapping.size()); //lsbMappingOffset
  //data << (uint32_t)(20 + itemVariationStore.size() + advanceMapping.size() + lsbMapping.size()); //rsbMappingOffset
  data << (uint32_t)0; //rsbMappingOffset
  data.append(itemVariationStore);
  data.append(advanceMapping);
  data.append(lsbMapping);
  //data.append(lsbMapping);

  return data;

}

QByteArray ToOpenType::HVAR_Old() {

  struct EntryValue {
    int32_t value1 = 0;
    int32_t value2 = 0;
    int32_t value3 = 0;
    int32_t value4 = 0;
  };

  std::unordered_map<uint32_t, EntryValue> lsbs;

  QByteArray variationRegionList = getVariationRegionList();
  QByteArray aWidthVariationData;
  QByteArray lsbVariationData;

  int glyphCount = glyphs.lastKey() + 1;

  //subtable 0
  aWidthVariationData << (uint16_t)glyphCount; //itemCount
  aWidthVariationData << (uint16_t)4; //shortDeltaCount
  aWidthVariationData << (uint16_t)4; //regionIndexCount
  for (int i = 0; i < 4; i++) {
    aWidthVariationData << (uint16_t)i; //regionIndexes
  }
  //subtable 1
  lsbVariationData << (uint16_t)glyphCount; //itemCount
  lsbVariationData << (uint16_t)4; //shortDeltaCount
  lsbVariationData << (uint16_t)4; //regionIndexCount
  for (int i = 0; i < 4; i++) {
    lsbVariationData << (uint16_t)i; //regionIndexes
  }

  for (int i = 0; i < glyphCount; i++) {
    QByteArray glyphArray;

    bool entryExist = false;

    if (glyphs.contains(i)) {
      auto& glyph = *glyphs.value(i);
      /*
      if (!ot_layout->glyphs.contains(it.first)) {
        throw new std::runtime_error("Glyph name "+ it.first.toStdString() + " does not exist");
      }*/

      const auto& ff = ot_layout->expandableGlyphs.find(glyph.name);

      DefaultDelta advanceDelta;
      DefaultDelta lsbDelta;

      if (ff != ot_layout->expandableGlyphs.end()) {
        entryExist = true;
        auto& jj = ff->second;
        if (jj.maxLeft != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = jj.maxLeft;
          parameters.righttatweel = 0.0;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.maxLeft = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.maxLeft = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

          aWidthVariationData << (uint16_t)(toInt(alternate->width) - toInt(glyph.width));
          lsbVariationData << (uint16_t)(toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx));

        }
        else {
          aWidthVariationData << (uint16_t)0;
          lsbVariationData << (uint16_t)0;
        }
        if (jj.minLeft != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = jj.minLeft;
          parameters.righttatweel = 0.0;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.minLeft = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.minLeft = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

          aWidthVariationData << (uint16_t)(toInt(alternate->width) - toInt(glyph.width));
          lsbVariationData << (uint16_t)(toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx));

        }
        else {
          aWidthVariationData << (uint16_t)0;
          lsbVariationData << (uint16_t)0;
        }
        if (jj.maxRight != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = 0.0;
          parameters.righttatweel = jj.maxRight;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.maxRight = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.maxRight = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

          aWidthVariationData << (uint16_t)(toInt(alternate->width) - toInt(glyph.width));
          lsbVariationData << (uint16_t)(toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx));

        }
        else {
          aWidthVariationData << (uint16_t)0;
          lsbVariationData << (uint16_t)0;
        }
        if (jj.minRight != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = 0.0;
          parameters.righttatweel = jj.minRight;

          auto alternate = glyph.getAlternate(parameters);

          advanceDelta.minRight = toInt(alternate->width) - toInt(glyph.width);
          lsbDelta.minRight = toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx);

          aWidthVariationData << (uint16_t)(toInt(alternate->width) - toInt(glyph.width));
          lsbVariationData << (uint16_t)(toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx));

        }
        else {
          aWidthVariationData << (uint16_t)0;
          lsbVariationData << (uint16_t)0;
        }
      }
    }

    if (!entryExist) {
      aWidthVariationData << (uint16_t)0;
      aWidthVariationData << (uint16_t)0;
      aWidthVariationData << (uint16_t)0;
      aWidthVariationData << (uint16_t)0;

      lsbVariationData << (uint16_t)0;
      lsbVariationData << (uint16_t)0;
      lsbVariationData << (uint16_t)0;
      lsbVariationData << (uint16_t)0;
    }
  }

  QByteArray itemVariationStore;

  auto itemVariationDataCount = 2;
  auto startOffset = 8 + 4 * itemVariationDataCount;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)startOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)itemVariationDataCount; //itemVariationDataCount
  itemVariationStore << (uint32_t)(startOffset + variationRegionList.size()); //itemVariationDataOffsets[0]
  itemVariationStore << (uint32_t)(startOffset + variationRegionList.size() + aWidthVariationData.size()); //itemVariationDataOffsets[1]  
  itemVariationStore.append(variationRegionList);
  itemVariationStore.append(aWidthVariationData);
  itemVariationStore.append(lsbVariationData);

  QByteArray advanceMapping;
  advanceMapping << (uint16_t)0x3F; //entryFormat : 2 byte for outer, 2 byte for inner
  advanceMapping << (uint16_t)glyphCount; //entryFormat : 1 byte for outer, 2 byte for inner
  for (int i = 0; i < glyphCount; i++) {
    advanceMapping << (uint16_t)0;
    advanceMapping << (uint16_t)i;
  }

  QByteArray lsbMapping;
  lsbMapping << (uint16_t)0x3F; //entryFormat : 2 byte for outer, 2 byte for inner
  lsbMapping << (uint16_t)glyphCount; //entryFormat : 1 byte for outer, 2 byte for inner
  for (int i = 0; i < glyphCount; i++) {
    lsbMapping << (uint16_t)1;
    lsbMapping << (uint16_t)i;
  }


  //hvar
  QByteArray data;

  data << (uint16_t)1; //majorVersion
  data << (uint16_t)0; //minorVersion
  data << (uint32_t)20; //itemVariationStoreOffset
  data << (uint32_t)(20 + itemVariationStore.size()); //advanceWidthMappingOffset
  data << (uint32_t)(20 + itemVariationStore.size() + advanceMapping.size()); //lsbMappingOffset
  //data << (uint32_t)(20 + itemVariationStore.size() + advanceMapping.size() + lsbMapping.size()); //rsbMappingOffset
  data << (uint32_t)0; //rsbMappingOffset
  data.append(itemVariationStore);
  data.append(advanceMapping);
  data.append(lsbMapping);
  //data.append(lsbMapping);

  return data;

}


inline ToOpenType::PathLimits ToOpenType::PathLimits::next()
{
  PathLimits nn{};
  if (maxLeft != nullptr) {
    nn.maxLeft = maxLeft->next;
  }
  if (minLeft != nullptr) {
    nn.minLeft = minLeft->next;
  }
  if (maxRight != nullptr) {
    nn.maxRight = maxRight->next;
  }
  if (minRight != nullptr) {
    nn.minRight = minRight->next;
  }
  return nn;
}

inline ToOpenType::DeltaValues ToOpenType::PathLimits::x_coord() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->x_coord;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->x_coord;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->x_coord;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->x_coord;
    nn.active[3] = true;
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::y_coord() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->y_coord;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->y_coord;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->y_coord;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->y_coord;
    nn.active[3] = true;
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::left_x() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->left_x;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->left_x;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->left_x;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->left_x;
    nn.active[3] = true;
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::left_y() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->left_y;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->left_y;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->left_y;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->left_y;
    nn.active[3] = true;
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::right_x() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->right_x;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->right_x;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->right_x;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->right_x;
    nn.active[3] = true;
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::right_y() {
  DeltaValues nn{};
  if (maxLeft != nullptr) {
    nn.deltas[0] = maxLeft->right_y;
    nn.active[0] = true;
  }
  if (minLeft != nullptr) {
    nn.deltas[1] = minLeft->right_y;
    nn.active[1] = true;
  }
  if (maxRight != nullptr) {
    nn.deltas[2] = maxRight->right_y;
    nn.active[2] = true;
  }
  if (minRight != nullptr) {
    nn.deltas[3] = minRight->right_y;
    nn.active[3] = true;
  }
  return nn;
}

QByteArray ToOpenType::getVariationRegionList() {
  QByteArray variationRegionList;

  variationRegionList << (uint16_t)2; //axisCount
  variationRegionList << (uint16_t)regions.size(); //regionCount

  for (auto& region : regions) {
    for (auto& axis : region) {
      variationRegionList << axis.startCoord; // startCoord
      variationRegionList << axis.peakCoord; // peakCoord
      variationRegionList << axis.endCoord; // endCoord
    }
  }

  return variationRegionList;
}

std::pair<int, int> ToOpenType::getDeltaSetEntry(DefaultDelta delta, const ValueLimits& limits, std::vector<std::map<std::vector<int>, int>>& delatSets) {

  std::vector<int> deltaVector;

  int subregionIndex = 0;

  if (!uniformAxis) {
    deltaVector.push_back(delta.maxLeft);
    deltaVector.push_back(delta.minLeft);
    deltaVector.push_back(delta.maxRight);
    deltaVector.push_back(delta.minRight);
  }
  else {
    subregionIndex = regionSubtables[limits];
    if (limits.maxLeft != 0.0) {
      deltaVector.push_back(delta.maxLeft);
      if (limits.maxLeft != axisLimits.maxLeft) {
        deltaVector.push_back(delta.maxLeft);
        deltaVector.push_back(delta.maxLeft);
      }
    }
    if (limits.minLeft != 0.0) {
      deltaVector.push_back(delta.minLeft);
      if (limits.minLeft != axisLimits.minLeft) {
        deltaVector.push_back(delta.minLeft);
        deltaVector.push_back(delta.minLeft);
      }
    }
    if (limits.maxRight != 0.0) {
      deltaVector.push_back(delta.maxRight);
      if (limits.maxRight != axisLimits.maxRight) {
        deltaVector.push_back(delta.maxRight);
        deltaVector.push_back(delta.maxRight);
      }
    }
    if (limits.minRight != 0.0) {
      deltaVector.push_back(delta.minRight);
      if (limits.minRight != axisLimits.minRight) {
        deltaVector.push_back(delta.minRight);
        deltaVector.push_back(delta.minRight);
      }
    }

    //assert(deltaVector.size() == subRegions[subregionIndex].size());
  }

  auto& subRegionDataSet = delatSets[subregionIndex];

  const auto& it = subRegionDataSet.find(deltaVector);
  if (it != subRegionDataSet.end()) {
    return { subregionIndex,it->second };
  }
  else {
    int val = subRegionDataSet.size();
    subRegionDataSet.insert({ deltaVector ,val });
    return { subregionIndex,val };
  }
}
QByteArray ToOpenType::getItemVariationStore(const std::vector<std::map<std::vector<int>, int>>& delatSets) {

  if (delatSets.size() == 0) {
    return {};
  }

  QByteArray itemVariationStore;
  QByteArray ItemVariationDatas;

  int variationRegionListOffset = 2 /* format*/ + 4 /*variationRegionListOffset*/ + 2 /*itemVariationDataCount*/ + 4 * subRegions.size() /* itemVariationDataOffsets[itemVariationDataCount]*/;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)variationRegionListOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)(subRegions.size()); //itemVariationDataCount

  QByteArray variationRegionList = getVariationRegionList();

  int itemVariationDataOffsets = variationRegionListOffset + variationRegionList.size();

  for (int subregionIndex = 0; subregionIndex < subRegions.size(); subregionIndex++) {

    auto& subRegion = subRegions[subregionIndex];
    auto& defaultDeltaSet = delatSets[subregionIndex];

    itemVariationStore << (uint32_t)(itemVariationDataOffsets); //itemVariationDataOffsets[itemVariationDataCount]

    std::map<int, std::vector<int>> delatSets;
    for (auto& it : defaultDeltaSet) {
      delatSets.insert({ it.second,it.first });
    }

    QByteArray ItemVariationData;
    //subtable
    ItemVariationData << (uint16_t)delatSets.size(); //itemCount
    ItemVariationData << (uint16_t)subRegion.size(); //shortDeltaCount
    ItemVariationData << (uint16_t)(subRegion.size()); //regionIndexCount

    for (auto index : subRegion) {
      ItemVariationData << (uint16_t)index; //regionIndexes
    }
    for (auto& it : delatSets) {
      assert(subRegion.size() == it.second.size());
      for (auto value : it.second) {
        ItemVariationData << (uint16_t)value;
      }
    }

    itemVariationDataOffsets += ItemVariationData.size();

    ItemVariationDatas.append(ItemVariationData);

  }

  itemVariationStore.append(variationRegionList);
  itemVariationStore.append(ItemVariationDatas);

  return itemVariationStore;

}
