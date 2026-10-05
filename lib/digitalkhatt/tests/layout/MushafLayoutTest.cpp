#include <stdexcept>
#include <iostream>
#include <memory>
#include <sqlite3.h>
#include "Layout/MushafLayout.h"

using namespace digitalkhatt;
static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
static TextString text(const unsigned char* value) {
  if (!value) return {};
  auto* buffer = hb_buffer_create();
  hb_buffer_add_utf8(buffer, reinterpret_cast<const char*>(value), -1, 0, -1);
  unsigned count = 0;
  auto* info = hb_buffer_get_glyph_infos(buffer, &count);
  TextString result;
  for (unsigned i = 0; i < count; ++i) result.push_back(info[i].codepoint);
  hb_buffer_destroy(buffer);
  return result;
}
int main() {
  try {
    TextString word = u"\u06de";
    normalizeMushafWord(word, "dk_v1");
    require(word == u"\u06de ", "End-of-word rub marker must retain the GUI's trailing space");
    word = u"\u0627\u0654\u06d6\u06d6";
    normalizeMushafWord(word, "dk_v1");
    require(word == u"\u0627\u034f\u0654\u034f\u06d6", "CGJ and duplicate waqf normalization");
    sqlite3* raw = nullptr;
    require(sqlite3_open_v2(DIGITALKHATT_QURAN_DATABASE, &raw, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK, "Cannot open corpus");
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> database(raw, sqlite3_close);
    for (const auto* table : {"qpc_v1_layout", "qpc_v2_layout", "qpc_v4_layout", "indopak_15_lines_layout"}) {
      const std::string name = table;
      const std::string column = name.starts_with("indopak") ? "dk_indopak" : name == "qpc_v1_layout" ? "dk_v1" : "dk_v2";
      const auto query = "SELECT l.page,l.line,l.type,w." + column + " FROM " + name +
          " l LEFT JOIN words w ON l.type='ayah' AND l.range_start<=w.word_number_all AND l.range_end>=w.word_number_all ORDER BY l.page,l.line,w.word_number_all";
      sqlite3_stmt* rawStatement = nullptr;
      require(sqlite3_prepare_v2(raw, query.c_str(), -1, &rawStatement, nullptr) == SQLITE_OK, "Invalid corpus query");
      std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> statement(rawStatement, sqlite3_finalize);
      std::vector<MushafWordRow> rows;
      int state;
      while ((state = sqlite3_step(rawStatement)) == SQLITE_ROW)
        rows.push_back({sqlite3_column_int(rawStatement, 0), sqlite3_column_int(rawStatement, 1),
            reinterpret_cast<const char*>(sqlite3_column_text(rawStatement, 2)), text(sqlite3_column_text(rawStatement, 3))});
      require(state == SQLITE_DONE, "Incomplete corpus read");
      const auto pages = assembleMushafText(rows, column);
      require(pages.size() == (name.starts_with("indopak") ? 610 : 604), "Corpus page coverage");
      int lines = 0, surahs = 0;
      for (size_t p = 0; p < pages.size(); ++p) {
        const auto source = splitMushafLines(pages[p]);
        const auto input = mushafLineInputs(source, p + 1, 16400, name);
        lines += input.size();
        for (const auto& line : input) surahs += line.lineType == LineType::Sura;
        if (p == 0 && !name.starts_with("indopak"))
          require(input[1].basm2 && input[1].width == (name == "qpc_v2_layout" ? 16400 : 8200) &&
              input[1].lineType == LineType::Bism, "Opening-page basmala layout");
      }
      require(surahs == 114, "All surah headers must be classified");
      if (!name.starts_with("indopak")) require(lines == 9046, "QPC line coverage");
      std::cout << name << ": " << pages.size() << " pages, " << lines << " lines\n";
    }
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
