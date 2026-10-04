# Bake static layout glyph variants

`bake_layout_variants.py` converts a **DKLY v3** layout from Generate Layout Info
into a **DKLY v4** layout containing final outlines and direct variant IDs. It
requires Python 3.13+ for fused multiply/add arithmetic matching the iOS Release
renderer used in the equivalence check.

```sh
python3 tools/bake_layout_variants.py INPUT.bin OUTPUT.variants.bin
```

Export the input using the intended justification parameters, Force and tajweed.
The converter uses that input without reshaping or changing the layout. It
refuses to overwrite the input and prints counts, byte sizes and SHA-256 hashes.
The GUI exporter remains unchanged and continues to write v3.

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
paths, or the joining-glyph fallback. Bundled app assets must both be replaced
under their existing `oldmadinaLayout.bin` names to opt into v4, with a native
rebuild/install.

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
