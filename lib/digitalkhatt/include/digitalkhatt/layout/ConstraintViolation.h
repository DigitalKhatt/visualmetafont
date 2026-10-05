#pragma once

#include <array>
#include <string>

#include "digitalkhatt/geometry/geometry.h"

namespace digitalkhatt::layout {

// Which constraint family produced a post-solve violation. An enum (not a
// string): the set is closed and code-owned (the wired constraint types plus
// the free-function gap pass), so an enum gives switch-exhaustiveness for the
// Qt-side label/color tables and no producer/consumer typo drift. A human-readable name is
// provided once by violationTypeName() for the PDF/log.
enum class ViolationType {
  GenericGap,       // solveGapConstraint penetration (broadphase pair)
  HardStayAbove,    // below-rail floor breach (mark bottom below yMin)
  HardStayBelow,    // above-rail ceiling breach (mark top above yMax)
  StackOrderGap,    // stacked-mark min-gap / order breach
  BaseVicinity,     // mark horizontal leash overflow
  SqueezeCenter,    // per-obstacle min-gap breach under a squeezed mark
  Ylane,            // soft harakat band preference
  WaqfPlacement,    // waqf hard lower/upper bound breach or infeasible band
  HorizontalOrder,  // world-ink mark ordering breach
  MarkSide,         // final mark on the wrong side of its base/baseline
  BaseAssociation,  // invalid owner, or new geometric ambiguity with a neighbor
  MarkClassification,
  InvalidPlacement,
  ReturnToAnchor,
  MatchMarkPosition,
  SoftTargetResidual,
};

// Hard = a feasibility or structural safety failure; Soft = a tuning target
// residual or an ownership/classification issue requiring visual review.
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
    case ViolationType::MarkSide: return "MarkSide";
    case ViolationType::BaseAssociation: return "BaseAssociation";
    case ViolationType::MarkClassification: return "MarkClassification";
    case ViolationType::InvalidPlacement: return "InvalidPlacement";
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
  double allowedResidual = 0.0;  // expected compliant slack, when applicable
  bool structural = false;     // classification/owner failures have no length unit
  std::string detail;
  double initialSeverity = -1.0;  // negative: no corresponding shaped-position finding
  bool introduced = false;
  bool worsened = false;

  // Participating glyphs by GlyphInstance::globalIndex (page-local). -1 unused.
  int glyphA = -1;
  int glyphB = -1;

  // Optional markers for the PDF: 0 = none, 1 = a point (marker[0]),
  // 2 = a segment (marker[0]..marker[1]).
  int markerCount = 0;
  std::array<geometry::Vec2, 2> marker{};
};

// Rank safety failures and solver regressions before ordinary clearance noise.
inline int violationReviewPriority(const ConstraintViolation& v) {
  int priority = v.kind == ViolationKind::Hard ? 100 : 0;
  if (v.structural && v.kind == ViolationKind::Hard) priority += 400;
  if (v.type == ViolationType::MarkSide || v.type == ViolationType::InvalidPlacement) priority += 300;
  if (v.type == ViolationType::BaseAssociation) priority += 200;
  if (v.type == ViolationType::GenericGap &&
      (v.detail == "Ink intersection" || v.detail == "Collision-proxy intersection")) priority += 150;
  if (v.introduced || v.worsened) priority += 50;
  return priority;
}
inline bool violationPrecedes(const ConstraintViolation& a, const ConstraintViolation& b) {
  const int pa = violationReviewPriority(a), pb = violationReviewPriority(b);
  return pa != pb ? pa > pb : a.severity > b.severity;
}

}  // namespace digitalkhatt::layout
