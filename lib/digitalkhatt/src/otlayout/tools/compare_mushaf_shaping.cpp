#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <hb-ot.h>
#include "hb-buffer.hh"
#include <sqlite3.h>

#include "Layout/GlyphVis.h"
#include "Layout/OtLayout.h"
#include "MPFont.h"
#include "digitalkhatt/justify/FeatureJustifier.h"
#include "qurantext/quran.h"

namespace fs = std::filesystem;
using digitalkhatt::JustOption;
using digitalkhatt::JustStyle;
using digitalkhatt::LineJustification;
using digitalkhatt::LineLayoutInfo;
using digitalkhatt::LineToJustify;
using digitalkhatt::LineType;
using digitalkhatt::ShrinkType;
using digitalkhatt::TextString;
using digitalkhatt::JustType;

namespace {

std::string readFile(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("Could not open " + path.string());
  return {std::istreambuf_iterator<char>{stream}, {}};
}

TextString utf8ToUtf16(std::string_view input) {
  TextString result;
  for (std::size_t i = 0; i < input.size();) {
    const auto first = static_cast<unsigned char>(input[i++]);
    char32_t value = first;
    int continuation = 0;
    if ((first & 0xe0) == 0xc0) {
      value = first & 0x1f;
      continuation = 1;
    } else if ((first & 0xf0) == 0xe0) {
      value = first & 0x0f;
      continuation = 2;
    } else if ((first & 0xf8) == 0xf0) {
      value = first & 0x07;
      continuation = 3;
    }
    while (continuation-- > 0) {
      if (i >= input.size()) throw std::runtime_error("Invalid UTF-8");
      value = (value << 6) |
              (static_cast<unsigned char>(input[i++]) & 0x3f);
    }
    if (value <= 0xffff) {
      result.push_back(static_cast<char16_t>(value));
    } else {
      value -= 0x10000;
      result.push_back(static_cast<char16_t>(0xd800 + (value >> 10)));
      result.push_back(static_cast<char16_t>(0xdc00 + (value & 0x3ff)));
    }
  }
  return result;
}

void replaceAll(TextString& text, std::u16string_view from,
                std::u16string_view to) {
  for (auto pos = text.find(from); pos != TextString::npos;
       pos = text.find(from, pos + to.size()))
    text.replace(pos, from.size(), to);
}

void normalizeDkV1(TextString& word) {
  replaceAll(word, u"\u06d6\u06d6", u"\u06d6");
  replaceAll(word, u"\u0627\u0653", u"\u0627\u034f\u0653");
  replaceAll(word, u"\u0627\u0654", u"\u0627\u034f\u0654\u034f");
  replaceAll(word, u"\u0648\u0654", u"\u0648\u034f\u0654\u034f");
  replaceAll(word, u"\u064a\u0654", u"\u064a\u034f\u0654\u034f");
  replaceAll(word, u"\u0640\u0654", u"\u0640\u0654\u034f");
  replaceAll(word, u"\u0627\u0655", u"\u0627\u0655\u034f");
  for (std::size_t pos = 0; (pos = word.find(u'\u06de', pos)) != TextString::npos;
       ++pos) {
    if (pos + 1 < word.size() && word[pos + 1] != u' ')
      word.insert(pos + 1, 1, u' ');
  }
}

std::size_t registerGlyphSources(MPFont& font, const fs::path& glyphsPath) {
  const auto glyphs = readFile(glyphsPath);
  const std::string reset =
      "params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;";
  std::size_t count = 0;
  std::size_t pos = 0;
  while (pos < glyphs.size()) {
    const auto beginPos = glyphs.find("beginchar", pos);
    const auto defPos = glyphs.find("defchar", pos);
    const bool isDef = defPos != std::string::npos &&
                       (beginPos == std::string::npos || defPos < beginPos);
    const auto start = isDef ? defPos : beginPos;
    if (start == std::string::npos) break;
    const std::string_view endMarker = isDef ? "enddefchar;" : "endchar;";
    const auto end = glyphs.find(endMarker, start);
    if (end == std::string::npos)
      throw std::runtime_error("Unterminated glyph near byte " +
                               std::to_string(start));
    const auto blockEnd = end + endMarker.size();
    const auto block = glyphs.substr(start, blockEnd - start);
    const auto open = block.find('(');
    const auto comma1 = block.find(',', open);
    const auto comma2 = block.find(',', comma1 + 1);
    if (open == std::string::npos || comma1 == std::string::npos ||
        comma2 == std::string::npos)
      throw std::runtime_error("Invalid glyph header near byte " +
                               std::to_string(start));
    const auto name = block.substr(open + 1, comma1 - open - 1);
    const auto unicode =
        std::stoi(block.substr(comma1 + 1, comma2 - comma1 - 1));
    font.registerGlyphSource(name, block, isDef ? "defchar" : "beginchar",
                             unicode);
    font.execute(reset + block);
    ++count;
    pos = blockEnd;
  }
  return count;
}

void initializeFont(MPFont& font, const fs::path& project,
                    const fs::path& resources) {
  std::string source{"MPGUI:=1;"};
  source += readFile(resources / "mfplain.mp");
  source += readFile(resources / "mpost.mp");
  source += readFile(resources / "vmf.mp");
  source += readFile(project);
  font.initialize(std::move(source), project);
  const auto count = registerGlyphSources(font, project.parent_path() / "glyphs.mp");
  std::cout << "Registered " << count << " glyph sources\n";
}

struct SqliteDb {
  sqlite3* db = nullptr;
  explicit SqliteDb(const fs::path& path) {
    if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READONLY,
                        nullptr) != SQLITE_OK)
      throw std::runtime_error("Could not open SQLite database " +
                               path.string());
  }
  ~SqliteDb() {
    if (db) sqlite3_close(db);
  }
};

std::vector<std::vector<TextString>> loadQpcV1Pages(const fs::path& database) {
  SqliteDb connection(database);
  constexpr const char* sql =
      "SELECT page,line,type,dk_v1 FROM qpc_v1_layout AS l "
      "LEFT JOIN words w ON l.type='ayah' "
      "AND l.range_start<=w.word_number_all "
      "AND l.range_end>=w.word_number_all "
      "ORDER BY page,line,word_number_all";
  sqlite3_stmt* rawStatement = nullptr;
  if (sqlite3_prepare_v2(connection.db, sql, -1, &rawStatement, nullptr) !=
      SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(connection.db));
  struct StatementGuard {
    sqlite3_stmt* value;
    ~StatementGuard() { sqlite3_finalize(value); }
  } statement{rawStatement};

  std::vector<TextString> pageTexts;
  int lastPage = 1;
  int lastLine = 1;
  int lastSurah = 0;
  int wordNumberInLine = 1;
  TextString currentPage;
  while (sqlite3_step(statement.value) == SQLITE_ROW) {
    const int page = sqlite3_column_int(statement.value, 0);
    const int line = sqlite3_column_int(statement.value, 1);
    const auto* typeBytes = sqlite3_column_text(statement.value, 2);
    const auto* wordBytes = sqlite3_column_text(statement.value, 3);
    const std::string type =
        typeBytes ? reinterpret_cast<const char*>(typeBytes) : "";
    TextString word =
        wordBytes ? utf8ToUtf16(reinterpret_cast<const char*>(wordBytes))
                  : TextString{};
    if (type == "surah_name") {
      if (lastSurah >= static_cast<int>(surahNames.size()))
        throw std::runtime_error("Too many surah-name rows");
      word = utf8ToUtf16(surahNames[lastSurah++]);
    } else if (type == "basmallah") {
      word = u"\nبِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
    } else {
      normalizeDkV1(word);
    }

    if (lastPage != page) {
      pageTexts.push_back(std::move(currentPage));
      currentPage = std::move(word);
      lastPage = page;
      lastLine = 1;
    } else if (lastLine != line &&
               !(lastPage == 213 && lastLine == 4 &&
                 wordNumberInLine <= 11)) {
      currentPage += u'\n';
      currentPage += word;
      lastLine = line;
      wordNumberInLine = 1;
    } else if (currentPage.empty()) {
      currentPage = std::move(word);
      ++wordNumberInLine;
    } else {
      currentPage += u' ';
      currentPage += word;
      ++wordNumberInLine;
    }
  }
  pageTexts.push_back(std::move(currentPage));

  std::vector<std::vector<TextString>> pages;
  pages.reserve(pageTexts.size());
  for (const auto& page : pageTexts) {
    std::vector<TextString> lines;
    std::size_t start = 0;
    while (start <= page.size()) {
      const auto end = page.find(u'\n', start);
      const auto length =
          (end == TextString::npos ? page.size() : end) - start;
      if (length) lines.emplace_back(page.substr(start, length));
      if (end == TextString::npos) break;
      start = end + 1;
    }
    pages.push_back(std::move(lines));
  }
  return pages;
}

const std::map<int, double> oldMadinaLineWidths{
    {1 * 15 + 2, .5},   {1 * 15 + 3, .65},  {1 * 15 + 4, .80},
    {1 * 15 + 5, .9},   {1 * 15 + 6, .80},  {1 * 15 + 7, .65},
    {1 * 15 + 8, .4},   {2 * 15 + 2, .5},   {2 * 15 + 3, .65},
    {2 * 15 + 4, .85},  {2 * 15 + 5, .9},   {2 * 15 + 6, .85},
    {2 * 15 + 7, .65},  {2 * 15 + 8, .4},   {600 * 15 + 9, .82},
    {602 * 15 + 5, .57}, {602 * 15 + 15, .55},
    {603 * 15 + 10, .63}, {604 * 15 + 9, .79},
    {604 * 15 + 14, .67}, {604 * 15 + 15, .51}};

bool isSura(const TextString& line) { return line.starts_with(u"سُورَةُ "); }
bool isBism(const TextString& line) {
  return line == u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" ||
         line == u"بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
}

std::vector<LineToJustify> makeLines(const std::vector<TextString>& text,
                                     int page, int pageWidth) {
  std::vector<LineToJustify> result;
  result.reserve(text.size());
  for (int index = 0; index < static_cast<int>(text.size()); ++index) {
    int width = pageWidth;
    auto justification = LineJustification::Distribute;
    auto type = LineType::Line;
    bool basm2 = false;
    if (isSura(text[index]) || isBism(text[index])) {
      type = isSura(text[index]) ? LineType::Sura : LineType::Bism;
      if (!((page == 1 || page == 2) && index == 1)) {
        width = 0;
        justification = LineJustification::Center;
      } else {
        basm2 = true;
      }
    }
    const int key = page * 15 + index + 1;
    if (const auto found = oldMadinaLineWidths.find(key);
        found != oldMadinaLineWidths.end() && found->second < 1) {
      width = static_cast<int>(pageWidth * found->second);
      justification = LineJustification::Center;
    }
    result.push_back(
        {text[index], width, justification, type, basm2});
  }
  return result;
}

class OpenTypeProvider final
    : public digitalkhatt::justify::FeatureJustificationLayout {
 public:
  explicit OpenTypeProvider(const fs::path& path)
      : bytes_(readFile(path)),
        blob_(hb_blob_create(bytes_.data(), bytes_.size(),
                             HB_MEMORY_MODE_READONLY, nullptr, nullptr)),
        face_(hb_face_create(blob_, 0)) {
    if (hb_blob_get_length(blob_) == 0 || hb_face_get_glyph_count(face_) == 0)
      throw std::runtime_error("Invalid OpenType font " + path.string());
  }
  ~OpenTypeProvider() override {
    hb_face_destroy(face_);
    hb_blob_destroy(blob_);
  }
  hb_font_t* createFont(double emScale, bool) override {
    auto* font = hb_font_create(face_);
    hb_ot_font_set_funcs(font);
    const int upem = hb_face_get_upem(face_);
    const int scale = static_cast<int>(emScale * upem);
    hb_font_set_scale(font, scale, scale);
    hb_font_set_ppem(font, upem, upem);
    return font;
  }
  int scaleBy() const override { return OtLayout::SCALEBY; }
  int topSpace() const override { return OtLayout::TopSpace; }
  int interLineSpacing() const override { return OtLayout::InterLineSpacing; }
  hb_face_t* face() const { return face_; }

 private:
  std::string bytes_;
  hb_blob_t* blob_;
  hb_face_t* face_;
};

std::string glyphName(hb_font_t* font, std::uint32_t code) {
  char name[256]{};
  if (hb_font_get_glyph_name(font, code, name, sizeof(name))) return name;
  return "gid" + std::to_string(code);
}

struct SemanticGlyph {
  std::string base;
  double left = 0;
  double right = 0;
};

SemanticGlyph parseGeneratedName(const std::string& name) {
  static const std::regex generated(
      R"(^(.+?)\.(-?[0-9]+(?:\.[0-9]+)?)_(-?[0-9]+(?:\.[0-9]+)?)_[0-9]+$)");
  std::smatch match;
  if (std::regex_match(name, match, generated))
    // ToOpenType::post() encodes tatweel multiplied by ten so glyph names
    // contain no decimal separator ambiguity.
    return {match[1].str(), std::stod(match[2].str()) / 10.0,
            std::stod(match[3].str()) / 10.0};
  return {name, 0, 0};
}

struct ShapeTrace {
  long width = 0;
  std::vector<std::string> glyphs;
  std::vector<hb_position_t> advances;
};

ShapeTrace traceShape(const TextString& text, hb_font_t* font,
                      const std::vector<hb_feature_t>& features,
                      const OtLayout* sourceLayout = nullptr) {
  auto* buffer = hb_buffer_create();
  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", -1));
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  buffer->useCallback = true;
  hb_buffer_add_utf16(buffer, reinterpret_cast<const std::uint16_t*>(text.data()),
                      static_cast<int>(text.size()), 0,
                      static_cast<int>(text.size()));
  hb_shape(font, buffer, features.data(), features.size());
  unsigned count = 0;
  const auto* infos = hb_buffer_get_glyph_infos(buffer, &count);
  const auto* positions = hb_buffer_get_glyph_positions(buffer, &count);
  ShapeTrace result;
  result.glyphs.reserve(count);
  result.advances.reserve(count);
  for (unsigned i = 0; i < count; ++i) {
    result.width += positions[i].x_advance;
    std::string name;
    if (sourceLayout &&
        sourceLayout->glyphNamePerCode.contains(infos[i].codepoint))
      name = sourceLayout->glyphNamePerCode.at(infos[i].codepoint);
    else
      name = glyphName(font, infos[i].codepoint);
    if (infos[i].lefttatweel != 0 || infos[i].righttatweel != 0)
      name += "(" + std::to_string(infos[i].lefttatweel) + "," +
              std::to_string(infos[i].righttatweel) + ")";
    result.glyphs.push_back(std::move(name));
    result.advances.push_back(positions[i].x_advance);
  }
  hb_buffer_destroy(buffer);
  return result;
}

void traceShrinkFeatures(const TextString& text, OtLayout& sourceLayout,
                         OpenTypeProvider& otProvider, int page, int line) {
  auto* sourceFont = sourceLayout.createFont(1, false);
  auto* otFont = otProvider.createFont(1, false);
  std::vector<hb_feature_t> features;
  std::cout << "Shrink trace page " << page << ", line " << line << '\n';
  for (int index = 0; index <= 20; ++index) {
    if (index != 0) {
      const auto name =
          "sk" + std::string(index < 10 ? "0" : "") + std::to_string(index);
      features.push_back({hb_tag_from_string(name.c_str(), 4), 1, 0,
                          static_cast<unsigned>(-1)});
    }
    const auto source = traceShape(text, sourceFont, features, &sourceLayout);
    const auto ot = traceShape(text, otFont, features);
    std::cout << "  sk" << (index < 10 ? "0" : "") << index
              << ": source=" << source.width << " ot=" << ot.width
              << " delta=" << source.width - ot.width << '\n';
    if (source.glyphs != ot.glyphs) {
      const auto count = std::min(source.glyphs.size(), ot.glyphs.size());
      for (std::size_t glyph = 0; glyph < count; ++glyph) {
        const auto parsedOt = parseGeneratedName(ot.glyphs[glyph]);
        if (source.glyphs[glyph] != parsedOt.base ||
            source.advances[glyph] != ot.advances[glyph])
          std::cout << "    glyph " << glyph << ": "
                    << source.glyphs[glyph] << " <> " << ot.glyphs[glyph]
                    << " advances " << source.advances[glyph] << " <> "
                    << ot.advances[glyph]
                    << '\n';
      }
    }
  }
  hb_font_destroy(sourceFont);
  hb_font_destroy(otFont);
}

std::string csvQuote(std::string value) {
  for (std::size_t pos = 0; (pos = value.find('"', pos)) != std::string::npos;
       pos += 2)
    value.insert(pos, 1, '"');
  return '"' + value + '"';
}

void usage(const char* program) {
  std::cerr
      << "Usage: " << program << " [options] oldmadina.mp font.otf\n"
         "Compare qpc_v1_layout Mushaf shaping using live MetaPost outlines "
         "and a generated OpenType font.\n\n"
         "Options:\n"
         "  -o, --output PATH    Detailed CSV output\n"
         "  --database PATH      quran-data.sqlite path\n"
         "  --resources DIR      mfplain.mp/mpost.mp/vmf.mp directory\n"
         "  --pages A[-B]        Compare only this page or inclusive range\n"
         "  --tolerance N        Position tolerance in 1/256 font units\n"
         "  --trace-shrink P:L   Print cumulative sk01-sk20 widths\n"
         "  --fail-on-difference Return exit status 3 when differences exist\n"
         "  -h, --help           Show this help\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    fs::path project;
    fs::path otf;
    fs::path output{"mushaf-shaping-comparison.csv"};
    fs::path database{DIGITALKHATT_QURAN_DATABASE};
    fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    int firstPage = 1;
    int lastPage = 604;
    int tolerance = 1;
    bool failOnDifference = false;
    int tracePage = 0;
    int traceLine = 0;
    for (int i = 1; i < argc; ++i) {
      const std::string_view arg{argv[i]};
      auto value = [&]() -> std::string {
        if (++i >= argc)
          throw std::runtime_error("Missing value after " + std::string(arg));
        return argv[i];
      };
      if (arg == "-h" || arg == "--help") {
        usage(argv[0]);
        return 0;
      } else if (arg == "-o" || arg == "--output") {
        output = value();
      } else if (arg == "--database") {
        database = value();
      } else if (arg == "--resources") {
        resources = value();
      } else if (arg == "--tolerance") {
        tolerance = std::stoi(value());
      } else if (arg == "--fail-on-difference") {
        failOnDifference = true;
      } else if (arg == "--trace-shrink") {
        const auto location = value();
        const auto colon = location.find(':');
        if (colon == std::string::npos)
          throw std::runtime_error("--trace-shrink expects PAGE:LINE");
        tracePage = std::stoi(location.substr(0, colon));
        traceLine = std::stoi(location.substr(colon + 1));
      } else if (arg == "--pages") {
        const auto range = value();
        const auto dash = range.find('-');
        firstPage = std::stoi(range.substr(0, dash));
        lastPage = dash == std::string::npos
                       ? firstPage
                       : std::stoi(range.substr(dash + 1));
      } else if (!arg.empty() && arg.front() == '-') {
        throw std::runtime_error("Unknown option " + std::string(arg));
      } else if (project.empty()) {
        project = fs::absolute(arg);
      } else if (otf.empty()) {
        otf = fs::absolute(arg);
      } else {
        throw std::runtime_error("Too many positional arguments");
      }
    }
    if (project.empty() || otf.empty()) {
      usage(argv[0]);
      return 2;
    }
    if (firstPage < 1 || lastPage < firstPage)
      throw std::runtime_error("Invalid page range");

    MPFont mpFont;
    initializeFont(mpFont, project, resources);
    // LayoutWindow::loadLookupFile creates its persistent live shaper with
    // extended=true. This keeps tatweel in the custom online-shaper fields
    // instead of converting it to pre-generated equivalent glyph IDs.
    OtLayout sourceLayout(&mpFont, true, true);
    sourceLayout.useNormAxisValues = false;
    sourceLayout.quantizeGlyphAdvances = true;
    sourceLayout.loadLookupFile("features.fea");

    OpenTypeProvider otProvider(otf);
    digitalkhatt::justify::FeatureJustifier otJustifier(otProvider);
    const auto pages = loadQpcV1Pages(database);
    lastPage = std::min(lastPage, static_cast<int>(pages.size()));
    if (tracePage != 0) {
      if (tracePage < 1 || tracePage > static_cast<int>(pages.size()) ||
          traceLine < 1 ||
          traceLine > static_cast<int>(pages[tracePage - 1].size()))
        throw std::runtime_error("Trace page or line is out of range");
      traceShrinkFeatures(pages[tracePage - 1][traceLine - 1], sourceLayout,
                          otProvider, tracePage, traceLine);
    }

    std::ofstream report(output);
    if (!report) throw std::runtime_error("Could not create " + output.string());
    report << "page,line,glyph,source_name,ot_name,source_left,source_right,"
              "ot_left,ot_right,source_advance,ot_advance,source_x_offset,"
              "ot_x_offset,source_y_offset,ot_y_offset,status\n";

    constexpr JustOption options{JustType::Experimental2,
                                 JustStyle::FontSizeXScale,
                                 ShrinkType::Standard};
    const double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
    const int pageWidth = OtLayout::TextWidth << OtLayout::SCALEBY;
    std::size_t comparedGlyphs = 0;
    std::size_t differingGlyphs = 0;
    std::size_t differingLines = 0;
    bool sourceNewFace = true;
    bool otNewFace = true;

    for (int page = firstPage; page <= lastPage; ++page) {
      const auto lines = makeLines(pages.at(page - 1), page, pageWidth);
      auto sourcePage = sourceLayout.justifyPage(
          scale, pageWidth, lines, sourceNewFace, false,
          HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, options,
          "qpc_v1_layout");
      auto otPage = otJustifier.justifyPageUsingFeatures(
          scale, pageWidth, lines, otNewFace, false,
          HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, options,
          "qpc_v1_layout");
      sourceNewFace = false;
      otNewFace = false;

      auto* namingFont = otProvider.createFont(1, false);
      for (std::size_t line = 0;
           line < std::max(sourcePage.size(), otPage.size()); ++line) {
        if (line >= sourcePage.size() || line >= otPage.size()) {
          ++differingLines;
          report << page << ',' << line + 1
                 << ",-1,,,,,,,,,,,,,missing_line\n";
          continue;
        }
        const auto& sourceGlyphs = sourcePage[line].glyphs;
        const auto& otGlyphs = otPage[line].glyphs;
        if (page == tracePage && static_cast<int>(line + 1) == traceLine) {
          std::cout << "Final lookup trace page " << page << ", line "
                    << line + 1 << '\n';
          const auto traceCount = std::min(sourceGlyphs.size(), otGlyphs.size());
          for (std::size_t index = 0; index < traceCount; ++index) {
            if (sourceGlyphs[index].lookup_index !=
                    otGlyphs[index].lookup_index ||
                sourceGlyphs[index].subtable_index !=
                    otGlyphs[index].subtable_index ||
                sourceGlyphs[index].y_offset != otGlyphs[index].y_offset)
              std::cout << "  glyph " << index << ": source lookup "
                        << sourceGlyphs[index].lookup_index << "/"
                        << sourceGlyphs[index].subtable_index << " y="
                        << sourceGlyphs[index].y_offset << ", ot lookup "
                        << otGlyphs[index].lookup_index << "/"
                        << otGlyphs[index].subtable_index << " y="
                        << otGlyphs[index].y_offset << '\n';
          }
        }
        bool lineDiffers = sourceGlyphs.size() != otGlyphs.size();
        const auto count = std::max(sourceGlyphs.size(), otGlyphs.size());
        for (std::size_t glyphIndex = 0; glyphIndex < count; ++glyphIndex) {
          if (glyphIndex >= sourceGlyphs.size() ||
              glyphIndex >= otGlyphs.size()) {
            ++differingGlyphs;
            report << page << ',' << line + 1 << ',' << glyphIndex
                   << ",,,,,,,,,,,,,missing_glyph\n";
            continue;
          }
          ++comparedGlyphs;
          const auto& source = sourceGlyphs[glyphIndex];
          const auto& generated = otGlyphs[glyphIndex];
          const auto sourceName =
              sourceLayout.glyphNamePerCode.contains(source.codepoint)
                  ? sourceLayout.glyphNamePerCode.at(source.codepoint)
                  : "gid" + std::to_string(source.codepoint);
          const auto otName = glyphName(namingFont, generated.codepoint);
          SemanticGlyph semanticSource{sourceName, source.lefttatweel,
                                        source.righttatweel};
          if (const auto found = sourceLayout.glyphs.find(sourceName);
              sourceName.find(".added_") != std::string::npos &&
              found != sourceLayout.glyphs.end() &&
              !found->second.originalglyph.empty()) {
            semanticSource = {found->second.originalglyph,
                              found->second.charlt, found->second.charrt};
          } else if (source.lefttatweel != 0 ||
                     source.righttatweel != 0) {
            GlyphParameters requested;
            requested.lefttatweel = source.lefttatweel;
            requested.righttatweel = source.righttatweel;
            /*
             * The buffer stores requested justification parameters.  An
             * outline may clamp an unsupported direction or a value outside
             * the glyph's limits.  Compare the effective parameters recorded
             * by the generated live outline, matching the values encoded in
             * the generated OpenType glyph name.
             */
            if (auto* effective = sourceLayout.getAlternate(
                    source.codepoint, requested, false, false)) {
              semanticSource.left = effective->charlt;
              semanticSource.right = effective->charrt;
            }
          }
          const auto semanticOt = parseGeneratedName(otName);
          const bool nameDiffers = semanticSource.base != semanticOt.base;
          const bool tatweelDiffers =
              std::abs(semanticSource.left - semanticOt.left) > .001 ||
              std::abs(semanticSource.right - semanticOt.right) > .001;
          const bool metricDiffers =
              std::abs(source.x_advance - generated.x_advance) > tolerance ||
              std::abs(source.x_offset - generated.x_offset) > tolerance ||
              std::abs(source.y_offset - generated.y_offset) > tolerance;
          const bool differs = nameDiffers || tatweelDiffers || metricDiffers;
          if (differs) {
            ++differingGlyphs;
            lineDiffers = true;
          }
          report << page << ',' << line + 1 << ',' << glyphIndex << ','
                 << csvQuote(sourceName) << ',' << csvQuote(otName) << ','
                 << semanticSource.left << ',' << semanticSource.right << ','
                 << semanticOt.left << ',' << semanticOt.right << ','
                 << source.x_advance << ',' << generated.x_advance << ','
                 << source.x_offset << ',' << generated.x_offset << ','
                 << source.y_offset << ',' << generated.y_offset << ','
                 << (differs ? "different" : "equal") << '\n';
        }
        if (lineDiffers) ++differingLines;
      }
      hb_font_destroy(namingFont);
      std::cout << "Compared page " << page << '\n';
    }
    std::cout << "Compared glyphs: " << comparedGlyphs << '\n'
              << "Differing glyphs: " << differingGlyphs << '\n'
              << "Differing lines: " << differingLines << '\n'
              << "Report: " << fs::absolute(output) << '\n';
    return failOnDifference && differingGlyphs != 0 ? 3 : 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
