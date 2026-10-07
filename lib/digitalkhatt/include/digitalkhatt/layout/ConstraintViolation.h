#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <optional>
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
  WaqfPlacement,    // horizontal ownership drift or preceding-line intrusion
  WaqfBoundsResidual, // optional solver floor/ceiling or infeasible-band residual
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
    case ViolationType::WaqfBoundsResidual: return "WaqfBoundsResidual";
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

// Measurements in the same scaled world coordinates as glyph outlines.
struct WaqfPlacementMeasurements {
  int baseIndex = -1;
  double horizontalOffset = 0.0; // left edge relative to own base; negative = left
  double allowedLeftDrift = 0.0;
  double allowedRightDrift = 0.0;
  double heightAboveBaseline = 0.0; // waqf top, not bottom
  std::optional<double> previousBaselineDistance; // positive = below previous line
  std::optional<double> previousLineMargin;
  std::optional<double> previousInkBoxClearance; // AABB distance, not exact ink gap
};

// One reported violation. Geometry (markers) is in worldPolys space so the
// writer can draw it with the same fit transform it uses for the glyph outlines.
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
  std::string diagnostic; // stable reason, independent of values in detail
  std::optional<WaqfPlacementMeasurements> waqf;
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
  if (v.type == ViolationType::WaqfPlacement) priority += 200;
  if (v.type == ViolationType::GenericGap &&
      (v.detail.starts_with("Ink intersection") || v.detail == "Collision-proxy intersection")) priority += 150;
  if (v.introduced || v.worsened) priority += 50;
  return priority;
}
inline bool violationPrecedes(const ConstraintViolation& a, const ConstraintViolation& b) {
  const int pa = violationReviewPriority(a), pb = violationReviewPriority(b);
  return pa != pb ? pa > pb : a.severity > b.severity;
}

// Rank the excess beyond permitted slack, rather than a compliant residual.
// Discrete structural diagnostics have no length unit and form their own group.
inline double violationReportSeverity(const ConstraintViolation& v) {
  if (!std::isfinite(v.severity) || !std::isfinite(v.allowedResidual)) return 0.0;
  // Compliant clearance slack never excuses an intersecting collision proxy.
  if (v.type == ViolationType::GenericGap &&
      (v.detail.starts_with("Collision-proxy") || v.detail.starts_with("Ink intersection")))
    return std::max(0.0, v.severity);
  return std::max(0.0, v.severity - std::max(0.0, v.allowedResidual));
}
inline int violationReportGroup(const ConstraintViolation& v) {
  if (!v.structural) return v.type == ViolationType::WaqfPlacement ? 1 : 2;
  return v.kind == ViolationKind::Hard ? 0 : 3;
}
inline bool violationReportPrecedes(const ConstraintViolation& a, const ConstraintViolation& b,
                                   const std::string& sort) {
  const int ga = violationReportGroup(a), gb = violationReportGroup(b);
  if (ga != gb) return ga < gb;
  if (a.structural || sort == "priority") {
    const int pa = violationReviewPriority(a), pb = violationReviewPriority(b);
    if (pa != pb) return pa > pb;
  }
  return violationReportSeverity(a) > violationReportSeverity(b);
}

}  // namespace digitalkhatt::layout
