
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
#include "qpoint.h"

#if defined(ENABLE_PDF_GENERATION)
#include "Pdf/quranpdfwriter.h"
#endif

#include <Subtable.h>
#include <math.h>

#include <QPrinter>
#include <iostream>
#include <set>
#include <unordered_set>

#include "Export/ExportToHTML.h"
#include "Export/GenerateLayout.h"
#include "gllobal_strings.h"
#include "to_opentype.h"

// ---------- overlap helpers ----------

// Largest k ≥ 1 such that suffix_k(A) == prefix_k(B). Returns 0 if none.
static int maxOverlapSuffixPrefix(const QVector<int>& A,
                                  const QVector<int>& B) {
  const int maxK = std::min(A.size(), B.size());
  for (int k = maxK; k >= 1; --k) {
    bool ok = true;
    for (int i = 0; i < k; ++i) {
      if (A[A.size() - k + i] != B[i]) {
        ok = false;
        break;
      }
    }
    if (ok) return k;
  }
  return 0;
}

// Conflict if ANY suffix/prefix overlap (either direction) exists.
static inline bool conflicts(const QVector<int>& a, const QVector<int>& b) {
  return maxOverlapSuffixPrefix(a, b) > 0 || maxOverlapSuffixPrefix(b, a) > 0;
}

// ---------- graph ----------

struct Graph {
  int n = 0;
  QVector<QVector<int>> adj;  // adjacency list
  QVector<int> degree;        // degrees
};

static Graph buildConflictGraph(const QVector<QVector<int>>& seqs) {
  const int n = seqs.size();
  Graph g;
  g.n = n;
  g.adj.resize(n);
  g.degree.resize(n);

  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (conflicts(seqs[i], seqs[j])) {
        g.adj[i].push_back(j);
        g.adj[j].push_back(i);
        g.degree[i]++;
        g.degree[j]++;
      }
    }
  }
  return g;
}

// ---------- Exact DSATUR (branch & bound) ----------

struct DSATURState {
  const Graph* g = nullptr;
  QVector<int> color;      // -1 = uncolored
  QVector<int> satDegree;  // number of distinct neighbor colors
  QVector<QVector<bool>>
      seenCol;  // seenCol[v][c] true if neighbor of v has color c
  int usedColors = 0;
  int bestColors = std::numeric_limits<int>::max();
  QVector<int> bestColoring;
};

static int pickNextVertexExact(const DSATURState& S) {
  int pick = -1, bestSat = -1, bestDeg = -1;
  for (int v = 0; v < S.g->n; ++v) {
    if (S.color[v] != -1) continue;
    int sd = S.satDegree[v];
    int deg = S.g->degree[v];
    if (sd > bestSat || (sd == bestSat && deg > bestDeg)) {
      bestSat = sd;
      bestDeg = deg;
      pick = v;
    }
  }
  return pick;
}

static void assignColor(DSATURState& S, int v, int c,
                        QVector<int>& touchedSat) {
  S.color[v] = c;
  for (int nb : S.g->adj[v]) {
    if (S.color[nb] == -1 && !S.seenCol[nb][c]) {
      S.seenCol[nb][c] = true;
      S.satDegree[nb] += 1;
      touchedSat.push_back(nb);
    }
  }
}

static void unassignColor(DSATURState& S, int v, int c,
                          const QVector<int>& touchedSat) {
  for (int nb : touchedSat) {
    S.seenCol[nb][c] = false;
    S.satDegree[nb] -= 1;
  }
  S.color[v] = -1;
}

static void dsaturBacktrack(DSATURState& S) {
  if (S.usedColors >= S.bestColors) return;

  int v = pickNextVertexExact(S);
  if (v == -1) {
    S.bestColors = std::min(S.bestColors, S.usedColors);
    S.bestColoring = S.color;
    return;
  }

  // Try existing colors
  for (int c = 0; c < S.usedColors; ++c) {
    bool ok = true;
    for (int nb : S.g->adj[v])
      if (S.color[nb] == c) {
        ok = false;
        break;
      }
    if (!ok) continue;

    QVector<int> touched;
    assignColor(S, v, c, touched);
    dsaturBacktrack(S);
    unassignColor(S, v, c, touched);
  }

  // Try a new color
  {
    int newC = S.usedColors;
    QVector<int> touched;
    S.usedColors += 1;
    for (int i = 0; i < S.g->n; ++i)
      if (S.color[i] == -1 && S.seenCol[i].size() < S.usedColors)
        S.seenCol[i].resize(S.usedColors);
    assignColor(S, v, newC, touched);
    dsaturBacktrack(S);
    unassignColor(S, v, newC, touched);
    S.usedColors -= 1;
  }
}

static QVector<int> colorMinExact(const Graph& g) {
  DSATURState S;
  S.g = &g;
  S.color = QVector<int>(g.n, -1);
  S.satDegree = QVector<int>(g.n, 0);
  S.seenCol = QVector<QVector<bool>>(g.n, QVector<bool>(0));
  S.usedColors = 0;
  S.bestColors = std::numeric_limits<int>::max();

  // Start from a highest-degree vertex with color 0 (accelerates convergence)
  int start = 0;
  for (int i = 1; i < g.n; ++i)
    if (g.degree[i] > g.degree[start]) start = i;

  {
    QVector<int> touched;
    for (int i = 0; i < g.n; ++i) S.seenCol[i].resize(1);
    S.usedColors = 1;
    assignColor(S, start, 0, touched);
    dsaturBacktrack(S);
    unassignColor(S, start, 0, touched);
    S.usedColors = 0;
  }

  return S.bestColoring;
}

// ---------- Fast Greedy DSATUR (no backtracking) ----------

static int pickNextVertexGreedy(const QVector<int>& color,
                                const QVector<int>& satDegree, const Graph& g) {
  int pick = -1, bestSat = -1, bestDeg = -1;
  for (int v = 0; v < g.n; ++v) {
    if (color[v] != -1) continue;
    int sd = satDegree[v];
    int deg = g.degree[v];
    if (sd > bestSat || (sd == bestSat && deg > bestDeg)) {
      bestSat = sd;
      bestDeg = deg;
      pick = v;
    }
  }
  return pick;
}

static QVector<int> colorGreedyDSATUR(const Graph& g) {
  QVector<int> color(g.n, -1);
  QVector<int> satDegree(g.n, 0);
  QVector<QVector<bool>> seenCol(g.n, QVector<bool>(0));
  int usedColors = 0;

  // Initialize order by degree for first pick (optional)
  int start = 0;
  for (int i = 1; i < g.n; ++i)
    if (g.degree[i] > g.degree[start]) start = i;

  // Give the start vertex color 0
  color[start] = 0;
  usedColors = 1;
  for (int i = 0; i < g.n; ++i) seenCol[i].resize(1);
  for (int nb : g.adj[start]) {
    if (color[nb] == -1 && !seenCol[nb][0]) {
      seenCol[nb][0] = true;
      satDegree[nb] += 1;
    }
  }

  // Greedy loop: pick max saturation, assign lowest feasible color
  for (int colored = 1; colored < g.n; ++colored) {
    int v = pickNextVertexGreedy(color, satDegree, g);
    if (v == -1) break;  // all colored

    // Find lowest available color
    int c;
    for (c = 0; c < usedColors; ++c) {
      bool ok = true;
      for (int nb : g.adj[v])
        if (color[nb] == c) {
          ok = false;
          break;
        }
      if (ok) break;
    }
    if (c == usedColors) {
      // need a new color
      usedColors++;
      for (int i = 0; i < g.n; ++i)
        if (color[i] == -1 && seenCol[i].size() < usedColors)
          seenCol[i].resize(usedColors);
    }

    color[v] = c;
    // update neighbors' saturation sets
    for (int nb : g.adj[v]) {
      if (color[nb] == -1 && !seenCol[nb][c]) {
        seenCol[nb][c] = true;
        satDegree[nb] += 1;
      }
    }
  }

  return color;
}

// ---------- Public APIs ----------

// Exact or greedy based on flag
QVector<QVector<QVector<int>>> partitionMinNoOverlap(
    const QVector<QVector<int>>& seqs, bool useGreedy /* = false */) {
  if (seqs.size() == 0) return {};
  Graph g = buildConflictGraph(seqs);
  QVector<int> col = useGreedy ? colorGreedyDSATUR(g) : colorMinExact(g);

  int maxColor = -1;
  for (int c : col) maxColor = std::max(maxColor, c);
  const int numGroups = (maxColor >= 0) ? (maxColor + 1) : 0;

  QVector<QVector<QVector<int>>> groups(numGroups);
  for (int i = 0; i < seqs.size(); ++i) {
    int c = col[i];
    if (c >= 0) groups[c].push_back(seqs[i]);
  }
  return groups;
}

// Convenience: auto-switch to greedy past a size threshold
QVector<QVector<QVector<int>>> partitionMinNoOverlapAuto(
    const QVector<QVector<int>>& seqs, int autoGreedyThreshold = 120) {
  const bool useGreedy = (seqs.size() > autoGreedyThreshold);
  return partitionMinNoOverlap(seqs, useGreedy);
}

void LayoutWindow::generateOverlapLookups(
    const QList<QList<LineLayoutInfo>>& pages,
    const QList<QStringList>& originalPages,
    const QVector<OverlapResult>& result) {
  auto path = m_font->filePath();
  QFileInfo fileInfo = QFileInfo(path);

  QString outputFileName = fileInfo.path() + "/output/overlaps.txt";
  QFile file(outputFileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);  // we will serialize the data into the file
  out.setCodec("UTF-8");

  QString featureFileName = fileInfo.path() + "/output/adjustoverlap.fea";
  QFile featureFile(featureFileName);
  featureFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream featureOut(
      &featureFile);  // we will serialize the data into the file
  featureOut.setCodec("UTF-8");

  auto& basesClass = m_otlayout->automedina->classes["bases"];
  auto& marksClass = m_otlayout->automedina->classes["marks"];

  QString innerLookupsString;

  QTextStream innerlookups{&innerLookupsString};

  QSet<QVector<int>> spaceSequences;
  QSet<QVector<int>> betweenBasesSequences;
  QSet<QVector<int>> wordMarksSequences;
  std::set<QString> spaceWords;
  std::set<QString> betweenBasesWords;
  std::set<QString> wordMarksWords;

  QMap<QVector<int>, QSet<QString>> wordByBases;

  struct GlyphPos {
    QSet<quint16> set;
    QString lookupName;
  };

  struct MainLookup {
    QVector<QVector<GlyphPos>> posSubtables;
    QString name;
  };

  QVector<MainLookup> mainLookups;

  auto comp = [](QString a, QString b) {
    if (a.length() == b.length())
      return a < b;
    else
      return a.length() < b.length();
  };
  std::map<QString, std::unordered_set<int>, decltype(comp)> subLookupKerns(
      comp);

  int lastsubLookupNumber = 0;

  bool mergeWithExisting = true;

  if (mergeWithExisting) {
    for (auto i = m_otlayout->lookupsIndexByName.cbegin(),
              end = m_otlayout->lookupsIndexByName.cend();
         i != end; ++i) {
      auto lookupName = i.key();

      if (lookupName != "adjustoverlap" &&
          !lookupName.startsWith("adjustoverlap.main"))
        continue;

      mainLookups.push_back({});

      MainLookup& mainLookup = mainLookups.back();
      mainLookup.name = lookupName;

      auto adjustoverlapLookup = m_otlayout->lookups[i.value()];
      for (auto subtable : adjustoverlapLookup->subtables) {
        // SingleAdjustmentSubtable* kernTable =
        // dynamic_cast<SingleAdjustmentSubtable*>(subtable);
        ChainingSubtable* chainingSubtable =
            dynamic_cast<ChainingSubtable*>(subtable);

        QVector<GlyphPos> positions;

        for (int pos = 0; pos < chainingSubtable->compiledRule.input.size();
             pos++) {
          GlyphPos glyphPos{chainingSubtable->compiledRule.input[pos], {}};
          for (auto& lookupRecord :
               chainingSubtable->compiledRule.lookupRecords) {
            if (lookupRecord.position == pos) {
              glyphPos.lookupName = lookupRecord.lookupName;
              break;
            }
          }
          positions.append(glyphPos);
        }

        mainLookup.posSubtables.append(positions);
        for (auto& lookupRecord :
             chainingSubtable->compiledRule.lookupRecords) {
          if (!lookupRecord.lookupName.isEmpty()) {
            SingleAdjustmentSubtable* kernTable = dynamic_cast<
                SingleAdjustmentSubtable*>(
                m_otlayout
                    ->lookups[m_otlayout
                                  ->lookupsIndexByName[lookupRecord.lookupName]]
                    ->subtables[0]);
            if (subLookupKerns.find(lookupRecord.lookupName) ==
                subLookupKerns.end()) {
              int number =
                  std::stoi(lookupRecord.lookupName.mid(15).toStdString());
              auto pair = subLookupKerns.insert({lookupRecord.lookupName, {}});
              auto& res = *pair.first;
              for (auto codepoint : kernTable->singlePos.keys()) {
                res.second.insert(codepoint);
              }
              if (number > lastsubLookupNumber) {
                lastsubLookupNumber = number;
              }
            }
          }
        }
      }
    }
  }
  int subLookupNumber = lastsubLookupNumber + 1;

  QString subLookups;

  for (auto overlap : result) {
    auto& page = pages[overlap.pageIndex];
    auto& line = page[overlap.lineIndex];
    auto& text = originalPages[overlap.pageIndex][overlap.lineIndex];

    auto& prevGlyphLayout = line.glyphs[overlap.prevGlyph];
    QString prevGlyphName =
        m_otlayout->glyphNamePerCode[prevGlyphLayout.codepoint];

    auto& nextGlyphLayout = line.glyphs[overlap.nextGlyph];
    QString nextGlyphName =
        m_otlayout->glyphNamePerCode[nextGlyphLayout.codepoint];

    bool betweenBases = basesClass.contains(prevGlyphName) &&
                        basesClass.contains(nextGlyphName);

    QVector<int> basesIndexes;

    int prevBaseIndex;
    for (prevBaseIndex = overlap.prevGlyph; prevBaseIndex >= 0;
         prevBaseIndex--) {
      auto& glyphLayout = line.glyphs[prevBaseIndex];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (basesClass.contains(glyphName)) {
        break;
      }
    }

    basesIndexes.append(prevBaseIndex);

    for (int nextBaseIndex = prevBaseIndex + 1;
         nextBaseIndex <= overlap.nextGlyph; nextBaseIndex++) {
      auto& glyphLayout = line.glyphs[nextBaseIndex];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (basesClass.contains(glyphName)) {
        basesIndexes.append(nextBaseIndex);
      }
    }

    int lastIndex =
        overlap.nextGlyph;  // basesIndexes.last() > overlap.nextGlyph ?
                            // basesIndexes.last() : overlap.nextGlyph;

    QVector<int> sequence;

    bool containsSpace = false;

    for (int i = basesIndexes.first(); i <= lastIndex; i++) {
      auto& glyphLayout = line.glyphs[i];
      sequence.append(glyphLayout.codepoint);
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        containsSpace = true;
      }
    }

    if (containsSpace) {
      if (spaceSequences.contains(sequence)) continue;
      spaceSequences.insert(sequence);
    } else if (betweenBases) {
      if (betweenBasesSequences.contains(sequence)) continue;
      betweenBasesSequences.insert(sequence);
    } else {
      if (wordMarksSequences.contains(sequence)) continue;
      wordMarksSequences.insert(sequence);
    }

    // generate word

    int startCluster = 0;
    int endCluster = text.size();

    for (int i = overlap.prevGlyph; i >= 0; i--) {
      auto& glyphLayout = line.glyphs[i];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        startCluster = glyphLayout.cluster + 1;
        break;
      }
    }
    for (int i = overlap.nextGlyph; i < line.glyphs.size(); i++) {
      auto& glyphLayout = line.glyphs[i];
      QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
      if (glyphName.contains("space")) {
        endCluster = glyphLayout.cluster;
        break;
      }
    }

    QString word = text.mid(startCluster, endCluster - startCluster);

    if (containsSpace) {
      if (spaceWords.find(word) == spaceWords.end()) {
        spaceWords.insert(word);
      }
    } else if (betweenBases) {
      if (betweenBasesWords.find(word) == betweenBasesWords.end()) {
        betweenBasesWords.insert(word);
      }
    } else if (wordMarksWords.find(word) == wordMarksWords.end()) {
      wordMarksWords.insert(word);
    }

    if (containsSpace || betweenBases) {
      std::cout << "pos";
      for (int glyphIndex = basesIndexes.first(); glyphIndex <= lastIndex;
           glyphIndex++) {
        auto& glyphLayout = line.glyphs[glyphIndex];
        sequence.append(glyphLayout.codepoint);
        QString glyphName = m_otlayout->glyphNamePerCode[glyphLayout.codepoint];
        std::cout << " " << glyphName.toStdString() << "'";
      }
      std::cout << "; # page " << overlap.pageIndex + 1 << " line "
                << overlap.lineIndex + 1 << " " << word.toStdString()
                << std::endl;
      continue;
    }

    QVector<int> baseCodepoints;

    for (auto baseIndex : basesIndexes) {
      auto& glyphLayout = line.glyphs[baseIndex];
      baseCodepoints.append(glyphLayout.codepoint);
    }

    auto tt = wordByBases.find(baseCodepoints);
    if (tt == wordByBases.end()) {
      wordByBases.insert(baseCodepoints, {word});
    } else {
      tt->insert(word);
    }
  }
  QVector<QVector<int>> myVector(wordMarksSequences.begin(),
                                 wordMarksSequences.end());

  auto wordMarksSequenceGroups = partitionMinNoOverlap(myVector, true);

  for (auto& group : wordMarksSequenceGroups) {
    mainLookups.push_back({});
    MainLookup& mainLookup = mainLookups.back();
    mainLookup.name = QString("adjustoverlap.main%1").arg(mainLookups.size());
    for (auto& sequence : group) {
      int seqLength = sequence.size();

      bool alreadyExist = false;

      for (auto& mainLookup : mainLookups) {
        for (auto& posSubtable : mainLookup.posSubtables) {
          if (posSubtable.size() == seqLength) {
            alreadyExist = true;
            for (int i = 0; i < seqLength; i++) {
              if (!posSubtable[i].set.contains(sequence[i])) {
                alreadyExist = false;
                break;
              }
            }
            if (alreadyExist) break;
          }
        }
        if (alreadyExist) break;
      }

      if (alreadyExist) continue;

      QVector<GlyphPos> posSubtable;
      for (int i = 0; i < seqLength; i++) {
        auto codepoint = sequence[i];
        GlyphPos glyphPos;

        glyphPos.set.insert(codepoint);

        auto find = false;
        for (auto& sublookup : subLookupKerns) {
          auto res = sublookup.second.find(codepoint);
          if (res == sublookup.second.end()) {
            glyphPos.lookupName = sublookup.first;
            find = true;
            sublookup.second.insert(codepoint);
            break;
          }
        }
        if (!find) {
          glyphPos.lookupName =
              QString("adjustoverlap.l%1").arg(subLookupNumber++);
          subLookupKerns.insert({glyphPos.lookupName, {codepoint}});
        }
        posSubtable.append(glyphPos);
      }

      mainLookup.posSubtables.append(posSubtable);
    }
    if (mainLookup.posSubtables.size() == 0) {
      mainLookups.removeLast();
    }
  }

  // Output

  out << "**************************************inner mark "
         "collisions**************\n";
  int nbWordbyline = 0;
  for (auto& word : wordMarksWords) {
    if (nbWordbyline == 8) {
      out << '\n';
      nbWordbyline = 0;
    } else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;
  }

  out << "\n**************************************words between "
         "bases**************\n";

  nbWordbyline = 0;
  for (auto& word : betweenBasesWords) {
    if (nbWordbyline == 6) {
      out << '\n';
      nbWordbyline = 0;
    } else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;
  }

  out << "\n**************************************otherWords**************\n";

  nbWordbyline = 0;
  for (auto& word : spaceWords) {
    if (nbWordbyline == 6) {
      out << '\n';
      nbWordbyline = 0;
    } else if (nbWordbyline != 0) {
      out << ' ';
    }
    out << word;
    nbWordbyline++;
  }

  out << "\n**************************************one base**************\n";

  for (auto i = wordByBases.cbegin(), end = wordByBases.cend(); i != end; ++i) {
    if (i.key().size() == 1) {
      for (auto& word : i.value()) {
        out << word << ' ';
      }
      out << '\n';
    }
  }

  out << "\n**************************************two or more "
         "bases**************\n";

  for (auto i = wordByBases.cbegin(), end = wordByBases.cend(); i != end; ++i) {
    if (i.key().size() != 1) {
      for (auto& word : i.value()) {
        out << word << ' ';
      }
      out << '\n';
    }
  }

  /*
  out << "\n**************************************containing
  sequences**************\n";

  QList<QVector<int>> seqsList = sequences.values();

  for (int i = 0; i < seqsList.size(); ++i) {
    for (int j = 0; j < seqsList.size(); ++j) {
        if (i == j) continue;
        if (isSubsequence(seqsList[j], seqsList[i])) {
          out << "Sequence " ;
          for(auto codepoint : seqsList[i]){
            QString glyphName = m_otlayout->glyphNamePerCode[codepoint];
            out << glyphName << " ";
          }
          out << "is contained in " ;
          for(auto codepoint : seqsList[j]){
            QString glyphName = m_otlayout->glyphNamePerCode[codepoint];
            out << glyphName << " ";
          }
          out << '\n';
        }
    }
  }*/

  // generate features

  for (auto& subLookupKern : subLookupKerns) {
    auto lookupName =
        subLookupKern
            .first;  // QString("adjustoverlap.l%1").arg(subLookupKern.first);
    QString sublookup = "  lookup " + lookupName + " {\n";
    for (auto& codepoint : subLookupKern.second) {
      QString glyphName = m_otlayout->glyphNamePerCode[codepoint];
      sublookup += "    pos [" + glyphName + "] <0 0 0 0>;\n";
    }
    sublookup += "  } " + lookupName + ";\n";
    subLookups += sublookup;
  }

  QVector<QString> mainLookupStrings;

  for (auto& lookup : mainLookups) {
    std::sort(lookup.posSubtables.begin(), lookup.posSubtables.end(),
              [](const QVector<GlyphPos>& a, const QVector<GlyphPos>& b) {
                return a.size() > b.size();
              });
    QString mainLookup;
    mainLookup = QString("  lookup %1 {\n    feature mkmk;\n").arg(lookup.name);
    for (auto& posSubtable : lookup.posSubtables) {
      QString posLine = "    pos";
      QString debugLine = "pos";
      for (auto glyphPos : posSubtable) {
        // if (glyphPos.set.size() > 1) {
        posLine += " [";
        debugLine += " [";
        //}

        for (auto glyph : glyphPos.set) {
          QString glyphName = m_otlayout->glyphNamePerCode[glyph];
          // posLine += " /^" + glyphName + "([.]added_.*)?$/";
          // debugLine += " /^" + glyphName + "/";
          posLine += glyphName;
          debugLine += glyphName;
        }
        // if (glyphPos.set.size() > 1) {
        posLine += "]";
        debugLine += "]";
        //}
        posLine += "'";
        debugLine += "'";
        if (!glyphPos.lookupName.isEmpty()) {
          posLine += "lookup " + glyphPos.lookupName;
        }
      }
      posLine += ";\n";
      debugLine += ";\n";
      // std::cout << debugLine.toStdString();
      mainLookup += posLine;
    }
    mainLookup += QString("  } %1;\n").arg(lookup.name);
    mainLookupStrings.append(mainLookup);
  }

  featureOut << "lookup adjustoverlap {" << '\n';
  // featureOut << "  feature mkmk;\n";
  featureOut << subLookups;
  for (auto& mainLookupString : mainLookupStrings) {
    featureOut << mainLookupString;
  }

  featureOut << "} adjustoverlap;" << '\n';

  file.close();
}
