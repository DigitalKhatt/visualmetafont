#pragma once

#include <string>
#include <vector>

#include <filesystem>
#include <fstream>
#include <memory>

#include "digitalkhatt/geometry/geometry.h"
#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/OptParams.h"

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
namespace digitalkhatt::pdf {

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
    int baseGlobalIndex = -1;
    int lineNumber = 0;
    double dx = 0.0, dy = 0.0;
    std::shared_ptr<const geometry::GeometrySet> ownedGeometry;
    std::string wordText;
    int cluster = -1;
    int wordNumber = 0;  // 1-based, source word within its line
  };

  // One diagnostic page. `glyphs` is indexed by GlyphInstance::globalIndex so
  // violations (which reference glyphs by that index) resolve directly.
  struct Page {
    std::vector<GlyphRef> glyphs;
    std::vector<digitalkhatt::layout::ConstraintViolation> violations;
    int pageNumber = 0;
    std::vector<std::string> notes;
  };

  // Writes both the PDF and the log. A missing label font degrades gracefully
  // (glyph outlines + markers are still drawn; only the text labels are
  // skipped). Returns true on success.
  bool write(const std::vector<Page>& pages,
             const std::filesystem::path& pdfPath,
             const std::filesystem::path& logPath,
             const layout::OptParams& params = {});

  bool start(const std::filesystem::path& pdfPath, const std::filesystem::path& logPath,
             const layout::OptParams& params = {});
  // Collect globally ranked findings. Geometry for retained pages is owned
  // here, so a streaming caller may release its solved page immediately.
  bool appendPage(const Page& page);
  bool finish();

  // One summary row with both participants' words and owning bases.
  // summaryEntries() derives this context from the retained page's glyphs.
  struct WordEntry {
    int pageNumber = 0;  // 1-based, for display
    int lineNumber = 0;  // 1-based, for display
    int otherLineNumber = 0;
    digitalkhatt::layout::ConstraintViolation violation;
    std::vector<GlyphRef> wordGlyphs;  // participating words and owning bases
    int wordNumber = 0, otherWordNumber = 0;
    std::size_t rank = 0;  // matches CSV report_rank and both PDF summaries
  };

  // Ordered index of retained violations across all pages:
  // one row per violation with page number, line number, constraint type and
  // severity, plus a small rendering of the participating words and bases.
  // Critical structural errors and structural review warnings have separate
  // sections from geometric residuals. `entries` is sorted in place.
  bool writeSummary(std::vector<WordEntry>& entries, const std::filesystem::path& pdfPath,
                    const std::vector<std::string>& notes = {});

  // Dense visual index: 30 vector word-context crops per landscape A4 page.
  bool writeCompact(std::vector<WordEntry>& entries, const std::filesystem::path& pdfPath,
                    const std::vector<std::string>& notes = {});

  // Self-contained offline viewer with vector contexts, filters and review status.
  bool writeWeb(const std::vector<WordEntry>& entries, const std::filesystem::path& htmlPath,
                const std::vector<std::string>& notes = {}) const;

  // Available after finish(); uses exactly the CSV/overview selection/order.
  std::vector<WordEntry> summaryEntries() const;
  std::size_t selectedCount() const { return m_selected.size(); }
  std::size_t eligibleCount() const { return m_eligibleCount; }

 private:
  struct Finding {
    std::shared_ptr<Page> page;
    layout::ConstraintViolation violation;
    std::size_t sequence = 0;
    int pageNumber = 0;
  };
  bool precedes(const Finding& a, const Finding& b) const;
  bool drawPage(const Page& page);
  void writeCsvFinding(const Finding& finding, std::size_t rank);

  layout::OptParams m_params;
  std::vector<Finding> m_selected;  // worst-first heap until finish()
  std::size_t m_sequence = 0;
  std::size_t m_eligibleCount = 0;
  std::vector<std::string> m_notes;
  PDFWriter m_writer;
  std::ofstream m_log;
  PDFUsedFont* m_labelFont = nullptr;
  int m_pageIndex = 0;
  bool m_started = false;

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

}  // namespace digitalkhatt::pdf
