#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace digitalkhatt {

using TextString = std::u16string;
using TextView = std::u16string_view;
using StyleString = std::string;

// Replaces every occurrence of `from` with `to`. Used to build regex patterns
// from named character-class variables (mirrors QString::arg()/JS template
// literals) without pulling in a full templating engine for a `%N` marker.
inline TextString replaceAll(TextString text, TextView from, TextView to) {
  if (from.empty()) return text;
  size_t pos = 0;
  while ((pos = text.find(from, pos)) != TextString::npos) {
    text.replace(pos, from.size(), to);
    pos += to.size();
  }
  return text;
}

enum class LineType {
  Line = 0,
  Sura = 1,
  Bism = 2
};

struct GlyphLayoutInfo {
  int advance;
  int x_offset;
  int y_offset;
  int x_advance;
  int y_advance;
  int codepoint;
  int cluster;
  unsigned int lookup_index;
  unsigned int subtable_index;
  double lefttatweel = 0;
  double righttatweel = 0;
  uint32_t base_codepoint;
  bool beginsajda;
  bool endsajda;
  uint32_t color = 0;
};

struct LineLayoutInfo {
  std::vector<GlyphLayoutInfo> glyphs;
  int xstartposition;
  int ystartposition;
  LineType type = LineType::Line;
  float overfull;
  int desiredLineWidth;
  int currentLineWidth;
  double fontSize;
  double xscale = 1;
  double xscaleparameter = 0;
};

enum class LineJustification {
  Center,
  Distribute
};

struct LineToJustify {
  std::u16string text;
  int width;
  LineJustification lineJustification;
  LineType lineType;
  bool basm2 = false;
};

enum class JustType {
  None,
  Local,
  HarfBuzz,
  Madina,
  IndoPak,
  Experimental,
  Experimental2
};
enum class JustStyle {
  None,
  SameSizeByPage,
  XScale,
  FontSize,
  FontSizeXScale,
  SCLX
};
enum class ShrinkType {
  None,
  Standard,
  Test
};
struct JustOption {
  JustType justType = JustType::None;
  JustStyle justStyle = JustStyle::None;
  ShrinkType shrinkType = ShrinkType::None;
};

struct LineInput {
  LineType lineType = LineType::Line;
  TextString text;
};

using TajweedMap = std::map<int, StyleString>;
using PageTajweedResult = std::vector<TajweedMap>;
using SetTajweedCallback = std::function<void(int /*utf16Index*/, const StyleString& /*style*/)>;
using ResetIndexCallback = std::function<void()>;

}  // namespace digitalkhatt
