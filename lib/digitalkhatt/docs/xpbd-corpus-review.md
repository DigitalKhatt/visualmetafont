# XPBD whole-corpus review

**Historical decomposed-mark experiment:** this report describes the temporary
mark-decomposition version. That change has been reverted after the font author
clarified the intended component-hull collision policy. The original claim that
the unsplit mark geometry was incorrect was mistaken. Current results are in
[the restored hull-policy review](xpbd-hull-policy-review.md).

## Run and scope

The completed native run used Old Madina, `qpc_v1_layout`, the declarative
justification engine, `font-size-xscale`, standard shrink, line spacing 1683,
text width 16400, em scale 1 and Tajweed coloring. The saved editor lookup and
XPBD settings were reproduced, with Force explicitly enabled. PDF generation
is enabled by default; violation reports are optional.

The resolved configuration, source and executable fingerprints, completed run
manifest and CSV audit are in `output/pdf/xpbd-corpus/final/` relative to the
workspace root. The run took approximately 56 seconds. Reproduce it with:

```sh
build/mushaf-cli/visualmetafont/lib/digitalkhatt/digitalkhatt_generate_mushaf \
  --config output/pdf/xpbd-corpus/final/mushaf.settings.json \
  --output mushaf.pdf
```

The snapshot has Force and reporting enabled. For a fresh run with library
defaults, provide the font and add `--force --report` as needed. The editor
now exports its live configuration when Generate Mushaf is used. See
[the CLI guide](generate-mushaf.md) for build instructions and options.

| Coverage | Count |
| --- | ---: |
| Quran pages | 604 |
| Source lines | 9,046 |
| Glyph instances in placement pipeline | 848,298 |
| Mark instances | 435,360 |
| Surah headers | 114 |
| Basmala lines | 113 |
| Sajda start/end pairs | 15 |

Decorative surah icons are rendered separately and are excluded from the
placement glyph count. The Mushaf PDF contains 605 pages including its notice;
the overview report contains 604 pages and the capped summary contains 167.

## Findings after the safety changes

| Finding | Final count | Interpretation |
| --- | ---: | --- |
| BaseAssociation | 0 | No final mark crossed the audited neighboring-base ownership region |
| MarkSide | 3 | Existing contextual dot flags; none introduced or worsened |
| MarkClassification | 277 | All are `hamzaabove.lamalef`, without a declared top/bottom audit role |
| PlacementRejected | 33 | Unsafe candidate contacts were rejected and affected marks restored |
| GenericGap: ink intersection | 9 | All present in the shaped page; none introduced or worsened |
| GenericGap: minimum gap not reached | 71,478 | Desired clearance remains unresolved; distinct from ink intersection |
| BaseVicinity | 328 | Residual preferred vicinity constraints |
| HorizontalOrder | 136 | Residual order constraints |
| StackOrderGap | 291 | Residual stack separation constraints |
| WaqfPlacement | 733 | Residual pause-sign placement constraints |

There are 73,288 finding rows: 72,978 classified as hard and 310 as soft by the
current report rules. No invalid owner or non-finite placement diagnostics were
found. The 33 rejection events affect 30 pages; rejected candidate geometry is
not exported. Restoring safe positions can leave other constraints unresolved.

The report marks 10,216 rows as introduced and 2,433 as worsened relative to the
shaped baseline and report cutoff. These are mostly clearance residuals, rather
than new ink intersections. The generic gap finding count decreased from
186,116 in the shaped baseline to 71,487 after XPBD. The solver therefore
improves many gaps, but it does not satisfy every configured constraint.

### Remaining ink intersections

These are conservative geometric flags, checked on the actual integer offsets
used by the PDF. Page and line numbers refer to Quran pages, excluding the notice.

| Page | Lines | Glyph pair | Final / shaped penetration, font units |
| --- | --- | --- | ---: |
| 21 | 7 / 8 | `ain.fina` / `waqf.sad` | 54.394 / 69.303 |
| 30 | 13 / 14 | `meem.fina.ii` / `waqf.qaf` | 34.032 / 57.540 |
| 107 | 3 / 4 | `meem.fina.ii` / `waqf.qaf` | 4.779 / 4.779 |
| 108 | 2 / 3 | `meem.fina.ii` / `waqf.jeem` | 38.303 / 38.303 |
| 206 | 2 / 3 | `meem.fina.ii` / `waqf.jeem` | 36.990 / 36.990 |
| 271 | 7 / 8 | `endofaya36` / `waqf.sad` | 0.587 / 0.587 |
| 406 | 7 / 8 | `meem.fina.ii` / `waqf.jeem` | 61.218 / 61.218 |
| 453 | 5 / 5 | `fatha` / `lam.medi.laf` | 62.885 / 62.885 |
| 577 | 2 / 3 | `smalllowmeem` / `waqf.sad` | 52.766 / 52.766 |

The CSV severity for these pairs includes the 80-unit gap target. It must not
be read directly as penetration depth. Most remaining pairs involve a pause
sign and ink on the neighboring line. Enlarged rendered contexts were inspected
for these pages, as well as the classification and side examples.

### Contextual mark roles

The three side flags are `onedotdown` attached to `hah.init.beforeyeh` in
`يُزْجِي` (page 288 line 14 and page 355 line 12) and `تُرْجِي` (page 425 line 1).
The dot sits beneath the jeem head, but its center is above the generic reference
derived from the whole base. These look like a contextual audit-policy mismatch;
they should receive an explicit contextual side rule before any anchor changes.

All 277 unclassified marks are `hamzaabove.lamalef`. The font deliberately
excludes this glyph from its general top-mark class. Adding it to that class
globally could change shaping and placement. Give anchored components separate
semantic audit metadata instead, and retain the existing shaping classes.

## Changes applied

- Shared the GUI corpus assembly, layout widths, normalization, sajda detection
  and placement adapter with the native command. QPC v1, v2, v4 and IndoPak source
  pages match the original GUI assembly exactly.
- Added a Qt-free vector PDF writer, streaming overview/CSV reports and a
  bounded summary. Added searchable source text, ayah ornaments, surah artwork,
  Tajweed colors, bookmarks, page labels and a GUI-style notice.
- Exported the GUI's current settings and added resolved configuration and
  completed-run manifests to the CLI. Reports include baseline comparisons,
  words, clusters, owner information and restoration events.
- Temporarily decomposed mark outlines before GJK; this changed the intended
  collision policy and has since been reverted. Corrected placement scaling and
  audited the rounded offsets actually exported.
- Added default-on semantic bounds preserving a mark's original side and
  neighboring-base region, respecting the existing contextual exceptions.
- Added a final collision fallback: newly intersecting or materially deeper
  candidate contacts restore the affected marks to shaped positions. Contact
  checks repeat after restoration so one restoration cannot introduce another
  undetected contact.
- Added independent placement diagnostics and solver convergence checks that
  include hard residuals and applied movement. Added regression coverage for
  semantic bounds, rejected contacts, report data and corpus assembly.

## Recommended improvements

1. **Pause signs and adjacent lines.** Replace fixed vertical targets with bounds
   derived from both neighboring lines' actual ink. Most remaining physical
   contact flags involve cross-line pause signs. Keep semantic bounds and the
   final fallback while improving feasibility; increasing the iteration count
   alone did not resolve the observed conflicting proposals.
2. **Explicit semantic metadata.** Declare contextual roles for lam-alef hamza,
   embedded dots and manual bowl placements independently of GSUB/GPOS classes.
   This will make side and ownership checks more meaningful without changing
   the font's shaping behavior.
3. **Effective parameters in the GUI.** `minGapBody`, `minGapMark` and maximum
   shifts control broadphase padding rather than actual gap targets or movement
   limits. `attachStrength`, `smoothStrength` and `sepOvershoot` are not consumed
   by current XPBD. Label the padding controls accurately and wire or remove
   inactive controls. Expose the effective generic gap
   target, currently 80 with named 40/10-unit exceptions, through shared options.
4. **Separate collisions from preferences.** Make actual ink contact the first
   report category. Label desired clearance and compliant placement residuals
   explicitly; thousands of hard-colored clearance rows currently obscure the
   small set requiring visual review. Include gap target and penetration as
   separate CSV values, and retain rejected candidate geometry for inspection.
5. **Constraint priorities and contact updates.** Resolve incompatible lane,
   waqf, order and clearance proposals using explicit priorities and refresh
   contact normals when features change. Preserve restoration events as evidence
   of solver conflicts rather than hiding them by increasing report thresholds.
6. **Contour-aware final audit.** Convex decomposition conservatively fills
   holes. Preserve original contours for a hole-aware final intersection check
   to distinguish genuine filled-ink intersections from approximation artifacts.
7. **Repeatable corpus checks.** Add an optional full-corpus CI job asserting
   completion, coverage, no new side/ownership warnings, no new ink contacts,
   source parity and PDF metadata. Exercise multiple fonts and layout profiles;
   the completed placement review here uses Old Madina and QPC v1.

## Verification and limits

The native build and its font plugin have no Qt runtime dependency. The GUI
build also succeeded. All seven registered native layout tests passed; the
report writer checks passed, including CSV escaping, summary ordering and
empty reports. The corpus assembly comparison covers all four bundled layouts
(604 pages in each QPC layout, 610 in IndoPak).

All 604 final Quran pages rendered successfully. Contact sheets covering every
page and enlarged contexts for the critical findings were inspected. PDF checks
confirmed 9,046 searchable source lines, 115 bookmarks, page labels, all 15 sajda
pairs and the expected page counts. The first two native pages were also compared
visually with the original GUI export; rendering is not asserted to be identical
pixel for pixel.

The opening notice initially had a corrupt compressed content stream because
its URL annotation was written before the stream closed. The annotation now
follows stream closure, matching the GUI writer. The complete export was
regenerated. `tests/layout/check-mushaf-pdf.py` checks the notice text, URL link,
all page content streams and rendered opening/ending samples, and rejects any
Poppler syntax warning. It rejects the earlier broken export and passes the fix.

This is an automated review of every placement plus visual checks of page
coverage and critical contexts. It is not a word-by-word human Quran proofread
or proof of linguistic mark ownership. Intentional pair exclusions follow the
existing solver policy; original shaped errors can remain. The nine contact
flags, three contextual side flags and 277 missing role declarations are retained
in the delivered report for review.
