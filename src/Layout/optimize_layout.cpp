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
  bool report = m_solverParams.toggles.reportViolations;
  ViolationReportWriter writer;
  QString base;
  const auto flag = [](bool enabled) { return enabled ? "on" : "off"; };
  const auto& toggles = m_solverParams.toggles;
  const std::vector<std::string> notes = {
      std::string("Placement audit (side, class, owner): ") + flag(toggles.reportPlacementAudit) +
          ". Blue dashed: shaped position; red/amber: flagged result.",
      QString("Report generic gaps: %1; soft targets: %2; minimum excess severity: %3")
          .arg(flag(toggles.reportGenericGap)).arg(flag(toggles.reportSoftResiduals))
          .arg(m_solverParams.minViolationSeverity).toStdString(),
      QString("Gap report: %1; Save Collision clearance: %2 font units")
          .arg(m_solverParams.reportGenericGapCollisionsOnly ? "Save Collision cases" : "all solver residuals")
          .arg(m_solverParams.collisionReportMinGap).toStdString(),
      QString("Sort: %1; maximum findings: %2; only new/worsened (plus structural): %3")
          .arg(QString::fromStdString(m_solverParams.reportSort))
          .arg(m_solverParams.reportMaxFindings ? QString::number(m_solverParams.reportMaxFindings) : QString("unlimited"))
          .arg(flag(m_solverParams.reportOnlyChanged)).toStdString(),
      QString("BaseVicinity reporting tolerance (% of mark width): marks %1; dots %2")
          .arg(m_solverParams.baseVicinityMarkTolerancePercent)
          .arg(m_solverParams.baseVicinityDotTolerancePercent).toStdString(),
      QString("Waqf allowed left/right drift (% of waqf width): %1/%2")
          .arg(m_solverParams.waqfLeftDriftTolerancePercent).arg(m_solverParams.waqfRightDriftTolerancePercent).toStdString(),
      QString("Waqf previous-baseline margin (% of line spacing): %1; solver bound residuals: %2")
          .arg(m_solverParams.waqfPreviousLineMarginPercent).arg(flag(toggles.reportWaqfBounds)).toStdString()};
  if (report) {
    QFileInfo fi(m_font->filePath());
    QDir().mkpath(fi.path() + "/output");
    base = fi.path() + "/output/violations";
    report = writer.start((base + ".pdf").toStdString(), (base + ".csv").toStdString(), m_solverParams);
    if (!report) qWarning() << "XPBD violation report could not be started" << base;
  }
#else
  const bool report = false;
#endif
  digitalkhatt::layout::PlacementPipeline pipeline(*m_otlayout, emScale);
  for (int p = beginPage; p < beginPage + nbPages; ++p) {
    auto solved = pipeline.solve(pages[p], m_solverParams, true, report);
#if defined(ENABLE_PDF_GENERATION)
    if (!report) continue;
    std::unordered_set<const digitalkhatt::layout::GlyphInstance*> members;
    for (const auto& line : solved.glyphs)
      for (const auto& g : line) members.insert(&g);
    ViolationReportWriter::Page page;
    page.pageNumber = p + 1;
    page.notes = notes;
    page.violations = std::move(solved.violations);
    for (const auto& line : solved.glyphs) {
      for (const auto& g : line) {
        ViolationReportWriter::GlyphRef ref{&solved.reportGeometry(g), g.glyphName, g.globalIndex,
            g.isMark && members.contains(g.prevBase) ? g.prevBase->globalIndex : -1,
            g.lineIndex + 1, g.dx, g.dy};
        ref.cluster = g.glyphLayout ? static_cast<int>(g.glyphLayout->cluster) : -1;
        if (p < static_cast<int>(originalPages.size()) && g.lineIndex < static_cast<int>(originalPages[p].size())) {
          const auto& text = originalPages[p][g.lineIndex];
          ref.wordNumber = digitalkhatt::layout::violationWordNumber(text, ref.cluster);
          if (ref.wordNumber) {
            std::size_t begin = ref.cluster, end = ref.cluster;
            while (begin && text[begin - 1] != u' ') --begin;
            while (end < text.size() && text[end] != u' ') ++end;
            ref.wordText = QString::fromStdU16String(text.substr(begin, end - begin)).toUtf8().toStdString();
          }
        }
        page.glyphs.push_back(std::move(ref));
      }
    }
    if (!writer.appendPage(page)) qWarning() << "XPBD report page could not be collected" << p + 1;
#endif
  }
#if defined(ENABLE_PDF_GENERATION)
  if (report) {
    if (!writer.finish()) qWarning() << "XPBD violation report could not be written" << base;
    auto summary = writer.summaryEntries();
    auto summaryNotes = notes;
    summaryNotes.push_back("Selected " + std::to_string(writer.selectedCount()) + " of " +
        std::to_string(writer.eligibleCount()) + " eligible findings; same selection as CSV and page overview.");
    if (!writer.writeSummary(summary, base + "_summary.pdf", summaryNotes))
      qWarning() << "XPBD violation summary could not be written" << base;
    if (!writer.writeCompact(summary, (base + "_compact.pdf").toStdString(), summaryNotes))
      qWarning() << "XPBD compact violation report could not be written" << base;
    if (!writer.writeWeb(summary, (base + ".html").toStdString(), summaryNotes))
      qWarning() << "XPBD web violation report could not be written" << base;
  }
#endif
}
