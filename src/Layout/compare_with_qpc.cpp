
#include <QFile>
#include <QTreeView>
#include <QtSql>
#include <QtWidgets>

#include "JustificationContext.h"
#include "LayoutWindow.h"
#include "hb-ot-layout-gsub-table.hh"

// #include "hb-font.hh"
// #include "hb-buffer.hh"
// #include "hb-ft.hh"

#include "GraphicsSceneAdjustment.h"
#include "GraphicsViewAdjustment.h"
#include "font.hpp"
#include "glyph.hpp"
#include "qurantext/quran.h"
// #include <QGLWidget>

#include <vector>

#include "GlyphItem.h"
#include "GlyphVis.h"
#include "Lookup.h"
#include "automedina/automedina.h"
#include "qpoint.h"

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include <Subtable.h>
#include <math.h>

#include <QPrinter>
#include <chrono>
#include <iostream>
#include <set>
#include <unordered_set>

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"
#include "HbDrawToQtPath.h"
#include "gllobal_strings.h"

struct LineKey {
  int page = 0;
  int line = 0;

  bool operator<(const LineKey& o) const {
    if (page != o.page) return page < o.page;
    return line < o.line;
  }
};

struct WordRow {
  int page = 0;
  int line = 0;
  QString qpc;
  QString dk;
};

struct LineData {
  LineKey key;
  QVector<WordRow> words;
  long width = 0;
  QString type;
};

static QVector<LineData> loadLinesFromDb(int pageNumber) {
  QString sql =
      "select l.page,l.line,w.qpc_v1,w.dk_v1,l.type "
      "from qpc_v1_layout l "
      "LEFT JOIN words w "
      "  ON l.type = 'ayah' "
      " AND l.range_start <= w.word_number_all "
      " AND l.range_end   >= w.word_number_all ";

  if (pageNumber >= 1) {
    sql = sql + QString(" WHERE page = %1").arg(pageNumber);
  }
  sql = sql + " ORDER BY l.page, l.line, w.word_number_all;";

  QSqlQuery query(sql);

  QMap<LineKey, LineData> map;
  while (query.next()) {
    WordRow r;
    r.page = query.value(0).toInt();
    r.line = query.value(1).toInt();
    r.qpc = query.value(2).toString();
    r.dk = query.value(3).toString();
    QString type = query.value(4).toString();

    // if (type == "ayah") {
    LineKey key{r.page, r.line};
    if (!map.contains(key)) {
      LineData ld;
      ld.type = type;
      ld.key = key;
      map.insert(key, ld);
    }
    map[key].words.push_back(r);
  }

  return map.values().toVector();
}

struct HbOtFont {
  hb_blob_t* blob = nullptr;
  hb_face_t* face = nullptr;
  hb_font_t* font = nullptr;

  HbOtFont() = default;

  explicit HbOtFont(const QString& fontPath) {
    QFile f(fontPath);
    if (!f.open(QIODevice::ReadOnly)) {
      throw std::runtime_error(("Failed to open font: " + fontPath).toStdString());
    }
    QByteArray data = f.readAll();
    f.close();
    if (data.isEmpty()) {
      throw std::runtime_error(("Font file empty: " + fontPath).toStdString());
    }

    // Keep the data alive via a heap allocation and custom destroy func.
    auto* heapData = new QByteArray(std::move(data));
    blob = hb_blob_create(heapData->constData(),
                          (unsigned)heapData->size(),
                          HB_MEMORY_MODE_READONLY,
                          heapData,
                          [](void* user_data) {
                            delete static_cast<QByteArray*>(user_data);
                          });

    face = hb_face_create(blob, 0);
    if (!face) throw std::runtime_error(("hb_face_create failed: " + fontPath).toStdString());

    font = hb_font_create(face);
    if (!font) throw std::runtime_error(("hb_font_create failed: " + fontPath).toStdString());

    // Let HarfBuzz use OpenType tables directly for metrics and glyph mapping.
    hb_ot_font_set_funcs(font);

    // Set scale. Use UPEM to get advances in font units.
    m_upem = (int)hb_face_get_upem(face);
    hb_font_set_scale(font, m_upem, m_upem);
    hb_font_set_ppem(font, 0, 0);  // optional
  }

  ~HbOtFont() {
    if (font) hb_font_destroy(font);
    if (face) hb_face_destroy(face);
    if (blob) hb_blob_destroy(blob);
    font = nullptr;
    face = nullptr;
    blob = nullptr;
  }

  unsigned upem() {
    return m_upem;
  }

  HbOtFont(const HbOtFont&) = delete;
  HbOtFont& operator=(const HbOtFont&) = delete;

  HbOtFont(HbOtFont&& o) noexcept {
    blob = o.blob;
    o.blob = nullptr;
    face = o.face;
    o.face = nullptr;
    font = o.font;
    o.font = nullptr;
  }
  HbOtFont& operator=(HbOtFont&& o) noexcept {
    if (this == &o) return *this;
    this->~HbOtFont();
    blob = o.blob;
    o.blob = nullptr;
    face = o.face;
    o.face = nullptr;
    font = o.font;
    o.font = nullptr;
    return *this;
  }

 private:
  unsigned m_upem = 1000;
};

static long hbAdvance(hb_font_t* font, const QString& text,
                      const QVector<hb_feature_t>& features,
                      hb_script_t script,
                      hb_direction_t dir,
                      hb_language_t lang) {
  hb_buffer_t* buf = hb_buffer_create();
  hb_buffer_set_direction(buf, dir);
  hb_buffer_set_script(buf, script);
  hb_buffer_set_language(buf, lang);

  hb_buffer_add_utf16(buf,
                      reinterpret_cast<const uint16_t*>(text.utf16()),
                      text.size(),
                      0, text.size());

  hb_shape(font, buf, features.isEmpty() ? nullptr : features.data(),
           (unsigned)features.size());

  unsigned int n = 0;
  hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &n);

  long width = 0;
  for (unsigned int i = 0; i < n; ++i) width += pos[i].x_advance;

  hb_buffer_destroy(buf);
  return width;
}

static QString toHexCodepoints(const QString& s) {
  QStringList parts;
  for (int i = 0; i < s.size();) {
    uint32_t cp = s.at(i).unicode();
    if (s.at(i).isHighSurrogate() && i + 1 < s.size() && s.at(i + 1).isLowSurrogate()) {
      cp = QChar::surrogateToUcs4(s.at(i), s.at(i + 1));
      i += 2;
    } else {
      i += 1;
    }
    parts << QString("U+%1").arg(cp, 4, 16, QLatin1Char('0')).toUpper();
  }
  return parts.join(' ');
}

static int quantizeSpace(double gapFU, double gMinFU, double gMaxFU, double stepFU, int K) {
  if (K <= 1) return 0;
  double g = std::max(gMinFU, std::min(gMaxFU, gapFU));
  int idx = (int)std::llround((g - gMinFU) / stepFU);
  idx = std::max(0, std::min(K - 1, idx));
  return idx;
}

struct WordDiff {
  int idx = 0;
  QString qpc;
  QString dk;

  double pkgRef = 0.0;
  double wCurWord = 0.0;
  double gapTarget = 0.0;
  int spaceIndex = -1;
  bool isWeirdRef = false;
};

static void writeCsv(const QString& path, const LineData& line,
                     const QVector<WordDiff>& diffs) {
  QFile f(path);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) return;
  QTextStream out(&f);
  out << "page,line,word_idx,qpc_hex,dk,pkgRef,wCurWord,gapTarget,spaceIndex,isWeirdRef\n";
  for (const auto& d : diffs) {
    out << line.key.page << "," << line.key.line << "," << d.idx << ","
        << "\"" << toHexCodepoints(d.qpc) << "\"" << ","
        << "\"" << QString(d.dk).replace("\"", "\"\"") << "\"" << ","
        << QString::number(d.pkgRef, 'f', 2) << ","
        << QString::number(d.wCurWord, 'f', 2) << ","
        << QString::number(d.gapTarget, 'f', 2) << ","
        << d.spaceIndex << ","
        << (d.isWeirdRef ? "1" : "0")
        << "\n";
  }
  f.close();
}

static inline QString pageKey(int page) {
  return QString("%1").arg(page, 3, 10, QChar('0'));
}

static inline QString lineKey(int line) {
  return QString("%1").arg(line, 2, 10, QChar('0'));
}

QJsonDocument lineWidthsJson(QVector<LineData> lines) {
  // Ensure numeric order before insertion
  std::sort(lines.begin(), lines.end(),
            [](const LineData& a, const LineData& b) {
              return a.key < b.key;
            });

  QJsonObject root;

  for (const LineData& ld : lines) {
    const QString pKey = pageKey(ld.key.page);
    const QString lKey = lineKey(ld.key.line);

    // Get or create page object
    QJsonObject pageObj = root.value(pKey).toObject();

    // Insert line width
    pageObj.insert(lKey, static_cast<qint64>(ld.width));

    // Re-insert page object (QJsonObject is copy-on-write)
    root.insert(pKey, pageObj);
  }

  return QJsonDocument(root);
}

void LayoutWindow::compareWithQPC() {
  QVector<LineData> lines;

  try {
    lines = loadLinesFromDb(-1);
  } catch (const std::exception& e) {
    qCritical() << "DB error:" << e.what();
  }

  const hb_script_t script = HB_SCRIPT_ARABIC;
  const hb_direction_t dir = HB_DIRECTION_RTL;
  const hb_language_t lang = hb_language_from_string("ar", -1);

  // Reference fonts per page (cache)
  std::unordered_map<int, std::unique_ptr<HbOtFont>> refCache;

  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString refDir = fileInfo.path() + "/output/fonts";
  const QString curFontPath = fileInfo.path() + "/output/oldmadina.otf";
  const QString outDir = fileInfo.path() + "/output/analyze_result";

  // Quantization knobs (tune after first run; units are font units)
  const double gMinFU = 0.0;
  const double gMaxFU = 300.0;
  const double stepFU = 5.0;
  const int Kspaces = (int)std::floor((gMaxFU - gMinFU) / stepFU) + 1;

  auto getRef = [&](int page) -> hb_font_t* {
    auto it = refCache.find(page);
    if (it != refCache.end()) return it->second->font;

    QString refPath = QDir(refDir).filePath(QString("QCF_P%1.ttf").arg(page, 3, 10, QLatin1Char('0')));
    if (!QFileInfo::exists(refPath)) {
      throw std::runtime_error(("Missing ref font: " + refPath).toStdString());
    }

    auto ref = std::make_unique<HbOtFont>(refPath);
    hb_font_t* hbF = ref->font;
    refCache.emplace(page, std::move(ref));
    return hbF;
  };

  // Current font once
  std::unique_ptr<HbOtFont> curFont;
  try {
    curFont = std::make_unique<HbOtFont>(curFontPath);
  } catch (const std::exception& e) {
    qCritical() << "Current font load error:" << e.what();
    return;
  }

  // line widths
  for (auto& line : lines) {
    if (line.type != "ayah") continue;
    hb_font_t* refHb = nullptr;
    try {
      refHb = getRef(line.key.page);
    } catch (const std::exception& e) {
      qWarning() << "Skip page" << line.key.page << "line" << line.key.line << ":" << e.what();
      continue;
    }

    QString text;

    for (int j = 0; j < line.words.size(); ++j) {
      const auto& w = line.words[j];
      text += w.qpc;
    }

    line.width = hbAdvance(refHb, text, {}, script, dir, lang);
  }

  for (const auto& line : lines) {
    if (line.type != "ayah") continue;
    if (line.words.isEmpty()) continue;

    hb_font_t* refHb = nullptr;
    try {
      refHb = getRef(line.key.page);
    } catch (const std::exception& e) {
      qWarning() << "Skip page" << line.key.page << "line" << line.key.line << ":" << e.what();
      continue;
    }

    QVector<WordDiff> diffs;
    diffs.reserve(line.words.size());

    QVector<hb_feature_t> curFeatures;
    QVector<hb_feature_t> refFeatures;

    // Per-word reference package widths and current word widths
    for (int j = 0; j < line.words.size(); ++j) {
      const auto& w = line.words[j];

      WordDiff d;
      d.idx = j;
      d.qpc = w.qpc;
      d.dk = w.dk;
      d.isWeirdRef = (w.qpc.size() > 1);

      d.pkgRef = hbAdvance(refHb, w.qpc, refFeatures, script, dir, lang);
      d.wCurWord = hbAdvance(curFont->font, w.dk, curFeatures, script, dir, lang);

      diffs.push_back(d);
    }

    // Gap targets (space needed after each word) for all but last word
    for (int j = 0; j < diffs.size(); ++j) {
      if (j == diffs.size() - 1) {
        diffs[j].gapTarget = 0.0;
        diffs[j].spaceIndex = -1;
        continue;
      }
      const double gapFU = diffs[j].pkgRef - diffs[j].wCurWord;
      diffs[j].gapTarget = gapFU;

      // If gapFU < 0, spacing can't fix it (word too wide); still set index=0 for now.
      diffs[j].spaceIndex = (gapFU <= 0.0) ? 0 : quantizeSpace(gapFU, gMinFU, gMaxFU, stepFU, Kspaces);
    }

    /*
    QString base = QString("P%1_L%2")
                       .arg(line.key.page, 3, 10, QLatin1Char('0'))
                       .arg(line.key.line, 3, 10, QLatin1Char('0'));

    writeCsv(QDir(outDir).filePath(base + ".csv"), line, diffs);*/
  }

  QFile lineWidthsFile(outDir + "/line_widths.json");

  if (lineWidthsFile.open(QIODevice::WriteOnly)) {
    auto root = lineWidthsJson(lines);
    lineWidthsFile.write(root.toJson(QJsonDocument::JsonFormat::Indented));
  }
}

struct ShapedGlyph {
  hb_codepoint_t gid = 0;
  uint32_t cluster = 0;  // UTF-16 index (since we feed UTF-16)
  int x_advance = 0;
  int y_advance = 0;
  int x_offset = 0;
  int y_offset = 0;
  QPainterPath path;
};

struct ShapedRun {
  QString text;
  std::vector<ShapedGlyph> glyphs;
  std::vector<int> x_cursor;  // x at each glyph start (font units)
  int totalAdvance = 0;
};

struct ShapeConfig {
  hb_direction_t dir = HB_DIRECTION_RTL;
  hb_script_t script = HB_SCRIPT_ARABIC;
  hb_language_t lang = hb_language_from_string("ar", -1);
  std::vector<hb_feature_t> features;  // e.g. kern=1, liga=1, etc.
};

static ShapedRun shapeText(hb_font_t* font, const QString& text, const ShapeConfig& cfg) {
  static HbDrawToQtPath drawer;

  assert(font);

  hb_buffer_t* buf = hb_buffer_create();
  hb_buffer_set_direction(buf, cfg.dir);
  hb_buffer_set_script(buf, cfg.script);
  hb_buffer_set_language(buf, cfg.lang);

  hb_buffer_set_cluster_level(buf, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

  // Add UTF-16 from QString. clusters become UTF-16 indices.
  hb_buffer_add_utf16(buf,
                      reinterpret_cast<const uint16_t*>(text.utf16()),
                      (int)text.size(),
                      0,
                      (int)text.size());

  // hb_buffer_guess_segment_properties(buf);

  hb_shape(font, buf,
           cfg.features.empty() ? nullptr : cfg.features.data(),
           (unsigned)cfg.features.size());

  unsigned len = 0;
  hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buf, &len);
  hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buf, &len);

  ShapedRun out;
  out.text = text;
  out.glyphs.reserve(len);
  out.x_cursor.reserve(len);

  int x = 0;
  for (unsigned i = 0; i < len; i++) {
    out.x_cursor.push_back(x);

    ShapedGlyph g;
    g.gid = infos[i].codepoint;
    g.cluster = infos[i].cluster;
    g.x_advance = pos[i].x_advance;
    g.y_advance = pos[i].y_advance;
    g.x_offset = pos[i].x_offset;
    g.y_offset = pos[i].y_offset;
    g.path = drawer.drawGlyphPath(font, g.gid);

    x += g.x_advance;

    out.glyphs.push_back(std::move(g));
  }
  out.totalAdvance = x;

  hb_buffer_destroy(buf);
  return out;
}

void LayoutWindow::compareWithOldMadinah() {
  bool IsImage = false;
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString refDir = fileInfo.path() + "/output/mushafanalyzeroutput/wordseg";

  std::unordered_map<int, std::unique_ptr<HbOtFont>> refCache;

  int pageNumber = integerSpinBox->value();

  auto getFont = [&](int page) -> HbOtFont* {
    auto it = refCache.find(page);
    if (it != refCache.end()) return it->second.get();

    auto pageName = QString("page%1").arg(page, 3, 10, QLatin1Char('0'));

    QString refPath = QDir(refDir).filePath(pageName + "/" + pageName + ".ttf");

    if (!QFileInfo::exists(refPath)) {
      throw std::runtime_error(("Missing ref font: " + refPath).toStdString());
    }

    auto ref = std::make_unique<HbOtFont>(refPath);
    auto tt = refCache.emplace(page, std::move(ref));
    return tt.first->second.get();
  };

  QList<LineLayoutInfo> page;

  {
    int lineWidth = OtLayout::TextWidth << OtLayout::SCALEBY;

    double emScale = OtLayout::EMSCALE;
    QString textt = currentQuranText[pageNumber - 1];

    auto lines = textt.split(char(10), Qt::SkipEmptyParts);

    auto tempPage = m_otlayout->justifyPage(
        emScale, lineWidth, lineWidth, lines, LineJustification::Distribute,
        false, tajweedEnabled,
        HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS,
        getJustOption(),
        mushafLayouts->currentText());

    QList<QList<LineLayoutInfo>> pages = {tempPage};

    QList<QStringList> originalPages = {lines};

    if (this->applyForce) {
      optimizeLayout(pages, originalPages, 0, 1, emScale);
    }

    page = std::move(pages[0]);
  }

  QVector<LineData> lines;

  try {
    lines = loadLinesFromDb(pageNumber);
  } catch (const std::exception& e) {
    qCritical() << "DB error:" << e.what();
  }

  auto currChar = QChar(0xFC40);

  for (auto& line : lines) {
    for (int j = 0; j < line.words.size(); ++j) {
      auto& w = line.words[j];
      w.qpc = currChar;
      currChar = QChar(currChar.unicode() + 1);
    }
  }

  auto outputFileName = fileInfo.path() + "/output/page_compare" + QString("%1").arg(integerSpinBox->value());

  if (IsImage) {
    outputFileName = outputFileName + ".jpg";
  } else {
    outputFileName = outputFileName + ".pdf";
  }

  // PDF setup
  QPdfWriter pdf(outputFileName);

  int overflowMargin = 5000;

  int pageWpx = OtLayout::FrameWidth + overflowMargin;
  int pageHpx = 2 * OtLayout::FrameHeight;
  double yPos = OtLayout::TopSpace;
  double xMargin = OtLayout::Margin;
  double interLine = OtLayout::InterLineSpacing;
  double dpi = 300.0;
  double unitsToPt = 1;  // 50 / upem;
  auto xStartRightToLeft = pageWpx - xMargin;

  QImage image(QSize(pageWpx, pageHpx), QImage::Format_ARGB32);
  QPainter painter;

  if (IsImage) {
    image.fill(Qt::white);
    painter.begin(&image);
  } else {
    double pageWIn = pageWpx / dpi;
    double pageHIn = pageHpx / dpi;

    pdf.setPageSize(QPageSize(QSizeF(pageWIn, pageHIn), QPageSize::Unit::Inch));
    pdf.setResolution(dpi);
    pdf.setTitle("Font Compare Report");
    painter.begin(&pdf);
  }

  if (!painter.isActive()) throw std::runtime_error("Failed to start QPainter on QPdfWriter.");

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::black);

  QVector<ShapedRun> originialPage;

  // line widths
  for (auto& line : lines) {
    HbOtFont* font = nullptr;
    try {
      font = getFont(line.key.page);
    } catch (const std::exception& e) {
      qWarning() << "Skip page" << line.key.page << "line" << line.key.line << ":" << e.what();
      continue;
    }

    QString text;

    for (int j = 0; j < line.words.size(); ++j) {
      const auto& w = line.words[j];
      text += w.qpc;
    }

    ShapeConfig shapeConfig;

    auto shapeRun = shapeText(font->font, text, shapeConfig);
    originialPage.append(shapeRun);
  }

  for (int lineIndex = 0; lineIndex < originialPage.size(); lineIndex++) {
    const auto& shapeRun = originialPage[lineIndex];
    const auto& lineData = lines[lineIndex];
    const auto& line = page[lineIndex];

    auto totalWidth = shapeRun.totalAdvance;

    unsigned int n = shapeRun.glyphs.size();

    int penX = 0;

    painter.save();

    if (lineData.type != "ayah") {
      auto yshift = yPos + 300;

      if (lineData.type == "surah_name") {
        yshift += 500;
      }
      painter.translate((pageWpx + overflowMargin + totalWidth) / 2, yshift);
    } else {
      painter.translate(xStartRightToLeft, yPos);
      // painter.translate((pageWpx - overflowMargin - totalWidth) / 2, yPos);
    }

    painter.scale(unitsToPt, unitsToPt);

    for (int i = n - 1; i >= 0; --i) {
      const auto& g = shapeRun.glyphs[i];
      penX -= g.x_advance;
      const int gx = penX + g.x_offset;
      const int gy = -(g.y_offset);  // already y flipped in outlines

      if (!g.path.isEmpty()) {
        painter.save();
        painter.translate((double)gx, (double)gy);
        painter.drawPath(g.path);
        painter.restore();
      }
    }

    painter.restore();
    yPos += interLine * unitsToPt;

    ///
    painter.save();
    painter.translate(xStartRightToLeft, yPos);
    painter.scale(unitsToPt * line.xscale, -unitsToPt);
    int currentxPos = 0;

    for (auto& glyphLayout : line.glyphs) {
      currentxPos -= glyphLayout.x_advance;

      const int gx = currentxPos + glyphLayout.x_offset;
      const int gy = (glyphLayout.y_offset);  // because we flipped y in outlines

      GlyphParameters parameters{.lefttatweel = glyphLayout.lefttatweel,
                                 .righttatweel = glyphLayout.righttatweel,
                                 .scalex = 0};
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

      GlyphVis& glyph = m_otlayout->glyphs[glyphName];

      auto glyphPath = glyph.getAlternate(parameters)->path;

      if (!glyphPath.isEmpty()) {
        painter.save();
        painter.translate((double)gx, (double)gy);
        painter.drawPath(glyphPath);
        painter.restore();
      }
    }
    painter.restore();
    yPos += interLine * unitsToPt;
  }
  /*
    if (!IsImage) {
      pdf.newPage();

      yPos = OtLayout::TopSpace;
    }

      for (auto& line : page) {
        painter.save();
        painter.translate(xStartRightToLeft, yPos);
        painter.scale(unitsToPt * line.xscale, -unitsToPt);
        int currentxPos = 0;

        for (auto& glyphLayout : line.glyphs) {
          currentxPos -= glyphLayout.x_advance;

          const int gx = currentxPos + glyphLayout.x_offset;
          const int gy = (glyphLayout.y_offset);  // because we flipped y in outlines

          GlyphParameters parameters{.lefttatweel = glyphLayout.lefttatweel,
                                     .righttatweel = glyphLayout.righttatweel,
                                     .scalex = 0};
          QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

          GlyphVis& glyph = m_otlayout->glyphs[glyphName];

          auto glyphPath = glyph.getAlternate(parameters)->path;

          if (!glyphPath.isEmpty()) {
            painter.save();
            painter.translate((double)gx, (double)gy);
            painter.drawPath(glyphPath);
            painter.restore();
          }
        }
        painter.restore();
        yPos += interLine * unitsToPt;
      }*/

  painter.end();

  if (IsImage) {
    image.save(outputFileName, "JPG");
  }
}