#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace digitalkhatt {

using GlyphAxisId = std::uint32_t;
inline constexpr GlyphAxisId NoGlyphAxis = std::numeric_limits<GlyphAxisId>::max();
inline constexpr GlyphAxisId LeftTatweelAxis = 0;
inline constexpr GlyphAxisId RightTatweelAxis = 1;
inline constexpr GlyphAxisId ThirdAxis = 2;
inline constexpr GlyphAxisId FourthAxis = 3;
inline constexpr GlyphAxisId FifthAxis = 4;
inline constexpr GlyphAxisId ScaleXAxis = 5;

// Value stored directly on glyphs and external HarfBuzz instances. The common
// coordinates remain named for compatibility, while additional slots have no
// fixed limit and are addressed through value()/set().
struct GlyphParameters {
  double lefttatweel = 0, righttatweel = 0, third = 0, fourth = 0, fifth = 0, scalex = 0;
  std::vector<double> additional;

  double value(GlyphAxisId axis) const {
    switch (axis) {
      case LeftTatweelAxis: return lefttatweel;
      case RightTatweelAxis: return righttatweel;
      case ThirdAxis: return third;
      case FourthAxis: return fourth;
      case FifthAxis: return fifth;
      case ScaleXAxis: return scalex;
      default: return axis - 6 < additional.size() ? additional[axis - 6] : 0;
    }
  }
  void set(GlyphAxisId axis, double value) {
    if (axis == NoGlyphAxis || !std::isfinite(value)) throw std::invalid_argument("Invalid glyph parameter");
    switch (axis) {
      case LeftTatweelAxis: lefttatweel = value; break;
      case RightTatweelAxis: righttatweel = value; break;
      case ThirdAxis: third = value; break;
      case FourthAxis: fourth = value; break;
      case FifthAxis: fifth = value; break;
      case ScaleXAxis: scalex = value; break;
      default:
        if (axis - 6 >= additional.size()) {
          if (value == 0) return;
          additional.resize(static_cast<std::size_t>(axis) - 5, 0);
        }
        additional[axis - 6] = value;
    }
  }
  std::size_t size() const {
    auto count = additional.size();
    while (count && additional[count - 1] == 0) --count;
    return 6 + count;
  }
  bool hasExtraAxes() const { return third != 0 || fourth != 0 || fifth != 0 || size() > 6; }
  bool isDefault() const { return lefttatweel == 0 && righttatweel == 0 && scalex == 0 && !hasExtraAxes(); }
  GlyphParameters& operator+=(const GlyphParameters& other) {
    for (GlyphAxisId axis = 0; axis < other.size(); ++axis) {
      const auto left = value(axis);
      const auto right = other.value(axis);
      auto sum = left + right;
      const auto scale = std::max({1.0, std::abs(left), std::abs(right)});
      if (std::abs(sum) <= 8 * std::numeric_limits<double>::epsilon() * scale)
        sum = 0;
      set(axis, sum);
    }
    return *this;
  }
  friend GlyphParameters operator+(GlyphParameters lhs,
                                   const GlyphParameters& rhs) {
    lhs += rhs;
    return lhs;
  }
  bool operator==(const GlyphParameters& other) const {
    const auto count = size();
    if (count != other.size()) return false;
    for (GlyphAxisId i = 0; i < count; ++i) if (value(i) != other.value(i)) return false;
    return true;
  }
};

inline std::size_t hashGlyphParameters(const GlyphParameters& parameters) {
  std::size_t hash = 0;
  for (GlyphAxisId axis = 0; axis < parameters.size(); ++axis) {
    const auto value = std::hash<double>{}(parameters.value(axis));
    hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
  }
  return hash;
}

struct GlyphAxisDefinition {
  std::string name;
  unsigned metaPostIndex;
};

// Dense slots are separate from MetaPost suffixes (scalex maps to params100).
// Registry configuration happens at font load, not during glyph processing.
class GlyphAxisRegistry {
 public:
  GlyphAxisRegistry() {
    for (const auto& axis : std::vector<GlyphAxisDefinition>{{"lefttatweel", 0}, {"righttatweel", 1}, {"third", 2}, {"fourth", 3}, {"fifth", 4}, {"scalex", 100}})
      add(axis.name, axis.metaPostIndex);
    names_.emplace("body", 2);
  }
  GlyphAxisId find(std::string_view name) const {
    const auto found = names_.find(std::string(name));
    return found == names_.end() ? NoGlyphAxis : found->second;
  }
  GlyphAxisId findMetaPostIndex(unsigned metaPostIndex) const {
    for (GlyphAxisId i = 0; i < axes_.size(); ++i)
      if (axes_[i].metaPostIndex == metaPostIndex) return i;
    return NoGlyphAxis;
  }
  GlyphAxisId add(std::string name, unsigned metaPostIndex) {
    if (const auto found = find(name); found != NoGlyphAxis) {
      if (axes_[found].metaPostIndex != metaPostIndex) throw std::invalid_argument("Glyph axis name has conflicting MetaPost index");
      return found;
    }
    for (GlyphAxisId i = 0; i < axes_.size(); ++i) if (axes_[i].metaPostIndex == metaPostIndex) { names_.emplace(std::move(name), i); return i; }
    if (axes_.size() >= NoGlyphAxis) throw std::length_error("Too many glyph axes");
    auto id = static_cast<GlyphAxisId>(axes_.size());
    names_.emplace(name, id);
    axes_.push_back({std::move(name), metaPostIndex});
    return id;
  }
  const std::vector<GlyphAxisDefinition>& axes() const { return axes_; }
 private:
  std::vector<GlyphAxisDefinition> axes_;
  std::unordered_map<std::string, GlyphAxisId> names_;
};

} // namespace digitalkhatt

namespace std {
template<> struct hash<digitalkhatt::GlyphParameters> {
  size_t operator()(const digitalkhatt::GlyphParameters& p) const { return digitalkhatt::hashGlyphParameters(p); }
};
}
