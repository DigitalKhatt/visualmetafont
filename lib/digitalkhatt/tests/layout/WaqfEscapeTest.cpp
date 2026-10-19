#include <cmath>
#include <iostream>
#include <stdexcept>

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"
#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"

using namespace digitalkhatt::layout;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  ClassMap classes{{"waqfmarks", {"waqf.qaf"}}};
  std::vector<std::vector<GlyphInstance>> page{3};
  OptParams params;
  SolverContext context{page, classes};

  Fixture() {
    page[0].resize(1);
    page[1].resize(3);
    page[2].resize(1);
    int index = 0;
    for (int line = 0; line < 3; ++line) {
      for (size_t glyph = 0; glyph < page[line].size(); ++glyph) {
        auto& g = page[line][glyph];
        g.globalIndex = index++;
        g.lineIndex = line;
        g.glyphIndex = static_cast<int>(glyph);
        g.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
            {{0, 0}, {20, 0}, {20, 20}, {0, 20}}});
        g.glyphName = "base";
        if (line == 1 && glyph > 0) {
          g.isMark = g.isTopMark = true;
          g.mobility = 1.0;
          g.prevBase = &page[1][0];
          g.baseY = glyph == 1 ? 100.0 : 130.0;
          g.glyphName = glyph == 1 ? "damma" : "waqf.qaf";
        }
        buildWorldPolys(g);
      }
    }
  }

  WaqfPlacementConstraint placement() {
    return WaqfPlacementConstraint(page[1][2], 0.3, 5.0, 0.3, 0.24,
                                  70.0, 10.0, 5.0, 0.0, 200.0);
  }
};

void alternatingNormalsActivateEscape() {
  Fixture f;
  auto placement = f.placement();
  WaqfEscapeConstraint escape(placement, f.params);
  const geometry::Vec2 normals[]{{0, -1}, {0, 1}, {-1, 0}};
  for (int iteration = 0; iteration < 3; ++iteration) {
    escape.contacts.clear();
    escape.recordContact(f.page[0][0], normals[iteration], -2.0, 80.0);
    escape.project(f.context, 1.0);
    require(escape.active == (iteration == 2), "Escape needs three stalled upper contacts");
  }
  require(placement.collisionEscapeActive, "Placement must release its overlapping stack floor");
  require(f.page[1][2].dx < 0.0 && f.page[1][2].dy < 0.0,
          "Escape must supply both left and downward corrections");
  WaqfTopOrderConstraint(placement).project(f.context, 1.0);
  require(boxTopY(f.page[1][2]) >= boxTopY(f.page[1][1]),
          "Descent must preserve top-edge ordering");
  require(boxBottomY(f.page[1][2]) >= placement.escapeMinimumBottom(f.context),
          "Descent must preserve the visibility floor");
}

void sameAndFollowingLineContactsDoNotActivate() {
  for (int line : {1, 2}) {
    Fixture f;
    auto placement = f.placement();
    WaqfEscapeConstraint escape(placement, f.params);
    for (int iteration = 0; iteration < 5; ++iteration) {
      escape.contacts.clear();
      escape.recordContact(f.page[line][0], {0, -1}, -2.0, 80.0);
      escape.project(f.context, 1.0);
    }
    require(!escape.active, "Same/following-line contacts must not trigger upper escape");
    require(f.page[1][2].dx == 0.0 && f.page[1][2].dy == 0.0,
            "Inactive escape must not move the mark");
  }
}

void clearanceResetsPersistence() {
  Fixture f;
  auto placement = f.placement();
  WaqfEscapeConstraint escape(placement, f.params);
  for (double gap : {-2.0, -2.0, 20.0, -2.0, -2.0}) {
    escape.contacts.clear();
    escape.recordContact(f.page[0][0], {0, 1}, gap, 80.0);
    escape.project(f.context, 1.0);
    require(!escape.active, "A cleared contact must interrupt the three-iteration trigger");
  }
}

void initialPocketStartsSoftLeftEscape() {
  for (const std::string mode : {"squeeze", "clear_stack", "clear_upper", "outline_clear", "pinned"}) {
    Fixture f;
    f.params.waqfEscapeMinGap = 50.0;
    auto& upper = f.page[0][0];
    auto& stack = f.page[1][1];
    auto& waqf = f.page[1][2];
    upper.baseX = -10.0;
    upper.baseY = mode == "clear_upper" ? 200.0 : 135.0;
    if (mode == "clear_stack") stack.baseY = 50.0;
    if (mode == "outline_clear") {
      upper.baseX = upper.baseY = 0.0;
      upper.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
          {{12, 180}, {40, 145}, {40, 180}}});
    }
    if (mode == "pinned") waqf.mobility = 0.0;
    buildWorldPolys(upper); buildWorldPolys(stack);
    auto placement = f.placement();
    WaqfEscapeConstraint escape(placement, f.params);
    escape.project(f.context, 1.0);
    require(escape.active == (mode == "squeeze"),
        "Only initial upper-ink penetration beside a tight owning stack may start early escape");
    if (mode != "squeeze") {
      require(waqf.dx == 0.0 && waqf.dy == 0.0,
          "Clear outlines, roomy stacks and zero mobility must leave initial escape inactive");
      continue;
    }
    const auto bounds = waqf.worldPolys.boundingAABB();
    const double width = bounds.maxx - bounds.minx;
    require(waqf.dx < 0.0 && -waqf.dx < width * placement.xAlignmentBandPercent / 100.0,
        "Initial squeeze must approach the left band softly, not project to its edge");
    require(waqf.dy < 0.0 && boxBottomY(waqf) >= placement.escapeMinimumBottom(f.context),
        "Initial descent must retain the ordinary owning-stack visibility floor");
    for (int i = 0; i < 30; ++i) {
      escape.contacts.clear();
      escape.recordContact(upper, {0, -1}, -2.0, 80.0);
      escape.project(f.context, 1.0);
    }
    require(escape.targetOffset >= -width * placement.xAlignmentBandPercent / 100.0 - 1e-9,
        "Initial left preference must stop at the ordinary band rather than spend the full drift allowance");
    require(upper.dx == 0.0 && upper.dy == 0.0 && stack.dx == 0.0 && stack.dy == 0.0,
        "Early escape must move only its own waqf");
    waqf.dx -= 100.0;
    updateWorldPolys(waqf);
    const double overshoot = waqf.dx;
    escape.contacts.clear();
    escape.project(f.context, 1.0);
    require(waqf.dx > overshoot,
        "The early soft band must resist a contact-driven overshoot beyond the owning base");
    f.context.waqfEscapes.resize(5, nullptr);
    f.context.waqfEscapes[waqf.globalIndex] = &escape;
    waqf.dx = waqf.dy = 0.0;
    updateWorldPolys(waqf);
    double penetration = 0.0;
    solveGapConstraint(f.context, waqf, stack, f.params, 1.0, penetration);
    require(stack.dy == 0.0 && stack.mobility == 1.0 && waqf.dy > 0.0,
        "Waqf contact must not press its top stack downward or change global stack mobility");
    solveGapConstraint(f.context, upper, stack, f.params, 1.0, penetration);
    require(stack.dy != 0.0,
        "The top stack must remain vertically mobile under unrelated ordinary contacts");
  }
}

void clearedInitialPocketSettlesToOrdinaryHeight() {
  for (bool allowDescent : {false, true}) {
    Fixture f;
    f.params.waqfEscapeMinGap = 50.0;
    if (!allowDescent) f.params.waqfEscapeDownStepPercent = 0.0;
    auto& upper = f.page[0][0];
    auto& waqf = f.page[1][2];
    upper.baseX = -10.0; upper.baseY = 135.0;
    buildWorldPolys(upper);
    auto placement = f.placement();
    WaqfEscapeConstraint escape(placement, f.params);
    escape.project(f.context, 1.0);
    require(escape.active && escape.startedFromInitialSqueeze,
        "Fixture must enter the early escape before testing settlement");
    const double previousDy = waqf.dy;
    upper.baseX = 1000.0;
    buildWorldPolys(upper);
    escape.contacts.clear();
    const double previousTarget = escape.targetBottomOffset;
    escape.project(f.context, 1.0);
    if (allowDescent) {
      require(waqf.dy < previousDy &&
              escape.targetBottomOffset < previousTarget &&
              previousTarget - escape.targetBottomOffset <= boxHeight(waqf) * f.params.waqfEscapeDownStepPercent / 100.0 + 1e-9,
          "Clearing the upper ink must advance the soft target toward ordinary height by the configured down step");
      require(boxBottomY(waqf) > waqf.lineY + escape.targetBottomOffset &&
              boxBottomY(waqf) >= placement.escapeMinimumBottom(f.context),
          "Settlement must use a compliant target and retain the visibility floor, not project to the target");
      for (int i = 0; i < 40; ++i) escape.project(f.context, 1.0);
      require(std::abs(escape.targetBottomOffset - placement.preferredBottom(f.context) + waqf.lineY) < 1e-9,
          "Settlement must stop advancing at the ordinary preferred height");
      const double settledDy = waqf.dy;
      for (int i = 0; i < 10; ++i) escape.project(f.context, 1.0);
      require(std::abs(waqf.dy - settledDy) < 1e-9,
          "A settled target must retain its multiplier rather than hardening through repeated resets");
    } else {
      require(waqf.dy == previousDy,
          "Zero down step must also disable settling after an early squeeze clears");
    }
    require(f.page[1][1].dx == 0.0 && f.page[1][1].dy == 0.0,
        "Settling the waqf must not directly reposition its owning top stack");
  }
}

void compressedOwnStackSoftensOnlyAnOpposingBand() {
  for (const std::string mode : {"opposing", "aiding", "clear", "neighbor",
       "bottom", "nonmark", "waqf", "other_line", "stale", "hard", "pinned"}) {
    Fixture f;
    f.params.waqfEscapeDownStepPercent = 0.0;
    auto& stack = f.page[1][1];
    auto& waqf = f.page[1][2];
    stack.baseY = waqf.baseY;
    waqf.dx = 40.0;
    if (mode == "aiding") stack.baseX = 100.0;
    if (mode == "clear") stack.baseX = -100.0;
    if (mode == "neighbor") stack.prevBase = nullptr;
    if (mode == "bottom") stack.isTopMark = false;
    if (mode == "nonmark") stack.isMark = false;
    if (mode == "waqf") stack.glyphName = "waqf.qaf";
    if (mode == "other_line") stack.lineIndex = 0;
    if (mode == "pinned") waqf.mobility = 0.0;
    buildWorldPolys(stack); updateWorldPolys(waqf);
    auto placement = f.placement();
    if (mode == "hard") placement.xAlignCompliance = 0.0;
    WaqfEscapeConstraint escape(placement, f.params);
    escape.active = escape.startedFromInitialSqueeze = escape.initialContactsChecked = true;
    escape.targetOffset = 5.0;
    escape.leftBandMinimum = -15.0;
    escape.targetBottomOffset = waqf.baseY;
    // Deliberately stale samples: eligibility must use current outline geometry.
    escape.recordContact(stack, {-1, 0}, -500.0, 80.0);
    if (mode == "stale") {
      stack.baseX = -100.0;
      buildWorldPolys(stack);
    }
    escape.project(f.context, 1.0);
    const bool softened = mode == "opposing";
    const double alpha = mode == "hard" ? 0.0 : 0.24 * (softened ? 1.0 + 9.0 * 0.75 : 1.0);
    const double expected = mode == "pinned" ? 40.0 : 40.0 - 35.0 / (1.0 + alpha);
    require(std::abs(waqf.dx - expected) < 1e-9,
        "Only a compressed ordinary owning-stack contact opposing the band may soften it");
    require(escape.bandRelaxed == softened,
        "Relaxation must leave hard compliance, fixed marks and unrelated contacts unchanged");
    require(stack.dx == 0.0 && stack.dy == 0.0 && stack.mobility == 1.0,
        "Conditional compliance must not directly move or pin the owning stack");
    if (softened) {
      const double previous = waqf.dx;
      escape.contacts.clear();
      escape.project(f.context, 1.0);
      require(!escape.bandRelaxed && std::abs(waqf.dx - (previous - (previous - 5.0) / 1.24)) < 1e-9,
          "Clearing contact must release the relaxed multiplier and restore ordinary compliance");
      const double settled = waqf.dx;
      escape.project(f.context, 1.0);
      require(std::abs(waqf.dx - settled) < 1e-9,
          "An unchanged ordinary spring must retain its multiplier, not harden each iteration");
    }
  }
}

void relaxationRetainsTheExistingLeftAllowance() {
  for (const double offset : {-17.5, -30.0}) {
    Fixture f;
    f.params.waqfEscapeDownStepPercent = 0.0;
    f.params.waqfEscapeMaxLeftPercent = 100.0;
    auto& stack = f.page[1][1];
    auto& waqf = f.page[1][2];
    stack.baseY = waqf.baseY;
    stack.baseX = offset + 40.0;
    waqf.dx = offset;
    buildWorldPolys(stack); updateWorldPolys(waqf);
    auto placement = f.placement();
    WaqfEscapeConstraint escape(placement, f.params);
    escape.active = escape.startedFromInitialSqueeze = escape.initialContactsChecked = true;
    escape.targetOffset = -5.0;
    escape.leftBandMinimum = -15.0;
    escape.targetBottomOffset = waqf.baseY;
    escape.recordContact(stack, {-1, 0}, 20.0, 80.0);
    escape.project(f.context, 1.0);
    // Waqf width 20, preferred far edge -15, existing allowance -20.
    // Half the reserve remains at -17.5; none remains beyond -20.
    const double reserveFactor = offset == -17.5 ? 0.5 : 0.0;
    const double alpha = 0.24 * (1.0 + 9.0 * 0.75 * reserveFactor);
    require(std::abs(waqf.dx - (offset - (offset + 15.0) / (1.0 + alpha))) < 1e-9,
        "Relaxation must taper to ordinary stiffness at the unchanged outer left allowance");
    require(waqf.dx < -15.0 && stack.dx == 0.0 && stack.dy == 0.0,
        "The association response must remain soft rather than project to a final position");
  }
}
}  // namespace

int main() {
  try {
    alternatingNormalsActivateEscape();
    sameAndFollowingLineContactsDoNotActivate();
    clearanceResetsPersistence();
    initialPocketStartsSoftLeftEscape();
    clearedInitialPocketSettlesToOrdinaryHeight();
    compressedOwnStackSoftensOnlyAnOpposingBand();
    relaxationRetainsTheExistingLeftAllowance();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
