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
For changes affecting thousands of words, `--preview-limit 120` keeps the largest
movements and a deterministic spread of changed mark combinations in the ink
viewer. All words are still compared numerically and included in `changes.csv`;
the viewer explicitly labels its sampled preview coverage. The default remains
all changed words. This avoids multi-gigabyte contextual preview files.

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

## Lower dots and kasra

The experiment is opt-in for `behshape.init` and all `behshape.init.*` variants.
`xpbd.toggles.rehWawBehPlacement` defaults to `false`: absent/false settings
preserve the original solver and authored anchored-dot exceptions. Isolated,
medial and unrelated glyphs remain outside this scope. CLI overrides are
`--reh-waw-beh-placement` and `--no-reh-waw-beh-placement`.

The GUI controls are under Placement / Special placements / Reh / Waw followed
by initial Beh. Gaps, clearance floors, overhang percentage/cap and kasra horizontal
compliance persist in QSettings and export in the `xpbd.rehWaw` and
`xpbd.compliance` snapshot objects. See the [settings table](../docs/generate-mushaf.md)
for names and defaults.

A clear balanced gap at the current tip position has priority. The dot may
straddle the tip's horizontal position, but its center must remain at or right
of the tip; a left-shifted mark must use the separate left-slot rules. Actual
outline clearances decide
whether it fits, independently of the full-width left-slot restriction. The
whole same-owner dot/kasra pair must fit. If this gap is blocked, the solver
tries a full-width left-tip slot, followed by a small outline-checked
overhang. Only when all those searches fail do the independent single-dot and
double-dot checkboxes apply: checked permits below-tip placement (the default),
unchecked leaves ordinary collision handling active without forcing descent.
Standalone kasra/kasratan/kasratanidgham on initial Beh variants uses the same
ordered search, independently of those dot-specific checkboxes. Centered
standalone marks get zero mobility; left/below marks retain ordinary mobility
and their selected slot's directional constraints. A dot-linked kasra keeps
following its dot. A neighboring Reh/Waw's own kasra is unaffected.
The same-word/same-line preceding Reh/Waw check also gates stack registration
and generic-gap exclusions. Other initial Beh forms keep the ordinary solver
constraints they have with the option disabled, including inside a following
Ain/Hah bowl; they receive no contextual below-bowl bounds.
Standalone candidates satisfy the enabled below-mark rail before pinning.
After initial positioning, ordinary XPBD constraints solve together. There is
no final ink-floor projection; centered marks stay fixed through mobility zero,
while movable neighbors use ordinary collision constraints. `compactGap` checks
all surrounding ink during current-gap acceptance, including neighboring dots
and kasra. Other ink must also meet `neighborGap`, using the larger minimum.
Inside a feasible current gap, `rehWaw.preferredGap` (default 80) controls the
desired breathing room to Behshape and Reh/Waw. The search prioritizes the
smaller of those clearances up to this target, then balance and movement cost.
The target is separate from the minimum `compactGap`: failure to reach it
keeps the roomiest safe in-gap position instead of triggering another slot.

Positioning runs once before XPBD, from unchanged input geometry. Accepted
current-gap dots are flagged and get zero mobility. A left dot's vertical
center aligns with the leftmost tip point, and `rehWaw.leftGap` (default 30)
is its minimum horizontal clearance. Below fallback starts below the whole
Reh/Waw outline. It prioritizes the mark center remaining under its own base.
`compactGap` is used only for current-gap acceptance and balancing. Once that
gap fails, left placement uses `leftGap`, below placement uses `tipGap`, and
owning-Behshape ink uses `neighborGap`. Linked kasra inherits its dot's slot
for all checks. XPBD constraints do not read `compactGap`.
The search rejects
dots or linked kasra below a different base whose descender
extends past the normal near-reference slot.
If no complete local slot fits, ordinary compliant constraints adjust the
authored input instead of accepting a distant escape underneath that base.
Initial below-slot candidates use `belowMarkGap` (default 10) for same-line
neighboring marks. Below dots prefer the owner center horizontally and the reference
bottom vertically using an ordinary stack, plus the enabled base-vicinity
leash. Neighboring-mark spacing uses `compliance.rehWawBelowMarkGap` (1.0),
while bottom preference uses `compliance.rehWawBelowBottom` (0.2). The separate
initial clearance is not enforced by a final projection after bounds. Base ink
and next-line contacts remain unchanged.
Left and below dots remain movable during XPBD. Proposed owner groups are
validated together before application.

Same-owner lower dots and kasra/kasratan form an ordinary downward stack,
preserving their adjusted initial horizontal offset. Horizontal compliance
is `compliance.rehWawKasraFollow` (default 0.2); the vertical minimum uses
`compliance.stackOrderGap`. The stack follows the existing `stackOrder` switch.
There is no custom dot point attraction or repeated contextual search during
XPBD. The legacy `compliance.rehWawDotTarget` key is ignored. Extra vertical
separation is allowed, with complete-pair feasibility checked before XPBD.

Compact contact targets apply only to the reference Reh/Waw and owning
Behshape. Other ink retains ordinary gap targets. Fixed authored bowl marks
are obstacles for contextual lower marks and remain fixed. All positioning
still requires Force. Placement exports include `initialPositioning`,
`initialPlacement` and `mobility` to distinguish the initial pass from XPBD.

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
