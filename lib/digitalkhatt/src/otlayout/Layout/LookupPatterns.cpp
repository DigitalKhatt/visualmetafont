#include "LookupPatterns.h"
#include "OtLayout.h"
#include "Lookup.h"
#include "Subtable.h"
#include "GlyphVis.h"
#include "hb-ot-layout-gpos-table.hh"
#include <algorithm>
#include <limits>
#include <cmath>
#include <functional>
#include <set>
#include <stdexcept>

namespace digitalkhatt::layout {
std::uint64_t LookupPattern::combinations() const {
  std::uint64_t n = 1;
  if (glyphSets.empty()) return 0;
  for (const auto& slot : glyphSets) {
    if (slot.empty()) return 0;
    if (n > std::numeric_limits<std::uint64_t>::max() / slot.size())
      return std::numeric_limits<std::uint64_t>::max();
    n *= slot.size();
  }
  return n;
}
std::vector<std::uint16_t> LookupPattern::sequence(std::uint64_t index) const {
  if (index >= combinations()) throw std::out_of_range("Pattern case index");
  std::vector<std::uint16_t> result(glyphSets.size());
  for (std::size_t i = glyphSets.size(); i-- > 0;) {
    result[i] = glyphSets[i][index % glyphSets[i].size()];
    index /= glyphSets[i].size();
  }
  return result;
}
static std::vector<std::uint16_t> sorted(const std::unordered_set<std::uint16_t>& v) {
  std::vector<std::uint16_t> result(v.begin(), v.end());
  std::sort(result.begin(), result.end());
  return result;
}
std::vector<LookupPattern> lookupPatterns(OtLayout& layout, const std::string& name) {
  auto found = layout.lookupsIndexByName.find(name);
  if (found == layout.lookupsIndexByName.end())
    throw std::invalid_argument("Unknown lookup: " + name);
  auto* lookup = layout.lookups.at(found->second);
  std::vector<LookupPattern> result;
  for (std::size_t i = 0; i < lookup->subtables.size(); ++i) {
    auto* table = lookup->subtables[i];
    if (auto* mark = dynamic_cast<MarkBaseSubtable*>(table)) {
      std::set<std::uint16_t> bases(mark->sortedBaseCodes.begin(), mark->sortedBaseCodes.end());
      for (const auto& expr : mark->base) {
        auto codes = layout.classToUnicode(expr);
        bases.insert(codes.begin(), codes.end());
      }
      for (const auto& [cls, value] : mark->classes) {
        auto classBases = bases;
        for (const auto& [glyph, _] : value.baseanchors)
          if (layout.glyphCodePerName.contains(glyph))
            classBases.insert(layout.glyphCodePerName.at(glyph));
        auto marks = value.markCodes;
        for (const auto& expr : value.mark) {
          auto codes = layout.classToUnicode(expr);
          marks.insert(codes.begin(), codes.end());
        }
        for (const auto& [glyph, _] : value.markanchors)
          if (layout.glyphCodePerName.contains(glyph))
            marks.insert(layout.glyphCodePerName.at(glyph));
        LookupPattern p{name,
                        table->name,
                        cls,
                        i,
                        0,
                        2,
                        {{classBases.begin(), classBases.end()}, sorted(marks)}};
        if (p.combinations()) result.push_back(std::move(p));
      }
    } else if (auto* chain = dynamic_cast<ChainingSubtable*>(table)) {
      LookupPattern p{name,
                      table->name,
                      {},
                      i,
                      chain->compiledRule.backtrack.size(),
                      chain->compiledRule.input.size(),
                      {}};
      for (const auto* context : {&chain->compiledRule.backtrack, &chain->compiledRule.input,
                                  &chain->compiledRule.lookahead})
        for (const auto& slot : *context) p.glyphSets.push_back(sorted(slot));
      if (p.combinations()) result.push_back(std::move(p));
    }
  }
  // Start with ordinary Arabic glyphs; keep fallback and construction cases
  // available without making .notdef the first variation-editing example.
  auto rank = [&](std::uint16_t code) {
    const auto& glyph = layout.glyphs.at(layout.glyphNamePerCode.at(code));
    if (glyph.name == "notdef" || glyph.name == "null" || glyph.name == "space" ||
        glyph.name == "linefeed")
      return 2;
    return glyph.unicode >= 0x620 && glyph.unicode <= 0x64a ? 0 : 1;
  };
  for (auto& p : result)
    for (auto& slot : p.glyphSets)
      std::stable_sort(slot.begin(), slot.end(), [&](auto a, auto b) { return rank(a) < rank(b); });
  return result;
}
PatternPreview previewLookupPattern(OtLayout& layout, const std::string& name,
                                    const std::vector<std::uint16_t>& sequence,
                                    const std::vector<GlyphParameters>& parameters,
                                    bool preceding) {
  if (sequence.empty() || sequence.size() > 256 || sequence.size() != parameters.size())
    throw std::invalid_argument("Invalid glyph pattern");
  layout.getGPOS();
  const auto selected = layout.gposlookupsIndexByName.at(name);
  hb_font_t* font = layout.createFont(1, true);
  hb_buffer_t* buffer = hb_buffer_create();
  struct Cleanup {
    hb_font_t* font;
    hb_buffer_t* buffer;
    ~Cleanup() {
      hb_buffer_destroy(buffer);
      hb_font_destroy(font);
    }
  } cleanup{font, buffer};
  hb_buffer_set_direction(buffer, HB_DIRECTION_RTL);
  hb_buffer_set_script(buffer, HB_SCRIPT_ARABIC);
  hb_buffer_set_language(buffer, hb_language_from_string("ar", -1));
  for (unsigned i = 0; i < sequence.size(); ++i) buffer->add(sequence[i], i);
  hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_GLYPHS);
  buffer->clear_positions();
  _hb_buffer_allocate_gsubgpos_vars(buffer);
  hb_ot_layout_substitute_start(font, buffer);
  for (unsigned i = 0; i < buffer->len; ++i) {
    buffer->info[i].mask = 1;
    layout.setGlyphParameters(buffer->info[i], parameters[i]);
  }
  hb_font_get_glyph_h_advances(font, buffer->len, &buffer->info[0].codepoint,
                               sizeof(hb_glyph_info_t), &buffer->pos[0].x_advance,
                               sizeof(hb_glyph_position_t));
  for (unsigned i = 0; i < buffer->len; ++i)
    if (layout.glyphGlobalClasses[sequence[i]] == OtLayout::MarkGlyph) buffer->pos[i].x_advance = 0;
  hb_ot_layout_position_start(font, buffer);
  std::set<unsigned> indices;
  if (preceding)
    for (const auto& [feature, lookups] : layout.positioningFeatures())
      if (feature != "tjwd")
        for (auto index : lookups)
          if (index < unsigned(selected)) indices.insert(index);
  indices.insert(selected);
  PatternPreview result;
  std::set<std::string> editableLookups;
  std::function<void(const std::string&)> collect = [&](const std::string& current) {
    if (!editableLookups.insert(current).second) return;
    auto* lookup = layout.lookups.at(layout.lookupsIndexByName.at(current));
    for (auto* table : lookup->subtables)
      if (auto* chain = dynamic_cast<ChainingSubtable*>(table))
        for (const auto& record : chain->compiledRule.lookupRecords) collect(record.lookupName);
  };
  collect(name);
  {
    OT::hb_ot_apply_context_t c(1u, font, buffer);
    c.set_recurse_func(OT::PosLookup::template dispatch_recurse_func<OT::hb_ot_apply_context_t>);
    c.set_lookup_mask(1);
    c.set_auto_zwj(1);
    c.set_auto_zwnj(1);
    for (auto index : indices) {
      if (layout.disabledLookupNames().contains(layout.gposlookups[index]->name)) continue;
      const auto& lookup = font->face->table.GPOS->table->get_lookup(index);
      auto* accel = font->face->table.GPOS->get_accel(index);
      if (!accel) continue;
      c.set_lookup_index(index);
      c.set_lookup_props(lookup.get_props());
      buffer->idx = 0;
      const bool cache = accel->cache_enter(&c);
      while (buffer->idx < buffer->len && buffer->successful) {
        const auto previousIndex = buffer->idx;
        bool applied = false;
        if (accel->digest.may_have(buffer->cur().codepoint) &&
            (buffer->cur().mask & c.lookup_mask) &&
            c.check_glyph_property(&buffer->cur(), c.lookup_props))
          for (unsigned subtable = 0; subtable < lookup.get_subtable_count(); ++subtable) {
            c.subtable_index = subtable;
            if (lookup.get_subtable(subtable).dispatch(&c, lookup.get_type())) {
              applied = true;
              if (index == unsigned(selected)) {
                result.selectedLookupApplied = true;
                result.appliedSubtables.push_back(subtable);
              }
              break;
            }
          }
        if (applied && buffer->idx <= previousIndex) {
          if (cache) accel->cache_leave(&c);
          throw std::runtime_error("Preview lookup did not advance: " +
                                   layout.gposlookups[index]->name);
        }
        if (!applied) buffer->next_glyph();
      }
      if (cache) accel->cache_leave(&c);
    }
  }
  std::vector<int> parents(buffer->len, -1);
  for (unsigned i = 0; i < buffer->len; ++i)
    if (buffer->pos[i].attach_chain()) parents[i] = int(i) + buffer->pos[i].attach_chain();
  hb_ot_layout_position_finish_advances(font, buffer);
  hb_ot_layout_position_finish_offsets(font, buffer);
  // Keep logical glyph order. Place the first logical glyph at the right.
  double pen = 0;
  for (unsigned i = 0; i < buffer->len; ++i) {
    pen -= buffer->pos[i].x_advance;
    result.glyphs.push_back({std::uint16_t(buffer->info[i].codepoint), buffer->info[i].cluster,
                             pen + buffer->pos[i].x_offset, double(buffer->pos[i].y_offset),
                             double(buffer->pos[i].x_advance),
                             layout.glyphParameters(buffer->info[i])});
    const auto state = layout.glyphInstanceState(buffer->info[i]);
    if (!state.positioning || parents[i] < 0 || parents[i] >= int(buffer->len)) continue;
    const auto& provenance = *state.positioning;
    if (provenance.lookup_index >= layout.gposlookups.size()) continue;
    auto* owner = layout.gposlookups[provenance.lookup_index];
    if (provenance.subtable_index >= owner->subtables.size()) continue;
    auto* mark = dynamic_cast<MarkBaseSubtable*>(owner->subtables[provenance.subtable_index]);
    if (!mark || !editableLookups.contains(owner->name)) continue;
    const auto cls = mark->markCodes.find(sequence[i]);
    if (cls == mark->markCodes.end()) continue;
    result.attachments.push_back({std::size_t(parents[i]), i, provenance.subtable_index,
                                  owner->name, mark->classNamebyIndex.at(cls->second)});
  }
  _hb_buffer_deallocate_gsubgpos_vars(buffer);
  return result;
}
static MarkBaseSubtable& targetTable(OtLayout& layout, const AnchorTarget& target) {
  auto* lookup = layout.lookups.at(layout.lookupsIndexByName.at(target.lookup));
  auto* table = dynamic_cast<MarkBaseSubtable*>(lookup->subtables.at(target.subtable));
  if (!table || !table->classes.contains(target.markClass) || !layout.glyphs.contains(target.glyph))
    throw std::invalid_argument("Invalid anchor target");
  return *table;
}
std::optional<Point> anchorAdjustment(OtLayout& layout, const AnchorTarget& target) {
  auto& cls = targetTable(layout, target).classes.at(target.markClass);
  const auto& values = target.role == AnchorRole::Base ? cls.baseparameters : cls.markparameters;
  auto found = values.find(target.glyph);
  return found == values.end() ? std::nullopt : std::optional<Point>{found->second};
}
static void writeAdjustment(OtLayout& layout, const AnchorTarget& target,
                            std::optional<Point> value, bool dirty) {
  auto& table = targetTable(layout, target);
  auto& cls = table.classes.at(target.markClass);
  auto& values = target.role == AnchorRole::Base ? cls.baseparameters : cls.markparameters;
  if (value)
    values[target.glyph] = *value;
  else
    values.erase(target.glyph);
  if (dirty) {
    table.markDirty();
    layout.dirty = true;
  }
}
void setAnchorAdjustment(OtLayout& layout, const AnchorTarget& target, std::optional<Point> value) {
  writeAdjustment(layout, target, value, true);
}
AnchorDescription describeAnchor(OtLayout& layout, const AnchorTarget& target,
                                 const GlyphParameters& parameters) {
  auto& table = targetTable(layout, target);
  auto& cls = table.classes.at(target.markClass);
  auto resolve = [&] {
    return target.role == AnchorRole::Base
               ? table.getBaseAnchor(target.glyph, target.markClass, parameters)
               : table.getMarkAnchor(target.glyph, target.markClass, parameters);
  };
  auto saved = anchorAdjustment(layout, target);
  AnchorDescription result;
  result.adjustment = saved.value_or(Point{});
  result.resolved = resolve();
  writeAdjustment(layout, target, std::nullopt, false);
  try {
    result.calculated = resolve();
  } catch (...) {
    writeAdjustment(layout, target, saved, false);
    throw;
  }
  writeAdjustment(layout, target, saved, false);
  auto* glyph = layout.glyphs.at(target.glyph).getAlternate(parameters);
  const auto role = target.role == AnchorRole::Base ? "base" : "mark";
  std::vector<std::string> names;
  if (target.role == AnchorRole::Mark) {
    names.push_back(target.lookup + "_" + target.markClass + "_mark");
    names.push_back(target.markClass + "_mark");
  }
  for (auto n : {target.lookup + "_" + target.markClass, target.markClass, target.lookup,
                 target.lookup + "_" + table.name + "_" + target.markClass + "_" + role})
    names.push_back(n);
  for (const auto& name : names)
    if (glyph->conatinsAnchor(name, GlyphVis::AnchorType::MarkAnchor)) {
      result.source = "Glyph anchor: " + name;
      return result;
    }
  const auto authored = target.role == AnchorRole::Base
                            ? layout.getAnchor(*glyph, table, target.markClass, parameters)
                            : layout.getMarkAnchor(*glyph, table, target.markClass, parameters);
  if (authored)
    result.source = "Glyph anchor group";
  else if (target.role == AnchorRole::Base ? bool(cls.basefunction) : bool(cls.markfunction))
    result.source = "Calculated by anchor function";
  else
    result.source = "Explicit lookup coordinates";
  return result;
}
Point adjustmentForMove(OtLayout& layout, const AnchorTarget& target, const GlyphParameters& p,
                        Point movement) {
  const auto saved = anchorAdjustment(layout, target);
  const Point old = saved.value_or(Point{});
  const auto at = describeAnchor(layout, target, p).resolved;
  try {
    writeAdjustment(layout, target, old + Point{1, 0}, false);
    auto dx = describeAnchor(layout, target, p).resolved - at;
    writeAdjustment(layout, target, old + Point{0, 1}, false);
    auto dy = describeAnchor(layout, target, p).resolved - at;
    writeAdjustment(layout, target, saved, false);
    double det = double(dx.x()) * dy.y() - double(dy.x()) * dx.y();
    if (det == 0) throw std::runtime_error("This anchor does not respond to both adjustment axes");
    return old + Point{int(std::lround(
                           (double(movement.x()) * dy.y() - double(movement.y()) * dy.x()) / det)),
                       int(std::lround(
                           (double(movement.y()) * dx.x() - double(movement.x()) * dx.y()) / det))};
  } catch (...) {
    writeAdjustment(layout, target, saved, false);
    throw;
  }
}
}  // namespace digitalkhatt::layout
