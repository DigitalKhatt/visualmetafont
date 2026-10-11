# Compare final binary layouts

Use `compare_layout_binaries.py` to review solver, feature, font, and justification
changes from two **DKLY v4 Whole glyphs** exports. No font, Qt, server, or Python
packages are needed to compare saved exports. Outlines are read directly;
interpolated v3 layouts are not accepted.

```sh
python3 visualmetafont/lib/digitalkhatt/tools/compare_layout_binaries.py \
  --before before.bin --after after.bin \
  --before-repeat before-repeat.bin --after-repeat after-repeat.bin \
  --output-dir output/pdf/layout-comparison
```

Each repeat must be an independent export of the same version and settings.
Binary bytes and text sidecar bytes must match exactly within each version.
`--skip-repeatability-check` explicitly permits comparing two existing files
without repeats; the viewer identifies that comparison as unverified.

## Generate inputs

In the GUI, use **Generate Layout Info**, choosing **Whole glyphs** or **Both**.
Keep copies of `<font>Layout.variants.bin` and its adjacent `.bin.json` sidecar
before and after a change. The sidecar supplies source text, glyph names, and
line baselines. Rename it with the binary: `before.bin` needs `before.bin.json`.
Existing v4 exports without sidecars are supported, with review cases grouped
by line rather than word. `--line-spacing` sets their assumed baseline spacing.

The native CLI uses the same Qt-free v4 writer as the GUI:

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config settings.json -o output/pdf/mushaf.pdf \
  --layout-binary output/pdf/mushaf.bin
```

This writes the PDF, final binary, and `mushaf.bin.json`. Use `--no-pdf --no-report`
for a lighter independent repeat. Binary export is optional and must start at
page 1. It preserves the GUI's float32 outlines and fixed16.16 line metrics.

## Review

The self-contained `comparison.html` includes before/after cubic curves, search,
image/grid controls, movement sorting, and click-to-enlarge details. It preserves
every exact difference in final ink. The initial filter hides positional changes
below 30 font units; outline, scale, color and glyph-sequence changes remain
visible. Select **All exact differences** to show smaller movements too.

Comparison ignores variant IDs and contour/glyph enumeration order when final
positioned curves and colors are identical. It matches cases by source page,
line and word when sidecars exist, and supports changed glyph sequences. Cubic
control-point differences are exact numeric differences, not a pixel comparison
or a proof that two differently parameterized curves bound different ink.

`changes.csv`, `changes.json`, and `summary.json` preserve movements, change
types, source hashes, and repeatability evidence. The tool reads one page's
positioned outlines at a time; the HTML includes only changed cases and context.

The binary contains final placements and ink, not XPBD residuals or original
HarfBuzz positions. Use the existing placement/violation exports for explaining
solver constraints. Surah headings use a separate font, absent from v4 outlines,
so this comparison covers body lines and basmalas rather than heading outlines.
