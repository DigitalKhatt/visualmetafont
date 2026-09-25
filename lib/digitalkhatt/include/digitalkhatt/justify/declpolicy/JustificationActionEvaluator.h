#pragma once

#include <span>

#include <digitalkhatt/justify/declpolicy/DeclPolicyJustifier.h>
#include <digitalkhatt/justify/declpolicy/JustificationCatalog.h>
#include <digitalkhatt/justify/declpolicy/JustificationStagingBackend.h>

namespace digitalkhatt::justify {

// Runs one declared action over one matched site as a single transaction.
//
// Effects execute in declaration order against staged state, so a read observes
// what the effects before it wrote.  Guards are required to precede every write
// (checked when the catalog is compiled), which is what makes them observe
// committed state.  An effect whose target is an attachment that does not
// resolve is skipped rather than being an error, as is one addressing a slot
// outside the subword.
//
// Slots are addressed relative to the match: with matchOffset pointing at the
// match inside `context`, $1..$n are the matched slots and anything past them
// reaches the rest of the subword.
// underfull()/underfull_percent() read committed line metrics, not trial
// writes in this action. Candidate-pool conditions are evaluated at collection
// for each pass; they do not change the membership of an already selected pool.
FixedSlotActionResult evaluateJustificationAction(
    const CompiledJustificationCatalog& catalog,
    const JustificationAction& action,
    std::span<const FixedSlotSlot> context,
    int matchOffset,
    int anchorSlot,
    int wordIndex,
    JustificationStagingBackend& backend);

}  // namespace digitalkhatt::justify
