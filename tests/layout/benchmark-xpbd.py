"""Build isolated before/current XPBD cores and benchmark identical shaped pages.

Usage: python3 benchmark-xpbd.py BUILD_DIR CONFIG_JSON OUTPUT_DIR [--rounds 4]
The historical core comes from --before-ref (default HEAD). Production sources
are never swapped. Both cores use the build's Release compiler flags and shared
unchanged geometry implementation, in separate namespaces. Reports/PDFs are off.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import shlex
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('config', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--before-ref', default='HEAD')
    parser.add_argument('--rounds', type=int, default=4)
    args = parser.parse_args()
    repo = Path(__file__).resolve().parents[2]
    build, output = args.build.resolve(), args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    commands = subprocess.check_output(['ninja', '-t', 'commands', 'digitalkhatt_generate_mushaf'], cwd=build, text=True).splitlines()
    compile_command = next(c for c in commands if ' -c ' in c and c.endswith('/tools/generate_mushaf.cpp'))
    flags = shlex.split(compile_command)
    for flag in ('-o', '-c', '-MT', '-MF'):
        if flag not in flags:
            continue
        index = flags.index(flag)
        del flags[index:index + 2]
    flags = [flag for flag in flags if flag != '-MD']
    flags += ['-I' + str(output / 'include'), '-I' + str(repo / 'lib/digitalkhatt/tools')]
    old_paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', args.before_ref,
        'lib/digitalkhatt/include/digitalkhatt/layout', 'lib/digitalkhatt/src/layout'], cwd=repo, text=True).splitlines()
    current_paths = [str(p.relative_to(repo)) for root in ('include/digitalkhatt/layout', 'src/layout')
                     for p in (repo / 'lib/digitalkhatt' / root).rglob('*') if p.suffix in ('.h', '.cpp')]
    objects = []
    jobs = []
    for version, paths in [('before', old_paths), ('after', current_paths)]:
        namespace = 'bench_' + version
        for path in paths:
            text = (subprocess.check_output(['git', 'show', args.before_ref + ':' + path], cwd=repo, text=True)
                    if version == 'before' else (repo / path).read_text())
            text = text.replace('digitalkhatt::layout', 'digitalkhatt::' + namespace)
            text = text.replace('digitalkhatt/layout/', 'digitalkhatt/' + namespace + '/')
            relative = Path(path.removeprefix('lib/digitalkhatt/'))
            target = output / str(relative).replace('digitalkhatt/layout/', 'digitalkhatt/' + namespace + '/').replace('src/layout/', 'src/' + namespace + '/')
            target.parent.mkdir(parents=True, exist_ok=True)
            if relative.name == 'OptimizeLayout.cpp':
                text = text.replace('for (int iter = 0; iter < P.maxIters; ++iter) {',
                    'for (int iter = 0; iter < P.maxIters; ++iter) { ++benchStats.iterations;')
                text = text.replace('double maxPenetration = 0.0;', 'benchStats.pairs += pairs.size();\n    double maxPenetration = 0.0;')
                if version == 'after':
                    text = text.replace('      std::vector<ConstraintViolation> remaining;',
                        '      ++benchStats.convergenceChecks; const auto convergenceStart = BenchClock::now();\n      std::vector<ConstraintViolation> remaining;')
                    text = text.replace('      if (!hardBoundUnresolved) break;',
                        '      benchStats.convergence += secondsSince(convergenceStart);\n      if (!hardBoundUnresolved) break;')
                text = '#include "benchstats.h"\n' + text
            target.write_text(text)
            if target.suffix == '.cpp':
                obj = target.with_suffix('.o')
                jobs.append((target, obj))
                objects.append(str(obj))
    (output / 'include/benchstats.h').write_text('''#pragma once
#include <chrono>
using BenchClock = std::chrono::steady_clock;
inline double secondsSince(BenchClock::time_point t) { return std::chrono::duration<double>(BenchClock::now()-t).count(); }
struct BenchStats { unsigned long long iterations=0,pairs=0,convergenceChecks=0,fallbackPasses=0; double safetySetup=0,safetyProjection=0,convergence=0,finalSafety=0; };
namespace digitalkhatt::bench_before { inline BenchStats benchStats; }
namespace digitalkhatt::bench_after { inline BenchStats benchStats; }
''')
    def compile_one(job):
        source, obj = job
        subprocess.run(flags + ['-c', str(source), '-o', str(obj)], cwd=build, check=True)
    with ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(compile_one, jobs))
    main_source = Path(__file__).with_name('benchmark-xpbd.cpp')
    main_obj = output / 'main.o'
    compile_one((main_source, main_obj))
    link = shlex.split(commands[-1])[2:-2]
    link = [str(main_obj) if x.endswith('/tools/generate_mushaf.cpp.o') else x for x in link]
    binary = output / 'benchmark-xpbd'
    link[link.index('-o') + 1] = str(binary)
    link += objects
    subprocess.run(link, cwd=build, check=True)
    metadata = {'beforeRef': subprocess.check_output(['git', 'rev-parse', args.before_ref], cwd=repo, text=True).strip(),
                'compilerFlags': flags, 'rounds': args.rounds, 'reports': False, 'pdf': False}
    (output / 'build.json').write_text(json.dumps(metadata, indent=2) + '\n')
    subprocess.run([str(binary), str(args.config.resolve()), str(output), str(args.rounds)], check=True)


if __name__ == '__main__':
    main()
