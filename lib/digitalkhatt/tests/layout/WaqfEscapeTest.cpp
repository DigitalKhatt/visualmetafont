#include <cmath>
#include <iostream>
#include <stdexcept>

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/OptParams.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/WaqfMarkPlacement.h"
#include "digitalkhatt/layout/OptimizeLayout.h"
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
    return WaqfPlacementConstraint(page[1][2], 0.3, 0.3, 70.0, 10.0, 5.0, 200.0);
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
    require(waqf.dx < 0.0 && -waqf.dx < width * f.params.waqfHorizontalAlignmentBandPercent / 100.0,
        "Initial squeeze must approach the left band softly, not project to its edge");
    require(waqf.dy < 0.0 && boxBottomY(waqf) >= placement.escapeMinimumBottom(f.context),
        "Initial descent must retain the ordinary owning-stack visibility floor");
    for (int i = 0; i < 30; ++i) {
      escape.contacts.clear();
      escape.recordContact(upper, {0, -1}, -2.0, 80.0);
      escape.project(f.context, 1.0);
    }
    require(escape.targetOffset >= -width * f.params.waqfHorizontalAlignmentBandPercent / 100.0 - 1e-9,
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

void clearedEscapeHoldsItsTargetWithoutOrdinaryHeightSpring() {
  Fixture f;
  f.params.waqfEscapeMinGap = 50;
  auto& upper = f.page[0][0];
  upper.baseX = -10; upper.baseY = 135;
  buildWorldPolys(upper);
  auto placement = f.placement();
  WaqfEscapeConstraint escape(placement, f.params);
  escape.project(f.context, 1);
  require(escape.active, "Fixture must activate fallback");
  const double target = escape.targetBottomOffset;
  upper.baseX = 1000; buildWorldPolys(upper);
  escape.contacts.clear();
  for (int i = 0; i < 10; ++i) escape.project(f.context, 1);
  require(escape.targetBottomOffset == target,
      "Clearing a squeeze must not restart the removed ordinary preferred-height target");
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
    if (mode == "hard") f.params.compliance.waqfXAlign = 0.0;
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
void initialSearchOptimizesClearInkAndLeavesBlockedInkToEscape() {
  for (const std::string mode : {"clear", "blocked", "no_slot", "pinned"}) {
    Fixture f;
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5.0;
    f.params.waqfEscapeMaxLeftPercent = 300.0;
    auto& upper = f.page[0][0];
    auto& waqf = f.page[1][2];
    upper.baseX = mode == "clear" ? 1000.0 : -10.0;
    upper.baseY = 135.0;
    // Exclude the otherwise unused following row from the search region.
    f.page[2][0].baseY = -1000.0;
    buildWorldPolys(f.page[2][0]);
    if (mode == "no_slot")
      upper.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
          {{-100, -100}, {100, -100}, {100, 100}, {-100, 100}}});
    if (mode == "pinned") waqf.mobility = 0.0;
    buildWorldPolys(upper);
    auto placement = f.placement();
    placement.maxLeftPercent = f.params.waqfEscapeMaxLeftPercent;
    prepareWaqfPositions({&placement}, f.context, f.params);
    if (mode == "clear") {
      require(placement.hasInitialSlot && waqf.dy < 0,
          "Clear authored ink must still be searched when its preferred height is lower");
      continue;
    }
    if (mode != "blocked") {
      require(!placement.hasInitialSlot && waqf.dx == 0.0 && waqf.dy == 0.0,
          "Pinned or infeasible initial ink must preserve the authored position");
      if (mode == "no_slot") {
        WaqfEscapeConstraint escape(placement, f.params);
        for (int i = 0; i < 3; ++i) {
          escape.contacts.clear();
          escape.recordContact(upper, {0, -1}, -2.0, 5.0);
          escape.project(f.context, 1.0);
        }
        require(escape.active, "Unavailable initial slots must leave collision escape available");
      }
      continue;
    }
    require(placement.hasInitialSlot && waqf.initialPlacement == GlyphInstance::InitialPlacement::WaqfSlot &&
            waqf.mobility == 1.0, "A searched waqf must remain movable by XPBD");
    require(geometry::getDistance(waqf.worldPolys, upper.worldPolys, 5.0).contact.depth_or_gap >= 5.0 - 1e-6 &&
            geometry::getDistance(waqf.worldPolys, f.page[1][1].worldPolys, 5.0).contact.depth_or_gap >= 5.0 - 1e-6,
        "Initial search must clear both upper ink and the owning stack");
    require(upper.dx == 0.0 && f.page[1][1].dx == 0.0 && f.page[1][1].dy == 0.0,
        "Search must translate only its own waqf");
    const double initialX = waqf.dx;
    waqf.dx -= 5.0; updateWorldPolys(waqf);
    const double shifted = waqf.dx;
    placement.project(f.context, 1.0);
    require(waqf.dx == shifted && shifted < initialX,
        "Safety bounds must not restore horizontal alignment inside the owner interval");
    WaqfTopOrderConstraint(placement).project(f.context, 1.0);
    require(boxTopY(waqf) >= boxTopY(f.page[1][1]),
        "A searched slot must retain the owning stack's top ordering");
    WaqfEscapeConstraint escape(placement, f.params);
    for (int i = 0; i < 3; ++i) {
      escape.contacts.clear();
      escape.recordContact(upper, {0, -1}, -2.0, 5.0);
      escape.project(f.context, 1.0);
    }
    require(escape.active && placement.collisionEscapeActive,
        "A searched slot must still permit fallback escape if later contacts stall");
  }
}

void safetyBoundsDoNotCreatePlacementTargets() {
  for (const std::string mode : {"inside", "low", "high", "left", "right", "pinned"}) {
    Fixture f;
    auto& waqf = f.page[1][2];
    auto placement = f.placement();
    placement.hasInitialSlot = true;
    waqf.dx = -10;
    if (mode == "low") waqf.dy = -100;
    if (mode == "high") waqf.dy = 100;
    if (mode == "left") waqf.dx = -40;
    if (mode == "right") waqf.dx = 40;
    if (mode == "pinned") { waqf.mobility = 0; waqf.dx = 40; waqf.dy = 100; }
    updateWorldPolys(waqf);
    const double x = waqf.dx, y = waqf.dy;
    for (int i = 0; i < 5; ++i) placement.project(f.context, 1);
    if (mode == "inside" || mode == "pinned")
      require(waqf.dx == x && waqf.dy == y,
          "Valid positions and pinned glyphs must receive no height/alignment force");
    if (mode == "low") require(waqf.dy > y && waqf.dx == x, "Visibility floor must remain active");
    if (mode == "high") require(waqf.dy < y && waqf.dx == x, "Upper safety ceiling must remain active");
    if (mode == "left") require(waqf.dx > x && waqf.dy == y, "Left owner bound must remain active");
    if (mode == "right") require(waqf.dx < x && waqf.dy == y, "Right owner bound must remain active");
    require(f.page[1][1].dx == 0 && f.page[1][1].dy == 0,
        "Waqf safety bounds must not move ordinary top marks");
  }
}

void alignedContactSearchDetectsContainmentWithoutCorruption() {
  geometry::GeometrySet a(std::vector<geometry::Poly>{{{0,110},{20,110},{20,130},{0,130}}});
  geometry::GeometrySet b(std::vector<geometry::Poly>{{{0,100},{20,100},{20,120},{0,120}}});
  for (int i = 0; i < 10; ++i) {
    require(geometry::getDistance(a,b,5).contact.intersect &&
            geometry::getDistanceConvexHull(a,b,5).contact.intersect,
        "Aligned overlapping ink must be detected when repeated support distances enclose the origin");
    const auto box=a.boundingAABB();
    require(box.minx==0 && box.miny==110 && box.maxx==20 && box.maxy==130,
        "An aligned distance query must not overwrite its caller's geometry or bounds");
  }
  const auto separated=a.translated(0,15);
  const auto contact=geometry::getDistance(separated,b,5).contact;
  require(!contact.intersect && std::abs(contact.depth_or_gap-5)<1e-9,
      "The same aligned shapes must retain their correct separated clearance");
}

void initialSearchIncludesHeightAndBaseAlignmentPreferences() {
  for (bool withStack : {false, true}) {
    Fixture f;
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5;
    f.params.waqfEscapeMaxLeftPercent = 300;
    f.page[0][0].baseX = 1000;
    f.page[2][0].baseY = -1000;
    auto& waqf = f.page[1][2];
    waqf.baseX = 4; waqf.baseY = 160;
    if (!withStack) f.page[1][1].geomScaled = {};
    for (auto& line : f.page) for (auto& g : line) buildWorldPolys(g);
    auto placement = f.placement();
    placement.maxLeftPercent = 300;
    prepareWaqfPositions({&placement}, f.context, f.params);
    require(placement.hasInitialSlot && std::abs(boxBottomY(waqf) - (withStack ? 125 : 70)) < 1e-9 &&
            std::abs(waqf.worldPolys.boundingAABB().minx) < 1e-9,
        "Initial search must lower clear high anchors and prefer base-left alignment, with owning-stack clearance");
    const double x = waqf.dx, y = waqf.dy;
    for (int i = 0; i < 5; ++i) placement.project(f.context, 1);
    require(waqf.dx == x && waqf.dy == y,
        "Height/alignment preferences must be applied in the search without returning as XPBD springs");
  }
}

void initialSearchAlignsToOverhangingOwningTopMarks() {
  for (const std::string mode : {"overhang", "inside", "multiple", "foreign", "below",
                                  "other_waqf", "empty", "outside_bound", "blocked"}) {
    Fixture f;
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5;
    f.page[0][0].baseX = 1000;
    f.page[2][0].baseY = -1000;
    f.page[1][1].baseX = mode == "inside" ? 4 : mode == "outside_bound" ? -30 : -12;
    f.page[1][2].baseX = 4;
    if (mode == "foreign") f.page[1][1].prevBase = &f.page[0][0];
    if (mode == "below") f.page[1][1].isTopMark = false;
    if (mode == "other_waqf") f.page[1][1].glyphName = "waqf.qaf";
    if (mode == "empty") f.page[1][1].geomScaled = {};
    if (mode == "multiple") {
      f.page[1].push_back(f.page[1][1]);
      f.page[1].push_back(f.page[1][1]);
      for (size_t i = 1; i < f.page[1].size(); ++i) {
        f.page[1][i].prevBase = i == 4 ? &f.page[2][0] : &f.page[1][0];
        f.page[1][i].glyphIndex = static_cast<int>(i);
      }
      f.page[1][3].baseX = -15;
      f.page[1][4].baseX = -18;  // A farther-left mark belonging to another base.
      f.page[1][4].baseY = 50;
    }
    if (mode == "blocked") {
      f.page[0][0].baseX = -12;
      f.page[0][0].baseY = 125;  // Occupies the preferred aligned slot.
    }
    for (auto& line : f.page) for (auto& g : line) buildWorldPolys(g);
    auto placement = f.placement();
    prepareWaqfPositions({&placement}, f.context, f.params);
    auto& waqf = f.page[1][2];
    if (mode == "outside_bound") {
      const auto contact = geometry::getDistance(waqf.worldPolys, f.page[1][1].worldPolys, 5).contact;
      const double left = waqf.worldPolys.boundingAABB().minx;
      require(placement.hasInitialSlot && left >= -20 && left <= 5 &&
              !contact.intersect && contact.depth_or_gap >= 5 - 1e-6,
          "An overhanging mark must not extend the existing horizontal safety interval or override clearance");
      continue;
    }
    const double expected = mode == "multiple" ? -15 :
        mode == "overhang" || mode == "blocked" ? -12 : 0;
    const auto detail = "Initial waqf alignment for " + mode + ": expected " +
        std::to_string(expected) + ", got " + std::to_string(waqf.worldPolys.boundingAABB().minx);
    require(placement.hasInitialSlot &&
            std::abs(waqf.worldPolys.boundingAABB().minx - expected) < 1e-9,
        detail.c_str());
    if (mode == "blocked")
      require(!geometry::getDistance(waqf.worldPolys, f.page[0][0].worldPolys, 5).contact.intersect &&
              geometry::getDistance(waqf.worldPolys, f.page[0][0].worldPolys, 5).contact.depth_or_gap >= 5 - 1e-6,
          "Top-mark alignment must still search for clearance when its preferred slot is occupied");
    if (mode == "overhang" || mode == "multiple") {
      waqf.dx += 3;
      updateWorldPolys(waqf);
      const double x = waqf.dx, y = waqf.dy;
      placement.project(f.context, 1);
      require(waqf.dx == x && waqf.dy == y,
          "Top-mark alignment belongs to initial search, without adding an XPBD alignment spring");
    }
  }
}

void minimumHeightParameterDrivesInitialSearchAndSafetyFloor() {
  for (double minimum : {600.0, 900.0}) {
    Fixture f;
    for (auto& line : f.page) for (auto& g : line) {
      g.baseX *= 10; g.baseY *= 10;
      g.geomScaled = g.geomScaled.scaled(10,10);
    }
    f.page[0][0].baseX = 10000;
    f.page[2][0].baseY = -10000;
    f.page[1][1].geomScaled = {};
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5;
    f.params.waqfMinHeight = minimum;
    f.params.maxIters = 0;
    f.params.toggles.waqfEscape = false;
    optimizePage(f.page, f.classes, f.params);
    auto& waqf = f.page[1][2];
    require(waqf.initialPlacement == GlyphInstance::InitialPlacement::WaqfSlot &&
            std::abs(boxBottomY(waqf)-minimum)<1e-9,
        "The page factory must use the configured minimum in its initial height preference");
    auto placement=f.placement();
    placement.minDistFromBaseline=minimum;
    placement.upperCeilingY=1400;
    placement.hasInitialSlot=true;
    waqf.dy-=100;updateWorldPolys(waqf);
    WaqfTopOrderConstraint(placement).project(f.context,1);
    require(std::abs(boxBottomY(waqf)-minimum)<1e-9,
        "The configured height must remain the final visibility floor after contacts move the waqf");
  }
}

void crowdedInitialSearchKeepsClearInkWithoutRequiringEveryGap() {
  for (bool blocked : {false, true}) {
    Fixture f;
    for (auto& line : f.page) for (auto& g : line) {
      g.baseX *= 10; g.baseY *= 10;
      g.geomScaled = g.geomScaled.scaled(10,10);
    }
    auto& upper = f.page[0][0];
    upper.baseX = -1000;
    upper.baseY = blocked ? 1100 : 1350;
    upper.geomScaled = upper.geomScaled.scaled(10, blocked ? 10 : 1);
    f.page[1][1].baseX = 40;
    f.page[2][0].baseY = -10000;
    for (auto& line : f.page) for (auto& g : line) buildWorldPolys(g);
    f.params.sameWordGap = 80;
    f.params.differentLineGap = 50;
    WaqfPlacementConstraint placement(f.page[1][2], 0.3, 0.3, 700, 100, 50, 1400);
    prepareWaqfPositions({&placement}, f.context, f.params);
    const auto& waqf = f.page[1][2];
    require(placement.hasInitialSlot == !blocked,
        "A crowded ink-clear slot must be searched even when full spacing is impossible; occupied slots remain rejected");
    if (blocked) {
      require(waqf.dx == 0 && waqf.dy == 0,
          "A search with no ink-clear position must preserve input for ordinary XPBD and optional escape");
      continue;
    }
    const auto ownContact = geometry::getDistance(waqf.worldPolys, f.page[1][1].worldPolys, 80).contact;
    const auto upperContact = geometry::getDistance(waqf.worldPolys, upper.worldPolys, 50).contact;
    require(!ownContact.intersect && ownContact.depth_or_gap >= 10 && ownContact.depth_or_gap < 80 &&
            !upperContact.intersect && upperContact.depth_or_gap >= 10,
        "A crowded seed must retain ink clearance and leave only spacing shortfall for XPBD");
    require(waqf.mobility > 0 && f.page[1][1].dx == 0 && f.page[1][1].dy == 0 && upper.dx == 0 && upper.dy == 0,
        "The initial search must move only its waqf and retain refinement mobility");
  }
}

void initialSearchIncludesTheAuthoredColumn() {
  Fixture f;
  f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5;
  f.params.waqfEscapeMaxLeftPercent = 300;
  auto& upper = f.page[0][0];
  upper.baseX = -100; upper.baseY = 135;
  upper.geomScaled = upper.geomScaled.scaled(10, 1);
  f.page[1][1].baseY = 50;
  f.page[2][0].baseY = -1000;
  buildWorldPolys(upper); buildWorldPolys(f.page[1][1]); buildWorldPolys(f.page[2][0]);
  auto placement = f.placement();
  prepareWaqfPositions({&placement}, f.context, f.params);
  const auto& waqf = f.page[1][2];
  require(placement.hasInitialSlot && std::abs(waqf.dx) < 1e-9 && std::abs(waqf.dy + 55) < 1e-9,
      "A clear vertical slot must not acquire horizontal drift from a grid that misses authored X");
}

void initialPlansAreValidatedTogether() {
  for (bool together : {false, true}) {
    Fixture f;
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 5.0;
    f.params.waqfEscapeMaxLeftPercent = 300.0;
    f.page[1].push_back(f.page[1][2]);
    for (size_t i = 1; i < f.page[1].size(); ++i) {
      f.page[1][i].prevBase = &f.page[1][0];
      f.page[1][i].glyphIndex = static_cast<int>(i);
    }
    f.page[0][0].baseX = -10.0; f.page[0][0].baseY = 135.0;
    f.page[2][0].baseY = -1000.0;
    buildWorldPolys(f.page[0][0]); buildWorldPolys(f.page[2][0]);
    auto first = f.placement();
    WaqfPlacementConstraint second(f.page[1][3], 0.3, 0.3, 70.0, 10.0, 5.0, 200.0);
    prepareWaqfPositions(together ? std::vector<WaqfPlacementConstraint*>{&first, &second}
                                 : std::vector<WaqfPlacementConstraint*>{&first}, f.context, f.params);
    require(first.hasInitialSlot == !together,
        "Two independently clear plans selecting the same slot must be rejected together");
    if (together)
      require(!second.hasInitialSlot && f.page[1][2].dx == 0 && f.page[1][3].dx == 0,
          "Conflicting plans must preserve both authored inputs for XPBD fallback");
  }
}

void initialSearchHonorsForceSwitchesAndDiagnosticIndependence() {
  std::pair<double, double> reference;
  for (const std::string mode : {"apply", "diagnostics", "fallback_off", "skip", "placement_off", "gaps_off"}) {
    Fixture f;
    // Use production-sized ink with the factory's ordinary 700/1400 rails.
    for (auto& line : f.page) for (auto& g : line) {
      g.baseX *= 10; g.baseY *= 10;
      g.geomScaled = g.geomScaled.scaled(10, 10);
    }
    f.page[0][0].baseX = -100; f.page[0][0].baseY = 1350;
    f.page[2][0].baseY = -10000;
    f.params.sameWordGap = f.params.differentWordGap = f.params.differentLineGap = 50;
    f.params.waqfEscapeMaxLeftPercent = 300;
    f.params.maxIters = 0;
    f.params.toggles.waqfEscape = mode != "fallback_off";
    f.params.toggles.waqfPlacement = mode != "placement_off";
    f.params.toggles.genericGapConstraint = mode != "gaps_off";
    if (mode == "diagnostics") {
      f.params.waqfLeftDriftTolerancePercent = 0;
      f.params.waqfRightDriftTolerancePercent = 1000;
      f.params.waqfPreviousLineMarginPercent = 100;
      f.params.toggles.reportPlacementAudit = f.params.toggles.reportWaqfBounds = true;
    }
    optimizePage(f.page, f.classes, f.params, nullptr,
        mode == "skip" ? InitialPositioning::Skip : InitialPositioning::Apply);
    const auto& w = f.page[1][2];
    if (mode == "apply") {
      require(w.initialPlacement == GlyphInstance::InitialPlacement::WaqfSlot && w.mobility > 0,
          "The page factory must apply initial waqf plans without freezing mobility");
      reference = {w.dx, w.dy};
    } else if (mode == "diagnostics" || mode == "fallback_off") {
      require(w.dx == reference.first && w.dy == reference.second,
          "Reporting tolerances and disabling escape fallback must not alter initial search results");
    } else {
      require(w.initialPlacement == GlyphInstance::InitialPlacement::None && w.dx == 0 && w.dy == 0,
          "Input inspection and disabled waqf/gap forces must skip initial search");
    }
  }
}

}  // namespace

int main() {
  try {
    initialSearchOptimizesClearInkAndLeavesBlockedInkToEscape();
    initialPlansAreValidatedTogether();
    initialSearchIncludesTheAuthoredColumn();
    initialSearchIncludesHeightAndBaseAlignmentPreferences();
    initialSearchAlignsToOverhangingOwningTopMarks();
    minimumHeightParameterDrivesInitialSearchAndSafetyFloor();
    crowdedInitialSearchKeepsClearInkWithoutRequiringEveryGap();
    alignedContactSearchDetectsContainmentWithoutCorruption();
    safetyBoundsDoNotCreatePlacementTargets();
    initialSearchHonorsForceSwitchesAndDiagnosticIndependence();
    alternatingNormalsActivateEscape();
    sameAndFollowingLineContactsDoNotActivate();
    clearanceResetsPersistence();
    initialPocketStartsSoftLeftEscape();
    clearedEscapeHoldsItsTargetWithoutOrdinaryHeightSpring();
    compressedOwnStackSoftensOnlyAnOpposingBand();
    relaxationRetainsTheExistingLeftAllowance();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
