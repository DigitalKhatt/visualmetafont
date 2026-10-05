#include "digitalkhatt/pdf/ViolationReportWriter.h"

#include <algorithm>
#include <cstdio>
#include <cmath>
#include <set>
#include <sstream>
#include <unordered_map>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>

// PDF-Writer / PDFHummus
#include "EStatusCode.h"
#include "PDFPage.h"
#include "PDFRectangle.h"
#include "PDFUsedFont.h"
#include "PageContentContext.h"

using PDFHummus::eSuccess;
using namespace digitalkhatt::layout;
namespace digitalkhatt::pdf {

namespace {

// Scan the platform font directories for a file whose path contains `needle`.
// Mirrors the resolver in quranpdfwriterpdfhummus.cpp; used only for the
// optional text labels, so a miss just disables labels.
std::filesystem::path findFontPath(const std::string& needle) {
  for (const auto& dir : {"/Library/Fonts", "/System/Library/Fonts", "/usr/share/fonts", "/usr/local/share/fonts", "C:/Windows/Fonts"}) {
    std::error_code error;
    std::filesystem::recursive_directory_iterator it(dir, std::filesystem::directory_options::skip_permission_denied, error), end;
    for (; it != end; it.increment(error)) {
      if (error) { error.clear(); continue; }
      if (it->is_regular_file(error) && it->path().string().find(needle) != std::string::npos) return it->path();
    }
  }
  return {};
}

std::string pathToPdf(const geometry::GeometrySet& geometry, bool fill) {
  std::ostringstream out;
  out << std::fixed << std::setprecision(3);
  for (const auto& poly : geometry.polys()) {
    if (poly.empty()) continue;
    out << poly[0].x << ' ' << poly[0].y << " m\n";
    for (size_t i = 1; i < poly.size(); ++i) out << poly[i].x << ' ' << poly[i].y << " l\n";
    out << "h\n";
  }
  out << (fill ? "f\n" : "S\n");
  return out.str();
}

void raw(PageContentContext* ctx, const std::string& s) {
  ctx->WriteFreeCode(s);
}

std::string csvText(const std::string& value) {
  std::string result = "\"";
  for (char c : value) { if (c == '"') result += '"'; result += c; }
  return result + '"';
}

void drawHeader(PageContentContext* ctx, PDFUsedFont* font, double x, double y,
    const std::string& title, const std::vector<std::string>& notes) {
  if (!font) return;
  AbstractContentContext::TextOptions heading(font, 13.0, AbstractContentContext::eRGB, 0x000000);
  ctx->WriteText(x, y, title, heading);
  AbstractContentContext::TextOptions small(font, 7.5, AbstractContentContext::eRGB, 0x333333);
  for (size_t i = 0; i < notes.size(); ++i)
    ctx->WriteText(x, y - 16.0 - 12.0 * i, notes[i], small);
}

std::vector<std::string> wrapDetail(const std::string& detail) {
  std::istringstream words(detail);
  std::vector<std::string> lines;
  std::string word, line;
  while (words >> word) {
    if (!line.empty() && line.size() + word.size() + 1 > 65) {
      lines.push_back(line);
      line.clear();
    }
    if (!line.empty()) line += ' ';
    line += word;
  }
  if (!line.empty()) lines.push_back(line);
  return lines;
}

}  // namespace

void ViolationReportWriter::drawScene(
    PageContentContext* ctx,
    const std::vector<GlyphRef>& glyphs,
    const std::vector<ConstraintViolation>& violations,
    double x, double y, double w, double h,
    PDFUsedFont* labelFont) {
  std::unordered_map<int, bool> offenders;
  for (const auto& v : violations) {
    for (int index : {v.glyphA, v.glyphB})
      if (index >= 0) offenders[index] = offenders[index] || v.kind == ViolationKind::Hard;
  }
  // Union bounding box of all placed glyph geometry (worldPolys space).
  bool have = false;
  geometry::AABB B{0, 0, 0, 0};
  for (const auto& g : glyphs) {
    if (!g.worldPolys) continue;
    const auto bb = g.worldPolys->boundingAABB();
    if (!std::isfinite(bb.minx) || !std::isfinite(bb.miny) ||
        !std::isfinite(bb.maxx) || !std::isfinite(bb.maxy)) continue;
    if (!have) {
      B = bb;
      have = true;
    } else {
      B.minx = std::min(B.minx, bb.minx);
      B.miny = std::min(B.miny, bb.miny);
      B.maxx = std::max(B.maxx, bb.maxx);
      B.maxy = std::max(B.maxy, bb.maxy);
    }
    if (offenders.contains(g.globalIndex) && std::isfinite(g.dx) && std::isfinite(g.dy)) {
      B.minx = std::min(B.minx, bb.minx - g.dx);
      B.miny = std::min(B.miny, bb.miny - g.dy);
      B.maxx = std::max(B.maxx, bb.maxx - g.dx);
      B.maxy = std::max(B.maxy, bb.maxy - g.dy);
    }
  }
  if (!have) return;

  // Leave room for outline strokes at the edges of a summary cell.
  x += 3.0; y += 3.0; w -= 6.0; h -= 6.0;

  const double bw = std::max(1.0, B.maxx - B.minx);
  const double bh = std::max(1.0, B.maxy - B.miny);
  // Uniform fit (preserve aspect), centered in the (x,y,w,h) device rect.
  // worldPolys are Y-up and PDF user space is Y-up, so there is NO Y-flip:
  // larger world-y renders higher on the page.
  const double s = std::min(w / bw, h / bh);
  const double tx = x + 0.5 * (w - bw * s) - B.minx * s;
  const double ty = y + 0.5 * (h - bh * s) - B.miny * s;

  // Reference glyph height for this scene (median), used to scale stroke
  // widths and the label font proportionally to how large content actually
  // renders here, instead of a fixed device size. A fixed device size (e.g.
  // "always 0.5pt") looks fine on a single-word crop (large s, few big
  // glyphs) but renders as chunky/oversized strokes on a whole-page report
  // (small s, many tiny glyphs), since the stroke doesn't shrink along with
  // the content the way it visually should.
  double medianGlyphHeight = 0.0;
  {
    std::vector<double> heights;
    heights.reserve(glyphs.size());
    for (const auto& g : glyphs) {
      if (!g.worldPolys) continue;
      const auto bb = g.worldPolys->boundingAABB();
      const double hh = bb.maxy - bb.miny;
      if (hh > 0.0) heights.push_back(hh);
    }
    if (!heights.empty()) {
      std::sort(heights.begin(), heights.end());
      medianGlyphHeight = heights[heights.size() / 2];
    }
  }
  const double glyphDevicePx = medianGlyphHeight * s;
  // Device-space stroke width as a fraction of that, clamped so it never
  // vanishes (whole page, tiny glyphs) or dominates (single word, huge
  // glyphs).
  auto strokeWidth = [&](double fraction, double minPt, double maxPt) {
    const double devicePt = std::clamp(glyphDevicePx * fraction, minPt, maxPt);
    return devicePt / s;  // back to world units for the `w` op under the cm
  };

  raw(ctx, "q\n");

  // Keep markers and original-position overlays inside the scene cell.
  {
    std::ostringstream ss;
    ss << x << " " << y << " " << w << " " << h << " re W n\n";
    raw(ctx, ss.str());
  }

  ctx->cm(s, 0, 0, s, tx, ty);

  // Round line joins: the default miter join extends a stroke's outer corner
  // to a sharp point whose length grows without bound as the join angle
  // shrinks -- for a thin sliver triangle (a real, kept convex decomposition
  // piece near a tight/acute glyph corner) that spike can shoot far past the
  // triangle's own envelope. A round join instead arcs around the vertex with
  // radius = lineWidth/2, so the stroke never extends past the vertex by more
  // than half the stroke width, regardless of how acute the angle is.
  raw(ctx, "1 j\n");

  const double outlineW = strokeWidth(0.015, 0.15, 0.5);
  const double offenderW = strokeWidth(0.03, 0.3, 1.0);
  const double markerW = strokeWidth(0.045, 0.4, 1.5);

  // Original shaped positions and displacement lines for participating glyphs.
  {
    std::ostringstream style;
    style << "0 0.35 0.85 RG\n" << outlineW << " w\n[" << 3.0 / s << " " << 2.0 / s << "] 0 d\n";
    raw(ctx, style.str());
    for (const auto& g : glyphs) {
      if (!g.worldPolys || !offenders.contains(g.globalIndex) ||
          !std::isfinite(g.dx) || !std::isfinite(g.dy) || std::hypot(g.dx, g.dy) < 1e-9) continue;
      const auto bb = g.worldPolys->boundingAABB();
      if (!std::isfinite(bb.minx) || !std::isfinite(bb.miny) ||
          !std::isfinite(bb.maxx) || !std::isfinite(bb.maxy)) continue;
      raw(ctx, pathToPdf(g.worldPolys->translate(-g.dx, -g.dy), false));
      const double cx = 0.5 * (bb.minx + bb.maxx), cy = 0.5 * (bb.miny + bb.maxy);
      std::ostringstream arrow;
      arrow << cx - g.dx << " " << cy - g.dy << " m\n" << cx << " " << cy << " l\nS\n";
      raw(ctx, arrow.str());
    }
    raw(ctx, "[] 0 d\n");
  }

  // 1) all glyph outlines, light gray. Also index by globalIndex so offender
  // lookup below doesn't depend on `glyphs` being positionally indexed by
  // globalIndex -- writeSummary() hands in an arbitrary cropped subset.
  {
    std::ostringstream ss;
    ss << "0.72 0.72 0.72 RG\n"
       << outlineW << " w\n";
    raw(ctx, ss.str());
  }
  std::unordered_map<int, const GlyphRef*> byIndex;
  for (const auto& g : glyphs) {
    if (!g.worldPolys) continue;
    const auto bb = g.worldPolys->boundingAABB();
    if (!std::isfinite(bb.minx) || !std::isfinite(bb.miny) ||
        !std::isfinite(bb.maxx) || !std::isfinite(bb.maxy)) continue;
    raw(ctx, pathToPdf(*g.worldPolys, false));
    if (g.globalIndex >= 0) byIndex[g.globalIndex] = &g;
  }

  // 2) hard failures in red; soft targets / ownership review warnings in amber.
  for (const auto& [idx, hard] : offenders) {
    auto it = byIndex.find(idx);
    if (it != byIndex.end() && it->second->worldPolys) {
      std::ostringstream style;
      style << (hard ? "1 0 0 RG\n" : "0.85 0.45 0 RG\n") << offenderW << " w\n";
      raw(ctx, style.str());
      raw(ctx, pathToPdf(*it->second->worldPolys, false));
    }
  }

  // 3) violation markers, magenta
  {
    std::ostringstream ss;
    ss << "1 0 1 RG\n"
       << markerW << " w\n";
    raw(ctx, ss.str());
  }
  for (const auto& v : violations) {
    std::ostringstream ss;
    if (v.markerCount == 2) {
      ss << v.marker[0].x << " " << v.marker[0].y << " m\n"
         << v.marker[1].x << " " << v.marker[1].y << " l\nS\n";
    } else if (v.markerCount == 1) {
      const double r = strokeWidth(0.09, 0.9, 3.0);
      ss << (v.marker[0].x - r) << " " << (v.marker[0].y - r) << " "
         << (2 * r) << " " << (2 * r) << " re\nS\n";
    }
    raw(ctx, ss.str());
  }

  raw(ctx, "Q\n");  // pop the fit transform

  // 4) labels in device space, optional (writeSummary()'s crops skip these --
  // the row's own text columns already carry type/severity/glyph). Font size
  // scales with `glyphDevicePx` (the same page-relative reference as the
  // stroke widths above) rather than a fixed constant.
  if (labelFont) {
    const double labelFontSize = std::clamp(glyphDevicePx * 0.25, 5.0, 10.0);
    AbstractContentContext::TextOptions to(
        labelFont, labelFontSize, AbstractContentContext::eRGB, 0xCC0000);
    for (const auto& v : violations) {
      if (v.markerCount == 0) continue;
      const double X = v.marker[0].x * s + tx;
      const double Y = v.marker[0].y * s + ty;
      char buf[96];
      std::snprintf(buf, sizeof(buf), "%s (%d,%d)%s", violationTypeName(v.type), v.glyphA, v.glyphB,
          v.kind == ViolationKind::Soft ? " review" : "");
      ctx->WriteText(X + 2.0 * s, Y + 2.0 * s, std::string(buf), to);
    }
  }
}

bool ViolationReportWriter::start(const std::filesystem::path& pdfPath, const std::filesystem::path& logPath) {
  m_log.open(logPath);
  if (!m_log) return false;
  m_log << "page,type,kind,glyphA_index,glyphA_name,glyphB_index,glyphB_name,residual,severity,lineA,lineB,baseA_index,baseB_index,shiftA_x,shiftA_y,shiftB_x,shiftB_y,allowed_residual,structural,detail,initial_severity,introduced,worsened,wordA,wordB,clusterA,clusterB\n";
  m_log << std::setprecision(12);
  PDFCreationSettings settings(true, true);
  if (m_writer.StartPDF(pdfPath.string(), ePDFVersion17, LogConfiguration::DefaultLogConfiguration(), settings) != eSuccess) return false;
  auto font = findFontPath("Helvetica");
  if (font.empty()) font = findFontPath("Arial.ttf");
  if (font.empty()) font = findFontPath("DejaVuSans.ttf");
  m_labelFont = font.empty() ? nullptr : m_writer.GetFontForFile(font.string());
  m_started = true;
  m_pageIndex = 0;
  return true;
}

bool ViolationReportWriter::appendPage(const Page& page) {
  if (!m_started) return false;
  ++m_pageIndex;
  const int number = page.pageNumber > 0 ? page.pageNumber : m_pageIndex;
  auto glyphOf = [&](int index) -> GlyphRef {
    auto found = std::find_if(page.glyphs.begin(), page.glyphs.end(), [&](const auto& g) { return g.globalIndex == index; });
    return found == page.glyphs.end() ? GlyphRef{} : *found;
  };
  for (const auto& v : page.violations) {
    const auto a = glyphOf(v.glyphA), b = glyphOf(v.glyphB);
    m_log << number << ',' << violationTypeName(v.type) << ',' << (v.kind == ViolationKind::Hard ? "Hard" : "Soft") << ','
          << v.glyphA << ',' << csvText(a.name) << ',' << v.glyphB << ',' << csvText(b.name) << ','
          << v.residual << ',' << v.severity << ',' << a.lineNumber << ',' << b.lineNumber << ','
          << a.baseGlobalIndex << ',' << b.baseGlobalIndex << ',' << a.dx << ',' << a.dy << ',' << b.dx << ',' << b.dy << ','
          << v.allowedResidual << ',' << v.structural << ',' << csvText(v.detail) << ',' << v.initialSeverity << ','
          << v.introduced << ',' << v.worsened << ',' << csvText(a.wordText) << ',' << csvText(b.wordText) << ',' << a.cluster << ',' << b.cluster << '\n';
  }
  PDFPage* pdfPage = new PDFPage();
  pdfPage->SetMediaBox(PDFRectangle(0, 0, 842, 595));
  auto* ctx = m_writer.StartPageContentContext(pdfPage);
  drawHeader(ctx, m_labelFont, 24, 571, "XPBD placement report - Page " + std::to_string(number), page.notes);
  drawScene(ctx, page.glyphs, page.violations, 24, 24, 794, 547 - 24 - page.notes.size() * 12, m_labelFont);
  m_writer.EndPageContentContext(ctx);
  return m_writer.WritePageReleaseAndReturnPageID(pdfPage).first == eSuccess && bool(m_log);
}

bool ViolationReportWriter::finish() {
  if (!m_started) return false;
  m_log.flush();
  const bool logOK = bool(m_log);
  m_log.close();
  m_started = false;
  return m_writer.EndPDF() == eSuccess && logOK;
}

bool ViolationReportWriter::write(const std::vector<Page>& pages, const std::filesystem::path& pdfPath, const std::filesystem::path& logPath) {
  if (!start(pdfPath, logPath)) return false;
  for (const auto& page : pages) if (!appendPage(page)) { finish(); return false; }
  return finish();
}

bool ViolationReportWriter::writeSummary(std::vector<WordEntry>& entries, const std::filesystem::path& pdfPath,
    const std::vector<std::string>& notes) {
  std::stable_sort(entries.begin(), entries.end(), [](const WordEntry& a, const WordEntry& b) {
    return violationPrecedes(a.violation, b.violation);
  });

  PDFCreationSettings settings(true, true);
  if (m_writer.StartPDF(pdfPath.string(), ePDFVersion17,
                        LogConfiguration::DefaultLogConfiguration(),
                        settings) != eSuccess) {
    std::cerr << "ViolationReportWriter: StartPDF (summary) failed " << pdfPath << '\n';
    return false;
  }

  auto fontPath = findFontPath("Helvetica");
  if (fontPath.empty()) fontPath = findFontPath("Arial.ttf");
  if (fontPath.empty()) fontPath = findFontPath("DejaVuSans.ttf");
  PDFUsedFont* textFont =
      fontPath.empty() ? nullptr
                         : m_writer.GetFontForFile(fontPath.string());

  // A4 portrait: a tall list of rows reads better here than landscape.
  const double W = 595.0;
  const double H = 842.0;
  const double margin = 24.0;
  const double rowHeight = 114.0;
  const double rowPad = 8.0;
  const double cropW = 160.0;
  const double textX = margin + cropW + 16.0;
  const double headerHeight = 24.0 + notes.size() * 12.0;
  const int rowsPerPage = std::max(1, static_cast<int>((H - 2 * margin - headerHeight) / rowHeight));

  PDFPage* pdfPage = nullptr;
  PageContentContext* ctx = nullptr;
  auto endPage = [&]() {
    if (!ctx) return;
    m_writer.EndPageContentContext(ctx);
    m_writer.WritePageReleaseAndReturnPageID(pdfPage);
    ctx = nullptr;
    pdfPage = nullptr;
  };
  auto startPage = [&]() {
    pdfPage = new PDFPage();
    pdfPage->SetMediaBox(PDFRectangle(0, 0, W, H));
    ctx = m_writer.StartPageContentContext(pdfPage);
    drawHeader(ctx, textFont, margin, H - margin, "XPBD placement report - " + std::to_string(entries.size()) + " findings", notes);
  };

  if (entries.empty()) {
    startPage();
    if (textFont) {
      AbstractContentContext::TextOptions to(textFont, 11.0, AbstractContentContext::eRGB, 0x000000);
      ctx->WriteText(margin, H - margin - headerHeight - 24.0,
          "No findings above the report cutoff for the checks listed above.", to);
    }
  }

  for (size_t i = 0; i < entries.size(); ++i) {
    const int rowInPage = static_cast<int>(i % static_cast<size_t>(rowsPerPage));
    if (rowInPage == 0) {
      endPage();
      startPage();
    }
    const WordEntry& e = entries[i];
    const double rowTop = H - margin - headerHeight - rowInPage * rowHeight;
    const double cropY = rowTop - rowHeight + rowPad;
    const double cropH = rowHeight - 2 * rowPad;

    // crop border, so the word's device box reads as a distinct cell.
    {
      std::ostringstream ss;
      ss << "0.6 0.6 0.6 RG\n0.5 w\n"
         << margin << " " << cropY << " " << cropW << " " << cropH << " re\nS\n";
      raw(ctx, ss.str());
    }
    drawScene(ctx, e.wordGlyphs, {e.violation}, margin, cropY, cropW, cropH, nullptr);

    if (textFont) {
      AbstractContentContext::TextOptions to(textFont, 11.0, AbstractContentContext::eRGB, 0x000000);
      char buf[160];
      std::snprintf(buf, sizeof(buf), "Page %d, Line %d", e.pageNumber, e.lineNumber);
      if (e.otherLineNumber > 0 && e.otherLineNumber != e.lineNumber)
        std::snprintf(buf, sizeof(buf), "Page %d, Lines %d and %d", e.pageNumber, e.lineNumber, e.otherLineNumber);
      const std::string status = e.violation.introduced ? " [NEW]" : e.violation.worsened ? " [WORSE]" : "";
      ctx->WriteText(textX, rowTop - 22.0, std::string(buf) + status, to);
      std::snprintf(buf, sizeof(buf), "%s (%s)", violationTypeName(e.violation.type),
          e.violation.kind == ViolationKind::Hard ? "Hard" : "Review / soft");
      ctx->WriteText(textX, rowTop - 38.0, std::string(buf), to);
      if (e.violation.structural) std::snprintf(buf, sizeof(buf), "Structural issue; glyphs #%d, #%d", e.violation.glyphA, e.violation.glyphB);
      else std::snprintf(buf, sizeof(buf), "Severity: %.2f; glyphs #%d, #%d", e.violation.severity, e.violation.glyphA, e.violation.glyphB);
      ctx->WriteText(textX, rowTop - 54.0, std::string(buf), to);
      AbstractContentContext::TextOptions small(textFont, 8.5, AbstractContentContext::eRGB, 0x333333);
      const auto ownerOf = [&](int index) {
        const auto glyph = std::find_if(e.wordGlyphs.begin(), e.wordGlyphs.end(),
            [&](const auto& g) { return g.globalIndex == index; });
        return glyph != e.wordGlyphs.end() && glyph->baseGlobalIndex >= 0 ?
            "#" + std::to_string(glyph->baseGlobalIndex) : std::string("none");
      };
      ctx->WriteText(textX, rowTop - 66.0,
          "Assigned bases: A " + ownerOf(e.violation.glyphA) + "; B " + ownerOf(e.violation.glyphB), small);
      const auto detailLines = wrapDetail(e.violation.detail);
      for (size_t line = 0; line < std::min<size_t>(3, detailLines.size()); ++line)
        ctx->WriteText(textX, rowTop - 82.0 - 10.0 * line, detailLines[line], small);
    }
  }
  endPage();

  if (m_writer.EndPDF() != eSuccess) {
    std::cerr << "ViolationReportWriter: EndPDF (summary) failed";
    return false;
  }
  return true;
}

}  // namespace digitalkhatt::pdf
