/*
 * Copyright (c) 2015-2023 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include <QFile>
#include <QTreeView>
#include <QtWidgets>

#include "JustificationContext.h"
#include "LayoutWindow.h"
#include "hb-ot-layout-gsub-table.hh"

// #include "hb-font.hh"
// #include "hb-buffer.hh"
// #include "hb-ft.hh"

#include "GraphicsSceneAdjustment.h"
#include "GraphicsViewAdjustment.h"
#include "font.hpp"
#include "glyph.hpp"
#include "qurantext/quran.h"
// #include <QGLWidget>

#include <vector>

#include "GlyphItem.h"
#include "GlyphVis.h"
#include "Lookup.h"
#include "automedina/automedina.h"
#include "qpoint.h"

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include <math.h>

#include <QPrinter>
#include <iostream>

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"
#include "geometry.h"
#include "to_opentype.h"

using namespace geometry;

struct GlyphInstance {
  double baseX = 0.0;
  double baseY = 0.0;
  double lineY = 0.0;
  double dx = 0.0;
  double dy = 0.0;

  const GeometrySet* geom = nullptr;

  GeometrySet geomScaled;

  bool isMark = false;

  bool isTopMark = false;

  GeometrySet worldPolys;  // recomputed each iteration

  GlyphLayoutInfo* glyphLayout;
  GlyphVis* glyphVis;
  QString glyphName;
  int lineIndex;
  int glyphIndex;
  GlyphInstance* prevBase;
  GlyphInstance* nextBase;
  double mobility = 0.0;  // 0 = rigid, 1 = very movable
  double attachCompliance = 1;

  // XPBD state (persistent across iterations!)
  double attachLambda = 0.0;
  double ownLambdaLeft = 0.0;
  double ownLambdaRight = 0.0;
  double ownLambdaVert = 0.0;
};

struct OptParams {
  double minGapBody = 10.0;  // font units
  double minGapMark = 100.0;

  double maxShiftBodyX = 15.0;
  double maxShiftBodyY = 4.0;
  double maxShiftMark = 20.0;  // for marks: radial limit

  int maxIters = 20;
  double tolCollision = 0.5;  // stop when max penetration > -tolCollision

  double smoothStrength = 0.1;  // 0..1 small
  double attachStrength = 0.3;  // 0..1 for mark attachment
  double sepOvershoot = 1.05;   // push a bit extra to converge faster
};

inline void buildWorldPolys(GlyphInstance& g) {
  const double ox = g.baseX + g.dx;
  const double oy = g.baseY + g.dy;

  if (g.geom) {
    g.worldPolys = g.geom->translate(ox, oy);
  } else {
    g.worldPolys = g.geomScaled.translate(ox, oy);
  }
}

double gapCompliance(const GlyphInstance& A,
                     const GlyphInstance& B) {
  return 0.1;
  if (!A.isMark && !B.isMark)
    return 0.0;  // hard for bases
  if (A.isMark && B.isMark)
    return 1e-5;
  return 1e-6;  // mark vs base
}

inline double chooseMinGap(const GlyphInstance& A,
                           const GlyphInstance& B,
                           const OptParams& P) {
  if (A.glyphName.startsWith("alef") &&
      B.prevBase == &A &&
      (B.glyphName.startsWith("hamza") ||
       B.glyphName.startsWith("wasla") ||
       B.glyphName.startsWith("smallhighroundedzero"))) {
    return 40;
  } else {
    return 80;
  }
}

struct GapKey {
  GlyphInstance *a, *b;
  bool operator==(const GapKey& other) const {
    return ((a == other.a && b == other.b) || (a == other.b || b == other.a));
  }
};
struct GapInfo {
  double lambda = 0;
  double count = 0;
};

struct GapKeyHasher {
  std::size_t operator()(const GapKey& k) const {
    std::size_t h1 = std::hash<GlyphInstance*>{}(k.a);
    std::size_t h2 = std::hash<GlyphInstance*>{}(k.b);

    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};
using GapsInfo = std::unordered_map<GapKey, GapInfo, GapKeyHasher>;

void solveGapConstraint(GapsInfo& gapInfos,
                        GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& P,
                        double dt,
                        double& maxPenetration) {
  // Decide desired min gap.
  const bool isMarkExists = (A.isMark || B.isMark);

  if (!isMarkExists) return;

  // You can bias here: e.g. move marks more than base letters.
  double wA = A.mobility;
  double wB = B.mobility;
  // If both are completely rigid, we can't resolve here
  if (wA <= 0.0 && wB <= 0.0) {
    return;
  }

  const double gmin = chooseMinGap(A, B, P);
  double baseCompliance = gapCompliance(A, B);

  double normGmin = 80 / gmin;

  double compliance = baseCompliance / (normGmin * normGmin);

  auto dr = getDistance(A.worldPolys, B.worldPolys, gmin);
  if (!std::isfinite(dr.contact.depth_or_gap))
    return;

  double C = dr.contact.depth_or_gap - gmin;  // want C >= 0

  auto gapKey = GapKey{&A, &B};

  bool debug = false;

  if (C >= 0.0) {
    auto it = gapInfos.find(gapKey);
    if (it != gapInfos.end()) {
      it->second.lambda = 0;
      if (debug) {
        std::cout << "Reset lambda from "
                  << A.glyphName.toStdString()
                  << " To " << B.glyphName.toStdString() << " C = " << C
                  << " n=(" << dr.contact.normal.x
                  << "," << dr.contact.normal.y
                  << ")"
                  << std::endl;
      }
      return;
    }
  }

  auto& gapInfo = gapInfos[gapKey];

  double& lambda = gapInfo.lambda;

  maxPenetration = std::min(maxPenetration, C);

  Vec2 n = dr.contact.normal;
  double nlen = n.length();
  if (nlen < 1e-9) {
    n = {1.0, 0.0};
    nlen = 1.0;
  }
  n = n * (1.0 / nlen);

  // XPBD parameters
  double alpha = compliance / (dt * dt);

  double denom = wA + wB + alpha;
  if (denom < 1e-9)
    return;

  double deltaLambda = (-C - alpha * lambda) / denom;

  // Enforce inequality: lambda >= 0
  double lambdaNew = std::max(0.0, lambda + deltaLambda);
  deltaLambda = lambdaNew - lambda;
  lambda = lambdaNew;

  if (debug) {
    std::cout << "Resolve gap from "
              << A.glyphName.toStdString()
              << " To " << B.glyphName.toStdString() << " C = " << C
              << " n=(" << n.x
              << "," << n.y
              << ") deltaLambda=" << deltaLambda << " lambda=" << lambda;
  }

  gapInfo.count++;
  if (wA > 0.0) {
    Vec2 corrA = n * (-wA * deltaLambda);
    A.dx += corrA.x;
    A.dy += corrA.y;
    buildWorldPolys(A);
    if (debug)
      std::cout << " corrA=(" << corrA.x << "," << corrA.y << ")";
  }

  if (wB > 0.0) {
    Vec2 corrB = n * (wB * deltaLambda);
    B.dx += corrB.x;
    B.dy += corrB.y;
    buildWorldPolys(B);
    if (debug)
      std::cout << " corrB=(" << corrB.x << "," << corrB.y << ")";
  }
  if (debug)
    std::cout << std::endl;
}

void solveAttachConstraint(GlyphInstance& mark, double dt) {
  GlyphInstance& base = *mark.prevBase;

  const double w_m = mark.mobility;
  const double w_b = base.mobility;

  if (w_m <= 0.0 && w_b <= 0.0)
    return;

  Vec2 anchor = {mark.glyphLayout->x_offset - base.glyphLayout->x_offset, mark.glyphLayout->y_offset - base.glyphLayout->y_offset};

  Vec2 target = {
      base.baseX + base.dx + anchor.x,
      base.baseY + base.dy + anchor.y};
  Vec2 current = {
      mark.baseX + mark.dx,
      mark.baseY + mark.dy};

  Vec2 delta = current - target;

  double dist = len(delta);

  if (dist < 1e-6)
    return;

  Vec2 n = delta * (1.0 / dist);  // normalized gradient

  // XPBD compliance
  const double alpha = mark.attachCompliance / (dt * dt);

  // Compute Δλ
  double denom = w_m + w_b + alpha;
  if (denom < 1e-9)
    return;

  double deltaLambda =
      (-dist - alpha * mark.attachLambda) / denom;

  // mark.attachLambda += deltaLambda;

  // Apply corrections
  if (w_m > 0.0) {
    Vec2 corr = n * (w_m * deltaLambda);
    mark.dx += corr.x;
    mark.dy += corr.y;
    buildWorldPolys(mark);
  }

  // Optional: allow base motion later

  if (w_b > 0.0) {
    Vec2 corr = n * (-w_b * deltaLambda);
    base.dx += corr.x;
    base.dy += corr.y;
    buildWorldPolys(base);
  }
}

void solveOwnershipHorizontalXPBD(GlyphInstance& mark,
                                  double leftBoundary,
                                  double rightBoundary,
                                  double compliance,
                                  double dt) {
  if (mark.mobility <= 0.0)
    return;

  double alpha = compliance / (dt * dt);
  double w = mark.mobility;

  double x = mark.baseX + mark.dx;

  // --- Left boundary: leftBoundary - x <= 0
  {
    double C = leftBoundary - x;
    if (C > 0.0) {
      double denom = w + alpha;
      double deltaLambda =
          (-C - alpha * mark.ownLambdaLeft) / denom;

      double lambdaNew = std::max(0.0, mark.ownLambdaLeft + deltaLambda);
      deltaLambda = lambdaNew - mark.ownLambdaLeft;
      mark.ownLambdaLeft = lambdaNew;

      double dx = -w * deltaLambda;  // grad = (-1,0)
      mark.dx += dx;
      buildWorldPolys(mark);
    }
  }

  // --- Right boundary: x - rightBoundary <= 0
  {
    double C = x - rightBoundary;
    if (C > 0.0) {
      double denom = w + alpha;
      double deltaLambda =
          (-C - alpha * mark.ownLambdaRight) / denom;

      double lambdaNew = std::max(0.0, mark.ownLambdaRight + deltaLambda);
      deltaLambda = lambdaNew - mark.ownLambdaRight;
      mark.ownLambdaRight = lambdaNew;

      double dx = w * deltaLambda;  // grad = (1,0)
      mark.dx += dx;
      buildWorldPolys(mark);
    }
  }
}
void solveOwnershipVerticalXPBD(GlyphInstance& mark,
                                double limitY,
                                bool isAbove,
                                double compliance,
                                double dt) {
  if (mark.mobility <= 0.0)
    return;

  double alpha = compliance / (dt * dt);
  double w = mark.mobility;

  double y = mark.baseY + mark.dy;

  // Above: limitY - y <= 0
  // Below: y - limitY <= 0
  double C = isAbove ? (limitY - y) : (y - limitY);

  if (C <= 0.0)
    return;

  double& lambda = mark.ownLambdaVert;

  double denom = w + alpha;
  double deltaLambda =
      (-C - alpha * lambda) / denom;

  double lambdaNew = std::max(0.0, lambda + deltaLambda);
  deltaLambda = lambdaNew - lambda;
  lambda = lambdaNew;

  double dy = isAbove ? (-w * deltaLambda) : (w * deltaLambda);
  mark.dy += dy;
  buildWorldPolys(mark);
}

inline AABB padAABB(const AABB& b, double pad) {
  return {
      b.minx - pad,
      b.miny - pad,
      b.maxx + pad,
      b.maxy + pad};
}

struct SweepBroadphase {
  struct Box {
    int index;          // glyph index
    double minx, maxx;  // padded x-range
    double miny, maxy;  // padded y-range
  };

  std::vector<Box> boxes;  // all active AABBs (padded)

  SweepBroadphase(const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs,
                  const OptParams& P) {
    const double maxGap =
        std::max(P.minGapBody, P.minGapMark);
    const double maxShift =
        std::max({P.maxShiftBodyX, P.maxShiftBodyY, P.maxShiftMark});
    double pad = maxGap + 2.0 * maxShift;

    boxes.reserve(glyphs.size());
    for (int idx = 0; idx < glyphs.size(); ++idx) {
      AABB b = glyphs[idx].get().worldPolys.boundingAABB();
      b = padAABB(b, pad);
      boxes.push_back({idx, b.minx, b.maxx, b.miny, b.maxy});
    }

    std::sort(boxes.begin(), boxes.end(),
              [](const Box& a, const Box& b) {
                return a.minx < b.minx;
              });
  }

  // Run sweep and output candidate pairs
  void findPairs(std::vector<std::pair<int, int>>& pairs) const {
    pairs.clear();
    const size_t n = boxes.size();

    for (size_t i = 0; i < n; ++i) {
      const Box& A = boxes[i];
      // advance until B.minx > A.maxx
      for (size_t j = i + 1; j < n; ++j) {
        const Box& B = boxes[j];
        if (B.minx > A.maxx)
          break;  // rest are further right → done

        // check y-overlap
        if (!(A.maxy < B.miny || A.miny > B.maxy)) {
          A.index < B.index ? pairs.emplace_back(A.index, B.index) : pairs.emplace_back(B.index, A.index);
        }
      }
    }
  }
};

enum class MarkRole {
  None,
  Haraka,  // fatha/damma/kasra/tanween
  Shadda,
  Sukun,
  Maddah,
  HamzaAbove,
  HamzaBelow,
  WaqfSign,
  Dots
};

MarkRole classifyMark(GlyphInstance glyphInstrance) {
  auto& glyphName = glyphInstrance.glyphName;

  if (glyphName.startsWith("hamzaabove")) {
    return MarkRole::HamzaAbove;
  } else if (glyphName.startsWith("hamzabelow")) {
    return MarkRole::HamzaBelow;
  } else if (glyphName.startsWith("shadda")) {
    return MarkRole::Shadda;
  } else if (glyphName.startsWith("sukun")) {
    return MarkRole::Sukun;
  } else if (glyphName.startsWith("sukun")) {
    return MarkRole::Sukun;
  } else if (glyphName.startsWith("fatha") || glyphName.startsWith("damma") || glyphName.startsWith("kasra")) {
    return MarkRole::Haraka;
  } else if (glyphName.startsWith("maddahabove")) {
    return MarkRole::Maddah;
  } else if (glyphName.contains("waqf")) {
    return MarkRole::WaqfSign;
  } else if (glyphName.contains("dot")) {
    return MarkRole::Dots;
  } else {
    return MarkRole::None;
  }
}

void initGlyphMobilities(std::vector<std::vector<GlyphInstance>>& pageGlyphs) {
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      if (!g.isMark) {
        g.mobility = 0.0;  // base glyphs fixed (for now)
        continue;
      }
      g.mobility = 1.0;
      continue;

      MarkRole role = classifyMark(g);

      switch (role) {
        case MarkRole::Haraka:
          g.mobility = 1.0;  // very free
          break;

        case MarkRole::Shadda:
          g.mobility = 0.5;  // moderate
          break;

        case MarkRole::Sukun:
        case MarkRole::Maddah:
          g.mobility = 0.6;  // some freedom
          break;

        case MarkRole::HamzaAbove:
        case MarkRole::HamzaBelow:
          g.mobility = 0.5;  // almost rigid, prefer moving others
          break;

        case MarkRole::WaqfSign:
          g.mobility = 0.7;  // reasonably flexible
          break;
        case MarkRole::Dots:
          g.mobility = 0.2;  // reasonably flexible
          break;
        case MarkRole::None:
        default:
          // Unknown mark: medium
          g.mobility = 0.5;
          break;
      }
    }
  }
}
struct XPBDConstraint {
  double lambda = 0.0;      // warm-start state
  double compliance = 0.0;  // XPBD compliance

  virtual ~XPBDConstraint() = default;
  virtual void project(std::vector<std::reference_wrapper<GlyphInstance>>& p, double dt) = 0;
};
struct YlaneConstraint : XPBDConstraint {
  int markIndex;
  double yLane = 0.0;

  YlaneConstraint(int idx, double comp, double yLane) {
    markIndex = idx;
    compliance = comp;
    this->yLane = yLane;
  }

  void project(std::vector<std::reference_wrapper<GlyphInstance>>& p, double dt) override {
    GlyphInstance& mark = p[markIndex];
    if (mark.mobility == 0.0) return;

    // C = yLane - y
    double yOffset = mark.glyphVis->height / 2;
    if (mark.glyphName.startsWith("damma")) {
      yOffset = yOffset + mark.glyphVis->height / 6;
    }
    double C = mark.lineY + yLane - (mark.baseY + mark.dy + yOffset);

    // Gradient ∇C = (0, -1)
    Vec2 n = {0.0, -1.0};

    // If satisfied, run through XPBD with C = 0, n = 0
    // (this gives λ decay instead of freeze)
    if (C <= 0.0) {
      C = 0.0;
      n = {0.0, 0.0};
    }

    const double alpha = compliance / (dt * dt);
    const double w = mark.mobility;
    const double denom = w + alpha;
    if (denom < 1e-12) return;

    // XPBD update
    double deltaLambda = -(C + alpha * lambda) / denom;

    // One-sided clamp: λ ≤ 0
    double newLambda = std::min(0.0, lambda + deltaLambda);
    double applied = newLambda - lambda;
    lambda = newLambda;

    // Apply correction (only vertical)
    mark.dy += n.y * (w * applied);
    buildWorldPolys(mark);
  }
};
struct HorizontalOrderConstraint : XPBDConstraint {
  GlyphInstance& markA;  // belongs to base glyph
  GlyphInstance& markB;  // belongs to next glyph
  double overlapFactor;  // k in [0,1]

  HorizontalOrderConstraint(
      GlyphInstance& markA, GlyphInstance& markB,
      double k,
      double comp) : markA{markA}, markB{markB} {
    overlapFactor = k;
    compliance = comp;
  }

  void project(std::vector<std::reference_wrapper<GlyphInstance>>& p, double dt) override {
    const double wAinv = markA.mobility;
    const double wBinv = markB.mobility;
    if (wAinv + wBinv == 0.0) return;

    double widthA = markA.glyphVis->width;
    double widthB = markB.glyphVis->width;

    // C = (xB + k*wB) - (xA - k*wA)
    double C =
        (markB.baseX + markB.dx + widthB) -
        (markA.baseX + markA.dx) -
        std::min(overlapFactor * widthA, overlapFactor * widthB);

    // Gradients
    Vec2 nA = {-1.0, 0.0};
    Vec2 nB = {1.0, 0.0};

    // Inactive case → decay λ (soft preference)
    if (C <= 0.0) {
      C = 0.0;
      nA = {0.0, 0.0};
      nB = {0.0, 0.0};
    }

    const double alpha = compliance / (dt * dt);
    const double denom = wAinv + wBinv + alpha;
    if (denom < 1e-12) return;

    double deltaLambda = -(C + alpha * lambda) / denom;

    // One-sided clamp: λ ≤ 0
    double newLambda = std::min(0.0, lambda + deltaLambda);
    double applied = newLambda - lambda;
    lambda = newLambda;

    // Apply corrections
    markA.dx += nA.x * (wAinv * applied);
    buildWorldPolys(markA);

    markB.dx += nB.x * (wBinv * applied);
    buildWorldPolys(markB);
  }
};

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const OptParams& P) {
  initGlyphMobilities(pageGlyphs);
  double dt = 1.0;
  GapsInfo gapInfos;

  std::vector<std::reference_wrapper<GlyphInstance>> glyphs;
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      buildWorldPolys(g);
      glyphs.push_back(g);
    }
  }

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& mark = glyphs[i];
    if (mark.isMark && mark.prevBase != nullptr) {
      solveAttachConstraint(mark, dt);
    }
  }

  std::vector<std::unique_ptr<XPBDConstraint>> xpbdConstraints;

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& mark = glyphs[i];
    if (mark.isMark &&
        (mark.glyphName.startsWith("fatha") ||
         mark.glyphName.startsWith("damma") ||
         mark.glyphName.startsWith("sukun"))) {
      xpbdConstraints.push_back(std::make_unique<YlaneConstraint>(i, 0.1, 600));
    }
  }

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& markB = glyphs[i];
    if (markB.isMark && markB.prevBase->prevBase != nullptr) {
      auto prevBase = markB.prevBase->prevBase;
      for (int glyphIndex = prevBase->glyphIndex + 1; glyphIndex < markB.prevBase->glyphIndex; glyphIndex++) {
        auto& markA = pageGlyphs[markB.lineIndex][glyphIndex];
        if (markA.isTopMark == markB.isTopMark) {
          xpbdConstraints.push_back(std::make_unique<HorizontalOrderConstraint>(markA, markB, 0.2, 0.1));
        }
      }
    }
  }

  for (int iter = 0; iter < P.maxIters; ++iter) {
    // 1. Rebuild world polys

    // 2. Broadphase
    std::vector<std::pair<int, int>> pairs;
    SweepBroadphase sweep(glyphs, P);
    sweep.findPairs(pairs);

    double maxPenetration = 0.0;  // most negative C

    for (auto& c : xpbdConstraints) {
      c->project(glyphs, dt);
    }

    // 3. Gap / collision constraints
    // Iterate candidate pairs
    for (auto [i, j] : pairs) {
      solveGapConstraint(gapInfos, glyphs[i], glyphs[j], P, dt, maxPenetration);
    }

    // 5. mark attachment constraints
    /*
    for (size_t i = 0; i < glyphs.size(); ++i) {
      GlyphInstance& mark = glyphs[i];
      if (mark.isMark && mark.prevBase != nullptr) {
        solveAttachConstraint(mark, dt);
      }
    }*/

    // 7. Early exit
    if (-maxPenetration < P.tolCollision) {
      // maxPenetration is <= 0; closer to 0 is better.
      break;
    }
  }
#if false
  for (const auto& gap : gapInfos) {
    std::cout << gap.first.a->glyphName.toStdString() << "::" << gap.first.b->glyphName.toStdString()
              << " count=" << gap.second.count
              << " lambda=" << gap.second.lambda
              << std::endl;
  }
#endif
}

void LayoutWindow::optimizeLayout(QList<QList<LineLayoutInfo>>& pages, const QList<QStringList>& originalPages, int beginPage, int nbPages, double emScale) {
  auto scale = emScale;

  std::unordered_map<GlyphVis*, GeometrySet> glyphToPolys;

  auto& classes = m_otlayout->automedina->classes;
  auto& marks = classes["marks"];
  auto& topmarks = classes["topmarks"];
  auto& lowmarks = classes["lowmarks"];
  auto& waqfmarks = classes["waqfmarks"];
  auto& topdotmarks = classes["topdotmarks"];
  auto& downdotmarks = classes["downdotmarks"];

  auto isTopMark = [&topmarks, &lowmarks, &waqfmarks, &topdotmarks, &downdotmarks](QString glyphName) {
    return topmarks.contains(glyphName) || waqfmarks.contains(glyphName) || topdotmarks.contains(glyphName);
  };

  auto isBottomMark = [&topmarks, &lowmarks, &waqfmarks, &topdotmarks, &downdotmarks](QString glyphName) {
    return lowmarks.contains(glyphName) || downdotmarks.contains(glyphName);
  };

  // fetch all gryph initially otherwise mpost is not thread safe when
  // executing getAlternate

  std::vector<std::vector<std::vector<GlyphInstance>>> pagesGlyphs;
  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];
    pagesGlyphs.push_back({});
    auto& pageGlyphs = pagesGlyphs.back();
    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];
      pageGlyphs.push_back({});
      auto& lineGlyphs = pageGlyphs.back();
      // To guarantee also the validity of the pointers in GlyphInstance
      lineGlyphs.reserve(line.glyphs.size());
      auto xScale = line.fontSize * line.xscale;
      auto yScale = line.fontSize;

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
      currentyPos = currentyPos * -1;
      GlyphInstance* currentBase = nullptr;
      GlyphInstance* prevBase = nullptr;

      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        auto glyphVis = m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        auto glyphToPoly = glyphToPolys.find(glyphVis);

        if (glyphToPoly == glyphToPolys.end()) {
          if (marks.contains(glyphName)) {
            glyphToPoly = glyphToPolys.insert(
                                          {glyphVis,
                                           buildPolyFromCubics(
                                               getGlyphCubic(glyphVis->copiedPath),
                                               CUBIC_FLATNESS_TOLERANCE)
                                               .scaled(scale, scale)})
                              .first;
          } else {
            glyphToPoly = glyphToPolys.insert(
                                          {glyphVis,
                                           buildConvexPartsFromCubics(
                                               getGlyphCubic(glyphVis->copiedPath),
                                               CUBIC_FLATNESS_TOLERANCE)
                                               .scaled(scale, scale)})
                              .first;
          }
        }

        currentxPos -= glyphLayout.x_advance * line.xscale;

        auto& glyphInstance = lineGlyphs.emplace_back(GlyphInstance{});

        glyphInstance.isMark = marks.contains(glyphName);
        glyphInstance.isTopMark = isTopMark(glyphName);
        glyphInstance.lineY = currentyPos;
        glyphInstance.baseX = currentxPos + (glyphLayout.x_offset * line.xscale);
        glyphInstance.baseY = currentyPos + (glyphLayout.y_offset);
        glyphInstance.glyphLayout = &glyphLayout;
        glyphInstance.glyphVis = glyphVis;
        glyphInstance.glyphName = glyphName;
        glyphInstance.lineIndex = l;
        glyphInstance.glyphIndex = g;

        if (xScale == 1 && yScale == 1) {
          glyphInstance.geom = &glyphToPoly->second;
        } else {
          glyphInstance.geomScaled = glyphToPoly->second.scaled(xScale, yScale);
        }

        glyphInstance.prevBase = currentBase;

        if (!glyphInstance.isMark) {
          prevBase = currentBase;
          currentBase = &lineGlyphs.back();
          if (prevBase) {
            prevBase->nextBase = currentBase;
          }
        }
      }
    }
  }

  // optimize Pages
  OptParams optParams;
  for (auto& page : pagesGlyphs) {
    optimizePage(page, optParams);
  }

  // Update positions
  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];
    auto& pageGlyphs = pagesGlyphs[p];
    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];
      auto& lineGlyphs = pageGlyphs[l];
      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        auto& glyph = lineGlyphs[g];
        glyphLayout.x_offset += glyph.dx;
        glyphLayout.y_offset += glyph.dy;
      }
    }
  }
}
