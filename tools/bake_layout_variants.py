#!/usr/bin/env python3
"""Bake a Generate Layout Info v3 binary into a v4 file of final glyph variants.

Interpolation happens here, using the float masters and axis values actually
stored in v3. The app needs only paths and direct glyph IDs. This preserves the
existing rendering instead of changing it to MetaPost's exact nonlinear shape.
All numbers are big-endian; final path coordinates are IEEE-754 float32.
Requires Python 3.13+ for math.fma.
"""

import argparse
import hashlib
import json
import math
from pathlib import Path
import struct


def float32(value):
    return struct.unpack('>f', struct.pack('>f', value))[0]


class Reader:
    def __init__(self, data):
        self.data = data
        self.offset = 0

    def read(self, fmt):
        fmt = '>' + fmt
        size = struct.calcsize(fmt)
        if size > len(self.data) - self.offset:
            raise ValueError(f'Truncated layout at byte {self.offset}')
        values = struct.unpack_from(fmt, self.data, self.offset)
        self.offset += size
        return values[0] if len(values) == 1 else values

    def fixed(self):
        return float32(self.read('i') / 65536)

    def shapes(self):
        shapes = []
        for _ in range(self.read('B')):
            points = [[self.fixed(), self.fixed()]]
            for _ in range(self.read('B')):
                points.append([self.fixed() for _ in range(6)])
            shapes.append(points)
        return shapes


def read_v3(data):
    r = Reader(data)
    if r.read('IH') != (0x444B4C59, 3):
        raise ValueError('Input must be a DKLY version 3 layout')
    masters = {}
    for _ in range(r.read('H')):
        code = r.read('I')
        shapes = r.shapes()
        limits = [r.fixed() for _ in range(4)]
        extremes = [r.shapes() if limit else [] for limit in limits]
        limits.extend([r.fixed(), r.fixed()])
        extremes.extend(r.shapes() if limit else [] for limit in limits[4:])
        if code > 65535 or code in masters:
            raise ValueError(f'Invalid or duplicate glyph code {code}')
        masters[code] = (shapes, limits, extremes)
    pages = []
    for _ in range(r.read('H')):
        lines = []
        for _ in range(r.read('B')):
            glyphs = []
            for _ in range(r.read('B')):
                code, cluster, mask = r.read('HBB')
                if mask & 0x80:
                    raise ValueError('Unknown placement mask')
                # Preserve the placement values without altering rounding.
                values = [r.read(fmt) if mask & (1 << bit) else 0
                          for bit, fmt in enumerate(['h', 'h', 'h', 'B'])]
                axes = tuple(r.fixed() if mask & (1 << bit) else 0.0
                             for bit in range(4, 7))
                glyphs.append((code, cluster, mask & 15, values, axes))
            start = r.offset
            line_type = r.read('B')
            r.read('h')
            x_scale, font_size = r.fixed(), r.fixed()
            length = r.read('h')
            if line_type == 1:
                r.read('B')
            if line_type > 2 or length < 0 or x_scale <= 0 or font_size <= 0:
                raise ValueError('Invalid line metrics')
            clusters = [g[1] for g in glyphs]
            if clusters != sorted(clusters) or any(c >= length for c in clusters):
                raise ValueError('Invalid glyph clusters')
            lines.append((line_type, glyphs, data[start:r.offset]))
        pages.append(lines)
    if r.offset != len(data):
        raise ValueError(f'Trailing data at byte {r.offset}')
    return masters, pages


def interpolate(master, axes):
    shapes, limits, extremes = master
    axes = [max(limits[2*i], min(limits[2*i+1], axis))
            for i, axis in enumerate(axes)]
    result = []
    for i, contour in enumerate(shapes):
        points = []
        for j, segment in enumerate(contour):
            coords = []
            for k, default in enumerate(segment):
                value = default
                for axis_index, axis in enumerate(axes):
                    if axis == 0:
                        continue
                    index = 2*axis_index + (1 if axis > 0 else 0)
                    if not extremes[index]:
                        raise ValueError('Missing master for nonzero axis')
                    extreme = extremes[index][i][j][k]
                    # JsiPage::getGlyphPath adds each axis delta to the default,
                    # using doubles, then SkPathBuilder converts to float32.
                    # Match Clang's fused multiply/add in the Release native
                    # renderer; Python 3.13+ exposes the same IEEE operation.
                    value = math.fma(extreme - default, axis / limits[index], value)
                if not math.isfinite(value):
                    raise ValueError('Non-finite outline coordinate')
                coords.append(float32(value))
            points.append(coords)
        result.append(points)
    return result


def bake(data):
    masters, pages = read_v3(data)
    keys = sorted({(g[0], *g[4]) for page in pages for kind, glyphs, _ in page
                   if kind != 1 for g in glyphs})
    if not 0 < len(keys) <= 65535:
        raise ValueError('Variant count cannot fit uint16 IDs')
    ids = {key: i + 1 for i, key in enumerate(keys)}
    output = bytearray(struct.pack('>IHH', 0x444B4C59, 4, len(keys)))
    for code, *axes in keys:
        if code not in masters:
            raise ValueError(f'Missing master glyph {code}')
        shapes = interpolate(masters[code], axes)
        output.extend(struct.pack('>HB', code, len(shapes)))
        for points in shapes:
            output.extend(struct.pack('>ffB', *points[0], len(points) - 1))
            for cubic in points[1:]:
                output.extend(struct.pack('>6f', *cubic))
    outline_bytes = len(output) - 6
    output.extend(struct.pack('>H', len(pages)))
    placements = 0
    for page in pages:
        output.extend(struct.pack('>B', len(page)))
        for kind, glyphs, metadata in page:
            output.extend(struct.pack('>B', len(glyphs)))
            for code, cluster, mask, values, axes in glyphs:
                variant_id = ids[(code, *axes)] if kind != 1 else 0
                output.extend(struct.pack('>HBB', variant_id, cluster, mask))
                for bit, (fmt, value) in enumerate(zip(['h', 'h', 'h', 'B'], values)):
                    if mask & (1 << bit):
                        output.extend(struct.pack('>' + fmt, value))
                placements += kind != 1
            output.extend(metadata)
    return bytes(output), dict(pages=len(pages), placements=placements,
                               variants=len(keys), outlineTableBytes=outline_bytes)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('input', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if not hasattr(math, 'fma'):
        parser.error('Python 3.13+ is required for Release-equivalent fused arithmetic')
    if args.input.resolve() == args.output.resolve():
        parser.error('Use a separate output file to preserve the original layout')
    data = args.input.read_bytes()
    output, stats = bake(data)
    args.output.write_bytes(output)
    stats.update(sourceBytes=len(data), outputBytes=len(output),
                 changePercent=100 * (len(output) / len(data) - 1),
                 sourceSha256=hashlib.sha256(data).hexdigest(),
                 outputSha256=hashlib.sha256(output).hexdigest())
    print(json.dumps(stats, indent=2))


if __name__ == '__main__':
    main()
