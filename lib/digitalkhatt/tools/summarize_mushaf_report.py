#!/usr/bin/env python3
"""Summarize a completed Generate Mushaf CSV without GUI dependencies."""
import argparse
import collections
import csv
import json
import math
from pathlib import Path


def summarize(csv_path, manifest_path):
    manifest = json.loads(manifest_path.read_text())
    if not manifest.get('complete') or not manifest['options']['report']:
        raise ValueError('The manifest must describe a completed run with reporting enabled')
    counters = {key: collections.Counter() for key in ('by_type', 'by_detail', 'introduced', 'worsened')}
    examples = collections.defaultdict(list)
    total = 0
    with csv_path.open(newline='', encoding='utf-8') as stream:
        for row in csv.DictReader(stream):
            total += 1
            if not math.isfinite(float(row['severity'])):
                raise ValueError('Non-finite severity in CSV')
            counters['by_type'][row['type']] += 1
            counters['by_detail'][row['detail']] += 1
            for key in ('introduced', 'worsened'):
                if row[key] == '1':
                    counters[key][row['type']] += 1
            category = row['type'] if row['type'] in ('MarkSide', 'BaseAssociation', 'MarkClassification', 'InvalidPlacement', 'PlacementRejected') else row['detail']
            if category and len(examples[category]) < 8:
                examples[category].append({key: row[key] for key in ('page', 'lineA', 'lineB', 'glyphA_name', 'glyphB_name', 'wordA', 'wordB', 'severity', 'initial_severity', 'introduced', 'worsened', 'detail')})
    if total != manifest['statistics']['findings']:
        raise ValueError(f'CSV has {total} rows; manifest expects {manifest["statistics"]["findings"]}')
    return {'complete': True, 'configuration': manifest['options'], 'coverage': manifest['statistics'],
            'counts': {key: dict(value) for key, value in counters.items()}, 'examples': dict(examples),
            'interpretation': 'In historical reports, PlacementRejected records a restored candidate. Current XPBD has no additional placement clamps or contact rollback. Minimum gap warnings include clearance targets. Contacts use unsplit mark component hulls and decomposed bases, not exact filled ink; ownership and side diagnostics also require visual review.'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('csv', type=Path)
    parser.add_argument('--manifest', type=Path)
    parser.add_argument('-o', '--output', type=Path)
    args = parser.parse_args()
    manifest = args.manifest or args.csv.with_name(args.csv.name.removesuffix('_violations.csv') + '.run.json')
    result = summarize(args.csv, manifest)
    text = json.dumps(result, indent=2, ensure_ascii=False) + '\n'
    if args.output:
        args.output.write_text(text, encoding='utf-8')
    else:
        print(text, end='')


if __name__ == '__main__':
    main()
