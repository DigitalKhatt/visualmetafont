#include "digitalkhatt/pdf/ViolationReportWriter.h"
#include "ViolationReportWebTemplate.h"

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <sstream>

namespace digitalkhatt::pdf {
namespace {

// JSON embedded in a script element must also escape HTML delimiters, including
// a source word or diagnostic containing </script>. Keep Unicode source text.
void string(std::ostream& out, const std::string& value) {
  out << '"';
  for (unsigned char c : value) {
    switch (c) {
      case '"': out << "\\\""; break;
      case '\\': out << "\\\\"; break;
      case '<': out << "\\u003c"; break;
      case '>': out << "\\u003e"; break;
      case '&': out << "\\u0026"; break;
      default:
        if (c < 32) out << "\\u00" << "0123456789abcdef"[c >> 4] << "0123456789abcdef"[c & 15];
        else out << static_cast<char>(c);
    }
  }
  out << '"';
}

double finite(double value) { return std::isfinite(value) ? value : 0.0; }

// Exactly the same unsplit mark hulls and decomposed base geometry as the PDFs.
std::string svgPath(const geometry::GeometrySet& geometry) {
  std::ostringstream out;
  out.imbue(std::locale::classic());
  out << std::fixed << std::setprecision(3);
  for (const auto& poly : geometry.polys()) {
    if (poly.empty()) continue;
    if (!std::all_of(poly.begin(), poly.end(), [](const auto& p) {
          return std::isfinite(p.x) && std::isfinite(p.y);
        })) continue;
    out << 'M' << poly.front().x << ' ' << poly.front().y;
    for (std::size_t i = 1; i < poly.size(); ++i) out << 'L' << poly[i].x << ' ' << poly[i].y;
    out << 'Z';
  }
  return out.str();
}

}  // namespace

bool ViolationReportWriter::writeWeb(const std::vector<WordEntry>& entries,
    const std::filesystem::path& htmlPath, const std::vector<std::string>& notes) const {
  std::ostringstream data;
  data.imbue(std::locale::classic());
  data << std::setprecision(12) << "{\"eligible\":" << eligibleCount()
       << ",\"sort\":";
  string(data, m_params.reportSort);
  data << ",\"notes\":[";
  for (std::size_t i = 0; i < notes.size(); ++i) { if (i) data << ','; string(data, notes[i]); }
  data << "],\"entries\":[";
  for (std::size_t i = 0; i < entries.size(); ++i) {
    if (i) data << ',';
    const auto& e = entries[i];
    const auto& v = e.violation;
    data << "{\"rank\":" << (e.rank ? e.rank : i + 1) << ",\"page\":" << e.pageNumber
         << ",\"line\":" << e.lineNumber << ",\"word\":" << e.wordNumber
         << ",\"otherLine\":" << e.otherLineNumber << ",\"otherWord\":" << e.otherWordNumber
         << ",\"type\":";
    string(data, layout::violationTypeName(v.type));
    data << ",\"hard\":" << (v.kind == layout::ViolationKind::Hard ? "true" : "false")
         << ",\"structural\":" << (v.structural ? "true" : "false")
         << ",\"severity\":" << layout::violationReportSeverity(v)
         << ",\"rawSeverity\":" << finite(v.severity) << ",\"allowed\":" << finite(v.allowedResidual)
         << ",\"residual\":" << finite(v.residual) << ",\"initial\":" << finite(v.initialSeverity)
         << ",\"group\":" << layout::violationReportGroup(v)
         << ",\"priority\":" << layout::violationReviewPriority(v) << ",\"status\":";
    string(data, v.introduced ? "new" : v.worsened ? "worse" : "unchanged");
    data << ",\"detail\":"; string(data, v.detail);
    data << ",\"diagnostic\":"; string(data, v.diagnostic);
    if (v.waqf) {
      const auto& m = *v.waqf;
      data << ",\"waqf\":{\"base\":" << m.baseIndex
           << ",\"horizontalOffset\":" << finite(m.horizontalOffset)
           << ",\"allowedLeft\":" << finite(m.allowedLeftDrift)
           << ",\"allowedRight\":" << finite(m.allowedRightDrift)
           << ",\"heightAboveBaseline\":" << finite(m.heightAboveBaseline);
      const auto optional = [&](const char* name, const std::optional<double>& value) {
        data << ",\"" << name << "\":";
        if (value) data << finite(*value); else data << "null";
      };
      optional("previousBaselineDistance", m.previousBaselineDistance);
      optional("previousLineMargin", m.previousLineMargin);
      optional("previousInkBoxClearance", m.previousInkBoxClearance);
      data << '}';
    }
    data << ",\"a\":" << v.glyphA << ",\"b\":" << v.glyphB << ",\"markers\":[";
    for (int m = 0; m < std::clamp(v.markerCount, 0, 2); ++m) {
      if (m) data << ',';
      data << '[' << finite(v.marker[m].x) << ',' << finite(v.marker[m].y) << ']';
    }
    data << "],\"glyphs\":[";
    bool first = true;
    for (const auto& g : e.wordGlyphs) {
      if (!g.worldPolys) continue;
      const auto box = g.worldPolys->boundingAABB();
      if (!std::isfinite(box.minx) || !std::isfinite(box.miny) ||
          !std::isfinite(box.maxx) || !std::isfinite(box.maxy)) continue;
      if (!first) data << ',';
      first = false;
      data << "{\"index\":" << g.globalIndex << ",\"base\":" << g.baseGlobalIndex
           << ",\"line\":" << g.lineNumber << ",\"word\":" << g.wordNumber << ",\"name\":";
      string(data, g.name);
      data << ",\"text\":"; string(data, g.wordText);
      data << ",\"dx\":" << finite(g.dx) << ",\"dy\":" << finite(g.dy)
           << ",\"box\":[" << box.minx << ',' << box.miny << ',' << box.maxx << ',' << box.maxy << "]"
           << ",\"path\":";
      string(data, svgPath(*g.worldPolys));
      data << '}';
    }
    data << "]}";
  }
  data << "]}";
  const auto json = data.str();
  // Namespace saved review decisions by report contents, not merely its name.
  std::uint64_t fingerprint = 14695981039346656037ULL;
  for (unsigned char c : json) { fingerprint ^= c; fingerprint *= 1099511628211ULL; }
  std::ofstream out(htmlPath, std::ios::binary);
  out << web::beforeData << json << web::afterData;
  out << "\n<script>startReport(\"" << std::hex << fingerprint << "\");</script>\n</body></html>\n";
  out.flush();
  return bool(out);
}

}  // namespace digitalkhatt::pdf
