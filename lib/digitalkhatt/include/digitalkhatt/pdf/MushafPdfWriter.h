#pragma once
#include <filesystem>
#include <memory>
#include "Layout/OtLayout.h"

namespace digitalkhatt::pdf {

// Native counterpart of the editor's Generate Mushaf PDF writer. Glyph forms
// use the live MetaPost outlines, including colors and composed ayah numbers.
class MushafPdfWriter {
 public:
  struct Options {
    std::filesystem::path output;
    std::filesystem::path resources;
    double pageWidthMM = 90.2, pageHeightMM = 144.5;
    int textWidth = OtLayout::TextWidth;
    int margin = 400;
    bool notice = true;
    std::string title = "The Noble Quran";
  };
  MushafPdfWriter(OtLayout& layout, const Options& options);
  ~MushafPdfWriter();
  void start();
  void appendPage(const std::vector<LineLayoutInfo>& page,
                  const std::vector<TextString>& text,
                  int pageNumber, int surahBeforePage);
  void finish();
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
}  // namespace digitalkhatt::pdf
