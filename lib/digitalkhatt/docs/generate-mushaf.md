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

## Filter and order the violation report

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
