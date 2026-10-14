#pragma once
#include "GlyphParameters.h"

#include <cstdint>
#include <cmath>
#include <functional>
#include <limits>
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

// Glyph advances and offsets are unscaled font units.
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
  GlyphParameters parameters;
  // Derived placement metadata, not a glyph axis or a HarfBuzz buffer field.
  double justificationAddedLeftTatweel = 0;
  uint32_t base_codepoint;
  bool beginsajda;
  bool endsajda;
  uint32_t color = 0;
};

struct LineLayoutInfo {
  std::vector<GlyphLayoutInfo> glyphs;
  int xstartposition; // Page-space offset from the right text edge.
  int ystartposition; // Page-space baseline.
  LineType type = LineType::Line;
  float overfull; // Final rendered width minus the page-space target.
  int desiredLineWidth; // Page-space target width.
  int currentLineWidth; // Sum of unscaled glyph advances.
  double fontSize; // Whole-line uniform transform; never applied during shaping.
  double xscale = 1;
  double horizontalScale() const { return fontSize * (type == LineType::Line ? xscale : 1.0); }
  double renderedWidth() const { return currentLineWidth * horizontalScale(); }
  double renderedOverflow() const {
    if (desiredLineWidth == 0) return 0;
    const double residual = renderedWidth() - desiredLineWidth;
    // Exact fitting can leave arithmetic noise after multiplying the scales.
    const double epsilon = 8 * std::numeric_limits<double>::epsilon() * std::abs(double(desiredLineWidth));
    return std::abs(residual) <= epsilon ? 0 : residual;
  }
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
  Experimental2,
  DeclPolicy
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
  // Optional index overriding the stretch policy named by linepolicy. A
  // negative value uses the font-declared default.
  int justStretchPolicy = -1;
  // Optional index overriding the shrink policy named by linepolicy. This is
  // used by DeclPolicy; ShrinkType remains the legacy-engine selector.
  int justShrinkPolicy = -1;
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
