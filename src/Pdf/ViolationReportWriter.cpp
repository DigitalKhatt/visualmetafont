#include "ViolationReportWriter.h"

#include <QByteArray>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QIODevice>
#include <QPainterPath>
#include <QTextStream>

#include <algorithm>
#include <cstdio>
#include <set>
#include <sstream>
#include <unordered_map>

#include "GeometryQt.h"

// PDF-Writer / PDFHummus
#include "EStatusCode.h"
#include "PDFPage.h"
#include "PDFRectangle.h"
#include "PDFUsedFont.h"
#include "PageContentContext.h"

using PDFHummus::eSuccess;
using namespace digitalkhatt::layout;

namespace {

// Scan the platform font directories for a file whose path contains `needle`.
// Mirrors the resolver in quranpdfwriterpdfhummus.cpp; used only for the
// optional text labels, so a miss just disables labels.
QString findFontPath(const QString& needle) {
  const QStringList dirs = {"/Library/Fonts", "/System/Library/Fonts",
                            "/System/Library/Fonts/Supplemental"};
  for (const QString& dir : dirs) {
    QDirIterator it(dir, QStringList() << "*.ttf" << "*.otf" << "*.ttc",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString path = it.next();
      if (path.contains(needle)) return path;
    }
  }
  return QString();
}

// Emit a QPainterPath as PDF path-construction ops, terminated by a stroke (S)
// or fill (f). Copied from QuranPdfWriterPdfHummus::pathToPdf.
QByteArray pathToPdf(const QPainterPath& path, bool fill) {
  QByteArray out;
  QTextStream ts(&out, QIODevice::WriteOnly);
  ts.setRealNumberPrecision(3);
  ts.setRealNumberNotation(QTextStream::FixedNotation);

  int start = -1;
  for (int i = 0; i < path.elementCount(); ++i) {
    const auto e = path.elementAt(i);
    if (e.isMoveTo()) {
      if (start >= 0 && path.elementAt(start).x == path.elementAt(i - 1).x &&
          path.elementAt(start).y == path.elementAt(i - 1).y)
        ts << "h\n";
      ts << e.x << ' ' << e.y << " m\n";
      start = i;
    } else if (e.isLineTo()) {
      ts << e.x << ' ' << e.y << " l\n";
    } else if (e.type == QPainterPath::CurveToElement) {
      if (i + 2 >= path.elementCount()) break;
      const auto c1 = path.elementAt(i);
      const auto c2 = path.elementAt(i + 1);
      const auto ep = path.elementAt(i + 2);
      ts << c1.x << ' ' << c1.y << ' ' << c2.x << ' ' << c2.y << ' ' << ep.x
         << ' ' << ep.y << " c\n";
      i += 2;
    }
  }
  if (start >= 0 &&
      path.elementAt(start).x == path.elementAt(path.elementCount() - 1).x &&
      path.elementAt(start).y == path.elementAt(path.elementCount() - 1).y)
    ts << "h\n";
  ts << (fill ? "f\n" : "S\n");
  ts.flush();
  return out;
}

void raw(PageContentContext* ctx, const std::string& s) {
  ctx->WriteFreeCode(s);
}

}  // namespace

void ViolationReportWriter::drawScene(
    PageContentContext* ctx,
    const std::vector<GlyphRef>& glyphs,
    const std::vector<ConstraintViolation>& violations,
    double x, double y, double w, double h,
    PDFUsedFont* labelFont) {
  // Union bounding box of all placed glyph geometry (worldPolys space).
  bool have = false;
  geometry::AABB B{0, 0, 0, 0};
  for (const auto& g : glyphs) {
    if (!g.worldPolys) continue;
    const auto bb = g.worldPolys->boundingAABB();
    if (!have) {
      B = bb;
      have = true;
    } else {
      B.minx = std::min(B.minx, bb.minx);
      B.miny = std::min(B.miny, bb.miny);
      B.maxx = std::max(B.maxx, bb.maxx);
      B.maxy = std::max(B.maxy, bb.maxy);
    }
  }
  if (!have) return;

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

  // Clip to the device rect before applying the fit transform: a violation's
  // marker can reference a glyph outside `glyphs` (e.g. writeSummary()'s
  // per-word crop only includes the anchor glyph's own word, but a
  // HorizontalOrder marker can point at a mark on the adjacent word) and
  // would otherwise be drawn past the box, bleeding into a neighboring
  // summary row. Harmless no-op for write()'s whole-page case, where content
  // already fits the box by construction.
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
    raw(ctx, pathToPdf(toQPainterPath(*g.worldPolys), false).toStdString());
    if (g.globalIndex >= 0) byIndex[g.globalIndex] = &g;
  }

  // 2) offending glyphs re-stroked in red
  std::set<int> offenders;
  for (const auto& v : violations) {
    if (v.glyphA >= 0) offenders.insert(v.glyphA);
    if (v.glyphB >= 0) offenders.insert(v.glyphB);
  }
  {
    std::ostringstream ss;
    ss << "1 0 0 RG\n"
       << offenderW << " w\n";
    raw(ctx, ss.str());
  }
  for (int idx : offenders) {
    auto it = byIndex.find(idx);
    if (it != byIndex.end() && it->second->worldPolys) {
      raw(ctx, pathToPdf(toQPainterPath(*it->second->worldPolys), false).toStdString());
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
    const double labelFontSize = glyphDevicePx * 0.25;
    AbstractContentContext::TextOptions to(
        labelFont, labelFontSize, AbstractContentContext::eRGB, 0xCC0000);
    for (const auto& v : violations) {
      if (v.markerCount == 0) continue;
      const double X = v.marker[0].x * s + tx;
      const double Y = v.marker[0].y * s + ty;
      char buf[96];
      std::snprintf(buf, sizeof(buf), "%s %.0f (%d)", violationTypeName(v.type), v.severity, v.glyphA);
      ctx->WriteText(X + 2.0 * s, Y + 2.0 * s, std::string(buf), to);
    }
  }
}

bool ViolationReportWriter::write(const std::vector<Page>& pages,
                                  const QString& pdfPath,
                                  const QString& logPath) {
  // ---------------------- companion text log ----------------------
  {
    QFile lf(logPath);
    if (lf.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&lf);
      out << "page,type,kind,glyphA_index,glyphA_name,glyphB_index,glyphB_name,"
             "residual,severity\n";
      for (size_t p = 0; p < pages.size(); ++p) {
        const Page& page = pages[p];
        auto nameOf = [&](int idx) -> QString {
          if (idx < 0 || idx >= static_cast<int>(page.glyphs.size()))
            return QString();
          return QString::fromStdString(page.glyphs[idx].name);
        };
        for (const auto& v : page.violations) {
          out << static_cast<int>(p) << "," << violationTypeName(v.type) << ","
              << (v.kind == ViolationKind::Hard ? "Hard" : "Soft") << ","
              << v.glyphA << "," << nameOf(v.glyphA) << "," << v.glyphB << ","
              << nameOf(v.glyphB) << "," << v.residual << "," << v.severity
              << "\n";
        }
      }
      lf.close();
    } else {
      qWarning() << "ViolationReportWriter: cannot open log" << logPath;
    }
  }

  // ---------------------- diagnostic PDF ----------------------
  PDFCreationSettings settings(true, true);
  if (m_writer.StartPDF(pdfPath.toStdString(), ePDFVersion17,
                        LogConfiguration::DefaultLogConfiguration(),
                        settings) != eSuccess) {
    qWarning() << "ViolationReportWriter: StartPDF failed" << pdfPath;
    return false;
  }

  QString fontPath = findFontPath("Helvetica");
  if (fontPath.isEmpty()) fontPath = findFontPath("Arial");
  PDFUsedFont* labelFont =
      fontPath.isEmpty() ? nullptr
                         : m_writer.GetFontForFile(fontPath.toStdString());

  const double W = 842.0;      // A4 landscape width in points
  const double H = 595.0;      // A4 landscape height in points
  const double margin = 24.0;  // page margin in points

  for (const Page& page : pages) {
    const bool anyGeom = std::any_of(page.glyphs.begin(), page.glyphs.end(),
                                     [](const GlyphRef& g) { return g.worldPolys != nullptr; });
    if (!anyGeom) continue;

    PDFPage* pdfPage = new PDFPage();
    pdfPage->SetMediaBox(PDFRectangle(0, 0, W, H));
    PageContentContext* ctx = m_writer.StartPageContentContext(pdfPage);

    drawScene(ctx, page.glyphs, page.violations, margin, margin, W - 2 * margin, H - 2 * margin, labelFont);

    m_writer.EndPageContentContext(ctx);
    m_writer.WritePageReleaseAndReturnPageID(pdfPage);
  }

  if (m_writer.EndPDF() != eSuccess) {
    qWarning() << "ViolationReportWriter: EndPDF failed";
    return false;
  }
  return true;
}

bool ViolationReportWriter::writeSummary(std::vector<WordEntry>& entries, const QString& pdfPath) {
  std::sort(entries.begin(), entries.end(), [](const WordEntry& a, const WordEntry& b) {
    return a.violation.severity > b.violation.severity;
  });

  PDFCreationSettings settings(true, true);
  if (m_writer.StartPDF(pdfPath.toStdString(), ePDFVersion17,
                        LogConfiguration::DefaultLogConfiguration(),
                        settings) != eSuccess) {
    qWarning() << "ViolationReportWriter: StartPDF (summary) failed" << pdfPath;
    return false;
  }

  QString fontPath = findFontPath("Helvetica");
  if (fontPath.isEmpty()) fontPath = findFontPath("Arial");
  PDFUsedFont* textFont =
      fontPath.isEmpty() ? nullptr
                         : m_writer.GetFontForFile(fontPath.toStdString());

  // A4 portrait: a tall list of rows reads better here than landscape.
  const double W = 595.0;
  const double H = 842.0;
  const double margin = 24.0;
  const double rowHeight = 90.0;
  const double rowPad = 8.0;
  const double cropW = 160.0;
  const double textX = margin + cropW + 16.0;
  const int rowsPerPage = std::max(1, static_cast<int>((H - 2 * margin) / rowHeight));

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
  };

  for (size_t i = 0; i < entries.size(); ++i) {
    const int rowInPage = static_cast<int>(i % static_cast<size_t>(rowsPerPage));
    if (rowInPage == 0) {
      endPage();
      startPage();
    }
    const WordEntry& e = entries[i];
    const double rowTop = H - margin - rowInPage * rowHeight;
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
      ctx->WriteText(textX, rowTop - 22.0, std::string(buf), to);
      std::snprintf(buf, sizeof(buf), "%s", violationTypeName(e.violation.type));
      ctx->WriteText(textX, rowTop - 42.0, std::string(buf), to);
      std::snprintf(buf, sizeof(buf), "Severity: %.2f", e.violation.severity);
      ctx->WriteText(textX, rowTop - 62.0, std::string(buf), to);
    }
  }
  endPage();

  if (m_writer.EndPDF() != eSuccess) {
    qWarning() << "ViolationReportWriter: EndPDF (summary) failed";
    return false;
  }
  return true;
}
