#include "MushafLayout.h"
#include <map>
#include <algorithm>
#include "digitalkhatt/core/Regex16.h"
#include "qurantext/quran.h"
namespace digitalkhatt {
namespace {
const std::map<int, double> oldMadinaLineWidths = {
    {1 * 15 + 2, 0.5}, {1 * 15 + 3, 0.65}, {1 * 15 + 4, 0.80}, {1 * 15 + 5, 0.9}, {1 * 15 + 6, 0.80}, {1 * 15 + 7, 0.65}, {1 * 15 + 8, 0.4}, {2 * 15 + 2, 0.5}, {2 * 15 + 3, 0.65}, {2 * 15 + 4, 0.85}, {2 * 15 + 5, 0.9}, {2 * 15 + 6, 0.85}, {2 * 15 + 7, 0.65}, {2 * 15 + 8, 0.4}, {600 * 15 + 9, 0.82}, {602 * 15 + 5, 0.57}, {602 * 15 + 15, 0.55}, {603 * 15 + 10, 0.63}, {604 * 15 + 9, 0.79}, {604 * 15 + 14, 0.67}, {604 * 15 + 15, 0.51}};

const std::map<int, double> madinaLineWidths = {
    {586 * 15 + 1, 0.81},
    {593 * 15 + 2, 0.81},
    {594 * 15 + 5, 0.63},
    {600 * 15 + 10, 0.63},
    {601 * 15 + 3, 1},
    {601 * 15 + 4, 1},
    {601 * 15 + 7, 1},
    {601 * 15 + 8, 1},
    {601 * 15 + 9, 1},
    {601 * 15 + 10, 1},
    {601 * 15 + 13, 1},
    {601 * 15 + 14, 1},
    {601 * 15 + 15, 1},
    {602 * 15 + 5, 0.63},
    {602 * 15 + 11, 0.9},
    {602 * 15 + 15, 0.53},
    {603 * 15 + 10, 0.66},
    {603 * 15 + 13, 1},
    {603 * 15 + 15, 0.60},
    {604 * 15 + 3, 1},
    {604 * 15 + 4, 0.55},
    {604 * 15 + 7, 1},
    {604 * 15 + 8, 1},
    {604 * 15 + 9, 0.55},
    {604 * 15 + 12, 1},
    {604 * 15 + 13, 1},
    {604 * 15 + 14, 0.675},
    {604 * 15 + 15, 0.5},
};


void replaceText(TextString& text, TextView from, TextView to) {
  for (size_t p = 0; (p = text.find(from, p)) != TextString::npos; p += to.size()) text.replace(p, from.size(), to);
}
}  // namespace

void normalizeMushafWord(TextString& word, std::string_view column) {
  if (column == "dk_v1") replaceText(word, u"\u06d6\u06d6", u"\u06d6");
  replaceText(word, u"\u0627\u0653", u"\u0627\u034f\u0653");
  replaceText(word, u"\u0627\u0654", u"\u0627\u034f\u0654\u034f");
  replaceText(word, u"\u0648\u0654", u"\u0648\u034f\u0654\u034f");
  replaceText(word, u"\u064a\u0654", u"\u064a\u034f\u0654\u034f");
  replaceText(word, u"\u0640\u0654", u"\u0640\u0654\u034f");
  replaceText(word, u"\u0627\u0655", u"\u0627\u0655\u034f");
  for (size_t p = 0; (p = word.find(u'\u06de', p)) != TextString::npos; ++p)
    if (p + 1 == word.size() || word[p + 1] != u' ') word.insert(p + 1, 1, u' ');
}

std::vector<TextString> assembleMushafText(const std::vector<MushafWordRow>& rows,
    std::string_view column) {
  std::vector<TextString> pages;
  int lastPage = 1, lastLine = 1, surah = 0, wordInLine = 1;
  TextString current;
  for (const auto& row : rows) {
    TextString word = row.text;
    if (row.type == "surah_name") {
      if (surah >= static_cast<int>(surahNames.size())) throw std::runtime_error("Too many surah headers");
      const auto* bytes = reinterpret_cast<const uint8_t*>(surahNames[surah++].data());
      auto* buffer = hb_buffer_create();
      hb_buffer_add_utf8(buffer, reinterpret_cast<const char*>(bytes), -1, 0, -1);
      unsigned count = 0;
      auto* info = hb_buffer_get_glyph_infos(buffer, &count);
      word.clear();
      for (unsigned i = 0; i < count; ++i) word.push_back(static_cast<char16_t>(info[i].codepoint));
      hb_buffer_destroy(buffer);
      if (column == "dk_indopak") {
        replaceText(word, u"\u064e\u0670", u"\u0670");
        replaceText(word, u"\u0627\u0655\u0650", u"\u0627\u0650");
      }
    } else if (row.type == "basmallah") {
      word = column == "dk_indopak" ? u"\nبِسْمِ اللّٰهِ الرَّحْمٰنِ الرَّحِيْمِ ۝" :
          u"\n" + madinaBasmalaText(surah);
    } else normalizeMushafWord(word, column);
    // Preserve Generate Mushaf's corpus assembly, including its page-213 rule.
    if (lastPage != row.page) {
      pages.push_back(std::move(current));
      lastPage = row.page; lastLine = 1; current = std::move(word);
    } else if (lastLine != row.line && !(lastPage == 213 && lastLine == 4 && wordInLine <= 11)) {
      current += u'\n'; current += word; lastLine = row.line; wordInLine = 1;
    } else if (current.empty()) { current = std::move(word); ++wordInLine; }
    else { current += u' '; current += word; ++wordInLine; }
  }
  if (!rows.empty()) pages.push_back(std::move(current));
  return pages;
}

std::vector<TextString> splitMushafLines(TextView text) {
  std::vector<TextString> result;
  for (size_t start = 0; start <= text.size();) {
    auto end = text.find(u'\n', start);
    if (end == TextView::npos) end = text.size();
    if (end > start) result.emplace_back(text.substr(start, end-start));
    if (end == text.size()) break;
    start = end + 1;
  }
  return result;
}

std::vector<LineToJustify> mushafLineInputs(const std::vector<TextString>& text,
    int pageNumber, int pageWidth, std::string_view layoutName) {
  static const Regex16 header(u"^(سُورَةُ .*|بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ( ۝١)?|بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ)$");
  const auto& widths = layoutName == "qpc_v2_layout" ? madinaLineWidths : oldMadinaLineWidths;
  std::vector<LineToJustify> result;
  for (size_t i = 0; i < text.size(); ++i) {
    LineToJustify line{text[i], pageWidth, LineJustification::Distribute, LineType::Line, false};
    if (header.match(text[i]).hasMatch()) {
      line.lineType = text[i].starts_with(u"سُ") ? LineType::Sura : LineType::Bism;
      if ((pageNumber == 1 || pageNumber == 2) && i == 1) line.basm2 = true;
      else { line.width = 0; line.lineJustification = LineJustification::Center; }
    }
    const auto width = widths.find(pageNumber * 15 + static_cast<int>(i) + 1);
    if (width != widths.end() && width->second < 1) {
      line.width = static_cast<int>(pageWidth * width->second);
      line.lineJustification = LineJustification::Center;
    }
    result.push_back(std::move(line));
  }
  return result;
}

void finishMushafPage(std::vector<LineLayoutInfo>& page, const std::vector<TextString>& text, int pageNumber) {
  if (pageNumber == 1 || pageNumber == 2)
    for (size_t i = 1; i < page.size(); ++i) page[i].ystartposition += 3000 << OtLayout::SCALEBY;
  static const Regex16 sajda = [] {
    TextString pattern = u"(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)";
    replaceText(pattern, u"\u0627\u0654", u"\u0627\u034f\u0654\u034f");
    return Regex16(pattern);
  }();
  for (size_t l = 0; l < page.size(); ++l) {
    if (page[l].type != LineType::Line) continue;
    const auto match = sajda.match(text[l]);
    if (!match.hasMatch()) continue;
    const int start = match.start(match.lastCapturedIndex());
    int end = match.end(match.lastCapturedIndex()) - 1;
    // All final captured groups end on a base or a mark. Use the Unicode
    // combining categories used by HarfBuzz, matching QChar::isMark.
    while (end >= 0) {
      const auto category = hb_unicode_general_category(hb_unicode_funcs_get_default(), text[l][end]);
      if (category != HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK && category != HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK && category != HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK) break;
      --end;
    }
    bool began = false;
    for (auto& g : page[l].glyphs) {
      if (g.cluster == start && !began) { g.beginsajda = true; began = true; }
      else if (g.cluster == end) { g.endsajda = true; break; }
    }
  }
}
}  // namespace digitalkhatt
