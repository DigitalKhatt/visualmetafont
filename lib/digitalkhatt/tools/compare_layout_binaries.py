#!/usr/bin/env python3
"""Compare two final whole-glyph DKLY v4 exports, without loading a font or Qt.

Optional <binary>.json sidecars provide UTF-16 source words, glyph names and
absolute baselines. Without sidecars, review cases cover whole body lines.
"""
import argparse
import csv
import hashlib
import itertools
import json
import math
from pathlib import Path
import struct


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


class Reader:
    def __init__(self, path):
        self.data = path.read_bytes()
        self.offset = 0

    def read(self, fmt):
        size = struct.calcsize('>' + fmt)
        if self.offset + size > len(self.data):
            raise ValueError(f'Truncated binary at byte {self.offset}')
        values = struct.unpack_from('>' + fmt, self.data, self.offset)
        self.offset += size
        return values[0] if len(values) == 1 else values

    def outline(self):
        contours = []
        for _ in range(self.read('B')):
            start = self.read('ff')
            cubics = [self.read('6f') for _ in range(self.read('B'))]
            if any(not math.isfinite(v) for segment in [start, *cubics] for v in segment):
                raise ValueError('Non-finite outline')
            contours.append([start, *cubics])
        return contours


def read_binary(path):
    r = Reader(path)
    if r.read('IH') != (0x444B4C59, 4):
        raise ValueError('Use a DKLY v4 Whole glyphs export, not an interpolated v3 export')
    variants = {i + 1: (r.read('H'), r.outline()) for i in range(r.read('H'))}
    pages = []
    for _ in range(r.read('H')):
        lines = []
        for _ in range(r.read('B')):
            glyphs = []
            for _ in range(r.read('B')):
                variant, cluster, mask = r.read('HBB')
                if mask & 240:
                    raise ValueError('Unsupported placement mask')
                values = [r.read(fmt) if mask & (1 << bit) else 0
                          for bit, fmt in enumerate(('h', 'h', 'h', 'B'))]
                glyphs.append(dict(variant=variant, cluster=cluster,
                                   advance=values[0], dx=values[1], dy=values[2], color=values[3]))
            kind, x, sx, size, length = r.read('BhiiH')
            surah = r.read('B') if kind == 1 else None
            if kind > 2 or sx <= 0 or size <= 0:
                raise ValueError('Invalid line metrics')
            for g in glyphs:
                if g['cluster'] >= length:
                    raise ValueError('Cluster outside source text')
                if kind != 1 and g['variant'] not in variants:
                    raise ValueError('Missing body glyph variant')
            lines.append(dict(kind=kind, x=x, xscale=sx / 65536, fontSize=size / 65536,
                              length=length, surah=surah, glyphs=glyphs))
        pages.append(lines)
    if r.offset != len(r.data):
        raise ValueError('Trailing binary data')
    return dict(variants=variants, pages=pages)


def metadata(path, document):
    sidecar = Path(str(path) + '.json')
    if not sidecar.exists():
        return None
    data = json.loads(sidecar.read_text())
    if len(data['pages']) != len(document['pages']):
        raise ValueError('Sidecar page count differs')
    for p, page in enumerate(document['pages']):
        if len(data['pages'][p]) != len(page) or len(data['baselines'][p]) != len(page):
            raise ValueError('Sidecar line count differs')
        for l, line in enumerate(page):
            if len(data['pages'][p][l].encode('utf-16-le')) // 2 != line['length']:
                raise ValueError('Sidecar source length differs')
    return data


def word_map(text):
    units = list(struct.unpack('<' + 'H' * (len(text.encode('utf-16-le')) // 2), text.encode('utf-16-le')))
    result = []; words = {}; number = 0; start = 0
    for i, unit in enumerate(units):
        if unit != 32 and (i == 0 or units[i - 1] == 32):
            number += 1; start = i
        result.append(number if unit != 32 else 0)
        if unit != 32 and (i + 1 == len(units) or units[i + 1] == 32):
            words[number] = struct.pack('<' + 'H' * (i + 1 - start), *units[start:i + 1]).decode('utf-16-le')
    return result, words


def resolve(document, meta, spacing):
    for p, page in enumerate(document['pages']):
        glyphs = []; words = {}
        for l, line in enumerate(page):
            if line['kind'] == 1:
                continue  # Headings use a separate font, absent from DKLY outlines.
            mapping, labels = word_map(meta['pages'][p][l]) if meta else (None, {0: f'Line {l + 1}'})
            words.update({(l + 1, w): t for w, t in labels.items()})
            y = meta['baselines'][p][l] if meta else -(l * spacing)
            x = -line['x']; scale_x = line['xscale'] * line['fontSize']; scale_y = line['fontSize']
            for i, placement in enumerate(line['glyphs']):
                x -= placement['advance'] * line['xscale']
                origin = (x + placement['dx'] * line['xscale'], y + placement['dy'])
                code, outline = document['variants'][placement['variant']]
                word = mapping[placement['cluster']] if mapping is not None else 0
                if mapping is not None and word == 0:
                    continue  # Whitespace has no ink; advances still affect later glyphs.
                contours = []
                for contour in outline:
                    contours.append([tuple(origin[j % 2] + v * (scale_x if j % 2 == 0 else scale_y)
                                           for j, v in enumerate(segment)) for segment in contour])
                coords = [pt for contour in contours for segment in contour for pt in zip(segment[::2], segment[1::2])]
                if not coords:
                    continue
                bounds = [min(pt[0] for pt in coords), min(pt[1] for pt in coords),
                          max(pt[0] for pt in coords), max(pt[1] for pt in coords)]
                glyphs.append(dict(index=(l << 8) + i, base=-1, line=l + 1, word=word,
                    name=meta['glyphNames'].get(str(code), f'Glyph {code}') if meta else f'Glyph {code}',
                    cluster=placement['cluster'], code=code, origin=origin, color=placement['color'],
                    local=outline, scale=(scale_x, scale_y), contours=contours, box=bounds))
        yield dict(glyphs=glyphs, words=words)


def signature(glyphs):
    # IDs, glyph order, and contour order do not change the rendered ink.
    return sorted((g['color'], tuple(segment for segment in contour))
                  for g in glyphs for contour in g['contours'])


def compare(before, after):
    cases = []; total = 0; pages = 0
    for p, (a, b) in enumerate(itertools.zip_longest(before, after)):
        pages += 1
        a = a or dict(glyphs=[], words={})
        b = b or dict(glyphs=[], words={})
        total += len(b['glyphs'])
        keys = sorted(a['words'].keys() | b['words'].keys())
        for line, word in keys:
            old = [g for g in a['glyphs'] if (g['line'], g['word']) == (line, word)]
            new = [g for g in b['glyphs'] if (g['line'], g['word']) == (line, word)]
            if signature(old) == signature(new):
                continue
            changes = []; changed_indices = set(); maximum = 0; outline_changed = False; structure_changed = len(old) != len(new)
            for i in range(max(len(old), len(new))):
                ga = old[i] if i < len(old) else None; gb = new[i] if i < len(new) else None
                if ga and gb and signature([ga]) == signature([gb]):
                    continue
                changed_indices.update(g['index'] for g in (ga, gb) if g)
                same_shape = bool(ga and gb and ga['local'] == gb['local'] and
                                  ga['scale'] == gb['scale'] and ga['color'] == gb['color'])
                outline_changed |= not same_shape
                structure_changed |= not (ga and gb and ga['name'] == gb['name'] and ga['cluster'] == gb['cluster'])
                dx = gb['origin'][0] - ga['origin'][0] if ga and gb else 0
                dy = gb['origin'][1] - ga['origin'][1] if ga and gb else 0
                distance = math.hypot(dx, dy); maximum = max(maximum, distance)
                g = gb or ga
                changes.append(dict(index=g['index'], name=g['name'], base=-1, cluster=g['cluster'],
                    before=list(ga['origin']) if ga else None, after=list(gb['origin']) if gb else None,
                    x=dx, y=dy, distance=distance,
                    kind='placement' if same_shape else 'outline / glyph sequence'))
            neighbors = lambda record: [{k: v for k, v in g.items() if k != 'local'}
                                        for g in record['glyphs'] if abs(g['line'] - line) <= 1]
            cases.append(dict(page=p + 1, line=line, word=word, text=b['words'].get((line, word), a['words'].get((line, word), '')),
                changes=changes, maximumMovement=maximum, outlineChanged=outline_changed,
                structureChanged=structure_changed, regression=False,
                changedIndices=sorted(changed_indices),
                before=neighbors(a), after=neighbors(b), findings={}))
    return cases, dict(pages=pages, glyphs=total, changedWords=len(cases),
                       changedGlyphs=sum(len(c['changes']) for c in cases), rawOnlyGlyphs=0,
                       outlineChangedWords=sum(c['outlineChanged'] for c in cases), binaryComparison=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before', required=True, type=Path)
    parser.add_argument('--after', required=True, type=Path)
    parser.add_argument('--output-dir', required=True, type=Path)
    parser.add_argument('--before-repeat', type=Path)
    parser.add_argument('--after-repeat', type=Path)
    parser.add_argument('--skip-repeatability-check', action='store_true')
    parser.add_argument('--line-spacing', type=float, default=1683, help='Used only without text/baseline sidecars')
    args = parser.parse_args(); proof = {}
    for side in ('before', 'after'):
        original, repeat = getattr(args, side), getattr(args, side + '_repeat')
        if not args.skip_repeatability_check:
            if repeat is None:
                parser.error('Provide both --before-repeat and --after-repeat, or explicitly --skip-repeatability-check')
            digest = sha256(original)
            if digest != sha256(repeat):
                raise ValueError(f'Non-identical {side} binary repeat')
            sidecar = Path(str(original) + '.json'); repeated_sidecar = Path(str(repeat) + '.json')
            if sidecar.exists() and (not repeated_sidecar.exists() or sha256(sidecar) != sha256(repeated_sidecar)):
                raise ValueError(f'Non-identical {side} sidecar repeat')
            proof[side] = dict(verified=True, referenceSha256=digest, repeat=dict(path=str(repeat.resolve()), sha256=digest))
    documents = [read_binary(p) for p in (args.before, args.after)]
    labels = [metadata(p, d) for p, d in zip((args.before, args.after), documents)]
    if bool(labels[0]) != bool(labels[1]):
        raise ValueError('Provide sidecars for both exports, or neither')
    cases, summary = compare(*(resolve(d, m, args.line_spacing) for d, m in zip(documents, labels)))
    summary['inkGlyphs'] = summary['glyphs']
    summary['glyphs'] = sum(len(line['glyphs']) for page in documents[1]['pages'] for line in page if line['kind'] != 1)
    summary['repeatability'] = proof
    summary['configuration'] = dict(before=str(args.before.resolve()), after=str(args.after.resolve()),
        version=4, wordLabels=bool(labels[0]), lineSpacingWithoutSidecar=args.line_spacing,
        beforeSha256=sha256(args.before), afterSha256=sha256(args.after))
    args.output_dir.mkdir(parents=True, exist_ok=True)
    (args.output_dir / 'summary.json').write_text(json.dumps(summary, indent=2))
    (args.output_dir / 'changes.json').write_text(json.dumps(cases, ensure_ascii=False, separators=(',', ':')))
    with (args.output_dir / 'changes.csv').open('w') as stream:
        writer = csv.writer(stream); writer.writerow(['page', 'line', 'word', 'text', 'glyph', 'kind', 'delta_x', 'delta_y'])
        for c in cases:
            for g in c['changes']:
                writer.writerow([c['page'], c['line'], c['word'], c['text'], g['name'], g['kind'], g['x'], g['y']])
    template = Path(__file__).with_name('placement_comparison.html').read_text()
    payload = json.dumps(dict(summary=summary, cases=cases), ensure_ascii=False, separators=(',', ':')).replace('<', '\\u003c')
    (args.output_dir / 'comparison.html').write_text(template.replace('/*PLACEMENT_DATA*/', payload))
    print(json.dumps({k: v for k, v in summary.items() if k not in ('repeatability', 'configuration')}, indent=2))
    print(args.output_dir / 'comparison.html')


if __name__ == '__main__':
    main()
