#pragma once

#include <vector>

namespace digitalkhatt::layout {
struct SolverContext;
struct OptParams;
struct WaqfPlacementConstraint;

// Search against one unchanged page snapshot, validate the plans together,
// then apply accepted translations before the ordinary XPBD solve.
void prepareWaqfPositions(const std::vector<WaqfPlacementConstraint*>& placements,
                          SolverContext& context, const OptParams& params);
}
