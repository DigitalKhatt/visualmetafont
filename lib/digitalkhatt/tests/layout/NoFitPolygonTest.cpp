#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>
#include "digitalkhatt/geometry/NoFitPolygon.h"
#include "digitalkhatt/layout/GlyphInstance.h"
using namespace geometry;
namespace {
void require(bool value,const char* message) {if (!value) throw std::runtime_error(message);}
GeometrySet rectangle(double w,double h) {return GeometrySet(PolySet{{{0,0},{w,0},{w,h},{0,h}}});}
void sharedConvexContacts() {
  const Poly concave{{0,0},{20,0},{20,4},{4,4},{4,20},{0,20}};
  require(!isConvexPolygon(concave), "Concave outline incorrectly selected convex contacts");
  require(!isConvexPolygon({{0,0},{1,0},{2,0}}), "Degenerate polygon selected convex contacts");
  require(!isConvexPolygon({{0,0},{1,0},{1,0},{1,1},{0,1}}), "Repeated edge selected convex contacts");
  Poly polygon{{0,0},{5,0},{10,0},{10,10},{0,10},{0,0}};
  require(isConvexPolygon(polygon), "Convex polygon with collinear/closing vertices rejected");
  std::reverse(polygon.begin(),polygon.end());
  require(isConvexPolygon(polygon), "Clockwise convex polygon rejected");
  NoFitPolygonCache cache;
  for (const Vec2 offset : {Vec2{3,2},Vec2{13,0},Vec2{-13,2},Vec2{30,30}}) {
    const auto a=rectangle(10,10).translated(-9348.123,-19374.456);
    const auto b=rectangle(10,10).translated(-9348.123+offset.x,-19374.456+offset.y);
    const auto expected=getDistance(a,b,5).contact;
    const auto actual=cache.contactConvex(a,b,5).contact;
    require(actual.depth_or_gap==expected.depth_or_gap && actual.normal==expected.normal &&
            actual.intersect==expected.intersect && actual.pA==expected.pA && actual.pB==expected.pB,
            "Convex fast path differs from default GJK/EPA");
  }
  require(cache.statistics().convexGjkQueries==4 && cache.statistics().builds==0,
          "Convex fast path constructed NFP regions");
}

// Independent quadratic reference for the new linear edge merge. Test-only:
// it builds the hull of all B-A vertex differences and measures its boundary.
Poly referenceHull(Poly points) {
  std::sort(points.begin(),points.end(),[](Vec2 a,Vec2 b){return a.x<b.x || (a.x==b.x && a.y<b.y);});
  points.erase(std::unique(points.begin(),points.end()),points.end());
  Poly hull;
  for (auto p:points) {
    while (hull.size()>1 && crossz(hull.back()-hull[hull.size()-2],p-hull.back())<=0) hull.pop_back();
    hull.push_back(p);
  }
  const auto lower=hull.size();
  for (auto i=points.rbegin()+1;i!=points.rend();++i) {
    while (hull.size()>lower && crossz(hull.back()-hull[hull.size()-2],*i-hull.back())<=0) hull.pop_back();
    hull.push_back(*i);
  }
  hull.pop_back();return hull;
}
void linearMinkowskiMatchesReference() {
  std::mt19937 random(565);
  std::uniform_int_distribution<int> coordinate(-200,200),translation(-250,250);
  for (int trial=0;trial<500;++trial) {
    auto makePolygon=[&] {
      Poly points;
      for (int i=0;i<12;++i) points.emplace_back(coordinate(random),coordinate(random));
      return referenceHull(std::move(points));
    };
    const auto a=makePolygon(),b=makePolygon();
    Poly differences;
    for (auto pa:a) for (auto pb:b) differences.push_back(pb-pa);
    const auto region=referenceHull(std::move(differences));
    const Vec2 t{double(translation(random)),double(translation(random))};
    bool inside=true; double distanceSquared=INFINITY;
    for (std::size_t i=0;i<region.size();++i) {
      const auto p=region[i],edge=region[(i+1)%region.size()]-p;
      inside &= crossz(edge,t-p)>=0;
      const auto q=p+edge*std::clamp(dot(t-p,edge)/norm2(edge),0.0,1.0);
      distanceSquared=std::min(distanceSquared,norm2(t-q));
    }
    NoFitPolygonCache cache;
    const auto idA=cache.registerShape(GeometrySet(PolySet{a}));
    const auto idB=cache.registerShape(GeometrySet(PolySet{b}));
    const auto actual=cache.contact(idA,t,idB,{0,0},0).contact;
    if (!std::isfinite(actual.depth_or_gap)) {
      require(!inside, "Linear Minkowski missed an intersecting pair");
      continue;
    }
    const double expected=(inside?-1.0:1.0)*std::sqrt(distanceSquared);
    require(std::abs(actual.depth_or_gap-expected)<1e-7,
            "Linear Minkowski differs from quadratic hull reference");
    if (distanceSquared>1e-12) require(actual.intersect==inside,"Linear Minkowski containment differs");
  }
}
void unsplitOutlineInput() {
  const Poly outline{{0,0},{20,0},{20,4},{4,4},{4,20},{0,20}};
  ContourCubic contour;
  for (std::size_t i=0; i<outline.size(); ++i) {
    const auto a=outline[i],b=outline[(i+1)%outline.size()];
    contour.segs.push_back({a,a,b,b});
  }
  const GlyphCubic cubics{{contour}};
  digitalkhatt::layout::GlyphInstance base;
  base.geomScaled=buildConvexPartsFromCubics(cubics,0.1).scaled(1.5,2.0);
  base.noFitGeometry=buildPolyFromCubics(cubics,0.1).scaled(1.5,2.0);
  require(base.geomScaled.size()>1 && base.noFitLocalGeometry().size()==1,
          "NFP base input must retain its unsplit contour independently of solver parts");
  NoFitPolygonCache cache;
  const auto a=cache.registerShape(rectangle(2,2));
  const auto b=cache.registerShape(base.noFitLocalGeometry());
  require(!cache.contact(a,{12,16},b,{0,0},0).contact.intersect,
          "Unsplit NFP base filled the concave pocket");
  const auto boxes=base.geomScaled.boundingAABB();
  const auto bounds=base.noFitLocalGeometry().boundingAABB();
  require(boxes.maxx==bounds.maxx && boxes.maxy==bounds.maxy,
          "NFP outline scaling differs from ordinary solver scaling");
  base.noFitGeometry.reset();
  require(&base.noFitLocalGeometry()==&base.geomScaled,
          "Direct polygon callers must still support their supplied geometry");
}
void translationAndSwap() {
  NoFitPolygonCache cache;
  const auto a=cache.registerShape(rectangle(10,10)),b=cache.registerShape(rectangle(20,20));
  auto c=cache.contact(a,{15,5},b,{0,0},0).contact;
  require(c.intersect && std::abs(c.depth_or_gap+5)<1e-6,"Wrong rectangle penetration");
  require(c.normal.x < -.99,"Wrong separation normal");
  const auto moved=cache.contact(a,{115,205},b,{100,200},0).contact;
  require(std::abs(c.depth_or_gap-moved.depth_or_gap)<1e-6,"Common translation changed contact");
  const auto reversed=cache.contact(b,{0,0},a,{15,5},0).contact;
  require(std::abs(c.depth_or_gap-reversed.depth_or_gap)<1e-6 && c.normal.x*reversed.normal.x<-.99,
          "Reversing pair changed separation");
  const auto before=cache.statistics();
  cache.contact(a,{16,5},b,{0,0},0);
  require(cache.statistics().builds==before.builds,"Translation rebuilt no-fit region");
  require(cache.registerShape(rectangle(10,10))==a,"Identical shape not interned");
  require(cache.registerShape(rectangle(11,10))!=a,"Outline change reused stale shape");
}
void wholeUnionAndClearance() {
  NoFitPolygonCache cache;
  auto a=cache.registerShape(rectangle(2,2));
  auto b=cache.registerShape(GeometrySet(PolySet{
    {{0,0},{8,0},{8,10},{0,10}},{{6,0},{14,0},{14,10},{6,10}}}));
  const Vec2 t{6,4};
  const auto contact=cache.contact(a,t,b,{0,0},0).contact;
  require(contact.intersect && std::abs(contact.depth_or_gap+6)<1e-6,
          "Used internal piece boundary instead of complete union");
  const auto separated=t-contact.normal*(-contact.depth_or_gap+.01);
  require(!cache.contact(a,separated,b,{0,0},0).contact.intersect,"Whole-pair correction left another piece overlapping");
  const auto square=cache.registerShape(rectangle(10,10));
  const auto gap=cache.contact(square,{13,0},square,{0,0},5).contact;
  require(!gap.intersect && std::abs(gap.depth_or_gap-3)<1e-6,"Expanded clearance has an extra margin");
  const auto clear=Vec2{13,0}-gap.normal*(5-gap.depth_or_gap+.1);
  require(cache.contact(square,clear,square,{0,0},5).contact.depth_or_gap >= 5,"Correction did not clear requested gap");
}
void concavityAndHoles() {
  NoFitPolygonCache cache;
  auto a=cache.registerShape(rectangle(2,2));
  auto b=cache.registerShape(GeometrySet(PolySet{{{0,0},{20,0},{20,4},{4,4},{4,20},{0,20}}}));
  require(!cache.contact(a,{8,8},b,{0,0},0).contact.intersect,"Concave pocket filled by a hull");
  auto frame=cache.registerShape(GeometrySet(PolySet{
    {{0,0},{20,0},{20,2},{0,2}},{{0,18},{20,18},{20,20},{0,20}},
    {{0,2},{2,2},{2,18},{0,18}},{{18,2},{20,2},{20,18},{18,18}}}));
  require(!cache.contact(a,{8,8},frame,{0,0},0).contact.intersect,"Union hole was discarded");
  require(cache.contact(a,{1,8},frame,{0,0},0).contact.intersect,"Union outer frame missing");
  const auto closedPocket=cache.contact(a,{8,8},frame,{0,0},10).contact;
  require(!closedPocket.intersect && closedPocket.depth_or_gap < 0,
          "Ink containment confused with an expanded clearance pocket");
  const auto inset=cache.contact(a,{3,8},frame,{0,0},2).contact;
  require(inset.depth_or_gap < 2 && inset.normal.x < 0,"Hole clearance normal points into ink");
}
void eviction() {
  NoFitPolygonCache cache(1);
  auto a=cache.registerShape(rectangle(2,2)),b=cache.registerShape(rectangle(10,10));
  cache.contact(a,{0,0},b,{0,0},0);cache.contact(a,{0,0},b,{0,0},1);
  require(cache.statistics().regions==1 && cache.statistics().evictions==1,"Cache bound not enforced");
  require(cache.contact(a,{0,0},b,{0,0},0).contact.intersect,"Rebuilding evicted region failed");
}
}
int main() {
  try {sharedConvexContacts();linearMinkowskiMatchesReference();unsplitOutlineInput();translationAndSwap();wholeUnionAndClearance();concavityAndHoles();eviction();}
  catch (const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
  std::cout<<"No-fit polygon tests passed\n";
}
