#include <iostream>
#include <stdexcept>
#include "digitalkhatt/layout/TopMarkStack.h"
#include "digitalkhatt/layout/OptimizeLayout.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"
#include "digitalkhatt/layout/SolverContext.h"

using namespace digitalkhatt::layout;

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct Fixture {
  std::vector<GlyphInstance> marks;
  std::vector<GlyphInstance*> group;
  Fixture(std::initializer_list<const char*> names) : marks(names.size()) {
    int i=0;
    for (const char* name : names) {
      auto& mark=marks[i]; mark.glyphIndex=i++; mark.glyphName=name;
      group.push_back(&mark);
    }
  }
  void expect(std::initializer_list<std::pair<int,int>> parents) {
    const auto links=buildTopMarkStackLinks(group);
    require(links.size()==parents.size(), "Unexpected number of stack links");
    size_t i=0;
    for (const auto& [inner,outer] : parents) {
      require(links[i].inner==&marks[inner] && links[i].outer==&marks[outer], "Wrong semantic parent");
      ++i;
    }
  }
};
void semanticAdjacency() {
  Fixture hamza{"onedotup","hamzaabove.joined","damma"};
  hamza.expect({{1,2}});
  // Invalid initial heights and horizontal positions must not change ownership.
  hamza.marks[0].baseY=10000; hamza.marks[1].baseY=-10000;
  hamza.marks[2].baseX=10000; hamza.marks[2].baseY=5000;
  hamza.expect({{1,2}});
  Fixture{"twodotsup","shadda","fatha"}.expect({{0,1},{1,2}});
  Fixture{"twodotsup","shadda","fatha.stretched"}.expect({{0,1},{1,2}});
  Fixture{"onedotup","fatha.stretched"}.expect({{0,1}});
  Fixture{"twodotsup","shadda","hamzaabove.joined","damma"}.expect({{0,1},{2,3}});
  Fixture{"onedotup","hamzaabove","shadda","fatha"}.expect({{2,3}});
  Fixture{"onedotup","fatha","hamzaabove.joined","damma"}.expect({{2,3},{0,1}});
  Fixture{"shadda","fatha","hamzaabove.lamalef","fatha"}.expect({{0,1},{2,3}});
  Fixture{"smallalef.joined","maddahabove"}.expect({{0,1}});
  Fixture{"onedotup","unrelated","dammatanidgham"}.expect({});
  Fixture{"fatha","shadda"}.expect({}); // A later glyph cannot own a preceding mark.
  Fixture{"damma","waqf.sad"}.expect({});
  Fixture{"onedotup","onedotup","damma","dammatan"}.expect({{1,2}});
  Fixture{"shadda","fatha","damma"}.expect({{0,1}});
  Fixture{"onedotup","shadda","smallalef.joined","fatha"}.expect({{0,1}});
  Fixture{"smallalef.joined","fatha","maddahabove"}.expect({});
  Fixture filtered{"shadda","kasra","fatha"};
  filtered.group.erase(filtered.group.begin()+1); // Outside topmarks, still intervening.
  filtered.expect({});
  Fixture hiddenHamza{"shadda","hamzaabove.lamalef","fatha"};
  hiddenHamza.group.erase(hiddenHamza.group.begin()+1);
  hiddenHamza.expect({});
  Fixture hiddenBase{"shadda","alef.isol","fatha"};
  hiddenBase.group.erase(hiddenBase.group.begin()+1);
  hiddenBase.expect({});
  Fixture differentLines{"shadda","fatha"};
  differentLines.marks[1].lineIndex=1;
  differentLines.expect({});
  const auto links=buildTopMarkStackLinks(hamza.group);
  require(links[0].offsetX==30 && links[0].minGap==0, "Authored hamza rule changed");
  // Caller order is irrelevant; semantic order comes from glyphIndex.
  std::swap(hamza.group[0],hamza.group[2]);
  hamza.expect({{1,2}});
}
void unclassifiedHamzaBlocksNonAdjacentLink() {
  std::vector<std::vector<GlyphInstance>> page(1);
  auto& row=page[0]; row.resize(5);
  const char* names[]{"lam.init.lamalef","shadda","fatha","hamzaabove.lamalef","fatha"};
  for (int i=0;i<5;++i) {
    auto& g=row[i]; g.glyphName=names[i]; g.glyphIndex=i;
    g.isMark=i>0; g.prevBase=i>0?&row[0]:nullptr;
    g.geomScaled=geometry::GeometrySet(std::vector<geometry::Poly>{
        {{0,0},{20,0},{20,20},{0,20}}});
  }
  ClassMap classes{{"topmarks",{"shadda","fatha"}}}; // Hamza intentionally outside topmarks.
  OptParams params; params.maxIters=0; params.minViolationSeverity=0;
  params.toggles.ylane=false; params.toggles.waqfPlacement=false;
  params.toggles.waqfEscape=false; params.toggles.hardStayAboveBelow=false;
  params.toggles.baseVicinity=false; params.toggles.horizontalOrder=false;
  params.toggles.matchMarkPosition=false; params.toggles.bowlCluster=false;
  params.toggles.genericGapConstraint=false;
  std::vector<ConstraintViolation> violations;
  optimizePage(page,classes,params,&violations);
  int stacks=0;
  for (const auto& v:violations) if (v.type==ViolationType::StackOrderGap) {
    ++stacks;
    require(v.glyphA==1 && v.glyphB==2, "Stack skipped an intervening hamza");
  }
  require(stacks==1, "Expected one authored shadda/fatha stack");
}

void stretchedFathaPreservesGposStackOffset() {
  for (const auto* parent : {"onedotup", "shadda"}) {
    for (const auto* name : {"fatha", "fatha.stretched"}) {
      Fixture stack{parent, name};
      auto& inner = stack.marks[0];
      auto& outer = stack.marks[1];
      for (auto& mark : stack.marks) {
        mark.mobility = 1;
        mark.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {20, 0}, {20, 20}, {0, 20}}});
      }
      // Keep real horizontal ink overlap while the enlarged mark's center is
      // far to the left of its GPOS parent.
      outer.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{-200,0},{220,0},{220,20},{-200,20}}});
      outer.baseX = -200;
      outer.baseY = 60;
      inner.baseY = 60;
      outer.preserveGposHorizontal = true;
      buildWorldPolys(inner);
      buildWorldPolys(outer);
      auto links = buildTopMarkStackLinks(stack.group);
      require(links.size() == 1 && links[0].offsetX == 200, "Stretched Fatha must retain its GPOS stack offset");
      std::vector<std::vector<GlyphInstance>> page;
      const ClassMap classes{{"topdotmarks", {"onedotup"}}};
      SolverContext context(page, classes);
      StackOrderConstraint constraint(inner, outer, true, links[0].minGap, 0, 0, links[0].offsetX);
      constraint.project(context, 1);
      require(inner.dx == 0 && outer.dx == 0, "Vertical clearance must not recenter stretched Fatha");
      require(boxBottomY(outer) >= boxTopY(inner) + links[0].minGap, "Vertical stacking clearance must remain active");
      outer.dx = 30;
      buildWorldPolys(outer);
      constraint.project(context, 1);
      require(std::abs(boxCenterX(inner) - boxCenterX(outer) - links[0].offsetX) < 1e-9, "The stack target follows the GPOS offset, not arbitrary later displacement");
      outer.preserveGposHorizontal = false;
      links = buildTopMarkStackLinks(stack.group);
      require(links[0].offsetX == (std::string_view(parent) == "shadda" ? 0 : 30), "Natural Fatha must retain its existing authored stack target");
    }
  }
}

void stretchedFathaUsesLocalOutlineClearance() {
  std::vector<std::vector<GlyphInstance>> page;
  const ClassMap classes{{"topdotmarks", {"onedotup"}}};
  SolverContext context(page, classes);
  for (const auto* parent : {"onedotup", "shadda"}) {
    for (const bool above : {true, false}) {
      Fixture stack{parent, "fatha.stretched"};
      auto& inner = stack.marks[0];
      auto& outer = stack.marks[1];
      inner.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{80, 0}, {100, 0}, {100, 20}, {80, 20}}});
      outer.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {100, 100}, {100, 110}, {0, 10}}});
      if (!above) {
        inner.geomScaled = inner.geomScaled.flipY();
        outer.geomScaled = outer.geomScaled.flipY();
      }
      inner.mobility = outer.mobility = 1;
      outer.preserveGposHorizontal = true;
      buildWorldPolys(inner);
      buildWorldPolys(outer);
      const double offsetX = boxCenterX(inner) - boxCenterX(outer);
      StackOrderConstraint constraint(inner, outer, above, 10, 0, 0, offsetX);
      std::vector<ConstraintViolation> violations;
      constraint.reportViolations(context, violations);
      require(violations.empty(), "A distant Fatha tail must not create a false stack-gap report");
      constraint.project(context, 1);
      require(inner.dy == 0 && outer.dy == 0 && inner.dx == 0 && outer.dx == 0, "Clear local ink must retain its authored GPOS placement");

      outer.dy = above ? -75 : 75;
      buildWorldPolys(outer);
      constraint.reportViolations(context, violations);
      require(violations.size() == 1 && std::abs(violations[0].severity - 25) < 1e-9, "Actual local overlap must still be detected after vertical translation");
      require(violations[0].marker[0].x == 80 && violations[0].marker[1].x == 80, "Diagnostics must identify the limiting contour, not the bounding-box midpoint");
      constraint.project(context, 1);
      violations.clear();
      constraint.reportViolations(context, violations);
      require(violations.empty(), "Projection must clear actual local overlap");
      require(inner.dx == 0 && outer.dx == 0, "Ink-gap projection must not recenter the stretched Fatha");
      const double solvedInnerY = inner.dy, solvedOuterY = outer.dy;
      constraint.project(context, 1);
      require(inner.dy == solvedInnerY && outer.dy == solvedOuterY, "Cached outline gap must track vertical projections without repeated displacement");

      inner.dx += 80;
      outer.dx += 80;
      inner.dy += 30;
      outer.dy += 30;
      buildWorldPolys(inner);
      buildWorldPolys(outer);
      constraint.reportViolations(context, violations);
      require(violations.empty(), "Common translation must preserve cached outline clearance");
      outer.dx += 80;
      buildWorldPolys(outer);
      constraint.reportViolations(context, violations);
      bool foundLocalGap = false;
      for (const auto& v : violations)
        if (v.type == ViolationType::StackOrderGap) foundLocalGap |= std::abs(v.severity - 80) < 1e-9;
      require(foundLocalGap, "Relative X changes must invalidate the outline cache");
    }
  }
}

void disconnectedInkDoesNotInventStackObstacle() {
  std::vector<std::vector<GlyphInstance>> page;
  const ClassMap classes;
  SolverContext context(page, classes);
  Fixture stack{"twodotsup", "fatha.stretched"};
  auto& inner = stack.marks[0];
  auto& outer = stack.marks[1];
  inner.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {10, 0}, {10, 10}, {0, 10}}, {{90, 0}, {100, 0}, {100, 10}, {90, 10}}});
  // Low ink exists only in the empty space between the two dots.
  outer.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 30}, {45, 30}, {50, -20}, {55, 30}, {100, 30}, {100, 40}, {0, 40}}});
  inner.mobility = outer.mobility = 1;
  outer.preserveGposHorizontal = true;
  buildWorldPolys(inner);
  buildWorldPolys(outer);
  StackOrderConstraint constraint(inner, outer, true, 10, 0, 0);
  constraint.project(context, 1);
  require(inner.dy == 0 && outer.dy == 0, "Disconnected dots must not be treated as a solid bounding-box obstacle");
  std::vector<ConstraintViolation> violations;
  constraint.reportViolations(context, violations);
  require(violations.empty(), "Disconnected clear ink must not report a stack gap");

  outer.baseX = 200;
  buildWorldPolys(outer);
  StackOrderConstraint noOverlap(inner, outer, true, 10, 0, 0, boxCenterX(inner) - boxCenterX(outer));
  noOverlap.project(context, 1);
  noOverlap.reportViolations(context, violations);
  require(inner.dy == 0 && outer.dy == 0 && violations.empty(), "No shared horizontal ink needs no vertical stacking correction");

  outer.baseX = 0;
  outer.preserveGposHorizontal = false;
  buildWorldPolys(outer);
  StackOrderConstraint ordinary(inner, outer, true, 10, 0, 0);
  ordinary.project(context, 1);
  require(boxBottomY(outer) >= boxTopY(inner) + 10, "Ordinary marks must retain their existing bounding-box stack behavior");
}

void fathaShaddaOutlineClearanceIsIndependentOfAlignment() {
  std::vector<std::vector<GlyphInstance>> page;
  const ClassMap classes;
  SolverContext context(page, classes);
  for (const auto* name : {"fatha", "fatha.stretched"}) {
    Fixture stack{"shadda", name};
    auto& inner = stack.marks[0];
    auto& outer = stack.marks[1];
    inner.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{40, 0}, {60, 0}, {60, 20}, {40, 20}}});
    outer.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {100, 100}, {100, 110}, {0, 10}}});
    inner.mobility = outer.mobility = 1;
    outer.baseY = 40;
    outer.preserveGposHorizontal = std::string_view(name) == "fatha.stretched";
    buildWorldPolys(inner);
    buildWorldPolys(outer);
    const auto links = buildTopMarkStackLinks(stack.group);
    require(links.size() == 1 && links[0].outlineClearance && links[0].minGap == 60, "Both Fatha identities above Shadda must use the same authored outline clearance");
    StackOrderConstraint constraint(inner, outer, true, links[0].minGap, 0, 0, links[0].offsetX, links[0].outlineClearance);
    std::vector<ConstraintViolation> violations;
    constraint.reportViolations(context, violations);
    require(violations.empty(), "A clear slanted Fatha must not report the unrelated bottom of its bounding box");
    constraint.project(context, 1);
    require(inner.dy == 0 && outer.dy == 0, "A 60-unit outline gap must not be enlarged to a 60-unit bounding-box gap");

    outer.dy = -30;
    buildWorldPolys(outer);
    constraint.reportViolations(context, violations);
    require(violations.size() == 1 && std::abs(violations[0].severity - 30) < 1e-9, "Both identities must diagnose a real outline-clearance deficit");
    constraint.project(context, 1);
    violations.clear();
    constraint.reportViolations(context, violations);
    require(violations.empty(), "Both identities must project a real gap deficit using the same outline measure");
  }

  Fixture natural{"shadda", "fatha"};
  auto& inner = natural.marks[0];
  auto& outer = natural.marks[1];
  for (auto& mark : natural.marks) {
    mark.mobility = 1;
    mark.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {20, 0}, {20, 20}, {0, 20}}});
  }
  outer.baseX = 10;
  outer.baseY = 100;
  buildWorldPolys(inner);
  buildWorldPolys(outer);
  const auto links = buildTopMarkStackLinks(natural.group);
  require(links[0].outlineClearance && links[0].offsetX == 0 && !outer.preserveGposHorizontal, "Ordinary Fatha must retain centering without opting into GPOS preservation");
  StackOrderConstraint centered(inner, outer, true, links[0].minGap, 0, 0, links[0].offsetX, links[0].outlineClearance);
  centered.project(context, 1);
  require(std::abs(boxCenterX(inner) - boxCenterX(outer)) < 1e-9 && inner.dy == 0 && outer.dy == 0, "Outline clearance must not change ordinary Fatha's horizontal centering");

  Fixture ordinaryDots{"onedotup", "fatha"};
  require(!buildTopMarkStackLinks(ordinaryDots.group)[0].outlineClearance, "The new authored exception must stay scoped to Fatha above Shadda");
}

void stretchedFathaPreferredOutlineClearance() {
  std::vector<std::vector<GlyphInstance>> page;
  const ClassMap classes;
  SolverContext context(page, classes);
  for (bool above : {true, false}) {
    for (double initialGap : {30.0, 60.0, 65.0, 150.0}) {
      Fixture stack{"shadda", "fatha.stretched"};
      auto& inner = stack.marks[0];
      auto& outer = stack.marks[1];
      inner.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{80, 0}, {100, 0}, {100, 20}, {80, 20}}});
      outer.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {100, 100}, {100, 110}, {0, 10}}});
      outer.baseY = initialGap - 60;
      if (!above) {
        inner.geomScaled = inner.geomScaled.flipY();
        outer.geomScaled = outer.geomScaled.flipY();
        outer.baseY = -outer.baseY;
      }
      inner.mobility = outer.mobility = 1;
      outer.preserveGposHorizontal = true;
      buildWorldPolys(inner);
      buildWorldPolys(outer);
      const auto links = buildTopMarkStackLinks(stack.group);
      require(links.size() == 1 && links[0].preferredGap == 80 && links[0].minGap == 60 && links[0].outlineClearance, "Only stretched Fatha above Shadda must author a preferred 80-unit outline gap");
      StackOrderConstraint constraint(inner, outer, above, links[0].minGap, 0, 0, links[0].offsetX, links[0].outlineClearance, links[0].preferredGap);
      constraint.project(context, 1);
      const double expectedMove = (80 - initialGap) / 1.1;
      require(std::abs(outer.dy - (above ? expectedMove : -expectedMove)) < 1e-9, "Preferred gap must open short gaps and close excessive gaps using the local contour");
      require(inner.dy == 0 && inner.dx == 0 && outer.dx == 0, "Preferred gap must preserve Shadda and the GPOS horizontal offset");
      const double solvedY = outer.dy;
      constraint.project(context, 1);
      require(std::abs(outer.dy - solvedY) < 1e-9, "Repeated projection must not accumulate the preferred gap correction");
      std::vector<ConstraintViolation> violations;
      constraint.reportViolations(context, violations);
      require(violations.size() == 1 && violations[0].kind == ViolationKind::Soft && violations[0].detail == "Stack preferred outline clearance", "Preference slack must remain a soft diagnostic, not a collision finding");
    }
  }

  for (const auto& pair : {std::pair{"shadda", "fatha"}, std::pair{"onedotup", "fatha.stretched"}, std::pair{"hamzaabove.joined", "fatha.stretched"}}) {
    Fixture stack{pair.first, pair.second};
    require(!buildTopMarkStackLinks(stack.group)[0].preferredGap, "Ordinary Fatha, dots and joined marks must not gain the Shadda preference");
  }

  Fixture disjoint{"shadda", "fatha.stretched"};
  auto& inner = disjoint.marks[0];
  auto& outer = disjoint.marks[1];
  for (auto& mark : disjoint.marks) {
    mark.mobility = 1;
    mark.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {20, 0}, {20, 20}, {0, 20}}});
  }
  outer.baseX = 100;
  outer.preserveGposHorizontal = true;
  buildWorldPolys(inner);
  buildWorldPolys(outer);
  StackOrderConstraint noOverlap(inner, outer, true, 60, 0, 0, -100, true, 80);
  noOverlap.project(context, 1);
  require(inner.dy == 0 && outer.dy == 0, "A preferred gap must not invent a vertical obstacle without shared X");
  outer.baseX = 0;
  outer.baseY = 40;
  buildWorldPolys(outer);
  StackOrderConstraint belowMinimum(inner, outer, true, 60, 0, 0, 0, true, 10);
  belowMinimum.project(context, 1);
  require(boxBottomY(outer) - boxTopY(inner) >= 60, "Preference must never override the minimum safety gap");
}

void optimizerWiresPreferredStackGap() {
  for (double initialGap : {60.0, 120.0}) {
    std::vector<std::vector<GlyphInstance>> page(1);
    auto& row = page[0];
    row.resize(3);
    const char* names[]{"behshape.init", "shadda", "fatha.stretched"};
    for (int i = 0; i < 3; ++i) {
      auto& g = row[i];
      g.glyphName = names[i];
      g.glyphIndex = i;
      g.isMark = i > 0;
      g.prevBase = i > 0 ? &row[0] : nullptr;
      g.geomScaled = geometry::GeometrySet(std::vector<geometry::Poly>{{{0, 0}, {20, 0}, {20, 20}, {0, 20}}});
    }
    row[1].baseY = 400;
    row[2].baseY = 420 + initialGap;
    row[2].preserveGposHorizontal = true;
    ClassMap classes{{"topmarks", {"shadda", "fatha.stretched"}}};
    OptParams params;
    params.toggles.ylane = false;
    params.toggles.waqfPlacement = false;
    params.toggles.waqfEscape = false;
    params.toggles.hardStayAboveBelow = false;
    params.toggles.baseVicinity = false;
    params.toggles.horizontalOrder = false;
    params.toggles.matchMarkPosition = false;
    params.toggles.bowlCluster = false;
    params.toggles.genericGapConstraint = false;
    std::vector<ConstraintViolation> violations;
    optimizePage(page, classes, params, &violations);
    const double gap = boxBottomY(row[2]) - boxTopY(row[1]);
    require(std::abs(gap - 80) < 4 && row[1].dy == 0 && row[2].dx == 0, "Shared optimizer must forward the authored preferred gap and preserve its reference");
    require(violations.empty(), "Soft preference residual must not leak into normal collision reports");
  }
}
}  // namespace
int main() {
  try {
    semanticAdjacency();
    unclassifiedHamzaBlocksNonAdjacentLink();
    stretchedFathaPreservesGposStackOffset();
    stretchedFathaUsesLocalOutlineClearance();
    disconnectedInkDoesNotInventStackObstacle();
    fathaShaddaOutlineClearanceIsIndependentOfAlignment();
    stretchedFathaPreferredOutlineClearance();
    optimizerWiresPreferredStackGap();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
