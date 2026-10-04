# Compare MetaPost and Tarteel layout rendering

`digitalkhatt_compare_layout_rendering` is a standalone, headless executable.
It replays the placements in a Generate Layout Info binary and compares live
MetaPost outlines with Tarteel's precomputed-master interpolation. Force,
justification and Tajweed assignments come from the existing binary; no new
justification pass or GUI is needed.

Build from the **digitalkhatt workspace root**, using the existing macOS preset:

```sh
cmake --build --preset release-macos --target digitalkhatt_compare_layout_rendering
build/vscode-nmc/visualmetafont/tools/Release/digitalkhatt_compare_layout_rendering --help
```

Other configured native builds can build the same target with
`cmake --build BUILD_DIR --config Release --target digitalkhatt_compare_layout_rendering`.
The tool links the shared `digitalkhatt::otlayout` library and Qt Core/Gui/Svg.

## Reproduce the first Basmala's broken join

```sh
build/vscode-nmc/visualmetafont/tools/Release/digitalkhatt_compare_layout_rendering \
  --project oldmadinafont/oldmadina.mp \
  --binary /Users/amin/projects/tarteel/mobile-app/assets/data/oldmadinaLayout.bin \
  --glyph-map oldmadinafont/output/oldmadina.json \
  --page 1 --line 2 --glyphs 23:30 \
  --gap 25:28 --samples 1024 \
  --metapost-scale 1.02 \
  --output build/layout-rendering-comparison/basmala
```

Open the output's `index.html` to switch images, inspect the overlay, and read
measurements. Pages and lines are **one-based**; glyph indices are **zero-based
positions in the original line**, including marks and spaces. `--glyphs` crops
the selection without resetting the accumulated advances. `--gap` can repeat.
Omit `--glyphs` to compare the whole line.

`--metapost-scale` defaults to the binary line's **fontSize** (1 for v1/v2). In the PDF used for this investigation,
Basmala uses `1020 Tf` with a `0.001` Type3 font matrix, i.e. outline scale **1.02**.
Both GUI and PDF apply `line.fontSize` to each outline about the glyph origin.
Binary v3 serializes that field after each line's `xscale`, and Tarteel
applies it to each glyph outline after translating to its origin. Legacy v1/v2
files have no font size and retain scale 1. Supply the actual scale of the PDF/GUI line under investigation; do
not assume every line uses 1.02. The tool also renders MetaPost at scale 1 to
separate this effect from interpolation.

With the legacy v2 assets examined on 2026-09-25, the **meem/noon join** (indices 25/28)
has these approximate minimum clearances:

| Geometry | Clearance in line units |
| --- | ---: |
| Live MetaPost, outline scale 1.02 | 0 (filled paths overlap) |
| Live MetaPost, outline scale 1 | 31.58 |
| Tarteel interpolation, outline scale 1 | 30.81 |

This is the break directly **below** the small alif, between two base glyphs.
Measuring from the small alif itself to either base glyph measures a different
space. The missing outline scale in v2 reproduces this join failure; interpolation
slightly reduces its clearance in this example.

## Outputs and interpretation

- `metapost.png` / `.svg`: live outlines evaluated at the binary's clamped axes,
  with `--metapost-scale` applied.
- `tarteel.png` / `.svg`: binary outlines interpolated using the equivalent of
  `JsiPage::getGlyphPath`, including clamping, independent left/right/third
  deltas, fixed-point-to-float decoding and the final SkScalar conversion, then
  the line's outline font size (v3).
- `metapost-unit.png` / `.svg`: live outlines at scale 1 with identical origins.
- `overlay.png` / `.svg`: MetaPost in translucent magenta, Tarteel in cyan.
- `difference.png`: monochrome coverage difference, red for more MetaPost ink,
  blue for more Tarteel ink. Ordinary images use stored Tajweed colors unless
  `--monochrome` is supplied.
- `report.json`: input paths and SHA-256 hashes, placement indices, glyph names,
  offsets, origins, requested/clamped axes, default-outline drift warnings,
  curve error, pixel error and optional pair clearances.
- `index.html`: local viewer with the report embedded; no server required.

All views share a crop, scale and rasterizer. Line `xscale` is applied to glyph
positions and outlines. The global page translation/baseline is omitted from
both images, since it cannot affect within-line joins. Use
`--pixels-per-unit` for the target display scale (Tarteel's page scale is
`pageWidth * 0.97 / 16400`) or magnification. Inspect SVGs for unlimited zoom.

`outlineError` compares unpositioned outlines. `positionedError` includes the
chosen MetaPost scale and line xscale. RMS is the square root of the mean
squared **Euclidean distance** between corresponding points at uniformly
spaced Bézier parameters `t`, including the endpoints. It is not weighted by
arc length. A cubic starts at the previous endpoint, or the contour's move
point. Incompatible topology is reported explicitly and has no RMS value.

Clearance is the minimum distance between filled glyphs. Intersecting or
contained filled paths have zero clearance. Otherwise curves are approximated
by `--samples` segments per cubic; the report includes the closest endpoints.
These are approximate contour distances, not vertical gaps or pixel counts.
Raise `--samples` to check convergence. The reported curve maximum is sampled;
`maximumCoordinateDelta` separately measures coordinates/control points.

## Input checks and limits

The binary reader supports legacy v1 and `DKLY` v2/v3, including the third axis
and v3 line font size. Older binaries default to font size 1. Regenerate the
layout with Generate Layout Info to obtain the actual per-line values; changing
the format version alone cannot recover them.
Truncation, unknown versions/masks, invalid references and trailing bytes fail
explicitly. Since the binary contains no glyph names, the corresponding JSON
export supplies names. Its selected default paths must match the binary; a
stale or unrelated map is rejected. Live font defaults are checked separately
and differences appear as warnings so changed font sources can be investigated.
The JSON defaults to `PROJECT_DIR/output/PROJECT_NAME.json`.

The live path comes from `GlyphVis::getAlternate` in the shared layout library.
The binary parser and interpolation are intentionally local equivalents of
Tarteel's `MushafRendererJSI.cpp`, `binary_reader.h` and `jsipage.cpp`; review
this mirror when those implementations change. This tool does **not** link
React Native/JSI or execute Skia. Both branches use Qt's nonzero-fill rasterizer,
which isolates geometry differences but cannot establish device rasterizer
parity. Internal MetaPost fill colors/non-fill graphics and surah-title lines
(the app's external icomoon font) are rejected rather than silently approximated.

Run the analytic measurement checks with:

```sh
build/vscode-nmc/visualmetafont/tools/Release/digitalkhatt_compare_layout_rendering --self-test
```

With `BUILD_TESTING`, the same checks are registered as
`digitalkhatt.layout_rendering_measurements` in CTest. They cover Bézier start
points, RMS against known distances, three-axis interpolation, clamping, outline scaling about the glyph origin,
disjoint clearance, overlap and containment.
