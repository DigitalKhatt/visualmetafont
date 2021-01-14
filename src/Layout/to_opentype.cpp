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
#include "qfile.h"
#include <cmath>
#include "hb.h"
#include "qdatetime.h"
#include "OtLayout.h"
#include "GlyphVis.h"
#include "automedina/automedina.h"
#include <stdexcept>

extern "C"
{
#include "mplibps.h"
#include "mppsout.h"
}

ToOpenType::ToOpenType(OtLayout* layout) :ot_layout{ layout }
{
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
  globalValues.FullName = "DigitalKhatt Madina Quranic";
  globalValues.FamilyName = "DigitalKhatt Madina Quranic";
  globalValues.Weight = "Regular";
  globalValues.Copyright = "Copyright (c) 2020 Amine Anane";
  globalValues.License = R"license(This computer font is part of DigitalKhatt. It is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This font is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details. You should have received a copy of the GNU Affero General Public License along with DigitalKhatt. If not, see <https://www.gnu.org/licenses/>.

As a special exception, if you create a document which uses this font, and embed this font or unaltered portions of this font into the document, this font does not by itself cause the resulting document to be covered by the GNU General Public License. This exception does not however invalidate any other reasons why the document might be covered by the GNU General Public License. If you modify this font, you may extend this exception to your version of the font, but you are not obligated to do so. If you do not wish to do so, delete this exception statement from your version.)license";

}

int ToOpenType::toInt(double value) {
  if (value < 0)
    return floor(value);
  else
    return ceil(value);
}

void ToOpenType::setGIds() {


  QMap<quint16, quint16> newCodes;

  QMap<QString, quint16> glyphCodePerName;
  QMap<quint16, QString> glyphNamePerCode;
  QMap<quint16, quint16> unicodeToGlyphCode;
  QMap<quint16, OtLayout::GDEFClasses> glyphGlobalClasses;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> alternatePaths;
  std::unordered_map<int, std::unordered_map<GlyphParameters, GlyphVis*>> nojustalternatePaths;

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
      if (glyph->charcode != code) {
        int stop = 5;
      }
      glyph->charcode = newCode;
      newCode++;
    }
  }
  /*
  for(auto& glyph : ot_layout->glyphs){
    if(glyph.name != "notdef" && glyph.name != "null" ){
      newCodes.insert(glyph.charcode,newCode);
      glyph.charcode = newCode;
      newCode++;
    }
  }*/

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


  for (std::pair<int, std::unordered_map<GlyphParameters, GlyphVis*>> element : ot_layout->alternatePaths)
  {
    if (!newCodes.contains(element.first)) {
      throw new std::runtime_error(QString("Code %1 not found").arg(element.first).toStdString());
    }

    alternatePaths.insert({ newCodes.value(element.first), element.second });
  }

  for (std::pair<int, std::unordered_map<GlyphParameters, GlyphVis*>> element : ot_layout->nojustalternatePaths)
  {
    if (!newCodes.contains(element.first)) {
      throw new std::runtime_error(QString("Code %1 not found").arg(element.first).toStdString());
    }

    nojustalternatePaths.insert({ newCodes.value(element.first), element.second });
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
  ot_layout->alternatePaths = alternatePaths;
  ot_layout->nojustalternatePaths = nojustalternatePaths;
  ot_layout->glyphGlobalClasses = glyphGlobalClasses;



}

bool ToOpenType::GenerateFile(QString fileName) {

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

  ot_layout->loadLookupFile("lookups.json");

  //And new glyphhs
  auto gsubArray = gsub();

  setGIds();

  ot_layout->loadLookupFile("lookups.json");


  glyphs.clear();
  for (auto& glyph : ot_layout->glyphs) {
    glyphs.insert(glyph.charcode, &glyph);
  }

  initiliazeGlobals();

  QVector<Table> tables;
  QByteArray cffArray;

  setAyaGlyphPaths();

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
    tables.append({ MVAR(),HB_TAG('M','V','A','R') });
    tables.append({ STAT(),HB_TAG('S','T','A','T') });
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
  names.append(Name{ 1,globalValues.FamilyName });
  names.append(Name{ 2,globalValues.Weight });
  names.append(Name{ 3,"DigitalKhatt_V0.1_2020-09-18" });
  names.append(Name{ 4,globalValues.FullName });
  names.append(Name{ 5,"Version " + QString::number(globalValues.major) + "." + QString::number(globalValues.minor) });
  names.append(Name{ 6,"DigitalKhattQuranic" });
  //names.append(Name{ 7,"DigitalKhatt" });
  //names.append(Name{ 8,"DigitalKhatt" });
  names.append(Name{ 9,"Amine Anane" });
  names.append(Name{ 10,"This font is the OpenType version of the MetaFont-designed parametric font used in the DigitalKhatt Arabic typesetter which justifies text dynamically using curvilinear expansion of letters" });
  names.append(Name{ 11,"https://digitalkhatt.org/" });
  names.append(Name{ 12,"https://digitalkhatt.org/" });
  names.append(Name{ 13,globalValues.License });
  names.append(Name{ 14,"https://www.gnu.org/licenses/agpl-3.0.en.html" });

  names.append(Name{ 16,"DigitalKhatt Madina" });
  names.append(Name{ 17,"Quranic" });
  names.append(Name{ 19,"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" });

  names.append(Name{ 21,globalValues.FamilyName });
  names.append(Name{ 22,globalValues.Weight });

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

  data << (uint32_t)0x00030000; // version
  data << (uint32_t)0; // italicAngle
  data << (int16_t)-200; // underlinePosition
  data << (int16_t)globalValues.yStrikeoutSize; // underlineThickness
  data << (uint32_t)0; // isFixedPitch
  data << (uint32_t)0; // minMemType42
  data << (uint32_t)0; // maxMemType42
  data << (uint32_t)0; // minMemType1
  data << (uint32_t)0; // maxMemType1

  return data;
}
void ToOpenType::dumpPath(QByteArray& data, mp_fill_object* fill, double& currentx, double& currenty, PathLimits& pathLimits) {

  auto path = fill->path_p;

  mp_gr_knot p, q;
  PathLimits pLimits, qLimits;

  if (path == nullptr) return;

  double default = path->x_coord - currentx;
  fixed_to_cff2(data, default);
  if (isCff2) {
    auto delta = pathLimits.x_coord() - pathLimits.currentx - default;
    data.append(blend(delta));
  }

  default = path->y_coord - currenty;
  fixed_to_cff2(data, default);
  if (isCff2) {
    auto delta = pathLimits.y_coord() - pathLimits.currenty - default;
    data.append(blend(delta));
  }

  data << (uint8_t)21; // rmoveto;

  p = path;
  pLimits = pathLimits;
  do {
    q = p->next;
    qLimits = pLimits.next();

    default = p->right_x - p->x_coord;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto deltax = pLimits.right_x() - pLimits.x_coord() - default;
      data.append(blend(deltax));
    }

    default = p->right_y - p->y_coord;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto deltay = pLimits.right_y() - pLimits.y_coord() - default;
      data.append(blend(deltay));
    }

    default = q->left_x - p->right_x;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto delta = qLimits.left_x() - pLimits.right_x() - default;
      data.append(blend(delta));
    }

    default = q->left_y - p->right_y;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto delta = qLimits.left_y() - pLimits.right_y() - default;
      data.append(blend(delta));
    }

    default = q->x_coord - q->left_x;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto delta = qLimits.x_coord() - qLimits.left_x() - default;
      data.append(blend(delta));
    }

    default = q->y_coord - q->left_y;
    fixed_to_cff2(data, default);
    if (isCff2) {
      auto delta = qLimits.y_coord() - qLimits.left_y() - default;
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
QByteArray ToOpenType::charString(mp_graphic_object* body, bool iscff2, QVector<Layer>& layers, double& currentx, double& currenty, ToOpenType::ContourLimits contourLimits) {


  QByteArray data;

  Color ayablue = Color{ 255,240,220,255 };
  PathLimits pathlimits;

  if (body) {
    do {
      switch (body->type)
      {
      case mp_fill_code: {
        QByteArray layerArray;
        mp_fill_object* fillobject = (mp_fill_object*)body;


        if (iscff2) {
          pathlimits.maxLeft = contourLimits.maxLeft != nullptr ? ((mp_fill_object*)contourLimits.maxLeft)->path_p : nullptr;
          pathlimits.minLeft = contourLimits.minLeft != nullptr ? ((mp_fill_object*)contourLimits.minLeft)->path_p : nullptr;
          pathlimits.maxRight = contourLimits.maxRight != nullptr ? ((mp_fill_object*)contourLimits.maxRight)->path_p : nullptr;
          pathlimits.minRight = contourLimits.minRight != nullptr ? ((mp_fill_object*)contourLimits.minRight)->path_p : nullptr;
        }


        dumpPath(layerArray, fillobject, currentx, currenty, pathlimits);
        if (layerArray.size() != 0) {
          data.append(layerArray);
          Layer layer;
          QByteArray newGlyphArray;
          double tempx = 0;
          double tempy = 0;
          PathLimits tempPathLimits;
          dumpPath(newGlyphArray, fillobject, tempx, tempy, tempPathLimits);
          layer.charString = newGlyphArray;
          if (fillobject->color_model == mp_rgb_model) {
            layer.color.red = toInt(fillobject->color.a_val * 255);
            layer.color.green = toInt(fillobject->color.b_val * 255);
            layer.color.blue = toInt(fillobject->color.c_val * 255);
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

  for (int i = 0; i < glyphCount; i++) {
    QByteArray glyphArray;

    if (glyphs.contains(i)) {
      auto& glyph = *glyphs.value(i);

      QVector<Layer> layers;

      QByteArray glyphData;

      double currentx = 0.0;
      double currenty = 0.0;



      if (replacedGlyphs.contains(glyph.charcode)) {
        glyphData = replacedGlyphs.value(glyph.charcode);
      }
      else {
        ContourLimits contourLimits;

        auto& ff = expandableGlyphs.find(glyph.name);

        if (ff != expandableGlyphs.end()) {
          auto& jj = ff->second;
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
        glyphData = charString(glyph.copiedPath, iscff2, layers, currentx, currenty, contourLimits);
      }

      bool needColor = false;

      Color black;

      for (auto& layer : layers) {
        if (!(layer.color == black)) {
          needColor = true;
          break;
        }
      }

      if (needColor) {
        this->layers.insert(i, layers);
      }

      if (glyphData.size() == 0) {
        if (iscff2) {
          //glyphArray << (uint8_t)0;
        }
        else {
          int_to_cff2(glyphArray, toInt(glyph.width)); // width
          glyphArray << (uint8_t)14; //  endchar
        }

      }
      else {
        if (iscff2) {
          glyphArray = glyphData;
        }
        else {
          int_to_cff2(glyphArray, toInt(glyph.width)); // width
          glyphArray.append(glyphData);
          glyphArray << (uint8_t)14; //  endchar
        }

      }


    }
    else {
      if (iscff2) {
        //glyphArray << (uint8_t)0;
      }
      else {
        int_to_cff2(glyphArray, 0); // width
        glyphArray << (uint8_t)14; //  endchar
      }

    }

    offsets.append(offset);
    offset += glyphArray.size();
    objectData.append(glyphArray);
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

  stringIndexData.append(globalValues.FullName.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.FamilyName.toLatin1());
  stringIndex << (uint32_t)(1 + stringIndexData.size());
  lastSid++;

  stringIndexData.append(globalValues.Weight.toLatin1());
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
//https://docs.microsoft.com/en-us/typography/opentype/spec/cff2#charStrings
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
  int v = roundf(val * 65536.f);
  cff << (uint8_t)255;
  cff << (int32_t)v;
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
QByteArray ToOpenType::getAyaMono() {

  QVector<Layer> ayaLayers;

  double endofayax = 0.0;
  double endofayay = 0.0;

  GlyphVis& endofaya = ot_layout->glyphs["endofaya"];

  QByteArray endofayaArray = charString(endofaya.copiedPath, this->isCff2, ayaLayers, endofayax, endofayay, ContourLimits{});

  auto monoPath = (mp_fill_object*)mp_new_graphic_object(ot_layout->mp, mp_fill_code);


  auto source = endofaya.copiedPath->next;
  auto dest = monoPath;
  dest->path_p = ((mp_fill_object*)endofaya.copiedPath)->path_p;

  int pathIndex = 1;

  do {
    if (source->type == mp_fill_code && pathIndex != 6 && pathIndex != 7 && pathIndex != 8 && pathIndex != 9) {
      auto newpath = (mp_fill_object*)mp_new_graphic_object(ot_layout->mp, mp_fill_code);
      newpath->path_p = ((mp_fill_object*)source)->path_p;
      dest->next = (mp_graphic_object*)newpath;
      dest = newpath;
    }
    source = source->next;
    pathIndex++;

  } while (source != nullptr);

  endofayax = 0.0;
  endofayay = 0.0;

  QByteArray endofayaMonoArray = charString((mp_graphic_object*)monoPath, this->isCff2, ayaLayers, endofayax, endofayay, ContourLimits{});


  while (monoPath != nullptr) {
    auto q = monoPath->next;
    free(monoPath);
    monoPath = (mp_fill_object*)q;
  }

  return endofayaMonoArray;

}
void ToOpenType::setAyaGlyphPaths() {
  GlyphVis& endofaya = ot_layout->glyphs["endofaya"];

  QVector<Layer> ayaLayers;
  QVector<Layer> layers;

  double endofayax = 0.0;
  double endofayay = 0.0;

  QByteArray endofayaArray = charString(endofaya.copiedPath, this->isCff2, ayaLayers, endofayax, endofayay, ContourLimits{});

  QByteArray endofayaMonoArray = getAyaMono();

  this->layers.insert(endofaya.charcode, ayaLayers);
  replacedGlyphs.insert(endofaya.charcode, endofayaMonoArray);
  QByteArray endofayaArraySubr;
  endofayaArraySubr.append(endofayaMonoArray);
  endofayaArraySubr << (uint8_t)11; // return

  subrByGlyph.insert(endofaya.charcode, subrOffsets.size());
  subrOffsets.append(subrs.size() + 1);
  subrs.append(endofayaArraySubr);

  for (int i = 0; i < 10; i++) {

    double currentx = 0.0;
    double currenty = 0.0;

    auto digit = glyphs[ot_layout->unicodeToGlyphCode.value(0x0660 + i)];
    QByteArray charStringArray = charString(digit->copiedPath, this->isCff2, layers, currentx, currenty, ContourLimits{});
    charStringArray << (uint8_t)11; // return
    subrByGlyph.insert(digit->charcode, subrOffsets.size());
    subrOffsets.append(subrs.size() + 1);
    subrs.append(charStringArray);
  }

  int yOffset = 110;

  for (auto glyph : glyphs) {
    if (!glyph->name.startsWith("aya")) continue;

    auto ayaNumber = glyph->name.mid(3).toInt();

    if (ayaNumber < 10) {
      auto onesglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + ayaNumber)];
      auto position = endofaya.width / 2 - (onesglyph->width) / 2;
      QByteArray charStringArray;
      charStringArray.append(endofayaArray);
      fixed_to_cff2(charStringArray, position);
      fixed_to_cff2(charStringArray, 0);
      charStringArray << (uint8_t)21; // rmoveto;

      double currentx = 0.0;
      double currenty = 0.0;

      QByteArray oneDigitArray = charString(onesglyph->copiedPath, this->isCff2, layers, currentx, currenty, ContourLimits{});
      charStringArray.append(oneDigitArray);


      QVector<Layer> newlayers = ayaLayers;

      QByteArray addDigitArray;
      fixed_to_cff2(addDigitArray, position);
      fixed_to_cff2(addDigitArray, yOffset);
      addDigitArray << (uint8_t)21; // rmoveto;
      //addDigitArray.append(oneDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(onesglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;
      Layer newlayer;
      newlayer.charString = addDigitArray;
      newlayer.color.foreground = true;
      newlayers.append(newlayer);

      this->layers.insert(glyph->charcode, newlayers);

      QByteArray monoChromeArray;

      int_to_cff2(monoChromeArray, subrByGlyph.value(endofaya.charcode) - subIndexBias);
      monoChromeArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(monoChromeArray, -endofayax);
      fixed_to_cff2(monoChromeArray, -endofayay);
      monoChromeArray << (uint8_t)21; // rmoveto;
      monoChromeArray.append(addDigitArray);

      replacedGlyphs.insert(glyph->charcode, monoChromeArray);

    }
    else if (ayaNumber < 100) {
      int onesdigit = ayaNumber % 10;
      int tensdigit = ayaNumber / 10;

      auto onesglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + onesdigit)];
      auto tensglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + tensdigit)];
      auto position = endofaya.width / 2 - (onesglyph->width + tensglyph->width + 40) / 2;

      QVector<Layer> newlayers = ayaLayers;

      double lastx = 0.0;
      double lasty = 0.0;
      double currentx = 0.0;
      double currenty = 0.0;
      QByteArray onesDigitArray = charString(onesglyph->copiedPath, this->isCff2, layers, currentx, currenty, ContourLimits{});
      QByteArray tensDigitArray = charString(tensglyph->copiedPath, this->isCff2, layers, lastx, lasty, ContourLimits{});

      QByteArray addDigitArray;
      fixed_to_cff2(addDigitArray, position);
      fixed_to_cff2(addDigitArray, yOffset);
      addDigitArray << (uint8_t)21; // rmoveto;
      //addDigitArray.append(tensDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(tensglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(addDigitArray, tensglyph->width + 40 - lastx);
      fixed_to_cff2(addDigitArray, 0 - lasty);
      addDigitArray << (uint8_t)21; // rmoveto;

      //addDigitArray.append(onesDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(onesglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;

      Layer newlayer;
      newlayer.charString = addDigitArray;
      newlayer.color.foreground = true;
      newlayers.append(newlayer);

      this->layers.insert(glyph->charcode, newlayers);

      QByteArray monoChromeArray;

      int_to_cff2(monoChromeArray, subrByGlyph.value(endofaya.charcode) - subIndexBias);
      monoChromeArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(monoChromeArray, -endofayax);
      fixed_to_cff2(monoChromeArray, -endofayay);
      monoChromeArray << (uint8_t)21; // rmoveto;
      monoChromeArray.append(addDigitArray);

      replacedGlyphs.insert(glyph->charcode, monoChromeArray);


    }
    else {
      int onesdigit = ayaNumber % 10;
      int tensdigit = (ayaNumber / 10) % 10;
      int hundredsdigit = ayaNumber / 100;

      auto onesglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + onesdigit)];
      auto tensglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + tensdigit)];
      auto hundredsglyph = glyphs[ot_layout->unicodeToGlyphCode.value(1632 + hundredsdigit)];

      auto position = endofaya.width / 2 - (onesglyph->width + tensglyph->width + hundredsglyph->width + 80) / 2;

      QVector<Layer> newlayers = ayaLayers;

      double tensx = 0.0;
      double tensy = 0.0;
      double hundredsx = 0.0;
      double hundredsy = 0.0;
      double currentx = 0.0;
      double currenty = 0.0;
      QByteArray onesDigitArray = charString(onesglyph->copiedPath, this->isCff2, layers, currentx, currenty, ContourLimits{});
      QByteArray tensDigitArray = charString(tensglyph->copiedPath, this->isCff2, layers, tensx, tensy, ContourLimits{});
      QByteArray hundredsDigitArray = charString(hundredsglyph->copiedPath, this->isCff2, layers, hundredsx, hundredsy, ContourLimits{});

      QByteArray addDigitArray;
      fixed_to_cff2(addDigitArray, position);
      fixed_to_cff2(addDigitArray, yOffset);
      addDigitArray << (uint8_t)21; // rmoveto;
      //addDigitArray.append(hundredsDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(hundredsglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(addDigitArray, hundredsglyph->width + 40 - hundredsx);
      fixed_to_cff2(addDigitArray, 0 - hundredsy);
      addDigitArray << (uint8_t)21; // rmoveto;
      //addDigitArray.append(tensDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(tensglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(addDigitArray, tensglyph->width + 40 - tensx);
      fixed_to_cff2(addDigitArray, 0 - tensy);
      addDigitArray << (uint8_t)21; // rmoveto;

      //addDigitArray.append(onesDigitArray);
      int_to_cff2(addDigitArray, subrByGlyph.value(onesglyph->charcode) - subIndexBias);
      addDigitArray << (uint8_t)10; // callsubr;

      Layer newlayer;
      newlayer.charString = addDigitArray;
      newlayer.color.foreground = true;
      newlayers.append(newlayer);

      this->layers.insert(glyph->charcode, newlayers);

      QByteArray monoChromeArray;

      int_to_cff2(monoChromeArray, subrByGlyph.value(endofaya.charcode) - subIndexBias);
      monoChromeArray << (uint8_t)10; // callsubr;

      fixed_to_cff2(monoChromeArray, -endofayax);
      fixed_to_cff2(monoChromeArray, -endofayay);
      monoChromeArray << (uint8_t)21; // rmoveto;
      monoChromeArray.append(addDigitArray);

      replacedGlyphs.insert(glyph->charcode, monoChromeArray);

    }
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
  data << getFixed(-2); // minValue
  data << getFixed(0); // defaultValue
  data << getFixed(12); // maxValue
  data << (uint16_t)0; // flags
  data << (uint16_t)LTATNameId; // axisNameID

  //VariationAxisRecord[1]
  data << (uint32_t)HB_TAG('R', 'T', 'A', 'T');
  data << getFixed(-2); // minValue
  data << getFixed(0); // defaultValue
  data << getFixed(12); // maxValue
  data << (uint16_t)0; // flags
  data << (uint16_t)RTATNameId; // axisNameID



  return data;
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

  QByteArray variationRegionList = ot_layout->getVariationRegionList();

  QByteArray ItemVariationData;
  //subtable 0
  ItemVariationData << (uint16_t)0; //itemCount
  ItemVariationData << (uint16_t)0; //shortDeltaCount
  ItemVariationData << (uint16_t)4; //regionIndexCount
  for (int i = 0; i < 4; i++) {
    ItemVariationData << (uint16_t)i; //regionIndexes
  }

  QByteArray itemVariationStore;

  itemVariationStore << (uint16_t)1; //format
  itemVariationStore << (uint32_t)12; //variationRegionListOffset
  itemVariationStore << (uint16_t)1; //itemVariationDataCount
  itemVariationStore << (uint32_t)(12 + variationRegionList.size()); //itemVariationDataCount
  itemVariationStore.append(variationRegionList);
  itemVariationStore.append(ItemVariationData);

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
  QByteArray variationRegionList = ot_layout->getVariationRegionList();

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

  struct EntryValue {
    int32_t value1 = 0;
    int32_t value2 = 0;
    int32_t value3 = 0;
    int32_t value4 = 0;
  };

  std::unordered_map<uint32_t, EntryValue> lsbs;

  QByteArray variationRegionList = ot_layout->getVariationRegionList();
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

      auto& ff = expandableGlyphs.find(glyph.name);

      if (ff != expandableGlyphs.end()) {
        entryExist = true;
        auto& jj = ff->second;
        if (jj.maxLeft != 0) {
          GlyphParameters parameters{};

          parameters.lefttatweel = jj.maxLeft;
          parameters.righttatweel = 0.0;

          auto alternate = glyph.getAlternate(parameters);
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
  data << (uint32_t)(20 + itemVariationStore.size() + advanceMapping.size() + lsbMapping.size()); //rsbMappingOffset
  data.append(itemVariationStore);
  data.append(advanceMapping);
  data.append(lsbMapping);
  data.append(lsbMapping);

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
