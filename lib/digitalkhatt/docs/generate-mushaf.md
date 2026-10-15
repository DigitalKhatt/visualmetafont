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

Old Madina's declarative `standard` shrink policy tries features in order,
accepting the first undershoot and stopping further shrink trials. Final
`balance;` in `oldmadinafont/features.fea` distributes the created gap equally
among shaped spaces. The GUI and CLI use this same recipe. XPBD receives the
adjusted spacing when Force is enabled; No Style still leaves any remaining
overflow unscaled.

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  oldmadinafont/oldmadina.mp --layout qpc_v1_layout \
  --justifier decl-policy --style font-size-xscale --shrink standard \
  --line-spacing 1683 --tajweed --force --report --output mushaf.pdf
```

### Reproduce the current GUI settings

The editor's Generate Mushaf action now writes
`<font directory>/output/mushaf.settings.json`. It includes the selected font,
layout, engine, style, shrink policy, page geometry, paper size, line spacing, font scale, Tajweed/Force
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

### Page layout

Open **Settings → Page layout…** (also on the justification toolbar). Page width,
height, left/right margins, top/first baseline, bottom margin and line spacing
are in font units. The top value is the first baseline's distance from the page
top, not a limit on mark ink. The calculated text width is page width minus the
left and right margins. PDF paper width and height are in mm and linked to the
page geometry's aspect ratio. Changing either paper dimension updates the
other; changing page geometry retains paper width and recalculates height.
The displayed dimensions use two decimals; the stored dimensions retain full
precision so display rounding does not change the ratio.
**OK** saves the values across launches and refreshes the preview; **Cancel**
does not save, and **Restore defaults** restores the original geometry values.
The existing saved line-spacing preference is retained.

**Center lines horizontally** selects `Center` instead of `Distribute` when
creating line inputs, before calling `justifyPage`. It uses the existing centering
calculation and keeps the selected justification policy. It is off by default
and is saved across launches and in Generate Mushaf settings snapshots
(`pageLayout.centerLines`), so GUI and command-line PDF generation use the same
alignment. **Restore defaults** also resets this option.

Page height and bottom margin are linked for a standard 15-line page:
`pageHeight = topSpace + 14 * lineSpacing + bottomMargin`. Editing height updates
the bottom margin; editing the bottom margin updates height (and hence PDF
paper height). Changing the first baseline or line spacing retains page height
and recalculates the remaining bottom margin. If the final baseline would fall
below the page, the dialog raises page height to that baseline with zero bottom
margin. Saved/JSON geometry uses page height to derive bottom margin. For CLI
overrides, `--bottom-margin` determines height using the final top/spacing values;
if both `--page-height` and `--bottom-margin` are supplied, the last one chooses
which value determines the other.

The corpus keeps its prescribed lines per page. The dialog shows the last
baseline of a 15-line page and requires it to fit above the bottom margin;
change spacing or page height to make it fit. PDF generation also checks actual
baseline positions, including the first two pages' special spacing. The bottom
margin bounds baselines, not ink: marks can extend below the final baseline.

Generate Mushaf, layout export, collision review and minimum-size reports use
the configured text width and baseline positions. Both GUI and native Mushaf
PDF writers use the same aspect ratio as the logical page, preserving glyph
proportions without extra borders from mismatched paper geometry. The preview
frame reflects the configured page width and height, with only left/right
margin guides spanning the full page height. Layout geometry is reapplied when
the preview scene is recreated. The preview and PDF use the same horizontal
margins (the old PDF-only 400-unit margin is replaced by the configured margin).
Changing paper size alone changes printed scale, not shaping or XPBD placement.

The GUI snapshot includes `pageLayout`. It is authoritative over the legacy
flat `textWidth`, `lineSpacing`, `pageWidthMM` and `pageHeightMM` snapshot values.
CLI arguments override it: `--page-width`, `--page-height`, `--left-margin`,
`--right-margin`, `--top-space`, `--bottom-margin`, `--line-spacing`,
`--paper-width-mm`, and `--paper-height-mm`. `--text-width` changes page width
while retaining the margins. Old snapshots without `pageLayout` retain the
legacy PDF sizing and margin behavior.

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

## GUI minimum-size overflow report

Choose **No style** in page fitting, then **Calculate minimum size**, to inspect
overflow remaining after justification. Shaping and justification both use a
scale-1 HarfBuzz font. The diagnostic multiplies the final integer advance sum
by `line.fontSize * line.xscale` and subtracts the page-space target after fitting.
It requires no extra justifier measurements or accepted-width field.

The main `output/overflows_with_just.csv` (or `overflows.csv` without justification)
keeps its five columns: selected font size ratio, page, line, fractional overflow
in unscaled font units, and additional fit ratio. Overflow is no longer rounded
up to a whole unit. The fit ratio uses the measured target and width in the same
coordinate system, including when the selected font size is 0.95.

Every positive overflow is included. Applying a font size such as 0.95 does
not round each advance a second time. Fractional space filling carries its
remainder across spaces and rounds down so it cannot exceed its budget.
Only floating-point arithmetic noise around zero is discarded. There is one
CSV; the status bar shows the number of overflow lines. With X-scale fitting,
a line that fits after its transform is not reported as overflowing. Select
**No style** to inspect overflow before horizontal fitting.

## Line coordinates and exported layouts

All page-justification engines shape at scale 1. Glyph outlines, advances and
attachment offsets remain in font units. `currentLineWidth` is their unscaled
advance sum; `desiredLineWidth`, the line origin and the baseline are page units.
`line.fontSize` and `line.xscale` are applied when placing the complete line:

```text
horizontal scale = fontSize * xscale
vertical scale   = fontSize
```

The origin and baseline are translated before this transform, so changing font
size does not move page margins or scale line spacing. Surah titles retain their
separate icon treatment. GUI, PDF, collision reports and Tarteel use this rule.
XPBD builds geometry in page coordinates, then divides solved X/Y corrections
by the corresponding line scales before storing font-unit integer offsets.

The JSON and DKLY exporters write these native coordinates directly. Regenerate
layouts after this coordinate change; old scaled-coordinate exports are not a
compatibility target. The binary fields and DKLY version numbers are unchanged.

## Conditional waqf left/down escape

**Waqf left/down escape** is a separate XPBD preference, enabled by default
when waqf placement and generic gap forces are enabled. It uses contacts
already sampled by the ordinary gap pass; there are no extra broadphase scans,
GJK queries, placement searches, contact sweeps or solver iterations.

The preference activates when a previous-line obstacle remains too close to the
waqf and its estimated clearance stays below 10 font units with less than 10%
of the desired gap improvement for three iterations. If that tight upper contact
opposes a still-crowded contact with an ordinary top mark attached to the same
base (contact normals more than 120 degrees apart), escape activates immediately.
This recognizes a squeeze before normal alignment pulls the waqf back and the
contacts select a rightward exit. An opposing lower mark is not required for the
three-iteration fallback. The obstacle's owning line identifies it as an upper obstacle;
an alternating separation normal must not reset a persistent squeeze.
Contact estimates account for translations after sampling;
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

GUI controls: **Trigger clearance**, **Left step**, **Down step** and
**Maximum left target offset** in Placement / Waqf placement. JSON stores `toggles.waqfEscape`, `waqfEscapeMinGap`,
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

`sameWordGap` and `differentWordGap` tune the generic XPBD target clearance in
solver font/world units. Both default to 80. Different-word clearance also
applies between lines. Word identity is assigned once per page from glyph order,
spaces (including space variants), and line boundaries; moving a mark never
changes its word. The 40-unit own-alef attachment and 10-unit kasra/iqlab pair
exceptions retain their gaps. Dedicated stack constraints retain their authored
gaps. Base glyphs remain fixed, so a larger value requests more clearance from
movable marks; compliance and conflicting constraints can leave a residual.
Raising a target above 80 retains ordinary contact stiffness; the smaller
attachment gaps keep their historical stronger contacts.
Zero requests no extra clearance while retaining intersection resolution.

GUI: **Placement / Collision avoidance / Resolve gaps and collisions**, then
**Same-word gap** and **Different-word gap**. For example, keep the former at 80
and set the latter to 120 to request more spacing between words.

```sh
digitalkhatt_generate_mushaf --config mushaf.settings.json --force \
  --same-word-gap 80 --different-word-gap 120 -o mushaf.pdf
```

The corresponding JSON values are stored under `xpbd`. Solver-proxy gap reports
use the same pair-specific targets. Save Collision clearance remains a separate
reporting setting.

`broadphasePadding` sets the minimum expansion of each glyph's bounding box for
candidate search, defaulting to 140 font units. Effective expansion is at least
half the largest target gap (including the fixed 40-unit exception), so raising
the different-word gap cannot put relevant pairs outside the search horizon.

The GUI Solver Tuning panel has three tabs:

- **Placement:** iteration limit and convergence tolerance, then grouped
  controls for collisions, base association, stacking/harakat, waqf, and special
  placements. Each constraint's enable switch is beside its parameters.
- **Reports:** collection, finding selection, diagnostic types, and allowances.
  These settings affect reporting, not placement forces. Collection and the
  placement audit remain off by default.
- **Advanced:** collision-search padding. Search padding does not cap movement.

Compliance controls explain that zero is rigid and larger values are softer.
Horizontal exposure fractions are displayed as percentages but remain fractions
in snapshots. Waqf escape controls are unavailable while waqf placement or
collision avoidance is disabled. Tabs and selected sections are remembered.

Older GUI settings and CLI snapshots migrate the five legacy gap/shift fields
using their original formula:

```text
broadphasePadding = max(minGapBody, minGapMark)
                 + 2 * max(maxShiftBodyX, maxShiftBodyY, maxShiftMark)
```

An explicit `broadphasePadding` takes precedence over legacy fields in the same
settings file. CLI partial legacy overrides carry forward the other values loaded earlier in the
same invocation; omitted values initially use the former defaults. New exports
contain only `broadphasePadding`. The inactive `attachStrength`, `smoothStrength`,
`sepOvershoot`, and return-to-anchor toggle/compliance have been removed.
Loading older CLI snapshots ignores those retired fields; other unknown fields
still produce configuration errors.

Summarize and validate a completed CSV:

```sh
python3 visualmetafont/lib/digitalkhatt/tools/summarize_mushaf_report.py \
  mushaf_violations.csv --output mushaf.audit.json
```
