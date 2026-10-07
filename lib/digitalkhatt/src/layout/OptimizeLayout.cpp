#include "digitalkhatt/layout/OptimizeLayout.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/MarkClassifier.h"
#include "digitalkhatt/layout/PlacementAudit.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/SweepBroadphase.h"
#include "digitalkhatt/layout/XPBDConstraint.h"
#include "digitalkhatt/layout/constraints/BaseVicinityConstraint.h"
#include "digitalkhatt/layout/constraints/BowlClusterConstraint.h"
#include "digitalkhatt/layout/constraints/HardStayAboveConstraint.h"
#include "digitalkhatt/layout/constraints/HardStayBelowConstraint.h"
#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"
#include "digitalkhatt/layout/constraints/MatchMarkPositionWithOffsetConstraint.h"
#include "digitalkhatt/layout/constraints/SqueezeCenterConstraint.h"
#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"
#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"
#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"
#include "digitalkhatt/layout/constraints/YlaneConstraint.h"

namespace digitalkhatt::layout {

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

namespace {

// Explicit, hand-authored rules for which top marks are allowed to stack
// directly on one another — analogous to an OpenType contextual rule,
// rather than a generic "chain whatever shares a base" heuristic (which
// stacked marks that were never meant to relate to each other).
//
// Each row names both sides of the relationship directly: `aboveNames` is
// the mark being positioned (further from the base), `belowNames` is the
// mark-family it attaches beneath. Hash sets rather than vectors, since
// every row gets membership-tested against every mark on a base.
//
// offsetX is a plain per-row number for now (same status as minGap) -- once
// a real formula for optical alignment (e.g. why fatha sits slightly left
// of shadda's center) is worked out, this is the field that becomes a
// named, pluggable function instead of a constant.
struct StackSlot {
  std::unordered_set<std::string> aboveNames;
  std::unordered_set<std::string> belowNames;
  double offsetX = 0.0;  // target centerX(below) - centerX(above)
  double minGap = 40.0;
  double gapCompliance = 0.1;
  double xAlignCompliance = 0.2;
};
using StackTable = std::vector<StackSlot>;

// "open"/"not open" fathatan and dammatan are read here as the plain glyph
// (fathatan/dammatan) vs. the idgham-assimilated glyph
// (fathatanidgham/dammatanidgham) -- the only two variants this font has.
const StackTable& topStackTable() {
  static const StackTable table = {
      {{"shadda"}, {"onedotup", "twodotsup", "three_dots"}},
      {{"fatha", "fathatanidgham", "fathatan"}, {"shadda"}, 0, 0},
      {{"damma", "dammatanidgham", "dammatan"}, {"shadda"}, 30, 0},
      {{"fatha", "fathatanidgham", "fathatan", "damma", "dammatanidgham", "dammatan"}, {"hamzaabove", "hamzaabove.joined"}, 30, 0},
      {{"fatha", "fathatanidgham", "fathatan", "damma", "dammatanidgham", "dammatan"}, {"onedotup", "twodotsup", "three_dots"}, 30, 10},
      {{"maddahabove"}, {"smallalef.isol", "smallalef.replacement", "smallalef.joined"}, 30, 0}};
  return table;
}

GlyphInstance* findMember(const std::vector<GlyphInstance*>& group,
                          const std::unordered_set<std::string>& names) {
  for (GlyphInstance* g : group) {
    if (names.contains(g->glyphName)) return g;
  }
  return nullptr;
}

// Whether some other mark in the group sits (vertically) between `below`
// and `above` -- if so, they aren't actually adjacent in this base's stack,
// and a rule matching them directly must not fire (e.g. fatha must not
// stack directly on dot-above when shadda is sitting between them).
bool hasMarkBetween(const std::vector<GlyphInstance*>& group,
                    const GlyphInstance* below, const GlyphInstance* above) {
  const double belowY = boxCenterY(*below);
  const double aboveY = boxCenterY(*above);
  for (GlyphInstance* m : group) {
    if (m == below || m == above) continue;
    const double y = boxCenterY(*m);
    if (y > belowY && y < aboveY) return true;
  }
  return false;
}

void wireStackTable(const std::vector<GlyphInstance*>& group, const StackTable& table,
                    const OptParams& P,
                    std::vector<std::unique_ptr<XPBDConstraint>>& xpbdConstraints,
                    std::set<std::pair<int, int>>& excludedFromGenericGap) {
  for (const auto& rule : table) {
    GlyphInstance* above = findMember(group, rule.aboveNames);
    GlyphInstance* below = findMember(group, rule.belowNames);
    if (above && below && !hasMarkBetween(group, below, above)) {
      xpbdConstraints.push_back(std::make_unique<StackOrderConstraint>(
          *below, *above, /*isAbove=*/true,
          rule.minGap, P.compliance.stackOrderGap, P.compliance.stackOrderXAlign, rule.offsetX));
      // StackOrderConstraint already enforces the correct (often 0, for an
      // intentional tight stack) gap for this pair every iteration; keep the
      // blunt generic broadphase minGap from also fighting over it.
      excludedFromGenericGap.insert(
          {std::min(below->globalIndex, above->globalIndex),
           std::max(below->globalIndex, above->globalIndex)});
    }
  }
}

}  // namespace

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const ClassMap& classes,
                  const OptParams& P) {
  optimizePage(pageGlyphs, classes, P, nullptr);
}

void optimizePage(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                  const ClassMap& classes,
                  const OptParams& P,
                  std::vector<ConstraintViolation>* outViolations) {
  initGlyphMobilities(pageGlyphs);
  double dt = 1.0;
  SolverContext solverContext{pageGlyphs, classes};

  std::vector<std::reference_wrapper<GlyphInstance>> glyphs;
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      g.globalIndex = static_cast<int>(glyphs.size());
      buildWorldPolys(g);
      glyphs.push_back(g);
    }
  }

  std::unordered_set<const GlyphInstance*> validBases;
  for (const auto& ref : glyphs) {
    const auto& g = ref.get();
    if (!g.isMark && g.glyphName != "space" && !g.worldPolys.empty()) validBases.insert(&g);
  }
  const auto validOwner = [&](const GlyphInstance& g) {
    return validBases.contains(g.prevBase) && g.prevBase->lineIndex == g.lineIndex;
  };
  std::vector<std::unique_ptr<XPBDConstraint>> xpbdConstraints;

  std::vector<std::unique_ptr<XPBDConstraint>> hardConstraints;
  std::vector<WaqfPlacementConstraint*> waqfPlacements;
  std::vector<std::unique_ptr<WaqfEscapeConstraint>> waqfEscapes;
  if (P.toggles.waqfEscape && P.toggles.waqfPlacement && P.toggles.genericGapConstraint)
    solverContext.waqfEscapes.resize(glyphs.size(), nullptr);

  // Per-base groups of top marks, matched against the explicit stacking
  // chains below. (No below-mark stacking chains are defined yet -- kasra
  // et al. don't currently get a stacking pass.)
  std::unordered_map<GlyphInstance*, std::vector<GlyphInstance*>> aboveStackGroups;

  // Per-base groups of marks trapped inside a bowl-shaped base (Jeem/Hah/Khah).
  // Each group is centered as a block by one BowlClusterConstraint built after
  // the loop; the members are excluded from the generic gap there.
  std::unordered_map<GlyphInstance*, std::vector<GlyphInstance*>> bowlMarkGroups;

  // Pre-pass: which bases actually have a bowl situation? A base qualifies if
  // ANY of its non-above marks is geometrically enclosed, or if it's in the
  // named bowlbases allowlist. Once a base qualifies, EVERY non-above mark
  // attached to it joins the cluster below -- not just the subset that
  // individually passes the strict per-mark geometric test. Without this, a
  // sibling mark sitting right at the bowl's edge (common when marks are
  // packed tight) would fall through to the normal path, and its gap pair
  // against the properly-detected marks would never be excluded, silently
  // reproducing the original overlap bug for that one pair.
  std::unordered_set<GlyphInstance*> bowlBases;
  if (P.toggles.bowlCluster) {
    for (auto& gRef : glyphs) {
      GlyphInstance& g = gRef.get();
      if (!g.isMark || !validOwner(g)) continue;
      const bool isAbove = solverContext.topmarks.contains(g.glyphName) ||
                           solverContext.topdotmarks.contains(g.glyphName);
      if (isAbove) continue;
      if (/*markEnclosedByBase(g, *g.prevBase) ||*/ solverContext.isBowlBase(*g.prevBase)) {
        bowlBases.insert(g.prevBase);
      }
    }
  }

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& mark = glyphs[i];
    if (!mark.isMark || !validOwner(mark)) continue;
    // NOTE: the harakat lane (YlaneConstraint) is built per-base in a second
    // pass after this loop, once aboveStackGroups is fully populated.

    if (solverContext.isWaqf(mark)) {
      if (P.toggles.waqfPlacement) {
        auto placement = std::make_unique<WaqfPlacementConstraint>(
            mark, P.compliance.waqfMin, P.compliance.waqfTarget,
            P.compliance.waqfMax, P.compliance.waqfXAlign,
            /*minDistFromBaseline=*/700.0, /*minGapToBase=*/100.0,
            /*minGapToTopMarks=*/50.0, /*desiredExtraLift=*/0.0,
            /*upperCeilingY=*/1400.0, P.waqfHorizontalAlignmentBandPercent);
        waqfPlacements.push_back(placement.get());
        if (!solverContext.waqfEscapes.empty()) {
          auto escape = std::make_unique<WaqfEscapeConstraint>(*placement, P);
          solverContext.waqfEscapes[mark.globalIndex] = escape.get();
          waqfEscapes.push_back(std::move(escape));
        }
        xpbdConstraints.push_back(std::move(placement));
      }
      // Waqf marks already get a full placement solve above; skip the
      // generic below rail/vicinity/squeeze handling for them.
      continue;
    }

    if (!mark.isMark || mark.prevBase == nullptr) continue;

    const bool isAboveMark = solverContext.topmarks.contains(mark.glyphName) ||
                             solverContext.topdotmarks.contains(mark.glyphName);
    const bool isBelowMark = solverContext.lowmarks.contains(mark.glyphName) ||
                             solverContext.downdotmarks.contains(mark.glyphName);

    // Marks the caller positions by hand before invoking the solver, so no
    // automatic constraint of any kind (rail, squeeze, vicinity, bowl-cluster,
    // horizontal order, generic gap) may touch them:
    //   - a below-mark on a bowl-shaped base (Jeem/Hah/Khah) -- the caller
    //     places it inside the bowl itself, not the generic below rail.
    //   - a below-dot (onedotdown/twodotsdown) on a behshape.init base that is
    //     directly preceded by a reh/waw base, AND already sits above the
    //     bottom of that reh/waw's bowl -- it must stay above the reh/waw's
    //     bowl and below behshape.init, a position no generic rule here
    //     produces. Dots that are already below the reh/waw's bbox bottom
    //     aren't in the collision zone this exception is for, so they keep
    //     the normal generic handling.
    const bool manuallyPositioned = isManuallyPositionedMark(mark, solverContext);
    if (manuallyPositioned) {
      solverContext.gapExcludedGlyphs.insert(mark.globalIndex);
      continue;
    }

    // Trapped inside a bowl-shaped base (Jeem/Hah/Khah)? Membership is decided
    // per-BASE (see the bowlBases pre-pass above), not per-mark: once a base
    // qualifies, every one of its non-above marks joins the cluster, even a
    // sibling that wouldn't individually pass the geometric enclosure test.
    // Such marks get NO separating force (stay rail, squeeze, vicinity all
    // fight the anchor); instead one BowlClusterConstraint per base centers
    // the whole cluster as a block, built after the loop.
    const bool inBowl = P.toggles.bowlCluster && !isAboveMark &&
                        bowlBases.contains(mark.prevBase);

    if (inBowl) {
      bowlMarkGroups[mark.prevBase].push_back(&mark);
    } else {
      // ---- 1) Generic hard stay-above/stay-below rail ----
      if (isAboveMark) {
        if (P.toggles.hardStayAboveBelow) {
          auto minDistFromBaseline = 100.0;
          if (mark.prevBase != nullptr && mark.prevBase->glyphName == "yehshape.fina.ii" && mark.glyphName == "smallalef.replacement") {
            minDistFromBaseline = 0;
          }
          hardConstraints.push_back(std::make_unique<HardStayAboveConstraint>(
              mark, P.compliance.hardStayAbove, /*minGapToBase=*/0.0, minDistFromBaseline, /*useBaseTop=*/false, true));
        }
        aboveStackGroups[mark.prevBase].push_back(&mark);
      } else if (isBelowMark) {
        // A bowl-base below-mark would have hit the manuallyPositioned
        // continue above, so isBowlBase is never true here.
        if (P.toggles.hardStayAboveBelow) {
          hardConstraints.push_back(std::make_unique<HardStayBelowConstraint>(
              mark, P.compliance.hardStayBelow, /*maxGapToBase=*/0.0, /*maxDistToBaseline=*/0.0,
              /*useBaseBottom=*/false, /*testBothReferences=*/true,
              /*belowBaselineFraction=*/0.66));
        }
      }

      // ---- 3) Structural squeeze centering (no glyph-name matching) ----
      if (P.toggles.squeezeCenter) {
        std::vector<GlyphInstance*> obstacles;
        obstacles.push_back(mark.prevBase);
        if (mark.prevBase->prevBase != nullptr) obstacles.push_back(mark.prevBase->prevBase);
        if (mark.prevBase->nextBase != nullptr) obstacles.push_back(mark.prevBase->nextBase);

        if (obstacles.size() >= 2) {
          // SweepBroadphase always reports pairs as (min index, max index); the
          // exclusion set must be keyed the same way or it silently fails to
          // exclude anything.
          const int markIndex = static_cast<int>(i);
          for (GlyphInstance* obstacle : obstacles) {
            const int obstacleIndex = obstacle->globalIndex;
            solverContext.excludedFromGenericGap.insert(
                {std::min(markIndex, obstacleIndex), std::max(markIndex, obstacleIndex)});
          }
          xpbdConstraints.push_back(std::make_unique<SqueezeCenterConstraint>(mark, P, obstacles, P.compliance.squeezeCenter));
        }
      }

      // ---- 5) Base-vicinity leash (ownership ambiguity) ----
      if (P.toggles.baseVicinity) {
        const MarkRole role = classifyMark(mark);
        const auto bounds = mark.worldPolys.boundingAABB();
        const double width = bounds.maxx - bounds.minx;
        const double margin = (role == MarkRole::Dots) ? width * 0.5 : width;
        xpbdConstraints.push_back(std::make_unique<BaseVicinityConstraint>(
            mark, margin, P.compliance.baseVicinityLeft, P.compliance.baseVicinityRight));
      }
    }

    // Compound-mark cohesion (kasra + smalllowmeem forming one visual
    // unit): not a collision/placement rule, keep as an explicit pairing.
    if (P.toggles.matchMarkPosition && mark.glyphName == "smalllowmeem" && i > 0) {
      GlyphInstance& prevMark = glyphs[i - 1];
      if (prevMark.glyphName == "kasra" && prevMark.prevBase == mark.prevBase &&
          prevMark.lineIndex == mark.lineIndex) {
        xpbdConstraints.push_back(std::make_unique<MatchMarkPositionWithOffsetConstraint>(
            prevMark, mark, 130, 0, P.compliance.matchMarkPosition));
      }
    }
  }

  // ---- Bowl-cluster centering: marks trapped inside a bowl base ----
  // Runs AFTER the main loop so each base's full mark cluster is known. Each
  // cluster is centered as a block. The members opt out of the generic gap
  // entirely (gapExcludedGlyphs), not just against each other and their base:
  // a member that could still be pushed by the gap against an ADJACENT base
  // would drag the group's midX off-center, and BowlClusterConstraint's block
  // re-centering would then slide every member and create a fresh overlap.
  if (P.toggles.bowlCluster) {
    for (auto& [base, members] : bowlMarkGroups) {
      for (GlyphInstance* m : members) {
        solverContext.gapExcludedGlyphs.insert(m->globalIndex);
      }
      xpbdConstraints.push_back(std::make_unique<BowlClusterConstraint>(
          *base, members, /*compliance=*/P.compliance.bowlCluster, /*interiorInset=*/0.0));
    }
  }

  // ---- Two-sided harakat lane: one constraint per base, on its anchor ----
  // Runs AFTER the main loop so aboveStackGroups is fully populated (the loop
  // fills aboveStackGroups[mark.prevBase] as it visits each above-mark).
  if (P.toggles.ylane) {
    auto isBandParticipant = [](const std::string& n) {
      return n.starts_with("fatha") || n.starts_with("damma") ||
             n.starts_with("sukun") /*|| n.starts_with("shadda")*/;
    };
    auto isLaneHaraka = [](const std::string& n) {
      return n.starts_with("fatha") || n.starts_with("damma") ||
             n.starts_with("sukun");
    };

    for (auto& [base, group] : aboveStackGroups) {
      // Only bases that actually carry a fatha/damma/sukun get a lane. (Anchor
      // coverage depends on these marks being in `topmarks` so they enter
      // aboveStackGroups; if the font class drops one, that base gets no lane.)
      bool hasHaraka = false;
      for (GlyphInstance* m : group) {
        if (isLaneHaraka(m->glyphName)) {
          hasHaraka = true;
          break;
        }
      }
      if (!hasHaraka) continue;

      // Anchor = bottom-most band participant (shadda when present, else the
      // haraka itself). Dots are in `group` via topdotmarks but are excluded
      // here so their i'jam placement stays free.
      GlyphInstance* anchor = nullptr;
      double bestY = 0.0;
      for (GlyphInstance* m : group) {
        if (!isBandParticipant(m->glyphName)) continue;
        const double y = boxCenterY(*m);
        if (anchor == nullptr || y < bestY) {
          bestY = y;
          anchor = m;
        }
      }

      if (anchor && anchor->mobility > 0.0) {
        xpbdConstraints.push_back(std::make_unique<YlaneConstraint>(
            *anchor, /*compliance=*/P.compliance.ylane, /*laneHeight=*/600.0));
      }
    }
  }

  // ---- 4) Explicit, case-by-case top-mark stacking ----
  if (P.toggles.stackOrder) {
    for (auto& [base, group] : aboveStackGroups) {
      wireStackTable(group, topStackTable(), P, xpbdConstraints, solverContext.excludedFromGenericGap);
      // Any combination of top marks not covered by a table row is left
      // unstacked until a rule for it is defined.
    }
  }

  if (P.toggles.horizontalOrder) {
    for (size_t i = 0; i < glyphs.size(); ++i) {
      GlyphInstance& markB = glyphs[i];
      if (markB.isMark && validOwner(markB) &&
          validBases.contains(markB.prevBase->prevBase) &&
          markB.prevBase->prevBase->lineIndex == markB.lineIndex) {
        if (solverContext.isWaqf(markB) && (markB.prevBase->glyphName == "smallwaw" || markB.prevBase->glyphName == "smallyeh"))
          continue;
        // Manually-positioned marks (see the manuallyPositioned continue
        // above, which also populates gapExcludedGlyphs for them) opt out of
        // this too -- the caller's hand-placed position is the whole point.
        if (solverContext.gapExcludedGlyphs.contains(markB.globalIndex)) continue;
        auto prevBase = markB.prevBase->prevBase;
        for (int glyphIndex = prevBase->glyphIndex + 1; glyphIndex < markB.prevBase->glyphIndex; glyphIndex++) {
          auto& markA = pageGlyphs[markB.lineIndex][glyphIndex];
          if (solverContext.gapExcludedGlyphs.contains(markA.globalIndex)) continue;
          if (markA.isMark && markA.isTopMark == markB.isTopMark &&
              !solverContext.isWaqf(markA) && !solverContext.isWaqf(markB)) {
            xpbdConstraints.push_back(std::make_unique<HorizontalOrderConstraint>(
                markA, markB, P.horizontalOrderSelfKeep, P.horizontalOrderCrossKeep, P.compliance.horizontalOrder));
          }
        }
      }
    }
  }

  // Apply waqf top ordering after all ordinary mark rails, so their upward
  // corrections cannot leave the waqf below its own stack's highest top.
  for (auto* placement : waqfPlacements)
    hardConstraints.push_back(std::make_unique<WaqfTopOrderConstraint>(*placement));

  std::vector<std::pair<int, int>> pairs;
  const double toleranceSquared = P.tolCollision * P.tolCollision;
  for (int iter = 0; iter < P.maxIters; ++iter) {
    for (auto& ref : glyphs) ref.get().iterationMaxMovementSquared = 0.0;
    // Consume the previous ordinary gap pass, then clear its samples. The
    // existing broadphase/contact pass below handles any induced movement.
    for (auto& escape : waqfEscapes) {
      escape->project(solverContext, dt);
      escape->contacts.clear();
    }

    for (auto& c : xpbdConstraints) {
      c->project(solverContext, dt);
    }
    // Lane/stack/placement projections can move marks beyond the old candidate
    // horizon. Build candidate pairs from the updated world geometry.
    SweepBroadphase(glyphs, P).findPairs(pairs);
    double maxPenetration = 0.0;

    // 3. Gap / collision constraints
    // Iterate candidate pairs
    if (P.toggles.genericGapConstraint) {
      for (auto& pair : pairs) {
        GlyphInstance& glyphA = glyphs[pair.first];
        GlyphInstance& glyphB = glyphs[pair.second];
        if (!solverContext.isGapExcluded(pair)) {
          solveGapConstraint(solverContext, glyphA, glyphB, P, dt, maxPenetration);
        }
      }
    }

    for (auto& c : hardConstraints) {
      c->project(solverContext, dt);
    }

    double maxMovementSquared = 0.0;
    for (const auto& ref : glyphs)
      maxMovementSquared = std::max(maxMovementSquared, ref.get().iterationMaxMovementSquared);
    if (P.tolCollision >= 0.0 && maxMovementSquared <= toleranceSquared) {
      // Re-evaluate ALL active hard bounds, including types omitted from the
      // user-facing report. Soft targets may retain their designed spring slack;
      // their applied corrections above still participate in convergence.
      std::vector<ConstraintViolation> remaining;
      for (const auto& c : xpbdConstraints) c->reportViolations(solverContext, remaining);
      for (const auto& c : hardConstraints) c->reportViolations(solverContext, remaining);
      if (P.toggles.genericGapConstraint) collectGapViolations(solverContext, glyphs, P, remaining);
      const bool hardBoundUnresolved = std::any_of(remaining.begin(), remaining.end(), [&](const auto& v) {
        return v.kind == ViolationKind::Hard && v.severity > P.tolCollision;
      });
      if (!hardBoundUnresolved) break;
    }
  }

  // Post-solve diagnostic: collect remaining hard violations. Done here, at the
  // only point where the constraint objects, solverContext, glyphs, and the
  // final worldPolys all still coexist. Read-only; no effect when null.
  if (outViolations) {
    for (const auto& c : xpbdConstraints) {
      if (c->reportEnabled) c->reportViolations(solverContext, *outViolations);
    }
    for (const auto& c : hardConstraints) {
      if (c->reportEnabled) c->reportViolations(solverContext, *outViolations);
    }
    for (const auto& escape : waqfEscapes)
      if (escape->reportEnabled) escape->reportViolations(solverContext, *outViolations);
    if (P.toggles.reportGenericGap && !P.reportGenericGapCollisionsOnly) {
      collectGapViolations(solverContext, glyphs, P, *outViolations);
    }
    if (!P.toggles.reportSoftResiduals) {
      std::erase_if(*outViolations, [](const auto& v) { return v.kind == ViolationKind::Soft; });
    }
    if (!P.toggles.reportWaqfBounds) {
      std::erase_if(*outViolations, [](const auto& v) {
        return v.type == ViolationType::WaqfBoundsResidual;
      });
    }
    // Association diagnostics are collected only for reports. They neither
    // participate in convergence nor depend on the soft-target/force switches.
    collectWaqfPlacementViolations(solverContext, P, *outViolations);
    // The optional audit is independent of the corresponding force switches.
    // Geometric ownership warnings are retained regardless of the soft-target
    // preference switch: they indicate placement ambiguity, not a tuning goal.
    if (P.toggles.reportPlacementAudit)
      collectPlacementViolations(solverContext, *outViolations);

    // Annotate only the final report. The convergence check above continues
    // to evaluate raw hard residuals, independently of reporting allowances.
    for (auto& v : *outViolations) {
      if (v.type != ViolationType::BaseVicinity || v.glyphA < 0 ||
          v.glyphA >= static_cast<int>(glyphs.size())) continue;
      const auto& mark = glyphs[v.glyphA].get();
      const auto bounds = mark.worldPolys.boundingAABB();
      const bool dots = classifyMark(mark) == MarkRole::Dots;
      const double percent = dots ? P.baseVicinityDotTolerancePercent : P.baseVicinityMarkTolerancePercent;
      const double width = std::max(0.0, bounds.maxx - bounds.minx);
      v.allowedResidual = width * (std::isfinite(percent) ? std::clamp(percent, 0.0, 100.0) : 0.0) / 100.0;
      const auto baseBounds = mark.prevBase->worldPolys.boundingAABB();
      const bool left = v.marker[0].x < 0.5 * (baseBounds.minx + baseBounds.maxx);
      if (v.detail.empty())
        v.detail = std::string(left ? "Left" : "Right") +
            " horizontal overflow beyond owning base footprint + margin";
      v.detail += std::string("; ") + (dots ? "dot" : "mark") + " reporting tolerance applied";
    }

    // Drop residual-scale entries (see OptParams::minViolationSeverity):
    // constraints are solved iteratively and rarely land on an exact C=0, so
    // without this cutoff every report would carry a tail of hundredths-of-a-
    // unit "violations" that are really just XPBD's convergence noise floor,
    // not something a font designer should act on.
    std::erase_if(*outViolations, [&](const ConstraintViolation& v) {
      return !v.structural &&
          ((v.type == ViolationType::BaseVicinity && v.severity <= v.allowedResidual) ||
           violationReportSeverity(v) < P.minViolationSeverity);
    });
  }
}

}  // namespace digitalkhatt::layout
