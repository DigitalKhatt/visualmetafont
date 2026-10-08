#!/usr/bin/env python3
"""Compare GJK/EPA and cached whole-outline NFP contacts within the same XPBD solver.

Uses the standard-library only. Audits use the unchanged Save Collision detector;
timing runs disable PDF/report generation and start with fresh caches.
"""
import argparse
import csv
import html
import json
from pathlib import Path
import statistics
import subprocess

CASES = ['17:5:5', '45:3:8', '78:12:3', '120:12:3', '153:8:2', '169:2:9',
         '271:8:6', '399:6:9', '423:9:4', '443:12:7', '465:5:7', '565:13:8']
METHODS = ('gjk', 'nfp')


def execute(args, method, directory, timing=False):
    directory.mkdir(parents=True, exist_ok=True)
    command = [str(args.executable), '--config', str(args.config), '--contact-method', method,
               '--force', '--pages', args.pages, '--output', str(directory / 'mushaf.pdf'),
               '--placement-audit' if args.placement_audit else '--no-placement-audit',
               '--nfp-cache-limit', str(args.nfp_cache_limit)]
    if timing:
        command += ['--no-report', '--no-pdf']
    else:
        command += ['--report', '--report-generic-gap', '--report-gap-mode', 'collisions',
                    '--report-limit', '0', '--all-findings']
        if not args.pdf:
            command += ['--no-pdf']
        for case in args.cases:
            command += ['--review-word', case]
    (directory / 'command.json').write_text(json.dumps(command, indent=2)+'\n')
    print(f'{method.upper()}: {"timing" if timing else "audit"} -> {directory}', flush=True)
    with (directory / 'run.log').open('w') as log:
        subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
    manifest = json.loads((directory / 'mushaf.run.json').read_text())
    if not manifest['complete']:
        raise RuntimeError(f'Incomplete run: {directory}')
    return manifest


def rows(directory):
    with (directory / 'mushaf_violations.csv').open() as source:
        return list(csv.DictReader(source))


def key(row):
    second = row['baseA_index'] if row['type'] == 'WaqfPlacement' else row['glyphB_index']
    return tuple(row[f] for f in ('page', 'type', 'glyphA_index')) + (second, row['diagnostic'])


def shaping_options(options):
    return {k: v for k, v in options.items() if k != 'xpbd'}


def placement_options(options):
    result = {k: v for k, v in options.items() if k not in ('pdf', 'report', 'xpbd', 'summaryLimit')}
    params = {k: v for k, v in options['xpbd'].items()
              if not k.startswith('report') and k != 'minViolationSeverity'}
    params['toggles'] = {k: v for k, v in params['toggles'].items() if not k.startswith('report')}
    result['xpbd'] = params
    return result


def write_comparison(args, audits, timings):
    output = args.output_dir
    baseline, prototype = (audits[m] for m in METHODS)
    if baseline['sources'] != prototype['sources']:
        raise RuntimeError('Input or executable changed between audits')
    if shaping_options(baseline['options']) != shaping_options(prototype['options']):
        raise RuntimeError('Shaping settings differ')
    old, new = (dict(audits[m]['options']['xpbd']) for m in METHODS)
    old.pop('useNoFitPolygons'); new.pop('useNoFitPolygons')
    if old != new:
        raise RuntimeError('XPBD parameters differ beyond the contact method')
    for m in METHODS:
        if audits[m]['options']['xpbd']['noFitPolygonCacheLimit'] != args.nfp_cache_limit:
            raise RuntimeError('NFP cache capacity differs from requested comparison')
        if audits[m]['options']['xpbd']['toggles']['reportPlacementAudit'] != args.placement_audit:
            raise RuntimeError('Placement audit setting differs from requested comparison')
        for run in timings[m]:
            if run['sources'] != audits[m]['sources'] or placement_options(run['options']) != placement_options(audits[m]['options']):
                raise RuntimeError('Inputs or placement settings changed for timing runs')
    for field in ('pages', 'lines', 'glyphs', 'marks'):
        if baseline['statistics'][field] != prototype['statistics'][field]:
            raise RuntimeError(f'Corpus mismatch: {field}')
    findings = {m: rows(output/m) for m in METHODS}
    keyed = {m: {key(r): r for r in findings[m]} for m in METHODS}
    collision = {m: [r for r in findings[m] if r['type'] == 'GenericGap'] for m in METHODS}
    intersection = {m: [r for r in collision[m] if r['detail'].startswith('Ink intersection')] for m in METHODS}
    metric = {m: {'runs': [r['seconds'] for r in timings[m]],
                  'mean': statistics.mean(r['seconds'] for r in timings[m]),
                  'placementMean': statistics.mean(r['placementSeconds'] for r in timings[m])}
              for m in METHODS if timings[m]}
    summary = {'pages': baseline['statistics']['pages'], 'placementAudit': args.placement_audit, 'timings': metric,
               'statistics': {m: audits[m]['statistics'] for m in METHODS},
               'cache': prototype['noFitPolygons'],
               'collisionCounts': {m: len(collision[m]) for m in METHODS},
               'inkIntersections': {m: len(intersection[m]) for m in METHODS},
               'introducedComparedWithGjk': [r for k, r in keyed['nfp'].items() if k not in keyed['gjk']],
               'removedComparedWithGjk': [r for k, r in keyed['gjk'].items() if k not in keyed['nfp']],
               'changedSeverity': [{'key': k, 'gjk': float(keyed['gjk'][k]['report_severity']),
                                     'nfp': float(keyed['nfp'][k]['report_severity'])}
                                   for k in sorted(keyed['gjk'].keys() & keyed['nfp'].keys())
                                   if abs(float(keyed['gjk'][k]['report_severity'])-float(keyed['nfp'][k]['report_severity'])) > .5],
               'cases': args.cases,
               'note': 'Same XPBD constraints, mobility, compliance, iterations and gaps. Only generic-gap contact geometry changes. Dedicated stacking/cohesion constraints keep their existing geometry. NFP uses buildPolyFromCubics for every glyph and the existing filled-hole policy; single-convex pairs share the GJK/EPA contact routine; BaseAssociation is a read-only placement audit, not a solver constraint; clearance arcs use the requested gap without an extra allowance.'}
    (output/'comparison.json').write_text(json.dumps(summary, ensure_ascii=False, indent=2)+'\n')
    table = []
    def add(label, values):
        table.append('<tr><th>'+html.escape(label)+'</th>'+''.join('<td>'+html.escape(str(values[m]))+'</td>' for m in METHODS)+'</tr>')
    add('Corpus pages', {m: audits[m]['statistics']['pages'] for m in METHODS})
    if metric:
        add('Mean elapsed — PDF/report disabled', {m: f"{metric[m]['mean']:.2f} s" for m in METHODS})
        add('Mean placement time — PDF/report disabled', {m: f"{metric[m]['placementMean']:.2f} s" for m in METHODS})
        add('Elapsed timing runs', {m: ', '.join(f'{r:.2f}' for r in metric[m]['runs'])+' s' for m in METHODS})
    add('Save Collision findings (clearance 10 unless configured otherwise)', summary['collisionCounts'])
    add('Ink intersections within those findings', summary['inkIntersections'])
    for family in sorted(set(baseline['statistics']['byType']) | set(prototype['statistics']['byType'])):
        if family != 'GenericGap':
            label = 'BaseAssociation (placement audit)' if family == 'BaseAssociation' else family+' findings'
            add(label, {m: audits[m]['statistics']['byType'].get(family, 0) for m in METHODS})
    for role in ('dots', 'waqf', 'other-marks'):
        add(role+' mean movement', {m: f"{audits[m]['statistics']['movement'][role]['totalDistance']/max(1,audits[m]['statistics']['movement'][role]['count']):.2f} units" for m in METHODS})
    payload = {m: json.loads((output/m/'mushaf.review.json').read_text()) for m in METHODS}
    data = json.dumps(payload, ensure_ascii=False).replace('<', '\\u003c')
    clearance = baseline['options']['xpbd']['collisionReportMinGap']
    page = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>XPBD contacts: GJK/EPA vs no-fit polygons</title><style>
body{font:15px system-ui;background:#f4f6f5;color:#17291f;margin:28px}h1{font-size:25px}table{border-collapse:collapse;background:white}th,td{padding:7px 14px;border:1px solid #d6dfda;text-align:left}.case{background:white;padding:18px;border-radius:12px;margin:18px 0}.pair{display:grid;grid-template-columns:1fr 1fr;gap:20px}.pane svg{width:100%;height:240px}a{color:#1769a0}dialog{width:90vw;max-width:1500px}dialog svg{width:100%;height:70vh}.note{max-width:1050px;line-height:1.5}@media(max-width:700px){.pair{grid-template-columns:1fr}}
</style><h1>XPBD: GJK/EPA vs cached no-fit polygons</h1><p class="note">Same font, layout and XPBD settings. This compares generic-gap contact geometry, not a new placement solver. Audits use the unchanged Save Collision detector at CLEARANCE units after integer output rounding. Timing runs are serialized with fresh caches, PDF/report disabled. Blue: waqf; purple: dots; gray: surrounding ink. Hover over a glyph for its displacement; click an image to enlarge.</p>
<table><thead><tr><th>Metric</th><th>GJK/EPA (current)</th><th>NFP (prototype)</th></tr></thead><tbody>TABLE</tbody></table>
<p><a href="gjk/mushaf.pdf">GJK Mushaf PDF</a> · <a href="nfp/mushaf.pdf">NFP Mushaf PDF</a> · <a href="gjk/mushaf_violations.html">GJK findings</a> · <a href="nfp/mushaf_violations.html">NFP findings</a> · <a href="comparison.json">Comparison data</a></p>
<p class="note">Placement audit (side, classification and base association): AUDIT_STATE for both methods. This optional diagnostic does not apply a solver force.</p>
<p class="note">Cache statistics: CACHE. NFP inputs use buildPolyFromCubics for every glyph, including bases. Single-convex pairs use the same GJK/EPA routine as the baseline. Minkowski construction merges convex edge sequences; clearance has no extra allowance. The cached regions retain concavities and component gaps; original outline holes follow the existing filled-hole policy. Pair separation does not guarantee a globally collision-free layout.</p><div id="cases"></div><dialog id="detail"><button onclick="this.closest('dialog').close()">Close</button><div id="enlarged"></div></dialog>
<script type="application/json" id="data">DATA</script><script>SCRIPT</script></html>'''
    script = r'''
const data=JSON.parse(document.getElementById('data').textContent), ns='http://www.w3.org/2000/svg';
const sets=Object.fromEntries(Object.entries(data).map(([b,v])=>[b,new Map(v.words.map(w=>[[w.page,w.line,w.word].join(':'),w]))]));
function render(word,bounds){const svg=document.createElementNS(ns,'svg');svg.setAttribute('viewBox',`${bounds[0]} ${-bounds[3]} ${bounds[2]-bounds[0]} ${bounds[3]-bounds[1]}`);
for(const g of word.glyphs){const p=document.createElementNS(ns,'path');p.setAttribute('d',g.polygons.filter(p=>p.length).map(p=>'M'+p.map(v=>v[0]+' '+(-v[1])).join('L')+'Z').join(''));const target=g.line===word.line&&g.word===word.word;p.setAttribute('fill',target?(g.name.includes('waqf')?'#1769a0':g.name.includes('dot')?'#843caf':'#17291f'):'#adb7b0');const title=document.createElementNS(ns,'title');title.textContent=`${g.name} · line ${g.line} · word ${g.word} · dx ${g.dx.toFixed(2)} · dy ${g.dy.toFixed(2)}`;p.append(title);svg.append(p);}return svg;}
for(const key of new Set([...sets.gjk.keys(),...sets.nfp.keys()])){const words=['gjk','nfp'].map(b=>sets[b].get(key));const glyphs=words.flatMap(w=>w?.glyphs||[]);const target=words.flatMap(w=>w?.glyphs.filter(g=>g.line===w.line&&g.word===w.word)||[]);const section=document.createElement('section');section.className='case';const h=document.createElement('h3');const [page,line,word]=key.split(':');h.textContent=`Page ${page} · Line ${line} · Word ${word}`;section.append(h);document.getElementById('cases').append(section);if(!target.length){section.append('Requested word not found.');continue;}const bounds=[Math.min(...target.map(g=>g.box[0]))-400,Math.min(...glyphs.map(g=>g.box[1]))-100,Math.max(...target.map(g=>g.box[2]))+400,Math.max(...glyphs.map(g=>g.box[3]))+100];const pair=document.createElement('div');pair.className='pair';section.append(pair);['gjk','nfp'].forEach((b,i)=>{const pane=document.createElement('div');const label=document.createElement('h4');label.textContent=b==='gjk'?'GJK/EPA':'NFP';pane.className='pane';pane.append(label);const svg=render(words[i]||{glyphs:[]},bounds);svg.style.cursor='zoom-in';svg.onclick=()=>{document.getElementById('enlarged').replaceChildren(render(words[i],bounds));document.getElementById('detail').showModal()};pane.append(svg);pair.append(pane);});}
'''
    cache = summary['cache']
    cache_text = f"{cache['convexGjkQueries']} shared convex GJK/EPA contacts; {cache['shapes']} shapes; {cache['builds']} builds; {cache['hits']} hits; {cache['evictions']} evictions; {cache['buildSeconds']:.2f} s construction and {cache['querySeconds']:.2f} s queries (audit run)"
    page = page.replace('AUDIT_STATE', 'enabled' if args.placement_audit else 'disabled').replace('TABLE', ''.join(table)).replace('CLEARANCE', str(clearance)).replace('CACHE', html.escape(cache_text)).replace('DATA', data).replace('SCRIPT', script)
    if not args.pdf:
        page = page.replace('<a href="gjk/mushaf.pdf">GJK Mushaf PDF</a> · <a href="nfp/mushaf.pdf">NFP Mushaf PDF</a> · ', '')
    (output/'comparison.html').write_text(page)
    print(json.dumps({'timings': metric, 'collisions': summary['collisionCounts'], 'intersections': summary['inkIntersections'], 'cache': cache}, indent=2), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--executable', type=Path, required=True)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    parser.add_argument('--pages', default='1-604')
    parser.add_argument('--timing-runs', type=int, default=3)
    parser.add_argument('--nfp-cache-limit', type=int, default=16384,
                        help='Pair-region cache capacity for this comparison (default 16384)')
    parser.add_argument('--case', dest='cases', action='append')
    parser.add_argument('--pdf', action='store_true', help='Generate both complete Mushaf PDFs')
    parser.add_argument('--placement-audit', action='store_true',
                        help='Enable optional side/classification/base association diagnostics (default off)')
    parser.add_argument('--reuse-audits', action='store_true')
    args = parser.parse_args()
    if not 1 <= args.nfp_cache_limit <= 1000000:
        parser.error('--nfp-cache-limit must be between 1 and 1000000')
    if args.timing_runs < 0:
        parser.error('--timing-runs must be nonnegative')
    for field in ('executable', 'config', 'output_dir'):
        setattr(args, field, getattr(args, field).resolve())
    args.cases = args.cases or CASES
    if args.reuse_audits:
        audits = {m: json.loads((args.output_dir/m/'mushaf.run.json').read_text()) for m in METHODS}
        if any(not v['complete'] for v in audits.values()):
            raise RuntimeError('Cannot reuse incomplete audit')
    else:
        audits = {m: execute(args, m, args.output_dir/m) for m in METHODS}
    timings = {m: [] for m in METHODS}
    for run in range(args.timing_runs):
        # Alternate order to reduce systematic warmup/thermal bias.
        for m in METHODS if run % 2 == 0 else reversed(METHODS):
            timings[m].append(execute(args, m, args.output_dir/'timing'/f'{m}-{run+1}', timing=True))
    write_comparison(args, audits, timings)


if __name__ == '__main__':
    main()
