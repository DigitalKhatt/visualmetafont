#include "digitalkhatt/layout/PlacementAudit.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

bool isManuallyPositionedMark(const GlyphInstance& mark, const SolverContext& context) {
  if (!mark.prevBase) return false;
  const bool below = context.lowmarks.contains(mark.glyphName) ||
                     context.downdotmarks.contains(mark.glyphName);
  const auto& base = *mark.prevBase;
  const auto precedingBaseExists = [&] {
    return base.prevBase && std::any_of(context.pageGlyphs.begin(), context.pageGlyphs.end(),
        [&](const auto& line) { return std::any_of(line.begin(), line.end(),
            [&](const auto& g) { return &g == base.prevBase; }); });
  };
  return (below && context.isBowlBase(base)) ||
      (context.downdotmarks.contains(mark.glyphName) &&
       base.glyphName.starts_with("behshape.init") &&
       base.glyphName != "behshape.init.beforenoon" && precedingBaseExists() &&
       (base.prevBase->glyphName.starts_with("reh") || base.prevBase->glyphName.starts_with("waw")) &&
       boxBottomY(mark) > boxBottomY(*base.prevBase));
}

namespace {
double horizontalDistance(double x, const geometry::AABB& box) {
  return std::max({box.minx - x, 0.0, x - box.maxx});
}

ConstraintViolation diagnostic(const GlyphInstance& mark, ViolationType type,
                               const char* detail, bool structural = false) {
  ConstraintViolation v;
  v.type = type;
  v.glyphA = mark.globalIndex;
  v.structural = structural;
  v.detail = detail;
  if (!mark.worldPolys.empty() && std::isfinite(boxCenterX(mark)) && std::isfinite(boxCenterY(mark))) {
    v.markerCount = 1;
    v.marker[0] = {boxCenterX(mark), boxCenterY(mark)};
  }
  return v;
}
}  // namespace

void collectPlacementViolations(const SolverContext& context,
                                std::vector<ConstraintViolation>& out) {
  std::unordered_set<const GlyphInstance*> pageMembers;
  for (const auto& line : context.pageGlyphs)
    for (const auto& g : line) pageMembers.insert(&g);

  for (const auto& line : context.pageGlyphs) {
    std::vector<const GlyphInstance*> bases;
    for (const auto& g : line)
      if (!g.isMark && g.glyphName != "space" && !g.worldPolys.empty()) bases.push_back(&g);

    const GlyphInstance* expectedBase = nullptr;
    for (const auto& mark : line) {
      const bool above = context.topmarks.contains(mark.glyphName) ||
                         context.topdotmarks.contains(mark.glyphName) || context.isWaqf(mark);
      const bool below = context.lowmarks.contains(mark.glyphName) ||
                         context.downdotmarks.contains(mark.glyphName);
      if ((above && below) || ((above || below) && !mark.isMark) ||
          (mark.isMark != context.marks.contains(mark.glyphName)) ||
          (mark.isMark && above != below && mark.isTopMark != above)) {
        out.push_back(diagnostic(mark, ViolationType::MarkClassification,
            "Inconsistent mark membership or conflicting top/bottom classes", true));
      } else if (mark.isMark && !above && !below) {
        auto v = diagnostic(mark, ViolationType::MarkClassification,
            "Mark has no top/bottom role; side safety cannot be checked", true);
        v.kind = ViolationKind::Soft;
        out.push_back(v);
      }

      if (!std::isfinite(mark.baseX) || !std::isfinite(mark.baseY) ||
          !std::isfinite(mark.dx) || !std::isfinite(mark.dy)) {
        out.push_back(diagnostic(mark, ViolationType::InvalidPlacement,
            "Non-finite placement coordinates", true));
        continue;
      }
      if (!mark.isMark) {
        expectedBase = mark.glyphName == "space" ? nullptr : &mark;
        continue;
      }

      const auto* base = mark.prevBase;
      if (!base || !pageMembers.contains(base) || base->isMark ||
          base->lineIndex != mark.lineIndex || base->glyphName == "space" || base->worldPolys.empty()) {
        out.push_back(diagnostic(mark, ViolationType::BaseAssociation,
            "Mark has no valid owning base on this line", true));
        continue;
      }
      if (base != expectedBase) {
        auto v = diagnostic(mark, ViolationType::BaseAssociation,
            "Assigned base disagrees with the preceding base in the shaped glyph run", true);
        v.glyphB = base->globalIndex;
        out.push_back(v);
      }
      if (mark.worldPolys.empty()) continue;
      const auto mb = mark.worldPolys.boundingAABB();
      const auto bb = base->worldPolys.boundingAABB();
      const double cx = boxCenterX(mark), cy = boxCenterY(mark);

      // Check semantic side on the ink center, with the looser base/baseline
      // reference. This is deliberately less restrictive than the optional
      // whole-ink rails; existing contextual anchors can lie inside base ink.
      if (above != below && !isManuallyPositionedMark(mark, context)) {
        const double rail = above ? std::min(bb.maxy, base->lineY)
                                  : std::max(bb.miny, base->lineY);
        const double residual = above ? rail - cy : cy - rail;
        if (residual > 0.0) {
          auto v = diagnostic(mark, ViolationType::MarkSide,
              above ? "Top mark center is below its base/baseline reference"
                    : "Bottom mark center is above its base/baseline reference");
          v.residual = v.severity = residual;
          v.glyphB = base->globalIndex;
          v.markerCount = 2;
          v.marker[0] = {mb.minx, rail};
          v.marker[1] = {mb.maxx, rail};
          out.push_back(v);
        }
      }

      // Only flag NEW horizontal ambiguity: an original anchor already near a
      // neighbor is not evidence of solver-induced misassociation. Require the
      // final center to lie over the neighbor's ink and outside its owner's.
      const auto ownIt = std::find(bases.begin(), bases.end(), base);
      if (ownIt == bases.end()) continue;
      const double initialX = cx - mark.dx;
      const double ownDistance = horizontalDistance(cx, bb);
      const double initialOwnDistance = horizontalDistance(initialX, bb);
      for (int direction : {-1, 1}) {
        const auto index = std::distance(bases.begin(), ownIt) + direction;
        if (index < 0 || index >= static_cast<std::ptrdiff_t>(bases.size())) continue;
        const auto* neighbor = bases[index];
        const auto nb = neighbor->worldPolys.boundingAABB();
        const double neighborDistance = horizontalDistance(cx, nb);
        if (ownDistance > 0.0 && neighborDistance == 0.0 &&
            initialOwnDistance <= horizontalDistance(initialX, nb)) {
          auto v = diagnostic(mark, ViolationType::BaseAssociation,
              "Solver moved the mark over a neighboring base; review visual ownership");
          v.kind = ViolationKind::Soft;
          v.residual = v.severity = ownDistance;
          v.glyphB = neighbor->globalIndex;
          v.markerCount = 2;
          v.marker[0] = {cx, cy};
          v.marker[1] = {boxCenterX(*neighbor), boxCenterY(*neighbor)};
          out.push_back(v);
        }
      }
    }
  }
}

}  // namespace digitalkhatt::layout
