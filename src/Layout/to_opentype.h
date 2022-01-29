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
#include "qstring.h"
#include "commontypes.h"

class OtLayout;
class GlyphVis;
struct mp_graphic_object;
struct mp_fill_object;
typedef struct mp_gr_knot_data* mp_gr_knot;

class ToOpenType
{
public:
  ToOpenType(OtLayout* layout);
  bool GenerateFile(QString fileName);

  struct GlobalValues {
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

  struct DeltaValues {
    std::vector<double> deltas{ 0.0,0.0,0.0,0.0 };
    std::vector<bool> active{ false,false,false,false };

    bool isEmpty() {
      for (int i = 0; i < deltas.size(); i++) {
        if (active[i] && deltas[i] != 0.0) {
          return false;
        }
      }
      return true;
    }

    DeltaValues& operator+=(const DeltaValues& rhs)
    {
      assert(rhs.deltas.size() == deltas.size());

      for (int i = 0; i < deltas.size(); i++) {
        assert(rhs.active[i] == this->active[i]);
        if (active[i]) {
          deltas[i] += rhs.deltas[i];
        }
      }

      return *this; // return the result by reference
    }

    DeltaValues& operator-=(const DeltaValues& rhs)
    {
      assert(rhs.deltas.size() == deltas.size());

      for (int i = 0; i < deltas.size(); i++) {
        //assert(rhs.active[i] == this->active[i]);
        if (active[i]) {
          deltas[i] -= rhs.deltas[i];
        }
      }

      return *this; // return the result by reference
    }

    friend DeltaValues operator+(DeltaValues lhs,
      double value)
    {
      for (int i = 0; i < lhs.deltas.size(); i++) {
        if (lhs.active[i]) {
          lhs.deltas[i] += value;
        }
      }
      return lhs; // return the result by value (uses move constructor)
    }

    friend DeltaValues operator-(DeltaValues lhs,
      double value)
    {
      for (int i = 0; i < lhs.deltas.size(); i++) {
        if (lhs.active[i]) {
          lhs.deltas[i] -= value;
        }
      }
      return lhs; // return the result by value (uses move constructor)
    }


    friend DeltaValues operator+(DeltaValues lhs,
      const DeltaValues& rhs)
    {
      lhs += rhs; // reuse compound assignment
      return lhs; // return the result by value (uses move constructor)
    }

    friend DeltaValues operator-(DeltaValues lhs,
      const DeltaValues& rhs)
    {
      lhs -= rhs; // reuse compound assignment
      return lhs; // return the result by value (uses move constructor)
    }
  };

  QByteArray blend(DeltaValues delta, ValueLimits limits) {
    QByteArray data;
    bool isEmpty = true;
    if (!uniformAxis) {
      for (int i = 0; i < delta.deltas.size(); i++) {
        if (delta.active[i] && delta.deltas[i] != 0.0) {
          isEmpty = false;
          fixed_to_cff2(data, delta.deltas[i]);
        }
        else {
          int_to_cff2(data, 0);
        }
      }
    }
    else {
      std::vector<bool> onlyOne;
      delta.active[0] = limits.maxLeft != 0.0; onlyOne.push_back(limits.maxLeft == axisLimits.maxLeft);
      delta.active[1] = limits.minLeft != 0.0; onlyOne.push_back(limits.minLeft == axisLimits.minLeft);
      delta.active[2] = limits.maxRight != 0.0; onlyOne.push_back(limits.maxRight == axisLimits.maxRight);
      delta.active[3] = limits.minRight != 0.0; onlyOne.push_back(limits.minRight == axisLimits.minRight);
      
      for (int i = 0; i < delta.deltas.size(); i++) {
        if (delta.active[i]) {
          isEmpty = false; 
          if (delta.deltas[i] != 0.0) {            
            fixed_to_cff2(data, delta.deltas[i]);
            if (!onlyOne[i]) {
              fixed_to_cff2(data, delta.deltas[i]);
              fixed_to_cff2(data, delta.deltas[i]);
            }
            
          }
          else {
            int_to_cff2(data, 0);
            if (!onlyOne[i]) {
              int_to_cff2(data, 0);
              int_to_cff2(data, 0);
            }           
          }
        }
        
      }
    }
    
    if (isEmpty) {
      data.clear();
    }
    else {
      int_to_cff2(data, 1);
      data << (uint8_t)16; // blend;
    }
    return data;
  }

  struct ContourLimits {
    mp_graphic_object* maxLeft = nullptr;
    mp_graphic_object* minLeft = nullptr;
    mp_graphic_object* maxRight = nullptr;
    mp_graphic_object* minRight = nullptr;
    ValueLimits limits;
  };



  struct PathLimits {
    ValueLimits limits;
    DeltaValues currentx;
    DeltaValues currenty;
    mp_gr_knot maxLeft = nullptr;
    mp_gr_knot minLeft = nullptr;
    mp_gr_knot maxRight = nullptr;
    mp_gr_knot minRight = nullptr;

    inline PathLimits next();
    inline DeltaValues x_coord();
    inline DeltaValues y_coord();
    inline DeltaValues left_x();
    inline DeltaValues left_y();
    inline DeltaValues right_x();
    inline DeltaValues right_y();
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
      if (red != b.red)
        return red < b.red;
      else if (green != b.green)
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

  QMap<uint16_t, QVector<Layer>> layers;

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
  QByteArray fvar();
  QByteArray HVAR();
  QByteArray HVAR_Old();
  QByteArray STAT();
  QByteArray MVAR();
  QByteArray JTST();

  bool colrcpal(QByteArray& colr, QByteArray& cpal);
  bool isCff2 = true;
  QByteArray getPrivateDictCff2(int* size);  

  QByteArray CFF2VariationStore();

  void populateGlyphs();
  QByteArray getVariationRegionList();

  


  const uint16_t LTATNameId = 256;
  const uint16_t RTATNameId = 257;

  ValueLimits axisLimits;

  void setUniformAxis(bool uniform);

  std::pair<int, int> getDeltaSetEntry(DefaultDelta delta, const ValueLimits& limits, std::vector<std::map<std::vector<int>, int>>& delatSets);
  std::pair<int, int> getDeltaSetEntry(DefaultDelta delta, const ValueLimits& limits) {
    return getDeltaSetEntry(delta, limits, GDEFDeltaSets);
  }

  QByteArray getGDEFItemVariationStore() {
    return getItemVariationStore(GDEFDeltaSets);
  }
  QByteArray getItemVariationStore(const std::vector<std::map<std::vector<int>, int>>& delatSets);

  bool isUniformAxis() {
    return uniformAxis;
  }

private:
  OtLayout* ot_layout;
  uint32_t calcTableChecksum(uint32_t* Table, uint32_t Length);

  QMap<quint16, GlyphVis*> glyphs;
  GlobalValues globalValues;
  
  void int_to_cff2(QByteArray& cff, int val);
  void fixed_to_cff2(QByteArray& cff, double val);
  QByteArray charStrings(bool iscff2);
  QByteArray charString(mp_graphic_object* object, bool iscff2, QVector<Layer>& layers, double& currentx, double& currenty, ContourLimits contourLimits);
  void initiliazeGlobals(); 

  int nbSubrs = 0;
  QByteArray subrs;
  QVector<int> subrOffsets;
  int subIndexBias = 107;
  void setGIds();
  void setAyaGlyphPaths();
  QMap<uint16_t, int> subrByGlyph;
  QMap<uint16_t, QByteArray> replacedGlyphs;

  void dumpPath(QByteArray& data, mp_fill_object* fill, double& currentx, double& currenty, PathLimits& pathLimits);

  QByteArray getSubrs();
  QByteArray getAyaMono();

  bool uniformAxis;
  std::unordered_map<ValueLimits, int> regionSubtables;
  std::vector<VariationRegion> regions;
  std::vector<std::vector<int>> subRegions;

  std::vector<std::map<std::vector<int>, int>> GDEFDeltaSets;
};

inline bool operator<(const ToOpenType::Color& a, const ToOpenType::Color& b)
{
  if (a.red != b.red)
    return a.red < b.red;
  else if (a.green != b.green)
    return a.green < b.green;
  else if (a.blue != b.blue)
    return a.blue < b.blue;
  else
    return a.alpha < b.alpha;
}


#endif // TOOPENTYPE_H
