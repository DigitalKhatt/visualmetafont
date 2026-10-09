#include <iostream>
#include <stdexcept>

#include "digitalkhatt/layout/GlyphInstanceUtils.h"
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
}  // namespace

int main() {
  try {
    alternatingNormalsActivateEscape();
    sameAndFollowingLineContactsDoNotActivate();
    clearanceResetsPersistence();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
