#pragma once

#include <vector>

#include "digitalkhatt/layout/ClassMap.h"
#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstance.h"
#include "digitalkhatt/layout/OptParams.h"

namespace digitalkhatt::layout {

enum class InitialPositioning { Apply, Skip };

void initGlyphMobilities(std::vector<std::vector<GlyphInstance>>& pageGlyphs);

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const ClassMap& classes,
                  const OptParams& P);

// Same as above, but when `outViolations` is non-null, collects the hard
// constraint violations remaining after the solve (used by the diagnostic
// report). Passing nullptr is byte-for-byte identical to the 3-arg overload.
// Skip initial positioning for read-only input diagnostics and Force-off runs.
void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const ClassMap& classes,
                  const OptParams& P,
                  std::vector<ConstraintViolation>* outViolations,
                  InitialPositioning initialPositioning = InitialPositioning::Apply);

}  // namespace digitalkhatt::layout
