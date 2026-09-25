# Justification golden snapshots

`*.snapshot` records what one justification engine produced for the whole
qpc_v1 Mushaf: one row per rendered line, with two digests over its glyphs.

## Why these exist

The whole-Mushaf gate

```bash
digitalkhatt_compare_mushaf_shaping --same-provider --fail-on-difference \
    oldmadinafont/oldmadina.mp oldmadinafont/output/oldmadina.otf
```

compares the DeclPolicy engine against Experimental2. That answers "did this
change relative to the reference", which is the right question only while the
reference exists.  Experimental2 is meant to be retired, and the moment it goes
that gate goes with it.

A snapshot answers the same question against a recorded baseline instead, so
the net survives the changeover — and a deliberate change arrives as a
reviewable diff rather than as a count of differing glyphs.

`decl-policy.snapshot` and `experimental2.snapshot` are byte-identical below
their headers.  That is the two engines agreeing, stated in the format that
will outlive one of them.

## Using them

Check the current build against the baseline:

```bash
digitalkhatt_compare_mushaf_shaping --check-snapshot \
    visualmetafont/lib/digitalkhatt/tests/declpolicy/golden/decl-policy.snapshot \
    --fail-on-difference \
    oldmadinafont/oldmadina.mp oldmadinafont/output/oldmadina.otf
```

Exit status 3 means lines differ; each is named on stdout and written to the
CSV report.  **Regenerate `oldmadina.otf` first** — a stale font shifts every
GSUB lookup index and every line will differ:

```bash
cd oldmadinafont && ../build/<config>/visualmetafont/lib/digitalkhatt/Release/digitalkhatt_generate_opentype \
    -o output/oldmadina.otf oldmadina.mp
```

Re-record after an intended change, and review the diff:

```bash
digitalkhatt_compare_mushaf_shaping --snapshot \
    visualmetafont/lib/digitalkhatt/tests/declpolicy/golden/decl-policy.snapshot \
    oldmadinafont/oldmadina.mp oldmadinafont/output/oldmadina.otf
```

`--snapshot-engine experimental2` records the other engine; `--pages A-B`
limits the range.

## Format

```
# digitalkhatt-justification-snapshot 1
# engine=decl-policy pages=1-604 glyphs=no
page,line,glyphs,shape,metrics,xscale,fontsize,width
1,1,23,819d3ad9dd1ab581,3051805886da2cfb,1,1.02,3767
```

Two digests rather than one, because the two kinds of regression read
differently:

- `shape` — FNV-1a over each glyph's codepoint, cluster and left/right tatweel:
  *which* glyphs justification chose and how far it stretched them.
- `metrics` — over x_advance, x_offset and y_offset: where they ended up.

A line whose `shape` holds but whose `metrics` moved is a positioning change; a
line whose `shape` moved is a different justification decision.

`--snapshot-glyphs` additionally writes a `g,` row per glyph with its name,
tatweels, advance, offsets and cluster.  The check never decides anything from
those rows — the digests have already decided — it uses them only to print what
moved.  They multiply the file size by roughly sixty, so keep detail snapshots
out of version control and generate them per page range when investigating.

## Regenerating is not the same as fixing

A snapshot is only as good as the run that produced it.  Re-record when you
*meant* to change the output, and say in the commit message what changed and
why.  Re-recording to make a red gate green is how a golden file stops meaning
anything.
