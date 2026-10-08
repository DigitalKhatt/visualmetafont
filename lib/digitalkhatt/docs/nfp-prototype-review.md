# Cached no-fit polygon prototype — 2026-10-08

## Reproduction

Font: Old Madina. Layout: qpc_v1_layout. Corpus: 604 pages, 848,298 glyphs,
435,360 marks. Base settings: `output/pdf/mushaf-cli/mushaf.settings.json`.
Both methods use the same XPBD constraints, mobility, compliance, 20 iterations,
80-unit preferred generic gap and existing exceptions, hard above/below rails,
waqf height 5.0 / horizontal compliance 0.24, and integer output conversion.

Only generic-gap contact geometry differs. The prototype is enabled with
`--contact-method nfp`; the default remains `gjk`. Dedicated constraints are
unchanged. Reports keep findings unlimited but explicitly disable the independent
placement audit for both methods, overriding saved preferences. BaseAssociation,
classification and side audit diagnostics are therefore absent. The unchanged
Save Collision detector measures a 10-unit minimum clearance. Timing runs disable PDF/report generation, alternate method
order, and use a fresh process/cache each run.

```sh
python3 visualmetafont/lib/digitalkhatt/tools/compare_contacts.py \
  --executable build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config output/pdf/mushaf-cli/mushaf.settings.json \
  --output-dir output/pdf/nfp-comparison --pdf --timing-runs 3
```

NFP receives a separate `buildPolyFromCubics()` outline for **every glyph**,
including bases, with the same scale as the ordinary solver geometry. Its input
is independent of `buildGlyphCollisionGeometry()`. NFP may decompose these
outlines internally to construct Minkowski sums, but unions the complete
configuration-space region before selecting a separation. Other constraints and
Save Collision retain their existing geometry.

## Measured results

| Metric | Current GJK/EPA | NFP prototype |
| --- | ---: | ---: |
| Mean elapsed time, no PDF/report | 17.60 s | 37.33 s |
| Mean placement time, no PDF/report | 7.39 s | 26.90 s |
| Reported ink intersections | 0 | 0 |
| Save Collision clearance findings | 0 | 2 |
| BaseVicinity findings | 530 | 483 |
| HorizontalOrder findings | 86 | 79 |
| StackOrderGap findings | 100 | 87 |
| WaqfPlacement findings | 150 | 141 |

GJK elapsed runs: 17.476, 17.774, 17.543 seconds.
NFP elapsed runs: 37.585, 37.358, 37.057 seconds.
NFP is 2.12 times the elapsed time in this version.
There are 866 findings with GJK and 792 with NFP. Warning counts alone do not
establish typographic correctness. Disabling the read-only placement audit
left the recorded mark movements unchanged. The earlier switch to unsplit base
input also preserved that version's constraint counts and movement totals.

The two NFP findings are **clearance shortfalls, not ink intersections**:

- Page 270, line 4, word 9: kasra / smalllowmeem, 0.15048 units short of 10.
- Page 58, line 3, word 7: kasra / smalllowmeem, 0.09688 units short of 10.

They are smaller than the solver's 0.5-unit convergence tolerance and are audited
on the rounded exported positions. The comparison does not hide these findings
or change the kasra / meem-iqlab policy to improve the score.

Mean mark movements (world font units):

| Role | GJK/EPA | NFP |
| --- | ---: | ---: |
| dots | 8.65 | 7.43 |
| waqf | 64.09 | 63.12 |
| other-marks | 35.08 | 34.79 |

The comparison identifies 99 removed findings and
25 new findings; changed severity and complete
finding records are in `output/pdf/nfp-comparison/comparison.json`.

## Contact consistency and optimization

Single-convex pairs use the identical GJK/EPA function, world vertices, pair
order, requested gap and XPBD projection as the default solver. Eligibility
requires both raw outlines and ordinary solver geometries to be single convex
polygons. A convex mark against a base with multiple pieces still uses the NFP
union. The ordinary placement constraints, including fatha attachment policy,
are unchanged; the proposed attachment bands are deferred.

The whole-outline path expands by exactly the requested gap. The former
0.025-unit allowance is removed. Polygon/grid approximation and equally valid
boundary directions can still differ on this path. Convex sum construction now
merges CCW edge sequences in O(n+m), instead of sorting all n*m vertex
combinations. Convex components skip decomposition. The default pair-region
capacity is 16,384, and the comparison explicitly overrides an older saved limit.

| NFP configuration | Mean elapsed, PDF/report off |
| --- | ---: |
| Previous implementation, 4,096 entries (three runs) | 46.80 s |
| Previous implementation, 16,384 entries (two trials) | 39.79 s |
| Current implementation, 16,384 entries (three runs) | 37.33 s |

Total elapsed time falls by 20.2%, and placement time by 26.0%, against the
previous 4,096-entry implementation. The previous comparison data is archived at
`tmp/nfp-comparison/pre-optimizations/`. All 12 NFP review cases differ by less
than 0.081 world font units in displacement. Finding identities and the two
rounded Save Collision clearance measurements are unchanged; the raw
displacement fields have small changes.

The current report run registered 30,550 shapes and used shared convex
GJK/EPA contacts 1,228,417 times. It built 65,242 regions
across 8,180,989 whole-outline queries, with 8,115,747 hits
(99.20%). Construction consumed 13.98 seconds
and boundary queries 5.84 seconds. The LRU retained
16,384 regions and evicted 48,858.

The larger cache uses more memory: retained expanded boundary vertices increase
from approximately 1.22 million to 4.89 million. This is a
vertex count, not a total memory measurement. Translation does not invalidate a
region; outline and scale variants do. Reusing unexpanded regions under a common
line transform remains a possible later optimization.

## Correctness scope and verification

- Native CLI and Release/Debug GUI builds succeed.
- `digitalkhatt.no_fit_polygon` passes: translation invariance and cache reuse,
  reversed pairs, outline changes, complete-union separation, clearance,
  concavities, unsplit base input and scaling, union holes, expanded pockets,
  and LRU eviction. Added checks cover exact shared GJK/EPA contacts, removal
  of the clearance allowance, convex classification and 500 seeded random
  edge-merge comparisons against an independent quadratic hull reference.
- The default GJK constraint finding counts and mark movement summaries match
  the prior XPBD comparison exactly after excluding the disabled audit families.
  All 11 shared requested word previews, including their surrounding ink and
  displacements, match that prior comparison exactly.
- An earlier baseline check of `digitalkhatt.placement_safety` failed its first side-audit
  assertion. It leaves `reportPlacementAudit` disabled while expecting side
  findings. An isolated build from pristine HEAD reproduces the same failure;
  the unrelated test was left unchanged. Log:
  `tmp/nfp-comparison/head-baseline-test.log`.
- Both Mushaf PDFs have 605 pages: 604 corpus pages plus the configured notice.
  Representative page 153 renders for both methods and NFP page 465 were inspected earlier.
  Updated NFP pages 153 and 565 were inspected after optimization. The offline viewer contains
  12 paired cases and passes JavaScript syntax and embedded-data checks. Browser
  automation cannot open local file URLs in this environment.

The geometry uses the existing 5-unit cubic flattening tolerance and filled-hole
policy. Clearance arcs use a 0.02-unit approximation tolerance without an additional
clearance allowance. This is a polygon prototype, not exact cubic collision
certification. NFP's signed expanded-region residual is kept distinct from ink
intersection in full gap reports. A pair correction can still conflict with
other placement constraints or neighboring glyphs.

## Artifacts and recommendation

`output/pdf/nfp-comparison/comparison.html` provides paired word previews, glyph
hover details, timings, and links to both complete Mushaf PDFs and the two
violation viewers. CSV/PDF reports and completed manifests are alongside each
Mushaf under `gjk/` and `nfp/`.

Keep GJK/EPA as the default. NFP reduces some constraint findings but is
substantially slower and does not improve the current zero-intersection result. Keep it optional for visual review and
further cache/construction optimization.
