# DigitalKhatt native runtime

This directory provides a small C ABI over DigitalKhatt's HarfBuzz fork. It
loads a compiled CFF2/JTST font, shapes one independent Arabic line, returns
glyph placement and tatweel coordinates, and streams the selected CFF2
outlines to renderer-owned callbacks.

It deliberately excludes Qt, MetaPost, page policy, page composition, corpus
loading, and platform UI code. Those belong outside the low-level typography
kernel.

## ABI v1

The public header supports:

- loading a font from a file or copied memory;
- shaping one RTL Arabic UTF-16 line, with an optional JTST target width;
- reading versioned line metrics and glyph records;
- emitting a glyph outline at normalized LTAT/RTAT coordinates;
- cancellation or failure from any path callback.

All request and output records carry `struct_size`; later fields are append-only.
Opaque line results own their storage, so no borrowed fixed-stride glyph array
crosses the ABI. The engine and immutable results support concurrent reads and
independent calls as documented in `engine.h`.

The line operation is intentionally low-level. It does not implement a Mushaf
page profile or the page-wide planning policy used by `FeatureJustifier`.

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

Set `DIGITALKHATT_ENGINE_TEST_FONT` to exercise another compatible font. Shared
installs export `digitalkhatt::engine` through
`find_package(DigitalKhattEngine CONFIG REQUIRED)`.

The runtime inherits this repository's AGPL-3.0-or-later license. Review those
terms before distributing an application that embeds the library.
