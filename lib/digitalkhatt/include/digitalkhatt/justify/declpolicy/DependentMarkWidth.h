#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <span>
#include <digitalkhatt/core/GlyphParameters.h>

namespace digitalkhatt::justify {

// Isolate the signed physical width change of selected axes on the current glyph.
// Body/other axes stay at their accepted values in both measurements. Selected
// axes return to their post-GSUB baseline, not zero: contextual shaping is
// not mistaken for extra kashida length. An omitted baseline is zero.
template <class Width>
std::optional<double> dependentMarkAxisWidthChange(const GlyphParameters& parameters, std::span<const GlyphAxisId> axes, Width width, const GlyphParameters& baseline = {}) {
  auto neutral = parameters;
  bool changed = false;
  for (const auto axis : axes) {
    changed |= parameters.value(axis) != baseline.value(axis);
    neutral.set(axis, baseline.value(axis));
  }
  if (!changed) return 0;
  const auto current = width(parameters);
  const auto first = width(neutral);
  if (!current || !first || !std::isfinite(*current) || !std::isfinite(*first)) return std::nullopt;
  return *current - *first;
}

template <class Width>
std::optional<double> dependentMarkAxisWidthGrowth(const GlyphParameters& parameters, std::span<const GlyphAxisId> axes, Width width, const GlyphParameters& baseline = {}) {
  const auto change = dependentMarkAxisWidthChange(parameters, axes, width, baseline);
  return change ? std::optional<double>(std::max(0.0, *change)) : std::nullopt;
}

// A connection belongs to its outgoing (right-hand) glyph. Its neighbor's
// incoming deformation contributes width but never triggers that neighbor's
// mark by itself. A missing neighbor contributes zero, not another word.
template <class CurrentWidth, class NextWidth>
std::optional<double> dependentMarkConnectionWidthGrowth(const GlyphParameters& current, GlyphAxisId outgoingAxis, const GlyphParameters* next, GlyphAxisId incomingAxis, CurrentWidth currentWidth, NextWidth nextWidth, const GlyphParameters& currentBaseline = {}, const GlyphParameters* nextBaseline = nullptr) {
  if (current.value(outgoingAxis) <= currentBaseline.value(outgoingAxis)) return 0;
  const std::array<GlyphAxisId, 1> outgoing{outgoingAxis}, incoming{incomingAxis};
  const auto first = dependentMarkAxisWidthChange(current, outgoing, currentWidth, currentBaseline);
  if (!first) return std::nullopt;
  if (!next) return std::max(0.0, *first);
  const auto second = dependentMarkAxisWidthChange(*next, incoming, nextWidth, nextBaseline ? *nextBaseline : GlyphParameters{});
  if (!second) return std::nullopt;
  // One side may contract as the other extends. Charge only net connection
  // growth; clamping each side separately counts contextual compensation.
  return std::max(0.0, *first + *second);
}

// Invert a monotone mark deformation on its declared parameter grid. The
// secant estimate solves linear profiles in one probe; nonlinear profiles use
// bounded binary refinement. Returning the lower neighbour never overshoots.
// The provider caches glyph geometry, so these probes also warm final GPOS.
template <class Width>
std::optional<double> dependentMarkWidthParameter(double target, double maximum, unsigned grid, Width width) {
  if (!std::isfinite(target) || !std::isfinite(maximum) || maximum <= 0 || !grid || maximum * grid > 1000000) return std::nullopt;
  const auto first = width(0);
  const auto last = width(maximum);
  if (!first || !last || !std::isfinite(*first) || !std::isfinite(*last) || *last <= *first) return std::nullopt;
  if (target <= *first) return 0;
  if (target >= *last) return maximum;
  unsigned low = 0, high = static_cast<unsigned>(std::ceil(maximum * grid));
  if (high <= 1) return 0;
  unsigned probe = std::clamp(static_cast<unsigned>(std::floor((target - *first) / (*last - *first) * maximum * grid)), 1u, high - 1);
  while (low + 1 < high) {
    const auto measured = width(std::min(maximum, double(probe) / grid));
    if (!measured || !std::isfinite(*measured)) return std::nullopt;
    if (*measured <= target + 1e-7) low = probe;
    else high = probe;
    // Check the upper grid neighbour first: normally the secant was exact.
    probe = low + 1;
    if (probe >= high) break;
    const auto next = width(std::min(maximum, double(probe) / grid));
    if (!next || !std::isfinite(*next)) return std::nullopt;
    if (*next > target + 1e-7) break;
    low = probe;
    probe = low + (high - low) / 2;
  }
  return double(low) / grid;
}

}  // namespace digitalkhatt::justify
