#include "digitalkhatt/layout/constraints/BowlClusterConstraint.h"

#include <algorithm>
#include <limits>

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/SolverContext.h"

namespace digitalkhatt::layout {

void BowlClusterConstraint::project(SolverContext& solverContext, double dt) {
  if (marks.empty()) return;

  // Combined horizontal extent of the cluster (union of member world bboxes)
  // and how many members can actually move.
  double groupMinX = std::numeric_limits<double>::infinity();
  double groupMaxX = -std::numeric_limits<double>::infinity();
  int mobileCount = 0;
  for (const GlyphInstance* m : marks) {
    const auto mb = m->worldPolys.boundingAABB();
    groupMinX = std::min(groupMinX, mb.minx);
    groupMaxX = std::max(groupMaxX, mb.maxx);
    if (m->mobility > 0.0) ++mobileCount;
  }
  if (mobileCount == 0) return;

  const double midX = 0.5 * (groupMinX + groupMaxX);
  const double bowlCenterX = boxCenterX(base);

  // Soft equality on the cluster's single horizontal translation DOF:
  //   C = midX - bowlCenterX = 0
  // Treat the cluster as one rigid body (unit effective mass): the same delta
  // is applied to every mobile member, so their anchor-relative arrangement is
  // preserved -- only the block as a whole slides toward the bowl center.
  const double C = midX - bowlCenterX;

  const double alpha = compliance / (dt * dt);
  const double denom = 1.0 + alpha;
  if (denom < 1e-12) return;

  const double deltaLambda = (-C - alpha * lambda) / denom;
  lambda += deltaLambda;

  for (GlyphInstance* m : marks) {
    if (m->mobility <= 0.0) continue;
    m->dx += deltaLambda;
    buildWorldPolys(*m);
  }
}

}  // namespace digitalkhatt::layout
