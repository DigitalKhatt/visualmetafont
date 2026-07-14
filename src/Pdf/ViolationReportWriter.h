#pragma once

#include <string>
#include <vector>

#include <QString>

#include "digitalkhatt/geometry/geometry.h"
#include "digitalkhatt/layout/ConstraintViolation.h"

// PDF-Writer / PDFHummus
#include "PDFWriter.h"

class PageContentContext;
class PDFUsedFont;

// Standalone diagnostic PDF (+ companion text log) of the constraint violations
// left after the XPBD layout solver runs. Each page draws the solved glyph
// outlines in the solver's own coordinate space (worldPolys), with offending
// glyphs highlighted in red and violation markers/labels overlaid -- the PDF
// analogue of the on-screen debugDistance() visualizer.
//
// Sibling to QuranPdfWriterPdfHummus rather than an extension of it: this needs
// none of that class's OtLayout / Type3-font / surah-frame machinery, just a
// plain PDFWriter and the reusable path/geometry helpers.
class ViolationReportWriter {
 public:
  struct GlyphRef {
    const geometry::GeometrySet* worldPolys = nullptr;  // final solved geometry
    std::string name;
    // Page-global index (GlyphInstance::globalIndex) this ref came from --
    // ConstraintViolation::glyphA/glyphB are matched against this, not
    // against the ref's position, so a caller may hand in either a whole
    // page's glyphs (position == globalIndex, the common case) or an
    // arbitrary cropped subset (e.g. one word's glyphs for writeSummary()).
    int globalIndex = -1;
  };

  // One diagnostic page. `glyphs` is indexed by GlyphInstance::globalIndex so
  // violations (which reference glyphs by that index) resolve directly.
  struct Page {
    std::vector<GlyphRef> glyphs;
    std::vector<digitalkhatt::layout::ConstraintViolation> violations;
  };

  // Writes both the PDF and the log. A missing label font degrades gracefully
  // (glyph outlines + markers are still drawn; only the text labels are
  // skipped). Returns true on success.
  bool write(const std::vector<Page>& pages,
             const QString& pdfPath,
             const QString& logPath);

  // One row of the severity-ordered summary report: the caller resolves
  // page/line number and the glyphs of the single word containing the
  // violation (word-boundary detection needs Qt-side glyph-name/text
  // knowledge this class doesn't otherwise depend on).
  struct WordEntry {
    int pageNumber = 0;  // 1-based, for display
    int lineNumber = 0;  // 1-based, for display
    digitalkhatt::layout::ConstraintViolation violation;
    std::vector<GlyphRef> wordGlyphs;  // just this word's glyphs, worldPolys space
  };

  // Severity-ordered (descending) index of every violation across all pages:
  // one row per violation with page number, line number, constraint type and
  // severity, plus a small cropped rendering of the word containing it.
  // Complements write()'s per-page overview. `entries` is sorted in place.
  bool writeSummary(std::vector<WordEntry>& entries, const QString& pdfPath);

 private:
  PDFWriter m_writer;

  // Draws glyph outlines (light gray), offending glyphs (red) and violation
  // markers (magenta) for `glyphs`/`violations`, uniformly fit into the
  // device rect (x,y,w,h) on the page currently open on `ctx`. Offenders are
  // matched by GlyphRef::globalIndex against violation.glyphA/glyphB, not by
  // vector position. Shared by the full-page overview (write()) and the
  // per-word crops (writeSummary()); `labelFont` may be null to skip the
  // per-marker text labels (used for the crops, where the row's own text
  // columns already carry that information).
  void drawScene(PageContentContext* ctx,
                 const std::vector<GlyphRef>& glyphs,
                 const std::vector<digitalkhatt::layout::ConstraintViolation>& violations,
                 double x, double y, double w, double h,
                 PDFUsedFont* labelFont);
};
