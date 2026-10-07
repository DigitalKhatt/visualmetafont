#include "PlacementPipeline.h"

#include <map>
#include <tuple>
#include "GlyphVis.h"
#include "digitalkhatt/layout/GlyphCollisionGeometry.h"
#include "digitalkhatt/layout/CollisionReport.h"

namespace digitalkhatt::layout {

PlacementPipeline::PlacementPipeline(OtLayout& layout, double emScale)
    : layout_(layout), emScale_(emScale) {}

void PlacementPipeline::collectCollisions(const std::vector<LineLayoutInfo>& page,
    PlacementPage& result, double minimumGap, std::vector<ConstraintViolation>& violations) {
  std::size_t count = 0;
  for (const auto& line : result.glyphs) count += line.size();
  result.collisionGeometry.clear();
  result.collisionGeometry.reserve(count);
  std::vector<CollisionGlyph> glyphs;
  glyphs.reserve(count);
  for (std::size_t l = 0; l < page.size(); ++l) {
    const auto& line = page[l];
    // Match Save Collision's integer placement after solved offsets are rounded.
    int x = -line.xstartposition;
    const int y = -(line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY));
    int word = 0;
    for (const auto& g : result.glyphs[l]) {
      const auto& positioned = line.glyphs[g.glyphIndex];
      auto* outline = layout_.getGlyph(positioned);
      auto found = collisionGeometry_.find(outline);
      if (found == collisionGeometry_.end())
        found = collisionGeometry_.emplace(outline, geometry::buildConvexPartsFromCubics(
            geometry::getGlyphCubic(outline->copiedPath), geometry::CUBIC_FLATNESS_TOLERANCE)).first;
      x -= positioned.x_advance * line.xscale;
      const int px = x + positioned.x_offset * line.xscale;
      const int py = y + positioned.y_offset;
      auto& geometry = result.collisionGeometry.emplace_back(found->second.scaleTranslate(
          line.fontSize * line.xscale, line.fontSize, px, py));
      glyphs.push_back({&geometry, g.glyphName, g.lineIndex, word, g.isMark, line.fontSize});
      if (isCollisionSpace(g.glyphName)) ++word;
    }
  }
  for (const auto& c : findCollisionContacts(glyphs, minimumGap * emScale_)) {
    ConstraintViolation v;
    v.type = ViolationType::GenericGap;
    v.kind = ViolationKind::Hard;
    v.residual = c.contact.depth_or_gap - c.minimumGap;
    v.severity = -v.residual;
    v.diagnostic = "save-collision";
    v.detail = c.contact.depth_or_gap < 0.0 ? "Ink intersection (Save Collision)"
                                          : "Clearance below Save Collision minimum";
    v.glyphA = static_cast<int>(c.first);
    v.glyphB = static_cast<int>(c.second);
    v.markerCount = 2;
    v.marker[0] = c.contact.pA;
    v.marker[1] = c.contact.pB;
    violations.push_back(std::move(v));
  }
}

PlacementPage PlacementPipeline::solve(std::vector<LineLayoutInfo>& page,
    const OptParams& params, bool force, bool report) {
  PlacementPage result;
  auto classes = layout_.glyphClasses();
  if (!classes.contains("bowlbases")) classes["bowlbases"] = {"hah.isol", "hah.fina", "ain.fina"};
  const auto& marks = classesOrEmpty(classes, "marks");
  const auto& top = classesOrEmpty(classes, "topmarks");
  const auto& dots = classesOrEmpty(classes, "topdotmarks");
  const auto& waqf = classesOrEmpty(classes, "waqfmarks");
  result.glyphs.reserve(page.size());
  for (int l = 0; l < static_cast<int>(page.size()); ++l) {
    auto& line = page[l];
    auto& glyphs = result.glyphs.emplace_back();
    // The Mushaf exporter draws surah icons and their frame, rather than the
    // shaped header glyphs. They must not participate in placement forces.
    if (line.type == LineType::Sura) continue;
    const double xscale = line.type == LineType::Line ? line.xscale : 1.0;
    glyphs.reserve(line.glyphs.size());
    double x = -line.xstartposition;
    const double y = -(line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY));
    GlyphInstance* base = nullptr;
    for (int i = 0; i < static_cast<int>(line.glyphs.size()); ++i) {
      auto& positioned = line.glyphs[i];
      auto* outline = layout_.getGlyph(positioned);
      if (!outline) throw std::runtime_error("Missing outline in placement pipeline");
      const auto& name = layout_.glyphNamePerCode.at(positioned.codepoint);
      auto geom = geometry_.find(outline);
      if (geom == geometry_.end()) {
        auto cubics = geometry::getGlyphCubic(outline->copiedPath);
        auto polygons = buildGlyphCollisionGeometry(cubics, marks.contains(name));
        // fontSize already includes emScale; multiplying it twice makes the
        // solver's ink disagree with the exported PDF at other font sizes.
        geom = geometry_.emplace(outline, std::move(polygons)).first;
      }
      x -= positioned.x_advance * xscale;
      auto& g = glyphs.emplace_back();
      g.isMark = marks.contains(name);
      g.isTopMark = top.contains(name) || dots.contains(name) || waqf.contains(name);
      g.glyphName = name;
      g.lineIndex = l; g.glyphIndex = i;
      g.lineY = y;
      g.baseX = x + positioned.x_offset * xscale;
      g.baseY = y + positioned.y_offset;
      g.glyphLayout = &positioned;
      g.metrics = {outline->width, outline->height, outline->bbox.llx, outline->bbox.urx};
      g.geomScaled = geom->second.scaled(line.fontSize * xscale, line.fontSize);
      g.prevBase = base;
      if (!g.isMark) {
        if (base) base->nextBase = &g;
        base = &g;
      }
    }
  }
  auto options = params;
  const bool collisionReport = report && params.toggles.reportGenericGap && params.reportGenericGapCollisionsOnly;
  if (report) {
    auto initialOptions = options;
    initialOptions.maxIters = 0;
    optimizePage(result.glyphs, classes, initialOptions, &result.initialViolations);
    if (collisionReport) collectCollisions(page, result, params.collisionReportMinGap, result.initialViolations);
  }
  if (!force) options.maxIters = 0;
  optimizePage(result.glyphs, classes, options, report ? &result.violations : nullptr);
  if (force) {
    for (size_t l = 0; l < page.size(); ++l)
      for (size_t i = 0; i < result.glyphs[l].size(); ++i)
        applySolvedGlyphOffsets(page[l].glyphs[i], result.glyphs[l][i],
            page[l].type == LineType::Line ? page[l].xscale : 1.0);
  }
  if (collisionReport) collectCollisions(page, result, params.collisionReportMinGap, result.violations);
  if (report) {
    using Key = std::tuple<ViolationType, int, int, std::string>;
    const auto key = [](const ConstraintViolation& v) -> Key {
      // The nearest previous-line glyph can change during solving. Association
      // findings still refer to the same waqf, its owner, and the same axis.
      return {v.type, v.glyphA, v.waqf ? v.waqf->baseIndex : v.glyphB, v.diagnostic};
    };
    // Keep the raw initial value for display, and the excess for association
    // change detection (left/right allowances can differ).
    std::map<Key, std::pair<double, double>> before;
    for (const auto& v : result.initialViolations)
      before[key(v)] = {v.severity, violationReportSeverity(v)};
    for (auto& v : result.violations) {
      const auto prior = before.find(key(v));
      if (prior != before.end()) v.initialSeverity = prior->second.first;
      v.introduced = force && prior == before.end();
      v.worsened = force && prior != before.end() &&
          (v.waqf ? violationReportSeverity(v) > prior->second.second + params.tolCollision
                  : v.severity > prior->second.first + params.tolCollision);
    }
  }
  return result;
}

}  // namespace digitalkhatt::layout
