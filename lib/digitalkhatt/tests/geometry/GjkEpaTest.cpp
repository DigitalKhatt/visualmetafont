#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

#include "digitalkhatt/geometry/geometry.h"

using namespace geometry;

namespace {
constexpr double tolerance = 0.003;  // GJK/EPA uses a 0.001-unit tolerance.

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void check(const Poly& a, const Poly& b, bool intersect, double signedDistance) {
  const GeometrySet setA(std::vector<Poly>{a}), setB(std::vector<Poly>{b});
  for (bool reverse : {false, true}) {
    const Contact contacts[]{
        reverse ? contactGjkEpaPoly(b,a) : contactGjkEpaPoly(a,b),
        reverse ? contactGjkEpaSet(setB,setA) : contactGjkEpaSet(setA,setB)};
    for (const auto& contact : contacts) {
      require(std::isfinite(contact.depth_or_gap) && std::isfinite(contact.normal.x) &&
              std::isfinite(contact.normal.y) && std::isfinite(contact.pA.x) &&
              std::isfinite(contact.pA.y) && std::isfinite(contact.pB.x) && std::isfinite(contact.pB.y),
          "GJK/EPA must return finite contacts for valid and degenerate input");
      require(contact.intersect == intersect || std::abs(signedDistance) <= tolerance,
          "GJK overlap classification differs from independent geometry");
      require(std::abs(contact.depth_or_gap - signedDistance) <= tolerance,
          "GJK/EPA penetration or separation differs from independent geometry");
      if (!contact.intersect)
        require(std::abs((contact.pB-contact.pA).length()-signedDistance) <= tolerance,
            "Separated witnesses must reproduce the geometric gap");
    }
  }
}

Poly translated(Poly p, Vec2 delta) {
  for (auto& v : p) v += delta;
  return p;
}

void equalSupportLengthsDoNotProveSeparation() {
  // The old test sends both cases into the separated-distance branch:
  // (-20,-10) and (20,-10) have squared length 500; (-2,0) and (0,-2)
  // have squared length 4. Their distinct support points still enclose zero.
  const Poly upper{{0,110},{20,110},{20,130},{0,130}};
  const Poly lower{{0,100},{20,100},{20,120},{0,120}};
  const Poly diamond{{1,0},{0,1},{-1,0},{0,-1}};
  const Poly square{{0,0},{20,0},{20,20},{0,20}};
  for (Vec2 offset : {Vec2{}, Vec2{100000,-20000}, Vec2{1e9,-1e9}}) {
    check(translated(upper,offset), translated(lower,offset), true, -10);
    check(translated(diamond,offset), translated(diamond,offset), true, -std::sqrt(2.0));
    check(translated(square,offset), translated(square,offset), true, -20);
    check(translated(upper,offset+Vec2{0,15}), translated(lower,offset), false, 5);
  }
}

void touchingAndDegenerateInput() {
  const Poly square{{0,0},{20,0},{20,20},{0,20}};
  check(square, translated(square,{20,0}), true, 0);
  check(square, translated(square,{20,20}), true, 0);
  check(square, translated(square,{20.01,0}), false, 0.01);
  const Poly line{{0,0},{10,0},{20,0},{20,0}};
  check(line,line,true,0);
  check(line,translated(line,{25,0}),false,5);
  check(line,translated(line,{0,5}),false,5);
  const Poly repeated{{0,0},{0,0},{10,0},{20,0},{20,20},{0,20},{0,0}};
  check(repeated,repeated,true,-20);
  check(repeated,translated(repeated,{25,0}),false,5);
}

Poly convexHull(Poly points) {
  std::sort(points.begin(),points.end(),[](Vec2 a, Vec2 b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  });
  const auto turn = [](Vec2 a, Vec2 b, Vec2 c) { return crossz(b-a,c-a); };
  Poly hull;
  for (auto p : points) {
    while (hull.size()>1 && turn(hull[hull.size()-2],hull.back(),p)<=0) hull.pop_back();
    hull.push_back(p);
  }
  const size_t lowerSize = hull.size();
  for (auto i=points.rbegin()+1; i!=points.rend(); ++i) {
    while (hull.size()>lowerSize && turn(hull[hull.size()-2],hull.back(),*i)<=0) hull.pop_back();
    hull.push_back(*i);
  }
  hull.pop_back();
  return hull;
}

double pointEdgeSquared(Vec2 p, Vec2 a, Vec2 b) {
  const auto edge=b-a;
  const double t=edge.lengthSq()>0 ? std::clamp(dot(p-a,edge)/edge.lengthSq(),0.0,1.0) : 0.0;
  return (p-a-edge*t).lengthSq();
}

// Independent SAT penetration and vertex/edge separation oracle. No GJK
// simplex decisions or support-mapping distance estimates are reused here.
void checkAgainstOracle(const Poly& a, const Poly& b) {
  double depth=std::numeric_limits<double>::infinity();
  bool intersect=true;
  for (const auto* polygon : {&a,&b}) for (size_t i=0; i<polygon->size(); ++i) {
    auto axis=perp((*polygon)[(i+1)%polygon->size()]-(*polygon)[i]);
    if (axis.lengthSq()==0) continue;
    axis=axis/axis.length();
    double amin=INFINITY, amax=-INFINITY, bmin=INFINITY, bmax=-INFINITY;
    for (auto p : a) { amin=std::min(amin,dot(p,axis)); amax=std::max(amax,dot(p,axis)); }
    for (auto p : b) { bmin=std::min(bmin,dot(p,axis)); bmax=std::max(bmax,dot(p,axis)); }
    if (amax<bmin || bmax<amin) intersect=false;
    depth=std::min({depth,amax-bmin,bmax-amin});
  }
  if (intersect) { check(a,b,true,-depth); return; }
  double squared=std::numeric_limits<double>::infinity();
  for (auto p : a) for (size_t i=0; i<b.size(); ++i)
    squared=std::min(squared,pointEdgeSquared(p,b[i],b[(i+1)%b.size()]));
  for (auto p : b) for (size_t i=0; i<a.size(); ++i)
    squared=std::min(squared,pointEdgeSquared(p,a[i],a[(i+1)%a.size()]));
  check(a,b,false,std::sqrt(squared));
}

void deterministicConvexPairs() {
  std::mt19937 random(12345);
  std::uniform_real_distribution<double> coordinate(-100,100);
  for (int sample=0; sample<1024; ++sample) {
    Poly a,b;
    for (int i=0; i<8; ++i) {
      a.push_back({coordinate(random),coordinate(random)});
      b.push_back({coordinate(random)+100,coordinate(random)});
    }
    a=convexHull(std::move(a)); b=convexHull(std::move(b));
    if (sample%7==0) b=a;
    if (sample%11==0) { a.insert(a.begin(),a.front()); b.insert(b.begin(),b.front()); }
    if (sample%13==0) {
      a.insert(a.begin()+1,(a[0]+a[1])*0.5);
      b.insert(b.begin()+1,(b[0]+b[1])*0.5);
    }
    if (sample%3==0) { a=translated(std::move(a),{1e9,-1e9}); b=translated(std::move(b),{1e9,-1e9}); }
    checkAgainstOracle(a,b);
  }
}
}

int main() {
  try {
    equalSupportLengthsDoNotProveSeparation();
    touchingAndDegenerateInput();
    deterministicConvexPairs();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
