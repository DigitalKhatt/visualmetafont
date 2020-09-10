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

ToOpenType::ToOpenType(OtLayout* layout):ot_layout{layout}
{
  for(auto& glyph : layout->glyphs){
    glyphs.insert(glyph.charcode,&glyph);
    if(glyph.bbox.llx < globalValues.xMin){
      globalValues.xMin = toInt(glyph.bbox.llx);
    }
    if(glyph.bbox.lly < globalValues.yMin ){
      globalValues.yMin= toInt(glyph.bbox.lly);
    }
    if(glyph.bbox.urx > globalValues.xMax ){
      globalValues.xMax = toInt(glyph.bbox.urx);
    }
    if(glyph.bbox.ury > globalValues.yMax ){
      globalValues.yMax = toInt(glyph.bbox.ury);
    }
    if(glyph.height> globalValues.ascender ){
      globalValues.ascender = toInt(glyph.height);
    }
    if(glyph.depth > globalValues.descender ){
      globalValues.descender = toInt(glyph.depth);
    }
    if(glyph.width > globalValues.advanceWidthMax ){
      globalValues.advanceWidthMax = toInt(glyph.width);
    }

    int rightSB = toInt(glyph.width) - toInt (glyph.bbox.urx);
    if(rightSB < globalValues.minRightSideBearing ){
      globalValues.minRightSideBearing = rightSB;
    }

  }

  globalValues.minLeftSideBearing = globalValues.xMin;
  globalValues.xMaxExtent = globalValues.xMax;
  globalValues.lineGap = 1500;
  globalValues.yStrikeoutSize = 20;
}

int ToOpenType::toInt(double value){
  if(value < 0)
    return floor(value);
  else
    return ceil(value);
}

bool ToOpenType::GenerateFile(QString fileName){

  struct Table {
    QByteArray data;
    hb_tag_t tag;
    uint32_t checkSum = 0;
    uint32_t offset= 0;
    uint32_t length= 0;
  };

  constexpr uint16_t numTables = 12;

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
     return false;

  Table tables[numTables];
  int index =  0;

  tables[index++] = { head(),HB_TAG('h', 'e', 'a', 'd')};
  tables[index++] = { hhea(),HB_TAG('h', 'h', 'e', 'a')};
  tables[index++] = { maxp(),HB_TAG('m', 'a', 'x', 'p')};
  tables[index++] = { os2(),HB_TAG('O','S','/','2')};
  tables[index++] = { name(),HB_TAG('n','a','m','e')};
  tables[index++] = { cmap(),HB_TAG('c','m','a','p')};
  tables[index++] = { post(),HB_TAG('p','o','s','t')};
  tables[index++] = { cff2(),HB_TAG('C','F','F','2')};
  tables[index++] = { hmtx(),HB_TAG('h','m','t','x')};
  tables[index++] = { gdef(),HB_TAG('G','D','E','F')};
  tables[index++] = { gsub(),HB_TAG('G','S','U','B')};
  tables[index++] = { gpos(),HB_TAG('G','P','O','S')};

  int offset = sizeof(uint32_t)+4*sizeof(uint16_t) + numTables*4*sizeof(uint32_t);

  for(int i = 0 ; i < numTables ; i++ ){

    tables[i].length = tables[i].data.size();
    uint32_t paddingLength = (tables[i].length+3) & ~3;
    for(uint32_t pad  = 0 ; pad < paddingLength - tables[i].length ; pad++){
      tables[i].data << (uint8_t)0;
    }
    tables[i].checkSum = calcTableChecksum((uint32_t *)tables[i].data.constData(),paddingLength);
    tables[i].offset = offset;
    offset+= paddingLength;
  }

  QByteArray data;


  uint16_t entrySelector = floor(log2(numTables));
  uint16_t searchRange = exp2(entrySelector) * 16;

  data << 0x4F54544F ;
  data << numTables;
  data << searchRange;
  data << entrySelector;
  data << (uint16_t)(numTables*16-searchRange);

  Table tagordered[numTables];
  std::copy(tables, tables + numTables, tagordered);


  std::sort(tagordered, tagordered + numTables,[](const Table &a, const Table &b){return a.tag < b.tag;} );

  for(int i =0; i < numTables; i++){
    auto& table = tagordered[i];
    data << (uint32_t)table.tag;
    data << (uint32_t)table.checkSum;
    data << (uint32_t)table.offset;
    data << (uint32_t)table.length;
  }

  for(int i =0; i < numTables; i++){
    auto& table = tables[i];
    data.append(table.data);
  }

  file.write(data);

  return true;
}

uint32_t ToOpenType::calcTableChecksum(uint32_t *table, uint32_t length)
{
  uint32_t sum = 0L;
  uint32_t *endPtr = table+((length+3) & ~3) / sizeof(uint32_t);
  while (table < endPtr){
    sum += *table++;
  }

  return sum;
}

QByteArray ToOpenType::cmap(){
  return ot_layout->getCmap();
}
QByteArray ToOpenType::gdef(){ 
  return ot_layout->getGDEF();
}
QByteArray ToOpenType::gpos(){
    return ot_layout->getGPOS();

}
QByteArray ToOpenType::gsub(){
    return ot_layout->getGSUB();
}
QByteArray ToOpenType::head(){
  QByteArray data;

  QDateTime now = QDateTime::currentDateTimeUtc();
  qint64 secs = QDate(1904,1,1).startOfDay(Qt::UTC).secsTo(now);

  data << (uint16_t)1; // majorVersion
  data << (uint16_t)0; // minorVersion
  data << (uint32_t)(1 << 16); // fontRevision 1.0
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
  data << (uint16_t)0; // lowestRecPPEM
  data << (int16_t)2; // fontDirectionHint
  data << (int16_t)0; // indexToLocFormat
  data << (int16_t)0; // glyphDataFormat

  return data;
}
QByteArray ToOpenType::hhea(){
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
  data << (int16_t)1; // caretSlopeRun
  data << (int16_t)0; // caretOffset
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // reserved
  data << (int16_t)0; // metricDataFormat
  data << (uint16_t)glyphs.size(); // numberOfHMetrics

  return data;
}
QByteArray ToOpenType::hmtx(){
  QByteArray data;

  for(auto glyph : glyphs){
    data << (uint16_t)toInt(glyph->width);
    data << (int16_t)toInt(glyph->bbox.llx);
  }

  return data;
}

QByteArray ToOpenType::maxp(){
  QByteArray data;

  data << (uint32_t)0x00005000 ;
  data << (uint16_t)glyphs.size() ;

  return data;
}
QByteArray ToOpenType::name(){

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

    names.append(Name{ 0,"Copyright" });
    names.append(Name{ 3,"DigitalKhatt" });
    names.append(Name{ 5,"Version 1.0" });
    names.append(Name{ 6,"DigitalKhatt" });
    names.append(Name{ 9,"Amine Anane" });
    names.append(Name{ 10,"Description" });
    names.append(Name{ 11,"https://digitalkhatt.org/" });
    names.append(Name{ 12,"https://digitalkhatt.org/" });

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
QByteArray ToOpenType::os2(){
  QByteArray data;

  data << (uint16_t)0x0005; // version
  data << (int16_t)500; // xAvgCharWidth
  data << (uint16_t)400; // usWeightClass
  data << (uint16_t)100; // usWidthClass
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

  
  data << (uint32_t) (1 << 13); //ulUnicodeRange1(Bits 0–31)
  data << (uint32_t)0; // ulUnicodeRange2(Bits 32–63)
  data << (uint32_t)0; //ulUnicodeRange3 (Bits 64–95)
  data << (uint32_t)0; //ulUnicodeRange4 (Bits 96–127)

  data << (uint32_t)0;  //achVendID
  data << (uint16_t)0b0000000011000000;  //fsSelection

  data << (uint16_t)10;  //usFirstCharIndex
  data << (uint16_t)0x08F3;  //usLastCharIndex

  data << (int16_t)globalValues.ascender; // sTypoAscender
  data << (int16_t)globalValues.descender; // sTypoDescender
  data << (int16_t)globalValues.lineGap; // sTypoLineGap
  data << (uint16_t)globalValues.ascender; // usWinAscent
  data << (uint16_t)globalValues.descender; // usWinDescent

  data << (uint32_t)(1 << 6); // ulCodePageRange1 Bits 0–31
  data << (uint32_t)(1 << 19); // ulCodePageRange2 Bits 32–63

  data << (int16_t)400; // sxHeight
  data << (int16_t)1000; // sCapHeight
  data << (uint16_t)0; // usDefaultChar
  data << (uint16_t)0x0020; // usBreakChar

  data << (uint16_t)30; // usMaxContext

  data << (uint16_t)0; // usLowerOpticalPointSize
  data << (uint16_t)0xFFFF; // usUpperOpticalPointSize

  return data;
}
QByteArray ToOpenType::post(){
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
QByteArray ToOpenType::cff2(){
  QByteArray data;

  return data;
}
