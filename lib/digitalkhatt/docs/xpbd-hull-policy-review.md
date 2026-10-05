# Restored mark hull policy - 2026-10-05

> Historical run: the additional placement guards were enabled for these
> measurements. They have since been removed at the user's request; current
> XPBD output is not clamped or restored by those guards.

## Correction

Marks intentionally use `buildPolyFromCubics`, while bases use
`buildConvexPartsFromCubics`. GJK's `supportPoint` searches all vertices for the
furthest point in a direction. It therefore implicitly tests the convex hull
of each unsplit component. Passing a concave outline to this support mapping
does not mean that GJK tests its concave interior.

Splitting a connected mark into convex pieces changed that coherent hull
contact into contacts against individual pieces. The earlier description of
this change as a required correctness fix was wrong. The intended policy is
restored in the shared GUI/CLI adapter through `buildGlyphCollisionGeometry`
in `include/digitalkhatt/layout/GlyphCollisionGeometry.h`. Disconnected outer
components remain separate, as in the original implementation. The semantic
bounds and final contact-restoration guard were enabled for this run.

The regression test uses a concave L-shaped outline and a small rectangle
inside its empty concave region. The mark proxy must report contact through
its implicit hull; decomposed base parts must preserve the free space. This
test exercises the same geometry helper used by the placement pipeline.

## Whole-corpus rerun

Old Madina and the saved QPC v1 GUI profile were rerun over all 604 pages,
9,046 lines and 435,360 marks, with Force and reports enabled. Outputs and
resolved configuration are under `output/pdf/xpbd-corpus/hull-policy/`.

| Finding | Count |
| --- | ---: |
| BaseAssociation | 0 |
| MarkSide | 3 existing contextual flags, none new or worsened |
| MarkClassification | 277 existing missing roles (`hamzaabove.lamalef`) |
| PlacementRejected | 5 restored proposals across 4 pages |
| Collision-proxy intersection | 3 existing contacts, none new or worsened |
| Minimum gap not reached | 73,760 |
| BaseVicinity | 334 |
| HorizontalOrder | 141 |
| StackOrderGap | 344 |
| WaqfPlacement | 740 |

There are 75,607 finding rows, including 75,325 hard and 282 soft findings.
The three retained contacts are on page 120 (line 11/12, kasra/waqf.sad and
meem.fina.ii/waqf.sad) and page 453 (line 5, fatha/lam.medi.laf). Candidate
restorations occur on pages 120, 399, 453 and 465. Rejected candidates are not
exported. Original contextual side and role warnings remain as described in
the historical corpus review.

These contacts describe collision proxies. A hull can cover empty regions of
the visible glyph, so proxy penetration is not proof of filled-ink intersection.
Report labels now say `Collision-proxy intersection` and the candidate event
details say `proxy penetration`. The CSV severity for generic-gap findings still
includes the desired clearance target. Contact counts across the temporary
decomposed and restored-hull runs measure different proxies and must not be
interpreted as an equivalent count of physical ink collisions.

## Verification

The GUI and native builds succeeded. All seven registered layout tests passed,
including the new intentional-hull regression. The 605-page Mushaf (notice plus
604 Quran pages) passed content, notice/link and rendered-sample checks.
Every Quran page rendered without Poppler diagnostics. Contact sheets covering
all pages, opening/ending samples, the report summary and enlarged pages 120/453
were inspected. This is automated placement review and visual sampling, not a
word-by-word human Quran proofread. Full reports, CSV, audit JSON and source
fingerprints are retained with the output.

## Restored-policy performance

Four counterbalanced rounds over the same 604 shaped pages, with reports and
PDF generation disabled, gave these medians:

| Version | Core solver | Complete placement |
| --- | ---: | ---: |
| Before review (`0421bbd`) | 6.903 s | 7.354 s |
| Restored mark policy, current safety enabled | 9.042 s | 9.526 s |
| Historical core with current adapter | 6.919 s | 7.393 s |
| Current core with safety disabled for attribution | 7.501 s | 7.964 s |

The remaining increase is **31.0% in the core** and **29.5% for complete
placement**. Both actual versions use 20 iterations per page. Current core
timings ranged from 8.930 to 9.063 seconds. Observed safety phase medians are
0.179 seconds for bounds/snapshots, 0.825 seconds for per-iteration projection
and 0.496 seconds for final rounding/contact restoration. Convergence checks
cost approximately 0.007 seconds. Safety-disabled timing is an experimental
cost attribution, not a recommended production configuration.

The previous decomposed-mark experiment measured 11.339 seconds for the current
core in a separate four-round batch. The restored-policy run is roughly 20%
faster than that result; the matched before/current comparison in this batch
is the more reliable measure of the remaining overhead. The unchanged-rebuild
optimization proposed in the earlier performance report remains applicable,
with the projection cost now approximately 0.825 seconds. It has not been
applied as part of restoring the collision policy.

Compiler/reference metadata, raw timings and medians are retained under
`output/benchmarks/xpbd-hull-policy-2026-10-05/`. Both cores use the same Release
flags and unchanged geometry implementation, with outlines prepared before
timing. No task-owned renderer, build or other benchmark overlapped the timed
rounds. Geometry-policy selection in the benchmark uses the same helper as
the actual current adapter.

## Reproduction

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config output/pdf/xpbd-corpus/hull-policy/mushaf.settings.json \
  --output mushaf.pdf
```
