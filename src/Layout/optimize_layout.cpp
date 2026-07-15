/*
 * Copyright (c) 2015-2023 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

// Thin Qt-side adapter for the Qt-free XPBD collision-avoidance / mark-
// positioning solver in lib/digitalkhatt (namespace digitalkhatt::layout).
// Mirrors the pattern used for FeatureJustifier in just_features.cpp: build
// plain-C++ input from the Qt-side glyph/font data, call the Qt-free
// algorithm, then apply the result back onto the Qt-side layout structures.

#include <digitalkhatt/layout/OptimizeLayout.h>

#include <unordered_map>
#include <utility>
#include <vector>

#include "GlyphVis.h"
#include "LayoutWindow.h"
#include "automedina/automedina.h"
#include "digitalkhatt.h"

#if defined(ENABLE_PDF_GENERATION)
#include <QDir>
#include <QFileInfo>

#include "Pdf/ViolationReportWriter.h"
#endif

using namespace geometry;

void LayoutWindow::optimizeLayout(QList<QList<LineLayoutInfo>>& pages, const QList<QStringList>& originalPages, int beginPage, int nbPages, double emScale) {
  auto scale = emScale;

  std::unordered_map<GlyphVis*, GeometrySet> glyphToPolys;

  auto& classes = m_otlayout->automedina->classes;
  auto& marks = classes["marks"];
  auto& topmarks = classes["topmarks"];
  auto& lowmarks = classes["lowmarks"];
  auto& waqfmarks = classes["waqfmarks"];
  auto& topdotmarks = classes["topdotmarks"];
  auto& downdotmarks = classes["downdotmarks"];

  auto isTopMark = [&topmarks, &lowmarks, &waqfmarks, &topdotmarks, &downdotmarks](const std::string& glyphName) {
    return topmarks.contains(glyphName) || waqfmarks.contains(glyphName) || topdotmarks.contains(glyphName);
  };

  auto isBottomMark = [&topmarks, &lowmarks, &waqfmarks, &topdotmarks, &downdotmarks](QString glyphName) {
    auto glyphNameStd = glyphName.toStdString();
    return lowmarks.contains(glyphNameStd) || downdotmarks.contains(glyphNameStd);
  };

  // fetch all gryph initially otherwise mpost is not thread safe when
  // executing getAlternate

  std::vector<std::vector<std::vector<digitalkhatt::layout::GlyphInstance>>> pagesGlyphs;
  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];
    pagesGlyphs.push_back({});
    auto& pageGlyphs = pagesGlyphs.back();
    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];
      pageGlyphs.push_back({});
      auto& lineGlyphs = pageGlyphs.back();
      // To guarantee also the validity of the pointers in GlyphInstance
      lineGlyphs.reserve(line.glyphs.size());
      auto xScale = line.fontSize * line.xscale;
      auto yScale = line.fontSize;

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
      currentyPos = currentyPos * -1;
      digitalkhatt::layout::GlyphInstance* currentBase = nullptr;
      digitalkhatt::layout::GlyphInstance* prevBase = nullptr;

      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        const auto& glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        auto glyphVis = m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        auto glyphToPoly = glyphToPolys.find(glyphVis);

        if (glyphToPoly == glyphToPolys.end()) {
          if (marks.contains(glyphName)) {
            glyphToPoly = glyphToPolys.insert(
                                          {glyphVis,
                                           buildPolyFromCubics(
                                               getGlyphCubic(glyphVis->copiedPath),
                                               CUBIC_FLATNESS_TOLERANCE)
                                               .scaled(scale, scale)})
                              .first;
          } else {
            glyphToPoly = glyphToPolys.insert(
                                          {glyphVis,
                                           buildConvexPartsFromCubics(
                                               getGlyphCubic(glyphVis->copiedPath),
                                               CUBIC_FLATNESS_TOLERANCE)
                                               .scaled(scale, scale)})
                              .first;
          }
        }

        currentxPos -= glyphLayout.x_advance * line.xscale;

        auto& glyphInstance = lineGlyphs.emplace_back(digitalkhatt::layout::GlyphInstance{});

        glyphInstance.isMark = marks.contains(glyphName);
        glyphInstance.isTopMark = isTopMark(glyphName);
        glyphInstance.lineY = currentyPos;
        glyphInstance.baseX = currentxPos + (glyphLayout.x_offset * line.xscale);
        glyphInstance.baseY = currentyPos + (glyphLayout.y_offset);
        glyphInstance.glyphLayout = &glyphLayout;
        glyphInstance.metrics = {glyphVis->width, glyphVis->height, glyphVis->bbox.llx, glyphVis->bbox.urx};
        glyphInstance.glyphName = glyphName;
        glyphInstance.lineIndex = l;
        glyphInstance.glyphIndex = g;

        if (xScale == 1 && yScale == 1) {
          glyphInstance.geom = &glyphToPoly->second;
        } else {
          glyphInstance.geomScaled = glyphToPoly->second.scaled(xScale, yScale);
        }

        glyphInstance.prevBase = currentBase;

        if (!glyphInstance.isMark) {
          prevBase = currentBase;
          currentBase = &lineGlyphs.back();
          if (prevBase) {
            prevBase->nextBase = currentBase;
          }
        }
      }
    }
  }

  // optimize Pages (m_solverParams is a persistent member so the Solver
  // Tuning dock's edits take effect on the next render)
  auto coreClasses = classes;
  // Seed the bowl-base allowlist used by BowlClusterConstraint's named-override
  // detection, if the font's features.fea did not define one. Geometric
  // enclosure is the primary signal; this list is a safety net for the known
  // isolated/final hah-family bowls (Hah/Jeem/Khah). A font-defined
  // "bowlbases" class takes precedence and is left untouched.
  if (!coreClasses.contains("bowlbases")) {
    coreClasses["bowlbases"] = {"hah.isol", "hah.fina", "ain.fina"};
  }
#if defined(ENABLE_PDF_GENERATION)
  const bool report = m_solverParams.toggles.reportViolations;
#else
  const bool report = false;
#endif
  std::vector<std::vector<digitalkhatt::layout::ConstraintViolation>> allViolations;
  for (auto& page : pagesGlyphs) {
    std::vector<digitalkhatt::layout::ConstraintViolation> pageV;
    digitalkhatt::layout::optimizePage(page, coreClasses, m_solverParams,
                                       report ? &pageV : nullptr);
    if (report) allViolations.push_back(std::move(pageV));
  }

  // Update positions
  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];
    auto& pageGlyphs = pagesGlyphs[p];
    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];
      auto& lineGlyphs = pageGlyphs[l];
      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        auto& glyph = lineGlyphs[g];
        glyphLayout.x_offset += glyph.dx;
        glyphLayout.y_offset += glyph.dy;
      }
    }
  }

#if defined(ENABLE_PDF_GENERATION)
  if (report) {
    // Build one diagnostic page per solved page. Flatten line-then-glyph so
    // each glyph's position matches the globalIndex optimizePage assigned it
    // (the violations reference glyphs by that index). pagesGlyphs (and its
    // worldPolys) are still alive here.
    std::vector<ViolationReportWriter::Page> reportPages;
    reportPages.reserve(pagesGlyphs.size());
    // One row per violation across every page, severity-ordered by the
    // writer: page/line number plus the glyphs of just the word containing
    // it, so the summary PDF can show a small cropped rendering per row.
    std::vector<ViolationReportWriter::WordEntry> summaryEntries;
    for (size_t p = 0; p < pagesGlyphs.size(); ++p) {
      ViolationReportWriter::Page rp;
      for (auto& lineGlyphs : pagesGlyphs[p]) {
        for (auto& g : lineGlyphs) {
          rp.glyphs.push_back({&g.worldPolys, g.glyphName, g.globalIndex});
        }
      }
      if (p < allViolations.size()) rp.violations = std::move(allViolations[p]);

      // Page-global-index -> line lookup, used below to find the word
      // (space-delimited run of glyphs, "space" being the literal glyph
      // HarfBuzz shaping emits for whitespace) containing each violation.
      // Built from pagesGlyphs directly rather than rp.glyphs so each entry
      // keeps a GlyphInstance* (rp.glyphs only has the projected GlyphRef).
      std::vector<digitalkhatt::layout::GlyphInstance*> flat;
      std::vector<int> lineStart;  // first global index of each line, + a
                                   // trailing sentinel = total glyph count
      for (auto& lineGlyphs : pagesGlyphs[p]) {
        lineStart.push_back(static_cast<int>(flat.size()));
        for (auto& g : lineGlyphs) flat.push_back(&g);
      }
      lineStart.push_back(static_cast<int>(flat.size()));

      for (const auto& v : rp.violations) {
        const int anchor = v.glyphA >= 0 ? v.glyphA : v.glyphB;
        if (anchor < 0 || anchor >= static_cast<int>(flat.size())) continue;

        int line = 0;
        while (line + 1 < static_cast<int>(lineStart.size()) - 1 && lineStart[line + 1] <= anchor) line++;
        const int lineLo = lineStart[line];
        const int lineHi = lineStart[line + 1];  // exclusive

        int left = anchor;
        while (left > lineLo && flat[left - 1]->glyphName != "space") left--;
        int right = anchor;
        while (right + 1 < lineHi && flat[right + 1]->glyphName != "space") right++;

        ViolationReportWriter::WordEntry entry;
        entry.pageNumber = static_cast<int>(p) + 1;
        entry.lineNumber = line + 1;
        entry.violation = v;
        entry.wordGlyphs.reserve(right - left + 1);
        for (int idx = left; idx <= right; idx++) {
          if (flat[idx]->glyphName == "space") continue;
          entry.wordGlyphs.push_back({&flat[idx]->worldPolys, flat[idx]->glyphName, idx});
        }
        summaryEntries.push_back(std::move(entry));
      }

      reportPages.push_back(std::move(rp));
    }

    QFileInfo fi(m_font->filePath());
    QDir().mkpath(fi.path() + "/output");
    const QString base = fi.path() + "/output/violations";
    ViolationReportWriter writer;
    writer.write(reportPages, base + ".pdf", base + ".csv");
    writer.writeSummary(summaryEntries, base + "_summary.pdf");
  }
#endif
}
