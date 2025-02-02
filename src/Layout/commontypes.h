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

#ifndef H_COMMONTYPES
#define H_COMMONTYPES

#include <math.h>

struct GlyphParameters {
  double lefttatweel{ 0.0 };
  double righttatweel{ 0.0 };
  double third{ 0.0 };
  double fourth{ 0.0 };
  double fifth{ 0.0 };
  double scalex{ 0.0 };

  bool operator==(const GlyphParameters& r) const {
    return r.lefttatweel == lefttatweel && r.righttatweel == righttatweel && r.third == third && r.fourth == fourth
      && r.fifth == fifth
      && r.scalex == scalex;
  }
};
namespace std {
  template<>
  struct hash<GlyphParameters> {
    size_t operator()(const GlyphParameters& r) const
    {
      return hash<double>{}(r.lefttatweel)
        ^ hash<double>{}(r.righttatweel)
        ^ hash<double>{}(r.third)
        ^ hash<double>{}(r.fourth)
        ^ hash<double>{}(r.fifth)
        ^ hash<double>{}(r.scalex);
    }
  };

  template<>
  struct equal_to<GlyphParameters> {
    bool operator()(const GlyphParameters& r, const GlyphParameters& r2) const
    {
      return r == r2;
    }
  };
}



struct RegionAxisCoordinate {
  int16_t startCoord = 0;
  int16_t peakCoord = 0;
  int16_t endCoord = 0;

  bool operator==(const RegionAxisCoordinate& r) const {
    return r.startCoord == startCoord && r.peakCoord == peakCoord && r.endCoord == endCoord;
  }
};

using VariationRegion = std::vector<RegionAxisCoordinate>;

struct ValueLimits {
  float maxLeft = 0.0;
  float minLeft = 0.0;
  float maxRight = 0.0;
  float minRight = 0.0;

  bool operator==(const ValueLimits& r) const {
    return r.maxLeft == maxLeft && r.minLeft == minLeft && r.maxRight == maxRight && r.minRight == minRight;
  }
};

using DefaultDelta = std::vector<int>;

/*
struct DefaultDelta {
  int maxLeft = 0;
  int minLeft = 0;
  int maxRight = 0;
  int minRight = 0;
  int maxScaleX = 0;
  int minScaleX = 0;

  bool operator==(const DefaultDelta& r) const {
    return r.maxLeft == maxLeft && r.minLeft == minLeft && r.maxRight == maxRight && r.minRight == minRight
      && r.maxScaleX == maxScaleX && r.minScaleX == minScaleX;;
  }
};*/


inline int32_t getFixed(float val) {
  return  roundf(val * 65536.f);
}
inline  int16_t getF2DOT14(float val) {
  return  roundf(val * 16384.f);
}

inline int toInt(double value) {
  if (value < 0)
    return floor(value);
  else
    return ceil(value);
}

namespace std
{
  /*template<> struct hash<DefaultDelta>
  {
    std::size_t operator()(DefaultDelta const& s) const noexcept
    {
      return hash<int>{}(s.maxLeft)
        ^ hash<int>{}(s.minLeft)
        ^ hash<int>{}(s.maxRight)
        ^ hash<int>{}(s.minRight);
    }
  };


  template<>
  struct equal_to<DefaultDelta> {
    bool operator()(const DefaultDelta& r, const DefaultDelta& r2) const
    {
      return r == r2;
    }
  };*/

  template<> struct hash<ValueLimits>
  {
    std::size_t operator()(ValueLimits const& s) const noexcept
    {
      return hash<int>{}(s.maxLeft)
        ^ hash<int>{}(s.minLeft)
        ^ hash<int>{}(s.maxRight)
        ^ hash<int>{}(s.minRight);
    }
  };


  template<>
  struct equal_to<ValueLimits> {
    bool operator()(const ValueLimits& r, const ValueLimits& r2) const
    {
      return r == r2;
    }
  };

  template<> struct hash<RegionAxisCoordinate>
  {
    std::size_t operator()(RegionAxisCoordinate const& s) const noexcept
    {
      return hash<int>{}(s.startCoord)
        ^ hash<int>{}(s.peakCoord)
        ^ hash<int>{}(s.endCoord);
    }
  };


  template<>
  struct equal_to<RegionAxisCoordinate> {
    bool operator()(const RegionAxisCoordinate& r, const RegionAxisCoordinate& r2) const
    {
      return r == r2;
    }
  };


}

struct VarAxis {
  QString name;
  uint32_t axisTag = 0;
  float minValue = 0.0;
  float defaultValue = 0.0;
  float maxValue = 0.0;
  QString equivExpr;
};

#endif // H_COMMONTYPES
