"""Real CLI regression, using temporary copies of the approved font source."""
import argparse, hashlib, json, re, shutil, struct, subprocess, tempfile
from pathlib import Path


def tables(path):
    data = path.read_bytes()
    count, = struct.unpack_from('>H', data, 4)
    result = {}
    for i in range(count):
        tag, checksum, offset, size = struct.unpack_from('>4sIII', data, 12 + i*16)
        value = bytearray(data[offset:offset+size])
        if tag == b'head':
            value[8:12] = b'\0'*4       # checksumAdjustment
            value[20:36] = b'\0'*16     # creation/modification timestamps
        result[tag] = bytes(value)
    return result


def mapped(path, cp):
    cmap = tables(path)[b'cmap']
    for i in range(struct.unpack_from('>H', cmap, 2)[0]):
        offset, = struct.unpack_from('>I', cmap, 8 + i*8)
        if struct.unpack_from('>H', cmap, offset)[0] != 4:
            continue
        count = struct.unpack_from('>H', cmap, offset+6)[0]//2
        end = offset+14; start = end+count*2+2
        delta = start+count*2; ranges = delta+count*2
        for j in range(count):
            a = struct.unpack_from('>H', cmap, start+j*2)[0]
            b = struct.unpack_from('>H', cmap, end+j*2)[0]
            if a <= cp <= b:
                shift = struct.unpack_from('>h', cmap, delta+j*2)[0]
                ro = struct.unpack_from('>H', cmap, ranges+j*2)[0]
                if not ro:
                    return bool((cp+shift)&65535)
                glyph = struct.unpack_from('>H', cmap, ranges+j*2+ro+(cp-a)*2)[0]
                return bool(glyph and (glyph+shift)&65535)
    return False


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--exporter', type=Path, required=True)
    p.add_argument('--mushaf', type=Path, required=True)
    p.add_argument('--font', type=Path, required=True)
    p.add_argument('--pages', default='1-2')
    p.add_argument('--artifacts', type=Path)
    args = p.parse_args()
    args.exporter = args.exporter.resolve(); args.mushaf = args.mushaf.resolve(); args.font = args.font.resolve()
    with tempfile.TemporaryDirectory(prefix='dk-cli-source-test-') as temp:
        tmp = Path(temp); project = tmp/'project'; project.mkdir()
        out = args.artifacts.resolve() if args.artifacts else tmp/'results'
        out.mkdir(parents=True, exist_ok=True)
        original = args.font.parent
        source_hash = hashlib.sha256((original/'glyphs.mp').read_bytes()).hexdigest()
        for file in original.iterdir():
            if file.is_file(): shutil.copy2(file, project/file.name)
        meta = json.loads((project/'font-project.json').read_text())
        raw = b'\n'.join((original/name).read_bytes() for name in meta.get('glyphFiles', ['glyphs.mp']))
        override_source = tmp/'combined.mp'
        override_source.write_bytes(raw)
        markers = list(re.finditer(rb'(?m)^(?:beginchar|defchar)\(', raw))
        assert len(markers) > 2
        cut = markers[len(markers)//2].start()
        (project/'glyphs').mkdir()
        (project/'glyphs/first.mp').write_bytes(b'% First file; defchar(commented,1,0,0,0);enddefchar;\n'+raw[:cut])
        (project/'glyphs/second.mp').write_bytes(b'% Second file\n'+raw[cut:])
        meta['glyphFiles'] = ['glyphs/first.mp', 'glyphs/second.mp']
        manifest = project/'font-project.json'
        def save_meta(): manifest.write_text(json.dumps(meta, ensure_ascii=False, indent=2)+'\n')
        save_meta()
        # There is no legacy file to accidentally fall back to.
        (project/'glyphs.mp').unlink()
        def run(command, name, expected_error=None):
            result = subprocess.run([str(x) for x in command], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            (out/(name+'.log')).write_text(result.stdout)
            if expected_error:
                assert result.returncode != 0 and expected_error in result.stdout, result.stdout
            else:
                assert result.returncode == 0, result.stdout
            return result
        def export(font, target, *extra):
            run([args.exporter, '--output', out/target, *extra, font], target)
        export(args.font, 'single.otf')
        export(project/args.font.name, 'multiple.otf')
        assert tables(out/'single.otf') == tables(out/'multiple.otf'), 'Single/multi OTF table difference'
        shared = ['--layout', 'qpc_v2_layout', '--justifier', 'experimental', '--style', 'xscale', '--shrink', 'none', '--no-force', '--no-report', '--no-pdf', '--no-notice', '--no-tajweed', '--pages', args.pages]
        for name, font in [('single', args.font), ('multiple', project/args.font.name)]:
            run([args.mushaf, '--font', font, *shared, '--layout-binary', out/(name+'.bin'), '--output', out/(name+'.pdf')], name+'-corpus')
        for suffix in ['.bin', '.bin.json']:
            assert (out/('single'+suffix)).read_bytes() == (out/('multiple'+suffix)).read_bytes(), 'Corpus difference: '+suffix
        sources = json.loads((out/'multiple.run.json').read_text())['sources']
        assert all(str(project/name) in sources for name in meta['glyphFiles'])
        assert str(manifest) in sources, 'Manifest missing from provenance'
        extra_code = next(cp for cp in [0x067e, *range(0xe000, 0xf900)] if not mapped(out/'single.otf', cp))
        (project/'glyphs/extra.mp').write_text(f'beginchar(project.cliGlyph,{extra_code},500,700,0);\nfill (0,0)--(100,0)--(100,100)--cycle;\nendchar;\n')
        meta['glyphFiles'].append('glyphs/extra.mp'); save_meta()
        export(project/args.font.name, 'additional.otf')
        assert mapped(out/'additional.otf', extra_code), 'Additional-file glyph is missing from exported cmap'
        meta['glyphFiles'] = ['glyphs/missing.mp']; save_meta()
        export(project/args.font.name, 'override.otf', '--glyphs', override_source)
        assert tables(out/'single.otf') == tables(out/'override.otf'), 'Explicit override differs'
        run([args.exporter, '--output', out/'bad.otf', project/args.font.name], 'missing-file', 'missing.mp')
        meta['glyphFiles'] = ['glyphs/first.mp', 'glyphs/first.mp']; save_meta()
        run([args.exporter, '--output', out/'bad.otf', project/args.font.name], 'duplicate-path', 'duplicate glyph source path')
        meta['glyphFiles'] = ['glyphs/first.mp', 'glyphs/duplicate.mp']; save_meta()
        shutil.copyfile(project/'glyphs/first.mp', project/'glyphs/duplicate.mp')
        run([args.exporter, '--output', out/'bad.otf', project/args.font.name], 'duplicate-glyph', 'Duplicate glyph')
        assert hashlib.sha256((original/'glyphs.mp').read_bytes()).hexdigest() == source_hash
        summary = dict(pages=args.pages, otfTablesIdentical=True, liveBinaryIdentical=True, liveSidecarIdentical=True, nestedFileHashesRecorded=True, additionalGlyphExported=f'U+{extra_code:04X}', overrideAndErrorsPassed=True)
        (out/'summary.json').write_text(json.dumps(summary, indent=2)+'\n')
        print('PASS CLI multi-file project:', json.dumps(summary))
if __name__ == '__main__': main()
