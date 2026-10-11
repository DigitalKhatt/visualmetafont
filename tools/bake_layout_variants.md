# Whole-glyph layout export and standalone conversion

## GUI export (direct GlyphVis outlines)

Use **Generate Layout Info** and select **Whole glyphs (no app interpolation)**
or **Both formats** (the initial selection). The choice is saved between sessions.
The GUI writes `<font>Layout.variants.bin` (DKLY v4) and, when selected,
`<font>Layout.bin` (DKLY v3) in the font project's `output/` directory.
Keep the intended justification parameters, Force, Tajweed and line spacing.

V4 generation is implemented in the GUI's `generateLayoutBinary.cpp`. For each
distinct body placement's glyph code and complete parameter vector, it obtains
`OtLayout::getGlyph(placement)` and writes the resulting `GlyphVis::mpPath()`
contours directly. Values are converted from MetaPost coordinates to float32
without intermediate fixed-point rounding. Additional axes are included in
variant identity too. No interpolation or Python is involved.

V3 and v4 share the placement/line-metadata writer. Surah headings use ID zero
in v4 and the app's separate heading font; body placements use nonzero IDs.
The native loader accepts both formats. Copy the selected v4 file to both app
assets under the existing `oldmadinaLayout.bin` names, then rebuild/install.
Glyph caching remains enabled for all glyphs by default.

## Standalone conversion (baked interpolation)

`bake_layout_variants.py` remains available to convert an existing **DKLY v3**
file to **DKLY v4** without the source font. It requires Python 3.13+ for fused
multiply/add arithmetic matching the iOS Release renderer used in the earlier
equivalence check:

```sh
python3 tools/bake_layout_variants.py INPUT.bin OUTPUT.variants.bin
```

This converter blends the serialized interpolation masters. Its outlines can
differ from the GUI's direct MetaPost evaluation. It does not regenerate the
font's true parameterized `GlyphVis` outlines. The preservation details and
historical prototype measurements below apply to this standalone converter.

## What is preserved

Body glyph variants are deduplicated by `(glyph code, left, right, third)` using
the exact values in the source binary. Interpolation and clamping match the
current Tarteel Release implementation. Final coordinates are stored as float32
to preserve the resulting SkPath values without another fixed-point rounding
step. This moves interpolation offline; it does not evaluate exact nonlinear
MetaPost shapes.

Line metadata is copied byte for byte. Clusters, advances, offsets and tajweed
colors retain their values. Surah-heading placements use ID zero because the
app renders those headings with a separate font. Every body placement has a
nonzero variant ID, including decorative and ayah basmalas.

The matching mushaf-renderer loader accepts v2/v3/v4 automatically. V4 creates a
document-owned immutable typeface at load time; page recording does not scan
variants or interpolate paths. Cache options still select cached glyph drawing,
paths, or the joining-glyph fallback. Tarteel now bundles v4 with glyph caching enabled for all glyphs. To update it,
replace both app assets under their existing `oldmadinaLayout.bin` names with
the generated `.variants.bin`, then rebuild/install the native app. Copy the v3
file instead to switch back to interpolation.

## V4 format

All numbers are big-endian. Outline coordinates are finite IEEE-754 float32.
Counts and IDs are unsigned; optional advances and offsets are signed.

| Field | Encoding |
| --- | --- |
| Magic, version | u32 `0x444B4C59` (`DKLY`), u16 `4` |
| Variant count | u16, 1–65,535; IDs assigned consecutively from 1 |
| Each variant | u16 original glyph code, u8 contour count |
| Each contour | float32 x/y start, u8 cubic count |
| Each cubic | Six float32 coordinates: control 1, control 2, endpoint |
| Page count | u16 |
| Each page | u8 line count |
| Each line | u8 placement count, placements, then line metadata |
| Each placement | u16 variant ID, u8 cluster, u8 optional-field mask |
| Optional fields | Mask bit 0: i16 advance; bit 1: i16 x offset; bit 2: i16 y offset; bit 3: u8 tajweed color, in that order |
| Line metadata | u8 type, i16 x, i32 fixed16.16 x scale, i32 fixed16.16 font size, i16 UTF-16 text length |
| Surah heading (type 1) | Additional u8 surah index |

Mask bits 4–7 are reserved and must be zero. ID zero is reserved for headings;
the loader rejects it in body lines. Empty outlines are allowed for spacing
glyphs. Contours follow the existing v3 path convention, with no extra close
verb. No interpolation masters or axis fields are stored in v4.

## Prototype results

The October 2, 2026 input contained 604 pages, 848,298 body placements and 4,793
variants. Output size increased from 9,230,791 to 10,278,464 bytes (+11.35%).
Native Release checks found exact SkPath equality for every variant and preserved
all placement mappings and line metrics. Three iOS simulator page captures were
identical to the interpolated single-typeface rendering.

Renderer loading plus full-typeface preparation took 70.42 ms with interpolation
and 24.00 ms with v4 in one startup measurement. Across 18 first-picture samples,
median native picture recording fell from 0.92 to 0.26 ms; warm redraw and GPU
drawing remained approximately the same. These are Release simulator results,
not complete app-startup or physical-device measurements.
