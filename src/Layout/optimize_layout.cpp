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

// Shared native placement pipeline; Qt only supplies editor preferences and paths.
#include "LayoutWindow.h"
#include "font.hpp"
#include "Layout/PlacementPipeline.h"
#include <digitalkhatt/layout/ViolationReportContext.h>
#include <unordered_set>
#if defined(ENABLE_PDF_GENERATION)
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include "Pdf/ViolationReportWriter.h"
#endif

void LayoutWindow::optimizeLayout(LayoutPageList& pages, const OriginalPageList& originalPages,
    int beginPage, int nbPages, double emScale) {
  (void)originalPages;
#if defined(ENABLE_PDF_GENERATION)
  const bool report = m_solverParams.toggles.reportViolations;
#else
  const bool report = false;
#endif
  digitalkhatt::layout::PlacementPipeline pipeline(*m_otlayout, emScale);
  std::vector<std::vector<std::vector<digitalkhatt::layout::GlyphInstance>>> pagesGlyphs;
  std::vector<std::vector<digitalkhatt::layout::ConstraintViolation>> allViolations;
  for (int p = beginPage; p < beginPage + nbPages; ++p) {
    auto solved = pipeline.solve(pages[p], m_solverParams, true, report);
    pagesGlyphs.push_back(std::move(solved.glyphs));
    if (report) allViolations.push_back(std::move(solved.violations));
  }
#if defined(ENABLE_PDF_GENERATION)
  if (report) {
    const auto& toggles = m_solverParams.toggles;
    const auto flag = [](bool enabled) { return enabled ? "on" : "off"; };
    const std::vector<std::string> notes = {
        "Final side, class and base-association audit: on. Blue dashed: shaped position; red/amber: flagged result.",
        QString("Report generic gaps: %1; soft targets: %2; cutoff: %3; gap slack margin: %4")
            .arg(flag(toggles.reportGenericGap)).arg(flag(toggles.reportSoftResiduals))
            .arg(m_solverParams.minViolationSeverity).arg(m_solverParams.complianceResidualMargin).toStdString(),
        QString("Forces: lane %1; stack %2; vicinity %3; order %4; waqf %5; gap %6; side rails %7; match %8")
            .arg(flag(toggles.ylane)).arg(flag(toggles.stackOrder)).arg(flag(toggles.baseVicinity))
            .arg(flag(toggles.horizontalOrder)).arg(flag(toggles.waqfPlacement)).arg(flag(toggles.genericGapConstraint))
            .arg(flag(toggles.hardStayAboveBelow)).arg(flag(toggles.matchMarkPosition)).toStdString()};
    std::unordered_set<const digitalkhatt::layout::GlyphInstance*> reportMembers;
    for (const auto& page : pagesGlyphs)
      for (const auto& line : page)
        for (const auto& g : line) reportMembers.insert(&g);
    const auto glyphRef = [&](const digitalkhatt::layout::GlyphInstance& g) {
      return ViolationReportWriter::GlyphRef{&g.worldPolys, g.glyphName, g.globalIndex,
          g.isMark && reportMembers.contains(g.prevBase) ? g.prevBase->globalIndex : -1,
          g.lineIndex + 1, g.dx, g.dy};
    };
    // Build one diagnostic page per solved page. Flatten line-then-glyph so
    // each glyph's position matches the globalIndex optimizePage assigned it
    // (the violations reference glyphs by that index). pagesGlyphs (and its
    // worldPolys) are still alive here.
    std::vector<ViolationReportWriter::Page> reportPages;
    reportPages.reserve(pagesGlyphs.size());
    // One row per violation across every page, severity-ordered by the
    // writer: page/line number plus the participating words and owning bases,
    // so the summary PDF can show their relationship in each row.
    std::vector<ViolationReportWriter::WordEntry> summaryEntries;
    for (size_t p = 0; p < pagesGlyphs.size(); ++p) {
      ViolationReportWriter::Page rp;
      rp.pageNumber = beginPage + static_cast<int>(p) + 1;
      rp.notes = notes;
      for (auto& lineGlyphs : pagesGlyphs[p]) {
        for (auto& g : lineGlyphs) {
          rp.glyphs.push_back(glyphRef(g));
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
        ViolationReportWriter::WordEntry entry;
        entry.pageNumber = rp.pageNumber;
        entry.lineNumber = line + 1;
        if (v.glyphB >= 0 && v.glyphB < static_cast<int>(flat.size()))
          entry.otherLineNumber = flat[v.glyphB]->lineIndex + 1;
        entry.violation = v;
        for (int idx : digitalkhatt::layout::violationContextIndices(flat, lineStart, v))
          entry.wordGlyphs.push_back(glyphRef(*flat[idx]));
        summaryEntries.push_back(std::move(entry));
      }

      reportPages.push_back(std::move(rp));
    }

    QFileInfo fi(m_font->filePath());
    QDir().mkpath(fi.path() + "/output");
    const QString base = fi.path() + "/output/violations";
    ViolationReportWriter writer;
    if (!writer.write(reportPages, base + ".pdf", base + ".csv"))
      qWarning() << "XPBD violation report could not be written" << base;
    if (!writer.writeSummary(summaryEntries, base + "_summary.pdf", notes))
      qWarning() << "XPBD violation summary could not be written" << base;
  }
#endif
}
