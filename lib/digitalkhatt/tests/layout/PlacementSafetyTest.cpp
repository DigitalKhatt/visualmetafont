#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/BelowMarkPlacement.h"
#include "digitalkhatt/layout/GlyphCollisionGeometry.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptimizeLayout.h"
#include "digitalkhatt/layout/PlacementAudit.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/ViolationReportContext.h"
#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"
#include "digitalkhatt/layout/constraints/HorizontalGapConstraint.h"
#include "digitalkhatt/layout/constraints/YlaneConstraint.h"
#include "digitalkhatt/layout/RehWawMarkPlacement.h"
#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"

using namespace digitalkhatt::layout;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void intentionalMarkHullPolicy() {
  const geometry::Poly lShape{{0,0},{100,0},{100,20},{20,20},{20,100},{0,100}};
  geometry::ContourCubic contour;
  for (size_t i=0;i<lShape.size();++i) {
    const auto a=lShape[i], b=lShape[(i+1)%lShape.size()];
    contour.segs.push_back({a,a,b,b});
  }
  const geometry::GlyphCubic outline{{contour}};
  const auto mark=buildGlyphCollisionGeometry(outline,true);
  const auto base=buildGlyphCollisionGeometry(outline,false);
  require(mark.size()==1, "Marks must retain an unsplit component hull proxy");
  require(base.size()>1, "Concave bases must retain decomposed collision parts");
  const geometry::GeometrySet inCavity(std::vector<geometry::Poly>{
      {{30,30},{40,30},{40,40},{30,40}}});
  require(geometry::getDistance(mark,inCavity,80).contact.intersect,
          "Mark support mapping must collide with space inside its component hull");
  require(!geometry::getDistance(base,inCavity,80).contact.intersect,
          "Decomposed base parts must preserve their concave free space");
}

struct Fixture {
  ClassMap classes{{"marks", {"fatha", "shadda", "onedotup", "kasra", "smalllowmeem", "waqf.meem"}},
      {"topmarks", {"fatha", "shadda"}}, {"topdotmarks", {"onedotup"}},
      {"lowmarks", {"kasra", "smalllowmeem"}}, {"waqfmarks", {"waqf.meem"}},
      {"bowlbases", {"hah.isol"}}};
  std::vector<std::vector<GlyphInstance>> page{1};

  Fixture() { page[0].reserve(16); }
  GlyphInstance& add(const std::string& name, double x, double y, double width = 20, double height = 40) {
    auto& line = page[0];
    auto& g = line.emplace_back();
    g.glyphName = name;
    g.glyphIndex = g.globalIndex = static_cast<int>(line.size()) - 1;
    g.isMark = classes["marks"].contains(name);
    g.isTopMark = classes["topmarks"].contains(name) || classes["topdotmarks"].contains(name) || classes["waqfmarks"].contains(name);
    g.mobility = g.isMark ? 1.0 : 0.0;
    g.baseX = x;
    g.baseY = y;
    if (name != "space") g.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
        {{0, 0}, {width, 0}, {width, height}, {0, height}}});
    // Deliberately inconsistent design metrics: safety must use world ink.
    g.metrics = {900, 900, -900, 900};
    for (int i = g.glyphIndex - 1; i >= 0; --i) {
      if (!line[i].isMark) { g.prevBase = &line[i]; break; }
    }
    if (!g.isMark && g.prevBase) g.prevBase->nextBase = &g;
    buildWorldPolys(g);
    return g;
  }
};

// Apply an isolated planner result for geometry tests. Page integration
// validates all plans together and pins accepted current-gap dots separately.
void applyPreferred(RehWawMarkPlacement& placement) {
  const auto shape = placement.preferredShape();
  if (shape.empty()) return;
  auto& mark = placement.mark;
  const auto box = shape.boundingAABB();
  mark.dx += 0.5 * (box.minx + box.maxx) - boxCenterX(mark);
  mark.dy += box.maxy - boxTopY(mark);
  updateWorldPolys(mark);
}

OptParams quiet() {
  OptParams p;
  p.maxIters = 0;
  auto& t = p.toggles;
  t.ylane = t.waqfPlacement = t.hardStayAboveBelow = t.stackOrder = false;
  t.baseVicinity = t.horizontalOrder = t.matchMarkPosition = t.genericGapConstraint = false;
  t.reportGenericGap = false;
  t.reportPlacementAudit = true;  // These safety tests explicitly inspect audit findings.
  return p;
}

bool contains(const std::vector<ConstraintViolation>& v, ViolationType type) {
  return std::any_of(v.begin(), v.end(), [&](const auto& entry) { return entry.type == type; });
}

void independentSideAndOwnerChecks() {
  Fixture f;
  f.add("base", 0, -50, 100, 100);
  auto& top = f.add("fatha", 20, 20, 20, 20);
  top.dy = -60;
  auto& bottom = f.add("kasra", 30, -30, 20, 20);
  bottom.dy = 60;
  std::vector<ConstraintViolation> v;
  optimizePage(f.page, f.classes, quiet(), &v);
  require(std::count_if(v.begin(), v.end(), [](const auto& e) { return e.type == ViolationType::MarkSide; }) == 2,
      "Side flips must be reported even with all forces and generic reports off");

  Fixture leading;
  leading.add("fatha", 0, 100);
  auto p = quiet();
  p.maxIters = 20;
  p.minViolationSeverity = 1000;
  p.toggles.horizontalOrder = true;
  v.clear();
  optimizePage(leading.page, leading.classes, p, &v);
  require(contains(v, ViolationType::BaseAssociation), "Leading marks must report missing owners without crashing or cutoff suppression");

  Fixture bowl;
  bowl.add("hah.isol", 0, -40, 100, 80);
  bowl.add("kasra", 30, 10, 20, 20);
  v.clear();
  optimizePage(bowl.page, bowl.classes, quiet(), &v);
  require(!contains(v, ViolationType::MarkSide), "Intentional bowl marks must retain their contextual placement");

  Fixture classification;
  classification.add("base", 0, -40, 100, 80);
  classification.add("fatha", 30, 100).isTopMark = false;
  v.clear();
  optimizePage(classification.page, classification.classes, quiet(), &v);
  require(contains(v, ViolationType::MarkClassification), "An inconsistent top-mark flag must be reported");
}

void ownershipAndReportContext() {
  Fixture f;
  f.add("base.right", 200, -40, 100, 80);
  auto& mark = f.add("fatha", 230, 600, 20, 20);
  f.add("space", 150, 0);
  f.add("base.left", 0, -40, 100, 80);
  f.add("kasra", 40, -40, 20, 20);
  mark.dx = -200;
  std::vector<ConstraintViolation> v;
  optimizePage(f.page, f.classes, quiet(), &v);
  const auto warning = std::find_if(v.begin(), v.end(), [](const auto& e) { return e.type == ViolationType::BaseAssociation && !e.structural; });
  require(warning != v.end() && warning->glyphB == 3 && warning->kind == ViolationKind::Soft,
      "New drift over neighbor ink must be identified as an ownership review warning");
  std::vector<GlyphInstance*> flat;
  for (auto& g : f.page[0]) flat.push_back(&g);
  require(violationContextIndices(flat, {0, 5}, *warning) == std::vector<int>({0, 1, 3, 4}),
      "Report context must include both words and the assigned owner");

  // An unchanged, intentional anchor near a neighbor is not solver-induced drift.
  mark.baseX += mark.dx;
  mark.dx = 0;
  v.clear();
  optimizePage(f.page, f.classes, quiet(), &v);
  require(!contains(v, ViolationType::BaseAssociation), "Existing anchor ambiguity must not be mislabelled as new solver drift");
}

void worldBoundsAndAppliedOffsets() {
  Fixture f;
  f.add("base", 200, 0);
  auto& a = f.add("fatha", 100, 100, 40, 20);
  auto& b = f.add("shadda", 90, 100, 40, 20);
  SolverContext context(f.page, f.classes);
  HorizontalOrderConstraint c(a, b, 0.8, 0.0, 0.0);
  std::vector<ConstraintViolation> v;
  c.reportViolations(context, v);
  require(v.size() == 1 && std::abs(v[0].severity - 22.0) < 1e-8, "Order residual must use scaled ink bounds, not raw metrics");
  c.project(context, 1.0);
  v.clear();
  c.reportViolations(context, v);
  require(v.empty(), "World ink ordering should be resolved by a hard projection");

  digitalkhatt::GlyphLayoutInfo layout{};
  layout.x_offset = 10;
  layout.y_offset = -10;
  GlyphInstance delta;
  delta.dx = 30;
  delta.dy = 7;
  applySolvedGlyphOffsets(layout, delta, 1.5);
  require(layout.x_offset == 30 && layout.y_offset == -3, "Rendered X offsets must undo line scaling for world-space corrections");
  delta.dx = std::numeric_limits<double>::max();
  applySolvedGlyphOffsets(layout, delta, 0.5);
  require(layout.x_offset == 30 && layout.y_offset == -3, "Overflowing offsets must not corrupt placement integers");

  a.iterationMaxMovementSquared = 0;
  a.dx += 10;
  buildWorldPolys(a);
  a.dx -= 10;
  buildWorldPolys(a);
  require(a.iterationMaxMovementSquared >= 100, "Opposing corrections must not appear converged just because net movement is zero");
}

void stretchedFathaJoinedHorizontalGap() {
  for (const auto* fathaName : {"fatha.stretched"}) {
    for (const auto* joinedName : {"smallalef.joined", "hamzaabove.joined"}) {
      for (bool fathaIsA : {true, false}) {
        for (double added : {0.0, 0.0625, 0.125, 0.5, 1.0, 2.0}) {
          Fixture f;
          f.classes["marks"].insert(fathaName);
          f.classes["marks"].insert(joinedName);
          auto& a = f.add(fathaIsA ? fathaName : joinedName, 100, 100, fathaIsA ? 200 : 20, 20);
          auto& b = f.add(fathaIsA ? joinedName : fathaName, 90, 100, fathaIsA ? 20 : 200, 20);
          auto& fatha = fathaIsA ? a : b;
          auto& joined = fathaIsA ? b : a;
          fatha.preserveGposHorizontal = true;
          fatha.justificationAddedLeftTatweel = added;
          SolverContext context(f.page, f.classes);
          HorizontalOrderConstraint constraint(a, b, 0.8, 0.0, 0.0);
          const double gap = std::min(100.0, 100 * added);
          const double deficit = b.worldPolys.boundingAABB().maxx + gap - a.worldPolys.boundingAABB().minx;
          std::vector<ConstraintViolation> violations;
          constraint.reportViolations(context, violations);
          require(violations.size() == 1 && std::abs(violations[0].severity - deficit) < 1e-9, "Joined/Fatha report must measure the same scaled horizontal gap as projection");
          constraint.project(context, 1);
          require(fatha.dx == 0 && fatha.dy == 0 && joined.dy == 0, "Only the joined mark may move horizontally for this pair");
          require(std::abs(joined.dx - (fathaIsA ? -deficit : deficit)) < 1e-9, "Joined mark must move in semantic ownership order on either side of Fatha");
          require(fatha.mobility == 1 && joined.mobility == 1, "Ordering must not change global glyph mobilities");
          require(std::abs(a.worldPolys.boundingAABB().minx - b.worldPolys.boundingAABB().maxx - gap) < 1e-9, "Projection must establish the justification-dependent edge-to-edge gap");
          violations.clear();
          constraint.reportViolations(context, violations);
          require(violations.empty(), "Satisfied scaled gap must not report a violation");
          const double solvedX = joined.dx;
          constraint.project(context, 1);
          require(fatha.dx == 0 && joined.dx == solvedX, "An already satisfied pair must not keep drifting");

          joined.mobility = 0;
          joined.dx = 0;
          buildWorldPolys(joined);
          constraint.project(context, 1);
          require(fatha.dx == 0 && joined.dx == 0, "Rigid joined marks must not cause the ordering constraint to move Fatha instead");
          violations.clear();
          constraint.reportViolations(context, violations);
          require(violations.size() == 1, "An infeasible rigid pair must retain its gap diagnostic");
        }
      }
    }
  }

  // Even a contextual positive-width Fatha keeps the ordinary rule. Its
  // legacy preserve flag must not turn it into a justification-stretched mark.
  for (const auto* neighbor : {"smallalef.joined", "hamzaabove.joined", "shadda"}) {
    Fixture f;
    f.classes["marks"].insert(neighbor);
    auto& a = f.add("fatha", 100, 100, 40, 20);
    auto& b = f.add(neighbor, 90, 100, 40, 20);
    a.preserveGposHorizontal = true;
    SolverContext context(f.page, f.classes);
    HorizontalOrderConstraint constraint(a, b, 0.8, 0.0, 0.0);
    constraint.project(context, 1);
    require(std::abs(a.dx - 11) < 1e-9 && std::abs(b.dx + 11) < 1e-9, "Ordinary pairs must retain the existing width-fraction rule and both mobilities");
  }
}

void gapReportingAndKeys() {
  Fixture f;
  auto& a = f.add("fatha", 0, 100, 20, 20);
  auto& b = f.add("shadda", 19, 100, 20, 20);
  auto& c = f.add("waqf.meem", 100, 100, 20, 20);
  SolverContext context(f.page, f.classes);
  const GapKey ab{&a, &b}, ba{&b, &a}, ca{&c, &a};
  require(ab == ba && !(ab == ca), "Gap identity must require both participants");
  context.gapInfos[ab].lambda = 2;
  require(context.gapInfos.find(ba) != context.gapInfos.end(), "Reversed gap keys must share a hash and state");
  c.mobility = 2;
  const auto [wa, wb] = gapMobilities(context, a, c);
  require(wa + wb == 4, "Gap diagnostics and projection must use the same waqf mobility bias");

  context.gapInfos[ab].initialC = -5000;
  context.gapInfos[ab].hasInitialC = true;
  std::vector<ConstraintViolation> v;
  collectGapViolations(context, {a, b}, OptParams{}, v);
  require(contains(v, ViolationType::GenericGap), "Actual ink intersection must not be hidden as expected compliance slack");

  Fixture clearance;
  clearance.add("base", 0, 0, 20, 20);
  clearance.add("fatha", 0, 25, 20, 20);
  auto p = quiet();
  p.toggles.reportGenericGap = true;
  p.reportGenericGapCollisionsOnly = false;
  v.clear();
  optimizePage(clearance.page, clearance.classes, p, &v);
  require(contains(v, ViolationType::GenericGap), "An explicit clearance report must include positive violations, even with gap forces off");
}

void completeConstraintCoverageAndConvergence() {
  Fixture f;
  f.add("base", 0, 0, 100, 100);
  f.add("onedotup", 30, 560, 20, 40);
  f.add("shadda", 30, 580, 20, 40);
  f.add("fatha", 30, 620, 20, 40);
  auto p = quiet();
  p.toggles.stackOrder = true;
  std::vector<ConstraintViolation> v;
  optimizePage(f.page, f.classes, p, &v);
  require(contains(v, ViolationType::StackOrderGap), "Stack constraints must participate in the report");
  p.maxIters = 20;
  v.clear();
  optimizePage(f.page, f.classes, p, &v);
  require(!contains(v, ViolationType::StackOrderGap), "Coupled stacks must converge even when generic gap is disabled");

  Fixture waqf;
  waqf.add("base", 0, 0, 100, 100);
  waqf.add("waqf.meem", 20, 200, 20, 40);
  p = quiet();
  p.toggles.waqfPlacement = true;
  v.clear();
  optimizePage(waqf.page, waqf.classes, p, &v);
  require(contains(v, ViolationType::WaqfPlacement), "Waqf bounds must participate in the report");

  Fixture lane;
  lane.add("base", 0, 0, 100, 100);
  lane.add("fatha", 20, 100, 20, 20);
  p = quiet();
  p.toggles.ylane = true;
  p.toggles.reportSoftResiduals = true;
  v.clear();
  optimizePage(lane.page, lane.classes, p, &v);
  require(contains(v, ViolationType::Ylane), "Optional soft report must include lane residuals");
  p.toggles.reportSoftResiduals = false;
  v.clear();
  optimizePage(lane.page, lane.classes, p, &v);
  require(!contains(v, ViolationType::Ylane), "Soft targets must remain distinct from hard safety failures");
}
void stretchedMarksRetainCollisionSafety() {
  Fixture f;
  f.add("base", 0, 0, 100, 100);
  auto& mark = f.add("fatha", -500, 20, 200, 40);
  mark.preserveGposHorizontal = true;
  f.add("neighbor", -450, 0, 100, 100);
  auto p = quiet();
  p.toggles.squeezeCenter = p.toggles.baseVicinity = true;
  p.toggles.reportGenericGap = true;
  p.reportGenericGapCollisionsOnly = false;
  std::vector<ConstraintViolation> v;
  optimizePage(f.page, f.classes, p, &v);
  require(!contains(v, ViolationType::BaseVicinity) && !contains(v, ViolationType::SqueezeCenter), "Authored stretched-mark placement must not receive generic centering/leash constraints");
  require(contains(v, ViolationType::GenericGap), "A stretched mark must still participate in collision safety");
  mark.preserveGposHorizontal = false;
  v.clear();
  optimizePage(f.page, f.classes, p, &v);
  require(contains(v, ViolationType::BaseVicinity), "Ordinary marks must retain their base-vicinity constraint");
}

void stretchedFathaBandRespectsAuthoredHeight() {
  for (const auto& [name, preserve] : {std::pair{"fatha", false}, std::pair{"fatha", true}, std::pair{"fatha.stretched", false}, std::pair{"fatha.stretched", true}}) {
    Fixture f;
    f.classes["marks"].insert(name);
    f.classes["topmarks"].insert(name);
    auto& base = f.add("base", 0, -1000, 100, 100);
    base.lineY = -1000;
    auto& mark = f.add(name, 20, -720, 400, 40);
    mark.preserveGposHorizontal = preserve;
    SolverContext context(f.page, f.classes);
    YlaneConstraint lane(mark, 0, 600);
    const bool stretched = std::string_view(name) == "fatha.stretched" && preserve;
    require(lane.laneHeight == (stretched ? 300 : 600), "Only justification-stretched Fatha may use its authored GPOS band floor");
    std::vector<ConstraintViolation> violations;
    lane.reportViolations(context, violations);
    require(contains(violations, ViolationType::Ylane) != stretched, "Lane reporting must use the same adjusted floor as projection");
    lane.project(context, 1);
    require(std::abs(mark.dy - (stretched ? 0 : 300)) < 1e-9, "Long stretched Fatha must not be lifted to the ordinary high band");
    if (!stretched) continue;

    mark.dy = -50;
    buildWorldPolys(mark);
    lane.project(context, 1);
    require(std::abs(mark.dy) < 1e-9, "The adjusted lane must still restore a mark pushed below its authored floor");
    mark.dy = 80;
    buildWorldPolys(mark);
    lane.project(context, 1);
    require(mark.dy == 80, "The adjusted lane must never pull a raised mark down");
    YlaneConstraint repeated(mark, 0, 600);
    require(repeated.laneHeight == 300, "Rebuilding constraints must not adopt previous solver displacement as the authored floor");
    mark.baseY = -120;
    mark.dy = 0;
    buildWorldPolys(mark);
    YlaneConstraint alreadyHigh(mark, 0, 600);
    require(alreadyHigh.laneHeight == 600, "Stretched Fatha already above the ordinary band must retain its existing floor");
    alreadyHigh.project(context, 1);
    require(mark.dy == 0, "Already-high stretched Fatha must not be moved downward");
  }

  Fixture wired;
  wired.classes["marks"].insert("fatha.stretched");
  wired.classes["topmarks"].insert("fatha.stretched");
  wired.add("base", 0, 0, 100, 100);
  auto& mark = wired.add("fatha.stretched", 20, 280, 400, 40);
  mark.preserveGposHorizontal = true;
  auto p = quiet();
  p.maxIters = 20;
  p.toggles.ylane = true;
  p.toggles.reportSoftResiduals = true;
  p.compliance.ylane = 0;
  std::vector<ConstraintViolation> violations;
  optimizePage(wired.page, wired.classes, p, &violations);
  require(mark.dy == 0 && !contains(violations, ViolationType::Ylane), "Per-base lane wiring must respect the stretched Fatha's original height");

  mark.baseY = 20;
  buildWorldPolys(mark);
  p.toggles.hardStayAboveBelow = true;
  violations.clear();
  optimizePage(wired.page, wired.classes, p, &violations);
  require(boxBottomY(mark) >= 100 - 1e-3 && !contains(violations, ViolationType::HardStayAbove), "Relaxing the decorative high band must not weaken the hard baseline safety rail");
}
void fontAuthoredBaselineClearance() {
  for (const double minimum : {50.0, 100.0, 200.0}) {
    Fixture f;
    f.classes["marks"].insert("fatha.stretched");
    f.classes["topmarks"].insert("fatha.stretched");
    auto& base = f.add("base", 0, -100, 100, 10);
    base.lineY = -250;
    auto& mark = f.add("fatha.stretched", -300, -270, 400, 40);
    mark.lineY = -250;
    mark.preserveGposHorizontal = true;
    mark.baselineClearance = minimum;
    auto p = quiet();
    p.maxIters = 20;
    p.minViolationSeverity = 0.001;
    std::vector<ConstraintViolation> violations;
    optimizePage(f.page, f.classes, p, &violations);
    require(boxBottomY(mark) >= base.lineY + minimum - 1e-3, "Font-authored floor must protect the lowest tip relative to the line baseline, even outside the base footprint");
    require(mark.dx == 0 && base.dy == 0 && !contains(violations, ViolationType::HardStayAbove), "Baseline floor must not recenter Fatha, move the base or leave an unresolved bound");
    const double solvedY = mark.dy;
    optimizePage(f.page, f.classes, p, &violations);
    require(std::abs(mark.dy - solvedY) < 1e-3, "Repeated optimization must not accumulate baseline lift");
  }
  Fixture ordinary;
  ordinary.add("base", 0, 0);
  auto& untouched = ordinary.add("fatha", 0, 0);
  auto p = quiet();
  p.maxIters = 20;
  std::vector<ConstraintViolation> violations;
  optimizePage(ordinary.page, ordinary.classes, p, &violations);
  require(untouched.dy == 0, "Absent font metadata must leave ordinary marks and other fonts unchanged");

  Fixture interaction;
  interaction.classes["marks"].insert("fatha.stretched");
  interaction.classes["topmarks"].insert("fatha.stretched");
  interaction.add("base", 0, 0, 100, 20);
  interaction.add("shadda", 0, 0, 20, 20);
  auto& stretched = interaction.add("fatha.stretched", 0, 90, 200, 20);
  stretched.preserveGposHorizontal = true;
  stretched.baselineClearance = 200;
  p.toggles.stackOrder = true;
  p.toggles.reportSoftResiduals = true;
  violations.clear();
  optimizePage(interaction.page, interaction.classes, p, &violations);
  require(boxBottomY(stretched) >= 200 - 1e-3 && !contains(violations, ViolationType::HardStayAbove) && !contains(violations, ViolationType::StackOrderGap), "Baseline safety must win over a lower preferred Shadda gap without weakening either hard bound");
}
void lowerDotKasraStack() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");
  f.classes["downdotmarks"].insert("onedotdown");
  f.add("behshape.init", 0, 0, 200, 50);
  auto& dot = f.add("onedotdown", 40, -120, 40, 30);
  f.add("fatha", 50, 120);  // An intervening top mark must not break ownership.
  auto& kasra = f.add("kasra", 20, -130, 100, 20);
  const double dotY = dot.baseY, offset = boxCenterX(dot) - boxCenterX(kasra);
  auto p = quiet();
  p.toggles.rehWawBehPlacement = true;
  p.maxIters = 60;
  p.toggles.stackOrder = p.toggles.genericGapConstraint = true;
  p.minViolationSeverity = 0.1;
  std::vector<ConstraintViolation> violations;
  optimizePage(f.page, f.classes, p, &violations);
  require(boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - 0.1,
      "Lower stack must put kasra below same-owner dots, even across a top mark");
  require(dot.dy == 0 && dot.baseY == dotY && std::abs(boxCenterX(dot) - boxCenterX(kasra) - offset) < 0.1,
      "Lower stack must preserve dot placement and authored horizontal offset");
  require(!contains(violations, ViolationType::StackOrderGap), "Lower stack must converge without a stack residual");

  Fixture unrelated;
  unrelated.classes = f.classes;
  unrelated.add("behshape.init", 0, 0, 200, 50);
  auto& otherDot = unrelated.add("onedotdown", 40, -70, 40, 30);
  unrelated.add("base", 0, 0, 200, 50);
  auto& otherKasra = unrelated.add("kasra", 20, -80, 100, 20);
  p.toggles.genericGapConstraint = false;
  optimizePage(unrelated.page, unrelated.classes, p);
  require(otherDot.dy == 0 && otherKasra.dy == 0, "Marks with different owners must not form a lower stack");
}

void rehWawLowerMarkPlacement() {
  for (const std::string owner : {"behshape.init", "behshape.init.beforeseen"}) {
    for (const std::string reference : {"reh.isol", "waw.fina"}) {
      for (const std::string dotName : {"", "onedotdown", "twodotsdown"}) {
        Fixture f;
        for (const auto name : {"onedotdown", "twodotsdown"}) {
          f.classes["marks"].insert(name);
          f.classes["downdotmarks"].insert(name);
        }
        auto& reh = f.add(reference, 150, -150, 120, 200);
        auto& beh = f.add(owner, 0, 0, 120, 50);
        if (!dotName.empty()) f.add(dotName, 90, -110, dotName == "onedotdown" ? 30 : 60, 30);
        auto& kasra = f.add("kasra", 20, -60, 100, 20);
        auto p = quiet();
        p.toggles.rehWawBehPlacement = true;
        p.maxIters = 60;
        p.toggles.stackOrder = p.toggles.genericGapConstraint = true;
        std::vector<ConstraintViolation> violations;
        optimizePage(f.page, f.classes, p, &violations);
        require(kasra.prevBase == &beh && reh.dx == 0 && reh.dy == 0,
            "Tip placement must retain semantic ownership and fixed base geometry");
        if (dotName.empty()) {
          require(boxTopY(kasra) < boxBottomY(reh) && std::abs(boxCenterX(kasra) - 150) < 25,
              "A lone kasra must prefer below the Reh/Waw tip");
        } else {
          const auto& dot = f.page[0][2];
          require(boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - p.tolCollision,
              "Ordinary stacking must keep kasra below its dot after initial positioning");
          if (dot.initialPlacement == GlyphInstance::InitialPlacement::CenteredGap)
            require(dot.mobility == 0 && dot.dx == dot.initialPositioningDx && dot.dy == dot.initialPositioningDy,
                "A centered dot must remain at its initial positioned input throughout XPBD");
        }
        require(chooseMinGap(reh, kasra, p) == rehWawTipGap && chooseMinGap(kasra, reh, p) == rehWawTipGap,
            "Contextual clearance must be symmetric in broadphase pair order");
        require(!contains(violations, ViolationType::StackOrderGap), "Contextual X target must not fight dot stacking");
        if (!dotName.empty()) {
          auto& dot = f.page[0][2];
          SolverContext context(f.page, f.classes);
          context.rehWawBehPlacement = true;
          require(!isManuallyPositionedMark(dot, context), "Contextual dots must participate in collision and stack constraints");
        }
        // Word/line boundaries must take priority over the contextual exception.
        kasra.wordIndex = reh.wordIndex + 1;
        require(chooseMinGap(reh, kasra, p) == p.differentWordGap, "Tip clearance must not cross word boundaries");
        kasra.wordIndex = reh.wordIndex;
        kasra.lineIndex = 1;
        require(chooseMinGap(reh, kasra, p) == p.differentWordGap, "Tip clearance must not cross lines");
      }
    }
  }
  Fixture f;
  f.add("reh.isol", 150, -150, 120, 200);
  auto& kasra = f.add("kasra", 150, -160, 100, 20);
  f.add("space", 0, 0);
  f.add("behshape.init", 0, 0, 120, 50);
  auto& separated = f.add("kasra", 10, -60, 100, 20);
  auto p = quiet(); p.maxIters = 40;
  p.toggles.rehWawBehPlacement = true;
  optimizePage(f.page, f.classes, p);
  require(kasra.dx == 0 && kasra.dy == 0, "Tip context is for the following letter; Reh's own kasra retains its authored placement");
  require(separated.dx == 0 && separated.dy == 0, "A space must block contextual placement");

  Fixture swash;
  auto& remote = swash.add("noon.fina.basmala", 100, -320, 150, 60);
  swash.add("space", 0, 0);
  swash.add("reh.isol", 150, -150, 120, 200);
  swash.add("behshape.init", 0, 0, 120, 50);
  auto& underTip = swash.add("kasra", 20, -60, 100, 20);
  p.toggles.genericGapConstraint = true;
  optimizePage(swash.page, swash.classes, p);
  require(boxTopY(underTip) <= boxBottomY(remote) - p.differentWordGap + 0.1 &&
          !geometry::getDistance(remote.worldPolys, underTip.worldPolys, 10).contact.intersect,
      "Tip placement must clear a physically overlapping swash even across a word boundary and intervening bases");

  Fixture bowlDot;
  bowlDot.classes["marks"].insert("onedotdown");
  bowlDot.classes["downdotmarks"].insert("onedotdown");
  bowlDot.add("reh.isol", 150, -150, 120, 200);
  bowlDot.add("behshape.init", 0, 0, 120, 50);
  auto& insideDot = bowlDot.add("onedotdown", 90, -110, 30, 30);
  auto& neighbor = bowlDot.add("ain.fina", 0, -300, 240, 350);
  neighbor.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
      {{0,0},{240,0},{240,20},{0,20}},
      {{0,0},{20,0},{20,350},{0,350}},
      {{220,0},{240,0},{240,350},{220,350}}});
  optimizePage(bowlDot.page, bowlDot.classes, p);
  require(boxBottomY(insideDot) > boxBottomY(neighbor) + 100,
      "A single dot near the Reh tip must be allowed to use a neighboring bowl's free interior");
}
void persistentDotInkEscape() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");
  f.classes["downdotmarks"].insert("onedotdown");
  auto& stroke = f.add("neighbor", 0, -100, 100, 30);
  auto& dot = f.add("onedotdown", 30, -90, 30, 40);
  SolverContext context(f.page, f.classes);
  StackOrderConstraint escape(stroke, dot, false, 30, 0.0, 0.2, 0, true);
  escape.keepInnerForGap = true;
  escape.alignHorizontal = false;
  escape.onlyAfterInkContact = true;
  escape.project(context, 1);
  escape.project(context, 1);
  require(dot.dy == 0 && !escape.inkEscapeActive, "A transient dot contact must not force it below an entire neighboring bowl");
  escape.project(context, 1);
  require(escape.inkEscapeActive && boxTopY(dot) <= boxBottomY(stroke) - 30 + 0.001,
      "Persistent dot/ink contact must activate a downward escape with clearance");
}
void initialBehOptInAndWideSlot() {
  require(!OptParams{}.toggles.rehWawBehPlacement, "Initial-Beh placement must default to the previous implementation");
  for (const std::string owner : {"behshape.init", "behshape.init.ii", "behshape.init.beforereh", "behshape.init.beforebeh", "behshape.init.beforeheh", "behshape.init.beforeyeh", "behshape.init.beforeseen", "behshape.init.beforeseen.ii", "behshape.init.beforeseen.iii", "behshape.init.beforeseen.iv", "behshape.init.beforenoon", "behshape.init.beforehah", "behshape.init.alt", "behshape.isol", "behshape.medi", "behshape.initial", "behshape.initish", "hah.init"}) {
    for (bool enabled : {false, true}) {
      Fixture f;
      f.classes["marks"].insert("twodotsdown");
      f.classes["downdotmarks"].insert("twodotsdown");
      f.add("reh.isol", 150, -150, 120, 200);
      f.add(owner, 0, 0, 120, 50);
      auto& dot = f.add("twodotsdown", 60, -100, 50, 30);
      auto& kasra = f.add("kasra", 20, -60, 100, 20);
      auto p = quiet(); p.maxIters = 40;
      p.toggles.stackOrder = true;
      p.toggles.rehWawBehPlacement = enabled;
      optimizePage(f.page, f.classes, p);
      const bool supported = owner != "behshape.isol" && owner != "behshape.medi" &&
                             owner != "behshape.initial" && owner != "behshape.initish" && owner != "hah.init";
      if (!enabled || !supported) {
        require(dot.dx == 0 && dot.dy == 0 && kasra.dx == 0 && kasra.dy == 0,
            "Disabled placement and unrelated glyph forms must retain the old behavior");
      } else {
        require(isRehWawDot(dot) && rehWawReference(kasra) == &f.page[0][0],
            "Supported initial-Beh variants must recognize their lower dots and Reh reference");
        SolverContext context(f.page, f.classes);
        context.rehWawBehPlacement = true;
        require(!isManuallyPositionedMark(dot, context),
            "Supported variants must release the anchored-dot exception when enabled");
        require(boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - p.tolCollision,
            "Supported initial-Beh variants must keep kasra beneath their dots");
      }
    }
  }
  Fixture wide;
  wide.classes["marks"].insert("twodotsdown");
  wide.classes["downdotmarks"].insert("twodotsdown");
  auto& reh = wide.add("reh.isol", 400, -150, 120, 200);
  wide.add("behshape.init", 0, 0, 350, 50);
  auto& dot = wide.add("twodotsdown", 300, -160, 60, 30);
  auto p = quiet(); p.maxIters = 40; p.toggles.rehWawBehPlacement = true;
  optimizePage(wide.page, wide.classes, p);
  require(boxCenterX(dot) < reh.worldPolys.boundingAABB().minx - 90 &&
          std::abs(boxCenterY(dot) - boxBottomY(reh)) < 0.1,
      "A wide initial Beh must use the available space to the left of the tip with its center at the leftmost tip level");

  Fixture narrow;
  narrow.classes = wide.classes;
  auto& narrowReh = narrow.add("reh.isol", 150, -200, 446, 160);
  narrowReh.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
      {{0,0},{446,0},{446,20},{0,20}},
      {{426,0},{446,0},{446,160},{426,160}}});
  auto& beh = narrow.add("behshape.init", 0, 0, 246, 50);
  auto& dots = narrow.add("twodotsdown", 60, -140, 230, 135);
  // Leave enough bowl width for the larger 40-unit neighboring clearance.
  auto& ain = narrow.add("ain.fina", -300, -500, 660, 550);
  ain.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
      {{0,0},{660,0},{660,20},{0,20}},
      {{0,0},{20,0},{20,550},{0,550}},
      {{640,0},{660,0},{660,550},{640,550}}});
  narrow.classes["marks"].insert("kasratan");
  narrow.classes["lowmarks"].insert("kasratan");
  narrow.classes["bowlbases"].insert("ain.fina");
  auto& fixedKasratan = narrow.add("kasratan", -173, -301, 194, 273);
  p.toggles.genericGapConstraint = true;
  optimizePage(narrow.page, narrow.classes, p);
  const double behGap = geometry::getDistance(dots.worldPolys, beh.worldPolys, 1000).contact.depth_or_gap;
  const double rehGap = geometry::getDistance(dots.worldPolys, narrowReh.worldPolys, 1000).contact.depth_or_gap;
  require(behGap > 10 && rehGap > 10 && boxTopY(dots) <= -200 - rehWawTipGap + 0.1,
      "An obstructed current gap and insufficient left space must fall back below the Reh tip");
  require(boxBottomY(dots) > boxBottomY(ain) + 100 && boxCenterX(dots) >= 0 && boxCenterX(dots) <= 246,
      "Two dots must stay near their Beh owner instead of dropping beneath the next Ain's bowl");
  require(geometry::getDistance(dots.worldPolys, fixedKasratan.worldPolys, 1000).contact.depth_or_gap >= rehWawNeighborInkGap - p.tolCollision &&
          fixedKasratan.dx == 0 && fixedKasratan.dy == 0,
      "A fixed kasratan inside the following Ain must remain an obstacle without being moved");
}
void conditionalDotSlotAndKasraFollow() {
  for (bool blocked : {false, true}) {
    Fixture f;
    f.classes["marks"].insert("twodotsdown");
    f.classes["downdotmarks"].insert("twodotsdown");
    GlyphInstance* blocker = blocked ? &f.add("blocker", 0, -150, 390, 160) : nullptr;
    auto& reh = f.add("reh.isol", 400, -150, 120, 200);
    auto& beh = f.add("behshape.init.beforeseen", 0, 0, 350, 50);
    auto& dot = f.add("twodotsdown", 300, -160, 60, 30);
    auto& kasra = f.add("kasra", 300, -230, 60, 20);
    auto p = quiet();
    RehWawMarkPlacement placement(dot, reh, p);
    placement.obstacles = {&reh, &beh};
    if (blocker) placement.obstacles.push_back(blocker);
    placement.prepareDotTarget();
    applyPreferred(placement);
    require(placement.dotTargetClear && placement.dotUsesLeftSlot == !blocked,
        "A wide left slot is eligible only when the whole dot also clears neighboring ink");
    require(blocked ? boxTopY(dot) <= -180 + 0.1 : std::abs(boxCenterY(dot) + 150) < 0.1,
        "Blocked side space must select below the tip; clear side space must align the center with it");
    if (!blocked) {
      RehWawMarkPlacement follow(kasra, reh, p);
      follow.obstacles = {&reh, &beh};
      follow.dots.push_back(&dot);
      follow.followedDotPlacement = &placement;
      applyPreferred(follow);
      require(boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - 0.1,
          "Initial kasra positioning must fit beneath the same-owner dot");
    }
  }

  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  f.classes["downdotmarks"].insert("twodotsdown");
  auto& reference = f.add("reh.isol", 150, -150, 120, 200);
  auto& beh = f.add("behshape.init", 0, 0, 120, 50);
  auto& dot = f.add("twodotsdown", 60, -100, 60, 30);
  auto& nextReh = f.add("reh.fina", -80, -360, 240, 410);
  auto p = quiet();
  RehWawMarkPlacement placement(dot, reference, p);
  placement.obstacles = {&reference, &beh, &nextReh};
  placement.prepareDotTarget();
  applyPreferred(placement);
  require(placement.dotTargetClear && !placement.dotUsesLeftSlot &&
          boxTopY(dot) <= boxBottomY(nextReh) - 14.0 + 0.1 &&
          !geometry::getDistance(dot.worldPolys, nextReh.worldPolys, 10).contact.intersect,
      "An obstructing final Reh must allow a clear downward fallback past the original search limit");
}

void dotSlotMustFitFollowingKasra() {
  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  f.classes["downdotmarks"].insert("twodotsdown");
  auto& reh = f.add("reh.isol", 400, -150, 120, 200);
  auto& beh = f.add("behshape.init", 0, 0, 350, 50);
  auto& dot = f.add("twodotsdown", 300, -160, 60, 30);
  auto& kasra = f.add("kasra", 300, -230, 60, 140);
  auto& nextLine = f.add("fatha", 0, -360, 400, 145);
  nextLine.lineIndex = 1; nextLine.lineY = -600;
  auto p = quiet();
  RehWawMarkPlacement placement(dot, reh, p);
  placement.obstacles = {&reh, &beh, &nextLine};
  placement.prepareDotTarget();
  require(placement.dotTargetClear && placement.dotUsesLeftSlot,
      "The test's upper side slot must fit the dot by itself");
  RehWawMarkPlacement follow(kasra, reh, p);
  follow.obstacles = {&reh, &beh};
  follow.dots.push_back(&dot);
  follow.followedDotPlacement = &placement;
  placement.kasras.push_back(&follow);
  placement.prepareDotTarget();
  require(!placement.dotTargetClear,
      "A slot that fits the dot but puts its following kasra into the next line must be rejected");
  applyPreferred(follow);
  require(kasra.dx == 0 && kasra.dy == 0,
      "An infeasible pair target must not pull kasra into neighboring ink");
}

void dotNeighborClearance() {
  double previousGap = 0.0;
  for (double preferredGap : {40.0, 80.0}) {
    Fixture f;
    f.classes["marks"].insert("onedotdown");
    auto& reh = f.add("reh.isol", 400, -150, 120, 200);
    auto& beh = f.add("behshape.init", 0, 0, 350, 50);
    auto& dot = f.add("onedotdown", 300, -160, 30, 30);
    auto& heh = f.add("heh.medi", 0, -200, 150, 200);
    auto p = quiet();
    p.toggles.rehWawBehPlacement = true;
    p.sameWordGap = preferredGap;
    p.differentWordGap = 120.0;
    require(chooseMinGap(heh, dot, p) == preferredGap && chooseMinGap(dot, heh, p) == preferredGap,
        "The following letter must retain ordinary same-word spacing instead of the compact tip exception");
    require(chooseMinGap(reh, dot, p) == rehWawTipGap && chooseMinGap(beh, dot, p) == rehWawTipGap,
        "Compact contacts belong only to the reference Reh/Waw and the dot's own Beh");
    RehWawMarkPlacement placement(dot, reh, p);
    placement.obstacles = {&reh, &beh, &heh};
    placement.prepareDotTarget();
    applyPreferred(placement);
    const double gap = geometry::getDistance(dot.worldPolys, heh.worldPolys, 1000).contact.depth_or_gap;
    require(placement.dotTargetClear && gap >= rehWawNeighborInkGap - 0.1,
        "Other letters must have at least 40 units of candidate clearance");
    if (previousGap > 0.0)
      require(gap > previousGap + 10, "Candidate ranking must prefer the configured larger neighboring gap when space exists");
    previousGap = gap;
  }
}

void followingKasraCanUseClearBowlInterior() {
  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  f.classes["downdotmarks"].insert("twodotsdown");
  f.add("reh.isol", 200, -150, 120, 200);
  f.add("behshape.init", 0, 0, 170, 50);
  auto& dot = f.add("twodotsdown", 70, -160, 60, 30);
  auto& kasra = f.add("kasra", 50, -220, 100, 20);
  auto& ain = f.add("ain.fina", -300, -500, 600, 550);
  ain.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
      {{0,0},{600,0},{600,20},{0,20}},
      {{0,0},{20,0},{20,550},{0,550}},
      {{580,0},{600,0},{600,550},{580,550}}});
  auto p = quiet(); p.maxIters = 40;
  p.toggles.rehWawBehPlacement = p.toggles.stackOrder = true;
  optimizePage(f.page, f.classes, p);
  require(boxTopY(kasra) > boxBottomY(ain) + 200 &&
          boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - p.tolCollision &&
          !geometry::getDistance(kasra.worldPolys, ain.worldPolys, 10).contact.intersect,
      "Following kasra must use a clear bowl interior instead of being forced below the entire Ain");
}

void limitedLeftSlotOverhang() {
  for (const std::string dotName : {"onedotdown", "twodotsdown"}) {
    for (double shortfall : {2.0, 40.0}) {
      Fixture f;
      f.classes["marks"].insert(dotName);
      const double width = dotName == "onedotdown" ? 122.0 : 232.0;
      const double tipX = width + 100;
      auto& reh = f.add("reh.isol", tipX, -150, 120, 200);
      // No current cavity fits at the vertical Reh edge. The owner's width,
      // rather than the reference's tip position, limits the separate left slot.
      auto& beh = f.add("behshape.init", 0, 0, width - shortfall, 50);
      auto& dot = f.add(dotName, 60, -140, width, 60);
      auto& kasra = f.add("kasra", 60, -240, 60, 20);
      auto& heh = f.add("heh.fina", -100, 0, 120, 100);
      auto p = quiet();
      RehWawMarkPlacement placement(dot, reh, p), follow(kasra, reh, p);
      placement.obstacles = {&reh, &beh, &heh};
      follow.dots.push_back(&dot);
      placement.kasras.push_back(&follow);
      placement.prepareDotTarget();
      applyPreferred(placement);
      require(placement.dotTargetClear && placement.dotUsesLeftSlot == (shortfall == 2.0),
          "A small bounding-box shortfall may use clear left space; a large one must retain the below-tip fallback");
      if (shortfall != 2.0) continue;

      const auto box = dot.worldPolys.boundingAABB();
      const double limit = std::min(rehWawLeftOverhangLimit, width * rehWawLeftOverhangFraction);
      require(box.minx < 0 && box.minx >= -limit - 0.1 && box.maxx <= tipX + 0.1 && std::abs(boxCenterY(dot) + 150) < 0.1,
          "Overhang must stay small and keep the dot left of the reference tip with its center aligned to it");
      require(geometry::getDistance(dot.worldPolys, reh.worldPolys, 1000).contact.depth_or_gap >= rehWawCompactInkGap - 0.1 &&
              geometry::getDistance(dot.worldPolys, beh.worldPolys, 1000).contact.depth_or_gap >= rehWawCompactInkGap - 0.1 &&
              geometry::getDistance(dot.worldPolys, heh.worldPolys, 1000).contact.depth_or_gap >= rehWawNeighborInkGap - 0.1,
          "Overhanging dots must retain the actual 14/40-unit outline clearances");
      applyPreferred(follow);
      require(boxBottomY(dot) - boxTopY(kasra) >= belowDotKasraGap - 0.1,
          "The overhang search must still accommodate the following same-owner kasra");

      p.rehWaw.overhangPercent = 0;
      placement.prepareDotTarget();
      require(!placement.dotUsesLeftSlot, "Setting allowed overhang to zero must restore the full-width restriction");
      p.rehWaw.overhangPercent = 10;
      p.rehWaw.overhangLimit = 0;
      placement.prepareDotTarget();
      require(!placement.dotUsesLeftSlot, "The configured absolute cap must limit overhang independently of its percentage");
      p.rehWaw.overhangLimit = 14;

      auto& blocker = f.add("blocker", -limit, -150, tipX + limit, 150);
      placement.obstacles.push_back(&blocker);
      placement.prepareDotTarget();
      applyPreferred(placement);
      require(placement.dotTargetClear && !placement.dotUsesLeftSlot && boxTopY(dot) <= -150 - rehWawTipGap + 0.1,
          "An obstructed overhang must fall back below the tip instead of bypassing clearance checks");
    }
  }
}

void clearCurrentGapHasPriority() {
  for (const std::string dotName : {"onedotdown", "twodotsdown"}) {
    for (bool belowTip : {false, true}) {
      for (double clearance : {6.0, 14.0}) {
        Fixture f;
        f.classes["marks"].insert(dotName);
        const double width = dotName == "onedotdown" ? 120.0 : 230.0;
        auto& reh = f.add("waw.isol", width - 80, -200, 446, 20);
        auto& beh = f.add("behshape.init", 0, -100, width + 16, 50);
        // The current cavity is 80 units high for a 60-unit mark: it can
        // clear both strokes by 6 units, but cannot clear both by 14.
        auto& dot = f.add(dotName, 10, -170, width, 60);
        auto p = quiet();
        p.rehWaw.compactGap = clearance;
        p.rehWaw.oneDotBelowTip = p.rehWaw.twoDotsBelowTip = belowTip;
        RehWawMarkPlacement placement(dot, reh, p);
        placement.obstacles = {&reh, &beh};
        placement.prepareDotTarget();
        applyPreferred(placement);
        if (clearance == 6.0) {
          require(placement.dotTargetClear && boxCenterY(dot) > -180 && boxCenterY(dot) < -100,
              "A clear balanced gap must take priority even when below-tip fallback is checked");
          require(geometry::getDistance(dot.worldPolys, reh.worldPolys, 1000).contact.depth_or_gap >= clearance - 0.1 &&
                  geometry::getDistance(dot.worldPolys, beh.worldPolys, 1000).contact.depth_or_gap >= clearance - 0.1,
              "Keeping the current gap must still enforce the configured outline clearance");
        } else if (belowTip) {
          require(placement.dotTargetClear && boxTopY(dot) <= -200 - p.rehWaw.tipGap + 0.1,
              "Below-tip fallback is allowed only after the current gap and left slot cannot fit safely");
        } else {
          require(!placement.dotTargetClear && dot.dx == 0 && dot.dy == 0,
              "An unchecked below-tip option must not force a descent when no clear gap fits");
        }
      }
    }
  }
}

void currentGapMustFitFollowingKasra() {
  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  auto& reh = f.add("waw.isol", 150, -200, 446, 20);
  auto& beh = f.add("behshape.init", 0, -100, 246, 50);
  auto& dot = f.add("twodotsdown", 10, -170, 230, 60);
  auto& kasra = f.add("kasra", 10, -250, 230, 20);
  auto& blocker = f.add("blocker", -300, -280, 1000, 60);
  auto p = quiet();
  p.rehWaw.compactGap = 6;
  RehWawMarkPlacement placement(dot, reh, p), follow(kasra, reh, p);
  placement.obstacles = {&reh, &beh, &blocker};
  follow.obstacles = placement.obstacles;
  follow.dots.push_back(&dot);
  follow.followedDotPlacement = &placement;
  placement.prepareDotTarget();
  require(placement.dotTargetClear && placement.dotMinY >= -200,
      "The current cavity must fit the dot before its kasra is linked");
  placement.kasras.push_back(&follow);
  placement.prepareDotTarget();
  applyPreferred(placement);
  applyPreferred(follow);
  require(placement.dotTargetClear && boxTopY(dot) <= -280 - p.rehWaw.neighborGap + 0.1,
      "A dot-only current gap must yield to below-tip fallback if its following kasra cannot fit");
  require(boxBottomY(dot) - boxTopY(kasra) >= p.rehWaw.dotKasraGap - 0.1 &&
          geometry::getDistance(kasra.worldPolys, blocker.worldPolys, 1000).contact.depth_or_gap >= p.rehWaw.neighborGap - 0.1,
      "The same-owner kasra must follow the relocated dot with stacking and neighboring clearance");
}

void currentGapUsesAvailableClearance() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");
  auto& waw = f.add("waw.fina", 150, -200, 446, 20);
  auto& beh = f.add("behshape.init", 0, 0, 246, 50);
  auto& dot = f.add("onedotdown", 90, -70, 120, 40);
  auto p = quiet();
  p.rehWaw.preferredGap = 80;
  RehWawMarkPlacement placement(dot, waw, p);
  placement.obstacles = {&waw, &beh};
  placement.prepareDotTarget();
  applyPreferred(placement);
  const double room = std::min(
      geometry::getDistance(dot.worldPolys, waw.worldPolys, 1000).contact.depth_or_gap,
      geometry::getDistance(dot.worldPolys, beh.worldPolys, 1000).contact.depth_or_gap);
  require(placement.dotTargetClear && !placement.dotUsesLeftSlot && room >= 69.5,
      "A current gap with 70 units available on both sides must use that room before preferring the authored position");
  p.rehWaw.preferredGap = p.rehWaw.compactGap;
  placement.prepareDotTarget();
  applyPreferred(placement);
  require(std::min(geometry::getDistance(dot.worldPolys, waw.worldPolys, 1000).contact.depth_or_gap,
                   geometry::getDistance(dot.worldPolys, beh.worldPolys, 1000).contact.depth_or_gap) < 69.5,
      "Reducing the comfort target to the minimum must restore authored-position ranking within that safe gap");
  p.rehWaw.preferredGap = 1000;
  placement.prepareDotTarget();
  require(placement.dotTargetClear && !placement.dotUsesLeftSlot && placement.dotMinY >= -200,
      "An unattainable comfort target must not reject a safe current gap or trigger below-tip fallback");
}

void kasraCanYieldBelowPreferredGap() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");
  auto& waw = f.add("waw.fina", 150, -300, 120, 160);
  auto& beh = f.add("behshape.init", 0, 0, 246, 50);
  auto& dot = f.add("onedotdown", 80, -100, 120, 40);
  auto& kasra = f.add("kasra", 80, -450, 200, 20);
  auto p = quiet();
  p.rehWaw.compactGap = 30;
  p.rehWaw.dotKasraGap = 50;
  p.rehWaw.oneDotBelowTip = false;
  RehWawMarkPlacement placement(dot, waw, p), follow(kasra, waw, p);
  placement.obstacles = {&waw, &beh};
  follow.obstacles = placement.obstacles;
  follow.dots.push_back(&dot);
  follow.followedDotPlacement = &placement;
  placement.kasras.push_back(&follow);
  placement.prepareDotTarget();
  applyPreferred(placement);
  applyPreferred(follow);
  require(placement.dotTargetClear && !placement.dotUsesLeftSlot && boxCenterY(dot) > -140,
      "A clear current dot gap must not be rejected merely because kasra cannot fit at the exact preferred stack gap");
  require(boxBottomY(dot) - boxTopY(kasra) > 100 &&
          geometry::getDistance(kasra.worldPolys, waw.worldPolys, 1000).contact.depth_or_gap >= 30 - 0.1,
      "Following kasra may retain extra vertical separation to clear Waw while respecting the minimum stack gap");
  require(std::abs(boxCenterX(kasra) - boxCenterX(dot) - 40) < 0.1,
      "Yielding vertically must preserve same-owner horizontal following");
}

void rehWawFallbackChoiceAndParameters() {
  for (const std::string dotName : {"onedotdown", "twodotsdown"}) {
    for (bool oneBelow : {false, true}) {
      for (bool twoBelow : {false, true}) {
        Fixture f;
        f.classes["marks"].insert(dotName);
        const double width = dotName == "onedotdown" ? 120.0 : 230.0;
        auto& reh = f.add("reh.isol", width - 80, -200, 446, 160);
        reh.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{
            {{0,0},{446,0},{446,20},{0,20}}, {{426,0},{446,0},{446,160},{426,160}}});
        buildWorldPolys(reh);
        auto& beh = f.add("behshape.init.beforehah", 0, 0, width + 16, 50);
        // Neither current gap nor left slot fits a mark this tall. Only the
        // below-tip checkbox may request a special target in this fixture.
        auto& dot = f.add(dotName, 60, -140, width, 200);
        auto p = quiet();
        p.rehWaw.oneDotBelowTip = oneBelow;
        p.rehWaw.twoDotsBelowTip = twoBelow;
        p.rehWaw.tipGap = 55;
        p.compliance.rehWawDotTarget = 0.6;
        RehWawMarkPlacement placement(dot, reh, p);
        placement.obstacles = {&reh, &beh};
        placement.prepareDotTarget();
        applyPreferred(placement);
        const bool belowTip = dotName == "onedotdown" ? oneBelow : twoBelow;
        require(placement.dotTargetClear == belowTip && !placement.dotUsesLeftSlot,
            "Configured compliance and fallback must apply to all initial-Beh variants");
        require(belowTip ? boxTopY(dot) <= -200 - p.rehWaw.tipGap + 0.1 : dot.dx == 0 && dot.dy == 0,
            "Each dot type must independently use its own below-tip or current-gap fallback");
        p.toggles.rehWawBehPlacement = true;
        require(chooseMinGap(reh, dot, p) == p.rehWaw.tipGap,
            "Configured tip spacing must also control generic contacts");
      }
    }
  }

  Fixture f;
  f.classes["marks"].insert("onedotdown");
  auto& reh = f.add("reh.isol", 400, -150, 120, 200);
  auto& beh = f.add("behshape.init.beforeheh", 0, 0, 350, 50);
  auto& dot = f.add("onedotdown", 300, -160, 30, 30);
  auto& kasra = f.add("kasra", 300, -240, 60, 20);
  auto& heh = f.add("heh.medi", 0, -200, 150, 200);
  auto p = quiet();
  p.rehWaw.compactGap = 25;
  p.rehWaw.neighborGap = 90;
  p.rehWaw.dotKasraGap = 70;
  p.compliance.rehWawKasraFollow = 0.4;
  RehWawMarkPlacement placement(dot, reh, p), follow(kasra, reh, p);
  placement.obstacles = {&reh, &beh, &heh};
  follow.dots.push_back(&dot);
  placement.kasras.push_back(&follow);
  placement.prepareDotTarget();
  applyPreferred(placement);
  require(placement.dotTargetClear && placement.dotUsesLeftSlot &&
          geometry::getDistance(dot.worldPolys, heh.worldPolys, 1000).contact.depth_or_gap >= 90 - 0.1,
      "The configured neighboring clearance must replace the default 40-unit floor");
  const double x = boxCenterX(dot), y = boxTopY(dot);
  p.rehWaw.oneDotBelowTip = false;
  placement.prepareDotTarget();
  applyPreferred(placement);
  require(placement.dotUsesLeftSlot && std::abs(boxCenterX(dot) - x) < 0.1 && std::abs(boxTopY(dot) - y) < 0.1,
      "Changing the fallback must not change an available left slot");
  applyPreferred(follow);
  require(boxBottomY(dot) - boxTopY(kasra) >= 70 - 0.1,
      "Configured kasra compliance and dot-kasra spacing must replace their defaults");
}

void belowFallbackUsesNearbySideSpaceBeforeDescending() {
  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  f.classes["downdotmarks"].insert("twodotsdown");
  auto& reh = f.add("reh.fina", 100, -200, 200, 250);
  auto& kasra = f.add("kasra", 200, -290, 100, 60);
  auto& beh = f.add("behshape.init", 150, 0, 150, 50);
  auto& dot = f.add("twodotsdown", 150, -100, 100, 60);
  auto p = quiet();
  p.rehWaw.tipGap = 40;
  p.rehWaw.compactGap = 30;
  p.rehWaw.neighborGap = 40;
  RehWawMarkPlacement placement(dot, reh, p);
  placement.obstacles = {&reh, &beh, &kasra};
  placement.prepareDotTarget();
  applyPreferred(placement);
  require(placement.dotTargetClear && !placement.dotUsesCurrentGap && !placement.dotUsesLeftSlot,
      "The fixture must use below-tip fallback after both non-descending slots fail");
  require(boxTopY(dot) >= -241 && boxCenterX(dot) < beh.worldPolys.boundingAABB().minx,
      "An allowed nearby sideways slot must beat descending below the Reh's kasra within a narrow owner band");
  require(geometry::getDistance(dot.worldPolys, kasra.worldPolys, 1000).contact.depth_or_gap >= 40 - 0.1 &&
          geometry::getDistance(dot.worldPolys, reh.worldPolys, 1000).contact.depth_or_gap >= 30 - 0.1 &&
          kasra.dx == 0 && kasra.dy == 0,
      "Sideways fallback must clear Reh and its independent kasra without moving that kasra");
}

void currentGapCannotBypassLeftSlotRules() {
  for (const std::string name : {"onedotdown", "twodotsdown"}) {
    for (double leftGap : {30.0, 70.0}) {
      Fixture f;
      f.classes["marks"].insert(name);
      f.classes["downdotmarks"].insert(name);
      // Only the space wholly LEFT of the vertical reference fits a dot
      // above the tip. That space must not be pinned as current-gap centering.
      auto& reh=f.add("waw.isol",150,-200,120,250);
      auto& beh=f.add("behshape.init",-100,0,360,50);
      auto& dot=f.add(name,80,-100,120,40);
      auto p=quiet();p.toggles.rehWawBehPlacement=true;p.rehWaw.leftGap=leftGap;
      optimizePage(f.page,f.classes,p);
      require(dot.initialPlacement==GlyphInstance::InitialPlacement::LeftOfRehWaw && dot.mobility>0,
          "A wholly left-shifted dot must use movable left placement instead of being pinned as current-gap centering");
      require(std::abs(boxCenterY(dot)-boxBottomY(reh))<1e-9 &&
              reh.worldPolys.boundingAABB().minx-dot.worldPolys.boundingAABB().maxx>=leftGap-1e-9,
          "Left placement must honor tip alignment and the minimum left clearance, including parameter changes");
    }
  }
}

void initialPositioningAndCenteredMobility() {
  std::pair<double,double> initial;
  for (int iterations : {0, 40}) {
    Fixture f;
    f.classes["marks"].insert("onedotdown");
    f.classes["downdotmarks"].insert("onedotdown");
    f.add("waw.fina", 150, -200, 446, 20);
    f.add("behshape.init", 0, 0, 246, 50);
    auto& dot = f.add("onedotdown", 90, -70, 120, 40);
    auto& kasra = f.add("kasra", 90, -400, 120, 20);
    auto p = quiet();
    p.toggles.rehWawBehPlacement = p.toggles.stackOrder = p.toggles.genericGapConstraint = true;
    p.maxIters = iterations;
    p.compliance.rehWawDotTarget = iterations ? 0.9 : 0.0; // legacy field cannot affect this stage
    auto inspectOptions = p;
    inspectOptions.maxIters = 0;
    std::vector<ConstraintViolation> initialViolations;
    optimizePage(f.page, f.classes, inspectOptions, &initialViolations, InitialPositioning::Skip);
    require(dot.dx == 0 && dot.dy == 0 && kasra.dx == 0 && kasra.dy == 0,
        "Input diagnostics and Force-off runs must not perform initial positioning");
    optimizePage(f.page, f.classes, p);
    require(dot.initialPlacement == GlyphInstance::InitialPlacement::CenteredGap && dot.mobility == 0,
        "An accepted centered input dot must be flagged and pinned before XPBD");
    require(dot.dx == dot.initialPositioningDx && dot.dy == dot.initialPositioningDy && dot.baseX == 90 && dot.baseY == -70,
        "XPBD must preserve centered positioning without altering the original HarfBuzz source coordinates");
    if (!iterations) initial = {dot.dx,dot.dy};
    else require(std::abs(dot.dx-initial.first) < 1e-9 && std::abs(dot.dy-initial.second) < 1e-9,
        "Initial dot positioning must be independent of XPBD iteration count and obsolete dot compliance");
    const double offset = boxCenterX(dot)-boxCenterX(kasra);
    SolverContext context(f.page,f.classes);
    StackOrderConstraint stack(dot,kasra,false,p.rehWaw.dotKasraGap,p.compliance.stackOrderGap,
                               p.compliance.rehWawKasraFollow,offset,true);
    stack.keepInnerForGap = true;
    kasra.dx += 12; updateWorldPolys(kasra);
    const double x = kasra.dx;
    stack.project(context,1.0);
    require(std::abs(kasra.dx-x+10) < 1e-9 && dot.dx == initial.first,
        "Ordinary stack alignment must use the adjusted initial offset and move only kasra with 0.2 compliance");
    p.toggles.rehWawBehPlacement = false;
    p.maxIters = 0;
    optimizePage(f.page,f.classes,p);
    require(dot.mobility == 1 && dot.initialPlacement == GlyphInstance::InitialPlacement::None,
        "Centered flags and zero mobility must not persist when the policy is disabled on a later solve");
  }
}

void leftTipAlignmentAndDoubleDotFallback() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");
  f.classes["downdotmarks"].insert("onedotdown");
  auto& waw = f.add("waw.isol",400,-150,120,200);
  // The leftmost tip is at y=-70, while the whole outline bottoms at -150.
  waw.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0,80},{120,0},{120,200}}});
  auto& beh = f.add("behshape.init",0,0,350,50);
  auto& dot = f.add("onedotdown",300,-160,60,30);
  auto p=quiet();p.toggles.rehWawBehPlacement=true;p.rehWaw.leftGap=60;
  optimizePage(f.page,f.classes,p);
  require(dot.initialPlacement == GlyphInstance::InitialPlacement::LeftOfRehWaw && dot.mobility == 1,
      "A left placement must be an initial movable position, not a pinned centered dot");
  require(std::abs(boxCenterY(dot)+70) < 1e-9 &&
          waw.worldPolys.boundingAABB().minx-dot.worldPolys.boundingAABB().maxx >= 60-1e-9,
      "Left dots must align their vertical center with the leftmost tip and keep configured horizontal clearance");
  SolverContext context(f.page,f.classes);
  HorizontalGapConstraint gap(waw,dot,60);
  dot.dx += 500;updateWorldPolys(dot);gap.project(context,1.0);
  require(gap.residual() >= -1e-9 && dot.mobility == 1 && beh.dx == 0,
      "Ordinary directional clearance must protect a movable left dot without a point-attraction constraint");

  Fixture narrow;
  narrow.classes["marks"].insert("twodotsdown");
  narrow.classes["downdotmarks"].insert("twodotsdown");
  auto& reh=narrow.add("reh.isol",40,-100,140,200);
  narrow.add("behshape.init",0,0,120,50);
  auto& two=narrow.add("twodotsdown",60,-140,230,200);
  optimizePage(narrow.page,narrow.classes,p);
  require(two.initialPlacement == GlyphInstance::InitialPlacement::BelowRehWaw && two.mobility == 1 &&
          boxTopY(two) <= boxBottomY(reh)-p.rehWaw.tipGap+1e-9,
      "Two dots lacking a safe current gap or minimum left clearance must initially fall below the whole Reh/Waw outline");
}

void plannedPairsAreValidatedTogether() {
  Fixture f;
  f.classes["marks"].insert("onedotdown");f.classes["downdotmarks"].insert("onedotdown");
  f.add("waw.isol",150,-200,446,20);f.add("behshape.init",0,0,300,50);
  auto& a=f.add("onedotdown",10,-80,20,20);
  f.add("space",700,0);
  f.add("waw.isol",150,-200,446,20);f.add("behshape.init",0,0,300,50);
  auto& b=f.add("onedotdown",280,-80,20,20);
  auto p=quiet();p.toggles.rehWawBehPlacement=true;
  optimizePage(f.page,f.classes,p);
  require(a.mobility+b.mobility >= 1 &&
          !geometry::getDistance(a.worldPolys,b.worldPolys,10).contact.intersect,
      "Independently planned pairs must not both be pinned into the same space; conflicting plans must yield to XPBD");
}

void fixedDotsRespectWaqfContactMobility() {
  Fixture f;
  auto& dot=f.add("fatha",0,100,30,30);dot.mobility=0;
  auto& waqf=f.add("waqf.meem",10,100,30,30);
  SolverContext context(f.page,f.classes);
  const auto forward=gapMobilities(context,dot,waqf),reverse=gapMobilities(context,waqf,dot);
  require(forward.first==0 && reverse.second==0 && forward.second>0 && reverse.first>0,
      "Waqf contact bias must honor zero mobility in either pair order");
  double penetration=0;OptParams p;
  solveGapConstraint(context,dot,waqf,p,1.0,penetration);
  require(dot.dx==0 && dot.dy==0 && (waqf.dx!=0 || waqf.dy!=0),
      "A Waqf contact must move the movable mark rather than a pinned centered dot");
}

void contextualLowerMarksYieldToNextLine() {
  Fixture f;
  f.classes["marks"].insert("twodotsdown");
  f.classes["downdotmarks"].insert("twodotsdown");
  auto& reh = f.add("reh.isol", 150, -150, 120, 200);
  auto& beh = f.add("behshape.init", 0, 0, 60, 50);
  auto& dot = f.add("twodotsdown", 30, -100, 100, 30);
  auto& nextBase = f.add("base", 0, -600, 400, 50);
  nextBase.lineIndex = 1; nextBase.lineY = -600;
  auto& upper = f.add("fatha", 0, -300, 400, 100);
  upper.lineIndex = 1; upper.lineY = -600;
  auto p = quiet();
  RehWawMarkPlacement placement(dot, reh, p);
  placement.obstacles = {&reh, &beh, &nextBase, &upper};
  placement.prepareDotTarget();
  require(!placement.dotTargetClear,
      "A blocked lower slot must not escape across the next line to find room");
  upper.dy = -1000; updateWorldPolys(upper);
  placement.prepareDotTarget();
  require(!placement.dotTargetClear,
      "Moving the next line's ink away must not manufacture space for a lower dot");

  auto& waqf = f.add("waqf.meem", 30, -100, 100, 40);
  waqf.lineIndex = 1; waqf.lineY = -600;
  SolverContext context(f.page, f.classes);
  context.rehWawBehPlacement = true;
  context.contextualBelowMarks.insert(&dot);
  const auto forward = gapMobilities(context, dot, waqf);
  const auto reverse = gapMobilities(context, waqf, dot);
  require(forward.first == dot.mobility && forward.second == 0 &&
          reverse.first == 0 && reverse.second == dot.mobility,
      "A contextual lower mark must resolve cross-line contact without displacing Waqf");
  double penetration = 0;
  solveGapConstraint(context, dot, waqf, p, 1, penetration);
  require(waqf.dx == 0 && waqf.dy == 0 && (dot.dx != 0 || dot.dy != 0),
      "Cross-line separation must move the lower dot, leaving the next line's Waqf intact");
  context.rehWawBehPlacement = false;
  require(gapMobilities(context, dot, waqf).second == 3.0,
      "Disabled placement must retain the original Waqf contact policy");
}

}  // namespace

int main() {
  try {
    intentionalMarkHullPolicy();
    independentSideAndOwnerChecks();
    ownershipAndReportContext();
    worldBoundsAndAppliedOffsets();
    stretchedFathaJoinedHorizontalGap();
    gapReportingAndKeys();
    completeConstraintCoverageAndConvergence();
    stretchedMarksRetainCollisionSafety();
    stretchedFathaBandRespectsAuthoredHeight();
    fontAuthoredBaselineClearance();
    lowerDotKasraStack();
    rehWawLowerMarkPlacement();
    persistentDotInkEscape();
    initialBehOptInAndWideSlot();
    conditionalDotSlotAndKasraFollow();
    belowFallbackUsesNearbySideSpaceBeforeDescending();
    currentGapCannotBypassLeftSlotRules();
    initialPositioningAndCenteredMobility();
    leftTipAlignmentAndDoubleDotFallback();
    plannedPairsAreValidatedTogether();
    fixedDotsRespectWaqfContactMobility();
    contextualLowerMarksYieldToNextLine();
    dotSlotMustFitFollowingKasra();
    dotNeighborClearance();
    followingKasraCanUseClearBowlInterior();
    limitedLeftSlotOverhang();
    clearCurrentGapHasPriority();
    currentGapMustFitFollowingKasra();
    currentGapUsesAvailableClearance();
    kasraCanYieldBelowPreferredGap();
    rehWawFallbackChoiceAndParameters();
    std::cout << "XPBD placement safety regressions passed\n";
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
