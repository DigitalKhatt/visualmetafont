#include "LookupPatterns.h"
#include "OtLayout.h"
#include "Subtable.h"
#include "Lookup.h"
#include "GlyphVis.h"
#include "MPFont.h"
#include "digitalkhatt/project/GlyphSources.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <algorithm>
using namespace digitalkhatt::layout;
namespace fs = std::filesystem;
static std::string read(const fs::path& p) {
  std::ifstream s(p);
  return {std::istreambuf_iterator<char>(s), {}};
}
static void require(bool b, const char* m) {
  if (!b) throw std::runtime_error(m);
}
int main(int argc, char** argv) {
  if (argc != 2) return 2;
  try {
    LookupPattern lazy;
    for (int i = 0; i < 50; ++i) lazy.glyphSets.push_back({1, 2, 3});
    require(lazy.combinations() == UINT64_MAX,
            "Large combinations must saturate without allocating sequences");
    require(lazy.sequence(1000).size() == 50, "A large pattern must support indexed access");
    LookupPattern small;
    small.glyphSets = {{1, 2}, {3, 4, 5}};
    require(small.combinations() == 6 && small.sequence(4) == std::vector<std::uint16_t>({2, 4}),
            "Mixed-radix paging must enumerate each combination once");
    bool rejected = false;
    try {
      small.sequence(6);
    } catch (const std::out_of_range&) {
      rejected = true;
    }
    require(rejected, "Out-of-range pattern indices must reject");
    MPFont font;
    const auto project = fs::absolute(argv[1]);
    const fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    font.initialize("MPGUI:=1;" + read(resources / "mfplain.mp") + read(resources / "mpost.mp") +
                        read(resources / "vmf.mp") + read(project),
                    project);
    digitalkhatt::project::ProjectGlyphSources::load(project).registerWith(font);
    OtLayout layout(&font, true, true);
    layout.useNormAxisValues = false;
    layout.loadLookupFile("features.fea");
    const std::string markLookup = layout.lookupsIndexByName.contains("marks")
        ? "marks" : "defaultmarkposition";
    {
      bool released = false;
      static hb_user_data_key_t key;
      auto* previewFont = layout.createFont(1, true);
      hb_font_set_user_data(
          hb_font_get_parent(previewFont), &key, &released,
          [](void* value) { *static_cast<bool*>(value) = true; }, true);
      hb_font_destroy(previewFont);
      require(released, "Destroying a preview font must release its parent and face reference");
    }
    auto patterns = lookupPatterns(layout, markLookup);
    require(!patterns.empty(), "Direct mark subtables must expose pattern groups");
    std::vector<std::uint16_t> sequence{std::uint16_t(layout.glyphCodePerName.at("behshape.init")),
                                        std::uint16_t(layout.glyphCodePerName.at("fatha"))};
    std::vector<GlyphParameters> params(2);
    auto preview = previewLookupPattern(layout, markLookup, sequence, params, false);
    require(preview.selectedLookupApplied,
            "The preview must apply the selected positioning lookup");
    require(preview.glyphs.size() == 2 && !preview.attachments.empty(),
            "Applied mark positioning must expose the actual attachment");
    auto attachment = preview.attachments.front();
    require(attachment.base == 0 && attachment.mark == 1,
            "Parent indices must follow the GPOS attachment chain");
    AnchorTarget target{attachment.lookup, attachment.markClass, "behshape.init",
                        attachment.subtable, AnchorRole::Base};
    AnchorTarget mark{attachment.lookup, attachment.markClass, "fatha", attachment.subtable,
                      AnchorRole::Mark};
    auto basePoint = describeAnchor(layout, target, {});
    auto markPoint = describeAnchor(layout, mark, {});
    require(int(preview.glyphs[0].x) + basePoint.resolved.x() ==
                int(preview.glyphs[1].x) + markPoint.resolved.x(),
            "Positioned attachment X coordinates must align");
    require(int(preview.glyphs[0].y) + basePoint.resolved.y() ==
                int(preview.glyphs[1].y) + markPoint.resolved.y(),
            "Positioned attachment Y coordinates must align");
    const auto original = anchorAdjustment(layout, target);
    const auto originalDirty = layout.dirty;
    describeAnchor(layout, target, {});
    require(anchorAdjustment(layout, target) == original && layout.dirty == originalDirty,
            "Inspecting an anchor must not alter adjustments or dirty state");
    auto moved = adjustmentForMove(layout, target, {}, Point{20, -10});
    setAnchorAdjustment(layout, target, moved);
    auto after = describeAnchor(layout, target, {});
    require(after.resolved == basePoint.resolved + Point{20, -10},
            "Dragging must account for the function's adjustment response");
    setAnchorAdjustment(layout, target, original);
    auto unchanged = previewLookupPattern(layout, markLookup, sequence, params, false);
    require(unchanged.glyphs[1].x == preview.glyphs[1].x &&
                unchanged.glyphs[1].y == preview.glyphs[1].y,
            "Restoring an adjustment must restore shaped positioning");
    // Saved adjustments use the same document as the Mushaf editor.
    ParameterJsonObject saved;
    layout.saveParameters(saved);
    setAnchorAdjustment(layout, target, Point{37, -12});
    ParameterJsonObject edited;
    layout.saveParameters(edited);
    std::string json;
    require(!writeParameterJson(edited, json), "Edited parameters must serialize");
    ParameterJsonObject loaded;
    require(!glz::read_json(loaded, json), "Saved parameters must parse");
    setAnchorAdjustment(layout, target, std::nullopt);
    layout.readParameters(loaded);
    require(anchorAdjustment(layout, target) == Point{37, -12},
            "Adjustment must survive a save/reload round trip");
    // readParameters overlays a document; an absent entry is restored explicitly.
    layout.readParameters(saved);
    setAnchorAdjustment(layout, target, original);
    require(anchorAdjustment(layout, target) == original,
            "Round-trip inspection must restore original state");
    const auto contextual = lookupPatterns(layout, "adjusttopmarks.l6").front();
    auto contextualTarget = AnchorTarget{contextual.lookup, contextual.markClass, "behshape.init",
                                         contextual.subtableIndex, AnchorRole::Base};
    ParameterJsonObject beforeInspection, afterInspection;
    layout.saveParameters(beforeInspection);
    GlyphParameters extended;
    extended.lefttatweel = 1;
    describeAnchor(layout, contextualTarget, extended);
    layout.saveParameters(afterInspection);
    std::string beforeJson, afterJson;
    writeParameterJson(beforeInspection, beforeJson);
    writeParameterJson(afterInspection, afterJson);
    require(beforeJson == afterJson,
            "Inspecting a stretched default must not insert parameter records");
    auto chains = lookupPatterns(layout, "lowmarkafterwawandreh.mark");
    require(!chains.empty(),
            "Contextual positioning must expose backtrack, input and lookahead witnesses");
    bool applied = false;
    for (const auto& p : chains) {
      for (std::uint64_t i = 0; i < std::min<std::uint64_t>(p.combinations(), 30); ++i) {
        auto s = p.sequence(i);
        auto r =
            previewLookupPattern(layout, p.lookup, s, std::vector<GlyphParameters>(s.size()), true);
        if (r.selectedLookupApplied &&
            std::find(r.appliedSubtables.begin(), r.appliedSubtables.end(), p.subtableIndex) !=
                r.appliedSubtables.end()) {
          applied = true;
          break;
        }
      }
      if (applied) break;
    }
    require(applied, "At least one generated contextual witness must exercise its selected lookup");
    std::cout << "Lookup patterns: lazy paging, GPOS witnesses, parent alignment, scoped "
                 "adjustments and restoration passed\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
