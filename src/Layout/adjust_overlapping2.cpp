#include <QFile>
#include <QTreeView>
#include <QtSql>
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
#include "contact_flexible.h"
#include "qpoint.h"

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include <Subtable.h>
#include <math.h>

#include <QPrinter>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <unordered_set>

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"
#include "geometry.h"
#include "gllobal_strings.h"
#include "to_opentype.h"

using namespace geometry;

static std::unordered_map<GlyphVis*, GeometrySet> glyphToPolys;

void LayoutWindow::adjustOverlapping2(QList<QList<LineLayoutInfo>>& pages,
                                      int lineWidth,
                                      QList<QStringList> originalPages,
                                      double emScale, bool sameLine,
                                      bool interLine) {
  // QPageSize pageSize{ { 90.2,144.5 },QPageSize::Millimeter, "MedianQuranBook"
  // };
  QPageSize pageSize{{90.2, 147.5}, QPageSize::Millimeter, "MedianQuranBook"};
  QPageLayout pageLayout{pageSize, QPageLayout::Portrait,
                         QMarginsF(0, 0, 0, 0)};

  int totalpageNb = pages.size();

  int nbthreads = 12;
  int pageperthread = totalpageNb / nbthreads;
  int remainingPages = totalpageNb;

  std::vector<QThread*> threads;
  std::vector<QVector<int>*> overlappages;

  std::vector<std::vector<OverlapResult>*> overlapResults;

  glyphToPolys.clear();

  // fetch all gryph initially otherwise mpost is not thread safe when
  // executing getAlternate
  auto tau = 1;
  for (auto& page : pages) {
    for (auto& line : page) {
      for (auto& glyph : line.glyphs) {
        // GlyphVis* currentGlyph =
        auto glyphVis = m_otlayout->getGlyph(
            glyph.codepoint, {.lefttatweel = glyph.lefttatweel,
                              .righttatweel = glyph.righttatweel});
        if (glyphToPolys.find(glyphVis) == glyphToPolys.end()) {
          glyphToPolys.insert(
              {glyphVis, buildConvexPartsFromCubics(
                             getGlyphCubic(glyphVis->copiedPath), tau)});
        }
      }
    }
  }

  while (remainingPages != 0) {
    int begin = totalpageNb - remainingPages;
    if (pageperthread == 0 || remainingPages < pageperthread) {
      pageperthread = remainingPages;
    }

    remainingPages -= pageperthread;

    QVector<int>* set = new QVector<int>();

    overlappages.push_back(set);

    std::vector<OverlapResult>* result1 = new std::vector<OverlapResult>();

    overlapResults.push_back(result1);

    QThread* thread =
        QThread::create([this, &pages, result = result1, begin, pageperthread, set,
                         lineWidth, emScale, sameLine, interLine] {
          adjustOverlapping2(pages, lineWidth, begin, pageperthread, *set,
                             emScale, *result, sameLine, interLine);
        });

    threads.push_back(thread);

    thread->start();
  }

  for (auto t : threads) {
    t->wait();
    delete t;
  }

  QVector<OverlapResult> overlapResult;

  QMap<QVector<int>, OverlapResult> sequences;

  for (auto overlaps : overlapResults) {
    for (auto& overlap : *overlaps) {
      auto& page = pages[overlap.pageIndex];
      auto& line = page[overlap.lineIndex];
      QVector<int> sequence;
      for (int i = overlap.prevGlyph; i <= overlap.nextGlyph; i++) {
        auto& glyphLayout = line.glyphs[i];
        sequence.append(glyphLayout.codepoint);
      }

      overlapResult.append(overlap);
      /*if (!sequences.contains(sequence)) {
        sequences.insert(sequence, overlap);
        overlapResult.append(overlap);
      }*/
    }
    delete overlaps;
  }

  generateOverlapLookups(pages, originalPages, overlapResult);

  QList<QList<LineLayoutInfo>> newpages;
  QList<QStringList> neworiginalPages;

  for (auto t : overlappages) {
    for (auto pIndex : *t) {
      newpages.append(pages[pIndex]);
      neworiginalPages.append(originalPages[pIndex]);

      // ADD page number

      int pageNumber = pIndex + 1;
      int digits[] = {-1, -1, -1};

      if (pageNumber < 10) {
        digits[0] = pageNumber;
      } else if (pageNumber < 100) {
        digits[0] = pageNumber % 10;
        digits[1] = pageNumber / 10;
      } else {
        digits[0] = pageNumber % 10;
        digits[1] = (pageNumber / 10) % 10;
        digits[2] = pageNumber / 100;
      }

      int totalwidth = 0;
      LineLayoutInfo lineInfo;

      for (int i = 0; i < 3; i++) {
        int digit = digits[i];
        if (digit == -1) break;

        auto& digitglyph =
            m_otlayout->glyphs[m_otlayout->glyphNamePerCode[1632 + digit]];
        GlyphLayoutInfo glyphInfo;

        glyphInfo.codepoint = 1632 + digit;
        glyphInfo.cluster = 0;
        glyphInfo.x_advance = (int)digitglyph.width + 40 << OtLayout::SCALEBY;
        glyphInfo.x_offset = 0;
        glyphInfo.y_offset = 0;
        glyphInfo.lookup_index = 0;
        glyphInfo.subtable_index = 0;
        glyphInfo.base_codepoint = 0;
        glyphInfo.lefttatweel = 0;
        glyphInfo.righttatweel = 0;

        lineInfo.glyphs.push_back(glyphInfo);

        totalwidth += glyphInfo.x_advance;
      }
      lineInfo.fontSize = emScale;
      lineInfo.ystartposition = 27400 + 200 << OtLayout::SCALEBY;
      lineInfo.xstartposition = (lineWidth - totalwidth) / 2;

      auto& curpage = newpages.last();

      curpage.append(lineInfo);

      auto& gg = neworiginalPages.last();
      gg.append(QString::number(pageNumber));
    }

    delete t;
  }
#if defined(ENABLE_PDF_GENERATION)
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);
  QString outputFileName = fileInfo.path() + "/output/overlapping.pdf";
  QuranPdfWriter allquran_overlapping(outputFileName, m_otlayout);
  allquran_overlapping.setPageLayout(pageLayout);
  allquran_overlapping.setResolution(4800 << OtLayout::SCALEBY);

  allquran_overlapping.generateQuranPages(newpages, lineWidth, neworiginalPages,
                                          emScale);
#endif
}
// taken from Qt qt/src/gui/graphicsview/qgraphicsitem.cpp
static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath& path,
                                                  const QPen& pen) {
  // We unfortunately need this hack as QPainterPathStroker will set a width
  // of 1.0 if we pass a value of 0.0 to QPainterPathStroker::setWidth()
  const qreal penWidthZero = qreal(0.00000001);
  if (path == QPainterPath() || pen == Qt::NoPen) return path;
  QPainterPathStroker ps;
  ps.setCapStyle(pen.capStyle());
  if (pen.widthF() <= 0.0)
    ps.setWidth(penWidthZero);
  else
    ps.setWidth(pen.widthF());
  ps.setJoinStyle(pen.joinStyle());
  ps.setMiterLimit(pen.miterLimit());
  QPainterPath p = ps.createStroke(path);
  p.addPath(path);
  return p;
}
void LayoutWindow::adjustOverlapping2(QList<QList<LineLayoutInfo>>& pages,
                                      int lineWidth, int beginPage, int nbPages,
                                      QVector<int>& set, double emScale,
                                      std::vector<OverlapResult>& result,
                                      bool sameLine, bool interLine) {
  if (pages.size() == 1) {
    glyphToPolys.clear();

    // fetch all gryph initially otherwise
    // mpost is not thread safe when
    // executing getAlternate
    auto tau = 1;
    for (auto& page : pages) {
      for (auto& line : page) {
        for (auto& glyph : line.glyphs) {
          auto glyphVis = m_otlayout->getGlyph(
              glyph.codepoint, {.lefttatweel = glyph.lefttatweel,
                                .righttatweel = glyph.righttatweel});
          if (glyphToPolys.find(glyphVis) == glyphToPolys.end()) {
            glyphToPolys.insert(
                {glyphVis, buildConvexPartsFromCubics(
                               getGlyphCubic(glyphVis->copiedPath), tau)});
          }
        }
      }
    }
  }
  double minDistance = 10;

  QPen pen = QPen();
  pen.setWidth(std::ceil(minDistance * emScale));

  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];

    std::vector<std::vector<QPoint>> pagePositions;
    std::vector<std::vector<GeometrySet>> geometrySets;
    bool intersection = false;

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      auto xScale = line.fontSize * line.xscale * 1.00;
      auto yScale = line.fontSize * 1.00;

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
      currentyPos = currentyPos * -1;

      pagePositions.emplace_back(std::vector<QPoint>());
      geometrySets.emplace_back(std::vector<GeometrySet>());

      auto& geoSet = geometrySets.back();
      auto& linePositions = pagePositions.back();

      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        GlyphVis& currentGlyph = *m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        currentxPos -= glyphLayout.x_advance * line.xscale;
        QPoint pos(currentxPos + (glyphLayout.x_offset * line.xscale),
                   currentyPos + (glyphLayout.y_offset));

        linePositions.emplace_back(QPoint{pos.x(), pos.y()});

        auto geo = glyphToPolys.find(&currentGlyph);
        if (geo == glyphToPolys.end()) {
          throw new std::runtime_error("glyphToPolys not found");
        }

        geoSet.emplace_back(
            geo->second.scaleTranslate(xScale, yScale, pos.x(), pos.y()));
      }
    }

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      auto xScale = line.fontSize * line.xscale * 1.00;
      auto yScale = line.fontSize * 1.00;

      QTransform pathtransform;
      pathtransform = pathtransform.scale(xScale, yScale);

      auto& linePositions = pagePositions[l];
      LineLayoutInfo suraName;

      std::vector<QPainterPath> paths;

      for (int g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];

        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        GlyphVis& currentGlyph = *m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        QPoint pos = linePositions[g];

        if (!glyphName.contains("space") && !glyphName.contains("cgj")) {
          auto gg = qt_graphicsItem_shapeFromPath(currentGlyph.path, pen);
          auto path = pathtransform.map(gg);
          path.translate(pos);
          paths.push_back(path);
        } else {
          paths.push_back({});
        }

        const auto& path = paths.back();

        const auto& geometrySet = geometrySets[l][g];

        if (glyphName.contains("space") || glyphName.contains("linefeed") ||
            !m_otlayout->glyphs.contains(glyphName))
          continue;

        // bool isIsol = glyphName.contains("isol");

        // bool isFina = glyphName.contains(".fina");

        bool isMark =
            m_otlayout->automedina->classes["marks"].contains(glyphName);

        // bool isWaqfMark =
        // m_otlayout->automedina->classes["waqfmarks"].contains(glyphName);

        // verify with the line above
        // if (l > 0 && false) {
        if (interLine && l > 0) {
          int prev_index = l - 1;
          const auto& prev_linePositions = pagePositions[prev_index];
          auto& prev_line = page[prev_index];

          for (size_t prev_g = 0; prev_g < prev_line.glyphs.size(); prev_g++) {
            auto& prev_glyphLayout = prev_line.glyphs[prev_g];
            QString prev_glyphName =
                m_otlayout->glyphNamePerCode[prev_glyphLayout.codepoint];

            bool isPrevMark = m_otlayout->automedina->classes["marks"].contains(
                prev_glyphName);
            bool isPrevrSpace = prev_glyphName.contains("space") ||
                                prev_glyphName.contains("linefeed");
            // bool isPrevIsol = prev_glyphName.contains("isol");

            if ((isMark || isPrevMark) &&
                !isPrevrSpace) {  //|| isIsol || isPrevIsol

              GlyphVis& otherGlyph = *m_otlayout->getGlyph(
                  prev_glyphName,
                  {.lefttatweel = prev_glyphLayout.lefttatweel,
                   .righttatweel = prev_glyphLayout.righttatweel,
                   .scalex =
                       line.xscaleparameter});  // m_otlayout->glyphs[prev_glyphName];

              QPoint otherpos = prev_linePositions[prev_g];

              // auto gg = qt_graphicsItem_shapeFromPath(otherGlyph.path,
              // pen); QPainterPath otherpath = pathtransform.map(gg);

              QPainterPath otherpath = pathtransform.map(otherGlyph.path);
              otherpath.translate(otherpos);

              auto otherGeometrySet = geometrySets[prev_index][prev_g];

              auto minDisPolys = minDistance * emScale;

              GSContact gsContact = getDistance(geometrySet, otherGeometrySet, minDisPolys);
              auto intersect1 = gsContact.contact.depth_or_gap < minDisPolys;
              auto intersect2 = path.intersects(otherpath);

              if (intersect1) {
                glyphLayout.color = 0xFF000000;
                prev_glyphLayout.color = 0xFF000000;
                intersection = true;
              }
              /*
              if (intersect1 != intersect2) {
                std::cout << "Difference inter line in page " << p + 1
                          << " line " << l + 1 << std::endl;
              }*/
            }
          }
        }

        if (sameLine) {
          bool isSameWord = true;

          for (int gg = g - 1; gg >= 0; gg--) {
            auto& otherglyphLayout = line.glyphs[gg];
            QString otherglyphName =
                m_otlayout->glyphNamePerCode[otherglyphLayout.codepoint];

            bool isOtherSpace = otherglyphName.contains("space") ||
                                otherglyphName.contains("linefeed");

            if (isOtherSpace) {
              isSameWord = false;
              continue;
            }

            if (isSameWord) {
              // TODO include lam.init kaf.medi for example

              bool isPrevInit = otherglyphName.contains(".init");
              bool isPrevMedi = otherglyphName.contains(".medi");

              if (glyphName.contains(".fina") && (isPrevMedi || isPrevInit))
                continue;
              if (glyphName.contains(".medi") && (isPrevMedi || isPrevInit))
                continue;
            }

            const auto& otherpath = paths[gg];
            const auto& otherGeometrySet = geometrySets[l][gg];

            auto minDisPolys = minDistance * emScale * line.fontSize;
            GSContact gsContact = getDistance(geometrySet, otherGeometrySet, minDisPolys);
            auto intersect1 = gsContact.contact.depth_or_gap < minDisPolys;
            auto intersect2 = path.intersects(otherpath);

            /*if (intersect1 != intersect2) {
              std::cout << "Difference intra line in page " << p + 1 << " line "
                        << l + 1 << std::endl;
            }*/

#if false
            if (intersect2 != intersect1) {
              auto& As = geometrySet.polys();
              auto& Bs = otherGeometrySet.polys();

              Contact test = contactGjkEpaPoly(As[gsContact.polyA], Bs[gsContact.polyB]);

              Contact test2 = contactFlexiblePoly(As[gsContact.polyA], Bs[gsContact.polyB]);

              std::vector<AABB> aabbsA(As.size()), aabbsB(Bs.size());
              for (size_t i = 0; i < As.size(); ++i) {
                aabbsA[i] = computeAABB(As[i]);
              }
              for (size_t j = 0; j < Bs.size(); ++j) {
                aabbsB[j] = computeAABB(Bs[j]);
              }

              auto geo1 = glyphToPolys.find(&currentGlyph);
              auto geo2 = glyphToPolys.find(&otherGlyph);

              auto aabb1 = geometrySet.boundingAABB();
              auto aabb2 = otherGeometrySet.boundingAABB();
              auto bRec1 = path.boundingRect();
              auto bRec2 = otherpath.boundingRect();
              std::cout << currentGlyph.name.toStdString() << " AABB1=["
                        << aabb1.minx << "," << aabb1.maxy << "," << aabb1.maxx
                        << "," << aabb1.miny << "] BRec1=["
                        << bRec1.bottomLeft().x() << ","
                        << bRec1.bottomLeft().y() << "," << bRec1.topRight().x()
                        << "," << bRec1.topRight().y() << "]" << std::endl;
              std::cout << otherGlyph.name.toStdString() << " AABB2=["
                        << aabb2.minx << "," << aabb2.maxy << "," << aabb2.maxx
                        << "," << aabb2.miny << "] bRec2=["
                        << bRec2.bottomLeft().x() << ","
                        << bRec2.bottomLeft().y() << "," << bRec2.topRight().x()
                        << "," << bRec2.topRight().y() << "]" << std::endl;

              auto geometrySetOri = geo1->second;
              auto otherGeometrySetOri = geo2->second;

              QPointF pA{gsContact.contact.pA.x, gsContact.contact.pA.y};
              QPointF pB{gsContact.contact.pB.x, gsContact.contact.pB.y};

              QPainterPath circle1;
              circle1.addEllipse(pA, 5, 5);

              QPainterPath circle2;
              circle1.addEllipse(pB, 5, 5);

              // Method 2: Using the distance formula
              qreal dx = pB.x() - pA.x();
              qreal dy = pB.y() - pA.y();
              qreal distance1 = std::sqrt(dx * dx + dy * dy);

              auto* view = new QGraphicsView();
              QGraphicsScene scene;

              scene.addPath(path);
              scene.addPath(otherpath);

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
              // for (int i = 0; i <
              // gsContact.contact.simplex.count; ++i) {
              //  auto pt = gsContact.contact.simplex.pts[i];
              //  QPainterPath pp;
              //  pp.addEllipse(QPointF(pt.a.x, pt.a.y), 1, 1);
              //  scene.addPath(pp, QPen(Qt::blue));
              //  QPainterPath pp2;
              //  pp2.addEllipse(QPointF(pt.b.x, pt.b.y), 1, 1);
              //  scene.addPath(pp2, QPen(Qt::blue));
              //}

              GeometrySet gg{std::vector<Poly>{tt}};
              GeometrySet gg2{std::vector<Poly>{tt2}};

              scene.addPath(gg.toQPainterPath(), QPen(Qt::red));

              scene.addPath(gg2.toQPainterPath(), QPen(Qt::red));

              scene.addPath(circle1, QPen(Qt::green));
              scene.addPath(circle2, QPen(Qt::green));

              QLineF line(pA, pB);

              scene.addLine(line, QPen(Qt::green));

              view->setScene(&scene);
              view->setRenderHints(QPainter::Antialiasing |
                                   QPainter::TextAntialiasing);

              view->setMinimumSize(1200, 100);

              view->scale(5, 5);

              QMessageBox box;
              box.setWindowTitle("Graphics Preview");
              box.setIcon(QMessageBox::Information);
              box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

              auto* grid = qobject_cast<QGridLayout*>(box.layout());
              int row = grid->rowCount();
              grid->addWidget(view, row, 0, 1, grid->columnCount());

              // Optional: give the box a reasonable minimum width
              box.setMinimumWidth(1500);

              // Show the message box
              box.exec();
            }
#endif

            if (intersect1) {
              glyphLayout.color = 0xFF000000;

              otherglyphLayout.color = 0xFF000000;
              intersection = true;

              OverlapResult overlap;

              overlap.pageIndex = p;
              overlap.lineIndex = l;
              overlap.nextGlyph = g;
              overlap.prevGlyph = gg;

              result.push_back(overlap);
            }
          }
        }
      }
    }

    if (intersection) {
      set.append(p);
    }
  }
}