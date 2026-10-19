#pragma once

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "digitalkhatt/layout/ClassMap.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

struct WaqfEscapeConstraint;

struct SolverContext {
  SolverContext(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                const ClassMap& classes);

  bool isWaqf(const GlyphInstance& glyphInstance) const {
    return waqfmarks.contains(glyphInstance.glyphName);
  }

  // Whether a broadphase pair should be skipped by the generic gap pass --
  // either the specific pair was handed to a dedicated pairwise constraint, or
  // one of the two glyphs opts out of gap collision entirely (gapExcludedGlyphs).
  // `pr` is keyed {min,max} globalIndex, matching SweepBroadphase.
  bool isGapExcluded(const std::pair<int, int>& pr) const {
    return excludedFromGenericGap.contains(pr) ||
           (!gapIncludedPairs.contains(pr) &&
            (gapExcludedGlyphs.contains(pr.first) || gapExcludedGlyphs.contains(pr.second)));
  }

  // Bases whose authored lower-mark placements are excluded from generic gaps.
  // This allowlist does not identify the geometry used by the below-mark rail.
  bool excludesLowerMarksFromGap(const GlyphInstance& base) const {
    return lowerMarkGapExcludedBases.contains(base.glyphName);
  }

  // Interior geometry is separate from the manual-placement allowlist:
  // Ain may participate in generic gaps while retaining marks inside its bowl.
  bool hasBowlInterior(const GlyphInstance& base) const {
    return base.glyphName == "hah.isol" ||
           base.glyphName == "hah.fina" || base.glyphName == "ain.isol" ||
           base.glyphName == "ain.fina";
  }

  GapsInfo gapInfos;
  std::unordered_map<const GlyphInstance*, GapSqueezeState> gapSqueezes;
  bool rehWawBehPlacement = false;
  // Registered by the opt-in setup. Across lines these lower marks yield to
  // the next line's ink, instead of displacing its top-mark/Waqf stacks.
  std::unordered_set<const GlyphInstance*> contextualBelowMarks;
  // Applied below-tip independent marks and dot-linked kasra; centered/left slots
  // retain their existing contact policy.
  std::unordered_set<const GlyphInstance*> belowFallbackMarks;
  // Registered once by global glyph index; null entries cost no class lookup.
  std::vector<WaqfEscapeConstraint*> waqfEscapes;
  const ClassMap& classes;
  std::vector<std::vector<GlyphInstance>>& pageGlyphs;

  // Glyph-index pairs (keyed as {min(globalIndex), max(globalIndex)}, matching
  // SweepBroadphase's pair ordering) whose gap is already correctly and
  // unconditionally enforced by a dedicated pairwise constraint -- currently
  // SqueezeCenterConstraint's per-obstacle bound and StackOrderConstraint's
  // gap arm. Both the live generic-gap pass and the post-solve violation
  // re-evaluation skip these pairs, so the blunt broadphase minGap (e.g. an
  // unrelated 80-unit default) doesn't fight a pair whose correct gap (often
  // 0, for an intentional tight stack) is already handled elsewhere.
  std::set<std::pair<int, int>> excludedFromGenericGap;

  // Glyphs excluded from the generic gap pass against ANY other glyph (not just
  // specific pairs), keyed by globalIndex. Used for marks whose placement is
  // owned entirely by a dedicated constraint -- currently bowl-cluster marks:
  // letting the broadphase gap push such a mark against an adjacent base would
  // shift its BowlClusterConstraint group's center and create new overlaps, so
  // the mark opts out of collision entirely and relies on its owning constraint
  // (and the bowl containment) instead.
  std::unordered_set<int> gapExcludedGlyphs;

  // Contextual lower marks still collide with fixed authored bowl marks.
  // Only these pairs override a glyph-wide exclusion; excluded marks stay fixed.
  std::set<std::pair<int, int>> gapIncludedPairs;

  ClassSet marks;
  ClassSet topmarks;
  ClassSet lowmarks;
  ClassSet waqfmarks;
  ClassSet topdotmarks;
  ClassSet downdotmarks;
  ClassSet lowerMarkGapExcludedBases;
};

}  // namespace digitalkhatt::layout
