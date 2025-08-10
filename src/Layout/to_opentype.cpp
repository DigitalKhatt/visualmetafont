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
#include "metafont.h"


ToOpenType::ToOpenType(OtLayout* layout) :ot_layout{ layout }
{
  isComponentsEnabled = false;
  setAxes();
}
void ToOpenType::setAxes() {

  uniformAxis = ot_layout->isOTVar;
  regions.clear();
  glyphParametersByRegion.clear();
  GDEFDeltaSets.clear();
  regionIndexesIndexByGlyph.clear();

  if (!ot_layout->isOTVar) {
    axisCount = 0;
    isComponentsEnabled = false;
    return;
  }

  auto& axes = ot_layout->font->axes;

  this->axisCount = axes.size();

  if (axisCount == 0) {
    isComponentsEnabled = false;
    return;
  }

  int axisNameId = this->AxisNameId;
  for (auto axis : axes) {
    axisNameIds.push_back(axisNameId++);
  }

  auto getVariationRegion = [&regions = regions, axisCount = this->axisCount](unsigned int axisIndex, RegionAxisCoordinate coordinate) {

    VariationRegion region;

    for (int i = 0; i < axisCount; i++) {
      if (i == axisIndex) {
        region.push_back(coordinate);
      }
      else {
        region.push_back({ 0,0,0 });
      }
    }
    return region;
    };

  auto getAxisIndex = [&axes = axes](QString axisName) {
    for (int i = 0; i < axes.size(); i++) {
      auto& axis = axes[i];
      if (axis.name == axisName) {
        return i;
      }
    }
    return -1;
    };

  auto leftTatweelIndex = getAxisIndex("leftTatweel");
  auto rightTatweelIndex = getAxisIndex("rightTatweel");
  auto scaleXIndex = getAxisIndex("scaleX");

  if (scaleXIndex == -1) {
    isComponentsEnabled = true;
  }

  auto getRegionIndex = [&regions = regions, &glyphParametersByRegion = glyphParametersByRegion](VariationRegion region, GlyphParameters parameters) {
    int size = regions.size();
    for (int i = 0; i < size; i++) {
      if (regions[i] == region) return i;
    }

    regions.push_back(region);
    glyphParametersByRegion.push_back(parameters);
    return size;
    };

  auto getRegionIndexesIndex = [&regionIndexesArray = regionIndexesArray](std::vector<int> regionIndexes) {
    int size = regionIndexesArray.size();
    for (int i = 0; i < size; i++) {
      if (regionIndexesArray[i] == regionIndexes) return i;
    }

    regionIndexesArray.push_back(regionIndexes);
    return size;
    };


  for (auto it = ot_layout->glyphCodePerName.keyValueBegin(); it != ot_layout->glyphCodePerName.keyValueEnd(); ++it) {
    auto glyphName = it->first;
    auto glyphCode = it->second;
    auto* glyph = &ot_layout->glyphs[it->first];
    std::vector<int> regionIndexes;
    if (leftTatweelIndex != -1 || rightTatweelIndex != -1) {
      auto ff = ot_layout->expandableGlyphs.find(glyphName);
      if (ff != ot_layout->expandableGlyphs.end()) {
        auto& limits = ff->second;
        if (leftTatweelIndex != -1) {
          auto& leftAxis = axes[leftTatweelIndex];
          if (limits.maxLeft != 0.0) {
            if (limits.maxLeft > leftAxis.maxValue) {
              throw new std::runtime_error("maxLeft exceeds axis Limit");
            }
            else if (limits.maxLeft == leftAxis.maxValue) {
              VariationRegion region = getVariationRegion(leftTatweelIndex, { 0, getF2DOT14(1.0), getF2DOT14(1.0) });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.maxLeft }));
            }
            else {
              int16_t normalLimit = getF2DOT14(limits.maxLeft / leftAxis.maxValue);
              VariationRegion region = getVariationRegion(leftTatweelIndex, { 0, normalLimit, normalLimit });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.maxLeft }));
              region = getVariationRegion(leftTatweelIndex, { (short)(normalLimit + 1),getF2DOT14(1),getF2DOT14(1) });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.maxLeft }));
              region = getVariationRegion(leftTatweelIndex, { (short)(normalLimit + 1),(short)(normalLimit + 1),getF2DOT14(1) });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.maxLeft }));
            }
          }
          if (limits.minLeft != 0.0) {
            if (limits.minLeft < leftAxis.minValue) {
              throw new std::runtime_error("minLeft exceeds axis Limit");
            }
            else if (limits.minLeft == leftAxis.minValue) {
              VariationRegion region = getVariationRegion(leftTatweelIndex, { getF2DOT14(-1.0),getF2DOT14(-1.0),0 });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.minLeft }));
            }
            else {
              int16_t normalLimit = getF2DOT14(-limits.minLeft / leftAxis.minValue);
              VariationRegion region = getVariationRegion(leftTatweelIndex, { normalLimit,normalLimit,0 });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.minLeft }));
              region = getVariationRegion(leftTatweelIndex, { getF2DOT14(-1),(short)(normalLimit - 1),(short)(normalLimit - 1) });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.minLeft }));
              region = getVariationRegion(leftTatweelIndex, { getF2DOT14(-1),getF2DOT14(-1),(short)(normalLimit - 1) });
              regionIndexes.push_back(getRegionIndex(region, { .lefttatweel = limits.minLeft }));
            }
          }
        }
        if (rightTatweelIndex != -1) {
          auto& rightAxis = axes[rightTatweelIndex];
          if (limits.maxRight != 0.0) {
            if (limits.maxRight > limits.maxRight) {
              throw new std::runtime_error("maxRight exceeds axis Limit");
            }
            else if (limits.maxRight == rightAxis.maxValue) {
              VariationRegion region = getVariationRegion(rightTatweelIndex, { 0, getF2DOT14(1.0), getF2DOT14(1.0) });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.maxRight }));
            }
            else {
              int16_t normalLimit = getF2DOT14(limits.maxRight / rightAxis.maxValue);
              VariationRegion region = getVariationRegion(rightTatweelIndex, { 0, normalLimit, normalLimit });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.maxRight }));
              region = getVariationRegion(rightTatweelIndex, { (short)(normalLimit + 1),getF2DOT14(1),getF2DOT14(1) });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.maxRight }));
              region = getVariationRegion(rightTatweelIndex, { (short)(normalLimit + 1),(short)(normalLimit + 1),getF2DOT14(1) });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.maxRight }));
            }
          }
          if (limits.minRight != 0.0) {
            if (limits.minRight < rightAxis.minValue) {
              throw new std::runtime_error("minRight exceeds axis Limit");
            }
            else if (limits.minRight == rightAxis.minValue) {
              VariationRegion region = getVariationRegion(rightTatweelIndex, { getF2DOT14(-1.0),getF2DOT14(-1.0),0 });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.minRight }));
            }
            else {
              int16_t normalLimit = getF2DOT14(-limits.minRight / rightAxis.minValue);
              VariationRegion region = getVariationRegion(rightTatweelIndex, { normalLimit,normalLimit,0 });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.minRight }));
              region = getVariationRegion(rightTatweelIndex, { getF2DOT14(-1),(short)(normalLimit - 1),(short)(normalLimit - 1) });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.minRight }));
              region = getVariationRegion(rightTatweelIndex, { getF2DOT14(-1),getF2DOT14(-1),(short)(normalLimit - 1) });
              regionIndexes.push_back(getRegionIndex(region, { .righttatweel = limits.minRight }));
            }
          }
        }
      }
    }
    if (scaleXIndex != -1 /* && !glyphName.contains(".added_")*/) {
      bool includeglyph = true;
      if (glyphName.contains(".added_")) {        
        if (glyph->charlt > 3 || glyph->charrt > 0) {
          includeglyph = false;
        }
      }
      if (includeglyph) {
        auto& scaleXAxis = axes[scaleXIndex];
        VariationRegion region = getVariationRegion(scaleXIndex, { 0, getF2DOT14(1.0), getF2DOT14(1.0) });
        regionIndexes.push_back(getRegionIndex(region, { .scalex = scaleXAxis.maxValue }));
        region = getVariationRegion(scaleXIndex, { getF2DOT14(-1.0),getF2DOT14(-1.0),0 });
        regionIndexes.push_back(getRegionIndex(region, { .scalex = scaleXAxis.minValue }));
      }      
    }
    auto regionIndexesIndex = getRegionIndexesIndex(regionIndexes);
    regionIndexesIndexByGlyph.insert({ glyphName, regionIndexesIndex });
  }

  GDEFDeltaSets.resize(regionIndexesArray.size());

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
  globalValues.familyName = ot_layout->font->familyName();
  globalValues.subFamilyName = "Regular";
  globalValues.Copyright = ot_layout->font->copyright();
  globalValues.License = R"license(This Font Software is licensed under the SIL Open Font License, Version 1.1. This license is available with a FAQ at: http://scripts.sil.org/OFL)license";
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

  setAxes();

  ot_layout->loadLookupFile(lokkupsFileName);


  glyphs.clear();
  for (auto it = ot_layout->glyphCodePerName.keyValueBegin(); it != ot_layout->glyphCodePerName.keyValueEnd(); ++it) {
    glyphs.insert(it->second, &ot_layout->glyphs[it->first]);
  }

  initiliazeGlobals();

  QVector<Table> tables;
  QByteArray cffArray;

  // TODO blend component to support OpenType variations
  if (isComponentsEnabled) {
    generateComponents();
  }  

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
    auto data = fvar();
    if (data.size() > 0) {
      tables.append({ data,HB_TAG('f','v','a','r') });
    }
    data = HVAR();
    if (data.size() > 0) {
      tables.append({ data,HB_TAG('H','V','A','R') });
    }

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
  names.append(Name{ 8,"DigitalKhatt" });
  //names.append(Name{ 9,"DigitalKhatt" }); // Designer
  //names.append(Name{ 10,"This font is the OpenType version of the MetaFont-designed parametric font used in the DigitalKhatt Arabic typesetter which justifies text dynamically using curvilinear expansion of letters" });
  names.append(Name{ 11,"https://github.com/DigitalKhatt" });
  names.append(Name{ 12,"https://github.com/DigitalKhatt" });
  names.append(Name{ 13,globalValues.License });
  names.append(Name{ 14,"http://scripts.sil.org/OFL" });

  names.append(Name{ 16,globalValues.familyName });
  names.append(Name{ 17,globalValues.subFamilyName });
  names.append(Name{ 19,"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" });

  names.append(Name{ 21,globalValues.familyName });
  names.append(Name{ 22,globalValues.subFamilyName });

  for (int i = 0; i < axisCount; i++) {
    names.append(Name{ (ushort)axisNameIds[i] ,ot_layout->font->axes[i].name });
  }


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

    QByteArray stringData;

    glyphs[0]->name = ".notdef";

    for (int i = 0; i < glyphCount; i++) {
      QByteArray glyphArray;

      if (glyphs.contains(i)) {
        data << (uint16_t)(index + 258);
        index++;
        auto glyph = glyphs.value(i);
        auto name = glyph->name.toLatin1();
        stringData << (uint8_t)name.size();
        stringData.append(name);
      }
      else {
        data << (uint16_t)(258);//  .notdef;
      }
    }
    data.append(stringData);   
  }

  return data;
}
void ToOpenType::dumpPath(GlyphVis& glyph, QByteArray& data, mp_graphic_object** body, double& currentx, double& currenty, PathLimits& pathLimits, ContourLimits& contourLimits) {

  mp_fill_object* fill = (mp_fill_object*)*body;
  if (isComponentsEnabled) {
    if (fill->pre_script && strcmp(fill->pre_script, "begincomponent") == 0) {
      auto comp = QString(fill->post_script).split(",");
      QString name = comp[0];
      GlyphVis& compGlyph = ot_layout->glyphs[name];

      if (subrByGlyph.contains (compGlyph.charcode))
        {

          auto subrByGlyphInfo = subrByGlyph.value (compGlyph.charcode);

          const auto &ff = regionIndexesIndexByGlyph.find (glyph.name);

          int mainGlyphregionIndexesArrayIndex = -1;

          if (ff != regionIndexesIndexByGlyph.end ())
            {

              mainGlyphregionIndexesArrayIndex = ff->second;
            }

          if (subrByGlyphInfo.regionIndexesArrayIndex == -1
              || mainGlyphregionIndexesArrayIndex == subrByGlyphInfo.regionIndexesArrayIndex)
            {

              *body = (*body)->next;
              contourLimits.setNext ();
              while ((*body)->type != mp_stroked_code
                     || ((mp_stroked_object *)*body)->pre_script == nullptr
                     || strcmp (((mp_stroked_object *)*body)->pre_script,
                                "endcomponent"))
                {
                  *body = (*body)->next;
                  contourLimits.setNext ();
                }

              mp_stroked_object *stroked_ob = (mp_stroked_object *)*body;

              if (stroked_ob == nullptr || stroked_ob->path_p == nullptr) {
                  throw new std::runtime_error ("ERROR");
                }

              auto initPosX = stroked_ob->path_p->x_coord;
              auto initPosY = stroked_ob->path_p->y_coord;
              auto dx = initPosX - currentx;
              auto dy = initPosY - currenty;

              QByteArray deltaXArray;
              QByteArray deltaYArray;
              if (contourLimits.contours.size() > 0) {
                  PathLimits compPathLimits;
                  for (auto contour : contourLimits.contours)
                    {
                      stroked_ob = (mp_stroked_object *)contour;
                      if (stroked_ob == nullptr || stroked_ob->path_p == nullptr)
                        {
                          throw new std::runtime_error ("ERROR");
                        }
                      compPathLimits.currentx.deltas.push_back (
                          stroked_ob->path_p->x_coord);
                      compPathLimits.currenty.deltas.push_back (
                          stroked_ob->path_p->y_coord);
                    }
                  auto deltaX = compPathLimits.currentx - pathLimits.currentx - dx;
                  auto deltaY = compPathLimits.currenty - pathLimits.currenty - dy;
                  deltaXArray = blend (deltaX);
                  deltaYArray = blend (deltaY);

                  if (subrByGlyphInfo.pathlimits.currentx.deltas.size () > 0)
                    {
                      pathLimits.currentx = compPathLimits.currentx
                                            + subrByGlyphInfo.pathlimits.currentx;
                      pathLimits.currenty = compPathLimits.currenty
                                            + subrByGlyphInfo.pathlimits.currenty;
                    }
                  else
                    {
                      pathLimits.currentx
                          = compPathLimits.currentx + subrByGlyphInfo.lastx;
                      pathLimits.currenty
                          = compPathLimits.currenty + subrByGlyphInfo.lasty;
                    }
                }

              fixed_to_cff2(data, dx);
              data.append(deltaXArray);
              fixed_to_cff2(data, dy);
              data.append(deltaYArray);
              data << (uint8_t)21; // rmoveto;
              int_to_cff2(data, subrByGlyphInfo.offset - subIndexBias);
              data << (uint8_t)10; // callsubr;

              currentx = initPosX + subrByGlyphInfo.lastx;
              currenty = initPosY + subrByGlyphInfo.lasty;

              return;
            }


      }
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
    data.append(blend(delta));
  }

  defaultt = path->y_coord - currenty;
  fixed_to_cff2(data, defaultt);
  if (isCff2) {
    auto delta = pathLimits.y_coord() - pathLimits.currenty - defaultt;
    data.append(blend(delta));
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
      data.append(blend(deltax));
    }

    defaultt = p->right_y - p->y_coord;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto deltay = pLimits.right_y() - pLimits.y_coord() - defaultt;
      data.append(blend(deltay));
    }

    defaultt = q->left_x - p->right_x;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.left_x() - pLimits.right_x() - defaultt;
      data.append(blend(delta));
    }

    defaultt = q->left_y - p->right_y;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.left_y() - pLimits.right_y() - defaultt;
      data.append(blend(delta));
    }

    defaultt = q->x_coord - q->left_x;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.x_coord() - qLimits.left_x() - defaultt;
      data.append(blend(delta));
    }

    defaultt = q->y_coord - q->left_y;
    fixed_to_cff2(data, defaultt);
    if (isCff2) {
      auto delta = qLimits.y_coord() - qLimits.left_y() - defaultt;
      data.append(blend(delta));
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
QByteArray ToOpenType::charString(GlyphVis& glyph, bool colored, bool iscff2, QVector<Layer>& layers, double& currentx, double& currenty, ToOpenType::ContourLimits contourLimits, PathLimits& pathlimits) {

  auto body = glyph.copiedPath;
  QByteArray data;

  Color ayablue = Color{ 255,240,220,255 };

  for (auto contour : contourLimits.contours) {
    pathlimits.currentx.deltas.push_back(currentx);
    pathlimits.currenty.deltas.push_back(currenty);
  }

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {
        QByteArray layerArray;
        mp_fill_object* fillobject = (mp_fill_object*)body;

        if (!colored) {
          if (iscff2) {
            pathlimits.knots.clear();
            for (auto contour : contourLimits.contours) {
              if (contour != nullptr) {
                pathlimits.knots.push_back(((mp_fill_object*)contour)->path_p);
              }
              else {
                pathlimits.knots.push_back(nullptr);
              }
            }
          }

          dumpPath(glyph, layerArray, &body, currentx, currenty, pathlimits, contourLimits);

          data.append(layerArray);

        }
        else {
          Layer layer;
          QByteArray newGlyphArray;
          double tempx = 0;
          double tempy = 0;
          PathLimits tempPathLimits;
          dumpPath(glyph, newGlyphArray, &body, tempx, tempy, tempPathLimits, contourLimits);
          layer.charString = newGlyphArray;          

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
      contourLimits.setNext ();

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

      int regionIndexesArrayIndex = 0;

      if (subrByGlyph.contains(glyph.charcode)) {
        int_to_cff2(glyphData, subrByGlyph.value(glyph.charcode).offset - subIndexBias);
        glyphData << (uint8_t)10; // callsubr;
      }
      else {
        ContourLimits contourLimits;

        if (ot_layout->isOTVar) {


          const auto& ff = regionIndexesIndexByGlyph.find(glyph.name);

          if (ff != regionIndexesIndexByGlyph.end()) {

            regionIndexesArrayIndex = ff->second;

            auto& regionIndexes = regionIndexesArray[regionIndexesArrayIndex];

            for (auto regionIndex : regionIndexes) {
              auto& parameters = glyphParametersByRegion[regionIndex];
              auto alternate = glyph.getAlternate(parameters);
              contourLimits.contours.push_back(alternate->copiedPath);
            }
          }
        }
        QVector<Layer> layers;
        PathLimits pathlimits;
        glyphData = charString(glyph, false, iscff2, layers, currentx, currenty, contourLimits,pathlimits);
      }

      if (glyphData.size() == 0) {
        if (!iscff2) {
          int_to_cff2(glyphArray, toInt(glyph.width)); // width
          glyphArray << (uint8_t)14; //  endchar
        }
      }
      else {
        if (iscff2) {
          if (regionIndexesArrayIndex != 0) {
            int_to_cff2(glyphArray, toInt(regionIndexesArrayIndex)); // regionSubtable
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
      PathLimits pathlimits;
      glyphData = charString(glyph, true, iscff2, layers, currentx, currenty, contourLimits,pathlimits);
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

  if (subrs.size() == 0) {
    *size = 0;
    return privateDict;
  }


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

  QByteArray varStore = CFF2VariationStore();



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
  if (varStore.size() > 0) {
    topDict << (int8_t)29 << (int32_t)0 << (uint8_t)24; // vstore
  }
  

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
  ret.append(varStore);
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

  if (varStore.size() > 0) {
    offsetData.clear();
    offsetData << (int32_t)variationStoreOffset;
    ret.replace(topDictOffset + 14, offsetData.size(), offsetData);
  }

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

  if (subrs.size() == 0) {
    QByteArray data;
    if (isCff2) {
      data << (uint32_t)0;
    }
    else {
      data << (uint16_t)0;
    }
    return data;
  }

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

    ContourLimits contourLimits;

    int regionIndexesArrayIndex = -1;

    if (ot_layout->isOTVar) {


        const auto& ff = regionIndexesIndexByGlyph.find(glyph.name);

        if (ff != regionIndexesIndexByGlyph.end()) {

            regionIndexesArrayIndex = ff->second;

            auto& regionIndexes = regionIndexesArray[regionIndexesArrayIndex];

            for (auto regionIndex : regionIndexes) {
                auto& parameters = glyphParametersByRegion[regionIndex];
                auto alternate = glyph.getAlternate(parameters);
                contourLimits.contours.push_back(alternate->copiedPath);
            }
        }
    }

    QVector<Layer> layers;
    PathLimits pathlimits;
    QByteArray charStringArray = charString(glyph, false, this->isCff2, layers, currentx, currenty, contourLimits, pathlimits);
    if (!isCff2) {
      charStringArray << (uint8_t)11; // return
    }
    subrByGlyph.insert(glyph.charcode, { subrOffsets.size(),currentx,currenty,regionIndexesArrayIndex,pathlimits });
    subrOffsets.append(subrs.size() + 1);
    subrs.append(charStringArray);
  }
}
QByteArray ToOpenType::fvar() {
  QByteArray data;

  if (axisCount == 0) return data;

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (uint16_t)16; // axesArrayOffset
  data << (uint16_t)2; // This field is permanently reserved. Set to 2.
  data << (uint16_t)axisCount; // axisCount
  data << (uint16_t)20; // axisSize
  data << (uint16_t)0; // instanceCount
  data << (uint16_t)(axisCount * 4 + 4); // instanceSize : axisCount * sizeof(Fixed) + 4

  auto& axes = ot_layout->font->axes;

  for (int i = 0; i < axisCount; i++) {
    auto& axis = axes[i];
    data << (uint32_t)axis.axisTag;
    data << getFixed(axis.minValue); // minValue
    data << getFixed(axis.defaultValue); // defaultValue
    data << getFixed(axis.maxValue); // maxValue
    data << (uint16_t)0; // flags
    data << (uint16_t)axisNameIds[i]; // axisNameID
  }

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
  data << (uint16_t)axisCount; // designAxisCount
  data << (uint32_t)20; // designAxesOffset
  data << (uint16_t)0; // axisValueCount
  data << (uint32_t)0; // offsetToAxisValueOffsets
  data << (uint16_t)2; // elidedFallbackNameID

  auto& axes = ot_layout->font->axes;

  for (int i = 0; i < axisCount; i++) {
    auto& axis = axes[i];
    data << (uint32_t)axis.axisTag; //axisTag
    data << (uint16_t)axisNameIds[i]; // axisNameID
    data << (uint16_t)i; // axisOrdering
  }

  return data;

}



QByteArray ToOpenType::CFF2VariationStore() {

  if (regionIndexesArray.size() == 0) {
    return {};
  }

  QByteArray itemVariationStore;
  QByteArray ItemVariationDatas;

  int variationRegionListOffset = 2 /* format*/ + 4 /*variationRegionListOffset*/ + 2 /*itemVariationDataCount*/ + 4 * regionIndexesArray.size() /* itemVariationDataOffsets[itemVariationDataCount]*/;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)variationRegionListOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)(regionIndexesArray.size()); //itemVariationDataCount

  QByteArray variationRegionList = getVariationRegionList();

  int itemVariationDataOffsets = variationRegionListOffset + variationRegionList.size();

  for (auto& regionIndexes : regionIndexesArray) {

    itemVariationStore << (uint32_t)(itemVariationDataOffsets); //itemVariationDataOffsets[itemVariationDataCount]

    QByteArray ItemVariationData;
    //subtable
    ItemVariationData << (uint16_t)0; //itemCount
    ItemVariationData << (uint16_t)0; //shortDeltaCount
    ItemVariationData << (uint16_t)(regionIndexes.size()); //regionIndexCount
    for (auto index : regionIndexes) {
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

  QByteArray data;

  if (regionIndexesArray.size() == 0) {
    return data;
  }

  int glyphCount = glyphs.lastKey() + 1;

  std::vector<std::map<std::vector<int>, int>> deltaSets;
  std::map<int, std::pair<int, int>> advaceLookup;
  std::map<int, std::pair<int, int>> lsbLookup;

  deltaSets.resize(regionIndexesArray.size());

  std::vector<int> defaultValue;
  defaultValue.resize(axisCount * 2);

  deltaSets[0].insert({ defaultValue ,0 });

  auto defaultPos = std::make_pair(0, 0);

  for (int i = 0; i < glyphCount; i++) {

    if (glyphs.contains(i)) {

      auto& glyph = *glyphs.value(i);

      auto find = regionIndexesIndexByGlyph.find(glyph.name);
      if (find != regionIndexesIndexByGlyph.end()) {
        DefaultDelta advanceDelta;
        DefaultDelta lsbDelta;
        auto regionIndexesArrayIndex = find->second;
        auto& regionIndexes = regionIndexesArray[regionIndexesArrayIndex];

        for (auto regionIndex : regionIndexes) {
          auto& parameters = glyphParametersByRegion[regionIndex];
          auto alternate = glyph.getAlternate(parameters);
          advanceDelta.push_back(toInt(alternate->width) - toInt(glyph.width));
          lsbDelta.push_back(toInt(alternate->bbox.llx) - toInt(glyph.bbox.llx));
        }
        auto advPos = getDeltaSetEntry(advanceDelta, regionIndexesArrayIndex, deltaSets);
        auto lsbPos = getDeltaSetEntry(lsbDelta, regionIndexesArrayIndex, deltaSets);

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
  for (auto v : knots) {
    if (v != nullptr) {
      nn.knots.push_back(v->next);
    }
    else {
      nn.knots.push_back(nullptr);
    }
  }
  return nn;
}

inline ToOpenType::DeltaValues ToOpenType::PathLimits::x_coord() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->x_coord);
    }
    else {
      nn.deltas.push_back(0.0);
    }
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::y_coord() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->y_coord);
    }
    else {
      nn.deltas.push_back(0.0);
    }

  }

  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::left_x() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->left_x);
    }
    else {
      nn.deltas.push_back(0.0);
    }
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::left_y() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->left_y);
    }
    else {
      nn.deltas.push_back(0.0);
    }
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::right_x() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->right_x);
    }
    else {
      nn.deltas.push_back(0.0);
    }
  }
  return nn;
}
inline ToOpenType::DeltaValues ToOpenType::PathLimits::right_y() {
  DeltaValues nn{};
  for (auto v : knots) {
    if (v != nullptr) {
      nn.deltas.push_back(v->right_y);
    }
    else {
      nn.deltas.push_back(0.0);
    }
  }
  return nn;
}

QByteArray ToOpenType::getVariationRegionList() {
  QByteArray variationRegionList;

  variationRegionList << (uint16_t)axisCount; //axisCount
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

std::pair<int, int> ToOpenType::getDeltaSetEntry(DefaultDelta delta, const int subregionIndex, std::vector<std::map<std::vector<int>, int>>& delatSets) {

  auto& subRegionDataSet = delatSets[subregionIndex];

  const auto& it = subRegionDataSet.find(delta);
  if (it != subRegionDataSet.end()) {
    return { subregionIndex,it->second };
  }
  else {
    int val = subRegionDataSet.size();
    subRegionDataSet.insert({ delta ,val });
    return { subregionIndex,val };
  }
}
QByteArray ToOpenType::getItemVariationStore(const std::vector<std::map<std::vector<int>, int>>& delatSets) {

  if (delatSets.size() == 0) {
    return {};
  }

  QByteArray itemVariationStore;
  QByteArray ItemVariationDatas;

  int variationRegionListOffset = 2 /* format*/ + 4 /*variationRegionListOffset*/ + 2 /*itemVariationDataCount*/ + 4 * regionIndexesArray.size() /* itemVariationDataOffsets[itemVariationDataCount]*/;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)variationRegionListOffset; //variationRegionListOffset
  itemVariationStore << (uint16_t)(regionIndexesArray.size()); //itemVariationDataCount

  QByteArray variationRegionList = getVariationRegionList();

  int itemVariationDataOffsets = variationRegionListOffset + variationRegionList.size();

  for (int subregionIndex = 0; subregionIndex < regionIndexesArray.size(); subregionIndex++) {

    auto& subRegion = regionIndexesArray[subregionIndex];
    auto& defaultDeltaSet = delatSets[subregionIndex];

    itemVariationStore << (uint32_t)(itemVariationDataOffsets); //itemVariationDataOffsets[itemVariationDataCount]

    std::map<int, std::vector<int>> localDelatSets;
    for (auto& it : defaultDeltaSet) {
      localDelatSets.insert({ it.second,it.first });
    }

    assert(localDelatSets.size() == defaultDeltaSet.size());

    QByteArray ItemVariationData;
    //subtable
    ItemVariationData << (uint16_t)localDelatSets.size(); //itemCount
    ItemVariationData << (uint16_t)subRegion.size(); //shortDeltaCount
    ItemVariationData << (uint16_t)(subRegion.size()); //regionIndexCount

    for (auto index : subRegion) {
      ItemVariationData << (uint16_t)index; //regionIndexes
    }
    for (auto& it : localDelatSets) {
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

QByteArray ItemVariationStore::getVariationRegionList() {
  QByteArray variationRegionList;

  variationRegionList << (uint16_t)axisCount; //axisCount
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
QByteArray ItemVariationStore::getOpenTypeTable() {
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

void
ToOpenType::ContourLimits::setNext ()
{
  for (int i = 0; i < contours.size (); i++)
    {
      contours[i] = contours[i]->next;
    }
}
