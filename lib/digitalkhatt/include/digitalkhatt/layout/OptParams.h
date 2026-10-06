#pragma once

#include <string>

namespace digitalkhatt::layout {

// Lets a caller (e.g. a tuning UI) disable individual constraint types to
// see their isolated effect on a layout. Final placement diagnostics are
// independent of these force switches.
struct ConstraintToggles {
  bool ylane = true;
  bool waqfPlacement = true;
  bool hardStayAboveBelow = false;  // both stay-above/stay-below rails
  bool returnToAnchor = false;      // bowl exception
  bool squeezeCenter = false;
  bool stackOrder = true;
  bool baseVicinity = true;
  bool horizontalOrder = true;
  bool matchMarkPosition = true;     // kasra + smalllowmeem cohesion
  bool bowlCluster = false;           // center marks trapped inside a bowl base
  bool genericGapConstraint = true;  // broadphase never-overlap pass
  bool reportViolations = false;     // post-solve diagnostic PDF + log

  // GenericGap has no per-instance constraint objects (it's a free function
  // over broadphase pairs, see GapConstraint.cpp/collectGapViolations), so
  // there's nothing to carry a per-instance XPBDConstraint::reportEnabled the
  // way class-based constraints (e.g. YlaneConstraint) do. This is the
  // equivalent type-level opt-out: independent of genericGapConstraint (which
  // gates whether the constraint runs at all), this gates whether its
  // violations are collected into the post-solve report.
  bool reportGenericGap = false;
  bool reportSoftResiduals = false;  // optional lane/alignment preferences
  bool reportPlacementAudit = false; // final side/class/owner diagnostics
};

// Per-constraint-type XPBD compliance (softness), broken out so each can be
// tuned independently instead of living as a magic number at its call site
// in OptimizeLayout.cpp. Defaults match the values previously hardcoded
// there. StackOrder's two fields apply to every row of topStackTable() --
// no row currently overrides gapCompliance/xAlignCompliance individually,
// so a single shared knob matches current behavior exactly.
struct ConstraintCompliance {
  double waqfMin = 0.3;         // hard-ish lower bound
  double waqfTarget = 0.4;      // softer target height
  double waqfMax = 0.3;         // hard-ish upper bound
  double waqfXAlign = 0.3;

  double hardStayAbove = 1e-7;  // near-zero: rigid rail
  double hardStayBelow = 1e-7;  // near-zero: rigid rail

  double returnToAnchor = 0.2;
  double squeezeCenter = 0.3;

  double baseVicinityLeft = 0.1;
  double baseVicinityRight = 0.1;

  double stackOrderGap = 0.001;
  double stackOrderXAlign = 0.2;

  double horizontalOrder = 0.0;

  double matchMarkPosition = 0.2;
  double bowlCluster = 0.3;
  double ylane = 0.1;
};

struct OptParams {
  double minGapBody = 10.0;  // font units
  double minGapMark = 100.0;

  double maxShiftBodyX = 15.0;
  double maxShiftBodyY = 4.0;
  double maxShiftMark = 20.0;  // for marks: radial limit

  int maxIters = 20;
  double tolCollision = 0.5;  // convergence tolerance for corrections and hard bounds

  // Non-structural violations whose excess beyond allowed slack is below this
  // magnitude (font units) are treated as
  // XPBD's leftover convergence residual after maxIters, not a real defect,
  // and are dropped from the post-solve violation report. Fallback floor for
  // constraint types without a formula-based prediction of their own (see
  // complianceResidualMargin for GenericGap's compliance-aware version).
  double minViolationSeverity = 1.0;

  // Shared GUI/CLI presentation controls; they do not affect XPBD forces.
  // Zero means unlimited. Structural findings are not removed by the cutoff.
  int reportMaxFindings = 1000;
  std::string reportSort = "severity";  // severity or priority
  bool reportOnlyChanged = false;     // keep new/worsened plus structural findings

  // Reporting allowance beyond the BaseVicinity boundary, as a percentage
  // of the mark's horizontal ink width. Does not affect projection or solver
  // convergence. Dots retain strict ownership boundaries by default.
  double baseVicinityMarkTolerancePercent = 5.0;
  double baseVicinityDotTolerancePercent = 0.0;

  // GenericGap violations are compared against expectedComplianceResidual()
  // (GapConstraint.h) -- the residual a soft (compliance > 0) pairwise
  // constraint is mathematically expected to settle at, given its own
  // compliance and the pair's combined mobility. A residual at or below that
  // prediction is the constraint behaving as designed (a spring, not a rigid
  // stop), not a defect, and is dropped from the report. This margin adds
  // slack above the exact prediction to absorb the small deviation from the
  // single-constraint-linear-gradient assumption the formula makes (multiple
  // constraints sharing a glyph, or a curved contact normal near a feature
  // change) -- 1.0 would only forgive violations at or below the theoretical
  // ideal exactly.
  double complianceResidualMargin = 1.25;

  double smoothStrength = 0.120;  // 0..1 small
  double attachStrength = 0.3;  // 0..1 for mark attachment
  double sepOvershoot = 1.05;   // push a bit extra to converge faster

  // HorizontalOrderConstraint anti-misassociation thresholds (see
  // HorizontalOrderConstraint.h): fraction of a mark's own ink that must
  // stay exposed (selfKeep), and the fraction of the NEIGHBOR's ink the
  // exposed protrusion must exceed (crossKeep).
  double horizontalOrderSelfKeep = 0.8;
  double horizontalOrderCrossKeep = 0.0;

  ConstraintToggles toggles;
  ConstraintCompliance compliance;
};

}  // namespace digitalkhatt::layout
