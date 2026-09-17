# DigitalKhatt native runtime

This directory provides a small C ABI over DigitalKhatt's HarfBuzz fork. It
loads a compiled CFF2/JTST font, shapes Arabic text, returns glyph placement and
tatweel coordinates, and streams selected CFF2 outlines to renderer-owned
callbacks.

It deliberately excludes Qt, MetaPost, page composition, corpus loading, and
platform UI code. Those belong outside the typography runtime.

## ABI v1

The ABI major remains 1. The `0.2` symbol set adds page typography without
changing any `0.1` symbol or structure.

The low-level line operation:

- shapes one independent RTL Arabic UTF-16 line;
- optionally runs the font's JTST target-width pass;
- does **not** apply a Mushaf page profile or FeatureJustifier page policy.

The additive page operation:

- accepts ordered UTF-16 lines, target widths, semantic roles, and alignment;
- applies a profile-versioned page-wide feature-planning policy;
- returns immutable line and glyph results with fractional final spacing;
- expresses widths and glyph positions in font design units, leaving device
  scaling to the host;
- leaves page breaking, vertical baselines, decorations, interaction, and
  rendering to the host.

`DK_PAGE_PROFILE_MADINAH_1441_V1` maps to the canonical New Madinah
FeatureJustifier policy. Revision 1 does not expose feature records, tajweed
colors, screen coordinates, selection data, or platform paths.

Page input and output records are append-only and size-versioned. Set their
`struct_size` fields to the matching `*_V1_SIZE` constant. The path sink follows
the same convention.

## Font contract

The runtime accepts the two-axis DigitalKhatt runtime profile:

- CFF2, GSUB, GPOS, GDEF, JTST, `fvar`, and HVAR tables;
- exactly two variation axes;
- `LTAT` at axis index 0 and `RTAT` at axis index 1;
- both axes centered on zero with negative and positive ranges.

Tatweel values crossing the ABI are normalized coordinates in `[-1, 1]`.

## Build and test

```sh
cmake -S lib/digitalkhatt/runtime -B build/digitalkhatt-runtime -G Ninja \
  -DBUILD_SHARED_LIBS=ON
cmake --build build/digitalkhatt-runtime
ctest --test-dir build/digitalkhatt-runtime --output-on-failure
cmake --install build/digitalkhatt-runtime --prefix /your/sdk/prefix
```

Set `DIGITALKHATT_ENGINE_TEST_FONT` to exercise another compatible font. Set
`DIGITALKHATT_ENGINE_PAGE_PARITY_FONT` to a corrected compiler runtime font to
enable the strict page-43 parity fixture (15 lines, 1,352 glyphs, and 1,230
visible outlines).

Shared installs export `digitalkhatt::engine` through
`find_package(DigitalKhattEngine CONFIG REQUIRED)`.

The runtime inherits this repository's AGPL-3.0-or-later license. Review those
terms before distributing an application that embeds the library.
