#include <iostream>
#include <stdexcept>
#include "digitalkhatt/layout/TopMarkStack.h"
#include "digitalkhatt/layout/OptimizeLayout.h"

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
}  // namespace
int main() {
  try { semanticAdjacency(); unclassifiedHamzaBlocksNonAdjacentLink(); }
  catch (const std::exception& error) { std::cerr<<error.what()<<'\n'; return 1; }
  return 0;
}
