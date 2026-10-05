# Current XPBD performance after guard removal — 2026-10-05

> The initial comparison and movement-tracking experiment below were measured
> before applying the five simple optimizations. The final section contains the
> matched measurement of those optimizations.

## Results

Six rounds over the complete 604-page Old Madina corpus, with Force enabled,
reports disabled and no PDF generation, produced these medians:

| Version | XPBD core | Complete placement |
| --- | ---: | ---: |
| Before review (`0421bbd`) with original adapter | 6.735 s | 7.188 s |
| Current code, additional placement guards removed | 7.266 s | 7.729 s |
| Historical core with current adapter and geometry | 6.775 s | 7.256 s |

Current core time increases by **0.531 seconds (7.9%)** relative to the original.
Complete placement increases by **0.542 seconds (7.5%)**. Complete placement
includes input copying, collision geometry preparation and offset export;
shaping, font initialization and PDF/report generation are excluded.

The core ranges were 6.614–7.009 seconds before and 7.152–7.395 seconds now.
The median within-round core difference was 0.530 seconds (8.0%), consistent
with the difference between the overall medians despite timing drift.

A separate run of the production CLI completed all 604 pages in **16.114
seconds**, including font initialization, shaping and placement, with both PDF
and reporting disabled. This is one current-version run; it is not a matched
before/after measurement of the whole application.

## Method

- Parameters come from the resolved GUI-derived profile at
  `output/pdf/mushaf-cli/mushaf.settings.json`: QPC v1, declarative justification,
  `font-size-xscale`, standard shrink, spacing 1683, em scale 1, Tajweed, saved
  disabled lookups and 20 maximum XPBD iterations. Force is enabled and both
  report switches are disabled for measurement.
- All pages are shaped once and all lazy outlines materialized before timing.
  Initial shaping took 7.772 seconds in the benchmark process.
- Original and current layout sources are compiled in isolated namespaces with
  the same native Release flags (`-O3 -DNDEBUG`) and compiler. Shared geometry
  sources are unchanged relative to the historical reference.
- Each measured variant starts with fresh page inputs and a fresh geometry
  cache. The three execution orders rotate across six rounds, so each variant
  occupies each position twice. Measurements and the later CLI run execute
  serially; no task-owned build overlaps the measurements.
- Small counters measure iterations, broadphase candidates and convergence
  residual checks. These timers/counters are included in the measured times.
  Results describe this machine and profile, with normal background activity.
- Production solver sources are not modified by the benchmark. SHA-256 source
  hashes were saved and checked after the runs.

## Interpretation

Both original and current solvers run **12,080 iterations**: 20 on every page.
The increase therefore does not come from running more iterations. Current
broadphase candidates are slightly fewer: 36,152,978 versus 36,287,524.

The historical core with current geometry takes 6.775 seconds, close to the
original 6.735 seconds. Adapter/geometry changes account for little of the
observed core difference in this experiment. Both versions preserve the
intentional unsplit mark component hulls. The current adapter skips decorative
surah-header glyphs and uses corrected coordinates and offset export.

The current solver performs 32 convergence residual checks across the corpus,
costing a median **0.0107 seconds**. That measured phase accounts for little of
the remaining increase. Other changed work includes movement tracking on every
world-geometry rebuild, per-iteration reset/maximum scans, owner validation and
changed projection/broadphase ordering. The initial comparison did not isolate
their individual costs. The follow-up experiment below measures the movement
tracking contribution.

The old placement clamps, geometry snapshots, contact rollback and mutating
post-solve rounding are absent from the current version. Earlier guard-enabled
measurements in `xpbd-hull-policy-review.md` are historical and were taken in a
different run; they should not be used as a matched measurement of this removal.

## Reproduction and artifacts

From the workspace root:

```sh
python3 visualmetafont/tests/layout/benchmark-xpbd.py \
  build/mushaf-cli output/pdf/mushaf-cli/mushaf.settings.json \
  tmp/benchmarks/xpbd-current-no-guards-2026-10-05 \
  --before-ref 0421bbd --rounds 6
```

Raw timings, summary, compiler flags, source hashes, measured settings and CLI
manifest/logs are saved in
`output/benchmarks/xpbd-current-no-guards-2026-10-05/`. Isolated source copies and
the probe executable remain under the matching `tmp/benchmarks/` directory.

No solver optimization or production behavior change was applied during this
measurement.

## Follow-up: movement tracking cost

Another six alternating rounds compared three isolated copies of the current
core with the same adapter, geometry and configuration:

| Variant | Core median | Median saving within each round |
| --- | ---: | ---: |
| Unmodified current code | 7.290 s | — |
| Squared movement and squared tolerance comparison | 7.104 s | 0.202 s |
| Movement tracking, reset/scans and convergence checks removed for attribution | 7.014 s | 0.297 s |

The squared variant replaces `std::hypot(deltaX, deltaY)` with
`deltaX * deltaX + deltaY * deltaY`, stores the maximum squared movement and
compares against `tolCollision * tolCollision`. It retains the convergence
residual checks. This is an experimental implementation, not a production
change; numerical behavior for other configurations and extreme values was
not reviewed.

The attribution variant always runs the configured maximum iterations. It
removes convergence logic only in the isolated benchmark copy, and is not a
recommended solver behavior. All three variants ran 12,080 iterations with
36,152,978 candidate pairs and identical hashes over every final double-valued
`dx` and `dy`. The hash comparison is an output consistency check for this
corpus, not a general correctness guarantee.

Movement tracking plus stopping work accounts for approximately 0.28–0.30
seconds in this matched experiment. The actual residual checks themselves cost
0.0104 seconds. Using squared movement saves approximately 0.19–0.20 seconds,
which identifies the repeated movement norm calculation as a measurable cost.
This does not explain the entire original 0.531-second increase. Remaining
differences, such as geometry-bound calculations, constraint changes, memory
layout and projection/broadphase ordering, have not been individually isolated.
Different batches and timing variation prevent exact subtraction of all these
numbers into a complete cost accounting.

Offset conversion is outside the core timer. Current full-corpus offset export
took approximately 0.003 seconds in the original comparison; the division and
rounding in `applySolvedGlyphOffsets` cannot explain the 0.531-second core
increase.

The experimental runner, source, compiler metadata, output hashes and raw
timings are retained in
`output/benchmarks/xpbd-movement-attribution-2026-10-05/`; isolated compiled
copies remain in the matching `tmp/benchmarks/` directory. No production code
was modified.

## Simple optimizations applied

The shared GUI/CLI solver now:

- Tracks maximum squared correction and compares it with squared collision
  tolerance. Corrections that cancel later in an iteration are still observed;
  convergence residual checks remain active.
- Uses `updateWorldPolys` during constraint projection to skip polygon rebuilds
  when both offsets match their last built values. Base positions and local
  geometry are fixed during projection. Initial preparation always calls
  `buildWorldPolys`, including for zero offsets.
- Computes and caches local geometry bounds before translation, allowing each
  translated polygon set to reuse those bounds.
- Reuses the candidate-pair vector's capacity across iterations without changing
  candidate sorting or constraint order.
- Uses `try_emplace` for a single gap-state lookup, retaining first-contact
  initialization and multiplier updates.

The existing movement assertion was updated for squared units. No new tests
were added or run as part of applying these changes. The combined speedup was
subsequently measured at the user's request, as described below. The
pre-optimization layout sources are retained at
`tmp/benchmarks/xpbd-before-simple-optimizations-2026-10-05/` for a future matched
comparison. Mark hull policy and displacement export are unchanged.

## Matched measurement of the five optimizations

Six alternating rounds over the same 604 shaped pages compare the saved layout
sources immediately before the five optimizations with the optimized sources:

| Measurement | Immediately before optimization | Optimized | Reduction |
| --- | ---: | ---: | ---: |
| XPBD core median | 6.900 s | 5.927 s | 0.973 s (14.1%) |
| Complete placement median | 7.378 s | 6.402 s | 0.977 s (13.2%) |

The median within-round core saving is 0.982 seconds. Core ranges are
6.730–6.955 seconds before and 5.820–6.006 seconds after. These are matched
measurements in the same process and batch; comparing against medians from
earlier batches would mix implementation effects with machine timing variation.

Both cores use the same current adapter, geometry policy, compiler and Release
flags, identical GUI-derived parameters and fresh page inputs/geometry caches
for each run. Reports and PDFs are disabled, Force enabled, and lazy outlines
materialized before timing. The complete placement timer includes input
copying, geometry preparation and integer offset export. Shaping and font
initialization are excluded; initial shaping took 7.755 seconds.

All twelve measured runs process 848,298 glyphs and 435,360 marks, with 12,080
iterations, 36,152,978 candidate pairs and 32 convergence residual checks. The
improvement comes from less work at the same iteration count, not earlier
termination. Convergence residual checks cost about 0.0095 seconds in both
versions. Individual savings from the five optimizations were not separately
measured in this batch.

The probe compares every final `dx` and `dy` by their exact double bit patterns,
and every exported integer `x_offset` and `y_offset`, against the first baseline
run. All values match in every run. These comparisons happen outside the timed
phases. Source fingerprints were checked after completion. This establishes
placement equivalence for this corpus and configuration; reports and other
parameter profiles were not exercised.

A separate optimized production CLI run completed 604 pages in **18.383
seconds**, including initialization, shaping and lazy outline generation,
with PDF/report generation disabled. This single complete-run timing is not a
matched before/after comparison; the earlier single CLI measurements should
not be used to infer an application-wide speedup or regression.

Raw timings, medians, compiler metadata, exact-comparison status, source hashes,
configuration, benchmark runner and CLI manifest/logs are saved at
`output/benchmarks/xpbd-simple-optimizations-2026-10-05/`. The runnable benchmark
and isolated source copies are at the matching `tmp/benchmarks/` directory.

Reproduce from the workspace root:

```sh
python3 tmp/benchmarks/xpbd-simple-optimizations-2026-10-05/benchmark-xpbd.py \
  build/mushaf-cli \
  tmp/benchmarks/xpbd-simple-optimizations-2026-10-05/measured.settings.json \
  tmp/benchmarks/xpbd-simple-optimizations-2026-10-05 --rounds 6
```

No further production solver changes were made during measurement.
