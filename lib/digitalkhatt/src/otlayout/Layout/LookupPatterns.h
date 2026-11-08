#pragma once
#include "commontypes.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
class OtLayout;
namespace digitalkhatt::layout {
// Glyph-level positioning witnesses, not claims of Unicode/GSUB reachability.
struct LookupPattern {
  std::string lookup, subtable, markClass;
  std::size_t subtableIndex = 0, inputBegin = 0, inputCount = 0;
  std::vector<std::vector<std::uint16_t>> glyphSets;
  std::uint64_t combinations() const;
  std::vector<std::uint16_t> sequence(std::uint64_t index) const;
};
std::vector<LookupPattern> lookupPatterns(OtLayout&, const std::string& lookup);
struct PatternGlyph {
  std::uint16_t code = 0;
  unsigned cluster = 0;
  double x = 0, y = 0, advance = 0;
  GlyphParameters parameters;
};
struct PatternAttachment {
  std::size_t base = 0, mark = 0, subtable = 0;
  std::string lookup, markClass;
};
struct PatternPreview {
  std::vector<PatternGlyph> glyphs;
  std::vector<PatternAttachment> attachments;
  std::vector<std::size_t> appliedSubtables;
  bool selectedLookupApplied = false;
};
PatternPreview previewLookupPattern(OtLayout&, const std::string& lookup,
                                    const std::vector<std::uint16_t>&,
                                    const std::vector<GlyphParameters>&,
                                    bool precedingLookups = true);
enum class AnchorRole { Base, Mark };
struct AnchorTarget {
  std::string lookup, markClass, glyph;
  std::size_t subtable = 0;
  AnchorRole role = AnchorRole::Base;
  bool operator==(const AnchorTarget&) const = default;
};
struct AnchorDescription {
  Point resolved, calculated, adjustment;
  std::string source;
};
std::optional<Point> anchorAdjustment(OtLayout&, const AnchorTarget&);
void setAnchorAdjustment(OtLayout&, const AnchorTarget&, std::optional<Point>);
AnchorDescription describeAnchor(OtLayout&, const AnchorTarget&, const GlyphParameters&);
Point adjustmentForMove(OtLayout&, const AnchorTarget&, const GlyphParameters&, Point movement);
}  // namespace digitalkhatt::layout
