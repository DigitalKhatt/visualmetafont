#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphCollisionGeometry.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/OptimizeLayout.h"
#include "digitalkhatt/layout/PlacementAudit.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/ViolationReportContext.h"
#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"

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

OptParams quiet() {
  OptParams p;
  p.maxIters = 0;
  auto& t = p.toggles;
  t.ylane = t.waqfPlacement = t.hardStayAboveBelow = t.stackOrder = false;
  t.baseVicinity = t.horizontalOrder = t.matchMarkPosition = t.genericGapConstraint = false;
  t.reportGenericGap = false;
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
  p.toggles.reportGenericGap = OptParams{}.toggles.reportGenericGap;
  v.clear();
  optimizePage(clearance.page, clearance.classes, p, &v);
  require(contains(v, ViolationType::GenericGap), "Default report must include positive clearance violations, even with gap forces off");
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
}  // namespace

int main() {
  try {
    intentionalMarkHullPolicy();
    independentSideAndOwnerChecks();
    ownershipAndReportContext();
    worldBoundsAndAppliedOffsets();
    gapReportingAndKeys();
    completeConstraintCoverageAndConvergence();
    std::cout << "XPBD placement safety regressions passed\n";
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
