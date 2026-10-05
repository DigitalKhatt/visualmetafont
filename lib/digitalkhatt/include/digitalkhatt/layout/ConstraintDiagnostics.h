#pragma once

#include <cmath>

#include "digitalkhatt/layout/ConstraintViolation.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"

namespace digitalkhatt::layout {

inline void reportSoftTarget(std::vector<ConstraintViolation>& out, ViolationType type,
    const GlyphInstance& a, const GlyphInstance* b, double residual, const char* detail) {
  if (std::abs(residual) <= 1e-9) return;
  ConstraintViolation v;
  v.type = type;
  v.kind = ViolationKind::Soft;
  v.residual = residual;
  v.severity = std::abs(residual);
  v.detail = detail;
  v.glyphA = a.globalIndex;
  v.glyphB = b ? b->globalIndex : -1;
  v.markerCount = b ? 2 : 1;
  v.marker[0] = {boxCenterX(a), boxCenterY(a)};
  if (b) v.marker[1] = {boxCenterX(*b), boxCenterY(*b)};
  out.push_back(v);
}

}  // namespace digitalkhatt::layout
