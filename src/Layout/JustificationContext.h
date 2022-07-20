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

#pragma once

#include "qvector.h"
#include "qmap.h"
#include  <unordered_map>


enum class StartEndLig {
  StartEnd,
  Start,
  End,
  EndKashida
};


struct GlyphExpansion {
  float MinLeftTatweel = 0.0;
  float MaxLeftTatweel = 0.0;
  float MinRightTatweel = 0.0;
  float MaxRightTatweel = 0.0;
  int weight = 1;
  int level = 0;
  StartEndLig startEndLig;
  bool shrinkIsAbsolute = false;
  bool stretchIsAbsolute = false;

};



class JustificationContext {
public:
  std::vector<unsigned int> GlyphsToExtend;
  std::vector<unsigned int> Substitutes;
  std::unordered_map<unsigned int, GlyphExpansion> Expansions;
  int totalWeight = 0;

  void clear() {
    GlyphsToExtend.clear();
    Substitutes.clear();
    Expansions.clear();
    totalWeight = 0;
  }
};
