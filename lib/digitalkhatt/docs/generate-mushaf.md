# Generate Mushaf from the command line

`digitalkhatt_generate_mushaf` generates the whole Mushaf PDF **by default**.
It follows the editor's **Generate Mushaf** action using shared corpus assembly,
line widths, justification, sajda detection, placement geometry and XPBD code.
The native executable and PDF/report libraries have no Qt dependency.

## Build

From the workspace root, with C++23, CMake, Ninja, Flex, Bison and SQLite available:

```sh
cmake -S . -B build/mushaf-cli -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DDIGITALKHATT_BUILD_GUI=OFF
cmake --build build/mushaf-cli --target digitalkhatt_generate_mushaf oldmadina_font
```

Use the corresponding font plugin target for other projects. The `.mp`,
`glyphs.mp`, feature/parameter files and compiled font plugin must be present
beside each other. Dependencies use the repository's existing CMake packages.
On macOS, configure Homebrew Flex/Bison and a C++23 compiler when the system
versions are older. The verified local native build also used
`DIGITALKHATT_BUILD_FONT_PROJECTS=OFF` and the already-built font plugin.

## Generate a PDF

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  oldmadinafont/oldmadina.mp --output mushaf.pdf
```

The default output is `mushaf.pdf` in the working directory. Force defaults to
off, as in the editor, so positions remain the justified shaping output.
Use `--force` to apply XPBD. Reports are optional and independent of Force.

Old Madina's declarative `standard` shrink policy first tries native features,
then reduces any remaining overflow using `reduce_spaces 0.95;` in
`oldmadinafont/features.fea`. Change that ratio to configure the minimum space
advance relative to its original width (`1` disables the extra reduction).
The GUI and CLI use this same recipe. XPBD receives the adjusted spacing when
Force is enabled; No Style still leaves any remaining overflow unscaled.

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  oldmadinafont/oldmadina.mp --layout qpc_v1_layout \
  --justifier decl-policy --style font-size-xscale --shrink standard \
  --line-spacing 1683 --tajweed --force --report --output mushaf.pdf
```

### Reproduce the current GUI settings

The editor's Generate Mushaf action now writes
`<font directory>/output/mushaf.settings.json`. It includes the selected font,
layout, engine, style, shrink policy, line spacing, font scale, Tajweed/Force
switches, disabled lookups and all solver parameters. Use that snapshot:

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config oldmadinafont/output/mushaf.settings.json \
  --force --report --output mushaf.pdf
```

CLI options override the snapshot. Without a snapshot the tool uses the shared
library defaults; it does not read Qt preferences. Saved GUI preferences can
therefore differ from fresh defaults. The old separate collision-adjustment
checkbox (`adjustOverlapping2`) is not part of this XPBD command; leave that
legacy pass disabled for reproducible GUI/CLI comparisons.

## Waqf height target

When a waqf is beside its stack, its soft target puts its top above the highest
same-base top mark by the existing top-mark gap (50 font units). The hard
top-order floor allows their tops to be level. When their horizontal bounds
overlap, the stronger bottom-above-top floor applies. Baseline/base visibility
limits and the upper ceiling also bound the target. Solver and reporting use
the same target calculation; there is no separate height-band parameter.

The shared defaults use `compliance.waqfTarget = 5.0` and
`compliance.waqfXAlign = 0.24`. The softer height target lets collision constraints
move a waqf down beside its own stack. The GUI target control accepts 0–10;
explicit values in saved GUI preferences and JSON snapshots override defaults.

## Waqf horizontal alignment band

Horizontal alignment prefers the leftmost ink edge among the owning base and
its attached top marks (including dots and excluding waqf signs), with no force
inside a band of 25% of the waqf's ink width on either side. Outside the band,
the existing `waqfXAlign` compliance supplies a soft restoring force. Contacts
can still move the waqf farther when needed; this is not a hard movement limit.
The unilateral multiplier resets when the mark enters the band or crosses to
the opposite side. There is no forced left relocation when a vertical stack
exceeds its ceiling. The reference follows the current base/top-stack footprint;
it does not subtract a whole waqf width to force a placement beside the stack.

Set **X band (% width)** in the GUI's **Waqf placement** row, use
`--waqf-x-alignment-band N`, or save `waqfHorizontalAlignmentBandPercent` under
`xpbd`. A value of 0 requests exact soft left-edge alignment; larger values
allow more free movement. This solver parameter is independent of the
horizontal drift reporting tolerances below. Optional soft-target reporting
measures only the excess outside the band.

## Conditional waqf left/down escape

**Waqf left/down escape** is a separate XPBD preference, enabled by default
when waqf placement and generic gap forces are enabled. It uses contacts
already sampled by the ordinary gap pass; there are no extra broadphase scans,
GJK queries, placement searches, contact sweeps or solver iterations.

The preference activates when a previous-line obstacle pushes downward on the
waqf and its estimated clearance stays below 10 font units with less than 10%
of the desired gap improvement for three iterations. An opposing lower mark
is not required. Contact estimates account for translations after sampling;
they trigger the preference but do not certify final clearance. Final collision
reporting still uses fresh geometry.

While clearance is below the trigger, compliant targets advance left by 5% of
waqf ink width and downward by 5% of its ink height per iteration. The left
target is capped at one waqf width left of its base. Descent stops at the owning
base's visibility floor (100 units above base ink and 700 above its baseline)
and keeps the waqf's top at or above every attached top mark's top. During
escape, overlapping horizontal boxes no longer impose a bottom-above-mark
floor; ordinary gap contacts protect the actual shapes. The existing final
top-order rail also enforces the visibility floor for escaping waqf signs.
Normal placement retains its 50-unit top lift preference before escape activates.

The left preference uses X-align compliance (0.24); descent uses the chosen
height-target compliance (5.0). Normal soft height/alignment targets yield to
the escape preferences, so each axis has one soft target while escaping.
Targets stop advancing once trigger clearance is reached and hold for the remainder of the solve. A preference is
inactive when its target is already satisfied. Contacts can move the glyph
farther left than its preferred target; association reporting remains active.

GUI controls: **Trigger gap**, **Step (% width)**, **Down step (% height)** and
**Max left (% width)**. JSON stores `toggles.waqfEscape`, `waqfEscapeMinGap`,
`waqfEscapeStepPercent`, `waqfEscapeDownStepPercent` and
`waqfEscapeMaxLeftPercent` under `xpbd`. CLI flags are `--waqf-escape` /
`--no-waqf-escape`, `--waqf-escape-min-gap N`, `--waqf-escape-step N`,
`--waqf-escape-down-step N`, and `--waqf-escape-max-left N`.
The trigger is a preference control, not a hard clearance guarantee. Optional
soft-target reporting includes the active left and downward escape residuals.

## Filter and order the violation report

When **Include generic gap violations** is enabled, **Only Save Collision cases**
is on by default. It uses the same shared collision detector as the GUI's
**Save Collision** action: decomposed outlines for every glyph, rounded output
positions, intentional cursive connections skipped, and cross-line checks when
at least one glyph is a mark. The clearance defaults to **10 font units** and
is controlled by **Save Collision clearance** in the GUI. That value also
controls Save Collision itself. Surah headings rendered as frames/icons remain
excluded from native placement reporting.

CLI: `--report-generic-gap --report-gap-mode collisions --collision-report-gap 10`.
`--report-gap-mode all` restores the full XPBD gap-residual report; the GUI can
uncheck **Only Save Collision cases** for the same result. JSON stores
`reportGenericGapCollisionsOnly` and `collisionReportMinGap` under `xpbd`.

Collision severity is the shortfall from this clearance, including penetration;
there is no compliant spring allowance. Small collisions are retained regardless
of the report's minimum severity, while the report limit and new/worsened filter
still apply. Collision contexts use the decomposed report outlines. This mode
does not change solver geometry, forces, gaps, iteration count or convergence;
when reports are disabled the collision audit does no work.

The GUI's **Solver Tuning > Violation report** section and the CLI use the
same settings and selection. Fresh defaults exclude GenericGap, retain up to
1,000 findings, and sort geometric findings by descending severity beyond
allowed slack. Reporting preferences do not disable their corresponding XPBD
forces. Existing saved preferences or snapshots can still enable GenericGap;
uncheck **Include generic gap violations**, or override it explicitly on the CLI.
The final placement audit is also off by default. Enable **Include placement audit
(side, class, owner)** or `--placement-audit` to collect those diagnostics;
`--no-placement-audit` overrides a saved preference.

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config oldmadinafont/output/mushaf.settings.json \
  --report --no-report-generic-gap --min-severity 10 \
  --report-limit 200 --report-sort severity --output mushaf.pdf
```

- **Minimum severity** / `--min-severity N`: filter numeric residuals by
  `max(0, severity - allowed_residual)`. Collision-proxy contacts/intersections,
  when GenericGap reporting is enabled, do not receive a compliance-slack
  discount. Discrete structural diagnostics bypass the numeric cutoff.
- **Base vicinity tolerance: marks** / `--base-vicinity-mark-tolerance N`:
  additional reporting allowance as a percentage of the mark's horizontal
  geometry width; default **5%** for marks other than dots.
- **Base vicinity tolerance: dots** / `--base-vicinity-dot-tolerance N`:
  separate allowance for dot marks, default **0%**. Dot clusters use their
  full horizontal geometry width. Both controls accept 0–100; zero restores
  reporting without this additional allowance.
  These allowances apply only to final BaseVicinity diagnostics. Solver forces,
  convergence, collision detection and other ownership diagnostics keep their
  existing rules. Findings within the allowance are omitted even when minimum
  severity is zero. Remaining findings retain their raw `severity`, record the
  allowance in `allowed_residual`, and are filtered/sorted by the excess beyond
  it. For example, a 223.22-unit-wide fatha receives 11.16 units of reporting
  slack at 5%, so a 6.47-unit BaseVicinity overrun is omitted.
- **Maximum findings** / `--report-limit N`: keep the highest-ranked N across
  the whole corpus, using the same selection in CSV, PDFs and web viewer.
  `0` means unlimited. `--summary-limit` is a legacy alias and now controls
  the whole report; older snapshot `summaryLimit` values migrate to this limit
  unless `xpbd.reportMaxFindings` is present.
- **Sort order** / `--report-sort severity|priority`: choose descending excess
  severity or review priority followed by excess severity. Critical structural
  diagnostics come first, geometric violations next, and structural review
  warnings last. The summary starts a separate section for each group.
  Ties preserve page order and the original finding order within a page.
- **Only new/worsened** / `--only-changed`: focus on changed numeric findings,
  retaining structural diagnostics. `--all-findings` restores the default.
  NEW/WORSE remains relative to the shaped-position report and its cutoff.

CSV rows and summary entries use a global order. Overview pages keep Mushaf
page order, with findings ordered within each page. Pages without retained
findings are omitted; an empty result produces one informational page.
CSV preserves the original `severity` and adds `report_severity` and
`report_rank` for the actual score and global order used in the report.
The compact PDF shows 30 word contexts per landscape A4 page. Tiny labels give
the global rank and Mushaf page, line and word (`P`, `L`, `W`). Both participants
are referenced when their words or lines differ. Ranks match the CSV and detailed
summary. Word numbers count space-delimited source tokens within each line,
including standalone ayah tokens. CSV adds `wordA_number` and `wordB_number`.

These controls are saved under `xpbd` in the GUI snapshot: `minViolationSeverity`,
`reportMaxFindings`, `reportSort` and `reportOnlyChanged`, plus the existing
`toggles.reportGenericGap`, `toggles.reportSoftResiduals` and
`toggles.reportPlacementAudit` switches.
The two BaseVicinity allowances are saved as
`baseVicinityMarkTolerancePercent` and `baseVicinityDotTolerancePercent` under
`xpbd`. Older snapshots without these fields receive the new 5% / 0% defaults.

### Waqf association reporting

`WaqfPlacement` reports two independent association risks, without changing XPBD
forces or convergence. These findings remain enabled when soft target residuals,
placement audit, or the waqf placement force are disabled:

- `horizontal-drift`: waqf ink left edge minus its owning base's ink left edge.
  Negative means left; positive means right. Reporting allowances default to
  100% of waqf width on the left and 50% on the right. Severity is the absolute
  offset beyond the allowance for that direction.
- `previous-line-intrusion`: the waqf top crosses a boundary 20% of the actual
  baseline spacing below the previous populated line's baseline. Severity is
  the amount above that boundary. The report includes the nearest previous-line
  glyph and its word. This is an association-risk heuristic, not proof of visual
  confusion. First lines have no previous-line finding. Empty surah-header rows
  are skipped.

The GUI's **Violation report** section exposes all three tolerances. JSON saves
them as `waqfLeftDriftTolerancePercent`, `waqfRightDriftTolerancePercent`, and
`waqfPreviousLineMarginPercent` under `xpbd`. CLI overrides are
`--waqf-left-drift-tolerance`, `--waqf-right-drift-tolerance`, and
`--waqf-previous-line-margin`. Horizontal percentages accept 0–1000; the baseline
margin accepts 0–100. A larger baseline margin flags more high placements.

The web detail view and CSV show horizontal offset, left/right allowances, top
height above its own baseline, distance below the previous baseline, required
margin, and nearest previous-line ink-box clearance. This clearance is an AABB
distance, not an exact polygon collision measurement. Distances use the solver's
scaled world coordinates. Offsets measure association with the owning base,
rather than movement from the initial HarfBuzz position.

Association findings rank after critical structural failures and before ordinary
constraint residuals. Within each group the selected severity/priority ordering
still applies. Initial/final comparisons match each axis to the same waqf and
owning base even if the nearest previous-line glyph changes.

The former waqf floor, ceiling, and infeasible-band findings are now named
`WaqfBoundsResidual` and omitted by default. Enable **Include waqf solver
height-bound residuals**, `toggles.reportWaqfBounds`, or `--report-waqf-bounds`
to include them. They continue participating in solver convergence regardless
of this reporting switch.

## Outputs

- `mushaf.pdf`: all corpus pages, plus the GUI-style notice page by default.
  Includes vector live MetaPost outlines, composed ayah numbers, Tajweed colors,
  surah icons/frames, sajda rules, searchable source text, bookmarks and page labels.
- `mushaf.settings.json`: resolved configuration for reproduction.
- `mushaf.run.json`: completion flag, coverage, counts, elapsed time and source,
  binary, font plugin and PDF resource fingerprints (FNV-1a, not a security hash).
- With `--report`: `mushaf_violations.pdf` (overviews for pages with retained
  findings), `mushaf_violations.csv` (globally ordered retained findings),
  `mushaf_violations_summary.pdf` (detailed word contexts), and
  `mushaf_violations_compact.pdf` (a compact visual grid).
  The GUI writes `violations_compact.pdf` beside `violations_summary.pdf`.
  The default limit of 1,000 applies to all report outputs.
  `mushaf_violations.html` provides the same retained findings in an offline web
  viewer. The GUI writes `violations.html`. Open the HTML file directly in a
  browser; no server, Qt dependency, installation or network request is needed.
  The run manifest distinguishes detected `findings`, filter-eligible
  `eligibleFindings`, and retained `reportedFindings`.

### Interactive placement review

The web viewer supports constraint, kind, change, review status, minimum excess
severity and page/line/word filters, plus search over source words, glyph names
and diagnostic details. Sort by report order, severity, review priority or
location. Critical structural diagnostics remain ahead of geometric findings
when sorting by severity or priority, matching the shared report policy.
Structural findings bypass the numeric severity cutoff.
Arabic word search ignores combining diacritics.

Click a word context to enlarge its vector geometry, inspect residuals and
assigned bases, and zoom up to 300%. The shaped-position overlay can be switched
off. Arrow keys navigate the filtered findings in the detail view. Contexts use
the same collision geometry as the PDFs, not exact filled ink.
The grid uses the viewport width with modest left and right margins and shows
images only. **Image size** provides a slider and −/+ buttons from 60% to 380%,
with **220%** as the centered default;
the grid adjusts its column count as images grow or shrink. **Grid width**
provides a separate slider and −/+ buttons from 20% to 100% of the available
page width, with **60%** as the centered default. The grid stays centered inside
the page margins. **Cases per page**
offers 24, 48, 96, 192, 384, or **All** filtered findings (default).
Hover over an image for its rank, location, constraint
and severity. Details and review controls appear when you click the image.

Mark findings **Acceptable** or **Needs fix** and add notes. Decisions are saved
in browser storage for the exact report contents, when storage is available.
Use **Export reviews** for a portable JSON copy; **Import reviews** restores
decisions for that same report. Browser review decisions do not change solver
parameters or PDF/CSV contents. Keep the HTML beside its CSV and detailed PDF
to use the corresponding links in the viewer.

A failed run leaves `complete: false` in its manifest. Verify completion before
using partial artifacts. The bundled surah frame PDF was baked from the editor's
SVG frame; it preserves its gradients without a Qt SVG renderer at runtime.
Notice/report labels use a platform font (Arial or DejaVu Sans).

## Other options

Run `--help` for the full list. Useful options include:

- `--layout qpc`, `v1`, `v2`, `v4`, or an explicit database layout table.
- `--stretch-policy NAME` / `--shrink-policy NAME` for declarative policies.
- `--disable-lookup NAME`, repeatable.
- `--pages A-B`, `--em-scale N`, `--text-width N`, `--line-spacing N`.
- `--xpbd-config PATH`: partial/full `OptParams` JSON; `--soft-targets` includes
  optional alignment and lane residuals.
- `--database PATH`, `--features PATH`, `--resources DIR`, `--pdf-resources DIR`.
- `--no-notice`; `--no-pdf` for parameter experiments with diagnostics.

The complete-corpus configuration uses the whole layout table; QPC layouts
contain 604 pages and the bundled IndoPak layout contains 610.

## Interpret the report

### Diagnostic meaning

The report compares each final finding with the same shaped page. `NEW` and
`WORSE` describe a finding relative to the configured report cutoff, rather
than proving a newly introduced physical collision. Blue dashed outlines show
shaped positions. Red denotes hard findings; amber denotes review findings.
CSV includes participating words, line/cluster indices, assigned bases,
actual exported shifts, residuals and compliance allowances.

The additional placement clamps and contact rollback have been removed. XPBD
exports its solved offsets using the normal integer conversion. Optional
placement diagnostics remain read-only. Older CLI configurations may still
contain `preserveMarkSemantics`; that retired field is ignored when loading.

`Minimum gap not reached` is a configured clearance warning, distinct from
`Collision-proxy intersection`. When enabled, the final placement audit checks
owner validity, side and mark classes independently of enabled forces. Marks intentionally
use `buildPolyFromCubics`: each component remains unsplit, and GJK's support
mapping implicitly tests its convex hull. Bases retain convex decomposition.
A proxy contact can occupy an empty concave region of the visible outline;
it is not proof of actual filled-ink intersection. Intentional pair exclusions
and manual bowl placements follow the solver policy. These checks are not a proof
of linguistic ownership or a replacement for a human Mushaf proofread.

The legacy `minGapBody`, `minGapMark` and maximum-shift fields control broadphase
candidate padding; they do not set the actual gap target or clamp displacement.
`attachStrength`, `smoothStrength` and `sepOvershoot` are not consumed by current
XPBD. These fields remain in snapshots for compatibility. The actual generic
gap target is currently 80 units, with named 40/10-unit exceptions.

Summarize and validate a completed CSV:

```sh
python3 visualmetafont/lib/digitalkhatt/tools/summarize_mushaf_report.py \
  mushaf_violations.csv --output mushaf.audit.json
```

## Experimental cached no-fit polygon contacts

XPBD still runs the placement constraints. `--contact-method nfp` replaces only
its generic-gap contact oracle with a whole-outline translational forbidden
region; `--contact-method gjk` retains the existing default component-hull
GJK/EPA path. There is no new GUI control in this prototype. Native/GUI runs
still default to GJK/EPA with the intentional mark hull policy.

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config output/pdf/mushaf-cli/mushaf.settings.json \
  --force --contact-method nfp --report --output output/pdf/nfp/mushaf.pdf
```

NFP has its own outline cache built with `buildPolyFromCubics()` for **every**
glyph, including bases. These unsplit outlines are scaled like the ordinary
solver geometry and supplied separately to NFP. Other constraints continue using
`buildGlyphCollisionGeometry()`; the Save Collision audit uses its existing
decomposed report outlines. NFP inputs are not derived from those decomposed
base polygons.

The cache interns the scaled local polygon coordinates (0.001-unit grid), with
an equality check after hashing. For a pair whose two raw outlines and ordinary
solver geometries are
single convex polygons, it calls the same floating-point GJK/EPA routine as the
default solver, with the same world vertices, pair order and requested gap.
Other pairs decompose each nonconvex silhouette once, merge the convex edge
sequences to compute `B - A` in linear time, and union all sums before querying
any contact. Convex components skip decomposition. Clearance expands that
complete union using round joins. The arc
approximation has a 0.02-unit chord tolerance. The expansion radius is exactly
the requested clearance; there is no extra outward allowance. Arc and grid
approximation can still introduce small differences on the whole-outline path.
The nearest boundary of the union gives one pair correction, including free
pockets and disconnected components. Original outline holes retain the existing
filled-hole policy; this is not an exact cubic-curve or filled-ink oracle.

Translations only change the relative query point. Outline, deformation or scale
changes register distinct shapes; clearance is part of the region key. Pair order
is canonicalized. PlacementPipeline shares its cache across the corpus; direct
`optimizePage` callers get a solve-local cache unless they supply a shared one.
`--nfp-cache-limit N` bounds pair/clearance entries with LRU eviction (default
16384); interned shapes and their decomposition are retained for the pipeline's
lifetime. Changing the limit resets that pipeline cache.

The NFP scalar is `clearance + signed_distance_to_expanded_region`. Subtracting
the clearance gives the solver's constraint residual. In a narrow concavity it
must not be interpreted as unexpanded ink penetration depth. The optional full
gap report labels ink intersection and insufficient clearance separately and
omits witness markers, since the nearest configuration-space point is not an
ink witness. Save Collision reporting remains unchanged and uses rounded output
positions plus decomposed geometry for both methods. Stacking, squeeze and kasra /
meem-iqlab cohesion keep their existing implementations and exclusions. Resolving
one pair does not guarantee that all neighboring pairs or semantic constraints
are satisfied.

The run manifest adds `placementSeconds` (including diagnostics when reporting is
on), `noFitPolygons` cache/query counters and times (including `convexGjkQueries`
for shared convex contacts; `queries` counts whole-outline queries), and movement
summaries by mark role. JSON parameters are `xpbd.useNoFitPolygons` and
`xpbd.noFitPolygonCacheLimit`. Enable `--review-word P:L:W` with `--report` to
export requested words and nearby upper-line geometry in `.review.json`, even
when the requested word has no finding.

Reproduce the full comparison, paired word viewer, reports and serialized timing
runs with a fresh cache per process:

```sh
python3 visualmetafont/lib/digitalkhatt/tools/compare_contacts.py \
  --executable build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config output/pdf/mushaf-cli/mushaf.settings.json \
  --output-dir output/pdf/nfp-comparison --pdf --timing-runs 3
```

`--pages A-B`, repeatable `--case P:L:W`, and `--timing-runs N` narrow a comparison.
`--nfp-cache-limit N` selects the cache capacity (default 16384). The comparison
passes it explicitly to both methods, overriding an older saved cache limit.
`--reuse-audits` accepts completed audits only when input/binary fingerprints and
placement settings still match the new timing runs. The comparison permits only
the contact-method setting to differ between methods. Placement audit is disabled
for both by default, overriding saved settings. Use `--placement-audit` to enable
it explicitly. Findings remain unlimited, and timings are measured without
PDF/report generation. Without `--pdf`, it still writes CSV, report PDFs and the
web viewer, but omits the two whole Mushaf PDFs. Findings are diagnostics for
visual review; their counts are not a proof of better Quran typography.

The comparison row **BaseAssociation (placement audit)** corresponds to the
existing `BaseAssociation` diagnostics from `collectPlacementViolations()`.
It is not an Ownership constraint. The audit checks owner validity and shaped-run
association, and flags a mark whose center moves over a neighbor's ink while
leaving its owner's ink. It is read-only and enabled explicitly for both methods
only when this comparison is given `--placement-audit` (off by default). Normal
GUI reporting keeps its chosen audit toggle. Bounding-box ambiguity warnings
require visual interpretation.
