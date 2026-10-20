#include "PlacementPipeline.h"

#include <map>
#include <tuple>
#include "GlyphVis.h"
#include "digitalkhatt/layout/GlyphCollisionGeometry.h"
#include "digitalkhatt/layout/CollisionReport.h"
#include "digitalkhatt/layout/CollisionMarkPolicy.h"
#include "digitalkhatt/layout/constraints/MarkFrameConstraint.h"

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
    // Apply the line transform after solved offsets are rounded in font units.
    double x = -line.xstartposition;
    const int y = -(line.ystartposition - (layout_.topSpace() << OtLayout::SCALEBY));
    int word = 0;
    for (const auto& g : result.glyphs[l]) {
      const auto& positioned = line.glyphs[g.glyphIndex];
      auto* outline = layout_.getGlyph(positioned);
      auto found = collisionGeometry_.find(outline);
      if (found == collisionGeometry_.end())
        found = collisionGeometry_.emplace(outline, geometry::buildConvexPartsFromCubics(
                                                        geometry::getGlyphCubic(outline->copiedPath), geometry::CUBIC_FLATNESS_TOLERANCE))
                    .first;
      x -= positioned.x_advance * line.horizontalScale();
      const double px = x + positioned.x_offset * line.horizontalScale();
      const double py = y + positioned.y_offset * line.fontSize;
      auto& geometry = result.collisionGeometry.emplace_back(found->second.scaleTranslate(
          line.horizontalScale(), line.fontSize, px, py));
      glyphs.push_back({&geometry, g.glyphName, g.lineIndex, word, g.isCollisionMark(), line.fontSize});
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
  if (!classes.contains("lowerMarkGapExcludedBases")) classes["lowerMarkGapExcludedBases"] = {"hah.isol", "hah.fina"};
  const auto& marks = classesOrEmpty(classes, "marks");
  const auto& top = classesOrEmpty(classes, "topmarks");
  const auto& dots = classesOrEmpty(classes, "topdotmarks");
  const auto& waqf = classesOrEmpty(classes, "waqfmarks");
  const auto& pageLayout = layout_.pageLayout();
  const double scale = 1 << OtLayout::SCALEBY;
  // Same outer rectangle as GraphicsSceneAdjustment::pageRect(), with Y up.
  const geometry::AABB frame{(-pageLayout.pageWidth + pageLayout.rightMargin) * scale,
      (pageLayout.topSpace - pageLayout.pageHeight) * scale,
      pageLayout.rightMargin * scale, pageLayout.topSpace * scale};
  result.glyphs.reserve(page.size());
  for (int l = 0; l < static_cast<int>(page.size()); ++l) {
    auto& line = page[l];
    auto& glyphs = result.glyphs.emplace_back();
    // The Mushaf exporter draws surah icons and their frame, rather than the
    // shaped header glyphs. They must not participate in placement forces.
    if (line.type == LineType::Sura) continue;
    const double xscale = line.horizontalScale();
    glyphs.reserve(line.glyphs.size());
    double x = -line.xstartposition;
    const double y = -(line.ystartposition - (layout_.topSpace() << OtLayout::SCALEBY));
    GlyphInstance* base = nullptr;
    for (int i = 0; i < static_cast<int>(line.glyphs.size()); ++i) {
      auto& positioned = line.glyphs[i];
      auto* outline = layout_.getGlyph(positioned);
      if (!outline) throw std::runtime_error("Missing outline in placement pipeline");
      const auto& name = layout_.glyphNamePerCode.at(positioned.codepoint);
      const bool collisionMark = marks.contains(name) || isAdditionalCollisionMark(name, params);
      auto& geometryCache = geometry_[collisionMark];
      auto geom = geometryCache.find(outline);
      if (geom == geometryCache.end()) {
        auto cubics = geometry::getGlyphCubic(outline->copiedPath);
        auto polygons = buildGlyphCollisionGeometry(cubics, collisionMark);
        // fontSize already includes emScale; multiplying it twice makes the
        // solver's ink disagree with the exported PDF at other font sizes.
        geom = geometryCache.emplace(outline, std::move(polygons)).first;
      }
      x -= positioned.x_advance * xscale;
      auto& g = glyphs.emplace_back();
      g.isMark = marks.contains(name);
      g.collisionAsMark = collisionMark && !g.isMark;
      g.isTopMark = top.contains(name) || dots.contains(name) || waqf.contains(name);
      g.glyphName = name;
      g.preserveGposHorizontal = layout_.preserveMarkGposHorizontal(positioned.codepoint, positioned.parameters);
      if (g.isMark) {
        if (const auto clearance = layout_.markBaselineClearance(positioned.codepoint, positioned.parameters)) g.baselineClearance = *clearance * line.fontSize;
      }
      g.justificationAddedLeftTatweel = positioned.justificationAddedLeftTatweel;
      g.lineIndex = l;
      g.glyphIndex = i;
      g.lineY = y;
      g.baseX = x + positioned.x_offset * xscale;
      g.baseY = y + positioned.y_offset * line.fontSize;
      g.glyphLayout = &positioned;
      g.metrics = {outline->width, outline->height, outline->bbox.llx, outline->bbox.urx};
      g.geomScaled = geom->second.scaled(xscale, line.fontSize);
      if (g.isCollisionMark() && !g.geomScaled.empty()) {
        const geometry::AABB ink{outline->bbox.llx * xscale, outline->bbox.lly * line.fontSize,
                                 outline->bbox.urx * xscale, outline->bbox.ury * line.fontSize};
        g.frameOriginBounds = markFrameOriginBounds(frame, ink, xscale, line.fontSize,
            pageLayout.borderStrokeWidth * scale / 2.0);
      }
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
    optimizePage(result.glyphs, classes, initialOptions, &result.initialViolations, InitialPositioning::Skip);
    if (collisionReport) collectCollisions(page, result, params.collisionReportMinGap, result.initialViolations);
  }
  if (!force) options.maxIters = 0;
  optimizePage(result.glyphs, classes, options, report ? &result.violations : nullptr,
               force ? InitialPositioning::Apply : InitialPositioning::Skip);
  if (force) {
    for (size_t l = 0; l < page.size(); ++l)
      for (size_t i = 0; i < result.glyphs[l].size(); ++i)
        applySolvedGlyphOffsets(page[l].glyphs[i], result.glyphs[l][i],
                                page[l].horizontalScale(), page[l].fontSize);
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
