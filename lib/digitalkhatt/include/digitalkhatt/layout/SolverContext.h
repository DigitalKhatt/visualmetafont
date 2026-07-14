#pragma once

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "digitalkhatt/layout/ClassMap.h"
#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/GlyphInstance.h"

namespace digitalkhatt::layout {

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
           gapExcludedGlyphs.contains(pr.first) ||
           gapExcludedGlyphs.contains(pr.second);
  }

  // True if `base` is a bowl-shaped base glyph (e.g. final/isolated
  // Jeem/Hah/Khah) whose below-mark anchor already places the mark inside
  // the bowl, so the generic stay-below rail must not apply to it.
  bool isBowlBase(const GlyphInstance& base) const {
    return bowlbases.contains(base.glyphName);
  }

  GapsInfo gapInfos;
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

  ClassSet marks;
  ClassSet topmarks;
  ClassSet lowmarks;
  ClassSet waqfmarks;
  ClassSet topdotmarks;
  ClassSet downdotmarks;
  ClassSet bowlbases;
};

}  // namespace digitalkhatt::layout
