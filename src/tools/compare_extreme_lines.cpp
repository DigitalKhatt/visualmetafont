#include <QGuiApplication>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <hb-ot.h>
#include <sqlite3.h>

#include "Layout/GlyphVis.h"
#include "Layout/HbDrawToQtPath.h"
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

struct HbFontFile {
  std::string bytes;
  hb_blob_t* blob = nullptr;
  hb_face_t* face = nullptr;
  hb_font_t* font = nullptr;

  explicit HbFontFile(const fs::path& path)
      : bytes(readFile(path)),
        blob(hb_blob_create(bytes.data(), bytes.size(), HB_MEMORY_MODE_READONLY,
                            nullptr, nullptr)),
        face(hb_face_create(blob, 0)),
        font(hb_font_create(face)) {
    if (hb_blob_get_length(blob) == 0 || hb_face_get_glyph_count(face) == 0)
      throw std::runtime_error("Invalid QPC font " + path.string());
    hb_ot_font_set_funcs(font);
    const int upem = hb_face_get_upem(face);
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
  QPainterPath path;
};

struct QpcRun {
  std::vector<QpcGlyph> glyphs;
  int advance = 0;
};

QpcRun shapeQpc(hb_font_t* font, const TextString& text,
                const HbDrawToQtPath& drawer) {
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
    QpcGlyph glyph{positions[index].x_advance,
                   positions[index].x_offset,
                   positions[index].y_offset,
                   drawer.drawGlyphPath(font, infos[index].codepoint)};
    run.advance += glyph.advance;
    run.glyphs.push_back(std::move(glyph));
  }
  hb_buffer_destroy(buffer);
  return run;
}

QPainterPath pathForKnot(mp_gr_knot first) {
  QPainterPath path;
  if (!first) return path;
  path.moveTo(first->x_coord, first->y_coord);
  auto* knot = first;
  do {
    auto* next = knot->next;
    path.cubicTo(knot->right_x, knot->right_y, next->left_x, next->left_y,
                 next->x_coord, next->y_coord);
    knot = next;
  } while (knot != first);
  if (first->data.types.left_type != mp_endpoint) path.closeSubpath();
  return path;
}

QPainterPath pathForGlyph(const GlyphVis& glyph) {
  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  for (auto* object = glyph.mpPath(); object; object = object->next) {
    if (object->type != mp_fill_code && object->type != mp_stroked_code)
      continue;
    const auto* fill = reinterpret_cast<const mp_fill_object*>(object);
    path.addPath(pathForKnot(fill->path_p));
  }
  return path;
}

void drawOldMadinaLine(QPainter& painter, OtLayout& layout,
                       const LineLayoutInfo& line, double right, double baseline,
                       double scale) {
  painter.save();
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::black);
  painter.translate(right, baseline);
  painter.scale(scale * line.xscale, -scale);
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
    if (path.isEmpty()) continue;
    painter.save();
    painter.translate(x + positioned.x_offset, positioned.y_offset);
    painter.scale(line.fontSize, line.fontSize);
    painter.drawPath(path);
    painter.restore();
  }
  painter.restore();
}

void drawQpcLine(QPainter& painter, const QpcRun& run, int targetWidth,
                 double right, double baseline, double scale) {
  if (run.advance == 0) return;
  const double qpcScale = static_cast<double>(targetWidth) / run.advance;
  painter.save();
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::black);
  painter.translate(right, baseline);
  painter.scale(scale * qpcScale, scale * qpcScale);
  double x = 0;
  for (auto glyph = run.glyphs.rbegin(); glyph != run.glyphs.rend(); ++glyph) {
    x -= glyph->advance;
    if (glyph->path.isEmpty()) continue;
    painter.save();
    painter.translate(x + glyph->xOffset, -glyph->yOffset);
    painter.drawPath(glyph->path);
    painter.restore();
  }
  painter.restore();
}

using JustifiedPages = std::map<int, std::vector<LineLayoutInfo>>;

JustifiedPages justifySelectedPages(
    OtLayout& layout, const std::vector<std::vector<QuranLine>>& pages,
    const std::vector<ExtremeLine>& underfulls,
    const std::vector<ExtremeLine>& overfulls, int count, int pageWidth) {
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

void renderPdf(const fs::path& path, OtLayout& layout,
               const std::vector<std::vector<QuranLine>>& pages,
               const std::vector<ExtremeLine>& underfulls,
               const std::vector<ExtremeLine>& overfulls,
               const JustifiedPages& justified, const fs::path& qpcFontDir,
               int count, int pageWidth) {
  QPdfWriter pdf(QString::fromStdString(path.string()));
  pdf.setResolution(144);
  pdf.setPageSize(QPageSize(QPageSize::A4));
  pdf.setPageOrientation(QPageLayout::Landscape);
  pdf.setPageMargins(QMarginsF(0, 0, 0, 0));
  pdf.setTitle("DigitalKhatt extreme-line comparison");

  QPainter painter(&pdf);
  if (!painter.isActive())
    throw std::runtime_error("Could not start PDF painter");
  painter.setRenderHint(QPainter::Antialiasing, true);

  const QRect pageRect = pdf.pageLayout().paintRectPixels(pdf.resolution());
  const double margin = 72;
  const double right = pageRect.right() - margin;
  const double usableWidth = pageRect.width() - 2 * margin;
  const double fontUnitScale = usableWidth / pageWidth;
  constexpr int itemsPerPage = 4;
  const double headerHeight = 66;
  const double itemHeight = (pageRect.height() - headerHeight - 24) / itemsPerPage;
  bool firstPhysicalPage = true;
  QpcFontCache fontCache(qpcFontDir);
  HbDrawToQtPath drawer;

  auto beginPage = [&](const QString& section, int logicalPage) {
    if (!firstPhysicalPage) pdf.newPage();
    firstPhysicalPage = false;
    painter.setPen(Qt::black);
    painter.setBrush(Qt::NoBrush);
    QFont titleFont("Helvetica", 14, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(QRectF(margin, 16, usableWidth, 30), Qt::AlignLeft,
                     section);
    QFont pageFont("Helvetica", 8);
    painter.setFont(pageFont);
    painter.drawText(QRectF(margin, 20, usableWidth, 24), Qt::AlignRight,
                     QString("comparison page %1").arg(logicalPage));
    painter.drawText(QRectF(margin, 43, usableWidth, 18), Qt::AlignLeft,
                     "Each item: DigitalKhatt justified line, then QPC reference line");
  };

  auto renderSection = [&](const QString& name,
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

      painter.setPen(QColor(190, 190, 190));
      painter.drawLine(QPointF(margin, top), QPointF(pageRect.right() - margin, top));
      painter.setPen(Qt::black);
      QFont labelFont("Helvetica", 8);
      painter.setFont(labelFont);
      const QString label =
          QString("#%1  page %2, line %3  natural %4 / target %5  %6%7%")
              .arg(index + 1)
              .arg(record.page)
              .arg(record.line)
              .arg(record.naturalWidth)
              .arg(record.targetWidth)
              .arg(record.overfull ? "+" : "-")
              .arg(record.percentage, 0, 'f', 2);
      painter.drawText(QRectF(margin, top + 7, usableWidth, 18),
                       Qt::AlignLeft, label);

      double lineRight = right;
      if (record.targetWidth != pageWidth)
        lineRight = margin + (usableWidth + record.targetWidth * fontUnitScale) / 2;
      drawOldMadinaLine(painter, layout, oldMadina, lineRight,
                        top + itemHeight * .43, fontUnitScale);
      auto& qpcFont = fontCache.get(record.page);
      const auto qpc = shapeQpc(qpcFont.font, quranLine.qpc, drawer);
      drawQpcLine(painter, qpc, record.targetWidth, lineRight,
                  top + itemHeight * .91, fontUnitScale);
    }
  };

  renderSection("Most underfull natural lines - loosest first", underfulls);
  renderSection("Most overfull natural lines - tightest first", overfulls);
  painter.end();
}

void usage(const char* program) {
  std::cerr
      << "Usage: " << program << " [options] oldmadina.mp QPC_FONT_DIR\n"
         "Rank natural DigitalKhatt line widths without justification, write "
         "underfull/overfull CSV files, then compare the most extreme justified "
         "lines with their QPC references in a PDF.\n\n"
         "Options:\n"
         "  -o, --output PATH  Output PDF (default extreme-lines.pdf)\n"
         "  --count N          Lines from each ranking in PDF (default 50)\n"
         "  --database PATH    quran-data.sqlite path\n"
         "  --resources DIR    mfplain.mp/mpost.mp/vmf.mp directory\n"
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
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
      qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    fs::path project;
    fs::path qpcFontDir;
    fs::path output{"extreme-lines.pdf"};
    fs::path database{DIGITALKHATT_QURAN_DATABASE};
    fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
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
    if (project.empty() || qpcFontDir.empty()) {
      usage(argv[0]);
      return 2;
    }
    if (count < 1) throw std::runtime_error("--count must be positive");
    if (firstPage < 1 || lastPage < firstPage || lastPage > 604)
      throw std::runtime_error("Invalid page range");
    output = fs::absolute(output);
    if (!output.parent_path().empty())
      fs::create_directories(output.parent_path());

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

    const auto justified = justifySelectedPages(
        layout, pages, rankings.underfulls, rankings.overfulls, count,
        pageWidth);
    renderPdf(output, layout, pages, rankings.underfulls, rankings.overfulls,
              justified, qpcFontDir, count, pageWidth);

    std::cout << "Underfull lines: " << rankings.underfulls.size() << '\n'
              << "Overfull lines: " << rankings.overfulls.size() << '\n'
              << "Underfull CSV: " << underfullCsv << '\n'
              << "Overfull CSV: " << overfullCsv << '\n'
              << "PDF: " << output << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
