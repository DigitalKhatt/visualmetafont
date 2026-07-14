#pragma once

#include <array>

#include "digitalkhatt/geometry/geometry.h"

namespace digitalkhatt::layout {

// Which constraint family produced a post-solve violation. An enum (not a
// string): the set is closed and code-owned (the wired constraint types plus
// the free-function gap pass), so an enum gives switch-exhaustiveness for the
// Qt-side label/color tables, no per-row allocation while collecting in the
// solver's tail, and no producer/consumer typo drift. A human-readable name is
// provided once by violationTypeName() for the PDF/log.
enum class ViolationType {
  GenericGap,       // solveGapConstraint penetration (broadphase pair)
  HardStayAbove,    // below-rail floor breach (mark bottom below yMin)
  HardStayBelow,    // above-rail ceiling breach (mark top above yMax)
  StackOrderGap,    // stacked-mark min-gap / order breach
  BaseVicinity,     // mark horizontal leash overflow
  SqueezeCenter,    // per-obstacle min-gap breach under a squeezed mark
  Ylane,            // harakat band floor breach (mark below the band). Its
                    // reportViolations() is implemented and emits Hard, but is
                    // currently excluded from the report via
                    // XPBDConstraint::reportEnabled (see YlaneConstraint's
                    // constructor) -- the band is considered a soft preference
                    // for now, not a hard bound.
  WaqfPlacement,    // waqf hard lower/upper bound breach or infeasible band
  HorizontalOrder,  // design-metric mark ordering breach
  // ---- soft residual types (phase 2; declared now, not emitted yet) ----
  ReturnToAnchor,
  MatchMarkPosition,
  SoftTargetResidual,
};

// Hard = a feasibility bound was breached; Soft = a soft target's residual
// (emitted in a later phase). The struct carries this now so soft reporting is
// purely additive later.
enum class ViolationKind { Hard, Soft };

inline const char* violationTypeName(ViolationType t) {
  switch (t) {
    case ViolationType::GenericGap: return "GenericGap";
    case ViolationType::HardStayAbove: return "HardStayAbove";
    case ViolationType::HardStayBelow: return "HardStayBelow";
    case ViolationType::StackOrderGap: return "StackOrderGap";
    case ViolationType::BaseVicinity: return "BaseVicinity";
    case ViolationType::SqueezeCenter: return "SqueezeCenter";
    case ViolationType::Ylane: return "Ylane";
    case ViolationType::WaqfPlacement: return "WaqfPlacement";
    case ViolationType::HorizontalOrder: return "HorizontalOrder";
    case ViolationType::ReturnToAnchor: return "ReturnToAnchor";
    case ViolationType::MatchMarkPosition: return "MatchMarkPosition";
    case ViolationType::SoftTargetResidual: return "SoftTargetResidual";
  }
  return "Unknown";
}

// One reported violation. Geometry (markers) is in worldPolys space so the
// Qt-side writer can draw it with the same fit transform it uses for the
// glyph outlines.
struct ConstraintViolation {
  ViolationType type = ViolationType::GenericGap;
  ViolationKind kind = ViolationKind::Hard;

  // Signed residual C in the same convention the producing project() uses (the
  // violation sign differs per family). `severity` normalizes it to a positive
  // magnitude (font units) for ranking / printing.
  double residual = 0.0;
  double severity = 0.0;

  // Participating glyphs by GlyphInstance::globalIndex (page-local). -1 unused.
  int glyphA = -1;
  int glyphB = -1;

  // Optional markers for the PDF: 0 = none, 1 = a point (marker[0]),
  // 2 = a segment (marker[0]..marker[1]).
  int markerCount = 0;
  std::array<geometry::Vec2, 2> marker{};
};

}  // namespace digitalkhatt::layout
