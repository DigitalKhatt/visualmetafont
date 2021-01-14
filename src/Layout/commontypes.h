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

struct ValueLimits {
  double maxLeft = 0.0;
  double minLeft = 0.0;
  double maxRight = 0.0;
  double minRight = 0.0;
};

struct DefaultDelta {
  int maxLeft = 0;
  int minLeft = 0;
  int maxRight = 0;
  int minRight = 0;

  bool operator==(const DefaultDelta& r) const {
    return r.maxLeft == maxLeft && r.minLeft == minLeft && r.maxRight == maxRight && r.minRight == minRight;
  }
};


inline int32_t getFixed(float val) {
  return  roundf(val * 65536.f);
}
inline  int16_t getF2DOT14(float val) {
  return  roundf(val * 16384.f);
}

namespace std
{
  template<> struct hash<DefaultDelta>
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
  };


}

#endif // H_COMMONTYPES
