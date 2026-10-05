# XPBD performance comparison - 2026-10-05

**Historical experiment:** the measurements below describe the temporary
decomposed-mark version. The font author clarified that unsplit mark components
and their implicit GJK hulls are intentional. The decomposition change has been
reverted. Calling it a correctness fix was mistaken: farthest-vertex support
mapping already tests the hull. Updated measurements are in
[the restored-policy review](xpbd-hull-policy-review.md#restored-policy-performance);
these timings remain as the record of that experiment.

## Measured result

With reporting disabled, the current XPBD solver is slower than the version
before this review. Four counterbalanced full-corpus rounds produced:

| Version | Core solver median | Geometry/adapter median | Complete placement median |
| --- | ---: | ---: | ---: |
| Before review (`0421bbd`) | 7.168 s | 0.444 s | 7.613 s |
| Current, safety enabled | 11.339 s | 0.506 s | 11.848 s |
| Before core with current geometry/adapter | 9.122 s | 0.489 s | 9.608 s |
| Current, safety disabled for measurement | 9.241 s | 0.491 s | 9.734 s |

The core solver increase is **58.2%**, or **4.17 seconds** across 604 pages.
Including geometry preparation and offset export, placement takes **55.6%**
longer, adding **4.23 seconds**. Mean per-page core time increases from about
11.9 ms to 18.8 ms. The core ranges were 7.064-7.223 seconds before and
11.235-11.385 seconds after. The safety-disabled row is an attribution experiment;
it does not provide the current placement guarantees.

A separate run of the actual production CLI with `--no-report --no-pdf` completed
the same 604 pages in **20.305 seconds**, including shaping, font initialization
and placement. It produced no PDF or report files. This is a single corroborating
run of the current executable, rather than a second before/after benchmark.

## Conditions and method

- Old Madina, QPC v1, declarative justification, `font-size-xscale`, standard
  shrink, spacing 1683, em scale 1, Tajweed and the saved GUI lookup settings.
- Force enabled, 20 maximum iterations, reporting disabled. No report files,
  baseline report pass, final report collector or PDF generation is timed.
- All 604 pages were shaped once from the same configuration; all outlines
  were materialized before measurements. Shaping alone took 7.84 seconds in
  that process and is excluded from the table.
- Historical and current cores were compiled with the same native Release
  flags (`-O3 -DNDEBUG`) in separate namespaces, sharing the unchanged geometry
  library. Production solver sources were not swapped or modified.
- Each version starts with a fresh outline-geometry cache and fresh page inputs.
  Four rounds rotate the execution order of the four versions. Processes run
  serially; no task-owned build or other benchmark overlaps the timed rounds.
- Lightweight counters/timers observe iterations, candidate pairs and current
  safety phases. Timing includes this instrumentation; values describe this
  machine/profile and are not general bounds for other fonts or layouts.

The old adapter includes shaped surah-header glyphs and uses unsplit mark
polygons, as the original GUI did. The current adapter skips the decorative
headers and decomposes marks into convex parts. Therefore the first two rows
compare actual before/current placement behavior. The third row supplies the
same modern adapter and geometry to the historical core to help attribute cost.

## Cost attribution

1. **Temporary mark decomposition: approximately 1.95 seconds.** The historical
   core takes 7.168 seconds with its original geometry and 9.122 seconds with
   current geometry. Total polygon parts increase from 3,925,708 to 4,792,900
   (22.1%), despite fewer glyphs. Splitting changes coherent component-hull
   contacts into per-part contacts. The original mark policy is intentional and
   has been restored. This experiment also includes the smaller adapter-coordinate
   and header changes.
2. **Safety guards: approximately 2.10 seconds.** The current core takes 11.339
   seconds with safety and 9.241 seconds with safety disabled. Observed phases
   with safety enabled are:
   - Building safety bounds and copying shaped geometry: 0.193 seconds.
   - Per-iteration semantic projection: 1.389 seconds.
   - Final rounding, contact checks and restoration: 0.557 seconds. The
     safety-disabled version still performs rounding, costing about 0.208 seconds.
3. **Convergence checks: approximately 0.011 seconds.** Both actual versions
   use 12,080 iterations in total: 20 on every page. The stricter stopping rule
   therefore does not increase iteration counts for this corpus. It performs
   34 additional hard-residual checks in the current version; these are internal
   stopping checks even with user-facing reports disabled.

Candidate pairs also remain similar: 36,287,544 before and 36,153,794 after.
The slowdown comes primarily from the cost per iteration and safety work.
Phase medians and differences are approximate attributions, not an exact
additive accounting; different geometry and guards also change contact paths.

## Improvements to pursue

1. **Avoid unchanged safety-projection rebuilds.** `projectPlacementSafety`
   currently calls `buildWorldPolys` for every guarded mark on every iteration,
   including when clamping leaves its displacement unchanged. Rebuild only when
   the displacement differs from the last built value. This targets part of the
   measured 1.389-second projection cost while preserving the bounds. Benchmark
   output equality and run placement regressions before adopting it.
2. **Reuse transformed geometry storage.** `buildWorldPolys` allocates and copies
   translated polygon sets repeatedly. Reuse polygon/AABB buffers or query
   immutable local parts with translation offsets to reduce cost in both gap
   solving and safety checks. The latter is a larger geometry API change.
3. **Accelerate collision-proxy contact queries.** Keep unsplit mark components
   and decomposed bases, and cache candidate pairs or add a per-base part index. `getDistance`
   currently walks pairs of parts for every contact query. Validate equivalent
   contact results on the full corpus, including the nine existing intersections
   and the 33 rejected proposals, before changing this path.
4. **Keep solver and reporting switches separate.** Disabling reports correctly
   removes report generation, but the guards protect exported placements and
   remain active. Their measured overhead should be optimized rather than made
   dependent on whether the user asks for a report.

## Reproduction and artifacts

From the workspace root:

```sh
python3 visualmetafont/tests/layout/benchmark-xpbd.py \
  build/mushaf-cli output/pdf/xpbd-corpus/final/mushaf.settings.json \
  tmp/benchmarks/xpbd-comparison --before-ref 0421bbd --rounds 4
```

Raw CSV, median summary and compiler/reference metadata are retained under
`output/benchmarks/xpbd-2026-10-05/`. The benchmark adapter and isolated source
copies are under `tmp/benchmarks/xpbd-2026-10-05/`. The runner and probe are in
`visualmetafont/tests/layout/benchmark-xpbd.py` and `benchmark-xpbd.cpp`.

No production solver behavior was changed during this performance comparison.
