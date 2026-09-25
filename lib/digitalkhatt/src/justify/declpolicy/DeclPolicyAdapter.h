#pragma once

#include <vector>

#include <digitalkhatt/justify/declpolicy/JustificationCandidate.h>

#include "../FeatureJustifierInternal.h"

namespace digitalkhatt::justify::decl {

double quantizeProportionalParameter(double value, double from, double to, unsigned subdivisions);

// Runs the declarative policy catalog over one line: converts the shaped
// glyphs to facts, asks the policy engine for candidate sites, and lets the
// declared action for each one stage and commit lookups/feature changes.
// Accepted measurement buffers refresh that word's recognition before the
// next action. Rejected trials leave glyph facts and attachments untouched.
//
// Returns true when a candidate overflowed the line, which abandons it.
bool applyDeclPolicyStage(const runtime::LineTextInfo& lineTextInfo, runtime::JustInfo& justInfo, std::span<const PolicyPhase> phases, unsigned parameterQuantization, CandidateWidthMode candidateWidth);

// Builds a measured, immutable inventory for a future line-wide allocator.
// It neither commits actions nor changes the current execution state.
std::vector<JustificationCandidate> collectDeclPolicyStageCandidates(const runtime::LineTextInfo& lineTextInfo, runtime::JustInfo& justInfo, std::span<const PolicyPhase> phases);

}  // namespace digitalkhatt::justify::decl
