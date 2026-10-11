#!/usr/bin/env python3
"""Compare every rounded glyph placement, then render changed words offline.

Requires two generate_mushaf builds supporting --placement-data and
--review-words-file. Runs both with exactly the same font/settings. No Qt,
Python packages, local server, or browser connection is required.
"""
import argparse
import csv
import hashlib
import itertools
import json
import math
from pathlib import Path
import subprocess


def run(executable, config, output, *extra):
    command = [str(executable), '--config', str(config), '-o', str(output),
               '--report', '--report-generic-gap', '--report-gap-mode', 'collisions',
               '--no-placement-audit', '--min-severity', '1', '--report-limit', '0',
               '--no-notice', *extra]
    log = output.with_suffix('.log')
    print(f'Generating {output.name}', flush=True)
    with log.open('w') as stream:
        subprocess.run(command, stdout=stream, stderr=subprocess.STDOUT, check=True)
    manifest = json.loads(output.with_suffix('.run.json').read_text())
    if not manifest['complete']:
        raise ValueError(f'Incomplete run: {output}')
    return manifest


def records(path):
    with path.open() as stream:
        for line in stream:
            yield json.loads(line)


def compare(before, after):
    cases = {}
    pages = glyphs = raw_only = 0
    for a, b in itertools.zip_longest(records(before), records(after)):
        if a is None or b is None or a['page'] != b['page'] or len(a['glyphs']) != len(b['glyphs']):
            raise ValueError('Page/glyph count differs between inputs')
        pages += 1
        for old, new in zip(a['glyphs'], b['glyphs']):
            glyphs += 1
            # Every field except the solved delta and renderer offsets is input.
            for field in old:
                if field not in ('dx', 'dy', 'final') and old[field] != new[field]:
                    raise ValueError(f'Shaped input differs on page {a["page"]}: {field}')
            if old['final'] == new['final']:
                raw_only += abs(old['dx']-new['dx']) > 1e-8 or abs(old['dy']-new['dy']) > 1e-8
                continue
            key = (a['page'], old['line'], old['word'])
            if key not in cases:
                cases[key] = dict(page=key[0], line=key[1], word=key[2], text=old['text'], changes=[])
            shift_x = (new['final'][0]-old['final'][0])*old['xscale']
            shift_y = new['final'][1]-old['final'][1]
            cases[key]['changes'].append(dict(index=old['index'], name=old['name'], base=old['base'],
                before=old['final'], after=new['final'], x=shift_x, y=shift_y,
                distance=math.hypot(shift_x, shift_y)))
    for case in cases.values():
        case['maximumMovement'] = max(g['distance'] for g in case['changes'])
    return sorted(cases.values(), key=lambda c: (c['page'], c['line'], c['word'])), dict(
        pages=pages, glyphs=glyphs, changedWords=len(cases),
        changedGlyphs=sum(len(c['changes']) for c in cases.values()), rawOnlyGlyphs=raw_only)


def verify_sources(before, after, config):
    # Executables and the layout library intentionally differ. Font/database
    # and PDF resources must not change during the comparison.
    font_dir = Path(config['font']).resolve().parent
    database = Path(config['database']).resolve()
    for name, checksum in before['sources'].items():
        path = Path(name).resolve()
        if path.parent == font_dir or path == database or path.suffix in ('.pdf', '.ttf'):
            if after['sources'].get(name) != checksum:
                raise ValueError(f'Comparison source differs: {name}')
    if before['options'] != after['options']:
        raise ValueError('Resolved settings differ')


def sha256(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def findings(path):
    with path.open(newline='') as stream:
        return {(row['page'],row['type'],row['glyphA_index'],row['glyphB_index'],row['diagnostic']): row
                for row in csv.DictReader(stream)}


def verify_repeatability(reference, repeat):
    expected, actual = sha256(reference), sha256(repeat)
    if expected != actual:
        raise ValueError(f'Non-identical repeated placements: {reference} vs {repeat}. '
                         'Resolve repeatability before reviewing a code change.')
    return dict(verified=True, referenceSha256=expected,
                repeat=dict(path=str(repeat.resolve()), sha256=actual))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before-executable', required=True, type=Path)
    parser.add_argument('--after-executable', required=True, type=Path)
    parser.add_argument('--config', required=True, type=Path)
    parser.add_argument('--output-dir', required=True, type=Path)
    parser.add_argument('--reuse', action='store_true', help='Rebuild viewer from already completed runs')
    parser.add_argument('--reuse-before', action='store_true', help='Keep the completed baseline Mushaf; regenerate after and previews')
    parser.add_argument('--before-repeat-data', type=Path, help='Existing independent baseline placement export to verify')
    parser.add_argument('--after-repeat-data', type=Path, help='Existing independent changed-build placement export to verify')
    parser.add_argument('--skip-repeatability-check', action='store_true', help='Explicitly generate an unverified comparison')
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    config = json.loads(args.config.read_text())
    manifests = []
    for label, executable in [('before', args.before_executable), ('after', args.after_executable)]:
        pdf = args.output_dir / f'{label}.pdf'
        if args.reuse or (args.reuse_before and label == 'before'):
            manifest = json.loads(pdf.with_suffix('.run.json').read_text())
            if not manifest['complete']:
                raise ValueError('Incomplete cached run')
        else:
            manifest = run(executable, args.config, pdf, '--placement-data',
                           str(args.output_dir / f'{label}.placements.jsonl'))
        manifests.append(manifest)
    verify_sources(*manifests, config)
    cases, summary = compare(args.output_dir/'before.placements.jsonl', args.output_dir/'after.placements.jsonl')
    requests = args.output_dir/'review-words.json'
    requests.write_text(json.dumps([[c['page'],c['line'],c['word']] for c in cases]))
    proof_path = args.output_dir/'repeatability.json'
    cached_proof = json.loads(proof_path.read_text()) if args.reuse and proof_path.exists() else {}
    repeatability = {}
    for label, executable in [('before', args.before_executable), ('after', args.after_executable)]:
        preview = args.output_dir / f'{label}-preview.pdf'
        existing_repeat = getattr(args, f'{label}_repeat_data')
        repeat = existing_repeat or args.output_dir/f'{label}-preview.placements.jsonl'
        if not args.reuse and cases:
            extra = [] if args.skip_repeatability_check or existing_repeat else ['--placement-data',str(repeat)]
            run(executable, args.config, preview, '--no-pdf', '--review-words-file', str(requests), *extra)
        if not args.skip_repeatability_check:
            if not args.reuse and not cases and not existing_repeat:
                # With no changed words there is no preview pass. Still require
                # an independent placement-only run of each build.
                run(executable, args.config, preview, '--no-pdf', '--no-report', '--placement-data',str(repeat))
            if args.reuse and not repeat.exists() and label in cached_proof:
                repeat = Path(cached_proof[label]['repeat']['path'])
            if not repeat.exists():
                raise ValueError(f'Missing {label} repeat export; provide --{label}-repeat-data or rerun without --reuse')
            repeatability[label] = verify_repeatability(args.output_dir/f'{label}.placements.jsonl',repeat)
        words = json.loads(preview.with_suffix('.review.json').read_text())['words'] if cases else []
        if len(words) != len(cases):
            raise ValueError('Incomplete preview selection')
        for case, word in zip(cases, words):
            if [case[k] for k in ('page','line','word')] != [word[k] for k in ('page','line','word')]:
                raise ValueError('Preview order differs')
            # Previews are drawn to 0.01 font unit precision. Trim serialized
            # floats too, without changing the exact placement comparison.
            for glyph in word['glyphs']:
                glyph['box'] = [round(v,2) for v in glyph['box']]
                glyph['polygons'] = [[[round(v,2) for v in point] for point in polygon]
                                     for polygon in glyph['polygons']]
            case[label] = word['glyphs']
            if not any(g['line']==case['line'] and g['word']==case['word'] for g in word['glyphs']):
                raise ValueError('Missing word ink')
    if repeatability:
        proof_path.write_text(json.dumps(repeatability,indent=2))
    summary['repeatability'] = repeatability
    summary['beforeSeconds'] = manifests[0]['seconds']
    summary['afterSeconds'] = manifests[1]['seconds']
    summary['beforePlacementSeconds'] = manifests[0]['placementSeconds']
    summary['afterPlacementSeconds'] = manifests[1]['placementSeconds']
    before_findings = findings(args.output_dir/'before_violations.csv')
    after_findings = findings(args.output_dir/'after_violations.csv')
    summary['findings'] = dict(before=manifests[0]['statistics']['byType'],
        after=manifests[1]['statistics']['byType'],
        new=len(after_findings.keys()-before_findings.keys()),
        resolved=len(before_findings.keys()-after_findings.keys()), minimumSeverity=1)
    for case in cases:
        case['findings'] = {}
        case['regression'] = False
        for label, rows in [('before',before_findings),('after',after_findings)]:
            case['findings'][label] = [dict(type=r['type'],severity=float(r['report_severity']),detail=r['detail'])
                for r in rows.values() if int(r['page'])==case['page'] and (
                    (int(r['lineA'])==case['line'] and int(r['wordA_number'])==case['word']) or
                    (int(r['lineB'])==case['line'] and int(r['wordB_number'])==case['word']))]
        for key,r in after_findings.items():
            belongs = int(r['page'])==case['page'] and (
                (int(r['lineA'])==case['line'] and int(r['wordA_number'])==case['word']) or
                (int(r['lineB'])==case['line'] and int(r['wordB_number'])==case['word']))
            previous = before_findings.get(key)
            if belongs and (previous is None or float(r['report_severity'])>float(previous['report_severity'])+1):
                case['regression'] = True
    attribution_path = args.output_dir/'stack-attribution.json'
    if attribution_path.exists():
        attribution = json.loads(attribution_path.read_text())
        for label in ('before','after'):
            if attribution[f'{label}PlacementsSha256'] != sha256(args.output_dir/f'{label}.placements.jsonl'):
                raise ValueError('Stack attribution belongs to a different placement comparison')
        for case in cases:
            case['attribution'] = attribution['words'][f'{case["page"]}:{case["line"]}:{case["word"]}']
        summary['attribution'] = {key:attribution[key] for key in ('counts','changedGroups','changedPages','patterns')}
    summary['executables'] = {label: dict(path=str(path.resolve()),sha256=sha256(path)) for label,path in
                              [('before',args.before_executable),('after',args.after_executable)]}
    summary['configuration'] = config
    (args.output_dir/'summary.json').write_text(json.dumps(summary, ensure_ascii=False, indent=2))
    (args.output_dir/'changes.json').write_text(json.dumps(cases, ensure_ascii=False, separators=(',',':')))
    with (args.output_dir/'changes.csv').open('w') as stream:
        writer = csv.writer(stream)
        writer.writerow(['page','line','word','text','glyph','index','base','before_x','before_y','after_x','after_y','delta_x_world','delta_y_world'])
        for c in cases:
            for g in c['changes']:
                writer.writerow([c['page'],c['line'],c['word'],c['text'],g['name'],g['index'],g['base'],*g['before'],*g['after'],g['x'],g['y']])
    template = Path(__file__).with_name('placement_comparison.html').read_text()
    payload = json.dumps(dict(summary=summary,cases=cases), ensure_ascii=False, separators=(',',':')).replace('<','\\u003c')
    (args.output_dir/'comparison.html').write_text(template.replace('/*PLACEMENT_DATA*/',payload))
    print(json.dumps({k:v for k,v in summary.items() if k not in ('configuration','executables')},indent=2))
    print(args.output_dir/'comparison.html')

if __name__ == '__main__':
    main()
