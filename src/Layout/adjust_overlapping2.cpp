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

class Timer {
  static std::mutex mtx;
  static std::map<std::string, int64_t> ms_Counts;
  static std::map<std::string, double> ms_Times;
  const std::string& m_sName;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_tmStart;

 public:
  // When constructed, save the name and current clock time
  Timer(const std::string& sName) : m_sName(sName) {
    m_tmStart = std::chrono::high_resolution_clock::now();
  }
  ~Timer() {
    auto tmNow = std::chrono::high_resolution_clock::now();
    auto msElapsed = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tmNow - m_tmStart);
    mtx.lock();
    ms_Counts[m_sName]++;
    ms_Times[m_sName] += msElapsed.count();
    mtx.unlock();
  }
  static void dump() {
    std::cerr << "Name\t\t\tCount\t\t\tTime(ms)\t\tAverage(ms)\n";
    std::cerr << "------------------------------------------------------------------------------------------------\n ";
    for (const auto& it : ms_Times) {
      auto iCount = ms_Counts[it.first];
      auto milli = it.second;
      std::cerr
          << it.first << "\t\t\t" << iCount << "\t\t\t" << milli << "\t\t\t" << milli / iCount << "\n";
    }
  }
};
std::mutex Timer::mtx;
std::map<std::string, int64_t> Timer::ms_Counts;
std::map<std::string, double> Timer::ms_Times;

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
                             getGlyphCubic(glyphVis->copiedPath), CUBIC_FLATNESS_TOLERANCE)});
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

  // Timer::dump();

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

struct GlyphInfo {
  GeometrySet geometrySet;
  GlyphVis* glyphVis;
  QString& glyphName;
  bool isInit;
  bool isMedi;
  bool isFina;
  bool isMark;
};

inline AABB padAABB(const AABB& b, double pad) {
  return {
      b.minx - pad,
      b.miny - pad,
      b.maxx + pad,
      b.maxy + pad};
}

struct Event {
  double x;
  int type;  // +1 = ENTER, -1 = EXIT
  int id;    // rectangle id
};

struct GlyphInfoForInt {
  AABB aabb;
  size_t lineIdx;
  int wordIdx;
  size_t glyphIdx;
};

struct Intersection {
  const GlyphInfoForInt& first;
  const GlyphInfoForInt& second;
};

// For active set; we just store ids here.
struct ByY {
  const std::vector<GlyphInfoForInt>* rects;
  bool operator()(int a, int b) const {
    if (a == b) return false;
    const AABB& ra = (*rects)[a].aabb;
    const AABB& rb = (*rects)[b].aabb;
    if (ra.miny < rb.miny) return true;
    if (ra.miny > rb.miny) return false;
    return a < b;  // tie-breaker for strict weak ordering
  }
};

// Check 1D interval overlap on y axis
inline bool yOverlap(const AABB& a, const AABB& b) {
  // Strict overlap (no mere edge touching)
  // return (a.y1 < b.y2) && (b.y1 < a.y2);

  // If you want to treat edge touching as intersection, use:
  // return !(a.y2 < b.y1 || b.y2 < a.y1);

  return (a.miny < b.maxy) && (b.miny < a.maxy);
}

void findIntersections(
    OtLayout* layout,
    QList<QList<LineLayoutInfo>>& pages,
    int pageIndex,
    const std::vector<std::vector<GlyphInfo>>& glyphs,
    double minDistance,
    std::vector<OverlapResult>& result,
    QVector<int>& set) {
  double pad_ = minDistance / 2;

  auto& page = pages[pageIndex];

  QString spaceName("space");
  QString linefeedName("linefeed");

  int glyphCount = 0;

  for (auto& line : glyphs) {
    glyphCount += line.size();
  }

  std::vector<GlyphInfoForInt> rectsInput;

  rectsInput.reserve(glyphCount);
  for (size_t i = 0; i < glyphs.size(); ++i) {
    auto& line = glyphs[i];
    auto wordId = 0;
    for (size_t j = 0; j < line.size(); ++j) {
      auto& glyphInfo = line[j];
      bool isSpace = glyphInfo.glyphName.contains(spaceName) || glyphInfo.glyphName.contains(linefeedName);
      if (isSpace) {
        ++wordId;
      } else {
        AABB raw = glyphInfo.geometrySet.boundingAABB();
        AABB box = padAABB(raw, pad_);
        rectsInput.push_back(GlyphInfoForInt{box, i, wordId, j});
      }
    }
  }

  int n = (int)rectsInput.size();
  if (n == 0) return;

  // Build events
  std::vector<Event> events;
  events.reserve(2 * n);
  for (int i = 0; i < n; ++i) {
    events.push_back(Event{rectsInput[i].aabb.minx, +1, i});  // ENTER
    events.push_back(Event{rectsInput[i].aabb.maxx, -1, i});  // EXIT
  }

  // Sort events by x; on tie, EXIT (-1) before ENTER (+1)
  sort(events.begin(), events.end(),
       [](const Event& a, const Event& b) {
         if (a.x < b.x) return true;
         if (a.x > b.x) return false;
         return a.type < b.type;  // -1 before +1
       });

  // Active set; ordered by y1
  ByY comp{&rectsInput};
  std::set<int, ByY> active(comp);

  // Output adjacency: intersections for each rectangle
  std::vector<Intersection> intersections;
  intersections.reserve(n);

  for (const Event& e : events) {
    int id = e.id;
    const auto& firstGlyph = rectsInput[id];
    const AABB& R = firstGlyph.aabb;

    if (e.type == -1) {
      auto it = active.find(id);
      if (it != active.end())
        active.erase(it);
    } else {
      // TODO Can be optimized. Sort by y and test only adjacents.
      // Takes about 90/750 ms for the whole Mushaf in Release
      {
        // Timer t(std::string("Active LOOP"));
        for (int j : active) {
          const auto& secondGlyph = rectsInput[j];
          const AABB& S = secondGlyph.aabb;

          if (yOverlap(R, S)) {
            if (firstGlyph.lineIdx > secondGlyph.lineIdx) {
              intersections.push_back({secondGlyph, firstGlyph});
            } else if (firstGlyph.lineIdx == secondGlyph.lineIdx) {
              if (firstGlyph.glyphIdx > secondGlyph.glyphIdx) {
                intersections.push_back({secondGlyph, firstGlyph});
              } else {
                intersections.push_back({firstGlyph, secondGlyph});
              }
            } else {
              intersections.push_back({firstGlyph, secondGlyph});
            }
          }
        }
      }

      active.insert(id);
    }
  }

  auto isIntersection = false;

  for (auto& intersection : intersections) {
    const auto& firstGlyph = intersection.first;
    const auto& secondGlyph = intersection.second;
    auto isSameLine = firstGlyph.lineIdx == secondGlyph.lineIdx;
    auto& glyphInfo1 = glyphs[firstGlyph.lineIdx][firstGlyph.glyphIdx];
    auto& glyphInfo2 = glyphs[secondGlyph.lineIdx][secondGlyph.glyphIdx];
    if (isSameLine) {
      auto isSameWord = isSameLine && (firstGlyph.wordIdx == secondGlyph.wordIdx);
      if (isSameWord && (glyphInfo2.isMedi || glyphInfo2.isFina) && (glyphInfo1.isInit || glyphInfo1.isMedi)) {
        continue;
      }

      auto& line = page[firstGlyph.lineIdx];

      auto minDisPolys = minDistance * line.fontSize;
      GSContact gsContact = getDistance(glyphInfo1.geometrySet, glyphInfo2.geometrySet, minDisPolys);
      auto intersect = gsContact.contact.depth_or_gap < minDisPolys;
      if (intersect) {
        line.glyphs[firstGlyph.glyphIdx].color = 0xFF000000;

        line.glyphs[secondGlyph.glyphIdx].color = 0xFF000000;
        isIntersection = true;

        OverlapResult overlap;

        overlap.pageIndex = pageIndex;
        overlap.lineIndex = firstGlyph.lineIdx;
        overlap.prevGlyph = firstGlyph.glyphIdx;
        overlap.nextGlyph = secondGlyph.glyphIdx;

        result.push_back(overlap);
      }
    } else {
      if (glyphInfo1.isMark || glyphInfo2.isMark) {
        auto minDisPolys = minDistance;

        GSContact gsContact = getDistance(glyphInfo1.geometrySet, glyphInfo2.geometrySet, minDisPolys);
        auto intersect = gsContact.contact.depth_or_gap < minDisPolys;

        if (intersect) {
          auto& line = page[firstGlyph.lineIdx];
          auto& secondLine = page[secondGlyph.lineIdx];
          line.glyphs[firstGlyph.glyphIdx].color = 0xFF000000;

          secondLine.glyphs[secondGlyph.glyphIdx].color = 0xFF000000;
          isIntersection = true;
        }
      }
    }
  }
  if (isIntersection) {
    set.append(pageIndex);
  }
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

    for (auto& page : pages) {
      for (auto& line : page) {
        for (auto& glyph : line.glyphs) {
          auto glyphVis = m_otlayout->getGlyph(
              glyph.codepoint, {.lefttatweel = glyph.lefttatweel,
                                .righttatweel = glyph.righttatweel});
          if (glyphToPolys.find(glyphVis) == glyphToPolys.end()) {
            glyphToPolys.insert(
                {glyphVis, buildConvexPartsFromCubics(
                               getGlyphCubic(glyphVis->copiedPath), CUBIC_FLATNESS_TOLERANCE)});
          }
        }
      }
    }
  }
  double minDistance = 10;

  auto& markClass = m_otlayout->automedina->classes["marks"];
  QString spaceName("space");
  QString linefeedName("linefeed");
  QString initName(".init");
  QString mediName(".medi");
  QString finaName(".fina");

  // QPen pen = QPen();
  // pen.setWidth(std::ceil(minDistance * emScale));

  for (int p = beginPage; p < beginPage + nbPages; p++) {
    auto& page = pages[p];

    // std::vector<std::vector<QPoint>> pagePositions;
    std::vector<std::vector<GlyphInfo>> geometrySets;
    bool intersection = false;

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      auto xScale = line.fontSize * line.xscale;
      auto yScale = line.fontSize;

      int currentxPos = -line.xstartposition;
      int currentyPos = line.ystartposition - (OtLayout::TopSpace << OtLayout::SCALEBY);
      currentyPos = currentyPos * -1;

      geometrySets.emplace_back(std::vector<GlyphInfo>());

      auto& geoSet = geometrySets.back();

      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];

        QString& glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        auto currentGlyph = m_otlayout->getGlyph(
            glyphName, {.lefttatweel = glyphLayout.lefttatweel,
                        .righttatweel = glyphLayout.righttatweel,
                        .scalex = line.xscaleparameter});
        currentxPos -= glyphLayout.x_advance * line.xscale;
        QPoint pos(currentxPos + (glyphLayout.x_offset * line.xscale),
                   currentyPos + (glyphLayout.y_offset));

        auto geo = glyphToPolys.find(currentGlyph);
        if (geo == glyphToPolys.end()) {
          throw new std::runtime_error("glyphToPolys not found");
        }
        bool isInit = glyphName.contains(initName);
        bool isMedi = glyphName.contains(mediName);
        bool isFina = glyphName.contains(finaName);
        auto isMark = markClass.contains(glyphName);
        if (xScale == 1 && yScale == 1) {
          geoSet.emplace_back(
              GlyphInfo{
                  geo->second.translate(pos.x(), pos.y()),
                  currentGlyph,
                  glyphName,
                  isInit, isMedi, isFina, isMark});
        } else {
          geoSet.emplace_back(
              GlyphInfo{geo->second.scaleTranslate(xScale, yScale, pos.x(), pos.y()), currentGlyph, glyphName, isInit, isMedi, isFina, isMark});
        }
      }
    }

    findIntersections(m_otlayout, pages, p, geometrySets, minDistance * emScale, result, set);
#if false

    continue;

    for (int l = 0; l < page.size(); l++) {
      auto& line = page[l];

      for (size_t g = 0; g < line.glyphs.size(); g++) {
        auto& glyphLayout = line.glyphs[g];
        const auto& geometrySet = geometrySets[l][g];

        QString& glyphName = geometrySet.glyphName;  // m_otlayout->glyphNamePerCode[glyphLayout.codepoint];

        if (glyphName.contains(spaceName) || glyphName.contains(linefeedName) || !m_otlayout->glyphs.contains(glyphName))
          continue;

        bool isMark =
            markClass.contains(glyphName);

        if (interLine && l > 0) {
          int prev_index = l - 1;

          auto& prev_line = page[prev_index];

          for (size_t prev_g = 0; prev_g < prev_line.glyphs.size(); prev_g++) {
            auto& prev_glyphLayout = prev_line.glyphs[prev_g];
            const auto& otherGeometrySet = geometrySets[prev_index][prev_g];

            auto& prev_glyphName = otherGeometrySet.glyphName;  // m_otlayout->glyphNamePerCode[prev_glyphLayout.codepoint];

            bool isPrevMark = markClass.contains(prev_glyphName);
            bool isPrevrSpace = prev_glyphName.contains(spaceName) || prev_glyphName.contains(linefeedName);

            if ((isMark || isPrevMark) && !isPrevrSpace) {
              auto minDisPolys = minDistance * emScale;

              GSContact gsContact = getDistance(geometrySet.geometrySet, otherGeometrySet.geometrySet, minDisPolys);
              auto intersect1 = gsContact.contact.depth_or_gap < minDisPolys;

              if (intersect1) {
                glyphLayout.color = 0xFF000000;
                prev_glyphLayout.color = 0xFF000000;
                intersection = true;
              }
            }
          }
        }

        if (sameLine) {
          bool isSameWord = true;

          for (int gg = g - 1; gg >= 0; gg--) {
            auto& otherglyphLayout = line.glyphs[gg];
            const auto& otherGeometrySet = geometrySets[l][gg];
            auto& otherglyphName = otherGeometrySet.glyphName;  // m_otlayout->glyphNamePerCode[otherglyphLayout.codepoint];

            bool isOtherSpace = otherglyphName.contains(spaceName) ||
                                otherglyphName.contains(linefeedName);

            if (isOtherSpace) {
              isSameWord = false;
              continue;
            }

            if (isSameWord) {
              if ((geometrySet.isMedi || geometrySet.isFina) && (otherGeometrySet.isInit || otherGeometrySet.isMedi))
                continue;
            }

            auto minDisPolys = minDistance * emScale * line.fontSize;
            GSContact gsContact = getDistance(geometrySet.geometrySet, otherGeometrySet.geometrySet, minDisPolys);
            auto intersect1 = gsContact.contact.depth_or_gap < minDisPolys;

#if false
            if (intersect1) {
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

              GlyphVis& otherGlyph = *m_otlayout->getGlyph(
                  otherglyphName,
                  {.lefttatweel = otherglyphLayout.lefttatweel,
                   .righttatweel = otherglyphLayout.righttatweel,
                   .scalex =
                       line.xscaleparameter});  // m_otlayout->glyphs[prev_glyphName];

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

              view->setMinimumSize(1500, 1000);

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
              box.setMinimumHeight(1200);

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
#endif
  }
}