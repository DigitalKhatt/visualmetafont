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

#include "hb-ot-layout-gsub-table.hh"
#include "LayoutWindow.h"


#include <QtWidgets>

#include <QTreeView>
#include <QFile>


#include "JustificationContext.h"

//#include "hb-font.hh"
//#include "hb-buffer.hh"
//#include "hb-ft.hh"

#include "font.hpp"

#include "glyph.hpp"




#include "GraphicsViewAdjustment.h"
#include "GraphicsSceneAdjustment.h"

#include "qurantext/quran.h"
//#include <QGLWidget>

#include "GlyphItem.h"

#include "Lookup.h"
#include "GlyphVis.h"
#include "qpoint.h"
#include "automedina/automedina.h"

#include <vector>

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"

#include <iostream>
#include <QPrinter>

#include <math.h> 

#include "to_opentype.h"


struct Force
{
  virtual void execute(double alpha) {}
  virtual ~Force() {}
};

struct GlyphNode {
  int x;
  int y;
  double vx;
  double vy;
  GlyphLayoutInfo* pos;
  GlyphVis* glyph;
  QString glyphName;
  QPolygonF polygon;
};

enum struct LinkType {
  RightMarkLeftmark,
  TopMarkBottomBase,
  BottomMarkTopBase,
  TopMarkRightBase,
  BottomMarkRightBase,
  TopMarkLeftBase,
  BottomMarkLeftBase,
};

struct LinkForceItem {
  LinkType linkType;
  GlyphNode* firstNode;
  GlyphNode* secondNode;
};

struct LinkForce : Force {
  std::vector<LinkForceItem> links;
  QTransform pathtransform;

  LinkForce(QTransform pathtransform) : pathtransform{ pathtransform } {

  }

  void execute(double alpha) override {
    for (auto& link : links) {

      QPoint firsPos{ link.firstNode->x,link.firstNode->y };
      QPoint secondPos{ link.secondNode->x,link.secondNode->y };

      /*
      QPainterPath firstpath = pathtransform.map(link.firstNode->glyph->path);
      QPainterPath secondpath = pathtransform.map(link.secondNode->glyph->path);
      firstpath.translate(firsPos);
      secondpath.translate(secondPos);
      auto intersection = firstpath.intersected(secondpath);*/

      /*
      auto firstTranslate = pathtransform.translate(link.firstNode->x, link.firstNode->y);
      auto secondTranslate = pathtransform.translate(link.secondNode->x, link.secondNode->y);

      auto firstpath = firstTranslate.map(link.firstNode->polygon);
      auto secondpath = secondTranslate.map(link.secondNode->polygon);
      auto intersection = firstpath.intersected(secondpath);


      if (!intersection.isEmpty()) {
        std::cout << link.firstNode->glyphName.toStdString() << " collides with " << link.secondNode->glyphName.toStdString() << std::endl;
        if (link.linkType == LinkType::BottomMarkLeftBase) {
          auto width = intersection.boundingRect().width();
        }
      }*/


      auto firstpath = pathtransform.map(link.firstNode->polygon);
      auto secondpath = pathtransform.map(link.secondNode->polygon);
      firstpath.translate(firsPos);
      secondpath.translate(secondPos);
      auto intersection = firstpath.intersected(secondpath);


      if (!intersection.isEmpty()) {
        std::cout << link.firstNode->glyphName.toStdString() << " collides with " << link.secondNode->glyphName.toStdString() << std::endl;
        if (link.linkType == LinkType::BottomMarkRightBase || link.linkType == LinkType::TopMarkRightBase) {
          auto width = intersection.boundingRect().width();
          link.firstNode->vx -= width;
        }
        else if (link.linkType == LinkType::BottomMarkLeftBase || link.linkType == LinkType::TopMarkLeftBase) {
          auto width = intersection.boundingRect().width();
          link.firstNode->vx += width;
        }
        else if (link.linkType == LinkType::RightMarkLeftmark) {
          auto width = intersection.boundingRect().width();
          link.firstNode->vx += width / 2 ;
          link.secondNode->vx -= width / 2;
        }
      }

    }
  }

};


struct ForceSimulation {
  double alpha = 1;
  double alphaMin = 0.001;
  double	alphaDecay = 1 - std::pow(alphaMin, 1.0 / 300);
  double	alphaTarget = 0;
  double	velocityDecay = 0.6;
  std::vector<std::unique_ptr<Force>>	forces;
  std::vector<std::unique_ptr<GlyphNode>>	nodes;

  void step() {
    tick();

    if (alpha < alphaMin) {
      return;
    }
  }
  void tick(int iterations = 1) {

    auto n = nodes.size();

    for (int k = 0; k < iterations; k++) {
      std::cout << "***************************** Iteration " << k << " *********************************************" << std::endl;
      alpha += (alphaTarget - alpha) * alphaDecay;
      for (auto& force : forces) {
        force->execute(alpha);
      }
      for (int i = 0; i < n; ++i) {
        auto& node = *nodes[i];
        node.vx *= velocityDecay;
        node.x += node.vx;
        node.vy *= velocityDecay;
        node.y += node.vy;
        node.pos->x_offset += node.vx;
        node.pos->y_offset += node.vy;
      }
    }
  }
};


struct BaseMark : Force {
  void execute(double alpha)  override {}
};

void LayoutWindow::applyDirectedForceLayout(QList<QList<LineLayoutInfo>>& pages, QList<QStringList> originalPages, int lineWidth, int beginPage, int nbPages, double emScale) {

  double scale = (double)emScale;

  QTransform pathtransform;

  pathtransform = pathtransform.scale(scale, -scale);

  auto& classes = m_otlayout->automedina->classes;
  auto& marks = classes["marks"];
  auto& topmarks = classes["topmarks"];
  auto& lowmarks = classes["lowmarks"];
  auto& waqfmarks = classes["waqfmarks"];
  auto& topdotmarks = classes["topdotmarks"];
  auto& downdotmarks = classes["downdotmarks"];

  auto isTopMark = [topmarks, lowmarks, waqfmarks, topdotmarks, downdotmarks](QString glyphName) {
    return topmarks.contains(glyphName)
      || waqfmarks.contains(glyphName)
      || topdotmarks.contains(glyphName);
  };

  auto isBottomMark = [topmarks, lowmarks, waqfmarks, topdotmarks, downdotmarks](QString glyphName) {
    return lowmarks.contains(glyphName)
      || downdotmarks.contains(glyphName);
  };

  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];

    QList<QList<QPoint>> pagePositions;
    bool intersection = false;

    ForceSimulation simulation;
    auto linkForce = std::make_unique<LinkForce>(pathtransform);

    std::vector<GlyphNode*> prevMarks;
    std::vector<GlyphNode*> currMarks;
    GlyphNode* prevBase = nullptr;
    GlyphNode* currentBase = nullptr;

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);

      QList<QPoint> linePositions;

      int lineGlyphCount = line.glyphs.size();
      int lastIndex = lineGlyphCount - 1;

      for (int g = 0; g < lineGlyphCount; g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        GlyphVis* currentGlyph = m_otlayout->getGlyph(glyphName, glyphLayout.lefttatweel, glyphLayout.righttatweel);

        currentxPos -= glyphLayout.x_advance;
        QPoint pos(currentxPos + (glyphLayout.x_offset), currentyPos - (glyphLayout.y_offset));

        linePositions.append(pos);

        GlyphNode* node = new GlyphNode{ pos.x(),pos.y(),0,0,&glyphLayout,currentGlyph, glyphName,currentGlyph->path.toFillPolygon() };

        simulation.nodes.push_back(std::unique_ptr<GlyphNode>{node});

        auto& currentNode = simulation.nodes.back();

        bool isMark = marks.contains(glyphName);

        if (isMark) {
          currMarks.push_back(currentNode.get());
          if (prevBase) {
            if (isTopMark(glyphName)) {
              linkForce->links.push_back({ LinkType::TopMarkRightBase, currentNode.get(),prevBase });
            }
            else {
              linkForce->links.push_back({ LinkType::BottomMarkRightBase, currentNode.get(),prevBase });
            }
          }
        }
        else {
          for (auto pm : currMarks) {
            if (isTopMark(pm->glyphName)) {
              linkForce->links.push_back({ LinkType::TopMarkLeftBase, pm,currentNode.get() });
            }
            else {
              linkForce->links.push_back({ LinkType::BottomMarkLeftBase, pm,currentNode.get() });
            }
          }
        }

        if (g == lastIndex || !isMark) {
          for (auto cm : currMarks) {
            for (auto pm : prevMarks) {
              linkForce->links.push_back({ LinkType::RightMarkLeftmark, pm,cm });
            }
          }
          prevMarks = currMarks;
          currMarks.clear();

        }

        if (!isMark) {
          if (currentBase && !currentBase->glyphName.contains("space"))
            prevBase = currentBase;
          currentBase = currentNode.get();
        }

      }


      pagePositions.append(linePositions);

    }

    int nbIterations = std::ceil(std::log(simulation.alphaMin)) / std::log(1 - simulation.alphaDecay);
    //nbIterations = 10;
    simulation.forces.push_back(std::move(linkForce));
    simulation.tick(nbIterations);

    /*
    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      QList<QPoint>& linePositions = pagePositions[l];
      LineLayoutInfo suraName;


      for (int g = 0; g < line.glyphs.size(); g++) {

        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        if (glyphName.contains("space") || glyphName.contains("linefeed") || !m_otlayout->glyphs.contains(glyphName)) continue;

        //bool isIsol = glyphName.contains("isol");

        //bool isFina = glyphName.contains(".fina");

        bool isMark = m_otlayout->automedina->classes["marks"].contains(glyphName);

        //bool isWaqfMark = m_otlayout->automedina->classes["waqfmarks"].contains(glyphName);

        GlyphVis& currentGlyph = *m_otlayout->getGlyph(glyphName, glyphLayout.lefttatweel, glyphLayout.righttatweel);
        QPoint pos = linePositions[g];
        QPainterPath path = pathtransform.map(currentGlyph.path);
        path.translate(pos);

        // verify with the line above
        //if (l > 0 && false) {
        if (l > 0) {
          int prev_index = l - 1;
          QList<QPoint>& prev_linePositions = pagePositions[prev_index];
          auto& prev_line = page[prev_index];

          for (int prev_g = 0; prev_g < prev_line.glyphs.size(); prev_g++) {
            auto& prev_glyphLayout = prev_line.glyphs[prev_g];
            QString prev_glyphName = m_otlayout->glyphNamePerCode[prev_glyphLayout.codepoint];

            bool isPrevMark = m_otlayout->automedina->classes["marks"].contains(prev_glyphName);
            bool isPrevrSpace = prev_glyphName.contains("space") || prev_glyphName.contains("linefeed");
            //bool isPrevIsol = prev_glyphName.contains("isol");


            if ((isMark || isPrevMark) && !isPrevrSpace) { //|| isIsol || isPrevIsol

              GlyphVis& otherGlyph = *m_otlayout->getGlyph(prev_glyphName, prev_glyphLayout.lefttatweel, prev_glyphLayout.righttatweel); //m_otlayout->glyphs[prev_glyphName];
              QPoint otherpos = prev_linePositions[prev_g];

              QPainterPath otherpath = pathtransform.map(otherGlyph.path);
              otherpath.translate(otherpos);
              if (path.intersects(otherpath)) {

                glyphLayout.color = 0xFF000000;
                prev_glyphLayout.color = 0xFF000000;
                intersection = true;
              }
            }

          }

        }

        // verify previous glyphs in the same line
        if (g == 0) continue;


        for (int gg = g - 1; gg >= 0; gg--) {

          auto& otherglyphLayout = line.glyphs[gg];
          QString otherglyphName = m_otlayout->glyphNamePerCode[otherglyphLayout.codepoint];

          bool isOtherMark = m_otlayout->automedina->classes["marks"].contains(otherglyphName);
          //bool isOtherWaqfMark = m_otlayout->automedina->classes["waqfmarks"].contains(otherglyphName);
          bool isOtherSpace = otherglyphName.contains("space") || otherglyphName.contains("linefeed");
          //bool isPrevIsol = otherglyphName.contains("isol");
          //bool isPrevFina = otherglyphName.contains(".fina");

          if ((isMark || isOtherMark) && !isOtherSpace) {// || isIsol || isPrevIsol || (isFina && isPrevFina)

            GlyphVis& otherGlyph = *m_otlayout->getGlyph(otherglyphName, otherglyphLayout.lefttatweel, otherglyphLayout.righttatweel); //glyphs[otherglyphName];
            QPointF otherpos = linePositions[gg];

            QPainterPath otherpath = pathtransform.map(otherGlyph.path);
            otherpath.translate(otherpos);

            if (path.intersects(otherpath)) {

              int adjust = 0;

              if ((glyphName == "kasra" || glyphName.contains("dot")) && (otherglyphName.contains("reh") || otherglyphName.contains("waw") || otherglyphName == "meem.fina" || otherglyphName == "meem.fina.afterkaf" || otherglyphName == "meem.fina.afterheh")) {

                //adjust = rectother.bottom() - rect.top();
                //continue;
              }

              //if (!isWaqfMark) {

              glyphLayout.color = 0xFF000000;

              if (adjust > 0) {
                glyphLayout.y_offset -= adjust << OtLayout::SCALEBY;
              }


              otherglyphLayout.color = 0xFF000000;
              intersection = true;
            }
          }
        }


      }
    }*/


  }
}
