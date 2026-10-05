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

## Outputs

- `mushaf.pdf`: all corpus pages, plus the GUI-style notice page by default.
  Includes vector live MetaPost outlines, composed ayah numbers, Tajweed colors,
  surah icons/frames, sajda rules, searchable source text, bookmarks and page labels.
- `mushaf.settings.json`: resolved configuration for reproduction.
- `mushaf.run.json`: completion flag, coverage, counts, elapsed time and source,
  binary, font plugin and PDF resource fingerprints (FNV-1a, not a security hash).
- With `--report`: `mushaf_violations.pdf` (one overview per corpus page),
  `mushaf_violations.csv` (every finding), and
  `mushaf_violations_summary.pdf` (highest-priority findings, capped at 1,000
  by default; increase `--summary-limit` to include more rows).

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
`Collision-proxy intersection`. The final audit also checks owner validity,
side and mark classes independently of enabled forces. Marks intentionally
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
