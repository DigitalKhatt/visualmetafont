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
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

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
  int globalIndex = 0;
  GlyphInstance* prevBase = nullptr;
  GlyphInstance* nextBase = nullptr;
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
    auto isSLMKasra = A.glyphName == "kasra" && B.glyphName == "smalllowmeem" && B.glyphIndex == A.glyphIndex + 1;
    if (isSLMKasra)
      return 10;
    else
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

struct SolverContext {
  SolverContext(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                const QHash<QString, QSet<QString>>& classes) : classes{classes}, pageGlyphs{pageGlyphs} {
    marks = classes["marks"];
    topmarks = classes["topmarks"];
    lowmarks = classes["lowmarks"];
    waqfmarks = classes["waqfmarks"];
    topdotmarks = classes["topdotmarks"];
    downdotmarks = classes["downdotmarks"];
  }
  bool isWaqf(const GlyphInstance& glyphInstance) {
    return waqfmarks.contains(glyphInstance.glyphName);
  }
  GapsInfo gapInfos;
  const QHash<QString, QSet<QString>>& classes;
  std::vector<std::vector<GlyphInstance>>& pageGlyphs;
  QSet<std::pair<int, int>> squeezedMarks;
  QSet<QString> marks;
  QSet<QString> topmarks;
  QSet<QString> lowmarks;
  QSet<QString> waqfmarks;
  QSet<QString> topdotmarks;
  QSet<QString> downdotmarks;
};

void debugDistance(const GlyphInstance& A, const GlyphInstance& B, const GSContact& gsContact) {
  const auto& geometrySet = A.worldPolys.scaledY(-1);
  auto otherGeometrySet = B.worldPolys.scaledY(-1);

  GlyphVis& currentGlyph = *A.glyphVis;
  GlyphVis& otherGlyph = *B.glyphVis;

  QPointF pA{gsContact.contact.pA.x, -gsContact.contact.pA.y};
  QPointF pB{gsContact.contact.pB.x, -gsContact.contact.pB.y};

  auto normal = gsContact.contact.normal;

  Vec2 dirA = gsContact.contact.pA + (normal * -10);
  Vec2 dirB = gsContact.contact.pB + (normal * 10);

  QPointF dirPA{dirA.x, -dirA.y};
  QPointF dirPB{dirB.x, -dirB.y};

  QLineF lineA(pA, dirPA);
  QLineF lineB(pB, dirPB);

  auto* view = new QGraphicsView();
  QGraphicsScene scene;

  scene.addPath(geometrySet.toQPainterPath(), QPen(Qt::black));
  scene.addPath(otherGeometrySet.toQPainterPath(), QPen(Qt::black));

  if (gsContact.polyA != -1 && gsContact.polyB != -1) {
    auto& tt = geometrySet.polys()[gsContact.polyA];
    auto& tt2 = otherGeometrySet.polys()[gsContact.polyB];

    for (auto& p : tt) {
      QPainterPath pp;
      pp.addEllipse(QPointF(p.x, p.y), 2, 2);
      scene.addPath(pp, QPen(Qt::red));
    }

    for (auto& p : tt2) {
      QPainterPath pp;
      pp.addEllipse(QPointF(p.x, p.y), 2, 2);
      scene.addPath(pp, QPen(Qt::red));
    }

    GeometrySet gg{std::vector<Poly>{tt}};
    GeometrySet gg2{std::vector<Poly>{tt2}};

    scene.addPath(gg.toQPainterPath(), QPen(Qt::red));

    scene.addPath(gg2.toQPainterPath(), QPen(Qt::red));
  }

  QLineF line(pA, pB);

  QPen pen(Qt::magenta, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

  scene.addLine(line, pen);
  scene.addLine(lineA, QPen(Qt::blue));
  // scene.addLine(lineB, QPen(Qt::green));

  qreal arrowSize = 10;
  double angle = std::acos(line.dx() / line.length());
  if (line.dy() >= 0)
    angle = 2 * M_PI - angle;

  QPointF arrowP1 = line.p2() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                        cos(angle + M_PI - M_PI / 3) * arrowSize);
  QPointF arrowP2 = line.p2() + QPointF(sin(angle + M_PI + M_PI / 3) * arrowSize,
                                        cos(angle + M_PI + M_PI / 3) * arrowSize);

  QPolygonF head;
  head << line.p2() << arrowP1 << arrowP2;
  // scene.addPolygon(head);

  QPainterPath circle1;
  circle1.addEllipse(pA, 5, 5);

  QPainterPath circle2;
  circle2.addEllipse(pB, 5, 5);

  scene.addPath(circle1, QPen(Qt::blue));
  scene.addLine(lineA, QPen(Qt::blue));
  scene.addPath(circle2, QPen(Qt::green));

  view->setScene(&scene);
  view->setRenderHints(QPainter::Antialiasing |
                       QPainter::TextAntialiasing);

  view->setMinimumSize(1500, 1000);

  view->scale(2, 2);

  QDialog box;
  box.setWindowTitle("Graphics Preview");

  QVBoxLayout* layout = new QVBoxLayout(&box);
  layout->addWidget(view);

  // Optional: give the box a reasonable minimum width
  box.setMinimumWidth(1500);
  box.setMinimumHeight(1200);

  // Show the message box
  box.exec();
}

static Vec2 normalizeSafe(const Vec2& v, const Vec2& fallback = {1.0, 0.0}) {
  double L = std::sqrt(v.x * v.x + v.y * v.y);
  if (L < 1e-12) return fallback;
  return {v.x / L, v.y / L};
}

static Vec2 applyDirectionalBias(const Vec2& rawN,
                                 const Vec2& preferredDir,
                                 double beta) {
  beta = std::clamp(beta, 0.0, 1.0);
  Vec2 nb{
      (1.0 - beta) * rawN.x + beta * preferredDir.x,
      (1.0 - beta) * rawN.y + beta * preferredDir.y};
  return normalizeSafe(nb, rawN);
}

void solveGapConstraint(SolverContext& solverContext,
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

  if (solverContext.isWaqf(A) && !solverContext.isWaqf(B)) {
    wA = 3;
    wB = 1;
  } else if (!solverContext.isWaqf(A) && solverContext.isWaqf(B)) {
    wA = 1;
    wB = 3;
  }

  // if (gapsContext.classes)

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
    auto it = solverContext.gapInfos.find(gapKey);
    if (it != solverContext.gapInfos.end()) {
      if (debug && it->second.lambda != 0) {
        std::cout << "Reset lambda from "
                  << A.glyphName.toStdString()
                  << " To " << B.glyphName.toStdString() << " C = " << C
                  << " n=(" << dr.contact.normal.x
                  << "," << dr.contact.normal.y
                  << ")"
                  << std::endl;
      }
      it->second.lambda = 0;
      return;
    }
  }

  // debugDistance(A, B, dr);

  auto& gapInfo = solverContext.gapInfos[gapKey];

  double& lambda = gapInfo.lambda;

  maxPenetration = std::min(maxPenetration, C);

  Vec2 n = dr.contact.normal;

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

      MarkRole role = classifyMark(g);

      switch (role) {
        case MarkRole::Haraka:
          g.mobility = 1.0;
          break;

        case MarkRole::Shadda:
          g.mobility = 1.0;
          break;

        case MarkRole::Sukun:
        case MarkRole::Maddah:
          g.mobility = 1.0;
          break;

        case MarkRole::HamzaAbove:
        case MarkRole::HamzaBelow:
          g.mobility = 1.0;
          break;

        case MarkRole::WaqfSign:
          g.mobility = 2;  // reasonably flexible
          break;
        case MarkRole::Dots:
          g.mobility = 1;
          break;
        case MarkRole::None:
        default:
          // Unknown mark: medium
          g.mobility = 1;
          break;
      }
    }
  }
}
struct XPBDConstraint {
  double lambda = 0.0;      // warm-start state
  double compliance = 0.0;  // XPBD compliance

  virtual ~XPBDConstraint() = default;
  virtual void project(SolverContext& solverContext, double dt) = 0;
};
struct YlaneConstraint : XPBDConstraint {
  double yLane = 0.0;
  GlyphInstance& mark;

  YlaneConstraint(GlyphInstance& mark, double comp, double yLane) : mark{mark} {
    compliance = comp;
    this->yLane = yLane;
  }

  void project(SolverContext& solverContext, double dt) override {
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

  void project(SolverContext& solverContext, double dt) override {
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

struct DotWidthWithinBaseConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Separate lambdas for left and right inequalities
  double lambdaLeft = 0.0;
  double lambdaRight = 0.0;
  double compLeft;
  double compRight;

  DotWidthWithinBaseConstraint(GlyphInstance& mark, double compLeft, double compRight) : mark{mark} {
    this->compLeft = compLeft;
    this->compRight = compRight;
  }

  void project(SolverContext& solverContext, double dt) override {
    const double w = mark.mobility;
    if (w <= 0.0) return;

    const double alphaLeft = compLeft / (dt * dt);
    const double denomLeft = w + alphaLeft;

    const double alphaRight = compRight / (dt * dt);
    const double denomRight = w + compRight;

    if (denomLeft < 1e-12 && denomRight < 1e-12) return;

    // Current dot extents
    const double x = mark.baseX + mark.dx;
    const double xMin = x + mark.glyphVis->bbox.llx;
    const double xMax = x + mark.glyphVis->bbox.urx;

    auto base = mark.prevBase;

    auto baseX = base->baseX + base->dx;

    auto baseLeft = baseX + base->glyphVis->bbox.llx;
    auto baseRight = baseX + base->glyphVis->bbox.urx;

    double xLeft = baseLeft - mark.glyphVis->width / 2;
    double xRight = baseRight + mark.glyphVis->width / 2;

    // -------------------------
    // Left inequality:
    // C_L = xLeft - xMin <= 0
    // grad = dC/dx = -1
    // -------------------------
    double CL = xLeft - xMin;
    if (CL > 0.0 && denomLeft >= 1e-12) {
      double deltaLambda =
          -(CL + alphaLeft * lambdaLeft) / denomLeft;

      double lambdaNew = std::min(0.0, lambdaLeft + deltaLambda);
      double applied = lambdaNew - lambdaLeft;
      lambdaLeft = lambdaNew;

      // grad = -1 in x
      mark.dx += (-1.0) * (w * applied);
      buildWorldPolys(mark);
    }

    // -------------------------
    // Right inequality:
    // C_R = xMax - xRight <= 0
    // grad = dC/dx = +1
    // -------------------------
    double CR = xMax - xRight;
    if (CR > 0.0 && denomRight >= 1e-12) {
      double deltaLambda =
          -(CR + alphaRight * lambdaRight) / denomRight;

      double lambdaNew = std::min(0.0, lambdaRight + deltaLambda);
      double applied = lambdaNew - lambdaRight;
      lambdaRight = lambdaNew;

      mark.dx += (+1.0) * (w * applied);
      buildWorldPolys(mark);
    }
  }
};

struct WaqfAlignXConstraint : XPBDConstraint {
  GlyphInstance& mark;

  WaqfAlignXConstraint(GlyphInstance& mark, double comp) : mark{mark} {
    compliance = comp;
  }

  void project(SolverContext& solverContext, double dt) override {
    const double w = mark.mobility;
    if (w <= 0.0) return;

    const double markX = mark.baseX + mark.dx;

    auto base = mark.prevBase;
    auto baseX = base->baseX + base->dx;

    // Equality constraint: C = x_mark - x_base = 0
    double C = markX - baseX;

    const double alpha = compliance / (dt * dt);
    const double denom = w + alpha;
    if (denom < 1e-12) return;

    // Equality XPBD update: no clamp
    double deltaLambda = (-C - alpha * lambda) / denom;
    lambda += deltaLambda;

    // grad = +1 in x
    mark.dx += w * deltaLambda;
    buildWorldPolys(mark);
  }
};

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------
static double boxTopY(const GlyphInstance& g) {
  return g.worldPolys.boundingAABB().maxy;
}

static double boxBottomY(const GlyphInstance& g) {
  return g.worldPolys.boundingAABB().miny;
}

static double boxCenterX(const GlyphInstance& g) {
  auto box = g.worldPolys.boundingAABB();
  return 0.5 * (box.minx + box.maxx);
}

static double boxHeight(const GlyphInstance& g) {
  auto box = g.worldPolys.boundingAABB();
  return box.maxy - box.miny;
}

struct WaqfPlacementConstraint : XPBDConstraint {
  GlyphInstance& waqfMark;

  // ---------- Vertical parameters ----------
  double minCompliance = 0.0;     // hard lower bound
  double targetCompliance = 0.0;  // soft preferred height
  double maxCompliance = 0.0;     // hard upper bound
  double xAlignCompliance = 0.0;  // horizontal alignment

  double minDistFromBaseline = 0.0;
  double minGapToBase = 0.0;
  double minGapToTopMarks = 0.0;
  double desiredExtraLift = 0.0;

  // Upper bound on waqf TOP (usually from line above)
  double upperCeilingY = 0.0;

  // ---------- Horizontal switching policy ----------
  bool enableBelowMarkFallbackX = true;

  // If free vertical band is below this, blend x target toward below mark
  double tightBandThreshold = 80.0;

  // If waqf top is this close to upperCeilingY, blend x target toward below mark
  double upperPressureThreshold = 40.0;

  // ---------- Persistent XPBD state ----------
  double lambdaMin = 0.0;     // lower bound inequality
  double lambdaTarget = 0.0;  // target-height equality
  double lambdaMax = 0.0;     // upper bound inequality
  double lambdaX = 0.0;       // horizontal equality

  explicit WaqfPlacementConstraint(
      GlyphInstance& waqf,
      double minCompliance_,
      double targetCompliance_,
      double maxCompliance_,
      double xAlignCompliance_,
      double minDistFromBaseline_,
      double minGapToBase_,
      double minGapToTopMarks_,
      double desiredExtraLift_,
      double upperCeilingY_)
      : waqfMark(waqf),
        minCompliance(minCompliance_),
        targetCompliance(targetCompliance_),
        maxCompliance(maxCompliance_),
        xAlignCompliance(xAlignCompliance_),
        minDistFromBaseline(minDistFromBaseline_),
        minGapToBase(minGapToBase_),
        minGapToTopMarks(minGapToTopMarks_),
        desiredExtraLift(desiredExtraLift_),
        upperCeilingY(upperCeilingY_) {}

  // Choose a below mark belonging to the same base.
  // Preference: closest in x to the waqf.
  static GlyphInstance* chooseBelowMark(
      GlyphInstance& waqfMark,
      GlyphInstance* base,
      std::vector<std::vector<GlyphInstance>>& pageGlyphs) {
    if (!base) return nullptr;

    GlyphInstance* best = nullptr;
    double bestScore = std::numeric_limits<double>::infinity();
    double xW = boxCenterX(waqfMark);

    auto& line = pageGlyphs[base->lineIndex];
    int startIndex = base->glyphIndex + 1;
    int endIndex = waqfMark.nextBase != nullptr ? waqfMark.nextBase->glyphIndex
                                                : static_cast<int>(line.size());

    for (int index = startIndex; index < endIndex; ++index) {
      GlyphInstance& gi = line[index];
      if (&gi == &waqfMark) continue;
      if (gi.lineIndex != base->lineIndex) continue;
      if (!gi.isMark) continue;
      if (!gi.isTopMark) continue;        // want above mark
      if (gi.prevBase != base) continue;  // same base only

      double dx = std::abs(boxCenterX(gi) - xW);
      if (dx < bestScore) {
        bestScore = dx;
        best = &gi;
      }
    }
    return best;
  }

  // ------------------------------------------------------------
  // Main projection
  // ------------------------------------------------------------
  void project(SolverContext& solverContext,
               double dt) override {
    if (waqfMark.mobility <= 0.0) return;

    GlyphInstance* base = waqfMark.prevBase;
    if (!base) return;

    const double w = waqfMark.mobility;
    if (w <= 0.0) return;

    // ----------------------------------------------------------
    // 1) Compute yMin = minimum allowed waqf bottom
    // ----------------------------------------------------------
    double yMin = boxTopY(*base) + minGapToBase;
    yMin = std::max(yMin, base->lineY + minDistFromBaseline);

    const auto& line = solverContext.pageGlyphs[base->lineIndex];
    int startIndex = base->glyphIndex + 1;
    int endIndex = waqfMark.nextBase != nullptr ? waqfMark.nextBase->glyphIndex
                                                : static_cast<int>(line.size());

    for (int index = startIndex; index < endIndex; ++index) {
      const GlyphInstance& gi = line[index];
      if (&gi == &waqfMark) continue;
      if (!gi.isMark) continue;
      if (!gi.isTopMark) continue;
      if (gi.prevBase != base) continue;

      yMin = std::max(yMin, boxTopY(gi) + minGapToTopMarks);
    }

    // Current waqf geometry
    double yBottom = boxBottomY(waqfMark);
    double yTop = boxTopY(waqfMark);
    double h = boxHeight(waqfMark);

    double yMax = base->lineY + upperCeilingY;

    // Convert top ceiling to bottom ceiling
    double yMaxBottom = yMax - h;

    // ----------------------------------------------------------
    // 2) Detect infeasibility / pressure
    // ----------------------------------------------------------
    bool infeasible = (yMin > yMaxBottom);
    double freeBand = yMaxBottom - yMin;
    bool tightBand = (freeBand < tightBandThreshold);
    bool upperPressure = ((yMax - yTop) < upperPressureThreshold);

    // ----------------------------------------------------------
    // 3) Choose vertical target
    // ----------------------------------------------------------
    double yTargetBottom = 0.0;

    if (!infeasible) {
      yTargetBottom = yMin + desiredExtraLift;
      yTargetBottom = std::min(std::max(yTargetBottom, yMin), yMaxBottom);
    } else {
      // midpoint compromise
      yTargetBottom = 0.5 * (yMin + yMaxBottom);

      // Hard bounds conflict; do not warm-start them
      lambdaMin = 0.0;
      lambdaMax = 0.0;
    }

    // ----------------------------------------------------------
    // 4) Hard lower bound: yMin - yBottom <= 0
    // ----------------------------------------------------------
    if (!infeasible) {
      double C = yMin - boxBottomY(waqfMark);
      if (C > 0.0) {
        double alpha = minCompliance / (dt * dt);
        double denom = w + alpha;
        if (denom > 1e-12) {
          double deltaLambda = -(C + alpha * lambdaMin) / denom;
          double lambdaNew = std::min(0.0, lambdaMin + deltaLambda);
          double applied = lambdaNew - lambdaMin;
          lambdaMin = lambdaNew;

          // grad dC/dy = -1
          waqfMark.dy += (-1.0) * w * applied;
          buildWorldPolys(waqfMark);
        }
      }
    }

    // ----------------------------------------------------------
    // 5) Hard upper bound: yTop - yMax <= 0
    // ----------------------------------------------------------
    if (!infeasible) {
      double C = boxTopY(waqfMark) - yMax;
      if (C > 0.0) {
        double alpha = maxCompliance / (dt * dt);
        double denom = w + alpha;
        if (denom > 1e-12) {
          double deltaLambda = -(C + alpha * lambdaMax) / denom;
          double lambdaNew = std::min(0.0, lambdaMax + deltaLambda);
          double applied = lambdaNew - lambdaMax;
          lambdaMax = lambdaNew;

          // grad dC/dy = +1
          waqfMark.dy += w * applied;
          buildWorldPolys(waqfMark);
        }
      }
    }

    // ----------------------------------------------------------
    // 6) Soft vertical target: yBottom - yTargetBottom = 0
    // ----------------------------------------------------------
    {
      double C = boxBottomY(waqfMark) - yTargetBottom;
      double alpha = targetCompliance / (dt * dt);
      double denom = w + alpha;
      if (denom > 1e-12) {
        double deltaLambda = -(C + alpha * lambdaTarget) / denom;
        lambdaTarget += deltaLambda;

        // grad dC/dy = +1
        waqfMark.dy += w * deltaLambda;
        buildWorldPolys(waqfMark);
      }
    }

    // ----------------------------------------------------------
    // 7) Dynamic horizontal target
    //
    // Normal case: align with base
    // Tight/infeasible case: blend toward below-mark x
    // ----------------------------------------------------------
    double xBase = base->worldPolys.boundingAABB().minx;
    double xTarget = xBase;

    if (enableBelowMarkFallbackX && infeasible) {
      if (GlyphInstance* below = chooseBelowMark(waqfMark, base, solverContext.pageGlyphs)) {
        double xBelow = below->worldPolys.boundingAABB().minx;

        auto bbox = waqfMark.worldPolys.boundingAABB();

        xTarget = xBelow - (bbox.maxx - bbox.minx);
      }
    }

    // ----------------------------------------------------------
    // 8) Soft horizontal equality: xWaqf - xTarget = 0
    // ----------------------------------------------------------
    {
      // double xW = boxCenterX(waqfMark);
      double xW = waqfMark.worldPolys.boundingAABB().minx;
      double C = xW - xTarget;

      double alpha = xAlignCompliance / (dt * dt);
      double denom = w + alpha;
      if (denom > 1e-12) {
        double deltaLambda = -(C + alpha * lambdaX) / denom;
        lambdaX += deltaLambda;

        // grad dC/dx = +1
        waqfMark.dx += w * deltaLambda;
        buildWorldPolys(waqfMark);
      }
    }
  }
};

struct HardStayAboveConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Hard lower bounds
  double minGapToBase = 0.0;
  double minDistFromBaseline = 0.0;

  // If true, use the base top as one of the lower bounds
  bool useBaseTop = true;

  explicit HardStayAboveConstraint(GlyphInstance& mark_,
                                   double compliance_,
                                   double minGapToBase_ = 0.0,
                                   double minDistFromBaseline_ = 0.0,
                                   bool useBaseTop_ = false)
      : mark(mark_),
        minGapToBase(minGapToBase_),
        minDistFromBaseline(minDistFromBaseline_),
        useBaseTop(useBaseTop_) {
    compliance = compliance_;
  }

  static double
  bottomY(const GlyphInstance& g) {
    return g.worldPolys.boundingAABB().miny;
  }

  static double topY(const GlyphInstance& g) {
    return g.worldPolys.boundingAABB().maxy;
  }

  void project(SolverContext& solverContext, double dt) override {
    const double w = mark.mobility;
    if (w <= 0.0) return;

    GlyphInstance* base = mark.prevBase;
    if (!base) return;

    double yMin = -std::numeric_limits<double>::infinity();

    // Base-top lower bound
    if (useBaseTop) {
      yMin = std::max(yMin, topY(*base) + minGapToBase);
    } else {
      // Baseline lower bound
      yMin = std::max(yMin, base->lineY + minDistFromBaseline);
    }

    // Constraint: yMin - markBottom <= 0
    const double C = yMin - bottomY(mark);
    if (C <= 0.0) return;  // hard feasibility: inactive => do nothing

    const double alpha = compliance / (dt * dt);
    const double denom = w + alpha;
    if (denom < 1e-12) return;

    double deltaLambda = -(C + alpha * lambda) / denom;

    // inequality with feasible C <= 0 => lambda <= 0
    double lambdaNew = std::min(0.0, lambda + deltaLambda);
    double applied = lambdaNew - lambda;
    lambda = lambdaNew;

    // grad dC/dy = -1
    mark.dy += (-1.0) * w * applied;
    buildWorldPolys(mark);
  }
};

struct HardStayBelowConstraint : XPBDConstraint {
  GlyphInstance& mark;

  // Hard upper bounds
  double maxGapToBase = 0.0;       // distance BELOW base bottom
  double maxDistToBaseline = 0.0;  // distance BELOW baseline

  // If true, use the base bottom as one of the upper bounds
  bool useBaseBottom = false;

  explicit HardStayBelowConstraint(GlyphInstance& mark_,
                                   double compliance_,
                                   double maxGapToBase_ = 0.0,
                                   double maxDistToBaseline_ = 0.0,
                                   bool useBaseBottom_ = false)
      : mark(mark_),
        maxGapToBase(maxGapToBase_),
        maxDistToBaseline(maxDistToBaseline_),
        useBaseBottom(useBaseBottom_) {
    compliance = compliance_;
  }

  static double bottomY(const GlyphInstance& g) {
    return g.worldPolys.boundingAABB().miny;
  }

  static double topY(const GlyphInstance& g) {
    return g.worldPolys.boundingAABB().maxy;
  }

  void project(SolverContext& solverContext,
               double dt) override {
    const double w = mark.mobility;
    if (w <= 0.0) return;

    GlyphInstance* base = mark.prevBase;
    if (!base) return;

    double yMax = std::numeric_limits<double>::infinity();

    // Base-bottom upper bound (stay below base)
    if (useBaseBottom) {
      yMax = std::min(yMax, bottomY(*base) - maxGapToBase);
    } else {
      // Baseline upper bound (stay below baseline + offset)
      yMax = std::min(yMax, base->lineY + maxDistToBaseline);
    }

    // Constraint: yTop - yMax <= 0
    const double C = topY(mark) - yMax;
    if (C <= 0.0) return;  // already satisfied

    const double alpha = compliance / (dt * dt);
    const double denom = w + alpha;
    if (denom < 1e-12) return;

    double deltaLambda = -(C + alpha * lambda) / denom;

    // inequality with feasible C <= 0 => lambda <= 0
    double lambdaNew = std::min(0.0, lambda + deltaLambda);
    double applied = lambdaNew - lambda;
    lambda = lambdaNew;

    // grad dC/dy = +1 → move downward
    mark.dy += w * applied;
    buildWorldPolys(mark);
  }
};

struct MatchMarkPositionWithOffsetConstraint : XPBDConstraint {
  GlyphInstance& A;
  GlyphInstance& B;
  double offsetX = 0.0;
  double offsetY = 0.0;

  double lambdaX = 0.0;
  double lambdaY = 0.0;

  explicit MatchMarkPositionWithOffsetConstraint(GlyphInstance& a,
                                                 GlyphInstance& b,
                                                 double offsetX_,
                                                 double offsetY_,
                                                 double compliance_) : A(a), B(b), offsetX(offsetX_), offsetY(offsetY_) {
    compliance = compliance_;
  }

  static double centerX(const GlyphInstance& g) {
    auto box = g.worldPolys.boundingAABB();
    return 0.5 * (box.minx + box.maxx);
  }

  static double centerY(const GlyphInstance& g) {
    auto box = g.worldPolys.boundingAABB();
    return 0.5 * (box.miny + box.maxy);
  }

  void project(SolverContext& solverContext,
               double dt) override {
    const double wA = A.mobility;
    const double wB = B.mobility;
    if (wA + wB <= 0.0) return;

    const double alpha = compliance / (dt * dt);
    const double denom = wA + wB + alpha;
    if (denom < 1e-12) return;

    {
      double C = (centerX(A) - centerX(B)) - offsetX;
      double deltaLambda = (-C - alpha * lambdaX) / denom;
      lambdaX += deltaLambda;

      if (wA > 0.0) A.dx += wA * deltaLambda;
      if (wB > 0.0) B.dx -= wB * deltaLambda;

      buildWorldPolys(A);
      buildWorldPolys(B);
    }

    {
      double C = (centerY(A) - centerY(B)) - offsetY;
      double deltaLambda = (-C - alpha * lambdaY) / denom;
      lambdaY += deltaLambda;

      if (wA > 0.0) A.dy += wA * deltaLambda;
      if (wB > 0.0) B.dy -= wB * deltaLambda;

      buildWorldPolys(A);
      buildWorldPolys(B);
    }
  }
};

struct VerticalStackAlignConstraint : XPBDConstraint {
  GlyphInstance& lowerMark;  // e.g. shadda
  GlyphInstance& upperMark;  // e.g. fatha / damma

  // Optional optical offset: x_lower - x_upper = offsetX
  double offsetX = 0.0;

  explicit VerticalStackAlignConstraint(GlyphInstance& lowerMark_,
                                        GlyphInstance& upperMark_,
                                        double compliance_,
                                        double offsetX_ = 0.0)
      : lowerMark(lowerMark_),
        upperMark(upperMark_),
        offsetX(offsetX_) {
    compliance = compliance_;
  }

  static double centerX(const GlyphInstance& g) {
    auto box = g.worldPolys.boundingAABB();
    return 0.5 * (box.minx + box.maxx);
  }

  void project(SolverContext& solverContext, double dt) override {
    const double wL = lowerMark.mobility;
    const double wU = upperMark.mobility;
    if (wL + wU <= 0.0) return;

    // Equality: x_lower - x_upper - offsetX = 0
    const double C = (centerX(lowerMark) - centerX(upperMark)) - offsetX;

    const double alpha = compliance / (dt * dt);
    const double denom = wL + wU + alpha;
    if (denom < 1e-12) return;

    const double deltaLambda = (-C - alpha * lambda) / denom;
    lambda += deltaLambda;

    // grad wrt lowerMark.x = +1
    // grad wrt upperMark.x = -1
    if (wL > 0.0) {
      lowerMark.dx += wL * deltaLambda;
      buildWorldPolys(lowerMark);
    }

    if (wU > 0.0) {
      upperMark.dx -= wU * deltaLambda;
      buildWorldPolys(upperMark);
    }
  }
};

struct NormalizedSqueezedMarkGapConstraint : XPBDConstraint {
  struct ContactState {
    GlyphInstance* obstacle = nullptr;
    double lambda = 0.0;

    Vec2 lastN{0.0, 0.0};
    bool hasLastN = false;
  };

  GlyphInstance& mark;
  const OptParams& P;

  std::vector<ContactState> contacts;

  // Shared normalized slack s >= 0
  double slack = 0.0;

  // Larger => easier to accept common relative violation
  double slackInvMass = 0.1;

  // Soft penalty driving slack back to zero
  double slackCompliance = 1e-4;
  double lambdaSlack = 0.0;

  // Optional normal-jump reset
  bool resetOnNormalJump = true;
  double normalJumpDotThreshold = 0.3;

  // ------------------------------------------------------------
  // Directional bias parameters
  // ------------------------------------------------------------
  bool enableDirectionalBias = false;

  double baseBiasStrength = 0.25;
  double slackBiasStrength = 0.35;
  double multiContactBiasStrength = 0.20;

  double upwardWeight = 0.6;
  double awayFromNextBaseWeight = 0.3;
  double awayFromObstacleWeight = 0.1;

  explicit NormalizedSqueezedMarkGapConstraint(
      GlyphInstance& mark_,
      const OptParams& P_,
      const std::vector<GlyphInstance*>& obstacles_,
      double slackInvMass_ = 0.25,
      double slackCompliance_ = 1e-4)
      : mark(mark_),
        P(P_),
        slackInvMass(slackInvMass_),
        slackCompliance(slackCompliance_) {
    contacts.reserve(obstacles_.size());
    for (GlyphInstance* g : obstacles_) {
      if (!g) continue;
      if (g == &mark) continue;
      contacts.push_back(ContactState{g});
    }
  }

  // ------------------------------------------------------------
  // Helpers
  // ------------------------------------------------------------
  static double centerX(const GlyphInstance& g) {
    auto box = g.worldPolys.boundingAABB();
    return 0.5 * (box.minx + box.maxx);
  }

  static double centerY(const GlyphInstance& g) {
    auto box = g.worldPolys.boundingAABB();
    return 0.5 * (box.miny + box.maxy);
  }

  static Vec2 center(const GlyphInstance& g) {
    return {centerX(g), centerY(g)};
  }

  static double dot2(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
  }

  static Vec2 add2(const Vec2& a, const Vec2& b) {
    return {a.x + b.x, a.y + b.y};
  }

  static Vec2 mul2(const Vec2& a, double s) {
    return {a.x * s, a.y * s};
  }

  Vec2 computePreferredDirection(const GlyphInstance& obstacle) const {
    Vec2 pref{0.0, 0.0};

    // Upward bias
    Vec2 up{0.0, 1.0};
    pref = add2(pref, mul2(up, upwardWeight));

    // Away from next base
    if (mark.nextBase != nullptr) {
      Vec2 cMark = center(mark);
      Vec2 cNext = center(*mark.nextBase);
      Vec2 awayFromNext = normalizeSafe({cMark.x - cNext.x, cMark.y - cNext.y}, up);
      pref = add2(pref, mul2(awayFromNext, awayFromNextBaseWeight));
    }

    // Slightly away from current obstacle
    {
      Vec2 cMark = center(mark);
      Vec2 cObs = center(obstacle);
      Vec2 awayObs = normalizeSafe({cMark.x - cObs.x, cMark.y - cObs.y}, up);
      pref = add2(pref, mul2(awayObs, awayFromObstacleWeight));
    }

    return normalizeSafe(pref, up);
  }

  double computeBiasStrength(int violatedCount) const {
    double beta = baseBiasStrength;

    if (slack > 1e-6) {
      beta += slackBiasStrength;
    }

    if (violatedCount >= 2) {
      beta += multiContactBiasStrength;
    }

    return std::clamp(beta, 0.0, 0.95);
  }

  // ------------------------------------------------------------
  // Main projection
  // ------------------------------------------------------------
  void project(SolverContext& solverContext, double dt) override {
    const double wM = mark.mobility;
    if (wM <= 0.0) return;

    // First pass: count raw violations
    int violatedCount = 0;
    for (auto& cs : contacts) {
      GlyphInstance& obs = *cs.obstacle;
      const double gmin = chooseMinGap(mark, obs, P);
      if (gmin <= 1e-9) continue;

      auto dr = getDistance(mark.worldPolys, obs.worldPolys, gmin);
      if (!std::isfinite(dr.contact.depth_or_gap)) continue;

      const double Craw = dr.contact.depth_or_gap - gmin;  // want >= 0
      if (Craw < 0.0) violatedCount++;
    }

    bool anyRawViolation = false;

    // ----------------------------------------------------------
    // Solve all normalized shared-slack contacts
    // ----------------------------------------------------------
    for (auto& cs : contacts) {
      GlyphInstance& obs = *cs.obstacle;

      const double gmin = chooseMinGap(mark, obs, P);
      if (gmin <= 1e-9) {
        cs.lambda = 0.0;
        cs.hasLastN = false;
        continue;
      }

      const double contactCompliance = gapCompliance(mark, obs);

      auto dr = getDistance(mark.worldPolys, obs.worldPolys, gmin);
      if (!std::isfinite(dr.contact.depth_or_gap)) {
        cs.lambda = 0.0;
        cs.hasLastN = false;
        continue;
      }

      // Raw gap violation: distance - gmin (want >= 0)
      const double Craw = dr.contact.depth_or_gap - gmin;
      if (Craw < 0.0) anyRawViolation = true;

      // Raw normal
      Vec2 n = dr.contact.normal;

      // Reset on raw normal jump
      if (resetOnNormalJump && cs.hasLastN) {
        double d = dot2(n, cs.lastN);
        if (d < normalJumpDotThreshold) {
          cs.lambda = 0.0;
        }
      }
      cs.lastN = n;
      cs.hasLastN = true;

      // --------------------------------------------------------
      // Directional bias
      // --------------------------------------------------------
      Vec2 nUsed = n;
      if (enableDirectionalBias) {
        Vec2 pref = computePreferredDirection(obs);
        double beta = computeBiasStrength(violatedCount);
        nUsed = applyDirectionalBias(n, pref, beta);
      }

      // --------------------------------------------------------
      // Normalized shared-slack constraint:
      //
      // C = (distance - gmin)/gmin + slack  >= 0
      // --------------------------------------------------------
      // const double C = (Craw / gmin) + slack;

      const double C = Craw + slack;

      if (C >= 0.0) {
        if (Craw >= 0.0) {
          cs.lambda = 0.0;
        }
        continue;
      }

      const double alpha = contactCompliance / (dt * dt);

      // Gradient wrt mark:
      // ∇C = -(1/gmin) * nUsed
      //
      // so |∇C|^2 = 1/(gmin^2)
      // const double invG = 1.0 / gmin;
      // const double gradNorm2 = invG * invG;

      // const double denom = wM * gradNorm2 + slackInvMass + alpha;
      double denom = wM + slackInvMass + alpha;
      double invG = 1;

      if (denom < 1e-12) continue;

      double deltaLambda = (-C - alpha * cs.lambda) / denom;

      // Feasible C >= 0 => lambda >= 0
      double lambdaNew = std::max(0.0, cs.lambda + deltaLambda);
      double applied = lambdaNew - cs.lambda;
      cs.lambda = lambdaNew;

      // Mark correction: Δx = w * ∇C * Δλ
      // ∇C = -(1/gmin) * nUsed
      mark.dx += (-nUsed.x) * wM * invG * applied;
      mark.dy += (-nUsed.y) * wM * invG * applied;

      // Shared normalized slack correction
      // ∂C/∂s = 1
      slack += slackInvMass * applied;
      if (slack < 0.0) slack = 0.0;

      buildWorldPolys(mark);
    }

    // ----------------------------------------------------------
    // Softly shrink slack back toward zero
    // ----------------------------------------------------------
    if (slack > 0.0) {
      const double alphaS = slackCompliance / (dt * dt);
      const double denomS = slackInvMass + alphaS;

      if (denomS > 1e-12) {
        double deltaLambda = (-slack - alphaS * lambdaSlack) / denomS;
        lambdaSlack += deltaLambda;

        slack += slackInvMass * deltaLambda;
        if (slack < 0.0) {
          slack = 0.0;
          lambdaSlack = 0.0;
        }
      }
    }

    // Clear stale lambdas if no raw violations remain
    if (!anyRawViolation) {
      for (auto& cs : contacts) {
        cs.lambda = 0.0;
      }
    }
  }
};

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs, QHash<QString, QSet<QString>> classes, const OptParams& P) {
  auto& marks = classes["marks"];
  auto& topmarks = classes["topmarks"];
  auto& lowmarks = classes["lowmarks"];
  auto& waqfmarks = classes["waqfmarks"];
  auto& topdotmarks = classes["topdotmarks"];
  auto& downdotmarks = classes["downdotmarks"];

  initGlyphMobilities(pageGlyphs);
  double dt = 1.0;
  SolverContext solverContext{pageGlyphs, classes};

  std::vector<std::reference_wrapper<GlyphInstance>> glyphs;
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      g.globalIndex = glyphs.size();
      buildWorldPolys(g);
      glyphs.push_back(g);
    }
  }

  std::vector<std::unique_ptr<XPBDConstraint>> xpbdConstraints;

  std::vector<std::unique_ptr<XPBDConstraint>> hardConstraints;

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& mark = glyphs[i];
    if (mark.isMark &&
        (mark.glyphName.startsWith("fatha") ||
         mark.glyphName.startsWith("damma") ||
         mark.glyphName.startsWith("sukun"))) {
      xpbdConstraints.push_back(std::make_unique<YlaneConstraint>(mark, 0.1, 600));
    }
    if (mark.glyphName.contains("dot")) {
      xpbdConstraints.push_back(std::make_unique<DotWidthWithinBaseConstraint>(mark, 0.1, 0.1));
    } else {
      // [lam.init.beforemeem]' [meem.medi.afterlam]'
      if (mark.glyphName.contains("fatha") && mark.prevBase->glyphName == "lam.init.beforemeem") {
        xpbdConstraints.push_back(std::make_unique<DotWidthWithinBaseConstraint>(mark, 0.1, 0.1));
      }
    }
    if (waqfmarks.contains(mark.glyphName)) {
      // xpbdConstraints.push_back(std::make_unique<WaqfAlignXConstraint>(mark, 0.3));
      xpbdConstraints.push_back(std::make_unique<WaqfPlacementConstraint>(mark, /*minCompliance=*/0.3,  // hard-ish lower bound
                                                                          /*targetCompliance=*/0.4,     // softer target height
                                                                          /*maxCompliance=*/0.3,        // hard-ish upper bound
                                                                          /*xAlignCompliance=*/0.3,
                                                                          /*minDistFromBaseline=*/700.0,
                                                                          /*minGapToBase=*/100.0,
                                                                          /*minGapToTopMarks=*/50.0,
                                                                          /*desiredExtraLift=*/0.0,
                                                                          /*upperCeilingY=*/1400.0));
    }
    /*if (mark.glyphName == "twodotsdown") {
      hardConstraints.push_back(std::make_unique<HardStayBelowConstraint>(mark, 1e-7, 0.0, 0.0, true));
    } else if (mark.glyphName == "onedotdown" && mark.prevBase->glyphName.startsWith("behshape.")) {
      hardConstraints.push_back(std::make_unique<HardStayBelowConstraint>(mark, 1e-7, 0.0, 0.0, true));
    }*/

    if (mark.glyphName == "smalllowmeem" && i > 0) {
      GlyphInstance& prevMark = glyphs[i - 1];
      if (prevMark.glyphName == "kasra") {
        xpbdConstraints.push_back(std::make_unique<MatchMarkPositionWithOffsetConstraint>(prevMark, mark, 130, 0, 0.2));
      }
    }
    if (mark.glyphName.startsWith("twodotsdown")) {
      if (mark.prevBase->glyphName.startsWith("behshape.init") && mark.prevBase->prevBase != nullptr && mark.prevBase->prevBase->glyphName.startsWith("reh.")) {
        auto markBottom = boxBottomY(mark);
        auto rehBottom = boxBottomY(*mark.prevBase->prevBase);
        if (markBottom > rehBottom) {
          std::vector<GlyphInstance*> obstacles;
          obstacles.push_back(mark.prevBase);
          obstacles.push_back(mark.prevBase->prevBase);
          if (mark.prevBase->nextBase != nullptr) {
            solverContext.squeezedMarks.insert({i, mark.prevBase->nextBase->globalIndex});
            obstacles.push_back(mark.prevBase->nextBase);
          }
          solverContext.squeezedMarks.insert({mark.prevBase->prevBase->globalIndex, i});
          solverContext.squeezedMarks.insert({mark.prevBase->globalIndex, i});
          xpbdConstraints.push_back(std::make_unique<NormalizedSqueezedMarkGapConstraint>(mark, P, obstacles));
        }
      }
    }
    if (mark.glyphName.startsWith("shadda")) {
      GlyphInstance& nextMark = glyphs[mark.globalIndex + 1];
      if (nextMark.glyphName.startsWith("fatha") || nextMark.glyphName.startsWith("damma")) {
        xpbdConstraints.push_back(std::make_unique<VerticalStackAlignConstraint>(mark, nextMark, 0.2));
      }
    }
  }

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& markB = glyphs[i];
    if (markB.isMark && markB.prevBase->prevBase != nullptr) {
      if (solverContext.isWaqf(markB) && (markB.prevBase->glyphName == "smallwaw" || markB.prevBase->glyphName == "smallyeh"))
        continue;
      auto prevBase = markB.prevBase->prevBase;
      for (int glyphIndex = prevBase->glyphIndex + 1; glyphIndex < markB.prevBase->glyphIndex; glyphIndex++) {
        auto& markA = pageGlyphs[markB.lineIndex][glyphIndex];
        if (markA.isTopMark == markB.isTopMark && !solverContext.isWaqf(markA) && !solverContext.isWaqf(markB)) {
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
      c->project(solverContext, dt);
    }

    // 3. Gap / collision constraints
    // Iterate candidate pairs
    for (auto& pair : pairs) {
      GlyphInstance& glyphA = glyphs[pair.first];
      GlyphInstance& glyphB = glyphs[pair.second];
      if (!solverContext.squeezedMarks.contains(pair)) {
        solveGapConstraint(solverContext, glyphA, glyphB, P, dt, maxPenetration);
      }
    }

    // 5. mark attachment constraints
    /*
    for (size_t i = 0; i < glyphs.size(); ++i) {
      GlyphInstance& mark = glyphs[i];
      if (mark.isMark && mark.prevBase != nullptr) {
        solveAttachConstraint(mark, dt);
      }
    }*/

    for (auto& c : hardConstraints) {
      c->project(solverContext, dt);
    }

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
    optimizePage(page, classes, optParams);
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
