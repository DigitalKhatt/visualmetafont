#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <hb-ot.h>
#include <glaze/glaze.hpp>
#include <sqlite3.h>
#include <png.h>

#include <digitalkhatt.h>
#include <digitalkhatt/layout/ClassMap.h>
#include <digitalkhatt/layout/OptimizeLayout.h>
#include <digitalkhatt/layout/OptParams.h>

#include "AbstractContentContext.h"
#include "EStatusCode.h"
#include "PDFPage.h"
#include "PDFRectangle.h"
#include "PDFUsedFont.h"
#include "PDFWriter.h"
#include "PageContentContext.h"
#include "Layout/GlyphVis.h"
#include "Layout/OtLayout.h"
#include "MPFont.h"
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
using PDFHummus::eSuccess;

struct SegmentedWord {
  std::string wordId;
  int wordIndex = 0;
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int gapToNextPx = 0;
  int bboxGapToNextPx = 0;
  int spacingGapToNextPx = 0;
};

struct SegmentedLine {
  int lineIndex = 0;
  int baseLine = 0;
  std::vector<SegmentedWord> words;
};

struct SegmentedPage {
  int schemaVersion = 0;
  int page = 0;
  int imageWidth = 0;
  int imageHeight = 0;
  std::vector<SegmentedLine> lines;
};

namespace {

enum class ReferenceMode { Qpc, Scan, Both };

ReferenceMode parseReferenceMode(std::string_view value) {
  if (value == "qpc") return ReferenceMode::Qpc;
  if (value == "scan" || value == "scanned") return ReferenceMode::Scan;
  if (value == "both") return ReferenceMode::Both;
  throw std::runtime_error(
      "--reference must be qpc, scan, or both (got " +
      std::string(value) + ")");
}

bool usesQpc(ReferenceMode mode) {
  return mode == ReferenceMode::Qpc || mode == ReferenceMode::Both;
}

bool usesScan(ReferenceMode mode) {
  return mode == ReferenceMode::Scan || mode == ReferenceMode::Both;
}

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

struct QuranLine {
  int page = 0;
  int line = 0;
  std::string type;
  TextString dk;
  TextString qpc;
};

std::vector<std::vector<QuranLine>> loadLines(const fs::path& database) {
  SqliteDb connection(database);
  constexpr const char* sql =
      "SELECT l.page,l.line,l.type,w.dk_v1,w.qpc_v1 "
      "FROM qpc_v1_layout AS l "
      "LEFT JOIN words w ON l.type='ayah' "
      "AND l.range_start<=w.word_number_all "
      "AND l.range_end>=w.word_number_all "
      "ORDER BY l.page,l.line,w.word_number_all";
  sqlite3_stmt* rawStatement = nullptr;
  if (sqlite3_prepare_v2(connection.db, sql, -1, &rawStatement, nullptr) !=
      SQLITE_OK)
    throw std::runtime_error(sqlite3_errmsg(connection.db));
  struct StatementGuard {
    sqlite3_stmt* value;
    ~StatementGuard() { sqlite3_finalize(value); }
  } statement{rawStatement};

  std::vector<std::vector<QuranLine>> pages;
  QuranLine* current = nullptr;
  int surah = 0;
  while (sqlite3_step(statement.value) == SQLITE_ROW) {
    const int page = sqlite3_column_int(statement.value, 0);
    const int line = sqlite3_column_int(statement.value, 1);
    const auto* typeBytes = sqlite3_column_text(statement.value, 2);
    const std::string type =
        typeBytes ? reinterpret_cast<const char*>(typeBytes) : "";
    while (static_cast<int>(pages.size()) < page) pages.emplace_back();
    if (!current || current->page != page || current->line != line) {
      pages[page - 1].push_back({page, line, type, {}, {}});
      current = &pages[page - 1].back();
      if (type == "surah_name") {
        if (surah >= static_cast<int>(surahNames.size()))
          throw std::runtime_error("Too many surah-name rows");
        current->dk = utf8ToUtf16(surahNames[surah++]);
      } else if (type == "basmallah") {
        current->dk = u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
      }
    }
    if (type != "ayah") continue;
    const auto* dkBytes = sqlite3_column_text(statement.value, 3);
    const auto* qpcBytes = sqlite3_column_text(statement.value, 4);
    TextString dk =
        dkBytes ? utf8ToUtf16(reinterpret_cast<const char*>(dkBytes))
                : TextString{};
    TextString qpc =
        qpcBytes ? utf8ToUtf16(reinterpret_cast<const char*>(qpcBytes))
                 : TextString{};
    normalizeDkV1(dk);
    if (!current->dk.empty()) current->dk.push_back(u' ');
    if (!current->qpc.empty()) current->qpc.push_back(u' ');
    current->dk += dk;
    current->qpc += qpc;
  }
  return pages;
}

const std::map<int, double> oldMadinaLineWidths{
    {1 * 15 + 2, .5},     {1 * 15 + 3, .65},   {1 * 15 + 4, .80},
    {1 * 15 + 5, .9},     {1 * 15 + 6, .80},   {1 * 15 + 7, .65},
    {1 * 15 + 8, .4},     {2 * 15 + 2, .5},    {2 * 15 + 3, .65},
    {2 * 15 + 4, .85},    {2 * 15 + 5, .9},    {2 * 15 + 6, .85},
    {2 * 15 + 7, .65},    {2 * 15 + 8, .4},    {600 * 15 + 9, .82},
    {602 * 15 + 5, .57},  {602 * 15 + 15, .55},
    {603 * 15 + 10, .63}, {604 * 15 + 9, .79}, {604 * 15 + 14, .67},
    {604 * 15 + 15, .51}};

bool isSura(const TextString& line) { return line.starts_with(u"سُورَةُ "); }
bool isBism(const TextString& line) {
  return line == u"بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" ||
         line == u"بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
}

std::vector<LineToJustify> makeLines(const std::vector<QuranLine>& text,
                                     int page, int pageWidth) {
  std::vector<LineToJustify> result;
  result.reserve(text.size());
  for (int index = 0; index < static_cast<int>(text.size()); ++index) {
    int width = pageWidth;
    auto justification = LineJustification::Distribute;
    auto type = LineType::Line;
    bool basm2 = false;
    if (isSura(text[index].dk) || isBism(text[index].dk)) {
      type = isSura(text[index].dk) ? LineType::Sura : LineType::Bism;
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
    result.push_back({text[index].dk, width, justification, type, basm2});
  }
  return result;
}

struct ExtremeLine {
  int page = 0;
  int line = 0;
  int pageLineIndex = 0;
  int naturalWidth = 0;
  int targetWidth = 0;
  double difference = 0;
  double percentage = 0;
  bool overfull = false;
};

struct Rankings {
  std::vector<ExtremeLine> underfulls;
  std::vector<ExtremeLine> overfulls;
};

Rankings rankNaturalWidths(OtLayout& layout,
                           const std::vector<std::vector<QuranLine>>& pages,
                           int firstPage, int lastPage, int pageWidth) {
  Rankings result;
  constexpr JustOption noJustification{JustType::None, JustStyle::None,
                                        ShrinkType::None};
  const double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  layout.applyJustification = false;
  for (int page = firstPage; page <= lastPage; ++page) {
    const auto input = makeLines(pages.at(page - 1), page, pageWidth);
    const auto natural = layout.justifyPage(
        scale, pageWidth, input, false, false,
        HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS, noJustification,
        "qpc_v1_layout");
    if (natural.size() != input.size())
      throw std::runtime_error("Natural shaping returned the wrong line count");
    for (int index = 0; index < static_cast<int>(natural.size()); ++index) {
      if (input[index].lineType != LineType::Line || input[index].width <= 0)
        continue;
      const int current = natural[index].currentLineWidth;
      const int target = input[index].width;
      const double difference = static_cast<double>(current) - target;
      ExtremeLine line{page,
                       pages[page - 1][index].line,
                       index,
                       current,
                       target,
                       difference,
                       std::abs(difference) * 100.0 / target,
                       difference > 0};
      if (difference > 0)
        result.overfulls.push_back(line);
      else if (difference < 0)
        result.underfulls.push_back(line);
    }
    if (page == firstPage || page == lastPage || page % 50 == 0)
      std::cout << "Measured natural widths through page " << page << '\n';
  }
  auto mostExtremeFirst = [](const ExtremeLine& a, const ExtremeLine& b) {
    if (a.percentage != b.percentage) return a.percentage > b.percentage;
    if (a.page != b.page) return a.page < b.page;
    return a.line < b.line;
  };
  std::sort(result.underfulls.begin(), result.underfulls.end(), mostExtremeFirst);
  std::sort(result.overfulls.begin(), result.overfulls.end(), mostExtremeFirst);
  return result;
}

void writeRankingCsv(const fs::path& path,
                     const std::vector<ExtremeLine>& lines) {
  std::ofstream output(path);
  if (!output) throw std::runtime_error("Could not create " + path.string());
  output << "rank,page,line,natural_width,target_width,difference,percentage\n";
  output << std::fixed << std::setprecision(4);
  for (std::size_t index = 0; index < lines.size(); ++index) {
    const auto& line = lines[index];
    output << index + 1 << ',' << line.page << ',' << line.line << ','
           << line.naturalWidth << ',' << line.targetWidth << ','
           << line.difference << ',' << line.percentage << '\n';
  }
}

class SegmentationRepository {
 public:
  explicit SegmentationRepository(fs::path root) : root_(std::move(root)) {}

  const SegmentedLine* line(int page, int lineIndex) {
    auto found = pages_.find(page);
    if (found == pages_.end())
      found = pages_.emplace(page, loadPage(page)).first;
    if (!found->second.page) return nullptr;
    const auto& lines = found->second.page->lines;
    const auto line = std::find_if(
        lines.begin(), lines.end(), [lineIndex](const SegmentedLine& value) {
          return value.lineIndex == lineIndex;
        });
    if (line == lines.end() || line->words.empty()) return nullptr;
    for (const auto& word : line->words) {
      if (word.w <= 0 || word.h <= 0 || word.wordId.empty() ||
          !fs::is_regular_file(wordImagePath(page, word)))
        return nullptr;
    }
    return &*line;
  }

  fs::path wordImagePath(int page, const SegmentedWord& word) const {
    return pageDirectory(page) / "wordimgs" / (word.wordId + ".png");
  }

 private:
  struct CachedPage {
    std::optional<SegmentedPage> page;
  };

  fs::path pageDirectory(int page) const {
    std::ostringstream name;
    name << "page" << std::setw(3) << std::setfill('0') << page;
    return root_ / name.str();
  }

  CachedPage loadPage(int page) const {
    const auto path = pageDirectory(page) / "words.json";
    if (!fs::is_regular_file(path)) return {};
    SegmentedPage result;
    const auto json = readFile(path);
    const auto error =
        glz::read<glz::opts{.error_on_unknown_keys = false}>(result, json);
    if (error)
      throw std::runtime_error("Could not parse " + path.string() + ": " +
                               glz::format_error(error, json));
    if (result.page != page)
      throw std::runtime_error("Unexpected page number in " + path.string());
    return {std::move(result)};
  }

  fs::path root_;
  std::map<int, CachedPage> pages_;
};

class TemporaryDirectory {
 public:
  TemporaryDirectory() {
    const auto stamp = std::chrono::high_resolution_clock::now()
                           .time_since_epoch()
                           .count();
    for (int attempt = 0; attempt < 100; ++attempt) {
      path_ = fs::temp_directory_path() /
              ("digitalkhatt-segments-" + std::to_string(stamp) + "-" +
               std::to_string(attempt));
      std::error_code error;
      if (fs::create_directory(path_, error)) return;
    }
    throw std::runtime_error("Could not create segmented-image cache");
  }
  ~TemporaryDirectory() {
    std::error_code error;
    fs::remove_all(path_, error);
  }
  TemporaryDirectory(const TemporaryDirectory&) = delete;
  TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

  const fs::path& path() const { return path_; }

 private:
  fs::path path_;
};

struct GrayMask {
  unsigned width = 0;
  unsigned height = 0;
  std::vector<png_byte> pixels;
};

GrayMask readGrayMask(const fs::path& source) {
  png_image input{};
  input.version = PNG_IMAGE_VERSION;
  if (!png_image_begin_read_from_file(&input, source.string().c_str()))
    throw std::runtime_error("Could not read " + source.string() + ": " +
                             input.message);
  input.format = PNG_FORMAT_GRAY;
  GrayMask result{input.width, input.height,
                  std::vector<png_byte>(PNG_IMAGE_SIZE(input))};
  if (!png_image_finish_read(&input, nullptr, result.pixels.data(), 0,
                             nullptr)) {
    const std::string message = input.message;
    png_image_free(&input);
    throw std::runtime_error("Could not decode " + source.string() + ": " +
                             message);
  }
  png_image_free(&input);
  return result;
}

void writeTransparentInk(const fs::path& destination, unsigned width,
                         unsigned height,
                         const std::vector<png_byte>& alpha) {
  if (alpha.size() != static_cast<std::size_t>(width) * height)
    throw std::runtime_error("Invalid transparent line-image dimensions");
  std::vector<png_byte> rgba(alpha.size() * 4, 0);
  for (std::size_t index = 0; index < alpha.size(); ++index)
    rgba[index * 4 + 3] = alpha[index];
  png_image output{};
  output.version = PNG_IMAGE_VERSION;
  output.width = width;
  output.height = height;
  output.format = PNG_FORMAT_RGBA;
  if (!png_image_write_to_file(&output, destination.string().c_str(), false,
                               rgba.data(), 0, nullptr)) {
    const std::string message = output.message;
    png_image_free(&output);
    throw std::runtime_error("Could not write " + destination.string() +
                             ": " + message);
  }
  png_image_free(&output);
}

struct SegmentedLineImage {
  fs::path path;
  int left = 0;
  int top = 0;
  int right = 0;
  int bottom = 0;
  double pixelsPerSourcePixel = 1;
};

class SegmentedImageCache {
 public:
  const SegmentedLineImage& image(int page, const SegmentedLine& line,
                                  const SegmentationRepository& segmentation) {
    const auto key = std::make_pair(page, line.lineIndex);
    const auto found = images_.find(key);
    if (found != images_.end()) return found->second;

    int left = line.words.front().x;
    int top = line.words.front().y;
    int right = line.words.front().x + line.words.front().w;
    int bottom = line.words.front().y + line.words.front().h;
    for (const auto& word : line.words) {
      left = std::min(left, word.x);
      top = std::min(top, word.y);
      right = std::max(right, word.x + word.w);
      bottom = std::max(bottom, word.y + word.h);
    }
    const int sourceWidth = right - left;
    const int sourceHeight = bottom - top;
    if (sourceWidth <= 0 || sourceHeight <= 0)
      throw std::runtime_error("Invalid segmented line dimensions");

    // A 4096-pixel scan line is about 383 dpi at the PDF's rendered width.
    // Keeping the original 16k-18k pixel raster adds PDF objects and decoding
    // work without a visible gain at this page size.
    constexpr int maxLineWidth = 4096;
    const double pixelsPerSourcePixel =
        std::min(1.0, static_cast<double>(maxLineWidth) / sourceWidth);
    const auto outputWidth = static_cast<unsigned>(
        std::max(1.0, std::ceil(sourceWidth * pixelsPerSourcePixel)));
    const auto outputHeight = static_cast<unsigned>(
        std::max(1.0, std::ceil(sourceHeight * pixelsPerSourcePixel)));
    std::vector<std::uint32_t> alphaSums(
        static_cast<std::size_t>(outputWidth) * outputHeight, 0);
    std::vector<unsigned> sourceColumnsPerOutput(outputWidth, 0);
    std::vector<unsigned> sourceRowsPerOutput(outputHeight, 0);
    for (int x = 0; x < sourceWidth; ++x) {
      const auto outputX = std::min(
          outputWidth - 1,
          static_cast<unsigned>(x * pixelsPerSourcePixel));
      ++sourceColumnsPerOutput[outputX];
    }
    for (int y = 0; y < sourceHeight; ++y) {
      const auto outputY = std::min(
          outputHeight - 1,
          static_cast<unsigned>(y * pixelsPerSourcePixel));
      ++sourceRowsPerOutput[outputY];
    }

    for (const auto& word : line.words) {
      const auto mask =
          readGrayMask(segmentation.wordImagePath(page, word));
      if (mask.width != static_cast<unsigned>(word.w) ||
          mask.height != static_cast<unsigned>(word.h))
        throw std::runtime_error("Segmented word dimensions do not match " +
                                 word.wordId);
      for (unsigned y = 0; y < mask.height; ++y) {
        const auto outputY = std::min(
            outputHeight - 1,
            static_cast<unsigned>((word.y - top + y) *
                                  pixelsPerSourcePixel));
        for (unsigned x = 0; x < mask.width; ++x) {
          const auto outputX = std::min(
              outputWidth - 1,
              static_cast<unsigned>((word.x - left + x) *
                                    pixelsPerSourcePixel));
          alphaSums[static_cast<std::size_t>(outputY) * outputWidth +
                    outputX] +=
              mask.pixels[static_cast<std::size_t>(y) * mask.width + x];
        }
      }
    }

    std::vector<png_byte> alpha(alphaSums.size(), 0);
    for (unsigned y = 0; y < outputHeight; ++y) {
      for (unsigned x = 0; x < outputWidth; ++x) {
        const auto area =
            sourceColumnsPerOutput[x] * sourceRowsPerOutput[y];
        if (area == 0) continue;
        const auto index = static_cast<std::size_t>(y) * outputWidth + x;
        alpha[index] = static_cast<png_byte>(
            std::min<std::uint32_t>(255, alphaSums[index] / area));
      }
    }

    std::ostringstream name;
    name << 'p' << std::setw(3) << std::setfill('0') << page << '_'
         << 'l' << std::setw(2) << line.lineIndex << ".png";
    const auto destination = directory_.path() / name.str();
    writeTransparentInk(destination, outputWidth, outputHeight, alpha);
    return images_
        .emplace(key, SegmentedLineImage{destination, left, top, right, bottom,
                                         pixelsPerSourcePixel})
        .first->second;
  }

 private:
  TemporaryDirectory directory_;
  std::map<std::pair<int, int>, SegmentedLineImage> images_;
};

struct HbFontFile {
  std::string bytes;
  hb_blob_t* blob = nullptr;
  hb_face_t* face = nullptr;
  hb_font_t* font = nullptr;
  int upem = 0;

  explicit HbFontFile(const fs::path& path)
      : bytes(readFile(path)),
        blob(hb_blob_create(bytes.data(), bytes.size(), HB_MEMORY_MODE_READONLY,
                            nullptr, nullptr)),
        face(hb_face_create(blob, 0)),
        font(hb_font_create(face)) {
    if (hb_blob_get_length(blob) == 0 || hb_face_get_glyph_count(face) == 0)
      throw std::runtime_error("Invalid QPC font " + path.string());
    hb_ot_font_set_funcs(font);
    upem = hb_face_get_upem(face);
    hb_font_set_scale(font, upem, upem);
    hb_font_set_ppem(font, upem, upem);
  }
  ~HbFontFile() {
    hb_font_destroy(font);
    hb_face_destroy(face);
    hb_blob_destroy(blob);
  }
  HbFontFile(const HbFontFile&) = delete;
  HbFontFile& operator=(const HbFontFile&) = delete;
};

struct QpcGlyph {
  int advance = 0;
  int xOffset = 0;
  int yOffset = 0;
  hb_glyph_extents_t extents{};
  bool hasExtents = false;
  std::string path;
};

struct QpcRun {
  std::vector<QpcGlyph> glyphs;
  int advance = 0;
};

class HbDrawToPdfPath {
 public:
  HbDrawToPdfPath() {
    funcs_ = hb_draw_funcs_create();
    hb_draw_funcs_set_move_to_func(funcs_, &moveTo, nullptr, nullptr);
    hb_draw_funcs_set_line_to_func(funcs_, &lineTo, nullptr, nullptr);
    hb_draw_funcs_set_quadratic_to_func(funcs_, &quadraticTo, nullptr, nullptr);
    hb_draw_funcs_set_cubic_to_func(funcs_, &cubicTo, nullptr, nullptr);
    hb_draw_funcs_set_close_path_func(funcs_, &closePath, nullptr, nullptr);
    hb_draw_funcs_make_immutable(funcs_);
  }
  ~HbDrawToPdfPath() { hb_draw_funcs_destroy(funcs_); }
  HbDrawToPdfPath(const HbDrawToPdfPath&) = delete;
  HbDrawToPdfPath& operator=(const HbDrawToPdfPath&) = delete;

  std::string drawGlyph(hb_font_t* font, hb_codepoint_t glyph) const {
    Context context;
    hb_font_draw_glyph(font, glyph, funcs_, &context);
    return context.hasPath ? context.stream.str() : std::string{};
  }

 private:
  struct Context {
    Context() { stream << std::fixed << std::setprecision(3); }
    std::ostringstream stream;
    bool hasPath = false;
    float currentX = 0;
    float currentY = 0;
  };

  static Context* context(void* data) { return static_cast<Context*>(data); }
  static void moveTo(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x,
                     float y, void*) {
    auto* ctx = context(data);
    ctx->stream << x << ' ' << y << " m\n";
    ctx->hasPath = true;
    ctx->currentX = x;
    ctx->currentY = y;
  }
  static void lineTo(hb_draw_funcs_t*, void* data, hb_draw_state_t*, float x,
                     float y, void*) {
    auto* ctx = context(data);
    ctx->stream << x << ' ' << y << " l\n";
    ctx->currentX = x;
    ctx->currentY = y;
  }
  static void quadraticTo(hb_draw_funcs_t*, void* data, hb_draw_state_t*,
                          float c1x, float c1y, float x, float y, void*) {
    auto* ctx = context(data);
    const float cubic1x = ctx->currentX + (c1x - ctx->currentX) * 2.0f / 3.0f;
    const float cubic1y = ctx->currentY + (c1y - ctx->currentY) * 2.0f / 3.0f;
    const float cubic2x = x + (c1x - x) * 2.0f / 3.0f;
    const float cubic2y = y + (c1y - y) * 2.0f / 3.0f;
    ctx->stream << cubic1x << ' ' << cubic1y << ' ' << cubic2x << ' '
                << cubic2y << ' ' << x << ' ' << y << " c\n";
    ctx->currentX = x;
    ctx->currentY = y;
  }
  static void cubicTo(hb_draw_funcs_t*, void* data, hb_draw_state_t*,
                      float c1x, float c1y, float c2x, float c2y, float x,
                      float y, void*) {
    auto* ctx = context(data);
    ctx->stream << c1x << ' ' << c1y << ' ' << c2x << ' ' << c2y << ' ' << x
                << ' ' << y << " c\n";
    ctx->currentX = x;
    ctx->currentY = y;
  }
  static void closePath(hb_draw_funcs_t*, void* data, hb_draw_state_t*, void*) {
    context(data)->stream << "h\n";
  }

  hb_draw_funcs_t* funcs_ = nullptr;
};

QpcRun shapeQpc(hb_font_t* font, const TextString& text,
                const HbDrawToPdfPath& drawer) {
  auto* buffer = hb_buffer_create();
  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", -1));
  hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
  hb_buffer_add_utf16(buffer,
                      reinterpret_cast<const std::uint16_t*>(text.data()),
                      static_cast<int>(text.size()), 0,
                      static_cast<int>(text.size()));
  hb_shape(font, buffer, nullptr, 0);
  unsigned count = 0;
  const auto* infos = hb_buffer_get_glyph_infos(buffer, &count);
  const auto* positions = hb_buffer_get_glyph_positions(buffer, &count);
  QpcRun run;
  run.glyphs.reserve(count);
  for (unsigned index = 0; index < count; ++index) {
    QpcGlyph glyph;
    glyph.advance = positions[index].x_advance;
    glyph.xOffset = positions[index].x_offset;
    glyph.yOffset = positions[index].y_offset;
    glyph.hasExtents = hb_font_get_glyph_extents(
        font, infos[index].codepoint, &glyph.extents);
    glyph.path = drawer.drawGlyph(font, infos[index].codepoint);
    run.advance += glyph.advance;
    run.glyphs.push_back(std::move(glyph));
  }
  hb_buffer_destroy(buffer);
  return run;
}

struct Bounds {
  double left = 0;
  double right = 0;
  double bottom = 0;
  double top = 0;
  bool valid = false;

  double width() const { return valid ? right - left : 0; }
  double height() const { return valid ? top - bottom : 0; }
};

Bounds qpcInkBounds(const QpcRun& run) {
  Bounds bounds;
  double x = 0;
  for (auto glyph = run.glyphs.rbegin(); glyph != run.glyphs.rend(); ++glyph) {
    x -= glyph->advance;
    if (!glyph->hasExtents) continue;
    const double x1 = x + glyph->xOffset + glyph->extents.x_bearing;
    const double x2 = x1 + glyph->extents.width;
    const double y1 = glyph->yOffset + glyph->extents.y_bearing;
    const double y2 = y1 + glyph->extents.height;
    const double left = std::min(x1, x2);
    const double right = std::max(x1, x2);
    const double bottom = std::min(y1, y2);
    const double top = std::max(y1, y2);
    if (!bounds.valid) {
      bounds = {left, right, bottom, top, true};
    } else {
      bounds.left = std::min(bounds.left, left);
      bounds.right = std::max(bounds.right, right);
      bounds.bottom = std::min(bounds.bottom, bottom);
      bounds.top = std::max(bounds.top, top);
    }
  }
  return bounds;
}

Bounds segmentedInkBounds(const SegmentedLine& line) {
  Bounds bounds;
  for (const auto& word : line.words) {
    const double left = word.x;
    const double right = word.x + word.w;
    const double bottom = word.y;
    const double top = word.y + word.h;
    if (!bounds.valid) {
      bounds = {left, right, bottom, top, true};
    } else {
      bounds.left = std::min(bounds.left, left);
      bounds.right = std::max(bounds.right, right);
      bounds.bottom = std::min(bounds.bottom, bottom);
      bounds.top = std::max(bounds.top, top);
    }
  }
  return bounds;
}

std::vector<TextString> splitWords(const TextString& text) {
  std::vector<TextString> words;
  for (std::size_t begin = 0; begin < text.size();) {
    while (begin < text.size() && text[begin] == u' ') ++begin;
    if (begin == text.size()) break;
    auto end = text.find(u' ', begin);
    if (end == TextString::npos) end = text.size();
    words.emplace_back(text.substr(begin, end - begin));
    begin = end;
  }
  return words;
}

void raw(PageContentContext* context, const std::string& commands) {
  context->WriteFreeCode(commands);
}

std::string pathForGlyph(const GlyphVis& glyph) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(3);
  bool hasPath = false;
  for (auto* object = glyph.mpPath(); object; object = object->next) {
    if (object->type != mp_fill_code && object->type != mp_stroked_code)
      continue;
    const auto* fill = reinterpret_cast<const mp_fill_object*>(object);
    auto* first = fill->path_p;
    if (!first) continue;
    hasPath = true;
    stream << first->x_coord << ' ' << first->y_coord << " m\n";
    auto* knot = first;
    do {
      auto* next = knot->next;
      stream << knot->right_x << ' ' << knot->right_y << ' ' << next->left_x
             << ' ' << next->left_y << ' ' << next->x_coord << ' '
             << next->y_coord << " c\n";
      knot = next;
    } while (knot != first);
    if (first->data.types.left_type != mp_endpoint) stream << "h\n";
  }
  return hasPath ? stream.str() : std::string{};
}

void drawOldMadinaLine(PageContentContext* context, OtLayout& layout,
                       const LineLayoutInfo& line, double right, double baseline,
                       double scale) {
  raw(context, "q\n0 g\n");
  context->cm(scale * line.xscale, 0, 0, scale, right, baseline);
  double x = 0;
  for (const auto& positioned : line.glyphs) {
    x -= positioned.x_advance;
    const auto found = layout.glyphNamePerCode.find(positioned.codepoint);
    if (found == layout.glyphNamePerCode.end()) continue;
    auto glyph = layout.glyphs.find(found->second);
    if (glyph == layout.glyphs.end()) continue;
    GlyphParameters parameters{.lefttatweel = positioned.lefttatweel,
                               .righttatweel = positioned.righttatweel,
                               .scalex = 0};
    auto* alternate = glyph->second.getAlternate(parameters);
    if (!alternate) continue;
    const auto path = pathForGlyph(*alternate);
    if (path.empty()) continue;
    raw(context, "q\n");
    context->cm(1, 0, 0, 1, x + positioned.x_offset, positioned.y_offset);
    context->cm(line.fontSize, 0, 0, line.fontSize, 0, 0);
    raw(context, path + "f\nQ\n");
  }
  raw(context, "Q\n");
}

void drawQpcLine(PageContentContext* context, const QpcRun& run, int targetWidth,
                 double right, double baseline, double scale) {
  const auto inkBounds = qpcInkBounds(run);
  if (!inkBounds.valid || inkBounds.width() <= 0) return;
  const double qpcScale =
      static_cast<double>(targetWidth) / inkBounds.width();
  const double renderedScale = scale * qpcScale;
  const double inkAlignedRight = right - renderedScale * inkBounds.right;
  raw(context, "q\n0 g\n");
  context->cm(renderedScale, 0, 0, renderedScale, inkAlignedRight, baseline);
  double x = 0;
  for (auto glyph = run.glyphs.rbegin(); glyph != run.glyphs.rend(); ++glyph) {
    x -= glyph->advance;
    if (glyph->path.empty()) continue;
    raw(context, "q\n");
    context->cm(1, 0, 0, 1, x + glyph->xOffset, glyph->yOffset);
    raw(context, glyph->path + "f\nQ\n");
  }
  raw(context, "Q\n");
}

bool drawSegmentedLine(PageContentContext* context,
                       SegmentationRepository& segmentation, int page,
                       int lineIndex, int targetWidth, double right,
                       double baseline, double fontUnitScale,
                       SegmentedImageCache& imageCache) {
  const auto* line = segmentation.line(page, lineIndex);
  if (!line) return false;
  const auto& image = imageCache.image(page, *line, segmentation);
  if (image.right <= image.left || image.bottom <= image.top ||
      line->baseLine <= 0)
    return false;
  const double sourceScale =
      targetWidth * fontUnitScale /
      static_cast<double>(image.right - image.left);
  const double imageScale = sourceScale / image.pixelsPerSourcePixel;
  AbstractContentContext::ImageOptions options;
  options.transformationMethod = AbstractContentContext::eMatrix;
  options.matrix[0] = imageScale;
  options.matrix[3] = imageScale;
  const double x = right - (image.right - image.left) * sourceScale;
  const double y =
      baseline - (image.bottom - line->baseLine) * sourceScale;
  const auto status = context->DrawImage(x, y, image.path.string(), options);
  if (status != eSuccess)
    throw std::runtime_error("Could not draw segmented line " +
                             std::to_string(lineIndex + 1) + " on page " +
                             std::to_string(page));
  return true;
}

void applyForceLayout(OtLayout& layout, std::vector<LineLayoutInfo>& page,
                      double scale) {
  using digitalkhatt::layout::GlyphInstance;
  using geometry::CUBIC_FLATNESS_TOLERANCE;
  using geometry::GeometrySet;
  using geometry::buildConvexPartsFromCubics;
  using geometry::buildPolyFromCubics;
  using geometry::getGlyphCubic;

  std::unordered_map<GlyphVis*, GeometrySet> glyphToPolys;
  const auto& classes = layout.glyphClasses();
  const auto& marks =
      digitalkhatt::layout::classesOrEmpty(classes, "marks");
  const auto& topmarks =
      digitalkhatt::layout::classesOrEmpty(classes, "topmarks");
  const auto& waqfmarks =
      digitalkhatt::layout::classesOrEmpty(classes, "waqfmarks");
  const auto& topdotmarks =
      digitalkhatt::layout::classesOrEmpty(classes, "topdotmarks");
  auto isTopMark = [&](const std::string& name) {
    return topmarks.contains(name) || waqfmarks.contains(name) ||
           topdotmarks.contains(name);
  };

  std::vector<std::vector<GlyphInstance>> pageGlyphs;
  pageGlyphs.reserve(page.size());
  for (int lineIndex = 0; lineIndex < static_cast<int>(page.size());
       ++lineIndex) {
    auto& line = page[lineIndex];
    auto& lineGlyphs = pageGlyphs.emplace_back();
    lineGlyphs.reserve(line.glyphs.size());
    const double xScale = line.fontSize * line.xscale;
    const double yScale = line.fontSize;
    double currentX = -line.xstartposition;
    const double currentY =
        -(line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY));
    GlyphInstance* currentBase = nullptr;
    GlyphInstance* previousBase = nullptr;

    for (int glyphIndex = 0;
         glyphIndex < static_cast<int>(line.glyphs.size()); ++glyphIndex) {
      auto& positioned = line.glyphs[glyphIndex];
      const auto name = layout.glyphNamePerCode.find(positioned.codepoint);
      if (name == layout.glyphNamePerCode.end())
        throw std::runtime_error("Force layout found an unnamed glyph");
      auto* glyph = layout.getGlyph(
          name->second, {.lefttatweel = positioned.lefttatweel,
                         .righttatweel = positioned.righttatweel,
                         .scalex = line.xscaleparameter});
      if (!glyph)
        throw std::runtime_error("Force layout could not load glyph " +
                                 name->second);
      auto geometry = glyphToPolys.find(glyph);
      if (geometry == glyphToPolys.end()) {
        auto polygons = marks.contains(name->second)
                            ? buildPolyFromCubics(
                                  getGlyphCubic(glyph->copiedPath),
                                  CUBIC_FLATNESS_TOLERANCE)
                            : buildConvexPartsFromCubics(
                                  getGlyphCubic(glyph->copiedPath),
                                  CUBIC_FLATNESS_TOLERANCE);
        geometry =
            glyphToPolys.emplace(glyph, polygons.scaled(scale, scale)).first;
      }

      currentX -= positioned.x_advance * line.xscale;
      auto& instance = lineGlyphs.emplace_back();
      instance.isMark = marks.contains(name->second);
      instance.isTopMark = isTopMark(name->second);
      instance.lineY = currentY;
      instance.baseX = currentX + positioned.x_offset * line.xscale;
      instance.baseY = currentY + positioned.y_offset;
      instance.glyphLayout = &positioned;
      instance.metrics =
          {glyph->width, glyph->height, glyph->bbox.llx, glyph->bbox.urx};
      instance.glyphName = name->second;
      instance.lineIndex = lineIndex;
      instance.glyphIndex = glyphIndex;
      if (xScale == 1 && yScale == 1)
        instance.geom = &geometry->second;
      else
        instance.geomScaled = geometry->second.scaled(xScale, yScale);
      instance.prevBase = currentBase;
      if (!instance.isMark) {
        previousBase = currentBase;
        currentBase = &instance;
        if (previousBase) previousBase->nextBase = currentBase;
      }
    }
  }

  auto solverClasses = classes;
  if (!solverClasses.contains("bowlbases"))
    solverClasses["bowlbases"] = {"hah.isol", "hah.fina", "ain.fina"};
  const digitalkhatt::layout::OptParams parameters;
  digitalkhatt::layout::optimizePage(pageGlyphs, solverClasses, parameters);
  for (int lineIndex = 0; lineIndex < static_cast<int>(page.size());
       ++lineIndex) {
    for (int glyphIndex = 0;
         glyphIndex < static_cast<int>(page[lineIndex].glyphs.size());
         ++glyphIndex) {
      auto& positioned = page[lineIndex].glyphs[glyphIndex];
      const auto& solved = pageGlyphs[lineIndex][glyphIndex];
      positioned.x_offset += solved.dx;
      positioned.y_offset += solved.dy;
    }
  }
}

using JustifiedPages = std::map<int, std::vector<LineLayoutInfo>>;

JustifiedPages justifySelectedPages(
    OtLayout& layout, const std::vector<std::vector<QuranLine>>& pages,
    const std::vector<ExtremeLine>& underfulls,
    const std::vector<ExtremeLine>& overfulls, int count, int pageWidth,
    bool applyForce) {
  std::set<int> selectedPages;
  for (int index = 0;
       index < std::min(count, static_cast<int>(underfulls.size())); ++index)
    selectedPages.insert(underfulls[index].page);
  for (int index = 0;
       index < std::min(count, static_cast<int>(overfulls.size())); ++index)
    selectedPages.insert(overfulls[index].page);

  constexpr JustOption justification{JustType::Experimental2,
                                      JustStyle::FontSizeXScale,
                                      ShrinkType::Standard};
  const double scale = (1 << OtLayout::SCALEBY) * OtLayout::EMSCALE;
  layout.applyJustification = true;
  JustifiedPages result;
  for (const int page : selectedPages) {
    const auto input = makeLines(pages.at(page - 1), page, pageWidth);
    auto shaped = layout.justifyPage(
        scale, pageWidth, input, false, false,
        HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS, justification,
        "qpc_v1_layout");
    if (shaped.size() != input.size())
      throw std::runtime_error("Justified shaping returned the wrong line count");
    if (applyForce) applyForceLayout(layout, shaped, scale);
    result.emplace(page, std::move(shaped));
    std::cout << "Justified selected page " << page << '\n';
  }
  return result;
}

class QpcFontCache {
 public:
  explicit QpcFontCache(fs::path directory)
      : directory_(std::move(directory)) {}
  HbFontFile& get(int page) {
    auto found = fonts_.find(page);
    if (found != fonts_.end()) return *found->second;
    std::ostringstream name;
    name << "QCF_P" << std::setw(3) << std::setfill('0') << page << ".ttf";
    auto inserted = fonts_.emplace(
        page, std::make_unique<HbFontFile>(directory_ / name.str()));
    return *inserted.first->second;
  }

 private:
  fs::path directory_;
  std::map<int, std::unique_ptr<HbFontFile>> fonts_;
};

std::string csvCell(std::string_view value) {
  if (value.find_first_of(",\"\r\n") == std::string_view::npos)
    return std::string(value);
  std::string result{"\""};
  for (const char character : value) {
    result += character;
    if (character == '"') result += '"';
  }
  result += '"';
  return result;
}

struct ScaleAverage {
  double horizontalTotal = 0;
  double verticalTotal = 0;
  std::size_t horizontalCount = 0;
  std::size_t verticalCount = 0;
  std::set<std::string> groups;

  void add(const std::optional<double>& horizontal,
           const std::optional<double>& vertical) {
    if (horizontal) {
      horizontalTotal += *horizontal;
      ++horizontalCount;
    }
    if (vertical) {
      verticalTotal += *vertical;
      ++verticalCount;
    }
  }

  std::optional<double> horizontalAverage() const {
    if (horizontalCount == 0) return std::nullopt;
    return horizontalTotal / horizontalCount;
  }

  std::optional<double> verticalAverage() const {
    if (verticalCount == 0) return std::nullopt;
    return verticalTotal / verticalCount;
  }

  std::size_t matchedCount() const {
    return std::min(horizontalCount, verticalCount);
  }
};

std::string fixedNumber(const std::optional<double>& value) {
  if (!value) return {};
  std::ostringstream output;
  output << std::fixed << std::setprecision(6) << *value;
  return output.str();
}

void writeCsvRow(std::ostream& output,
                 const std::vector<std::string>& columns) {
  for (std::size_t index = 0; index < columns.size(); ++index) {
    if (index != 0) output << ',';
    output << csvCell(columns[index]);
  }
  output << '\n';
}

void writeScaleReportSection(
    std::ostream& output, std::string_view group,
    const std::vector<ExtremeLine>& ranked,
    const std::vector<std::vector<QuranLine>>& pages,
    SegmentationRepository& segmentation, QpcFontCache& fontCache,
    const HbDrawToPdfPath& drawer, double fontUnitScale,
    std::map<int, ScaleAverage>& pageAverages) {
  for (std::size_t rank = 0; rank < ranked.size(); ++rank) {
    const auto& record = ranked[rank];
    const auto& quranLine =
        pages.at(record.page - 1).at(record.pageLineIndex);
    auto& font = fontCache.get(record.page);
    const auto qpc = shapeQpc(font.font, quranLine.qpc, drawer);
    const auto qpcBounds = qpcInkBounds(qpc);
    if (!qpcBounds.valid || qpcBounds.width() <= 0)
      throw std::runtime_error("QPC shaping produced empty ink bounds for page " +
                               std::to_string(record.page) + ", line " +
                               std::to_string(record.line));
    const double qpcScale =
        static_cast<double>(record.targetWidth) / qpcBounds.width();
    const double qpcPointScale = qpcScale * fontUnitScale;
    const auto qpcWords = splitWords(quranLine.qpc);
    std::vector<Bounds> qpcWordBounds;
    qpcWordBounds.reserve(qpcWords.size());
    for (const auto& word : qpcWords) {
      const auto wordRun = shapeQpc(font.font, word, drawer);
      qpcWordBounds.push_back(qpcInkBounds(wordRun));
    }

    const auto* scan = segmentation.line(record.page, record.pageLineIndex);
    const auto scanBounds = scan ? segmentedInkBounds(*scan) : Bounds{};
    const std::optional<double> scanScale =
        scanBounds.valid && scanBounds.width() > 0
            ? std::optional<double>(record.targetWidth * fontUnitScale /
                                    scanBounds.width())
            : std::nullopt;

    ScaleAverage lineAverage;
    const auto matchedWords =
        std::min(qpcWordBounds.size(), scan ? scan->words.size() : 0U);
    for (std::size_t wordIndex = 0; wordIndex < matchedWords; ++wordIndex) {
      const auto& bounds = qpcWordBounds[wordIndex];
      const auto& scanWord = scan->words[wordIndex];
      const std::optional<double> horizontalScale =
          scanScale && bounds.valid && bounds.width() > 0 && scanWord.w > 0
              ? std::optional<double>(bounds.width() * qpcPointScale /
                                      (scanWord.w * *scanScale))
              : std::nullopt;
      const std::optional<double> verticalScale =
          scanScale && bounds.valid && bounds.height() > 0 && scanWord.h > 0
              ? std::optional<double>(bounds.height() * qpcPointScale /
                                      (scanWord.h * *scanScale))
              : std::nullopt;
      lineAverage.add(horizontalScale, verticalScale);
      auto& pageAverage = pageAverages[record.page];
      pageAverage.add(horizontalScale, verticalScale);
      pageAverage.groups.emplace(group);
    }

    std::vector<std::string> lineColumns(11);
    lineColumns[0] = "line";
    lineColumns[1] = std::string(group);
    lineColumns[2] = std::to_string(rank + 1);
    lineColumns[3] = std::to_string(record.page);
    lineColumns[4] = std::to_string(record.line);
    lineColumns[6] = std::to_string(qpcWords.size());
    if (scan) lineColumns[7] = std::to_string(scan->words.size());
    lineColumns[8] = fixedNumber(lineAverage.horizontalAverage());
    lineColumns[9] = fixedNumber(lineAverage.verticalAverage());
    lineColumns[10] = std::to_string(lineAverage.matchedCount());
    writeCsvRow(output, lineColumns);

    const std::size_t wordCount =
        std::max(qpcWordBounds.size(), scan ? scan->words.size() : 0U);
    for (std::size_t wordIndex = 0; wordIndex < wordCount; ++wordIndex) {
      const SegmentedWord* scanWord =
          scan && wordIndex < scan->words.size() ? &scan->words[wordIndex]
                                                 : nullptr;
      const auto wordBounds = wordIndex < qpcWordBounds.size()
                                  ? qpcWordBounds[wordIndex]
                                  : Bounds{};
      const std::optional<double> wordScanRenderedWidth =
          scanScale && scanWord
              ? std::optional<double>(scanWord->w * *scanScale)
              : std::nullopt;
      const std::optional<double> wordScanRenderedHeight =
          scanScale && scanWord
              ? std::optional<double>(scanWord->h * *scanScale)
              : std::nullopt;
      const std::optional<double> horizontalScaleToScan =
          wordBounds.valid && wordBounds.width() > 0 && wordScanRenderedWidth
              ? std::optional<double>(wordBounds.width() * qpcPointScale /
                                      *wordScanRenderedWidth)
              : std::nullopt;
      const std::optional<double> verticalScaleToScan =
          wordBounds.valid && wordBounds.height() > 0 && wordScanRenderedHeight
              ? std::optional<double>(wordBounds.height() * qpcPointScale /
                                      *wordScanRenderedHeight)
              : std::nullopt;

      std::vector<std::string> wordColumns(11);
      wordColumns[0] = "word";
      wordColumns[1] = std::string(group);
      wordColumns[2] = std::to_string(rank + 1);
      wordColumns[3] = std::to_string(record.page);
      wordColumns[4] = std::to_string(record.line);
      wordColumns[5] = std::to_string(wordIndex + 1);
      wordColumns[8] = fixedNumber(horizontalScaleToScan);
      wordColumns[9] = fixedNumber(verticalScaleToScan);
      if (horizontalScaleToScan && verticalScaleToScan)
        wordColumns[10] = "1";
      writeCsvRow(output, wordColumns);
    }
  }
}

void writeScaleReport(
    const fs::path& path, const std::vector<std::vector<QuranLine>>& pages,
    const std::vector<ExtremeLine>& underfulls,
    const std::vector<ExtremeLine>& overfulls,
    SegmentationRepository& segmentation, const fs::path& qpcFontDir,
    int pageWidth) {
  std::ofstream output(path);
  if (!output) throw std::runtime_error("Could not create " + path.string());
  output << "scope,group,rank,page,line,word_index,"
            "qpc_word_count,scan_word_count,"
            "qpc_to_scan_h_scale,qpc_to_scan_v_scale,"
            "matched_word_count\n";
  output << std::fixed << std::setprecision(6);
  constexpr double pdfPageWidth = 842;
  constexpr double margin = 36;
  const double fontUnitScale = (pdfPageWidth - 2 * margin) / pageWidth;
  QpcFontCache fontCache(qpcFontDir);
  HbDrawToPdfPath drawer;
  std::map<int, ScaleAverage> pageAverages;
  writeScaleReportSection(output, "underfull", underfulls, pages,
                          segmentation, fontCache, drawer, fontUnitScale,
                          pageAverages);
  writeScaleReportSection(output, "overfull", overfulls, pages, segmentation,
                          fontCache, drawer, fontUnitScale, pageAverages);
  for (const auto& [page, average] : pageAverages) {
    std::vector<std::string> columns(11);
    columns[0] = "page";
    columns[1] = average.groups.size() == 1 ? *average.groups.begin() : "mixed";
    columns[3] = std::to_string(page);
    columns[8] = fixedNumber(average.horizontalAverage());
    columns[9] = fixedNumber(average.verticalAverage());
    columns[10] = std::to_string(average.matchedCount());
    writeCsvRow(output, columns);
  }
}

void renderPdf(const fs::path& path, OtLayout& layout,
               const std::vector<std::vector<QuranLine>>& pages,
               const std::vector<ExtremeLine>& underfulls,
               const std::vector<ExtremeLine>& overfulls,
               const JustifiedPages& justified, const fs::path& qpcFontDir,
               SegmentationRepository* segmentation, ReferenceMode mode,
               int count, int pageWidth) {
  PDFWriter writer;
  PDFCreationSettings settings(true, true);
  if (writer.StartPDF(path.string(), ePDFVersion17,
                      LogConfiguration::DefaultLogConfiguration(), settings) !=
      eSuccess)
    throw std::runtime_error("Could not create " + path.string());

  const std::vector<fs::path> labelFontCandidates{
      "/System/Library/Fonts/Supplemental/Arial.ttf",
      "/Library/Fonts/Arial.ttf",
      "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
      "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
      "C:/Windows/Fonts/arial.ttf"};
  PDFUsedFont* labelFont = nullptr;
  for (const auto& candidate : labelFontCandidates) {
    if (fs::exists(candidate)) {
      labelFont = writer.GetFontForFile(candidate.string());
      if (labelFont) break;
    }
  }

  constexpr double pageWidthPoints = 842;
  constexpr double pageHeightPoints = 595;
  constexpr double margin = 36;
  const double right = pageWidthPoints - margin;
  const double usableWidth = pageWidthPoints - 2 * margin;
  const double fontUnitScale = usableWidth / pageWidth;
  const bool useQpc = usesQpc(mode);
  const bool useSegmentation = usesScan(mode);
  const int itemsPerPage = mode == ReferenceMode::Both
                               ? 2
                               : (useSegmentation ? 3 : 4);
  constexpr double headerHeight = 33;
  const double itemHeight =
      (pageHeightPoints - headerHeight - 12) / itemsPerPage;
  std::unique_ptr<QpcFontCache> fontCache;
  if (useQpc)
    fontCache = std::make_unique<QpcFontCache>(qpcFontDir);
  std::unique_ptr<SegmentedImageCache> segmentedImages;
  if (useSegmentation)
    segmentedImages = std::make_unique<SegmentedImageCache>();
  HbDrawToPdfPath drawer;
  PDFPage* pdfPage = nullptr;
  PageContentContext* context = nullptr;
  const int totalItems =
      std::min(count, static_cast<int>(underfulls.size())) +
      std::min(count, static_cast<int>(overfulls.size()));
  int renderedItems = 0;
  const auto renderStarted = std::chrono::steady_clock::now();

  auto writeLabel = [&](double x, double y, const std::string& text,
                        double size) {
    if (!labelFont) return;
    AbstractContentContext::TextOptions options(
        labelFont, size, AbstractContentContext::eRGB, 0x000000);
    context->WriteText(x, y, text, options);
  };
  auto endPage = [&]() {
    if (!context) return;
    writer.EndPageContentContext(context);
    writer.WritePageReleaseAndReturnPageID(pdfPage);
    context = nullptr;
    pdfPage = nullptr;
  };
  auto beginPage = [&](const std::string& section, int logicalPage) {
    endPage();
    pdfPage = new PDFPage();
    pdfPage->SetMediaBox(
        PDFRectangle(0, 0, pageWidthPoints, pageHeightPoints));
    context = writer.StartPageContentContext(pdfPage);
    if (!context) throw std::runtime_error("Could not start PDF page");
    writeLabel(margin, pageHeightPoints - 19, section, 14);
    writeLabel(pageWidthPoints - margin - 96, pageHeightPoints - 17,
               "comparison page " + std::to_string(logicalPage), 8);
    writeLabel(margin, pageHeightPoints - 29,
               mode == ReferenceMode::Both
                   ? "Each item: DigitalKhatt, segmented scan, then QPC (QPC second when scan unavailable)"
               : useSegmentation
                   ? "Each item: DigitalKhatt justified line, then segmented scan line"
                   : "Each item: DigitalKhatt justified line, then QPC reference line",
               8);
  };

  auto renderSection = [&](const std::string& name,
                           const std::vector<ExtremeLine>& ranked) {
    const int selected = std::min(count, static_cast<int>(ranked.size()));
    for (int index = 0; index < selected; ++index) {
      const int slot = index % itemsPerPage;
      if (slot == 0) beginPage(name, index / itemsPerPage + 1);
      const auto& record = ranked[index];
      const auto& quranLine = pages.at(record.page - 1).at(record.pageLineIndex);
      const auto pageFound = justified.find(record.page);
      if (pageFound == justified.end())
        throw std::runtime_error("Missing justified page");
      const auto& oldMadina = pageFound->second.at(record.pageLineIndex);
      const double top = headerHeight + slot * itemHeight;
      const double separatorY = pageHeightPoints - top;
      {
        std::ostringstream commands;
        commands << "q\n0.75 G\n0.5 w\n" << margin << ' ' << separatorY
                 << " m\n" << pageWidthPoints - margin << ' ' << separatorY
                 << " l\nS\nQ\n";
        raw(context, commands.str());
      }
      std::ostringstream label;
      label << '#' << index + 1 << "  page " << record.page << ", line "
            << record.line << "  natural " << record.naturalWidth
            << " / target " << record.targetWidth << "  "
            << (record.overfull ? '+' : '-') << std::fixed
            << std::setprecision(2) << record.percentage << '%';
      writeLabel(margin, pageHeightPoints - top - 11, label.str(), 8);

      double lineRight = right;
      if (record.targetWidth != pageWidth)
        lineRight = margin + (usableWidth + record.targetWidth * fontUnitScale) / 2;
      const double digitalBaseline =
          mode == ReferenceMode::Both ? .30 : .43;
      drawOldMadinaLine(context, layout, oldMadina, lineRight,
                        pageHeightPoints - (top + itemHeight * digitalBaseline),
                        fontUnitScale);
      bool scanDrawn = false;
      if (useSegmentation) {
        const double scanBaseline =
            mode == ReferenceMode::Both ? .59 : .82;
        const double scanY =
            pageHeightPoints - (top + itemHeight * scanBaseline);
        scanDrawn = drawSegmentedLine(
            context, *segmentation, record.page, record.pageLineIndex,
            record.targetWidth, lineRight, scanY, fontUnitScale,
            *segmentedImages);
        if (!scanDrawn && mode != ReferenceMode::Both)
          writeLabel(margin, scanY,
                     "Segmented scan unavailable for page " +
                         std::to_string(record.page) + ", line " +
                         std::to_string(record.line),
                     8);
      }
      if (useQpc) {
        auto& qpcFont = fontCache->get(record.page);
        const auto qpc = shapeQpc(qpcFont.font, quranLine.qpc, drawer);
        const double qpcBaseline =
            mode == ReferenceMode::Both ? (scanDrawn ? .86 : .59) : .91;
        drawQpcLine(context, qpc, record.targetWidth, lineRight,
                    pageHeightPoints - (top + itemHeight * qpcBaseline),
                    fontUnitScale);
      }
      ++renderedItems;
      if (renderedItems == totalItems || renderedItems % 10 == 0)
        std::cout << "Rendered comparison PDF items: " << renderedItems << '/'
                  << totalItems << '\n';
    }
  };

  renderSection("Most underfull natural lines - loosest first", underfulls);
  renderSection("Most overfull natural lines - tightest first", overfulls);
  endPage();
  if (writer.EndPDF() != eSuccess)
    throw std::runtime_error("Could not finalize " + path.string());
  const auto renderSeconds = std::chrono::duration<double>(
      std::chrono::steady_clock::now() - renderStarted);
  std::cout << "Rendered comparison PDF in " << std::fixed
            << std::setprecision(2) << renderSeconds.count() << " seconds\n";
}

void usage(const char* program) {
  std::cerr
      << "Usage: " << program
      << " [options] oldmadina.mp [QPC_FONT_DIR]\n"
         "Rank natural DigitalKhatt line widths without justification, write "
         "underfull/overfull CSV files, then compare the most extreme justified "
         "lines with QPC references or segmented scan words in a PDF.\n\n"
         "Options:\n"
         "  -o, --output PATH  Output PDF (default extreme-lines.pdf)\n"
         "  --count N          Lines from each ranking in PDF (default 50)\n"
         "  --database PATH    quran-data.sqlite path\n"
         "  --resources DIR    mfplain.mp/mpost.mp/vmf.mp directory\n"
         "  --reference MODE   Reference source: qpc, scan, or both\n"
         "  --segmentation DIR Use DIR/pageNNN/words.json and wordimgs for "
         "scan references\n"
         "  --scale-report PATH Write line/word QPC-vs-scan scale CSV "
         "(requires --reference both)\n"
         "  --force            Apply automatic mark positioning (default)\n"
         "  --no-force         Disable automatic mark positioning\n"
         "  --pages A[-B]      Analyze only this page or inclusive range\n"
         "  -h, --help         Show this help\n";
}

fs::path rankingPath(const fs::path& pdf, std::string_view suffix) {
  const auto stem = pdf.stem().string();
  return pdf.parent_path() / (stem + std::string(suffix) + ".csv");
}

}  // namespace

int main(int argc, char** argv) {
  try {
    fs::path project;
    fs::path qpcFontDir;
    fs::path segmentationDir;
    fs::path scaleReport;
    fs::path output{"extreme-lines.pdf"};
    fs::path database{DIGITALKHATT_QURAN_DATABASE};
    fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    std::optional<ReferenceMode> requestedMode;
    bool applyForce = true;
    int count = 50;
    int firstPage = 1;
    int lastPage = 604;
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
      } else if (arg == "--count") {
        count = std::stoi(value());
      } else if (arg == "--database") {
        database = value();
      } else if (arg == "--resources") {
        resources = value();
      } else if (arg == "--reference") {
        requestedMode = parseReferenceMode(value());
      } else if (arg == "--segmentation") {
        segmentationDir = fs::absolute(value());
      } else if (arg == "--scale-report") {
        scaleReport = fs::absolute(value());
      } else if (arg == "--force") {
        applyForce = true;
      } else if (arg == "--no-force") {
        applyForce = false;
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
      } else if (qpcFontDir.empty()) {
        qpcFontDir = fs::absolute(arg);
      } else {
        throw std::runtime_error("Too many positional arguments");
      }
    }
    if (project.empty()) {
      usage(argv[0]);
      return 2;
    }
    const ReferenceMode mode = requestedMode.value_or(
        segmentationDir.empty() ? ReferenceMode::Qpc : ReferenceMode::Scan);
    if (usesQpc(mode) && qpcFontDir.empty())
      throw std::runtime_error(
          "QPC_FONT_DIR is required for --reference qpc or both");
    if (usesScan(mode) && segmentationDir.empty())
      throw std::runtime_error(
          "--segmentation DIR is required for --reference scan or both");
    if (!usesQpc(mode) && !qpcFontDir.empty())
      throw std::runtime_error(
          "QPC_FONT_DIR is only used with --reference qpc or both");
    if (!usesScan(mode) && !segmentationDir.empty())
      throw std::runtime_error(
          "--segmentation is only used with --reference scan or both");
    if (!scaleReport.empty() && mode != ReferenceMode::Both)
      throw std::runtime_error("--scale-report requires --reference both");
    if (count < 1) throw std::runtime_error("--count must be positive");
    if (firstPage < 1 || lastPage < firstPage || lastPage > 604)
      throw std::runtime_error("Invalid page range");
    output = fs::absolute(output);
    if (!output.parent_path().empty())
      fs::create_directories(output.parent_path());
    if (!scaleReport.empty() && !scaleReport.parent_path().empty())
      fs::create_directories(scaleReport.parent_path());

    MPFont mpFont;
    initializeFont(mpFont, project, resources);
    OtLayout layout(&mpFont, true, true);
    layout.useNormAxisValues = false;
    layout.quantizeGlyphAdvances = true;
    layout.loadLookupFile("features.fea");

    const auto pages = loadLines(database);
    if (lastPage > static_cast<int>(pages.size()))
      throw std::runtime_error("Page range exceeds database");
    const int pageWidth = OtLayout::TextWidth << OtLayout::SCALEBY;
    auto rankings =
        rankNaturalWidths(layout, pages, firstPage, lastPage, pageWidth);
    const auto underfullCsv = rankingPath(output, "-underfulls");
    const auto overfullCsv = rankingPath(output, "-overfulls");
    writeRankingCsv(underfullCsv, rankings.underfulls);
    writeRankingCsv(overfullCsv, rankings.overfulls);

    std::unique_ptr<SegmentationRepository> segmentation;
    std::vector<ExtremeLine> comparisonUnderfulls;
    std::vector<ExtremeLine> comparisonOverfulls;
    const auto underfullCount =
        std::min(count, static_cast<int>(rankings.underfulls.size()));
    const auto overfullCount =
        std::min(count, static_cast<int>(rankings.overfulls.size()));
    comparisonUnderfulls.assign(rankings.underfulls.begin(),
                                rankings.underfulls.begin() + underfullCount);
    comparisonOverfulls.assign(rankings.overfulls.begin(),
                               rankings.overfulls.begin() + overfullCount);
    if (usesScan(mode)) {
      segmentation =
          std::make_unique<SegmentationRepository>(segmentationDir);
      auto missingScans = [&](const std::vector<ExtremeLine>& lines) {
        return std::count_if(lines.begin(), lines.end(), [&](const auto& line) {
          return !segmentation->line(line.page, line.pageLineIndex);
        });
      };
      std::cout << "Underfull comparisons without scans (DigitalKhatt kept): "
                << missingScans(comparisonUnderfulls) << '\n'
                << "Overfull comparisons without scans (DigitalKhatt kept): "
                << missingScans(comparisonOverfulls) << '\n';
    }
    if (!scaleReport.empty())
      writeScaleReport(scaleReport, pages, comparisonUnderfulls,
                       comparisonOverfulls, *segmentation, qpcFontDir,
                       pageWidth);

    const auto justified = justifySelectedPages(
        layout, pages, comparisonUnderfulls, comparisonOverfulls, count,
        pageWidth, applyForce);
    renderPdf(output, layout, pages, comparisonUnderfulls,
              comparisonOverfulls, justified, qpcFontDir,
              segmentation.get(), mode, count, pageWidth);

    std::cout << "DigitalKhatt automatic mark positioning: "
              << (applyForce ? "enabled" : "disabled") << '\n'
              << "Underfull lines: " << rankings.underfulls.size() << '\n'
              << "Overfull lines: " << rankings.overfulls.size() << '\n'
              << "Underfull CSV: " << underfullCsv << '\n'
              << "Overfull CSV: " << overfullCsv << '\n'
              << (scaleReport.empty()
                      ? std::string{}
                      : "Scale report: " + scaleReport.string() + "\n")
              << "PDF: " << output << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
