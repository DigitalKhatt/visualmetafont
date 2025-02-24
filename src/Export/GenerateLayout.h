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

#ifndef GENERATELAYOUT_H
#define GENERATELAYOUT_H

#include "OtLayout.h"
#include "qtextstream.h"

struct mp_graphic_object;
typedef struct mp_gr_knot_data* mp_gr_knot;

class GenerateLayout
{
public:
  GenerateLayout(OtLayout* otlayout, LayoutPages& layoutPages);
  ~GenerateLayout();
  void generateLayout(int lineWidth, int scale);
  void generateLayoutProtoBuf(int lineWidth, int scale);
  void generatePages(QJsonArray& pagesArray, int lineWidth, int scale);

  void generateGlyphs(QJsonObject& glyphsObject);
  void generateSuraLocations(QJsonArray& surasArray);

  void edgetoHTML5Path(mp_graphic_object* body, QJsonArray& pathsArray);
  void filltoHTML5Path(mp_gr_knot h, QJsonArray& pathArray);

private:
  OtLayout* m_otlayout;
  QHash<int, int> usedGlyphs;
  LayoutPages& layoutPages;
};

#endif
