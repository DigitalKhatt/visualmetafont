/*
 * Copyright (c) 2020 Amine Anane. http://digitalkhatt/license
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

#ifndef TOOPENTYPE_H
#define TOOPENTYPE_H

#include "qstring.h"
#include "QByteArrayOperator.h"
#include "qmap.h"

class OtLayout;
class GlyphVis;
struct mp_graphic_object;
struct mp_fill_object;

class ToOpenType
{
public:
  ToOpenType(OtLayout* layout);
  bool GenerateFile(QString fileName);

  struct GlobalValues{
    int16_t ascender;
    int16_t descender;
    int16_t lineGap;
    int16_t sTypoAscender;
    int16_t sTypoDescender;
    int16_t sTypoLineGap;
    uint16_t advanceWidthMax;
    int16_t minLeftSideBearing;
    int16_t minRightSideBearing;
    int16_t xMaxExtent;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    int16_t yStrikeoutSize;
    int major;
    int minor;
    QString FullName;
    QString FamilyName;
    QString Weight;
    QString Copyright;
    QString License;
  };

  struct Color {
    uint8_t blue = 0;
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t alpha = 255;
    bool foreground = false;
    bool operator ==(Color b)
    {
       return (blue == b.blue && green == b.green && red == b.red && alpha == b.alpha);
    }
    bool operator <(Color b)
    {
      if(red != b.red)
        return red < b.red;
      else if(green != b.green)
        return green < b.green;
      else if (blue != b.blue)
        return blue < b.blue;
      else
       return alpha < b.alpha;
    }
  };



  struct Layer {
    QByteArray charString;
    Color color;
    uint16_t gid = 0;
  };

  QMap<uint16_t,QVector<Layer>> layers;

  QByteArray cmap();
  QByteArray gdef();
  QByteArray gpos();
  QByteArray gsub();
  QByteArray head();
  QByteArray hhea();
  QByteArray hmtx();
  QByteArray maxp();
  QByteArray name();
  QByteArray os2();
  QByteArray post();
  QByteArray cff2();
  QByteArray cff();
  QByteArray dsig();
  bool colrcpal(QByteArray& colr, QByteArray& cpal);

private:
  OtLayout* ot_layout;
  uint32_t calcTableChecksum(uint32_t *Table, uint32_t Length);
 
  QMap<quint16,GlyphVis*> glyphs;
  GlobalValues globalValues;
  int toInt(double value);
  void int_to_cff2(QByteArray& cff,int val) ;
  void fixed_to_cff2(QByteArray& cff,double val) ;
  QByteArray charStrings(bool iscff2);
  QByteArray charString(mp_graphic_object* object, bool iscff2, QVector<Layer>& layers, double& currentx, double& currenty);
  void initiliazeGlobals();

  int nbSubrs = 0;
  QByteArray subrs;
  QVector<int> subrOffsets;
  int subIndexBias = 107;
  void setGIds();
  void setAyaGlyphPaths();
  QMap<uint16_t,int> subrByGlyph;
  QMap<uint16_t,QByteArray> replacedGlyphs;
  bool isCff2 = false;
  void dumpPath(QByteArray& data, mp_fill_object* fill, double& currentx, double& currenty);

  QByteArray getSubrs();
  QByteArray getAyaMono();
};

inline bool operator<(const ToOpenType::Color &a, const ToOpenType::Color &b)
{
  if(a.red != b.red)
    return a.red < b.red;
  else if(a.green != b.green)
    return a.green < b.green;
  else if (a.blue != b.blue)
    return a.blue < b.blue;
  else
   return a.alpha < b.alpha;
}


#endif // TOOPENTYPE_H
