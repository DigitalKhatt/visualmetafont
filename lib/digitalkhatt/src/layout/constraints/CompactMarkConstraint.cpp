#include "digitalkhatt/layout/constraints/CompactMarkConstraint.h"

#include "digitalkhatt/layout/ConstraintDiagnostics.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"

namespace digitalkhatt::layout {

CompactMarkConstraint::CompactMarkConstraint(GlyphInstance& parent_, GlyphInstance& child_, double minGap_, double compliance_)
    : parent(parent_), child(child_), minGap(minGap_), offset{child.baseX - parent.baseX, child.baseY - parent.baseY},
      parentParts(geometry::buildConvexPartsFromPolygons(parent.geom ? *parent.geom : parent.geomScaled)), childParts(geometry::buildConvexPartsFromPolygons(child.geom ? *child.geom : child.geomScaled)) {
  compliance = compliance_;
  reportEnabled = true;
}

geometry::GSContact CompactMarkConstraint::contact() const {
  return geometry::getDistance(parentParts.translated(parent.baseX + parent.dx, parent.baseY + parent.dy),
                               childParts.translated(child.baseX + child.dx, child.baseY + child.dy), minGap);
}

void CompactMarkConstraint::project(SolverContext&, double dt) {
  const double parentMobility = std::max(0.0, parent.mobility);
  const double childMobility = std::max(0.0, child.mobility);
  const double totalMobility = parentMobility + childMobility;
  if (totalMobility <= 0) return;
  const double alpha = compliance / (dt * dt);
  const double denominator = totalMobility + alpha;
  const double dx = child.baseX + child.dx - parent.baseX - parent.dx - offset.x;
  const double dy = child.baseY + child.dy - parent.baseY - parent.dy - offset.y;
  const double correctionX = -(dx + alpha * lambdaX) / denominator;
  const double correctionY = -(dy + alpha * lambdaY) / denominator;
  lambdaX += correctionX;
  lambdaY += correctionY;
  parent.dx -= parentMobility * correctionX;
  parent.dy -= parentMobility * correctionY;
  child.dx += childMobility * correctionX;
  child.dy += childMobility * correctionY;
  updateWorldPolys(parent);
  updateWorldPolys(child);

  // Refine only this direct attachment. A small ink clearance replaces the
  // generic same-word gap, including when the parent's contour is concave.
  const auto result = contact();
  if (std::isfinite(result.contact.depth_or_gap) && result.contact.depth_or_gap < minGap) {
    const auto correction = result.contact.normal * ((minGap - result.contact.depth_or_gap) / totalMobility);
    parent.dx -= parentMobility * correction.x;
    parent.dy -= parentMobility * correction.y;
    child.dx += childMobility * correction.x;
    child.dy += childMobility * correction.y;
    updateWorldPolys(parent);
    updateWorldPolys(child);
  }
}

void CompactMarkConstraint::reportViolations(SolverContext&, std::vector<ConstraintViolation>& out) const {
  const auto result = contact();
  const double residual = result.contact.depth_or_gap - minGap;
  if (!std::isfinite(residual) || residual >= -1e-6) return;
  ConstraintViolation v;
  v.type = ViolationType::GenericGap;
  v.kind = ViolationKind::Hard;
  v.residual = residual;
  v.severity = -residual;
  v.detail = "Compact attachment ink clearance";
  v.glyphA = parent.globalIndex;
  v.glyphB = child.globalIndex;
  v.markerCount = 2;
  v.marker[0] = result.contact.pA;
  v.marker[1] = result.contact.pB;
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
