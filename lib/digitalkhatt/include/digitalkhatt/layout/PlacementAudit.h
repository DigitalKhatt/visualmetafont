#pragma once

#include <vector>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

struct SolverContext;

// Shared exception policy for anchored marks inside bowls and contextual dots.
// The caller must first validate mark.prevBase.
bool isManuallyPositionedMark(const GlyphInstance& mark, const SolverContext& context);

// Read-only final audit, independent of enabled forces and report-type switches.
// A geometric association warning is evidence for review, not a reassignment.
void collectPlacementViolations(const SolverContext& context,
                                std::vector<ConstraintViolation>& out);

}  // namespace digitalkhatt::layout
