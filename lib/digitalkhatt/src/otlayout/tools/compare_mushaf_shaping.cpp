#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <iomanip>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <hb-ot.h>
#include "hb-buffer.hh"
#include "hb-font.hh"
#include <sqlite3.h>

#include "Layout/GlyphVis.h"
#include "Layout/OtLayout.h"
#include "MPFont.h"
#include "digitalkhatt/justify/FeatureJustifier.h"
#include "digitalkhatt/justify/declpolicy/DeclPolicyPageJustifier.h"
#include "qurantext/quran.h"

namespace fs = std::filesystem;
using digitalkhatt::JustOption;
using digitalkhatt::JustStyle;
using digitalkhatt::JustType;
using digitalkhatt::LineJustification;
using digitalkhatt::LineLayoutInfo;
using digitalkhatt::LineToJustify;
using digitalkhatt::LineType;
using digitalkhatt::ShrinkType;
using digitalkhatt::TextString;

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
    {1 * 15 + 2, .5}, {1 * 15 + 3, .65}, {1 * 15 + 4, .80}, {1 * 15 + 5, .9}, {1 * 15 + 6, .80}, {1 * 15 + 7, .65}, {1 * 15 + 8, .4}, {2 * 15 + 2, .5}, {2 * 15 + 3, .65}, {2 * 15 + 4, .85}, {2 * 15 + 5, .9}, {2 * 15 + 6, .85}, {2 * 15 + 7, .65}, {2 * 15 + 8, .4}, {600 * 15 + 9, .82}, {602 * 15 + 5, .57}, {602 * 15 + 15, .55}, {603 * 15 + 10, .63}, {604 * 15 + 9, .79}, {604 * 15 + 14, .67}, {604 * 15 + 15, .51}};

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

std::string generatedGlyphIdentity(const std::string& name);

class OpenTypeProvider final
    : public digitalkhatt::justify::FeatureJustificationLayout {
 public:
  OpenTypeProvider(
      const fs::path& path,
      const digitalkhatt::justify::CompiledJustificationCatalog* catalog)
      : bytes_(readFile(path)),
        blob_(hb_blob_create(bytes_.data(), bytes_.size(),
                             HB_MEMORY_MODE_READONLY, nullptr, nullptr)),
        face_(hb_face_create(blob_, 0)),
        catalog_(catalog) {
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
  std::string glyphName(hb_font_t* font,
                        hb_codepoint_t glyph) const override {
    char name[256]{};
    return hb_font_get_glyph_name(font, glyph, name, sizeof(name))
               ? std::string{name}
               : std::string{};
  }
  std::string recognitionGlyphName(hb_font_t* font, hb_codepoint_t glyph) const override {
    const auto found = recognitionNames_.find(glyph);
    if (found != recognitionNames_.end()) return found->second;
    return recognitionNames_.emplace(glyph, generatedGlyphIdentity(glyphName(font, glyph))).first->second;
  }
  const digitalkhatt::justify::CompiledJustificationCatalog*
  justificationCatalog() const override {
    return catalog_;
  }
  hb_face_t* face() const { return face_; }

 private:
  mutable std::map<hb_codepoint_t, std::string> recognitionNames_;
  std::string bytes_;
  hb_blob_t* blob_;
  hb_face_t* face_;
  const digitalkhatt::justify::CompiledJustificationCatalog* catalog_;
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

SemanticGlyph liveSemanticGlyph(OtLayout& layout,
                                const GlyphLayoutInfo& glyph,
                                std::string& name) {
  name = layout.glyphNamePerCode.contains(glyph.codepoint)
             ? layout.glyphNamePerCode.at(glyph.codepoint)
             : "gid" + std::to_string(glyph.codepoint);
  SemanticGlyph semantic{name, glyph.parameters.lefttatweel, glyph.parameters.righttatweel};
  if (const auto found = layout.glyphs.find(name);
      found != layout.glyphs.end() &&
      found->second.sourceGlyphCode &&
      layout.glyphNamePerCode.contains(*found->second.sourceGlyphCode)) {
    semantic = {layout.glyphNamePerCode.at(*found->second.sourceGlyphCode),
                found->second.parameters.lefttatweel,
                found->second.parameters.righttatweel};
  } else if (glyph.parameters.lefttatweel != 0 || glyph.parameters.righttatweel != 0) {
    const auto& requested = glyph.parameters;
    /*
     * The buffer stores requested justification parameters. An outline may
     * clamp an unsupported direction or an out-of-range value. Compare the
     * effective parameters recorded by the generated live outline.
     */
    if (auto* effective = layout.getAlternate(glyph.codepoint, requested,
                                              false, false)) {
      semantic.left = effective->charlt;
      semantic.right = effective->charrt;
    }
  }
  return semantic;
}

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

std::string generatedGlyphIdentity(const std::string& name) {
  return parseGeneratedName(name).base;
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
    const auto tatweels = font->glyph_tatweels(infos[i]);
    if (tatweels.left != 0 || tatweels.right != 0)
      name += "(" + std::to_string(tatweels.left) + "," +
              std::to_string(tatweels.right) + ")";
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

// ---------------------------------------------------------------------------
// Golden snapshots
//
// The whole-Mushaf gate compares one engine against another.  That answers
// "did this change relative to the reference", which is exactly the right
// question while a reference exists -- and no question at all once it is
// retired.  A snapshot records what an engine actually produced, so the same
// gate survives Experimental2 going away, and so a deliberate change arrives
// as a reviewable diff rather than as a count of differing glyphs.
//
// The file is per line, not per glyph, so it stays small enough to keep under
// version control.  Two digests rather than one, because the two kinds of
// regression read differently: `shape` covers which glyphs were chosen and how
// far each was stretched, `metrics` covers where they were finally placed.
// With --snapshot-glyphs the per-glyph detail is written too; the check never
// decides anything from it, it only uses it to say what moved.

constexpr std::string_view kSnapshotMagic =
    "digitalkhatt-justification-snapshot";
constexpr int kSnapshotVersion = 1;
constexpr std::uint64_t kFnvOffsetBasis = 0xcbf29ce484222325ULL;

std::uint64_t hashBytes(std::uint64_t seed, const void* data,
                        std::size_t size) {
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t index = 0; index < size; ++index) {
    seed ^= bytes[index];
    seed *= 0x100000001b3ULL;  // FNV-1a
  }
  return seed;
}
template <typename T>
std::uint64_t hashValue(std::uint64_t seed, const T& value) {
  return hashBytes(seed, &value, sizeof(value));
}

struct LineDigest {
  std::size_t glyphs = 0;
  std::uint64_t shape = kFnvOffsetBasis;
  std::uint64_t metrics = kFnvOffsetBasis;
  double xscale = 1;
  double fontSize = 0;
  int width = 0;
  bool operator==(const LineDigest&) const = default;
};

// Tatweels are hashed as their exact bit patterns.  A golden file wants
// exactness: a tatweel that moved in the last bit is a change to explain, not
// one to round away.
LineDigest digestLine(const LineLayoutInfo& line, bool provenance = false) {
  LineDigest digest{.glyphs = line.glyphs.size(),
                    .xscale = line.xscale,
                    .fontSize = line.fontSize,
                    .width = line.currentLineWidth};
  for (const auto& glyph : line.glyphs) {
    digest.shape = hashValue(digest.shape, glyph.codepoint);
    digest.shape = hashValue(digest.shape, glyph.cluster);
    const auto& parameters = glyph.parameters;
    digest.shape = hashValue(digest.shape, parameters.lefttatweel);
    digest.shape = hashValue(digest.shape, parameters.righttatweel);
    // Preserve legacy neutral digests, while detecting extra-axis changes.
    if (parameters.third != 0 || parameters.fourth != 0 || parameters.fifth != 0) {
      digest.shape = hashValue(digest.shape, parameters.third);
      digest.shape = hashValue(digest.shape, parameters.fourth);
      digest.shape = hashValue(digest.shape, parameters.fifth);
    }
    for (digitalkhatt::GlyphAxisId axis = 5; axis < parameters.size(); ++axis) {
      if (parameters.value(axis) == 0) continue;
      digest.shape = hashValue(digest.shape, axis);
      digest.shape = hashValue(digest.shape, parameters.value(axis));
    }
    digest.metrics = hashValue(digest.metrics, glyph.x_advance);
    digest.metrics = hashValue(digest.metrics, glyph.x_offset);
    digest.metrics = hashValue(digest.metrics, glyph.y_offset);
    if (provenance) {
      digest.metrics = hashValue(digest.metrics, glyph.y_advance);
      digest.metrics = hashValue(digest.metrics, glyph.lookup_index);
      digest.metrics = hashValue(digest.metrics, glyph.subtable_index);
      digest.metrics = hashValue(digest.metrics, glyph.base_codepoint);
    }
  }
  return digest;
}

std::string hex64(std::uint64_t value) {
  std::ostringstream stream;
  stream << std::hex << std::setw(16) << std::setfill('0') << value;
  return stream.str();
}

// Enough precision to round-trip a double exactly, so rewriting a snapshot
// that did not change produces a byte-identical file.
std::string exact(double value) {
  std::ostringstream stream;
  stream << std::setprecision(17) << value;
  return stream.str();
}

struct SnapshotGlyph {
  std::string name;
  double lefttatweel = 0;
  double righttatweel = 0;
  int x_advance = 0;
  int x_offset = 0;
  int y_offset = 0;
  int cluster = 0;
  double third = 0, fourth = 0, fifth = 0;
  GlyphParameters parameters;
};

struct SnapshotLine {
  int page = 0;
  int line = 0;
  LineDigest digest;
  std::vector<SnapshotGlyph> glyphs;  // only when the file carries detail
};

std::vector<std::string> splitFields(const std::string& row) {
  std::vector<std::string> fields;
  std::size_t start = 0;
  while (true) {
    const auto comma = row.find(',', start);
    fields.push_back(row.substr(start, comma - start));
    if (comma == std::string::npos) break;
    start = comma + 1;
  }
  return fields;
}

struct Snapshot {
  std::string engine;
  bool hasGlyphs = false;
  std::map<std::pair<int, int>, SnapshotLine> lines;
};

Snapshot readSnapshot(const fs::path& path) {
  std::ifstream file(path);
  if (!file) throw std::runtime_error("Could not read snapshot " + path.string());
  Snapshot snapshot;
  std::string row;
  bool sawMagic = false;
  SnapshotLine* current = nullptr;
  while (std::getline(file, row)) {
    if (!row.empty() && row.back() == '\r') row.pop_back();
    if (row.empty()) continue;
    if (row.front() == '#') {
      if (row.find(kSnapshotMagic) != std::string::npos) sawMagic = true;
      const auto engine = row.find("engine=");
      if (engine != std::string::npos) {
        const auto end = row.find(' ', engine);
        snapshot.engine = row.substr(engine + 7, end - engine - 7);
      }
      if (row.find("glyphs=yes") != std::string::npos) snapshot.hasGlyphs = true;
      continue;
    }
    if (row.rfind("page,", 0) == 0) continue;  // column header
    const auto fields = splitFields(row);
    if (fields.front() == "g") {
      if (current == nullptr || fields.size() < 9)
        throw std::runtime_error("Malformed glyph row in " + path.string());
      current->glyphs.push_back({.name = fields[2],
                                 .lefttatweel = std::stod(fields[3]),
                                 .righttatweel = std::stod(fields[4]),
                                 .x_advance = std::stoi(fields[5]),
                                 .x_offset = std::stoi(fields[6]),
                                 .y_offset = std::stoi(fields[7]),
                                 .cluster = std::stoi(fields[8]),
                                 .third = fields.size() > 9 ? std::stod(fields[9]) : 0,
                                 .fourth = fields.size() > 10 ? std::stod(fields[10]) : 0,
                                 .fifth = fields.size() > 11 ? std::stod(fields[11]) : 0});
      auto& glyph = current->glyphs.back();
      glyph.parameters = {.lefttatweel = glyph.lefttatweel, .righttatweel = glyph.righttatweel, .third = glyph.third, .fourth = glyph.fourth, .fifth = glyph.fifth};
      for (std::size_t field = 12; field < fields.size(); ++field)
        glyph.parameters.set(static_cast<digitalkhatt::GlyphAxisId>(field - 7), std::stod(fields[field]));
      continue;
    }
    if (fields.size() < 8)
      throw std::runtime_error("Malformed line row in " + path.string());
    SnapshotLine entry;
    entry.page = std::stoi(fields[0]);
    entry.line = std::stoi(fields[1]);
    entry.digest.glyphs = static_cast<std::size_t>(std::stoul(fields[2]));
    entry.digest.shape = std::stoull(fields[3], nullptr, 16);
    entry.digest.metrics = std::stoull(fields[4], nullptr, 16);
    entry.digest.xscale = std::stod(fields[5]);
    entry.digest.fontSize = std::stod(fields[6]);
    entry.digest.width = std::stoi(fields[7]);
    const auto key = std::make_pair(entry.page, entry.line);
    current = &snapshot.lines.emplace(key, std::move(entry)).first->second;
  }
  if (!sawMagic)
    throw std::runtime_error(path.string() + " is not a justification snapshot");
  return snapshot;
}

void writeSnapshotHeader(std::ostream& out, std::string_view engine,
                         int firstPage, int lastPage, bool withGlyphs) {
  out << "# " << kSnapshotMagic << ' ' << kSnapshotVersion << '\n'
      << "# engine=" << engine << " pages=" << firstPage << '-' << lastPage
      << " glyphs=" << (withGlyphs ? "yes" : "no") << '\n'
      << "page,line,glyphs,shape,metrics,xscale,fontsize,width\n";
}

void writeSnapshotLine(std::ostream& out, int page, int lineNumber,
                       const LineLayoutInfo& line, const LineDigest& digest,
                       bool withGlyphs, hb_font_t* namingFont) {
  out << page << ',' << lineNumber << ',' << digest.glyphs << ','
      << hex64(digest.shape) << ',' << hex64(digest.metrics) << ','
      << exact(digest.xscale) << ',' << exact(digest.fontSize) << ','
      << digest.width << '\n';
  if (!withGlyphs) return;
  for (std::size_t index = 0; index < line.glyphs.size(); ++index) {
    const auto& glyph = line.glyphs[index];
    const auto& parameters = glyph.parameters;
    out << "g," << index << ',' << glyphName(namingFont, glyph.codepoint) << ','
        << exact(parameters.lefttatweel) << ',' << exact(parameters.righttatweel) << ','
        << glyph.x_advance << ',' << glyph.x_offset << ',' << glyph.y_offset
        << ',' << glyph.cluster << ',' << exact(parameters.third) << ',' << exact(parameters.fourth) << ',' << exact(parameters.fifth);
    for (digitalkhatt::GlyphAxisId axis = 5; axis < parameters.size(); ++axis) out << ',' << exact(parameters.value(axis));
    out << '\n';
  }
}

// Names the first field that differs, so a failure reads as a cause rather
// than as a page number.
std::string describeDigestDifference(const LineDigest& baseline,
                                     const LineDigest& current) {
  if (baseline.glyphs != current.glyphs)
    return "glyph count " + std::to_string(baseline.glyphs) + " -> " +
           std::to_string(current.glyphs);
  if (baseline.shape != current.shape) return "different glyphs or tatweels";
  if (baseline.metrics != current.metrics) return "different positions";
  if (baseline.width != current.width)
    return "line width " + std::to_string(baseline.width) + " -> " +
           std::to_string(current.width);
  if (baseline.xscale != current.xscale)
    return "xscale " + exact(baseline.xscale) + " -> " + exact(current.xscale);
  return "font size " + exact(baseline.fontSize) + " -> " +
         exact(current.fontSize);
}

std::string csvQuote(std::string value) {
  for (std::size_t pos = 0; (pos = value.find('"', pos)) != std::string::npos;
       pos += 2)
    value.insert(pos, 1, '"');
  return '"' + value + '"';
}

void printCandidateTrace(int page, const digitalkhatt::justify::JustificationDecisionTrace& trace);

// Writes a snapshot, or checks one.  Returns the number of differing lines;
// writing always returns zero.
std::size_t runSnapshot(OpenTypeProvider& provider, const std::vector<std::vector<TextString>>& pages, int firstPage, int lastPage, const fs::path& path, bool check, bool withGlyphs, JustType engine, int stretchPolicy, int shrinkPolicy, std::ostream& reportOut, OtLayout* live = nullptr, int tracePage = 0, int traceLine = 0) {
  const std::string engineName = std::string(live ? (live->quantizeGlyphAdvances ? "live-" : "live-native-") : "") +
      (engine == JustType::DeclPolicy ? "decl-policy" : engine == JustType::HarfBuzz ? "harfbuzz" : "experimental2");
  const JustOption options{engine, JustStyle::FontSizeXScale, ShrinkType::Standard, stretchPolicy, shrinkPolicy};
  const double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  const int pageWidth = OtLayout::TextWidth << OtLayout::SCALEBY;

  Snapshot baseline;
  if (check) {
    baseline = readSnapshot(path);
    if (!baseline.engine.empty() && baseline.engine != engineName) {
      throw std::runtime_error("Snapshot was recorded from the " +
                               baseline.engine + " engine, but this run uses " +
                               std::string(engineName));
    }
  }
  std::ofstream out;
  if (!check) {
    out.open(path);
    if (!out) throw std::runtime_error("Could not create " + path.string());
    writeSnapshotHeader(out, engineName, firstPage, lastPage, withGlyphs);
  }

  digitalkhatt::justify::FeatureJustifier featureJustifier(provider);
  digitalkhatt::justify::DeclPolicyPageJustifier declPolicyJustifier(provider);
  bool newFace = true;
  std::size_t differingLines = 0;
  std::size_t comparedLines = 0;
  std::size_t reported = 0;
  std::set<std::pair<int, int>> seen;
  constexpr std::size_t kMaxReported = 20;
  std::chrono::steady_clock::duration shapingTime{};
  const auto currentPage = std::make_shared<int>(0);
  if (tracePage != 0) {
    auto callback = [currentPage, tracePage, traceLine](const auto& trace) {
      if (*currentPage == tracePage && trace.lineIndex + 1 == traceLine) printCandidateTrace(*currentPage, trace);
    };
    if (live) live->setJustificationTraceCallback(callback);
    else provider.setJustificationTraceCallback(callback);
  }

  for (int page = firstPage; page <= lastPage; ++page) {
    *currentPage = page;
    const auto lines = makeLines(pages.at(page - 1), page, pageWidth);
    const auto started = std::chrono::steady_clock::now();
    const auto laidOut =
        live ? live->justifyPage(scale, pageWidth, lines, newFace, false,
                  HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, options, "qpc_v1_layout") : engine == JustType::DeclPolicy
            ? declPolicyJustifier.justifyPage(
                  scale, pageWidth, lines, newFace, false,
                  HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, options,
                  "qpc_v1_layout")
            : featureJustifier.justifyPage(
                  scale, pageWidth, lines, newFace, false,
                  HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, options,
                  "qpc_v1_layout");
    shapingTime += std::chrono::steady_clock::now() - started;
    newFace = false;
    auto* namingFont = live ? live->createFont(1, false) : provider.createFont(1, false);
    for (std::size_t index = 0; index < laidOut.size(); ++index) {
      const int lineNumber = static_cast<int>(index) + 1;
      const auto digest = digestLine(laidOut[index], live != nullptr);
      if (!check) {
        writeSnapshotLine(out, page, lineNumber, laidOut[index], digest,
                          withGlyphs, namingFont);
        continue;
      }
      ++comparedLines;
      seen.emplace(page, lineNumber);
      const auto recorded = baseline.lines.find({page, lineNumber});
      if (recorded == baseline.lines.end()) {
        ++differingLines;
        reportOut << page << ',' << lineNumber
                  << ",-1,,,,,,,,,,,,,missing_from_snapshot\n";
        if (reported++ < kMaxReported)
          std::cout << "  page " << page << " line " << lineNumber
                    << ": not in the snapshot\n";
        continue;
      }
      if (recorded->second.digest == digest) continue;
      ++differingLines;
      const auto reason =
          describeDigestDifference(recorded->second.digest, digest);
      reportOut << page << ',' << lineNumber << ",-1,,,,,,,,,,,,,"
                << csvQuote(reason) << '\n';
      if (reported < kMaxReported) {
        ++reported;
        std::cout << "  page " << page << " line " << lineNumber << ": "
                  << reason << '\n';
        // Detail is only ever explanatory: the digests above already decided.
        if (baseline.hasGlyphs) {
          const auto& before = recorded->second.glyphs;
          const auto& after = laidOut[index].glyphs;
          std::size_t shown = 0;
          for (std::size_t glyph = 0;
               glyph < std::max(before.size(), after.size()) && shown < 5;
               ++glyph) {
            if (glyph >= before.size() || glyph >= after.size()) {
              std::cout << "      glyph " << glyph << ": only on one side\n";
              ++shown;
              continue;
            }
            const auto& was = before[glyph];
            const auto& now = after[glyph];
            const auto name = glyphName(namingFont, now.codepoint);
            const auto& nowParameters = now.parameters;
            if (was.name == name && was.parameters == nowParameters &&
                was.x_advance == now.x_advance &&
                was.x_offset == now.x_offset && was.y_offset == now.y_offset)
              continue;
            std::cout << "      glyph " << glyph << ": " << was.name << " ["
                      << exact(was.lefttatweel) << ' '
                      << exact(was.righttatweel) << ' ' << exact(was.third) << "] adv "
                      << was.x_advance << "  ->  " << name << " ["
                      << exact(nowParameters.lefttatweel) << ' '
                      << exact(nowParameters.righttatweel) << ' ' << exact(nowParameters.third) << "] adv "
                      << now.x_advance << '\n';
            ++shown;
          }
        }
      }
    }
    hb_font_destroy(namingFont);
    std::cout << (check ? "Checked page " : "Recorded page ") << page << '\n';
  }
  if (check) {
    // A line the snapshot has and this run did not produce is a regression
    // too, and the page loop above cannot see it.
    for (const auto& [key, entry] : baseline.lines) {
      const auto [page, lineNumber] = key;
      if (page < firstPage || page > lastPage) continue;
      if (seen.contains(key)) continue;
      ++differingLines;
      reportOut << page << ',' << lineNumber
                << ",-1,,,,,,,,,,,,,missing_from_run\n";
      if (reported++ < kMaxReported)
        std::cout << "  page " << page << " line " << lineNumber
                  << ": in the snapshot but not produced (" << entry.digest.glyphs
                  << " glyphs)\n";
    }
    std::cout << "Snapshot lines compared: " << comparedLines << '\n'
              << "Differing lines: " << differingLines << '\n';
    if (reported >= kMaxReported && differingLines > kMaxReported)
      std::cout << "  (" << differingLines - kMaxReported
                << " more, see the report)\n";
  } else {
    std::cout << "Snapshot written: " << fs::absolute(path) << '\n';
  }
  std::cout << "Snapshot shaping seconds: " << std::chrono::duration<double>(shapingTime).count() << '\n';
  return differingLines;
}

void usage(const char* program) {
  std::cerr << "Usage: " << program
            << " [options] oldmadina.mp font.otf\n"
               "Compare qpc_v1_layout Mushaf shaping using live MetaPost outlines "
               "and a generated OpenType font.\n\n"
               "Options:\n"
               "  -o, --output PATH    Detailed CSV output\n"
               "  --database PATH      quran-data.sqlite path\n"
               "  --resources DIR      mfplain.mp/mpost.mp/vmf.mp directory\n"
               "  --features PATH      feature file used to compile the live policy\n"
               "  --pages A[-B]        Compare only this page or inclusive range\n"
               "  --tolerance N        Position tolerance in 1/256 font units\n"
               "  --trace-shrink P:L   Print cumulative sk01-sk20 widths\n"
               "  --trace-candidates P:L  Print candidate-pool decisions as JSON lines\n"
               "  --decl-policy        Compare Experimental2 with declarative-policy justification\n"
               "  --same-provider      Run both engines on one OpenType provider and print timings\n"
               "  --snapshot PATH      Record one engine's own output as a golden file\n"
               "  --check-snapshot PATH  Compare this run against such a file\n"
               "  --snapshot-glyphs    Include per-glyph detail when recording\n"
               "  --snapshot-live      Snapshot live rendering, including positioning provenance\n"
               "  --snapshot-native-metrics  Use unquantized live advances\n"
               "  --snapshot-engine E  decl-policy (default), experimental2, or harfbuzz\n"
               "  --stretch-policy NAME  Override the declarative stretchpolicy named by linepolicy\n"
               "  --shrink-policy NAME  Override the declarative shrinkpolicy named by linepolicy\n"
               "  --fail-on-difference Return exit status 3 when differences exist\n"
               "  -h, --help           Show this help\n";
}

void printCandidateTrace(int page, const digitalkhatt::justify::JustificationDecisionTrace& trace) {
  const auto optionalNumber = [](const std::optional<double>& value) {
    return value ? std::to_string(*value) : std::string{"null"};
  };
  std::cout << "candidate_trace {\"page\":" << page
            << ",\"line\":" << trace.lineIndex + 1
            << ",\"pass\":" << trace.pass
            << ",\"priority_band\":" << trace.priorityBand
            << ",\"selection\":" << std::quoted(trace.selection)
            << ",\"rule\":" << std::quoted(trace.rule)
            << ",\"word\":" << trace.wordIndex
            << ",\"subword\":" << trace.subwordIndex
            << ",\"site\":" << trace.site
            << ",\"subword_length\":" << trace.subwordLength
            << ",\"connection_after\":" << trace.connectionAfter
            << ",\"occurrence\":" << trace.occurrence
            << ",\"base_weight\":" << trace.baseWeight
            << ",\"occurrence_adjustment\":" << trace.occurrenceAdjustment
            << ",\"subword_length_adjustment\":" << trace.subwordLengthAdjustment
            << ",\"centrality_adjustment\":" << trace.centralityAdjustment
            << ",\"word_position_adjustment\":" << trace.wordPositionAdjustment
            << ",\"score\":" << trace.score
            << ",\"minimum_width_delta\":" << optionalNumber(trace.minimumWidthDelta)
            << ",\"maximum_width_delta\":" << optionalNumber(trace.maximumWidthDelta)
            << ",\"remaining_width\":" << trace.remainingWidth
            << ",\"applied_ratio\":" << trace.appliedRatio
            << ",\"decision\":" << std::quoted(trace.decision)
            << ",\"reason\":" << std::quoted(trace.reason)
            << ",\"parameters\":[";
  for (std::size_t index = 0; index < trace.parameters.size(); ++index) {
    if (index != 0) std::cout << ',';
    const auto& parameter = trace.parameters[index];
    std::cout << "{\"site\":" << parameter.site
              << ",\"attribute\":" << std::quoted(parameter.attribute)
              << ",\"initial\":" << parameter.initialValue
              << ",\"minimum\":" << parameter.minimumValue
              << ",\"maximum\":" << parameter.maximumValue
              << ",\"applied\":" << parameter.appliedValue << '}';
  }
  std::cout << "]}\n";
}

}  // namespace

int main(int argc, char** argv) {
  try {
    fs::path project;
    fs::path otf;
    fs::path output{"output/mushaf-shaping-comparison.csv"};
    fs::path database{DIGITALKHATT_QURAN_DATABASE};
    fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    fs::path features;
    int firstPage = 1;
    int lastPage = 604;
    int tolerance = 1;
    bool failOnDifference = false;
    bool compareDeclPolicy = false;
    bool sameProvider = false;
    fs::path snapshotPath;
    bool snapshotCheck = false;
    bool snapshotGlyphs = false;
    bool snapshotLive = false;
    bool snapshotNativeMetrics = false;
    JustType snapshotEngine = JustType::DeclPolicy;
    std::string stretchPolicyName;
    std::string shrinkPolicyName;
    int tracePage = 0;
    int traceLine = 0;
    int candidateTracePage = 0;
    int candidateTraceLine = 0;
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
      } else if (arg == "--features") {
        features = fs::absolute(value());
      } else if (arg == "--tolerance") {
        tolerance = std::stoi(value());
      } else if (arg == "--fail-on-difference") {
        failOnDifference = true;
      } else if (arg == "--decl-policy" || arg == "--dfa") {
        compareDeclPolicy = true;
      } else if (arg == "--same-provider") {
        compareDeclPolicy = true;
        sameProvider = true;
      } else if (arg == "--snapshot") {
        snapshotPath = fs::absolute(value());
        snapshotCheck = false;
      } else if (arg == "--check-snapshot") {
        snapshotPath = fs::absolute(value());
        snapshotCheck = true;
      } else if (arg == "--snapshot-glyphs") {
        snapshotGlyphs = true;
      } else if (arg == "--snapshot-live") {
        snapshotLive = true;
      } else if (arg == "--snapshot-native-metrics") {
        snapshotNativeMetrics = true;
      } else if (arg == "--snapshot-engine") {
        const auto name = value();
        if (name == "decl-policy" || name == "fixed-slot") {
          snapshotEngine = JustType::DeclPolicy;
        } else if (name == "experimental2") {
          snapshotEngine = JustType::Experimental2;
        } else if (name == "harfbuzz") {
          snapshotEngine = JustType::HarfBuzz;
        } else {
          throw std::runtime_error(
              "--snapshot-engine expects decl-policy, experimental2, or harfbuzz");
        }
      } else if (arg == "--stretch-policy") {
        stretchPolicyName = value();
      } else if (arg == "--shrink-policy") {
        shrinkPolicyName = value();
      } else if (arg == "--trace-shrink") {
        const auto location = value();
        const auto colon = location.find(':');
        if (colon == std::string::npos)
          throw std::runtime_error("--trace-shrink expects PAGE:LINE");
        tracePage = std::stoi(location.substr(0, colon));
        traceLine = std::stoi(location.substr(colon + 1));
      } else if (arg == "--trace-candidates") {
        const auto location = value();
        const auto colon = location.find(':');
        if (colon == std::string::npos) throw std::runtime_error("--trace-candidates expects PAGE:LINE");
        candidateTracePage = std::stoi(location.substr(0, colon));
        candidateTraceLine = std::stoi(location.substr(colon + 1));
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
    if (features.empty()) features = project.parent_path() / "features.fea";

    MPFont mpFont;
    initializeFont(mpFont, project, resources);
    // LayoutWindow::loadLookupFile creates its persistent live shaper with
    // extended=true. This keeps tatweel in the custom online-shaper fields
    // instead of converting it to pre-generated equivalent glyph IDs.
    OtLayout sourceLayout(&mpFont, true, true);
    sourceLayout.useNormAxisValues = false;
    sourceLayout.quantizeGlyphAdvances = !snapshotNativeMetrics;
    sourceLayout.loadLookupFile(features.string());

    const auto* catalog = sourceLayout.compiledJustificationCatalog
                              ? &*sourceLayout.compiledJustificationCatalog
                              : nullptr;
    const bool runsDeclPolicy = !snapshotPath.empty() ? snapshotEngine == JustType::DeclPolicy : compareDeclPolicy;
    if (!snapshotPath.empty() && snapshotEngine == JustType::HarfBuzz && !snapshotLive) throw std::runtime_error("the HarfBuzz snapshot engine requires --snapshot-live");
    int stretchPolicy = -1;
    if (!stretchPolicyName.empty()) {
      if (!runsDeclPolicy) throw std::runtime_error("--stretch-policy requires a declarative-policy comparison");
      if (!catalog) throw std::runtime_error("the font has no justification catalog");
      stretchPolicy = catalog->stretchPolicyIndex(stretchPolicyName);
      if (stretchPolicy < 0) throw std::runtime_error("unknown stretch policy " + stretchPolicyName);
    }
    int shrinkPolicy = -1;
    if (!shrinkPolicyName.empty()) {
      if (!runsDeclPolicy) throw std::runtime_error("--shrink-policy requires a declarative-policy comparison");
      if (!catalog) throw std::runtime_error("the font has no justification catalog");
      shrinkPolicy = catalog->shrinkPolicyIndex(shrinkPolicyName);
      if (shrinkPolicy < 0) throw std::runtime_error("unknown shrink policy " + shrinkPolicyName);
    }

    OpenTypeProvider otProvider(otf, catalog);
    digitalkhatt::justify::FeatureJustifier otJustifier(otProvider);
    digitalkhatt::justify::DeclPolicyPageJustifier otDeclPolicyJustifier(otProvider);
    const auto pages = loadQpcV1Pages(database);
    lastPage = std::min(lastPage, static_cast<int>(pages.size()));
    if (candidateTracePage != 0) {
      if (!runsDeclPolicy) throw std::runtime_error("--trace-candidates requires declarative-policy justification");
      if (candidateTracePage < 1 || candidateTracePage > static_cast<int>(pages.size()) || candidateTraceLine < 1 || candidateTraceLine > static_cast<int>(pages[candidateTracePage - 1].size())) throw std::runtime_error("Candidate trace page or line is out of range");
    }
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

    if (!snapshotPath.empty()) {
      const auto differing = runSnapshot(otProvider, pages, firstPage, lastPage, snapshotPath, snapshotCheck, snapshotGlyphs, snapshotEngine, stretchPolicy, shrinkPolicy, report, snapshotLive ? &sourceLayout : nullptr, candidateTracePage, candidateTraceLine);
      std::cout << "Report: " << fs::absolute(output) << '\n';
      return failOnDifference && differing != 0 ? 3 : 0;
    }

    constexpr JustOption sourceOptions{JustType::Experimental2,
                                       JustStyle::FontSizeXScale,
                                       ShrinkType::Standard};
    const JustOption otOptions{compareDeclPolicy ? JustType::DeclPolicy : JustType::Experimental2, JustStyle::FontSizeXScale, ShrinkType::Standard, stretchPolicy, shrinkPolicy};
    const double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
    const int pageWidth = OtLayout::TextWidth << OtLayout::SCALEBY;
    std::size_t comparedGlyphs = 0;
    std::size_t differingGlyphs = 0;
    std::size_t differingLines = 0;
    bool sourceNewFace = true;
    bool otNewFace = true;
    std::chrono::steady_clock::duration regexTime{};
    std::chrono::steady_clock::duration targetTime{};
    int currentPage = 0;
    if (candidateTracePage != 0) {
      otProvider.setJustificationTraceCallback([&](const auto& trace) {
        if (currentPage == candidateTracePage && trace.lineIndex + 1 == candidateTraceLine) printCandidateTrace(currentPage, trace);
      });
    }

    for (int page = firstPage; page <= lastPage; ++page) {
      currentPage = page;
      const auto lines = makeLines(pages.at(page - 1), page, pageWidth);
      std::vector<LineLayoutInfo> sourcePage;
      std::vector<LineLayoutInfo> otPage;
      auto runRegex = [&] {
        const auto start = std::chrono::steady_clock::now();
        if (sameProvider) {
          sourcePage = otJustifier.justifyPage(
              scale, pageWidth, lines, sourceNewFace, false,
              HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, sourceOptions,
              "qpc_v1_layout");
        } else {
          sourcePage = sourceLayout.justifyPage(
              scale, pageWidth, lines, sourceNewFace, false,
              HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES, sourceOptions,
              "qpc_v1_layout");
        }
        regexTime += std::chrono::steady_clock::now() - start;
        sourceNewFace = false;
      };
      auto runTarget = [&] {
        const auto start = std::chrono::steady_clock::now();
        otPage = compareDeclPolicy
                     ? otDeclPolicyJustifier.justifyPage(
                           scale, pageWidth, lines, otNewFace, false,
                           HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,
                           otOptions, "qpc_v1_layout")
                     : otJustifier.justifyPage(
                           scale, pageWidth, lines, otNewFace, false,
                           HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES,
                           otOptions, "qpc_v1_layout");
        otNewFace = false;
        targetTime += std::chrono::steady_clock::now() - start;
      };
      // Alternate execution order so neither engine consistently benefits
      // from running second on warmed layout/font caches.
      if (sameProvider && (page - firstPage) % 2 != 0) {
        runTarget();
        runRegex();
      } else {
        runRegex();
        runTarget();
      }

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
          std::string sourceName;
          const auto semanticSource = sameProvider
                                          ? parseGeneratedName(
                                                sourceName = glyphName(
                                                    namingFont,
                                                    source.codepoint))
                                          : liveSemanticGlyph(
                                                sourceLayout, source,
                                                sourceName);
          const auto otName = glyphName(namingFont, generated.codepoint);
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
    const auto regexMs =
        std::chrono::duration<double, std::milli>(regexTime).count();
    const auto targetMs =
        std::chrono::duration<double, std::milli>(targetTime).count();
    std::cout << "Compared glyphs: " << comparedGlyphs << '\n'
              << "Differing glyphs: " << differingGlyphs << '\n'
              << "Differing lines: " << differingLines << '\n'
              << "Regex justification time: " << regexMs << " ms\n"
              << (compareDeclPolicy ? "Declarative-policy justification time: "
                             : "OpenType justification time: ")
              << targetMs << " ms\n"
              << "Target/regex time ratio: " << targetMs / regexMs << '\n'
              << "Report: " << fs::absolute(output) << '\n';
    return failOnDifference && differingGlyphs != 0 ? 3 : 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
