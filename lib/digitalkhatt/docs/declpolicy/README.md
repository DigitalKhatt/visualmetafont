# Declarative-policy justification engine

The declarative engine is selected with `JustType::DeclPolicy`. It is
implemented in DigitalKhatt; HarfBuzz is used only to shape and measure trial
results.  The existing `Experimental2` implementation remains available as a
reference.

The data flow is:

1. Shape a word normally to obtain post-GSUB glyph ids, names, GDEF classes,
   forms, and source clusters.
2. Convert base glyphs to font-specific facts.  For oldmadina these include
   facts such as `BehFirst`, `FinalAscendant`, and `AlternateGroup1`.
3. Run the compiled fixed-slot DFA independently on every joining subword.
4. Filter and order the resulting opportunities using the Experimental2 policy
   table.
5. Ask the action backend to apply one candidate transactionally.  The current
   compatibility backend writes bounded `cv01`, `cv02`, `cv03`, and
   `cv10`-`cv19` values, reshapes the affected word, and commits only if it
   improves the line without crossing its target width.
6. Shape the final line normally, including GPOS.

The authoring copy of the policy catalog now lives in
`oldmadinafont/features.fea` as `table(justdfa) oldmadina.experimental2`:

```fea
table(justdfa) oldmadina.experimental2 {
  linepolicy { stretch fixed_steps; shrink standard; }
  facts {
    fact BehFirst form medi chars ["بتثنيسشصض"];
    fact BehSecond glyphs [behshape.medi];
  }
  opportunities {
    opportunity BehKashida {
      match [BehFirst BehSecond];
      prepare {
        add $1:cv01 1 clamp 6;
        update $2:cv02 = prod(2, $1:cv01);
      }
    }
  }
  selections {
    selection Beh {
      choose BehKashida first selected_only;
      traversal last_first;
      record Beh;
      forbid_recorded [FinalAscendant OtherKashidas];
    }
  }
  stages {
    stage FixedStretch {
      phase Beh 2;
    }
  }
  stretchpolicy fixed_steps {
    stage FixedStretch;
  }
  shrinkpolicy standard {
    fit_features [sk01 sk02];
    balance;
  }
};
```

The sections have intentionally separate responsibilities:

- `facts` maps font-specific glyph forms and source characters to predicates;
- optional `patterns` shares fixed-slot recognition across opportunities;
- `opportunities` associates fixed-slot matches with preparation and parameter ranges;
- optional `actions` holds shared prepare/range bodies referenced by opportunities;
- `selections` records rule priority, logical first/last match
  direction, subword traversal, named result records, and state constraints;
- `stages` groups reusable selection phases, priorities, allocators, and repetition budgets;
- `stretchpolicy` and `shrinkpolicy` define complete ordered recipes.

A catalog carries one or more named recipes in each direction. Everything else
-- facts, opportunities, attachments, shared actions, selections, and the `pagepolicy` and
`linepolicy` -- belongs to the catalog and is shared, so every recipe uses one
compiled DFA and one interned id space; a fact bit means the same thing in all
of them. `JustOption::justStretchPolicy` and `justShrinkPolicy` are optional
runtime overrides. Negative values use the names declared by `linepolicy`.

The line and page policies stay on the catalog rather than inside either
directional recipe because they govern selection, sizing, and rendering.

Sharing one catalog rather than declaring several `table(justdfa)` blocks is
deliberate. Separate catalogs would each intern their own fact bits and build
their own transition table, so nothing could be shared without the compiler
reconstructing which parts happened to be identical. Every recipe is scheduled
over one shared DFA, which is why adding a policy costs only its own ordered
steps. Every catalog must declare at least one named policy in each direction;
`linepolicy` selects them with `stretch Name;` and `shrink Name;`.

`table(justdfa)` is a DigitalKhatt extension to the feature-file grammar. It is
declarative metadata and never produces a GSUB or GPOS lookup, so moving or
editing it cannot change HarfBuzz lookup ordering. The parser validates and
compiles this table when the feature file is loaded. `DeclPolicy` then
uses that compiled catalog for facts, fixed-slot rules, action bindings,
selections, stages, and stretch policies. Missing references, duplicate
definitions, unsupported engine vocabulary, and incomplete rules fail at load
time.

There is no default or shadow copy of this policy in C++. Rule, selection, and
record names are arbitrary and receive compact numeric ids when the table is
compiled. Phases reference the resulting selection ids, so no font-specific
name registry exists in the engine.

Selections express state without font-specific C++ switches:

- `record Name` records the position of a successful action;
- `forbid_recorded [Names]` rejects a selection if any named result exists;
- `require_recorded [Names]` requires every named result to have been accepted in the same word (an empty list imposes no requirement);
- `different_subword [Names]` rejects a candidate in a recorded subword;
- `different_position [Names]` rejects a candidate at a recorded position.

These properties may also follow an individual `choose`. Choice-local records
let a scored selection mix opportunity kinds without giving every matched rule
the same state:

```fea
choose CandidateKafBody last all_non_overlapping
    weight 6 decay 0.85 record Horizontal forbid_recorded [Horizontal];
choose CandidateAlternateGroup1 last selected_only
    weight 5 decay 0.85 record Terminal;
```

Selection-level and choice-level constraints are both applied. A successful
choice records both names when both levels declare a record.

OldMadina's staged policy records Kaf preparation/body stretching as `KafBody`
instead of `Horizontal`. `CandidateKafCompanion` requires that accepted record
and forbids `Horizontal`, then records `Horizontal` for one modest Reh or
joined connection. Its trial left range is 0.5..1 and right range is 1..2.
It preserves existing parameters rather than clearing either slot. Ordinary
horizontal creation and continuation exclude `KafBody` words, so a later
stage cannot enlarge the companion beyond its cap. Kaf and terminal
continuation remain eligible under their existing action guards. The
companion phase follows both preferred Kaf selection and late Kaf fallback;
the record prevents those two phases from opening two connections. The
standalone `candidate_pool` and fixed-step comparison policies are unchanged.

C++ defines only generic matching behavior and the primitives an action is
built from. Actions themselves can now be declared in the table:

```fea
attachments {
  attachment Fatha find [1614] skip [1617] within 2;
}
actions {
  action IncrementAlternate {
    prepare {
      forbid $self:cv02 > 0;
      add $self:cv01 1 clamp 12;
      replace $self@Fatha { cv01 = sum(1, floordiv($self:cv01, 3)); }
    }
  }
}
opportunities {
  opportunity AlternateGroup1 {
    match [{ form terminal chars ["بتثكن"] }];
    action IncrementAlternate;
  }
}
```

Every action reference must name a declared shared action. An opportunity may
instead contain its prepare/range body inline; combining a reference and inline
body is rejected. Separate `rules`, `rule` and `bind` declarations are no longer
accepted. The compiled DFA still uses internal pattern IDs and effect programs;
there is no second runtime implementation for opportunities.

```fea
opportunity TerminalGroup1 {
  match [{ form terminal chars ["بتثكن"] }];
  prepare {
    forbid $self:righttatweel > 0;
    forbid $self:lefttatweel > 0;
    lookup $self justify.expand_terminal;
  }
  range $self:lefttatweel minimum 1 preferred 3 maximum 9;
}
```

An opportunity's match can mix named and inline facts:
`match [BehFirst { position last glyphs [meem.fina meem.fina.ii] }];`.
An inline fact has the same discriminators and AND semantics as a named fact.
Identical facts are interned where their definitions agree, including reuse of
named fact bits. Empty inline facts are rejected, as are empty patterns.

Repeated matches can be declared once in an optional `patterns` block immediately
after `facts`, before attachments and shared actions:

```fea
patterns {
  pattern FinalMeemConnection [BehFirst FinalMeem];
  pattern UpperConnection [BehFirst { position last glyphs [meem.fina meem.fina.ii] }];
}
actions {
  action OpenConnection {
    prepare { lookup $1 justify.decompose_left; }
    range $1:lefttatweel minimum 0.5 preferred 1.75 maximum 4;
  }
}
opportunities {
  opportunity PreferredFinalMeem {
    match FinalMeemConnection;
    action OpenConnection;
  }
  opportunity AlternativeConnection {
    match UpperConnection;
    prepare { lookup $1 justify.decompose_left; }
    range $1:lefttatweel minimum 1 preferred 2 maximum 5;
  }
}
```

`match Name;` references an entire named pattern; `match [Name];` instead matches
one slot using the named fact. Pattern bodies support the same mixture of named
and inline facts as inline matches. Pattern composition, repetition and recursive
pattern references are not supported: every pattern remains fixed-length.
Definitions are validated at load time, including unused patterns. Expansion
happens during catalog compilation, so opportunity identity, preparation,
selection weights, records and continuation remain independent. No extra runtime
matching pass or scheduling layer is introduced.

OldMadina shares repeated multi-slot connections through named patterns while
keeping simple one-slot matches and unique patterns inline.

`range` requires a `preferred` expression. An omitted `minimum` retains the
current staged value; an omitted `maximum` uses the preferred endpoint as the
full continuation limit. An explicit minimum installs the mandatory activation
value. Bounds accept the same expressions as updates. The old `vary ... to ...`
spelling has been removed from the feature language.

Preparation blocks and ranges are evaluated in declaration order while a trial
is built. Multiple prepare blocks may surround ranges when later preparation
depends on an earlier staged minimum. Continuation reuses accepted opportunities
without replaying their preparation. Fixed-step opportunities can contain only
`prepare`, with no ranges.

Guards combine with `and`, `or`, `not` and parentheses, and `isfact($n, Name)`
tests a slot's facts:

```fea
action ApplyKafPair {
  prepare {
    forbid isfact($2, Noon) and $2:cv01 > 0;
    update $1:cv03 = 1;
    update $2:cv03 = 1;
    replace $1@Fatha { cv01 = sum(1, floordiv($2:cv01, 3)); }
  }
}
```

`$1`..`$n` address the matched slots, so an effect on one slot can read another
-- the mark above hangs off slot 1 but takes its value from slot 2, which is
what the C++ did.

Slots are addressed **relative to the match inside its subword**, so an action
may look past the end of what the rule matched. `$3` on a two-slot rule is the
glyph after the match; addressing outside the subword matches nothing rather
than failing, so a lookahead condition is safe to write at a subword boundary.
This supported the original kashida decomposition's third-glyph tests without
widening the rule and changing what the DFA selected; glyph-driven stretching
now makes those particular tests unnecessary. The same mechanism will carry
backtrack once a syntax for slots before the match is chosen -- the context is
already there, only the addressing is not yet expressible.

Beyond `forbid`/`add`/`update`/`replace` there are `clear T;` and
`when <cond> { ... }`; conditions take `isfact($n, Fact)`, `has($n:tag)`,
`and`/`or`/`not` and parentheses, and `select(cond, a, b)` chooses inside an
expression. `has` is deliberately distinct from `> 0`: one kashida guard tests
presence alone.

The original cv11..cv19 decomposition cascade was first translated into
independent `when` blocks. Most of those guards are now eliminated: the
featureless stretch lookup's input glyph identifies the ligature component
directly. The lam/dal exception remains a policy condition, as explained below.

`$self` is the first matched slot, and `$1`..`$n` are positions relative to the
start of the opportunity's matched span. Effects run in
declaration order against staged state, so a read sees what an earlier effect
in the same action wrote -- which is how the mark above picks up the value the
`add` just produced. Guards must precede every write, which is checked when the
catalog is compiled and is what makes them observe committed state. `add`
merges and saturates, `update` merges, and `replace` clears the target first;
that distinction is load-bearing, since the old C++ merged into the base but
assigned the mark a fresh value.

Three syntax choices are forced by the feature-file lexer rather than chosen:
`:` separates a target from its attribute because `.` is legal inside an
identifier, so `$self.cv02` would arrive as one token; arithmetic is functional
(`sum`, `diff`, `prod`, `floordiv`, `min`, `max`) because `INT_LITERAL` matches
`[-+]?[0-9]+` and would swallow the sign of an infix operator; and there is no
`/` operator because `/` opens a `REGEXP` token.

Facts take keyword-tagged discriminators, each optional and ANDed:

```fea
fact BehFirst  form medi chars [1576 1578];
fact BehMedial glyphs @BehMediForms;      # any glyphset: class, glyph, or /regex/
fact SubwordLast position last;
```

`glyphs` accepts the same `glyphset` every other rule takes. Glyph sets are
expanded to glyph *names* after the features are populated, because
`getCodes` folds in substitutions and because one catalog has to stay valid
against a second font with independent glyph ids. `position` comes from the
adapter's real base indexes rather than from the glyph name, since a glyph whose
name carries no `.init`/`.medi`/`.fina`/`.isol` substring yields no form facts
at all.

Adding a new expression operator, effect verb, or glyph-form predicate still
requires engine code; adding or renaming facts, rules, selections, records,
constraints, phases, attachments, or actions does not.

For example, the Beh phase selects the logical first match, reproducing the
old non-greedy expression.  Direct Other-kashida selection tries the reh/zay
rule before the general joined-letter rule and selects the logical last match.
The `selected_only` multiplicity also reproduces an important PCRE detail: if
that selected greedy match is forbidden, the engine may try the next rule but
does not fall back to an earlier match of the same rule. `all_non_overlapping`
is used for the plain, unanchored expressions in the same-subword pass; it
advances beyond each accepted match just as the old regex loop did.
The final second-kashida pass is intentionally mixed: Beh and final-ascendant
remain greedy, while reh and other joined matches are visited from the logical
beginning, exactly as their old expressions without a leading `.*` behaved.

No rule has `*`, `+`, lookaround, or backtracking.  The compiler determinizes
the finite patterns, and each state's transition row covers only the predicates
that state branches on -- not the whole rule set's predicates.  That distinction
is what keeps the table small: a rule set may name up to 64 distinct slot
predicates, while a row is sized by the most any single position must tell
apart.  The oldmadina table has 11 predicates across 95 states with a maximum
branching factor of 6, giving 634 transition entries -- 1.2 KB.  Sized densely
over all 11 predicates instead, the same 95 states would need 194,560 entries,
or 380 KB.  Runtime scanning is bounded by the longest rule (two glyphs for the
current oldmadina table), and each glyph's predicates are evaluated once per
line rather than once per visit.

A glyph is not a symbol: it satisfies a *set* of predicates at once, so a
state's alphabet is the powerset of what it branches on rather than a flat
character range.  A lexer generator escapes this by enumerating its concrete
alphabet -- 256 bytes -- and merging the ones no rule distinguishes, which turns
overlapping character classes back into a partition.  That is not available
here: the concrete alphabet is glyph name crossed with source character and
subword position, and none of it is known when the rules compile.  So states up
to a branching factor of 8 are enumerated up front, and wider ones memoize the
combinations that real text actually produces.  This removes any bound on how
many predicates one position may distinguish; 64 total predicates remains the
only limit, because a glyph signature is one 64-bit word.  Every state in the
oldmadina table branches on at most 6, so none of them memoizes and the table is
identical to the fully enumerated one.  The cost of eager enumeration is
otherwise real: of the 634 entries built for oldmadina, only 21 distinct
transitions are ever taken across all 604 pages, the rest describing glyph
shapes -- medial and isolated at once, say -- that no font can produce.

Because match() memoizes, it mutates internal caches through a `const` object.
Like the rest of this library a `FixedSlotDfa` is single-threaded: one instance
must not be matched from several threads at once.

Memoizing also moves one failure from load time to render time.  State numbers
are 16-bit, so a rule set that reaches 65,535 deterministic states throws; when
every state was enumerated up front that happened while parsing the feature
file, and now a memoizing state can reach it mid-match.  Growth is proportional
to the combinations the text produces, not to the rule set: the randomized test
in `tests/declpolicy/DeclPolicyTest.cpp` -- twelve deliberately overlapping predicates at both
positions, fed random facts -- settles at roughly 3,800 states and 57,000
entries, while oldmadina reaches 13 states and 29 entries across all 604 pages.
A rule set ambiguous enough to approach the ceiling would need a cache budget
and a flush, which is what RE2 does; nothing here implements one yet.

The semantic action boundary is the migration point for parameterized glyphs.
The evaluator reaches the outside world only through
`JustificationStagingBackend` -- eight methods covering staged reads, merges,
clears, attachment resolution and commit -- so a backend that writes
`left_tatweel` instead of `cv01` is a drop-in replacement driving the same
catalog, evaluator and DFA. `FeatureStagingBackend` in `src/justify/declpolicy/DeclPolicyAdapter.cpp`
is the cvXX implementation; it owns no policy.  The engine has its own page
entry point, `DeclPolicyPageJustifier::justifyPage`
(`src/justify/declpolicy/DeclPolicyPageJustifier.cpp`), which runs the declarative policy
and nothing else; `FeatureJustifier::justifyPageUsingFeatures` keeps the four
hand-written regex policies.  The two share line analysis, shaping and
measurement through `src/justify/JustificationShaping.h`, so a line no policy
touches lays out identically through either. The interface's `applyLookup`
binds an action to one slot of a contextual match and applies a lookup there
the way contextual OpenType rules do.

An action stretches a glyph by applying a named GSUB lookup at a slot --
`lookup $1 justify.stretch_left clamp 6;` -- the way a contextual OpenType rule applies a
lookup to one position of its match.  `times <expr>` applies it a computed
number of times and `clamp <n>` caps what one site may accumulate, saturating
rather than refusing so that an exhausted action measures as `NoChange`, and
`applied($t, <name>)` reads the count back.  `clear` drops a site's
applications along with its attributes: staged state is a description replayed
from scratch on every measurement, never a committed mutation, so un-applying
is meaningful.

A lookup runs at *every glyph in the target's cluster*, which is what a feature
scoped to one character does -- `hb_feature_t`'s `[start, end)` is a character
range, and HarfBuzz sets the bit on every glyph whose cluster falls inside it.
It has to: an attachment target such as the fatha is a mark, and restricting to
the cluster's base glyph silently drops its stretch.  There is no separate substitution
vocabulary and no cvXX: ordinary single substitution selects a new glyph,
and `SingleSubstFormat11` can both `replace_glyph()` and add to the glyph's
`lefttatweel`/`righttatweel`, in one application.  Because they add rather
than assign, applying the same lookup again accumulates -- which is what
"stretch this by one more unit" means -- and the engine never writes those
fields itself.

That also keeps the static OTF equivalent for free: those subtables report
`isConvertible()`, and a non-extended font is emitted through
`getConvertedOpenTypeTables()` (`OtLayout.cpp:898`), which maps each
pre-generated alternate to the one a delta further along.  So the live shaper
carries tatweel in the buffer while the OTF walks a chain of pre-expanded
glyphs, and both reach the same result -- which is why the mushaf comparison
can hold one catalog against two fonts at all.

Lookup names, not indices, are what the catalog stores, for the same reason
glyph sets resolve to names: the two fonts number their GSUB lookups
differently, so each provider resolves the name against its own font
(`FeatureJustificationLayout::justificationLookup`, resolved once per page by
`resolveJustificationLookups`).  The numbering is not merely per-font but
per-mode: `getGSUBorGPOS` keeps the `fsmgsub` lookups only when the layout is
extended, which renumbers every lookup after them, so a name map for a
non-extended OTF has to come from a non-extended layout.

Applying a lookup at *one* glyph is scoping by mask, which is how OpenType
scopes a contextual lookup itself.  `hb_ot_layout_apply_gsub_lookup_at` (added
to the fork) sets a scratch bit on the target glyph's `info.mask`, runs the
lookup with that bit as the lookup mask, then clears the bit from every glyph.
Only the bit is touched, so everything GPOS reads afterwards survives; and
clearing by sweep rather than by remembered index survives a substitution that
changes the glyph count.  Bit 3 is the scratch bit because
`hb_ot_map_builder_t::compile` allocates feature bits from
`hb_popcount (HB_GLYPH_FLAG_DEFINED) + 1` upwards and reserves the top bit for
the global mask, leaving bit 3 unclaimed.

The action lookups are now independent, featureless definitions in
`oldmadinafont/features.fea`, not aliases of generated cvNN lookups:

```fea
lookup justify.stretch_left {
  sub noon.fina by noon.fina.expa;
  sub noon.fina.expa by noon.fina.expa add 1 0;
  # Other glyph mappings are listed in the real definition.
} justify.stretch_left;
```

One application enters the expanded form; subsequent applications add one
left-tatweel unit. `justify.stretch_right` adds half a right-tatweel unit.
The remaining `justify.decompose_pair` and `justify.decompose_lam_dal` lookups
contain ordinary single substitutions.
There is no alternate index in any of these lookups. `clamp 12` counts
applications, not parameter units, so entering an expanded form consumes a step.

The parser materializes standalone lookups referenced by `table(justdfa)`
without adding them to an OpenType feature. It uses their **declaration order**,
not their names or their first mention in an action. All decompositions precede
left stretching, which precedes right stretching. Ordinary shaping does not
execute these featureless lookups; only the external replay hook does.

#### Glyph-driven decomposition and stretching

Ligature decomposition is folded into the appropriate stretch lookup. For
example, the former `Heh && Meem && SubwordLast` condition and its two
decomposition applications are replaced by these entries:

```fea
# In justify.stretch_left:
sub heh.init.beforemeem by heh.init add 1 0;
# In justify.stretch_right:
sub meem.fina.afterheh by meem.fina add 0 0.5;
```

The first application decomposes and stretches; subsequent applications match
the ordinary output glyph and add the same delta. Existing parameters survive.
Left and right coverage must stay separate: a right component may still
belong to the preceding, unselected boundary when its left side is stretched.
Simply applying a union of all decomposition mappings to both slots is unsafe.

The same folding handles beh, beh/hah (including the before-yeh variant),
meem/hah, feh/hah, lam/hah, seen/reh and hah/meem. Rounded hah/ain forms already
had decomposition-plus-delta entries, so their character/lookahead guards were
unnecessary. Nine standalone decomposition lookups and ten action-only facts
are removed; recognition and acceptance scheduling are unchanged. Current
recognition does not need to remember that a ligature used to exist: replay
starts from the ordinary shaped word and the folded lookup reconstructs the
same result from the accepted application counts.

One decomposition guard remains intentionally:

```fea
when isfact($1, Lam) and isfact($1, SubwordFirst) and isfact($2, Dal) {
  lookup $1 justify.decompose_lam_dal clamp 1;
  lookup $2 justify.decompose_lam_dal clamp 1;
}
```

Dal and dhal share the same base glyphs, and `dal.fina.afterlam` can follow
medial lam too. The legacy policy decomposes only subword-initial lam + dal.
Removing these tests changed corpus output (initial lam/dhal on page 3 and
medial lam/dal on page 5). Genuine eligibility, source-identity and previous-
stretch restrictions remain in the policy rather than being inferred from
glyph coverage.

Validation: zero glyph or line differences against Experimental2 over all
604 pages (850,612 glyphs) in nine live profiles and the same-provider exported
OTF FontSizeXScale/Standard profile. The standard live run stages 341,065
lookup applications instead of 372,193, with the same 48,559 replay-hook calls.
Tests cover folded versus separate replay for 72 left/right repetition pairs,
nonzero starting parameters, opposite-side/nonmatching coverage, and the
retained initial-lam/dal guard. Re-export existing OTFs to use the new lookup
definitions and numbering.

Static export marks action lookups separately: if a target parameter state is
absent from the finite glyph grid, conversion still selects the unexpanded
target, matching the former parameterized-alternate fallback. This does not
change ordinary feature lookup conversion. Export reachability treats justdfa
as its own branch, so its outputs do not feed the legacy cv/sk branches.

Writing tatweel instead of feature values needs the rewrite to land between
GSUB and GPOS.  `hb_shape` cannot be re-entered with a buffer of glyphs --
`hb_ot_substitute_default` normalises and re-maps through cmap with no
content-type check, so glyph ids come back reinterpreted as Unicode -- and
upstream HarfBuzz exposes no API for positioning an existing glyph run
(`hb_ot_position` is `static inline` over a file-local context struct, on main
as much as on the vendored 7.3.0).  So the fork gained a client hook,
`hb_buffer_set_justify_func`. Its original post-GPOS mode sees positioned
glyphs and can request another positioning pass. Fixed-slot instead sets
`hb_buffer_set_justify_before_position(buffer, true)`: the staged lookups are
already decided, so they run after GSUB and before a single `hb_ot_position`.
No positioning occurs between hook calls. This applies to both word trials
and final shaping, independently of whether `mark`/`mkmk` are enabled. Only
measurement and recognition disable those features; final shaping keeps them.
The diagnostic lookup tests use this same production helper, and
`digitalkhatt.justification_shaping` checks the positioning-pass count against
the legacy two-pass mode as a positive control. Replaying the cvXX result
through that hook over the whole mushaf reproduces it exactly -- 5,463
justified lines, 489,859 glyphs, zero differences in glyphs, advances or
offsets.  Conditions involving word state (for example, no earlier kashida over
a given length) belong in policy filtering, not in glyph recognition and not
in HarfBuzz.

### Current-state recognition

Production DeclPolicy matching uses the latest **accepted** word state. The
initial line buffer supplies recognition until a word has accepted an action.
`tryApplyFeatures(..., true)` then retains that trial's measurement buffer in
`JustInfo::acceptedWordBuffers`, together with its committed width and lookup
history. Rejected/unchanged candidates are destroyed without replacing it.
There is no extra shaping pass just to refresh recognition.

After a positive transaction, the adapter derives current base facts and
glyph-based attachment facts from that word's retained buffer. Its local
clusters are translated back to line character indices; source characters and
source-backed word/subword/slot identities remain stable. Guard-only facts are
refreshed too. The engine re-runs the DFA only if a predicate-relevant fact or
slot structure changed; otherwise it retains the matches and updates their
action contexts. Other words are untouched.

Each action sees the accepted glyph-fact snapshot at its start. Its lookup
counts/attributes still observe earlier staged effects within that action.
Recognition changes only after the entire action is accepted. Existing phase,
level, selection and recording semantics remain: a successful action consumes
the word's turn at that level, and new matches become eligible on its next
scheduled turn. Re-entering a named stage also starts from accepted buffers,
not from initial recognition.

`FeatureJustificationLayout::recognitionGlyphName` separates logical identity
from generated parameter instances. The live adapter uses `originalglyph`
metadata; the exported-font provider decodes and caches its generated names.
Thus `fatha.20_0_10968` still matches the logical glyph `fatha`, while an actual
substitution from `noon.fina` to `noon.fina.expa` changes recognition. Raw names
remain available through `glyphName` for diagnostics.

Retained buffers are **read-only recognition snapshots**, not a new incremental
GSUB execution path. Every width trial and final shape still starts from source
text and replays the complete accepted/candidate log in GSUB order. In
particular, `clear` and newly requested earlier-order decompositions retain
their existing meaning; GPOS runs once per shaping call.

Regression tests cover newly enabled and invalidated matches, guard-only fact
changes, rejected trials, source-slot stability, word-relative clusters,
attachment invalidation, logical parameter-instance identity, and repeated
policy entry. The 604-page FontSizeXScale/Standard comparison has zero glyph,
parameter or positioning differences against Experimental2 in both live and
same-provider OTF shaping.

### Declarative line policy

`linepolicy` is an optional block after `pagepolicy` inside `table(justdfa)`. Catalogs
used only for glyph matching can omit it; `DeclPolicyPageJustifier` requires
it and has no built-in font-specific fallback. `justifyLine` in
`DeclPolicyPageJustifier.cpp` now prepares the initial measurements and a
shaping backend. `JustificationLinePolicy.cpp` compiles and executes the
recipes, independently of HarfBuzz. `JustificationShaping.cpp` remains the
shared measurement/final-shaping implementation.

The line policy selects one named default recipe in each direction. For example:

```fea
linepolicy {
  stretch fixed_steps;
  shrink standard;
}

stages {
  stage FixedStretch {
    phase Beh 2 fixed_steps;
    phase AlternateGroup1 2 fixed_steps;
  }
}

stretchpolicy fixed_steps {
  cap_spaces 250 250;
  stage FixedStretch;
  fill_spaces;
}

shrinkpolicy standard {
  fit_features [sk01 sk02 sk03 sk04 sk05 sk06 sk07 sk08 sk09 sk10 sk11 sk12 sk13 sk14 sk15 sk16 sk17 sk18 sk19 sk20];
  balance;
}

shrinkpolicy scale {
  scale;
}
```

The initial measured width selects stretch only when below the target;
otherwise it selects shrink, including at exact equality. The corresponding
named recipe supplies the steps. `JustOption::justStretchPolicy` and
`JustOption::justShrinkPolicy` can override the table defaults by compiled
index for tests and comparison tools. `ShrinkType` is not consulted by
`DeclPolicy`; legacy justifiers may continue to use it.

- `cap_spaces simple aya` allocates remaining width proportionally to the
  two space categories' capacities, capped at those absolute widths in the
  1000-unit measurement em. It never reduces existing space widths.
- `stage Name` runs that stage's glyph selections/phases while the line still
  needs expansion or reduction in the selected recipe's direction. Shrink
  stages require `candidate_pool`; `fixed_steps` and `baseline_pool` retain
  their stretch semantics. A policy may invoke several named stages,
  interleaved with its other steps.
- `fill_spaces` shares any remaining width equally among all spaces. For a
  line without spaces it uses horizontal scaling instead of dividing by zero.
- `fit_features [...]` tries the named features in written order, stopping
  at the target width. Stretch steps must increase width without exceeding
  the target. Shrink steps must reduce width. The first step reaching or
  falling below the target is accepted and stops further shrink trials.
  End the shrink recipe with `balance` to distribute any created gap equally
  among all spaces, without a restoration-capacity limit.
  Each trial starts from the accepted feature set. Rejected steps are absent
  from both subsequent measurements and final shaping.
- `all_features [...]` stages the complete list without measurement (`Test`).
- `balance` fills undershoot with spacing or scales remaining overshoot.
  After feature shrinking, spacing is added to the shaped space advances;
  it does not replace the shrink features' own space adjustments. The added
  spacing is converted from the measurement em to the final font scale and
  rounded with a carried remainder across spaces. No Style leaves remaining
  overshoot unscaled; a fitting style can apply the residual scale.
- `fit_sclx base` measures SCLX at `base * target / width` and retains the axis
  only if it reduces width; horizontal scaling fits the residual. This
  primitive still uses the existing SCLX result field, not a generic axis map.
- `scale` directly sets horizontal scaling to `target / width`.

The compiler rejects unknown operations, invalid numeric arguments, malformed
or repeated feature tags, empty or duplicate recipes, and missing policy
references. A well-formed feature tag absent from the font remains inert,
matching HarfBuzz and the old `sk01`–`sk20` loop.
Page-wide font-size thresholds and special sura/basmala handling are declared
in the separate page policy below.

### Weighted shrinking

OldMadina currently defaults to `shrinkpolicy weighted_native_spacebefore`;
`standard` retains a whole-line feature schedule for comparison. The
feature-based `weighted` recipe keeps
`sk01`–`sk03` as global compact-form/kerning steps, then offers `sk04` and `sk05`
at individual base clusters. These are discrete feature opportunities, so
weights control selection order rather than a continuous shrink amount.
Their existing contextual exclusions and glyph deformation ranges still apply.
Full word shaping measures each trial; a non-reducing change or one that
shrinks past the target is rejected without changing accepted state.

The first stage allows one opportunity per subword and two per word. A second
stage uses the remaining first-level opportunities before offering the second
level at sites already compressed. Initial/medial forms have weight 3 and
other forms weight 1, with occurrence decay 0.95 and a longer-subword bonus
0.2. These are initial tunable values, not a claim of an optimal visual balance.
Contextual feature fallbacks and final `balance` remain
declared in each recipe. The current recipes omit `sk12` and `sk13`.
Font-size policy and stretch recipes are unchanged.

For a native-parameter comparison, select **Declarative policy**, then set the
GUI's **Shrink policy** to `weighted_native`. Select `weighted` for the
feature-based recipe, or **Font default** to use the recipe declared by
`linepolicy`. Recipe labels match their exact names in the feature file. The
GUI retains the selected recipe name across reloads and resolves its index
against the current font catalog.

The toolbar shows declared shrink recipes only for Declarative policy and
legacy shrink rules only for Madina, IndoPak and the experimental engines.
HarfBuzz and No Justification expose neither shrink selector and offer only
their supported fitting styles (No Style, XScale and FontSize). Declarative
fitting choices come from the loaded font's page policy; SCLX Axis is shown
only when its selected shrink recipe uses `fit_sclx`.
Each engine retains its fitting preference across switching and font reloads.
The Font default entry also identifies the actual declared shrink recipe.

`weighted_native` keeps the same weights, two passes, discrete reductions and
fallbacks. It replaces per-site `sk04`/`sk05` feature writes with additive
`lefttatweel_delta` and `righttatweel_delta` assignments. Read-only attributes
such as `$self:sk04_lefttatweel` query that feature's contextual parameter
change at the site. A GSUB baseline and one probe per queried feature are
cached per line; native assignments are excluded from those probes. A missing
or structurally substituted site yields zero. Non-native writes or global
feature changes invalidate the probes. Queries cannot be written by actions.
These names reserve the `AXIS_delta` and `TAG_AXIS` attribute forms.

The native recipe estimates candidates using affected-glyph advances and
confirms accepted words by shaping. `weighted_native_full_shape` uses the same
native actions with exact word-shaped endpoint measurements for validation.
Deltas are applied after GSUB, preserving later additive reductions. Their
ordering differs from per-site GSUB features, so later structural fallbacks,
fixed-point rounding and final balancing can produce a different distribution.
`weighted_native_spacebefore` moves `sk10` and `sk11` before the native
shrinking stages, with `sk06`–`sk09` afterward. The native actions themselves
do not change page sizing; `linepolicy` selects the default recipe.
This comparison retains discrete levels; it does not yet replace them with
continuous proportional ranges.

The shared allocator also accepts decreasing native `vary` ranges. It plans
with positive reduction capacities while retaining signed endpoint deltas,
interpolates by weight, quantizes toward the mandatory endpoint, and confirms
an accepted trial with actual shaping. The font must supply appropriate
negative ranges and contextual guards when authoring such native actions.
Later global feature trials and space restoration measure the complete line,
including all accepted per-site features, parameters, and substitutions.

`digitalkhatt_compare_extreme_lines --justifier decl-policy --shrink-policy
weighted --compare-shrink-policy standard --line-list LINES.csv --output
comparison.pdf oldmadina.mp` renders both recipes at the same page sizing.
Each CSV row contains `page,line` (one-based), with an optional header.
Add `--text-width 16000` to override the target text width in font units
(positive integer, at most 100000; default `OtLayout::TextWidth`, currently
16400). It applies to natural-width ranking, justification, short-line target
ratios, and comparison rendering. PDF text areas retain their point width;
font-unit-to-PDF scaling follows the selected target width. Both policies
in a paired comparison use the same width.
Add `--font-size-scale 0.95` to multiply the normal DigitalKhatt base font size
by 0.95, like setting the GUI font-size control from 100 to 95. This affects
natural-width ranking, justification and mark positioning; it keeps the target
line widths and scanned reference sizes unchanged. Both policies in a paired
comparison use the same base multiplier. Values must be positive and finite.
The default multiplier is 1, with the existing FontSizeXScale page fitting.
The comparison PDF applies `fontSize` once to the whole line, including advances
and offsets. Natural-width rankings and XScale-only baselines use the rendered
width after that scale. Spacing A/B comparisons keep internal glyph coordinates
unchanged and redistribute width only through word spaces.
`--page-fitting XScale` keeps the base font size fixed while permitting residual
horizontal scaling; `--page-fitting FontSizeXScale` allows the usual page
adjustments. Other supported styles are None, SameSizeByPage, FontSize and SCLX.
Explicit size/fitting options are recorded in PDF labels and console output.
Add `--show-natural` to prepend the unjustified DigitalKhatt line, and
`--show-xscale` to prepend a horizontally compressed natural line for tight
lines only. Both flags are independent and off by default; when both are
enabled the order is natural, XScale-only (tight lines), justified result,
then the requested scan/QPC references. Loose lines never get an XScale-only
row. Baselines use the base `--font-size-scale` without page fitting, shrink
features, space adjustment or Force optimization. The XScale-only ratio is
target width divided by the natural advance width; vertical size and glyph
parameters are unchanged. These rows are also supported in paired policy
comparisons. All rows, including short lines and references, share a common
right ink edge so differences in their left ends reveal their widths.
Dashed left and right guides always show each line's target width, including
when both baseline flags are off and in paired policy comparisons.
Rows retain the original interline spacing, without individual row labels;
the page header lists their
order and the item description includes the XScale-only compression ratio for
tight lines. Items share a page when they fit, with extra paper width when
necessary to preserve overfull natural ink at the same unit scale.
The right alignment also applies when neither baseline flag is supplied.

QPC references default to `--qpc-layout tarteel`. This follows the QPC V1
layout in Tarteel's `generateMadinaParagraph`: one U+0020 space between
encoded words, the page font `QCF_Pnnn.ttf`, and a fixed page font size derived
from `paragraphWidth / 15.6`, with the comparison tuning described below.
Tarteel centers the advance width; the comparison aligns the right ink edge
with the other rows. Tarteel's `paragraphWidth` already excludes outer
page margins and may be reduced further by its aspect-ratio constraint. Its
DigitalKhatt renderer reserves another 1.5% on each side within that paragraph,
so the full text width is `paragraphWidth * 0.97`. The comparison's `pageWidth`
variable denotes that full text width (normally 16400 layout units), rather
than the paragraph or physical page width. At the comparison's drawing scale,
the exact equivalent QPC em would be `pageWidth / (0.97 * 15.6)` in DigitalKhatt
layout units. The comparison currently uses the tuned factor `0.95` instead:
`pageWidth / (0.95 * 15.6)`. It is independent of the individual line's target,
natural ink width and `--font-size-scale`. Short lines retain their natural
width and share the common right edge. No extra word spacing or horizontal
scaling is applied. The PDF and `--scale-report` use the same reference scale.
Use `--qpc-layout ink-fit` for the previous behavior: uniformly resize each
line until its ink width matches its DigitalKhatt target, aligning ink edges.
Comparison alignment differs from Tarteel's centered placement; PDF and Skia
rasterization can also differ.
The PDF labels the upper and lower policies explicitly. The runtime regression
in `tests/declpolicy/WeightedShrinkIntegrationTest.cpp` covers decreasing native
ranges, weighted allocation, small budgets, replay, contextual feature probes,
additive fallback composition, repeat guards, and discrete subword limits;
its header contains the command for the existing GUI probe runner.

### Scored candidate pools

Supported phase allocators are `fixed_steps` and `candidate_pool`, both with
an explicit level count. The standalone `proportional` allocator has been
removed. Candidate pools still interpolate `vary` ranges using a common ratio
and retain the `quantize` and `candidate_width` settings.

`candidate_pool` collects every eligible DFA match in its stage, ranks the
candidates, applies cheap policy constraints, measures the survivors' mandatory
and maximum width deltas, and distributes the available width among retained
sites. Collection itself is width-free: candidates rejected by repetition,
phase, word, subword, record, priority, or site-conflict checks are never shaped
for endpoint measurement. Only survivors are measured before the fit decision.
Candidate endpoints use affected-glyph advances by default. This deliberately
ignores cursive and contextual GPOS effects but avoids two word-shaping calls per
surviving candidate. A recipe can request exact endpoint shaping instead:

```fea
candidate_width full_shape;
```

Use `candidate_width advance;` to state the default explicitly. If the active
font provider cannot supply parameterized advances, `advance` falls back to a
full word shape. Selected candidates are always committed with normal shaping,
independently of this endpoint-measurement setting.
The rule order remains a hard priority only when phases use different priority
bands. `per_word` and `per_subword` occupancy is shared by all candidate-pool
phases in the same stage, so a fallback band cannot add extra opportunities by
resetting those limits. `limit` remains local to its phase. Within one phase
the score is:

```text
phase weight * rule weight * rule decay^occurrence
  + longer_subword * (subword length - 1)
  + central_connection * centrality
  + word_position * logical word position
```

`centrality` is zero for single-glyph alternates. For a two-glyph connection at
one-based position `p` in an `n`-glyph subword it is
`1 - abs(2*p - n)/n`. It therefore reaches one after glyph 2 in a four-glyph
subword. Logical word position is -1 for the first word and +1 for the last. A
phase can therefore be written:

```fea
phase CandidatePrimary 1 candidate_pool weight 1 limit 64 per_word 2 per_subword 1 longer_subword 1 central_connection 2 word_position 0.25;
```

The `choose` declaration supplies the rule weight and occurrence decay:

```fea
choose CandidateSwashNoon last all_non_overlapping weight 5 decay 0.85;
```

Decision tracing is disabled by default. Register a callback on
`FeatureJustificationLayout`, or on `OtLayout` for the live renderer, to receive
the phase, rule, occurrence, site, score breakdown, measured width range,
parameter values, applied ratio, remaining width, and rejection reason. The
comparison tool exposes the same data as JSON lines with
`--trace-candidates PAGE:LINE`; combine it with `--snapshot-live` to inspect
native glyph-parameter actions.

### Staged candidate policy

OldMadina's `staged_pool_continue` allocates preferred lengths first, then
continues those same accepted opportunities without separate preferred-continuation
rules:

```fea
stretchpolicy staged_pool_continue {
  candidate_width advance;
  cap_spaces 250 250;
  stage PreferredStretch;
  stage DecomposeConnections;
  stage RareTerminalStretch;
  stage DecomposeLastResort;
  stage PreferredStretch continue to 6;
  stage InitialShortStretch;
  stage PreferredStretch continue to 9;
  cap_spaces 400 400;
  stage RemainingStretch;
  fill_spaces;
}
```

Each preferred opportunity declares its activation, preferred endpoint and full
maximum together. For example, terminal group 1 uses lefttatweel minimum 1,
preferred 3 and maximum 9; Kaf uses body parameter `third` minimum 1, preferred
1.75 and maximum 4. `continue to` never exceeds the opportunity's own maximum.
It retains the selected site, original score, parameter relationships and record
ownership without repeating preparation lookups. Newly eligible sites may still
activate through ordinary recognition and preparation guards.

`RemainingStretch` still uses `ContinueLamAlternate`, `ContinueBeforeHah` and
`ContinueSadReh`. These belong to the separate rare-terminal and decomposition
stages, not `PreferredStretch`, so preferred-stage continuation does not replace
them. They recognize current glyphs, require an existing stretch where appropriate,
and leave minimal last-resort decomposition sites locked through
`different_position [MinimalDecomposition]`. They are followed by remaining Kaf,
Kaf-companion and generic fallback opportunities.

To measure endpoints with complete shaping instead, set
`candidate_width full_shape;` on the same policy. Separate `staged_pool` and
`staged_pool_full_shape` policies and their duplicate preferred-continuation
rules are no longer provided. `fixed_steps` and `native_fixed_steps` remain
independently selectable.

`baseline_pool` may run after other stretch stages. It skips the whole connection
if either requested parameter is already nonzero, preserving accepted stretching
and intrinsic shaping adjustments. Untouched supported connections remain
eligible. Endpoints must be finite and positive. Its separate baseline values
do not become accepted preferred opportunities and cannot by themselves trigger
stage continuation.

### Continuing a stage

```fea
stage PreferredStretch;
cap_spaces 250 250;
stage PreferredStretch continue by 1;
stage PreferredStretch continue to 9;
stage PreferredStretch continue;
```

Continuation uses `candidate_pool`, requires native glyph parameters, and is
stretch-only. `continue by N` offers up to N additional units on the leading
varying parameter of each opportunity. `continue to N` offers capacity up to
absolute value N, never reducing an accepted value. Bare `continue` offers each
action's entire remaining capacity. All three honor the action's declared
maximum; other varying parameters follow the same interpolation ratio (for
example, outgoing +1 and incoming +0.5). These are capacities, not forced values:
the weighted allocator chooses what fits the remaining line width. Existing
quantization and measured-width commit checks still apply. The experimental
`increment N` spelling has been removed.

An action can declare its ordinary preferred endpoint and a full maximum together:

```fea
range $self:lefttatweel minimum 1 preferred 3 maximum 9;
```

Ordinary stages still offer the endpoint 3. Continuation can advance up to 9;
omitting `maximum` uses the ordinary endpoint as the full limit as well.
The optional maximum is an expression, evaluated at activation, and must be
finite and no smaller than the preferred endpoint.

Accepted opportunities are retained only within the current line and stage name,
and only for stages referenced by continuation in the selected recipe. Thus an
ordinary first invocation retains its accepted sites without changing its
allocation, while recipes without continuation avoid the state-tracking cost.
Later invocations recognize the same rule on the current glyphs, validate the
accepted glyph identity and the parameters written by that opportunity, and offer the next range without replaying
activation guards, substitutions, clears, or mandatory updates. Changes made by
an intervening writer to an owned parameter invalidate the affected opportunity;
changes to unrelated axes on the same glyph do not. The site's own records
do not block its continuation, but other record constraints remain enforced.
New opportunities may also start during continuation, using ordinary recognition
and activation guards. Mandatory activation values count toward the offered
amount; a bound smaller than the mandatory minimum skips the site. Continuation
keeps this stage's original weights; it does
not implicitly adopt a separate continuation selection's weights or traversal.

OldMadina's default `staged_pool_continue` starts with ordinary PreferredStretch
allocation, then continues to 6 and 9 around the short-baseline pass.
`RemainingStretch` handles independent decomposition/rare-terminal continuation
and fallback families. The old staged comparison policies and their duplicate
preferred-continuation definitions have been removed.

### Declarative page policy

`pagepolicy` precedes `linepolicy`. It is optional for standalone glyph/line
catalogs and required by `DeclPolicyPageJustifier::justifyPage`; no implicit
font-specific fallback is supplied. `JustificationPagePolicy.cpp` compiles
and evaluates it without HarfBuzz. The adapter owns fonts and buffers,
collects measurements, invokes linepolicy, and shapes the resulting lines.

```fea
pagepolicy {
  measure [Line Sura Bism];
  sizing style SameSizeByPage { min_fit 1; }
  sizing style FontSizeXScale { bounded_fit 0.95 1.2 0.02 0.02; }
  sizing default { fixed_size 1; }

  line type Bism when basm2 { natural; final_features [bism]; }
  line type Bism { natural; final_features [basm]; }
  line type Sura { natural; }
  line default { use_line_policy; }

  render style FontSize { font_scale; normal_output; }
  render style FontSizeXScale { xscale_output; }
  render style SCLX { axis_output; }
  render style XScale { xscale_output; }
  render default { normal_output; }
}
```

Each group uses the first matching branch in written order and requires a
final default. `when basm2` tests the input line's variant flag; an
unconditional rule for the same line type must follow any conditional one.
The compiler rejects unknown selectors, hidden/duplicate branches, invalid
numeric arguments, malformed features and conflicting treatments.

- `measure [...]` chooses the input line types used for page-size statistics.
  Zero-width lines and invalid/empty measurements do not contribute.
- `fixed_size ratio` selects a fixed multiplier of the requested em scale.
- `min_fit ceiling` uses the smallest target/measured-width ratio, capped at
  the given ceiling. With no usable measurements it uses the ceiling.
- `bounded_fit lower upper shrink_limit stretch_limit` keeps size unchanged
  when all ratios are within the thresholds or when the page crosses both
  thresholds. Otherwise it moves size toward the outlying side, limited by
  both the declared cap and the room at the opposite threshold. With no
  usable measurements it returns 1. OldMadina's thresholds/caps are now data.
- `natural` bypasses line justification and keeps natural spacing;
  `use_line_policy` delegates to the declared line-level recipe.
- `final_features [...]` stages whole-line features for final shaping only;
  it does not affect initial page/line measurements. Tags and their order are
  explicit, including the two basmala variants.
- `font_scale` consumes a line's residual horizontal scale by changing its
  shaping font size, but only when page sizing has not already changed it.
- `normal_output` leaves body-line output xscale at 1; `xscale_output`
  exposes the residual scale for body lines, leaving headings unchanged.
- `axis_output` transfers residual scaling to font-size metadata and records
  the existing SCLX result. It applies to every line, preserving the prior
  SCLX path; it is not yet a generic multi-axis representation.

Rendering operations have fixed lifecycle phases: `font_scale` acts before
shaping, the selected output operation after shaping. They are not arbitrary
reorderable drawing commands. Font creation, GPOS, positioning/alignment from
the input line, and Tajweed processing remain execution services.

Page-policy migration validation: all 604 pages (850,612 glyphs per run)
agree exactly with the unchanged Experimental2 engine for all six JustStyle
values on the live provider with Standard shrinking. FontSizeXScale also
agrees with Test/None shrinking and with Tajweed enabled. The checks include
glyph IDs, clusters, tatweel, advances, offsets, colors, line dimensions and
positions, font sizes, and scale metadata. Exported-OTF FontSizeXScale/Standard
also passes the whole-corpus comparison. The `digitalkhatt.page_policy` unit
test additionally exercises policy mutations and malformed declarations.

For regression work, `digitalkhatt_compare_mushaf_shaping --decl-policy` runs the
existing `Experimental2` regex implementation against `DeclPolicy` and
writes every final glyph/parameter/position difference to CSV.
`--same-provider` is the sharper gate: it runs both engines on the OpenType
provider, isolating the engine difference from the outline-versus-font one.

`oldmadinafont/features.fea` drives all three actions through its named single
lookups. Neither the action definitions nor their `applied(...)` guards depend
on cvXX. The cvNN features and generated alternate lookups remain exclusively
for the legacy `FeatureJustifier::justifyPageUsingFeatures` comparison path.
Nine applications of `justify.stretch_left` take `noon.fina` to
`noon.fina.expa` at eight left-tatweel units in the live shaper.

`digitalkhatt.justification_lookups` tests parser declaration order (including
nested definitions) and production replay against a font containing single
lookups but **no features**. It checks that normal shaping does not run them,
replay sorts by GSUB order, parameter deltas accumulate, and other clusters
are untouched.

Validation of the featureless migration: the 604-page corpus (850,612 glyphs)
has zero final glyph/parameter/position differences with FontSizeXScale and
Standard shrinking, both with the live provider and with both engines using a
freshly exported OTF. A separate live font with the `glyphalternates` feature
block removed also matches. All thirteen exported single-lookup mappings match
the corresponding legacy first-alternate mappings across the entire glyph set,
including parameter states not reached by the corpus. Existing OTFs must be
re-exported to contain the new featureless lookups.

A passing gate proves agreement, not coverage. Mutating the catalog and
re-running is what proves a rule is reached at all, and doing so shows that the
mushaf exercises only four of the ten decomposition branches -- cv12, cv17,
cv18 and cv19. cv11, cv13, cv14, cv15 and both halves of cv16 never fire, in
the C++ as much as here, so their transcription rests on reading the original
rather than on the corpus. The lookahead conditions live in that unexercised
region, which is why they are covered by unit tests instead. The comparison
tool passes the catalog compiled from `oldmadinafont/features.fea` to its OTF
provider. A future standalone-font workflow should serialize the same catalog
in a DigitalKhatt-owned sidecar or custom SFNT table; HarfBuzz does not need to
interpret that data.

## Golden snapshots

The `--same-provider` and `--decl-policy` gates compare the DeclPolicy engine against
Experimental2, so they stop meaning anything once Experimental2 is retired.
`tests/declpolicy/golden/` holds a recorded baseline of each engine's own output, checked
with `--check-snapshot`, so the regression net survives that changeover.  See
`tests/declpolicy/golden/README.md`.

## Where the time goes

Trial measurement is the engine's cost: one full-Mushaf run shapes about 86,000
word trials plus 256,000 line and word widths, and shaping is essentially all
of the runtime.

Caching those measurements was tried and removed.  It is worth recording why,
so it is not tried again on the same assumption: **only about 2% of word trials
repeat a state** (1,677 of 86,207).  State accumulates monotonically, so a
repeat needs both "nothing changed since" and "the same action retried", which
the phase schedule rarely produces -- it mostly visits genuinely distinct
states.  Caching unjustified word widths and per-glyph facts as well brought
the total to roughly 3%, which did not pay for three caches and their lifetimes.

The lever is therefore the number of candidates the search tries, not the cost
of trying one.
