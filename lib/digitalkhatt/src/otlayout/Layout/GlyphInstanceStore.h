#pragma once

#include "commontypes.h"
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

// Immutable records, owned outside HarfBuzz. IDs are local to one OtLayout
// and remain valid until that layout is destroyed. Zero is the neutral record.
struct GlyphProvenance {
  std::uint32_t lookup_index = 0;
  std::uint32_t subtable_index = 0;
  std::uint32_t base_codepoint = 0;
  bool operator==(const GlyphProvenance&) const = default;
};

struct GlyphInstanceState {
  GlyphParameters parameters;
  // Existing lookup coordinates are kept in their original domain until read
  // with the current glyph. In normalized mode GSUB can change their scale.
  double lookupLeft = 0;
  double lookupRight = 0;
  std::optional<GlyphProvenance> positioning;
  bool operator==(const GlyphInstanceState&) const = default;
};

class GlyphInstanceStore {
  struct Hash {
    std::size_t operator()(const GlyphInstanceState& state) const {
      std::size_t hash = 0;
      auto add = [&](std::size_t value) { hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2); };
      add(digitalkhatt::hashGlyphParameters(state.parameters));
      add(std::hash<double>{}(state.lookupLeft));
      add(std::hash<double>{}(state.lookupRight));
      const auto& provenance = state.positioning;
      add(provenance.has_value());
      if (provenance) { add(provenance->lookup_index); add(provenance->subtable_index); add(provenance->base_codepoint); }
      return hash;
    }
  };

 public:
  std::uint32_t intern(const GlyphInstanceState& state) {
    for (digitalkhatt::GlyphAxisId axis = 0; axis < state.parameters.size(); ++axis)
      if (!std::isfinite(state.parameters.value(axis))) throw std::invalid_argument("Nonfinite glyph parameter");
    for (double axis : {state.lookupLeft, state.lookupRight}) {
      if (!std::isfinite(axis)) throw std::invalid_argument("Nonfinite glyph parameter");
    }
    if (state == GlyphInstanceState{}) return 0;
    if (auto found = ids.find(state); found != ids.end()) return found->second;
    if (values.size() >= std::numeric_limits<std::uint32_t>::max()) throw std::length_error("Glyph instance store exhausted");
    auto id = static_cast<std::uint32_t>(values.size());
    values.push_back(state);
    try { ids.emplace(state, id); } catch (...) { values.pop_back(); throw; }
    return id;
  }

  GlyphInstanceState get(std::uint32_t id) const { return values.at(id); }

 private:
  std::vector<GlyphInstanceState> values{GlyphInstanceState{}};
  std::unordered_map<GlyphInstanceState, std::uint32_t, Hash> ids;
};
