#pragma once

#include <string_view>
#include "OtLayout.h"

namespace digitalkhatt {

struct MushafWordRow {
  int page = 0, line = 0;
  std::string type;
  TextString text;
};
void normalizeMushafWord(TextString& word, std::string_view column);
std::vector<TextString> assembleMushafText(const std::vector<MushafWordRow>& rows,
                                        std::string_view column);
std::vector<TextString> splitMushafLines(TextView text);
std::vector<LineToJustify> mushafLineInputs(const std::vector<TextString>& text,
    int pageNumber, int pageWidth, std::string_view layoutName);
void finishMushafPage(std::vector<LineLayoutInfo>& page,
    const std::vector<TextString>& text, int pageNumber);
}  // namespace digitalkhatt
