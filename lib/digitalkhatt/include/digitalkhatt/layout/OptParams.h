#pragma once

#include <string>

namespace digitalkhatt::layout {

// Lets a caller (e.g. a tuning UI) disable individual constraint types to
// see their isolated effect on a layout. Final placement diagnostics are
// independent of these force switches.
struct ConstraintToggles {
  bool ylane = true;
  bool waqfPlacement = true;
  bool waqfEscape = true;         // conditional left/down preference for a squeeze
  bool hardStayAboveBelow = false;  // both stay-above/stay-below rails
  bool squeezeCenter = false;
  bool stackOrder = true;
  bool rehWawBehPlacement = false; // opt-in: behshape.init and its dotted-name variants
  bool baseVicinity = true;
  bool horizontalOrder = true;
  bool matchMarkPosition = true;     // kasra + smalllowmeem cohesion
  bool bowlCluster = false;           // center marks trapped inside a bowl base
  bool genericGapConstraint = true;  // broadphase never-overlap pass
  bool downwardEscape = false;      // opt-in: adaptive gap response for squeezed lower marks
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
  bool reportWaqfBounds = false;     // optional solver floor/ceiling residuals
};

// Per-constraint-type XPBD compliance (softness), broken out so each can be
// tuned independently instead of living as a magic number at its call site
// in OptimizeLayout.cpp. Defaults match the values previously hardcoded
// there. StackOrder's two fields apply to every row of topStackTable() --
// no row currently overrides gapCompliance/xAlignCompliance individually,
// so a single shared knob matches current behavior exactly.
struct ConstraintCompliance {
  double extraLineSpacing = 1.0; // extra cross-line spacing above ordinary word clearance
  double waqfMin = 0.3;         // hard-ish lower bound
  double waqfTarget = 5.0;      // legacy key: optional escape descent compliance only
  double waqfMax = 0.3;         // hard-ish upper bound
  double waqfXAlign = 0.24;     // legacy key: optional escape horizontal compliance only

  double hardStayAbove = 1e-7;  // near-zero: rigid rail
  double hardStayBelow = 1e-7;  // near-zero: rigid rail

  double squeezeCenter = 0.3;

  double baseVicinityLeft = 0.1;
  double baseVicinityRight = 0.1;

  double stackOrderGap = 0.001;
  double stackOrderXAlign = 0.2;
  double rehWawDotTarget = 0.2;   // legacy snapshot field; no point target in XPBD
  double rehWawKasraFollow = 0.2; // ordinary lower-stack horizontal alignment
  double rehWawBelowMarkGap = 1.0; // softer spacing to neighboring marks below the tip
  double rehWawBelowBottom = 0.2;  // preferred distance from the reference bottom

  double horizontalOrder = 0.0;

  double matchMarkPosition = 0.2;
  double bowlCluster = 0.3;
  double ylane = 0.1;
};

struct RehWawPlacementParams {
  double tipGap = 30.0;
  double dotKasraGap = 40.0;
  double compactGap = 14.0;    // current-gap acceptance only; never a relocated-mark floor
  double leftGap = 30.0;       // minimum horizontal clearance for a left dot
  double preferredGap = 80.0;  // in-gap comfort target; never a feasibility floor
  double neighborGap = 40.0;
  double belowMarkGap = 10.0; // neighboring-mark clearance for initial below candidates only
  double overhangPercent = 10.0;
  double overhangLimit = 14.0;
  bool oneDotBelowTip = true;  // false: do not force one dot below the tip
  bool twoDotsBelowTip = true; // false: do not force two dots below the tip
};

struct DownwardEscapeParams {
  int repeatedCollisions = 3;       // consecutive stalled, squeezed iterations
  double maxDescentPercent = 100.0; // % of mark height beyond adjusted initial placement
  double maxBelowBasePercent = 125.0; // mark top below owner bottom, % of mark height
};

struct OptParams {
  // Generic target clearances in solver world/font units. Dedicated stack
  // constraints and the 40/10-unit attachment exceptions keep their own gaps.
  double sameWordGap = 80.0;
  double differentWordGap = 80.0;  // different words on the same line
  double differentLineGap = 250.0; // glyphs on different lines

  // Minimum expansion of each ink box when finding potential contact pairs.
  // The search also covers half the largest configured target gap.
  double broadphasePadding = 140.0;  // font units

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
  bool reportGenericGapCollisionsOnly = true; // same check as GUI Save Collision
  double collisionReportMinGap = 10.0; // font units, independent of solver gaps

  // Reporting allowance beyond the BaseVicinity boundary, as a percentage
  // of the mark's horizontal ink width. Does not affect projection or solver
  // convergence. Dots retain strict ownership boundaries by default.
  double baseVicinityMarkTolerancePercent = 5.0;
  double baseVicinityDotTolerancePercent = 0.0;

  // Waqf association diagnostics only; no effect on forces or convergence.
  // Horizontal offsets compare ink left edges, allowing more movement left.
  double waqfLeftDriftTolerancePercent = 100.0;   // % of waqf ink width
  double waqfRightDriftTolerancePercent = 50.0;
  double waqfPreviousLineMarginPercent = 20.0;   // % of actual baseline spacing

  // Minimum waqf ink bottom above its owning baseline, in solver units.
  // Shared by initial placement and the ordinary visibility safety floor.
  double waqfMinHeight = 700.0;

  // Legacy key retained for snapshots: maximum right allowance for initial
  // search/owner bounds, and width of the early fallback's left band.
  // There is no ordinary horizontal-alignment spring.
  double waqfHorizontalAlignmentBandPercent = 25.0; // % of waqf ink width

  double waqfEscapeMinGap = 10.0;       // trigger/stop clearance, font units
  double waqfEscapeStepPercent = 5.0;   // target advance per iteration, % of ink width
  double waqfEscapeDownStepPercent = 5.0; // downward target advance, % of ink height
  double waqfEscapeMaxLeftPercent = 100.0; // cap target offset from owning base

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

  // HorizontalOrderConstraint anti-misassociation thresholds (see
  // HorizontalOrderConstraint.h): fraction of a mark's own ink that must
  // stay exposed (selfKeep), and the fraction of the NEIGHBOR's ink the
  // exposed protrusion must exceed (crossKeep).
  double horizontalOrderSelfKeep = 0.8;
  double horizontalOrderCrossKeep = 0.0;

  ConstraintToggles toggles;
  ConstraintCompliance compliance;
  RehWawPlacementParams rehWaw;
  DownwardEscapeParams downwardEscape;
};

}  // namespace digitalkhatt::layout
