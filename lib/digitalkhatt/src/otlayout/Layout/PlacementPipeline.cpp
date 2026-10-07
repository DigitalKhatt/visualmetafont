#include "PlacementPipeline.h"

#include <map>
#include <tuple>
#include "GlyphVis.h"
#include "digitalkhatt/layout/GlyphCollisionGeometry.h"

namespace digitalkhatt::layout {

PlacementPipeline::PlacementPipeline(OtLayout& layout, double emScale)
    : layout_(layout), emScale_(emScale) {}

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
  if (report) {
    auto initialOptions = options;
    initialOptions.maxIters = 0;
    optimizePage(result.glyphs, classes, initialOptions, &result.initialViolations);
  }
  if (!force) options.maxIters = 0;
  optimizePage(result.glyphs, classes, options, report ? &result.violations : nullptr);
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
  if (force) {
    for (size_t l = 0; l < page.size(); ++l)
      for (size_t i = 0; i < result.glyphs[l].size(); ++i)
        applySolvedGlyphOffsets(page[l].glyphs[i], result.glyphs[l][i],
            page[l].type == LineType::Line ? page[l].xscale : 1.0);
  }
  return result;
}

}  // namespace digitalkhatt::layout
