#include "digitalkhatt/layout/OptimizeLayout.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "digitalkhatt/layout/GapConstraint.h"
#include "digitalkhatt/layout/BelowMarkPlacement.h"
#include "digitalkhatt/layout/CollisionReport.h"
#include "digitalkhatt/layout/CollisionMarkPolicy.h"
#include "digitalkhatt/layout/GlyphInstanceUtils.h"
#include "digitalkhatt/layout/MarkClassifier.h"
#include "digitalkhatt/layout/PlacementAudit.h"
#include "digitalkhatt/layout/SolverContext.h"
#include "digitalkhatt/layout/SweepBroadphase.h"
#include "digitalkhatt/layout/TopMarkStack.h"
#include "digitalkhatt/layout/XPBDConstraint.h"
#include "digitalkhatt/layout/constraints/BaseVicinityConstraint.h"
#include "digitalkhatt/layout/constraints/BowlClusterConstraint.h"
#include "digitalkhatt/layout/constraints/HardStayAboveConstraint.h"
#include "digitalkhatt/layout/constraints/HardStayBelowConstraint.h"
#include "digitalkhatt/layout/constraints/MarkFrameConstraint.h"
#include "digitalkhatt/layout/constraints/HorizontalOrderConstraint.h"
#include "digitalkhatt/layout/constraints/MatchMarkPositionWithOffsetConstraint.h"
#include "digitalkhatt/layout/constraints/SqueezeCenterConstraint.h"
#include "digitalkhatt/layout/constraints/StackOrderConstraint.h"
#include "digitalkhatt/layout/constraints/CompactMarkConstraint.h"
#include "digitalkhatt/layout/RehWawMarkPlacement.h"
#include "digitalkhatt/layout/WaqfMarkPlacement.h"
#include "digitalkhatt/layout/constraints/HorizontalGapConstraint.h"
#include "digitalkhatt/layout/constraints/WaqfPlacementConstraint.h"
#include "digitalkhatt/layout/constraints/WaqfEscapeConstraint.h"
#include "digitalkhatt/layout/constraints/YlaneConstraint.h"

namespace digitalkhatt::layout {

void initGlyphMobilities(std::vector<std::vector<GlyphInstance>>& pageGlyphs) {
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      g.initialPlacement = GlyphInstance::InitialPlacement::None;
      g.initialPositioningDx = g.initialPositioningDy = 0.0;
      if (!g.isCollisionMark()) {
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

using GlyphSequence = std::vector<std::reference_wrapper<GlyphInstance>>;
using ConstraintList = std::vector<std::unique_ptr<XPBDConstraint>>;
using MarkGroups = std::unordered_map<GlyphInstance*, std::vector<GlyphInstance*>>;
using ValidBases = std::unordered_set<const GlyphInstance*>;

struct PageConstraints {
  ConstraintList placement;
  ConstraintList hard;
  std::vector<std::unique_ptr<WaqfEscapeConstraint>> waqfEscapes;
};

// Initial plans live only while assembling the page. XPBD receives ordinary
// constraints plus a mobility-zero flag for accepted current-gap dots.
struct RehWawBehSetup {
  MarkGroups stackGroups;
  std::unordered_map<GlyphInstance*, std::unique_ptr<RehWawMarkPlacement>> placements;
  ConstraintList stacks;
};

void addTopMarkStackConstraints(const std::vector<GlyphInstance*>& group,
                                const OptParams& P,
                                ConstraintList& xpbdConstraints,
                                std::set<std::pair<int, int>>& excludedFromGenericGap) {
  for (const auto& rule : buildTopMarkStackLinks(group)) {
    GlyphInstance* above = rule.outer;
    GlyphInstance* below = rule.inner;
    if (rule.compactAttachment)
      xpbdConstraints.push_back(std::make_unique<CompactMarkConstraint>(*below, *above, rule.minGap, P.compliance.matchMarkPosition));
    else {
      auto stack = std::make_unique<StackOrderConstraint>(
          *below, *above, /*isAbove=*/true,
          rule.minGap, P.compliance.stackOrderGap, P.compliance.stackOrderXAlign, rule.offsetX, rule.outlineClearance, rule.preferredGap);
      // A movable font base already transports this child. Refine the child
      // relative to that common frame, rather than moving both in opposition.
      stack->keepInnerForGap = !below->isMark && above->prevBase == below;
      if (rule.preserveAuthoredGap) stack->preserveAuthoredGap();
      xpbdConstraints.push_back(std::move(stack));
    }
    // The authored stack gap replaces generic clearance only for this direct
    // pair. Contacts with every other mark remain active.
    excludedFromGenericGap.insert(
        {std::min(below->globalIndex, above->globalIndex),
         std::max(below->globalIndex, above->globalIndex)});
  }
}

GlyphSequence prepareGlyphs(std::vector<std::vector<GlyphInstance>>& pageGlyphs) {
  GlyphSequence glyphs;
  int word = 0;
  for (auto& lineGlyphs : pageGlyphs) {
    for (auto& g : lineGlyphs) {
      const bool space = isCollisionSpace(g.glyphName);
      g.wordIndex = space ? -1 : word;
      if (space) ++word;
      g.globalIndex = static_cast<int>(glyphs.size());
      buildWorldPolys(g);
      glyphs.push_back(g);
    }
    ++word;  // a new line cannot continue the preceding line's word identity
  }
  return glyphs;
}

bool hasValidOwner(const GlyphInstance& mark, const ValidBases& validBases) {
  return validBases.contains(mark.prevBase) && mark.prevBase->lineIndex == mark.lineIndex;
}

std::vector<GlyphInstance*> collectLowerObstacles(const GlyphSequence& glyphs,
                                                  const ValidBases& validBases,
                                                  const GlyphInstance& mark) {
  auto span = mark.worldPolys.boundingAABB();
  const double width = span.maxx - span.minx;
  for (auto* base : {mark.prevBase, mark.prevBase->prevBase}) {
    if (!validBases.contains(base)) continue;
    const auto box = base->worldPolys.boundingAABB();
    span.minx = std::min(span.minx, box.minx);
    span.maxx = std::max(span.maxx, box.maxx);
  }
  std::vector<GlyphInstance*> obstacles;
  for (auto& ref : glyphs) {
    auto& base = ref.get();
    if (!validBases.contains(&base) || base.lineIndex != mark.lineIndex) continue;
    const auto box = base.worldPolys.boundingAABB();
    if (box.minx <= span.maxx + width && box.maxx >= span.minx - width)
      obstacles.push_back(&base);
  }
  return obstacles;
}

void prepareRehWawBehPositions(const GlyphSequence& glyphs,
                               const ValidBases& validBases,
                               SolverContext& solverContext,
                               const OptParams& P,
                               RehWawBehSetup& setup,
                               InitialPositioning initialPositioning) {
  // Gather and link every pair before searching. All searches see the same
  // unchanged HarfBuzz input, regardless of the order in which plans are made.
  for (auto& ref : glyphs) {
    auto& mark = ref.get();
    if (!mark.isMark || !hasValidOwner(mark, validBases) ||
        isManuallyPositionedMark(mark, solverContext)) continue;
    if (!solverContext.lowmarks.contains(mark.glyphName) &&
        !solverContext.downdotmarks.contains(mark.glyphName)) continue;
    if (!validBases.contains(mark.prevBase->prevBase)) continue;
    auto* tipReference = rehWawReference(mark);
    if (!tipReference) continue;
    // The switch scopes both positioning AND dot-kasra constraints. An
    // initial Beh after Alef (or across a word/line boundary) keeps the same
    // ordinary constraints it has with this option disabled.
    setup.stackGroups[mark.prevBase].push_back(&mark);
    solverContext.contextualBelowMarks.insert(&mark);
    auto placement = std::make_unique<RehWawMarkPlacement>(mark, *tipReference, P);
    placement->obstacles = collectLowerObstacles(glyphs, validBases, mark);
    {
      // Authored marks inside a following bowl are excluded from the generic
      // gap pass. They still occupy ink: include them in the dot's slot
      // search, without moving them or changing their bowl policy.
      const auto owner = mark.prevBase->worldPolys.boundingAABB();
      const auto box = mark.worldPolys.boundingAABB();
      double halfFootprint = (box.maxx - box.minx) * 0.5;
      for (const auto& ref : glyphs) {
        const auto& other = ref.get();
        if (other.prevBase != mark.prevBase || !isKasraMark(other) || other.worldPolys.empty()) continue;
        const auto bounds = other.worldPolys.boundingAABB();
        const double offset = std::abs((boxCenterX(mark) - mark.dx) - (boxCenterX(other) - other.dx));
        halfFootprint = std::max(halfFootprint, offset + (bounds.maxx - bounds.minx) * 0.5);
      }
      for (auto& ref : glyphs) {
        auto& other = ref.get();
        const bool neighboringMark = other.isMark && hasValidOwner(other, validBases) &&
                                     other.prevBase != mark.prevBase;
        const bool neighboringLineBase = !other.isMark && other.lineIndex != mark.lineIndex &&
                                         validBases.contains(&other);
        if ((!neighboringMark && !neighboringLineBase) ||
            std::abs(other.lineIndex - mark.lineIndex) > 1 || other.worldPolys.empty()) continue;
        const double margin = halfFootprint +
            (other.lineIndex == mark.lineIndex ? P.differentWordGap :
                std::max(P.differentWordGap, P.differentLineGap));
        const auto bounds = other.worldPolys.boundingAABB();
        if (bounds.minx <= owner.maxx + margin && bounds.maxx >= owner.minx - margin)
          placement->obstacles.push_back(&other);
      }
    }
    for (const auto* obstacle : placement->obstacles)
      if (obstacle->isMark && obstacle->prevBase != mark.prevBase)
        solverContext.gapIncludedPairs.insert(
            {std::min(mark.globalIndex, obstacle->globalIndex), std::max(mark.globalIndex, obstacle->globalIndex)});
    setup.placements[&mark] = std::move(placement);
  }
  for (auto& ref : glyphs) {
    auto& dot = ref.get();
    if (!isRehWawDot(dot)) continue;
    auto it = setup.placements.find(&dot);
    if (it == setup.placements.end()) continue;
    for (auto* mark : setup.stackGroups[dot.prevBase]) {
      if (!isKasraMark(*mark)) continue;
      auto kasra = setup.placements.find(mark);
      if (kasra == setup.placements.end()) continue;
      kasra->second->dots.push_back(&dot);
      kasra->second->followedDotPlacement = it->second.get();
      it->second->kasras.push_back(kasra->second.get());
    }
  }
  if (initialPositioning == InitialPositioning::Skip) return;
  // Complete the ownership links first: standalone kasra searches its own
  // slot, while a linked kasra only follows the owning dot's selected slot.
  for (auto& ref : glyphs)
    if (auto it = setup.placements.find(&ref.get()); it != setup.placements.end())
      it->second->prepareTarget();
  struct Plan {
    GlyphInstance* mark;
    RehWawMarkPlacement* placement;
    geometry::GeometrySet original;
    geometry::GeometrySet shape;
    double dx, dy;
    GlyphInstance::InitialPlacement kind;
  };
  std::vector<Plan> plans;
  for (auto& ref : glyphs) {
    auto& mark = ref.get();
    auto it = setup.placements.find(&mark);
    if (it == setup.placements.end()) continue;
    auto& placement = *it->second;
    auto shape = placement.preferredShape();
    if (shape.empty()) continue;
    const auto box = shape.boundingAABB();
    auto kind = GlyphInstance::InitialPlacement::None;
    if (placement.dots.empty())
      kind = placement.dotUsesCurrentGap ? GlyphInstance::InitialPlacement::CenteredGap : placement.dotUsesLeftSlot ? GlyphInstance::InitialPlacement::LeftOfRehWaw
                                                                                                                    : GlyphInstance::InitialPlacement::BelowRehWaw;
    plans.push_back({&mark, &placement, mark.worldPolys, std::move(shape),
                     0.5 * (box.minx + box.maxx) - boxCenterX(mark), box.maxy - boxTopY(mark), kind});
  }
  // Validate planned pairs together before pinning any dot. If simultaneous
  // moves conflict, restore that owner's input and let ordinary XPBD handle it.
  for (auto& plan : plans) plan.mark->worldPolys = plan.shape;
  std::unordered_set<GlyphInstance*> rejected;
  for (;;) {
    bool changed = false;
    for (const auto& plan : plans) {
      auto* owner = plan.mark->prevBase;
      if (rejected.contains(owner) || plan.placement->hasInkClearance(plan.shape)) continue;
      rejected.insert(owner);
      changed = true;
      for (const auto& sibling : plans)
        if (sibling.mark->prevBase == owner) sibling.mark->worldPolys = sibling.original;
    }
    if (!changed) break;
  }
  for (const auto& plan : plans) {
    auto& mark = *plan.mark;
    if (rejected.contains(mark.prevBase)) continue;
    mark.dx += plan.dx;
    mark.dy += plan.dy;
    mark.initialPositioningDx = plan.dx;
    mark.initialPositioningDy = plan.dy;
    mark.initialPlacement = plan.kind;
    if (plan.kind == GlyphInstance::InitialPlacement::CenteredGap) mark.mobility = 0.0;
    updateWorldPolys(mark);
  }
  for (auto& ref : glyphs) {
    auto& dot = ref.get();
    if (dot.initialPlacement != GlyphInstance::InitialPlacement::BelowRehWaw) continue;
    solverContext.belowFallbackMarks.insert(&dot);
    for (auto* sibling : setup.stackGroups[dot.prevBase])
      if (isKasraMark(*sibling)) solverContext.belowFallbackMarks.insert(sibling);
  }
}

void addRehWawBehStackConstraints(RehWawBehSetup& rehWawBeh,
                                  SolverContext& solverContext,
                                  const OptParams& P) {
  // Lower stacking uses semantic ownership, keeps the authored horizontal
  // offset, and lets kasra take the correction rather than displacing dots.
  // Contextual kasra follows its dot's X target as well as its vertical order.
  for (auto& [base, group] : rehWawBeh.stackGroups) {
    for (auto* dots : group) {
      if (!solverContext.downdotmarks.contains(dots->glyphName)) continue;
      for (auto* kasra : group) {
        if (!isKasraMark(*kasra)) continue;
        // Initial positioning is now the input baseline, just as if HarfBuzz
        // had produced it. Preserve that pair's horizontal offset in XPBD.
        const double authoredOffset = boxCenterX(*dots) - boxCenterX(*kasra);
        auto stack = std::make_unique<StackOrderConstraint>(
            *dots, *kasra, false, P.rehWaw.dotKasraGap, P.compliance.stackOrderGap,
            solverContext.contextualBelowMarks.contains(kasra) ? P.compliance.rehWawKasraFollow : P.compliance.stackOrderXAlign,
            authoredOffset, true);
        stack->keepInnerForGap = true;
        rehWawBeh.stacks.push_back(std::move(stack));
        solverContext.excludedFromGenericGap.insert(
            {std::min(dots->globalIndex, kasra->globalIndex), std::max(dots->globalIndex, kasra->globalIndex)});
      }
    }
  }
}

void appendRehWawBehHardConstraints(RehWawBehSetup& rehWawBeh,
                                    const GlyphSequence& glyphs,
                                    const OptParams& P,
                                    ConstraintList& hardConstraints) {
  // Preserve each independent mark's selected slot. Blanket below-base
  // bounds would undo a standalone kasra's current-gap or left-tip choice.
  // Iterate the shaped sequence for deterministic projection order.
  for (auto& ref : glyphs) {
    auto& mark = ref.get();
    if (mark.initialPlacement == GlyphInstance::InitialPlacement::LeftOfRehWaw) {
      if (auto* reference = rehWawReference(mark))
        hardConstraints.push_back(std::make_unique<HorizontalGapConstraint>(*reference, mark, P.rehWaw.leftGap));
    } else if (mark.initialPlacement == GlyphInstance::InitialPlacement::BelowRehWaw) {
      if (auto* reference = rehWawReference(mark)) {
        // Prefer the owner center and reference bottom only where base ink
        // allows it. A deeper bowl may require the already validated initial
        // position; neighboring marks do not determine these preferences.
        double preferredX = boxCenterX(*mark.prevBase);
        const double nearTop = boxBottomY(*reference) - P.rehWaw.tipGap;
        double preferredTop = nearTop;
        const auto& planner = *rehWawBeh.placements.at(&mark);
        auto baseClear = [&](double x, double top) {
          return planner.hasInkClearance(mark.worldPolys.translate(
                                             x - boxCenterX(mark), top - boxTopY(mark)),
                                         false);
        };
        if (!baseClear(preferredX, preferredTop)) {
          if (baseClear(boxCenterX(mark), nearTop))
            preferredX = boxCenterX(mark);
          else if (baseClear(preferredX, boxTopY(mark)))
            preferredTop = boxTopY(mark);
          else {
            preferredX = boxCenterX(mark);
            preferredTop = boxTopY(mark);
          }
        }
        const double ownerOffset = boxCenterX(*reference) - preferredX;
        auto bound = std::make_unique<StackOrderConstraint>(*reference, mark, false,
                                                            P.rehWaw.tipGap, P.compliance.hardStayBelow,
                                                            P.compliance.stackOrderXAlign, ownerOffset);
        bound->keepInnerForGap = true;
        bound->preferredGap = boxBottomY(*reference) - preferredTop;
        bound->preferredGapCompliance = P.compliance.rehWawBelowBottom;
        hardConstraints.push_back(std::move(bound));
      }
    }
  }
  // Dot ordering follows the base bounds, since a contextual dot may itself
  // have moved down around a descending stroke during this iteration.
  for (auto& stack : rehWawBeh.stacks) hardConstraints.push_back(std::move(stack));
  if (P.toggles.baseVicinity) {
    for (auto& ref : glyphs) {
      auto& mark = ref.get();
      const bool belowIndependentMark = mark.initialPlacement == GlyphInstance::InitialPlacement::BelowRehWaw;
      const auto it = rehWawBeh.placements.find(&mark);
      const bool followingBelowDot = isKasraMark(mark) && it != rehWawBeh.placements.end() &&
                                     !it->second->dots.empty() &&
                                     it->second->dots.front()->initialPlacement == GlyphInstance::InitialPlacement::BelowRehWaw;
      if ((!belowIndependentMark && !followingBelowDot) || mark.preserveGposHorizontal) continue;
      const auto box = mark.worldPolys.boundingAABB();
      const double margin = (box.maxx - box.minx) * 0.5;
      hardConstraints.push_back(std::make_unique<BaseVicinityConstraint>(
          mark, margin, P.compliance.baseVicinityLeft, P.compliance.baseVicinityRight));
    }
  }
}

void addBowlClusterConstraints(const MarkGroups& bowlMarkGroups,
                               SolverContext& solverContext,
                               const OptParams& P,
                               ConstraintList& placementConstraints) {
  // ---- Bowl-cluster centering: marks trapped inside a bowl base ----
  // Runs AFTER the main loop so each base's full mark cluster is known. Each
  // cluster is centered as a block. The members opt out of the generic gap
  // entirely (gapExcludedGlyphs), not just against each other and their base:
  // a member that could still be pushed by the gap against an ADJACENT base
  // would drag the group's midX off-center, and BowlClusterConstraint's block
  // re-centering would then slide every member and create a fresh overlap.
  for (auto& [base, members] : bowlMarkGroups) {
    for (GlyphInstance* m : members) {
      solverContext.gapExcludedGlyphs.insert(m->globalIndex);
    }
    placementConstraints.push_back(std::make_unique<BowlClusterConstraint>(
        *base, members, /*compliance=*/P.compliance.bowlCluster, /*interiorInset=*/0.0));
  }
}

void addHarakatLaneConstraints(const MarkGroups& aboveStackGroups,
                               const OptParams& P,
                               ConstraintList& placementConstraints) {
  // ---- Two-sided harakat lane: one constraint per base, on its anchor ----
  // Runs AFTER the main loop so aboveStackGroups is fully populated (the loop
  // fills aboveStackGroups[mark.prevBase] as it visits each above-mark).
  auto isBandParticipant = [](const std::string& n) {
    return n.starts_with("fatha") || n.starts_with("damma") ||
           n.starts_with("sukun");
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

    // Anchor = bottom-most participating fatha/damma/sukun. Dots are in
    // `group` via topdotmarks but are excluded from the lane.
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
      placementConstraints.push_back(std::make_unique<YlaneConstraint>(
          *anchor, /*compliance=*/P.compliance.ylane, /*laneHeight=*/600.0));
    }
  }
}

PageConstraints buildPageConstraints(std::vector<std::vector<GlyphInstance>>& pageGlyphs,
                                     const GlyphSequence& glyphs,
                                     SolverContext& solverContext,
                                     const OptParams& P,
                                     InitialPositioning initialPositioning) {
  ValidBases validBases;
  for (const auto& ref : glyphs) {
    const auto& g = ref.get();
    if (!g.isMark && g.glyphName != "space" && !g.worldPolys.empty()) validBases.insert(&g);
  }
  PageConstraints constraints;
  RehWawBehSetup rehWawBeh;
  if (P.toggles.rehWawBehPlacement)
    prepareRehWawBehPositions(glyphs, validBases, solverContext, P, rehWawBeh, initialPositioning);
  std::vector<WaqfPlacementConstraint*> waqfPlacements;
  if (P.toggles.waqfEscape && P.toggles.waqfPlacement && P.toggles.genericGapConstraint)
    solverContext.waqfEscapes.resize(glyphs.size(), nullptr);

  MarkGroups aboveStackGroups;

  // Per-base groups of marks trapped inside a bowl-shaped base (Jeem/Hah/Khah).
  // Each group is centered as a block by one BowlClusterConstraint built after
  // the loop; the members are excluded from the generic gap there.
  MarkGroups bowlMarkGroups;

  // Bowl membership is geometric and independent of the gap-exclusion list.
  // An eligible non-above mark enrolls its base's other non-above marks.
  std::unordered_set<GlyphInstance*> bowlBases;
  if (P.toggles.bowlCluster) {
    for (auto& gRef : glyphs) {
      GlyphInstance& g = gRef.get();
      if (!g.isMark || !hasValidOwner(g, validBases)) continue;
      const bool isAbove = solverContext.topmarks.contains(g.glyphName) ||
                           solverContext.topdotmarks.contains(g.glyphName);
      if (isAbove) continue;
      if (solverContext.hasBowlInterior(*g.prevBase)) {
        bowlBases.insert(g.prevBase);
      }
    }
  }

  for (size_t i = 0; i < glyphs.size(); ++i) {
    GlyphInstance& mark = glyphs[i];
    if (!mark.isMark || !hasValidOwner(mark, validBases)) continue;
    // NOTE: the harakat lane (YlaneConstraint) is built per-base in a second
    // pass after this loop, once aboveStackGroups is fully populated.

    if (solverContext.isWaqf(mark)) {
      if (P.toggles.waqfPlacement) {
        auto placement = std::make_unique<WaqfPlacementConstraint>(
            mark, P.compliance.waqfMin, P.compliance.waqfMax,
            /*minDistFromBaseline=*/P.waqfMinHeight, /*minGapToBase=*/100.0,
            /*minGapToTopMarks=*/50.0, /*upperCeilingY=*/1400.0);
        placement->maxLeftPercent = P.waqfEscapeMaxLeftPercent;
        placement->maxRightPercent = P.waqfHorizontalAlignmentBandPercent;
        placement->leftCompliance = P.compliance.baseVicinityLeft;
        placement->rightCompliance = P.compliance.baseVicinityRight;
        waqfPlacements.push_back(placement.get());
        if (!solverContext.waqfEscapes.empty()) {
          auto escape = std::make_unique<WaqfEscapeConstraint>(*placement, P);
          solverContext.waqfEscapes[mark.globalIndex] = escape.get();
          constraints.waqfEscapes.push_back(std::move(escape));
        }
        constraints.placement.push_back(std::move(placement));
      }
      // Waqf marks already get a full placement solve above; skip the
      // generic below rail/vicinity/squeeze handling for them.
      continue;
    }

    const bool isAboveMark = solverContext.topmarks.contains(mark.glyphName) ||
                             solverContext.topdotmarks.contains(mark.glyphName);
    const bool isBelowMark = solverContext.lowmarks.contains(mark.glyphName) ||
                             solverContext.downdotmarks.contains(mark.glyphName);

    // Even manually positioned bowl marks must not rise above their base.
    // Add this before their exclusion without changing the existing gap policy.
    // Bases stay fixed during XPBD: bottom - (-height) is their ink top.
    const bool bowlLowerMark = !isAboveMark && isBelowMark &&
                               solverContext.hasBowlInterior(*mark.prevBase);
    if (bowlLowerMark && P.toggles.hardStayAboveBelow)
      constraints.hard.push_back(std::make_unique<HardStayBelowConstraint>(
          mark, P.compliance.hardStayBelow,
          /*maxGapToBase=*/-boxHeight(*mark.prevBase), /*maxDistToBaseline=*/0.0,
          /*useBaseBottom=*/true));

    // Authored bowl interiors retain their dedicated placement policy.
    const bool manuallyPositioned = isManuallyPositionedMark(mark, solverContext);
    if (manuallyPositioned) {
      solverContext.gapExcludedGlyphs.insert(mark.globalIndex);
      continue;
    }

    const bool hasRehWawPlacement = solverContext.contextualBelowMarks.contains(&mark);

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
      // The original glyph indices also account for intervening glyphs that
      // are not in this group, so no special hamza membership is needed.
      if (isAboveMark)
        aboveStackGroups[mark.prevBase].push_back(&mark);

      // Generic hard stay-above/stay-below rail
      if (isAboveMark) {
        if (mark.baselineClearance) {
          // A font-local floor protects the low tail even outside its owner's
          // ink footprint. Do not use the generic OR-with-base-top exception.
          constraints.hard.push_back(std::make_unique<HardStayAboveConstraint>(mark, P.compliance.hardStayAbove, 0.0, *mark.baselineClearance, false, false));
        } else if (P.toggles.hardStayAboveBelow) {
          auto minDistFromBaseline = 100.0;
          if (mark.prevBase != nullptr && mark.prevBase->glyphName == "yehshape.fina.ii" && mark.glyphName == "smallalef.replacement") {
            minDistFromBaseline = 0;
          }
          constraints.hard.push_back(std::make_unique<HardStayAboveConstraint>(
              mark, P.compliance.hardStayAbove, /*minGapToBase=*/0.0, minDistFromBaseline, /*useBaseTop=*/false, true));
        }
      } else if (isBelowMark && !bowlLowerMark) {
        if (P.toggles.hardStayAboveBelow) {
          constraints.hard.push_back(std::make_unique<HardStayBelowConstraint>(
              mark, P.compliance.hardStayBelow, /*maxGapToBase=*/0.0, /*maxDistToBaseline=*/0.0,
              /*useBaseBottom=*/false, /*testBothReferences=*/true,
              /*belowBaselineFraction=*/0.66));
        }
      }

      // Structural squeeze centering (no glyph-name matching)
      if (P.toggles.squeezeCenter && !mark.preserveGposHorizontal && !hasRehWawPlacement) {
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
          constraints.placement.push_back(std::make_unique<SqueezeCenterConstraint>(mark, P, obstacles, P.compliance.squeezeCenter));
        }
      }

      // Base-vicinity leash (ownership ambiguity)
      if (P.toggles.baseVicinity && !mark.preserveGposHorizontal && !hasRehWawPlacement) {
        const MarkRole role = classifyMark(mark);
        const auto bounds = mark.worldPolys.boundingAABB();
        const double width = bounds.maxx - bounds.minx;
        const double margin = (role == MarkRole::Dots) ? width * 0.5 : width;
        constraints.placement.push_back(std::make_unique<BaseVicinityConstraint>(
            mark, margin, P.compliance.baseVicinityLeft, P.compliance.baseVicinityRight));
      }
    }

    // Compound-mark cohesion (kasra + smalllowmeem forming one visual
    // unit): not a collision/placement rule, keep as an explicit pairing.
    if (P.toggles.matchMarkPosition && mark.glyphName == "smalllowmeem" && i > 0) {
      GlyphInstance& prevMark = glyphs[i - 1];
      if (prevMark.glyphName == "kasra" && prevMark.prevBase == mark.prevBase &&
          prevMark.lineIndex == mark.lineIndex) {
        constraints.placement.push_back(std::make_unique<MatchMarkPositionWithOffsetConstraint>(
            prevMark, mark, 130, 0, P.compliance.matchMarkPosition));
      }
    }
  }

  if (P.toggles.rehWawBehPlacement && P.toggles.stackOrder)
    addRehWawBehStackConstraints(rehWawBeh, solverContext, P);

  if (P.toggles.bowlCluster)
    addBowlClusterConstraints(bowlMarkGroups, solverContext, P, constraints.placement);

  if (P.toggles.ylane)
    addHarakatLaneConstraints(aboveStackGroups, P, constraints.placement);

  // Explicit, case-by-case top-mark stacking
  if (P.toggles.stackOrder) {
    for (auto& [base, group] : aboveStackGroups) {
      if (base->collisionAsMark || base->glyphName == "smallalef.isol") {
        auto stackGroup = group;
        stackGroup.push_back(base);
        addTopMarkStackConstraints(stackGroup, P, constraints.placement, solverContext.excludedFromGenericGap);
      } else {
        addTopMarkStackConstraints(group, P, constraints.placement, solverContext.excludedFromGenericGap);
      }
      // Any combination of top marks not covered by a table row is left
      // unstacked until a rule for it is defined.
    }
  }

  if (P.toggles.horizontalOrder) {
    for (auto& lineGlyphs : pageGlyphs) {
      std::vector<GlyphInstance*> previousMarks, currentMarks;
      for (auto& markB : lineGlyphs) {
        if (!markB.isMark) {
          previousMarks = std::move(currentMarks);
          currentMarks.clear();
          continue;
        }
        if (!hasValidOwner(markB, validBases)) continue;
        // These substitutions absorb tatweel into a mark glyph. Its invisible
        // base still starts a separate horizontal owner, even though prevBase
        // points to the preceding visible letter. Later marks (e.g. maddah or
        // damma) belong to this owner until another base or joined mark starts.
        if (markB.glyphName == "smallalef.joined" || markB.glyphName == "hamzaabove.joined") {
          previousMarks = std::move(currentMarks);
          currentMarks.clear();
        }
        // Manually-positioned marks (see the manuallyPositioned continue
        // above, which also populates gapExcludedGlyphs for them) opt out of
        // this too -- the caller's hand-placed position is the whole point.
        if (solverContext.gapExcludedGlyphs.contains(markB.globalIndex)) continue;
        for (auto* markA : previousMarks) {
          if (markA->lineIndex == markB.lineIndex && markA->isTopMark == markB.isTopMark &&
              !solverContext.isWaqf(*markA) && !solverContext.isWaqf(markB)) {
            constraints.placement.push_back(std::make_unique<HorizontalOrderConstraint>(
                *markA, markB, P.horizontalOrderSelfKeep, P.horizontalOrderCrossKeep, P.compliance.horizontalOrder));
          }
        }
        currentMarks.push_back(&markB);
      }
    }
  }

  // Projection order after generic contacts: ordinary rails (already added),
  // Reh/Waw directional mark bounds, dot-kasra stacks,
  // then Waqf top order.
  // Appending these in a different order can change the solved placement.
  if (P.toggles.rehWawBehPlacement)
    appendRehWawBehHardConstraints(rehWawBeh, glyphs, P, constraints.hard);

  // Apply waqf top ordering after all ordinary mark rails, so their upward
  // corrections cannot leave the waqf below its own stack's highest top.
  // Preserve the existing search slot; the frame bound below corrects overflow
  // without narrowing the search into selecting the opposite side of the base.
  if (initialPositioning == InitialPositioning::Apply && P.toggles.waqfPlacement)
    prepareWaqfPositions(waqfPlacements, solverContext, P);
  for (auto* placement : waqfPlacements)
    constraints.hard.push_back(std::make_unique<WaqfTopOrderConstraint>(*placement));
  // The outer page border takes priority over every placement/contact rule,
  // including authored marks excluded from ordinary gaps and pinned dots.
  for (auto& ref : glyphs) {
    auto& mark = ref.get();
    if (mark.isCollisionMark() && mark.frameOriginBounds)
      constraints.hard.push_back(std::make_unique<MarkFrameConstraint>(mark, *mark.frameOriginBounds));
  }
  return constraints;
}

void solvePageConstraints(const GlyphSequence& glyphs,
                          SolverContext& solverContext,
                          const OptParams& P,
                          PageConstraints& constraints) {
  // Exclusion policies are fixed after assembly. Cache their precedence once:
  // glyph-wide exclusions, explicit inclusions, then dedicated-pair exclusions.
  const size_t glyphCount = glyphs.size();
  std::vector<unsigned char> gapExclusions;
  if (P.toggles.genericGapConstraint && P.maxIters > 0) {
    gapExclusions.resize(glyphCount * glyphCount, 0);
    const auto setExcluded = [&](int a, int b, unsigned char value) {
      gapExclusions[std::min(a, b) * glyphCount + std::max(a, b)] = value;
    };
    for (int a : solverContext.gapExcludedGlyphs)
      for (int b = 0; b < static_cast<int>(glyphCount); ++b) setExcluded(a, b, 1);
    for (auto [a, b] : solverContext.gapIncludedPairs) setExcluded(a, b, 0);
    for (auto [a, b] : solverContext.excludedFromGenericGap) setExcluded(a, b, 1);
  }

  const double dt = 1.0;
  // Per iteration: Waqf escape, placement targets, refreshed broadphase,
  // generic contacts, ordered hard bounds, then convergence checks.
  std::vector<std::pair<int, int>> pairs;
  const double toleranceSquared = P.tolCollision * P.tolCollision;
  for (int iter = 0; iter < P.maxIters; ++iter) {
    for (auto& ref : glyphs) ref.get().iterationMaxMovementSquared = 0.0;
    const bool gapEscapePending = P.toggles.genericGapConstraint &&
        beginGapPass(solverContext, glyphs, P, P.maxIters - iter);
    // Consume the previous ordinary gap pass, then clear its samples. The
    // existing broadphase/contact pass below handles any induced movement.
    for (auto& escape : constraints.waqfEscapes) {
      escape->project(solverContext, dt);
      escape->contacts.clear();
    }

    for (auto& c : constraints.placement) {
      c->project(solverContext, dt);
    }
    // Lane/stack/placement projections can move marks beyond the old candidate
    // horizon. Build candidate pairs from the updated world geometry.
    SweepBroadphase(glyphs, P).findPairs(pairs);
    double maxPenetration = 0.0;

    // Resolve generic contacts before the ordered hard bounds.
    if (P.toggles.genericGapConstraint) {
      for (auto& pair : pairs) {
        GlyphInstance& glyphA = glyphs[pair.first];
        GlyphInstance& glyphB = glyphs[pair.second];
        // These pairs already return immediately from the gap projection.
        if (!(glyphA.isCollisionMark() || glyphB.isCollisionMark()) ||
            (glyphA.mobility <= 0.0 && glyphB.mobility <= 0.0)) continue;
        if (!gapExclusions[pair.first * glyphCount + pair.second]) {
          solveGapConstraint(solverContext, glyphA, glyphB, P, dt, maxPenetration);
        }
      }
    }

    for (auto& c : constraints.hard) {
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
      for (const auto& c : constraints.placement) c->reportViolations(solverContext, remaining);
      for (const auto& c : constraints.hard) c->reportViolations(solverContext, remaining);
      if (P.toggles.genericGapConstraint) collectGapViolations(solverContext, glyphs, P, remaining);
      const bool hardBoundUnresolved = std::any_of(remaining.begin(), remaining.end(), [&](const auto& v) {
        return v.kind == ViolationKind::Hard && v.severity > P.tolCollision;
      });
      if (!hardBoundUnresolved && !gapEscapePending) break;
    }
  }
}

// Read-only final reporting is separate from convergence checks: report
// filtering and residual allowances never alter constraint projection.
void collectPageViolations(const GlyphSequence& glyphs,
                           SolverContext& solverContext,
                           const OptParams& P,
                           const PageConstraints& constraints,
                           std::vector<ConstraintViolation>& violations) {
  for (const auto& c : constraints.placement) {
    if (c->reportEnabled) c->reportViolations(solverContext, violations);
  }
  for (const auto& c : constraints.hard) {
    if (c->reportEnabled) c->reportViolations(solverContext, violations);
  }
  for (const auto& escape : constraints.waqfEscapes)
    if (escape->reportEnabled) escape->reportViolations(solverContext, violations);
  if (P.toggles.reportGenericGap && !P.reportGenericGapCollisionsOnly) {
    collectGapViolations(solverContext, glyphs, P, violations);
  }
  if (!P.toggles.reportSoftResiduals) {
    std::erase_if(violations, [](const auto& v) { return v.kind == ViolationKind::Soft; });
  }
  if (!P.toggles.reportWaqfBounds) {
    std::erase_if(violations, [](const auto& v) {
      return v.type == ViolationType::WaqfBoundsResidual;
    });
  }
  // Association diagnostics are collected only for reports. They neither
  // participate in convergence nor depend on the soft-target/force switches.
  collectWaqfPlacementViolations(solverContext, P, violations);
  // The optional audit is independent of the corresponding force switches.
  // Geometric ownership warnings are retained regardless of the soft-target
  // preference switch: they indicate placement ambiguity, not a tuning goal.
  if (P.toggles.reportPlacementAudit)
    collectPlacementViolations(solverContext, violations);

  // Annotate only the final report. The convergence check above continues
  // to evaluate raw hard residuals, independently of reporting allowances.
  for (auto& v : violations) {
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
  std::erase_if(violations, [&](const ConstraintViolation& v) {
    return !v.structural &&
           ((v.type == ViolationType::BaseVicinity && v.severity <= v.allowedResidual) ||
            violationReportSeverity(v) < P.minViolationSeverity);
  });
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
                  std::vector<ConstraintViolation>* outViolations,
                  InitialPositioning initialPositioning) {
  // 1. Prepare geometry and ownership indices.
  std::unordered_set<GlyphInstance*> collisionBases;
  for (auto& line : pageGlyphs) for (auto& g : line) {
    g.collisionAsMark = !g.isMark && isAdditionalCollisionMark(g.glyphName, P);
    g.collisionAttachments.clear();
    if (g.collisionAsMark) collisionBases.insert(&g);
  }
  for (auto& line : pageGlyphs) for (auto& g : line)
    if (g.isMark && collisionBases.contains(g.prevBase))
      g.prevBase->collisionAttachments.push_back(&g);
  initGlyphMobilities(pageGlyphs);
  SolverContext solverContext{pageGlyphs, classes};
  solverContext.rehWawBehPlacement = P.toggles.rehWawBehPlacement;
  const auto glyphs = prepareGlyphs(pageGlyphs);

  // 2. Assemble constraints once, preserving their projection order.
  auto constraints = buildPageConstraints(pageGlyphs, glyphs, solverContext, P, initialPositioning);

  // 3. Project constraints and check convergence.
  solvePageConstraints(glyphs, solverContext, P, constraints);

  // 4. Collect optional diagnostics from the final geometry.
  if (outViolations)
    collectPageViolations(glyphs, solverContext, P, constraints, *outViolations);
}

}  // namespace digitalkhatt::layout
