/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

#ifndef GENERATELAYOUT_H
#define GENERATELAYOUT_H

#include "OtLayout.h"
#include "qtextstream.h"
#include <string_view>

struct mp_graphic_object;
typedef struct mp_gr_knot_data* mp_gr_knot;

namespace GenerateLayoutSupport {
inline constexpr double KafBodyMin = 0.0;
inline constexpr double KafBodyMax = 20.0;

inline bool hasKafBodyAxis(std::string_view glyphName) {
  return glyphName == "kaf.init.ii" || glyphName == "kaf.medi.ii";
}
}  // namespace GenerateLayoutSupport

class GenerateLayout
{
public:
  enum class BinaryFormat { Interpolated, WholeGlyphs, Both };

  GenerateLayout(OtLayout* otlayout, LayoutPages& layoutPages);
  ~GenerateLayout();
  void generateLayoutJson(int lineWidth, int scale);
  void generateLayoutBinary(int lineWidth, int scale, BinaryFormat format = BinaryFormat::Both);
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
