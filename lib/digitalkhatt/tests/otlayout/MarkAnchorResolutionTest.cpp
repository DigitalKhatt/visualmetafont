#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "MPFont.h"
#include "automedina.h"
#include "Layout/GlyphVis.h"
#include "Layout/Lookup.h"
#include "Layout/OtLayout.h"
#include "Layout/Subtable.h"
#include "digitalkhatt/project/GlyphSources.h"

namespace fs = std::filesystem;

static std::string read(const fs::path& path) {
  std::ifstream stream(path);
  if (!stream) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>{stream}, {}};
}

// Exercise shared subset resolution independently of the selected font's policy.
class AnchorGroups : public Automedina {
 public:
  AnchorGroups(OtLayout& layout, MPFont& font) : Automedina(&layout, &font, true) {
    classes["marks_topmarks"] = {"smallhighyeh", "smallhighwaw", "fatha"};
    classes["marks_smallletters"] = {"smallhighyeh", "smallhighwaw"};
    for (const auto& [name, members] : classes) layout.addClass(name, members);
  }
  Lookup* getLookup(std::string) override { return nullptr; }
  CalcAnchor getanchorCalcFunctions(std::string, Subtable*) override { return {}; }
  using Automedina::resolveMarkClassAnchor;
};

int main(int argc, char** argv) {
  if (argc != 2) return 2;
  try {
    const auto project = fs::absolute(argv[1]);
    const fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
    MPFont font;
    font.initialize("MPGUI:=1;" + read(resources / "mfplain.mp") +
        read(resources / "mpost.mp") + read(resources / "vmf.mp") + read(project), project);
    digitalkhatt::project::ProjectGlyphSources::load(project).registerWith(font);
    OtLayout layout(&font, true, false);
    layout.loadLookupFile((project.parent_path() / "features.fea").string());
    Lookup lookup(&layout);
    lookup.name = "test.anchor.precedence";
    MarkBaseSubtable table(&lookup);
    table.name = "probe";
    const std::string className = "testmarkclass";
    const auto scopedName = lookup.name + "_" + className;
    auto& glyph = layout.glyphs.at("alef.isol");
    auto& markClass = table.classes[className];
    markClass.baseparameters[glyph.name] = Point{7, 11};
    markClass.markparameters[glyph.name] = Point{13, 17};
    auto put = [&](const std::string& name, Point point) {
      glyph.anchors.insert_or_assign({name, GlyphVis::AnchorType::MarkAnchor},
          GlyphVisAnchor{point, int(GlyphVis::AnchorType::MarkAnchor)});
    };
    auto erase = [&](const std::string& name) {
      glyph.anchors.erase({name, GlyphVis::AnchorType::MarkAnchor});
    };
    auto check = [&](Point anchor, const char* message) {
      if (table.getBaseAnchor(glyph.name, className, {}) != anchor + Point{7, 11} ||
          table.getMarkAnchor(glyph.name, className, {}) != anchor + Point{13, 17})
        throw std::runtime_error(message);
    };
    markClass.basefunction = markClass.markfunction = [](
        std::string, std::string, Point adjust, GlyphParameters) {
      return Point{700, 800} + adjust;
    };
    put(className, Point{100, 200});
    put(scopedName, Point{300, 400});
    put(lookup.name, Point{500, 600});
    check(Point{300, 400}, "Lookup/class anchor must override the shared class anchor in both roles");
    const auto rolePrefix = lookup.name + "_" + table.name + "_" + className;
    put(rolePrefix + "_base", Point{40, 50});
    put(rolePrefix + "_mark", Point{60, 70});
    check(Point{300, 400}, "Existing lookup/class overrides must retain precedence over role defaults");
    erase(scopedName);
    erase(className);
    erase(lookup.name);
    if (table.getBaseAnchor(glyph.name, className, {}) != Point{47, 61} ||
        table.getMarkAnchor(glyph.name, className, {}) != Point{73, 87})
      throw std::runtime_error("Explicit roles must remain distinct when one glyph serves both roles");
    erase(rolePrefix + "_base");
    erase(rolePrefix + "_mark");
    put(className, Point{100, 200});
    put(lookup.name, Point{500, 600});
    check(Point{100, 200}, "Shared class anchor must override the lookup-wide fallback");
    erase(className);
    check(Point{500, 600}, "Lookup-wide anchor must override calculation functions");
    erase(lookup.name);
    check(Point{700, 800}, "Calculation fallback must retain per-role adjustments");
    markClass.basefunction = markClass.markfunction = {};
    markClass.baseanchors[glyph.name] = markClass.markanchors[glyph.name] = Point{900, 1000};
    check(Point{900, 1000}, "Stored coordinates must remain the final fallback");
    AnchorGroups groups(layout, font);
    auto groupBase = [&] {
      return groups.resolveMarkClassAnchor(glyph, table, className, true)
          .value_or(Point{900, 1000}) + Point{7, 11};
    };
    auto groupMark = [&] {
      return groups.resolveMarkClassAnchor(glyph, table, className, true, "mark")
          .value_or(Point{900, 1000}) + Point{13, 17};
    };
    markClass.markCodes = {layout.glyphCodePerName.at("smallhighyeh")};
    groups.prepareAnchorResolution();
    put("marks_topmarks", Point{100, 200});
    put("marks_smallletters", Point{300, 400});
    if (groupBase() != Point{907, 1011})
      throw std::runtime_error("A lookup-qualified group must not leak into another lookup");
    lookup.name = "marks";
    groups.prepareAnchorResolution();
    if (groupBase() != Point{307, 411})
      throw std::runtime_error("The smallest containing lookup-qualified group must win");
    erase("marks_smallletters");
    if (groupBase() != Point{107, 211})
      throw std::runtime_error("A missing narrower group must allow the broader authored group");
    if (groupMark() != Point{913, 1017})
      throw std::runtime_error("Recipient group points must not replace attached-mark origins");
    put("marks_topmarks_mark", Point{200, 300});
    put("marks_smallletters_mark", Point{400, 500});
    if (groupMark() != Point{413, 517})
      throw std::runtime_error("The narrowest authored incoming group must retain mark adjustments");
    if (groupBase() != Point{107, 211})
      throw std::runtime_error("Incoming group points must not replace recipient points");
    markClass.markCodes.insert(layout.glyphCodePerName.at("fatha"));
    groups.prepareAnchorResolution();
    if (groupMark() != Point{213, 317})
      throw std::runtime_error("Incoming groups must contain the entire positioning class");
    const auto incomingOverride = lookup.name + "_" + table.name + "_" + className + "_mark";
    put(incomingOverride, Point{800, 900});
    if (table.getMarkAnchor(glyph.name, className, {}) != Point{813, 917})
      throw std::runtime_error("An explicit incoming role anchor must override subset matching");
    erase(incomingOverride);
    lookup.name = "test.anchor.precedence";
    groups.prepareAnchorResolution();
    if (groupMark() != Point{913, 1017})
      throw std::runtime_error("Lookup-scoped incoming groups must not leak into another lookup");
    std::cout << "Mark anchor precedence and adjustments passed for both attachment roles\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
