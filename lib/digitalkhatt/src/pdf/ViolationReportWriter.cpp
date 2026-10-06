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
#include <map>

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

bool ViolationReportWriter::start(const std::filesystem::path& pdfPath, const std::filesystem::path& logPath,
                                  const OptParams& params) {
  if (params.reportMaxFindings < 0 || !std::isfinite(params.minViolationSeverity) || params.minViolationSeverity < 0 ||
      (params.reportSort != "severity" && params.reportSort != "priority")) return false;
  m_params = params;
  m_selected.clear();
  m_sequence = m_eligibleCount = 0;
  m_notes.clear();
  m_log.open(logPath);
  if (!m_log) return false;
  m_log << "page,type,kind,glyphA_index,glyphA_name,glyphB_index,glyphB_name,residual,severity,lineA,lineB,baseA_index,baseB_index,shiftA_x,shiftA_y,shiftB_x,shiftB_y,allowed_residual,structural,detail,initial_severity,introduced,worsened,wordA,wordB,clusterA,clusterB,report_rank,report_severity,wordA_number,wordB_number\n";
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

bool ViolationReportWriter::precedes(const Finding& a, const Finding& b) const {
  if (violationReportPrecedes(a.violation, b.violation, m_params.reportSort)) return true;
  if (violationReportPrecedes(b.violation, a.violation, m_params.reportSort)) return false;
  if (a.pageNumber != b.pageNumber) return a.pageNumber < b.pageNumber;
  return a.sequence < b.sequence;
}

bool ViolationReportWriter::appendPage(const Page& page) {
  if (!m_started) return false;
  ++m_pageIndex;
  if (m_notes.empty()) m_notes = page.notes;
  std::shared_ptr<Page> ownedPage;
  const auto order = [this](const Finding& a, const Finding& b) { return precedes(a, b); };
  for (const auto& v : page.violations) {
    if (!m_params.toggles.reportGenericGap && v.type == ViolationType::GenericGap) continue;
    if (!v.structural && violationReportSeverity(v) < m_params.minViolationSeverity) continue;
    if (m_params.reportOnlyChanged && !v.structural && !v.introduced && !v.worsened) continue;
    ++m_eligibleCount;
    Finding candidate{ownedPage, v, m_sequence++, page.pageNumber > 0 ? page.pageNumber : m_pageIndex};
    const auto limit = static_cast<std::size_t>(m_params.reportMaxFindings);
    if (limit && m_selected.size() >= limit && !precedes(candidate, m_selected.front())) continue;
    if (!ownedPage) {
      ownedPage = std::make_shared<Page>();
      ownedPage->pageNumber = page.pageNumber > 0 ? page.pageNumber : m_pageIndex;
      ownedPage->notes = page.notes;
      ownedPage->glyphs = page.glyphs;
      for (auto& glyph : ownedPage->glyphs) {
        if (!glyph.worldPolys) continue;
        if (!glyph.ownedGeometry)
          glyph.ownedGeometry = std::make_shared<geometry::GeometrySet>(*glyph.worldPolys);
        glyph.worldPolys = glyph.ownedGeometry.get();
      }
    }
    candidate.page = ownedPage;
    if (limit && m_selected.size() >= limit) {
      std::pop_heap(m_selected.begin(), m_selected.end(), order);
      m_selected.pop_back();
    }
    m_selected.push_back(std::move(candidate));
    std::push_heap(m_selected.begin(), m_selected.end(), order);
  }
  return true;
}

void ViolationReportWriter::writeCsvFinding(const Finding& finding, std::size_t rank) {
  const auto& page = *finding.page;
  const auto& v = finding.violation;
  auto glyphOf = [&](int index) -> GlyphRef {
    auto found = std::find_if(page.glyphs.begin(), page.glyphs.end(), [&](const auto& g) { return g.globalIndex == index; });
    return found == page.glyphs.end() ? GlyphRef{} : *found;
  };
  const auto a = glyphOf(v.glyphA), b = glyphOf(v.glyphB);
  m_log << page.pageNumber << ',' << violationTypeName(v.type) << ',' << (v.kind == ViolationKind::Hard ? "Hard" : "Soft") << ','
          << v.glyphA << ',' << csvText(a.name) << ',' << v.glyphB << ',' << csvText(b.name) << ','
          << v.residual << ',' << v.severity << ',' << a.lineNumber << ',' << b.lineNumber << ','
          << a.baseGlobalIndex << ',' << b.baseGlobalIndex << ',' << a.dx << ',' << a.dy << ',' << b.dx << ',' << b.dy << ','
          << v.allowedResidual << ',' << v.structural << ',' << csvText(v.detail) << ',' << v.initialSeverity << ','
          << v.introduced << ',' << v.worsened << ',' << csvText(a.wordText) << ',' << csvText(b.wordText) << ',' << a.cluster << ',' << b.cluster << ',' << rank << ',' << violationReportSeverity(v) << ',' << a.wordNumber << ',' << b.wordNumber << '\n';
}

bool ViolationReportWriter::drawPage(const Page& page) {
  PDFPage* pdfPage = new PDFPage();
  pdfPage->SetMediaBox(PDFRectangle(0, 0, 842, 595));
  auto* ctx = m_writer.StartPageContentContext(pdfPage);
  const auto title = page.pageNumber > 0 ? "XPBD placement report - Page " + std::to_string(page.pageNumber) : "XPBD placement report - No retained findings";
  drawHeader(ctx, m_labelFont, 24, 571, title, page.notes);
  drawScene(ctx, page.glyphs, page.violations, 24, 24, 794, 547 - 24 - page.notes.size() * 12, m_labelFont);
  m_writer.EndPageContentContext(ctx);
  return m_writer.WritePageReleaseAndReturnPageID(pdfPage).first == eSuccess && bool(m_log);
}

bool ViolationReportWriter::finish() {
  if (!m_started) return false;
  std::stable_sort(m_selected.begin(), m_selected.end(), [this](const Finding& a, const Finding& b) { return precedes(a, b); });
  std::map<int, Page> pages;
  const std::string selection = "Selected " + std::to_string(m_selected.size()) + " of " + std::to_string(m_eligibleCount) +
      " eligible findings; sort " + m_params.reportSort + "; critical structural diagnostics first.";
  for (std::size_t i = 0; i < m_selected.size(); ++i) {
    const auto& finding = m_selected[i];
    writeCsvFinding(finding, i + 1);
    auto [it, inserted] = pages.try_emplace(finding.page->pageNumber);
    if (inserted) {
      it->second = *finding.page;
      it->second.notes.push_back(selection);
    }
    it->second.violations.push_back(finding.violation);
  }
  bool pdfOK = true;
  if (pages.empty()) {
    Page empty;
    empty.notes = m_notes;
    empty.notes.push_back(selection);
    pdfOK = drawPage(empty);
  }
  for (const auto& [number, page] : pages) pdfOK = drawPage(page) && pdfOK;
  m_log.flush();
  const bool logOK = bool(m_log);
  m_log.close();
  m_started = false;
  return m_writer.EndPDF() == eSuccess && logOK && pdfOK;
}

bool ViolationReportWriter::write(const std::vector<Page>& pages, const std::filesystem::path& pdfPath, const std::filesystem::path& logPath,
                                  const OptParams& params) {
  if (!start(pdfPath, logPath, params)) return false;
  for (const auto& page : pages) if (!appendPage(page)) { finish(); return false; }
  return finish();
}

std::vector<ViolationReportWriter::WordEntry> ViolationReportWriter::summaryEntries() const {
  std::vector<WordEntry> entries;
  for (const auto& finding : m_selected) {
    const auto& page = *finding.page;
    WordEntry entry;
    entry.pageNumber = page.pageNumber;
    entry.violation = finding.violation;
    entry.rank = entries.size() + 1;
    std::set<std::size_t> context;
    const auto positionOf = [&](int index) {
      return std::find_if(page.glyphs.begin(), page.glyphs.end(), [&](const auto& g) { return g.globalIndex == index; });
    };
    const auto addWord = [&](int index) {
      const auto glyph = positionOf(index);
      if (glyph == page.glyphs.end()) return;
      std::size_t left = glyph - page.glyphs.begin(), right = left;
      while (left > 0 && page.glyphs[left - 1].lineNumber == glyph->lineNumber && page.glyphs[left - 1].name != "space") --left;
      while (right + 1 < page.glyphs.size() && page.glyphs[right + 1].lineNumber == glyph->lineNumber && page.glyphs[right + 1].name != "space") ++right;
      for (std::size_t i = left; i <= right; ++i)
        if (page.glyphs[i].name != "space") context.insert(i);
    };
    const auto a = positionOf(entry.violation.glyphA), b = positionOf(entry.violation.glyphB);
    if (a != page.glyphs.end()) { entry.lineNumber = a->lineNumber; entry.wordNumber = a->wordNumber; }
    if (b != page.glyphs.end()) { entry.otherLineNumber = b->lineNumber; entry.otherWordNumber = b->wordNumber; }
    if (!entry.lineNumber) entry.lineNumber = entry.otherLineNumber;
    for (int index : {entry.violation.glyphA, entry.violation.glyphB}) {
      const auto glyph = positionOf(index);
      if (glyph == page.glyphs.end()) continue;
      addWord(index);
      addWord(glyph->baseGlobalIndex);
    }
    for (auto index : context) entry.wordGlyphs.push_back(page.glyphs[index]);
    entries.push_back(std::move(entry));
  }
  return entries;
}

bool ViolationReportWriter::writeSummary(std::vector<WordEntry>& entries, const std::filesystem::path& pdfPath,
    const std::vector<std::string>& notes) {
  std::stable_sort(entries.begin(), entries.end(), [this](const WordEntry& a, const WordEntry& b) {
    return violationReportPrecedes(a.violation, b.violation, m_params.reportSort);
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
  int section = entries.empty() ? 1 : violationReportGroup(entries.front().violation);
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
    const auto title = entries.empty() ? "XPBD placement report - No retained findings" :
        section == 0 ? "XPBD report - Critical structural diagnostics" :
        section == 2 ? "XPBD report - Structural review warnings" : "XPBD report - Geometric violations";
    drawHeader(ctx, textFont, margin, H - margin, title, notes);
  };

  if (entries.empty()) {
    startPage();
    if (textFont) {
      AbstractContentContext::TextOptions to(textFont, 11.0, AbstractContentContext::eRGB, 0x000000);
      ctx->WriteText(margin, H - margin - headerHeight - 24.0,
          "No findings above the report cutoff for the checks listed above.", to);
    }
  }

  std::size_t sectionRow = 0;
  for (size_t i = 0; i < entries.size(); ++i, ++sectionRow) {
    if (violationReportGroup(entries[i].violation) != section) {
      section = violationReportGroup(entries[i].violation);
      sectionRow = 0;
    }
    const int rowInPage = static_cast<int>(sectionRow % static_cast<size_t>(rowsPerPage));
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
      std::snprintf(buf, sizeof(buf), "#%zu Page %d, L%d W%d", e.rank ? e.rank : i + 1, e.pageNumber, e.lineNumber, e.wordNumber);
      if (e.otherLineNumber > 0 && (e.otherLineNumber != e.lineNumber || e.otherWordNumber != e.wordNumber))
        std::snprintf(buf, sizeof(buf), "#%zu Page %d, L%d W%d / L%d W%d", e.rank ? e.rank : i + 1,
            e.pageNumber, e.lineNumber, e.wordNumber, e.otherLineNumber, e.otherWordNumber);
      const std::string status = e.violation.introduced ? " [NEW]" : e.violation.worsened ? " [WORSE]" : "";
      ctx->WriteText(textX, rowTop - 22.0, std::string(buf) + status, to);
      std::snprintf(buf, sizeof(buf), "%s (%s)", violationTypeName(e.violation.type),
          e.violation.kind == ViolationKind::Hard ? "Hard" : "Review / soft");
      ctx->WriteText(textX, rowTop - 38.0, std::string(buf), to);
      if (e.violation.structural) std::snprintf(buf, sizeof(buf), "Structural issue; glyphs #%d, #%d", e.violation.glyphA, e.violation.glyphB);
      else std::snprintf(buf, sizeof(buf), "Severity: %.2f; glyphs #%d, #%d", violationReportSeverity(e.violation), e.violation.glyphA, e.violation.glyphB);
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

bool ViolationReportWriter::writeCompact(std::vector<WordEntry>& entries, const std::filesystem::path& pdfPath,
                                         const std::vector<std::string>& notes) {
  std::stable_sort(entries.begin(), entries.end(), [this](const WordEntry& a, const WordEntry& b) {
    return violationReportPrecedes(a.violation, b.violation, m_params.reportSort);
  });
  PDFCreationSettings settings(true, true);
  if (m_writer.StartPDF(pdfPath.string(), ePDFVersion17, LogConfiguration::DefaultLogConfiguration(), settings) != eSuccess)
    return false;
  auto fontPath = findFontPath("Helvetica");
  if (fontPath.empty()) fontPath = findFontPath("Arial.ttf");
  if (fontPath.empty()) fontPath = findFontPath("DejaVuSans.ttf");
  auto* font = fontPath.empty() ? nullptr : m_writer.GetFontForFile(fontPath.string());

  constexpr double W = 842, H = 595, margin = 18, gap = 6, header = 38, footer = 14;
  constexpr std::size_t columns = 5, rows = 6, perPage = columns * rows;
  constexpr double cellWidth = (W - 2 * margin - gap * (columns - 1)) / columns;
  constexpr double cellHeight = (H - 2 * margin - header - footer - gap * (rows - 1)) / rows;
  const std::size_t pageCount = std::max<std::size_t>(1, (entries.size() + perPage - 1) / perPage);
  bool pageOK = true;
  for (std::size_t p = 0; p < pageCount; ++p) {
    auto* page = new PDFPage();
    page->SetMediaBox(PDFRectangle(0, 0, W, H));
    auto* ctx = m_writer.StartPageContentContext(page);
    if (font) {
      AbstractContentContext::TextOptions title(font, 12, AbstractContentContext::eRGB, 0x000000);
      AbstractContentContext::TextOptions small(font, 6, AbstractContentContext::eRGB, 0x444444);
      ctx->WriteText(margin, H - margin - 1, "XPBD violations - compact visual index", title);
      const auto subtitle = std::to_string(entries.size()) + " findings; sort " + m_params.reportSort +
          "; GenericGap " + (m_params.toggles.reportGenericGap ? "on" : "off") +
          "; placement audit " + (m_params.toggles.reportPlacementAudit ? "on" : "off") +
          ". Red: hard; amber: review; blue dashed: shaped position.";
      ctx->WriteText(margin, H - margin - 13, subtitle, small);
      if (!notes.empty()) ctx->WriteText(margin, H - margin - 23, notes.back(), small);
      const auto navigation = "Sheet " + std::to_string(p + 1) + "/" + std::to_string(pageCount) +
          " | # matches CSV and detailed summary | P: Mushaf page; L: line; W: source word within line (1-based).";
      ctx->WriteText(margin, margin - 3, navigation, small);
    }
    if (entries.empty() && font) {
      AbstractContentContext::TextOptions text(font, 10, AbstractContentContext::eRGB, 0x333333);
      ctx->WriteText(margin, H - margin - header - 20, "No findings retained with the configured report filters.", text);
    }
    const auto begin = p * perPage, end = std::min(begin + perPage, entries.size());
    for (std::size_t i = begin; i < end; ++i) {
      const auto& e = entries[i];
      const auto slot = i - begin;
      const double x = margin + (slot % columns) * (cellWidth + gap);
      const double y = H - margin - header - (slot / columns + 1) * cellHeight - (slot / columns) * gap;
      std::ostringstream border;
      border << "0.82 0.82 0.82 RG\n0.35 w\n" << x << ' ' << y << ' ' << cellWidth << ' ' << cellHeight << " re\nS\n";
      raw(ctx, border.str());
      drawScene(ctx, e.wordGlyphs, {e.violation}, x + 2, y + 22, cellWidth - 4, cellHeight - 26, nullptr);
      if (font) {
        char location[160], description[160];
        const auto rank = e.rank ? e.rank : i + 1;
        std::snprintf(location, sizeof(location), "#%zu P%d L%d W%d", rank, e.pageNumber, e.lineNumber, e.wordNumber);
        if (e.otherLineNumber > 0 && (e.otherLineNumber != e.lineNumber || e.otherWordNumber != e.wordNumber))
          std::snprintf(location, sizeof(location), "#%zu P%d L%d W%d / L%d W%d", rank,
              e.pageNumber, e.lineNumber, e.wordNumber, e.otherLineNumber, e.otherWordNumber);
        const auto status = e.violation.introduced ? " NEW" : e.violation.worsened ? " WORSE" : "";
        if (e.violation.structural)
          std::snprintf(description, sizeof(description), "%s structural%s", violationTypeName(e.violation.type), status);
        else
          std::snprintf(description, sizeof(description), "%s %.2f%s", violationTypeName(e.violation.type), violationReportSeverity(e.violation), status);
        AbstractContentContext::TextOptions ref(font, 6, AbstractContentContext::eRGB, 0x222222);
        AbstractContentContext::TextOptions label(font, 5.5, AbstractContentContext::eRGB, 0x555555);
        ctx->WriteText(x + 5, y + 13, location, ref);
        ctx->WriteText(x + 5, y + 5, description, label);
      }
    }
    m_writer.EndPageContentContext(ctx);
    pageOK = m_writer.WritePageReleaseAndReturnPageID(page).first == eSuccess && pageOK;
  }
  return m_writer.EndPDF() == eSuccess && pageOK;
}

}  // namespace digitalkhatt::pdf
