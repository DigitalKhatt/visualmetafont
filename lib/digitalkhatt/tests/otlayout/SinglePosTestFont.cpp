#include "madina.h"
#include "fontplugin.h"

// Keep the callback fixture in the test, independent of the font's lookups.
class SinglePosTestFont : public Madina {
 public:
  using Madina::Madina;

  SingleAdjustFunc getSingleAdjustFunction(std::string name, Subtable* subtable) override {
    if (name != "test.singlepos.width") return Madina::getSingleAdjustFunction(name, subtable);
    return [this](GlyphVis* glyph) -> ValueRecord {
      auto* source = m_layout->getGlyph(glyph->sourceGlyphCode.value_or(glyph->charcode));
      const int entry = static_cast<int>(source->width) +
                        static_cast<int>(glyph->width - source->width);
      return {0, 0, static_cast<short>(entry - toInt(glyph->width)), 0};
    };
  }
};

extern "C" DIGITALKHATT_FONT_EXPORT Automedina* font_create(
    OtLayout* layout, MPFont* font, bool extended) {
  return new SinglePosTestFont(layout, font, extended);
}
