# Compare complete Mushaf placements

For feature/font changes or comparing saved GUI exports, use
[`compare_layout_binaries.py`](compare_layout_binaries.md). It compares final
DKLY v4 outlines and placements and accepts changed shaped inputs. The strict
placement tool below remains useful for investigating solver deltas and reports.

`compare_placements.py` runs two native `digitalkhatt_generate_mushaf` builds
with one font snapshot and one settings JSON, then creates `comparison.html`,
full before/after Mushafs, violation reports, a movement CSV, and run metadata.
Python's standard library is sufficient. The HTML is self-contained and works
offline without a server.

```sh
python3 visualmetafont/lib/digitalkhatt/tools/compare_placements.py \
  --before-executable /absolute/baseline/digitalkhatt_generate_mushaf \
  --after-executable /absolute/current/digitalkhatt_generate_mushaf \
  --config /absolute/settings.json \
  --output-dir /absolute/output/pdf/placement-comparison
```

Both executables must support the diagnostic options below. Copy the baseline
executable **and its layout library**, and make its dynamic-library reference
resolve to that copy before rebuilding the current implementation. Use a fixed
copy of the font project, its feature/glyph/parameter files, and its font library.
A copied executable that still loads the current layout library is not a baseline.

The comparison checks identical glyph identity, source cluster, owner, initial
positions, advances, scales, and all shape parameters on every page. It includes
**every word with a changed final integer renderer offset**, regardless of whether
there is a constraint finding. Solver delta differences that round to the same
renderer offsets are counted separately, not shown as visual changes. Surah
headers are drawn by the PDF writer and do not participate in mark placement.

The preview pass exports the same flattened/decomposed ink used by Save Collision,
after renderer rounding. Before/after images share their bounds. Gray ink provides
nearby context; click a word to see movements, neighboring lines, and page links.
Cases default to All. Search, movement sorting, grid width, and image size are
available. The grid's middle width is 60% of the screen.

Reporting uses Save Collision clearance, placement audit off, minimum finding
severity 1, unlimited findings, and no notice page. These are report/display
settings: solver/layout parameters come from the supplied JSON unchanged. The
Mushaf links therefore use `#page=P` directly. Timing includes reporting, preview
geometry and placement preparation; it is not an isolated solver benchmark.

`--reuse` rebuilds the viewer from completed outputs without regenerating PDFs.
`--reuse-before` preserves the baseline Mushaf and regenerates the changed run
and both preview selections. Use these only when the font/configuration and
baseline binary are unchanged. Every run validates its input source checksums.

## Native diagnostic exports

- `--placement-data PATH` streams one compact JSON record per page. Each record
  includes all solver glyphs, their shaped input and final rounded renderer offsets.
  Memory is bounded to one page; the export is disabled by default.
- `--review-words-file PATH` reads a JSON array of `[page,line,word]` triplets
  (all one-based) and writes `<output-stem>.review.json`. It requires
  `--report --report-generic-gap --report-gap-mode collisions` to capture actual
  Save Collision ink geometry rather than the solver's mark hulls.

The settings sidecar describes layout/solver options; these optional export paths
are command-line diagnostics and are not GUI solver settings.

## Top-mark semantic matching

`TopMarkStack.cpp` keeps the authored `topStackTable()` offsets and gaps. A row
applies only when its `belowNames` and `aboveNames` have consecutive original
indices in the full shaped glyph sequence on the same line and are attached to
the same base. Any intervening glyph blocks the relationship, even if it is
outside `topmarks` or has no table rule. No special hamza membership or branch
check is needed. Row order does not select the parent; it only preserves the
existing order of constraint projections.

Neither original nor solved coordinates determine parent selection. Other
marks block a relationship through semantic adjacency, never through their
initial heights. Generic gap contacts remain enabled for
all pairs except the direct stack pairs with their own authored gap.

## Repeatability and review thresholds

Each build is now run independently twice. The preview pass also exports its
placements; these must be **byte-identical** to the main pass, including raw
solver deltas. If no words changed, a placement-only repeat replaces the preview
pass. A mismatch stops comparison generation rather than hiding nondeterminism
behind a movement tolerance. The same executable, its paired layout library,
font snapshot, settings, and platform must be used for the repeat.

`--reuse` rechecks the saved repeatability proof and its placement exports.
Existing independent exports can be supplied with `--before-repeat-data PATH`
and `--after-repeat-data PATH`. `--skip-repeatability-check` is an explicit opt-out;
the HTML then identifies the comparison as unverified.

The HTML retains every exact difference but initially shows word movements of
at least **30 font units** (about 0.45 points at the current Mushaf scale). This is
a review threshold, not a solver tolerance. "All exact differences" sets it to
zero. New report findings and severity increases of more than one unit remain
visible by default, regardless of displacement. Uncheck that option to inspect
only movements above the chosen threshold. Report selection still uses minimum
severity 1; the viewer does not claim absence of smaller residuals.

For the semantic-stack investigation, `stack-attribution.json` records removed
and added table links and identifies neighboring effects. It is imported only
when its before/after placement hashes match. This optional diagnostic enables
rule-family filtering and the changed-link details. It is specific to the
compared implementations and must be regenerated for a new change; it is not
inferred from which glyph happens to move.

For release review, preserve the exact baseline inputs and build alongside its
approved output, require a zero-difference repeat check, then inspect larger
movements and new findings first. Keep the complete numeric export for examining
smaller movements. This tool currently requires matching shaped inputs: font,
shaping, or justification changes need a comparison that also matches and reports
changed glyph sequences and outlines rather than only placement offsets.
