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

class ToOpenType
{
public:
  ToOpenType(OtLayout* layout);
  bool GenerateFile(QString fileName);

  struct GlobalValues{
    int16_t ascender;
    int16_t descender;
    int16_t lineGap;
    uint16_t advanceWidthMax;
    int16_t minLeftSideBearing;
    int16_t minRightSideBearing;
    int16_t xMaxExtent;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    int16_t yStrikeoutSize;
  };

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

private:
  OtLayout* ot_layout;
  uint32_t calcTableChecksum(uint32_t *Table, uint32_t Length);
 
  QMap<quint16,GlyphVis*> glyphs;
  GlobalValues globalValues;
  int toInt(double value);
};

#endif // TOOPENTYPE_H
