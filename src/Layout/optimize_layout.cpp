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
  double dx = 0.0;
  double dy = 0.0;

  const GeometrySet* geom = nullptr;

  GeometrySet geomScaled;

  bool isMark = false;

  GeometrySet worldPolys;  // recomputed each iteration

  GlyphLayoutInfo* glyphLayout;
  GlyphVis* glyphVis;
  QString glyphName;
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

  double cellSize = 64.0;  // broadphase grid cell size (tune)
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

void solveGapConstraint(GlyphInstance& A,
                        GlyphInstance& B,
                        const OptParams& P,
                        double& maxPenetration) {
  // Decide desired min gap.
  const bool isMarkExists = (A.isMark || B.isMark);
  if (!isMarkExists) return;
  const double gmin = isMarkExists ? P.minGapMark : P.minGapBody;

  auto dr = getDistance(A.worldPolys, B.worldPolys, gmin);
  if (!std::isfinite(dr.contact.depth_or_gap))
    return;

  double C = dr.contact.depth_or_gap - gmin;  // want C >= 0

  if (C >= 0.0)
    return;  // already separated enough

  maxPenetration = std::min(maxPenetration, C);

  Vec2 n = dr.contact.normal;
  double nlen = n.length();
  if (nlen < 1e-9) {
    n = {1.0, 0.0};
    nlen = 1.0;
  }
  n = n * (1.0 / nlen);

  // Split correction; equal "mass" for now.
  double corr = -C * 0.5 * P.sepOvershoot;

  // You can bias here: e.g. move marks more than base letters.
  double wA = 1.0;
  double wB = 1.0;
  if (!A.isMark && B.isMark) {
    wA = 0.0;
    wB = 1;
  } else if (A.isMark && !B.isMark) {
    wA = 1;
    wB = 0.0;
  }

  double wsum = wA + wB;
  Vec2 dA = n * (-corr * (wA / wsum));
  Vec2 dB = n * (corr * (wB / wsum));

  A.dx += dA.x;
  A.dy += dA.y;
  B.dx += dB.x;
  B.dy += dB.y;

  if (dA.x != 0 || dA.y != 0) {
    buildWorldPolys(A);
  }

  if (dB.x != 0 || dB.y != 0) {
    buildWorldPolys(B);
  }
}

inline AABB padAABB(const AABB& b, double pad) {
  return {
      b.minx - pad,
      b.miny - pad,
      b.maxx + pad,
      b.maxy + pad};
}

class BroadphaseGrid {
 public:
  BroadphaseGrid(const std::vector<std::reference_wrapper<GlyphInstance>>& glyphs,
                 const OptParams& P) {
    cellSize_ = P.cellSize;

    // Conservative padding: any pair closer than minGap + motion is caught.
    const double maxGap =
        std::max(P.minGapBody, P.minGapMark);
    const double maxShift =
        std::max({P.maxShiftBodyX, P.maxShiftBodyY, P.maxShiftMark});
    pad_ = maxGap + 2.0 * maxShift;

    aabbs_.reserve(glyphs.size());
    for (size_t i = 0; i < glyphs.size(); ++i) {
      AABB raw = glyphs[i].get().worldPolys.boundingAABB();
      AABB box = padAABB(raw, pad_);
      aabbs_.push_back(box);

      int x0 = (int)std::floor(box.minx / cellSize_);
      int x1 = (int)std::floor(box.maxx / cellSize_);
      int y0 = (int)std::floor(box.miny / cellSize_);
      int y1 = (int)std::floor(box.maxy / cellSize_);
      for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
          cells_[{x, y}].push_back((int)i);
        }
    }
  }

  // Return potential neighbors of glyph i (within padded proximity).
  void query(size_t i, std::vector<int>& out) const {
    out.clear();
    const AABB& box = aabbs_[i];

    int x0 = (int)std::floor(box.minx / cellSize_);
    int x1 = (int)std::floor(box.maxx / cellSize_);
    int y0 = (int)std::floor(box.miny / cellSize_);
    int y1 = (int)std::floor(box.maxy / cellSize_);

    for (int y = y0; y <= y1; ++y)
      for (int x = x0; x <= x1; ++x) {
        auto it = cells_.find({x, y});
        if (it == cells_.end()) continue;
        for (int idx : it->second) {
          if ((size_t)idx != i) {
            out.push_back(idx);
          }
        }
      }
    // Duplicates are okay; filtered at caller with i<j and overlapAABB.
  }

  const AABB& aabb(size_t i) const { return aabbs_[i]; }

 private:
  struct CellKey {
    int x, y;
    bool operator==(const CellKey& o) const { return x == o.x && y == o.y; }
  };

  struct CellKeyHash {
    std::size_t operator()(const CellKey& k) const noexcept {
      return (std::hash<int>()(k.x) * 73856093u) ^ std::hash<int>()(k.y);
    }
  };
  double cellSize_ = 64.0;
  double pad_ = 0.0;
  std::vector<AABB> aabbs_;  // already padded
  std::unordered_map<CellKey, std::vector<int>, CellKeyHash> cells_;
};

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const OptParams& P) {
  std::vector<int> candidates;

  size_t glyphCount = 0;
  for (auto& lineGlyphs : pageGlyphs) {
    glyphCount += lineGlyphs.size();
  }
  candidates.reserve(glyphCount);

  for (int iter = 0; iter < P.maxIters; ++iter) {
    // 1. Rebuild world polys
    std::vector<std::reference_wrapper<GlyphInstance>> glyphs;
    for (auto& lineGlyphs : pageGlyphs) {
      for (auto& g : lineGlyphs) {
        buildWorldPolys(g);
        glyphs.push_back(g);
      }
    }

    // 2. Broadphase
    BroadphaseGrid bp(glyphs, P);

    double maxPenetration = 0.0;  // most negative C

    // 3. Gap / collision constraints
    for (size_t i = 0; i < glyphs.size(); ++i) {
      candidates.clear();
      bp.query(i, candidates);

      const AABB& boxI = bp.aabb(i);

      for (int j : candidates) {
        if ((size_t)j <= i) continue;

        const AABB& boxJ = bp.aabb(j);
        if (!overlapAABB(boxI, boxJ))
          continue;
        if (i != j) {
          solveGapConstraint(glyphs[i], glyphs[j], P, maxPenetration);
        }
      }
    }

    // 7. Early exit
    if (-maxPenetration < P.tolCollision) {
      // maxPenetration is <= 0; closer to 0 is better.
      break;
    }
  }
}

void LayoutWindow::optimizeLayout(QList<QList<LineLayoutInfo>>& pages, QList<QStringList> originalPages, int beginPage, int nbPages, double emScale) {
  auto scale = emScale;

  std::unordered_map<GlyphVis*, GeometrySet> glyphToPolys;

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
      auto xScale = line.fontSize * line.xscale;
      auto yScale = line.fontSize;

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
      currentyPos = currentyPos * -1;
      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        auto glyphVis = m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        auto glyphToPoly = glyphToPolys.find(glyphVis);
        if (glyphToPoly == glyphToPolys.end()) {
          glyphToPoly = glyphToPolys.insert(
                                        {glyphVis,
                                         buildConvexPartsFromCubics(
                                             getGlyphCubic(glyphVis->copiedPath),
                                             CUBIC_FLATNESS_TOLERANCE)
                                             .scaled(scale, scale)})
                            .first;
        }

        currentxPos -= glyphLayout.x_advance * line.xscale;

        GlyphInstance glyphInstance;
        glyphInstance.isMark = glyphLayout.x_advance == 0;
        glyphInstance.baseX = currentxPos + (glyphLayout.x_offset * line.xscale);
        glyphInstance.baseY = currentyPos + (glyphLayout.y_offset);
        glyphInstance.glyphLayout = &glyphLayout;
        glyphInstance.glyphVis = glyphVis;
        glyphInstance.glyphName = glyphName;
        if (xScale == 1 && yScale == 1) {
          glyphInstance.geom = &glyphToPoly->second;
        } else {
          glyphInstance.geomScaled = glyphToPoly->second.scaled(xScale, yScale);
        }
        lineGlyphs.emplace_back(std::move(glyphInstance));
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
