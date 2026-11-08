#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "MPFont.h"
#include "Layout/OtLayout.h"
#include "Layout/Lookup.h"
#include "Layout/Subtable.h"
#include "Layout/GlyphVis.h"
#include "hb-font.hh"

namespace fs = std::filesystem;

static void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static std::string read(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>{file}, {}};
}

static void initialize(MPFont& font, const fs::path& project) {
  const fs::path resources{DIGITALKHATT_METAFONT_RESOURCES};
  font.initialize("MPGUI:=1;" + read(resources / "mfplain.mp") +
      read(resources / "mpost.mp") + read(resources / "vmf.mp") + read(project), project);
  const auto sources = read(project.parent_path() / "glyphs.mp");
  for (std::size_t pos = 0; pos < sources.size();) {
    const auto begin = sources.find("beginchar", pos), def = sources.find("defchar", pos);
    const bool isDef = def != std::string::npos && (begin == std::string::npos || def < begin);
    const auto start = isDef ? def : begin;
    if (start == std::string::npos) break;
    const std::string ending = isDef ? "enddefchar;" : "endchar;";
    const auto end = sources.find(ending, start);
    require(end != std::string::npos, "Unterminated glyph source");
    const auto block = sources.substr(start, end + ending.size() - start);
    const auto paren = block.find('('), first = block.find(',', paren), second = block.find(',', first + 1);
    font.registerGlyphSource(block.substr(paren + 1, first - paren - 1), block,
        isDef ? "defchar" : "beginchar", std::stoi(block.substr(first + 1, second - first - 1)));
    font.execute("params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;" + block);
    pos = end + ending.size();
  }
}

static std::uint16_t u16(const digitalkhatt::ByteBuffer& bytes, std::size_t offset) {
  return (unsigned(bytes.data()[offset]) << 8) | bytes.data()[offset + 1];
}

int main(int argc, char** argv) {
  if (argc != 2) return 2;
  const fs::path project = fs::absolute(argv[1]);
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto temporary = fs::temp_directory_path() / ("singlepos-functions-" + std::to_string(stamp));
  fs::create_directory(temporary);
  try {
    MPFont font;
    auto fixture = project;
#ifdef DIGITALKHATT_SINGLEPOS_TEST_FONT
    fixture = temporary / "singlepostest.mp";
    fs::copy_file(project, fixture);
    fs::copy_file(project.parent_path() / "glyphs.mp", temporary / "glyphs.mp");
    if (fs::exists(project.parent_path() / "parameters.json"))
      fs::copy_file(project.parent_path() / "parameters.json", temporary / "parameters.json");
    const fs::path library{DIGITALKHATT_SINGLEPOS_TEST_FONT};
    fs::copy_file(library, temporary / library.filename());
#endif
    initialize(font, fixture);
    OtLayout layout(&font, true, false);
    const auto source = read(project.parent_path() / "features.fea");
    const auto path = temporary / "functions.fea";
    std::ofstream(path) << source << R"(
lookup test.single.named {
  feature test;
  pos heh.init <function test.singlepos.width>;
} test.single.named;
lookup test.single.inline {
  feature test;
  pos waw.isol heh.init'<function test.singlepos.width>;
} test.single.inline;
feature test {
  lookup test.single.named;
  lookup test.single.inline;
} test;
)";
    layout.loadLookupFile(path.string());
    const auto code = layout.glyphCodePerName.at("heh.init");
    const auto named = layout.lookups.at(layout.lookupsIndexByName.at("test.single.named"));
    auto* values = static_cast<SingleAdjustmentSubtable*>(named->subtables.at(0));
    GlyphParameters parameters;
    parameters.lefttatweel = 1;
    auto* glyph = layout.getAlternate(code, parameters);
    require(values->getValue(code, glyph).xAdvance == -2,
        "SinglePos must evaluate the actual fractional instance width");
    const auto inlineLookup = layout.lookups.at(layout.lookupsIndexByName.at("test.single.inline"));
    const auto* context = static_cast<ChainingSubtable*>(inlineLookup->subtables.at(0));
    const auto child = context->compiledRule.lookupRecords.at(0).lookupName;
    auto* inlineValues = static_cast<SingleAdjustmentSubtable*>(
        layout.lookups.at(layout.lookupsIndexByName.at(child))->subtables.at(0));
    require(inlineValues->getValue(code, glyph).xAdvance == -2,
        "Inline function values must use the same SinglePos implementation");

    // A result that gains advance fields only on a deformed instance must
    // retain those fields in the serialized format and callback payload.
    Lookup lookup(&layout);

    // Standard alternate export retains supported combinations. Missing choices
    // fall back to the target base without shifting subsequent feature indexes.
    const auto base = layout.glyphCodePerName.at("behshape.medi");
    const auto* one = layout.getAlternate(base, {.lefttatweel = 1}, true, true);
    const auto* two = layout.getAlternate(base, {.lefttatweel = 2}, true, true);
    const auto* four = layout.getAlternate(base, {.lefttatweel = 4}, true, true);
    GlyphParameters missing{.lefttatweel = 20, .righttatweel = 20};
    require(!layout.getSubstEquivGlyphs(base).contains(missing),
        "Missing alternate fixture must not be materialized");
    AlternateSubtableWithParameters alternates(&lookup);
    alternates.alternates[base] = {{base, missing}, {base, 1, 0}, {base, -1, 0}};
    auto converted = alternates.getConvertedAlternates();
    require(converted.at(base).at(0) == base &&
                converted.at(base).at(1) == one->charcode,
        "Missing alternates must use the target base and keep selector indexes");
    require(converted.at(one->charcode) ==
                std::vector<std::uint16_t>{base, two->charcode, base},
        "Supported targets, including the base glyph, must remain available for stretched inputs");
    AlternateSubtableWithParameters declared(&lookup);
    declared.alternates[four->charcode] = {{base, 1, 0}, {base, missing}};
    const auto declaredAlternates = declared.getConvertedAlternates();
    require(declaredAlternates.at(four->charcode) ==
                std::vector<std::uint16_t>{one->charcode, base},
        "Declared alternate parameters are absolute; missing targets use the base glyph");

    const GlyphParameters combined{.lefttatweel = 4, .righttatweel = 2};
    require(!layout.getSubstEquivGlyphs(base).contains(combined),
        "Missing combined target fixture must not be materialized");
    AlternateSubtableWithParameters rightStretch(&lookup);
    rightStretch.alternates[base] = {{base, 0, 2}, {base, 1, 0}};
    const auto combinedAlternates = rightStretch.getConvertedAlternates();
    require(combinedAlternates.at(four->charcode).at(0) == base,
        "An unsupported combined stretch must fall back to the target base glyph");
    require(!layout.getSubstEquivGlyphs(base).contains(combined),
        "Alternate conversion must not materialize missing targets");

    SingleAdjustmentSubtable table(&lookup);
    table.functions[code] = [](GlyphVis* g) -> ValueRecord {
      return g->width > 500 ? ValueRecord{12, -23, -2, 7} : ValueRecord{11, -22, 0, 0};
    };
    table.singlePos[code] = table.getValue(code);
    table.parameters[code] = {1, 2, 3, 0};
    const auto encoded = table.getOpenTypeTables(true).at(0);
    require(u16(encoded, 4) == 15, "Function-valued SinglePos needs all four fields");
    require(u16(encoded, 8) == 12 && std::int16_t(u16(encoded, 10)) == -20 &&
            u16(encoded, 12) == 3 && u16(encoded, 14) == 0,
        "Export must evaluate functions and retain parameter adjustments");
    hb_glyph_info_t info{};
    info.codepoint = code;
    layout.setGlyphParameters(info, parameters);
    std::uint8_t payload[8]{};
    hb_cursive_anchor_context_t callback{};
    callback.glyph_id = code;
    callback.instance_id = info.instance_id;
    callback.data = payload;
    callback.value_format = 15;
    table.getSingleValue(&callback);
    require(payload[1] == 13 && payload[2] == 255 && payload[3] == 235 &&
            payload[5] == 1 && payload[7] == 7,
        "Runtime callback must return all four instance values plus adjustments");
    require(encoded == table.getOpenTypeTables(true).at(0),
        "Runtime evaluation must not mutate the serialized base table");

    // Functions must survive the Offset16 splitter, including late chunks.
    for (unsigned i = 1; i < 15000; ++i) table.singlePos.try_emplace(i, ValueRecord{});
    const auto parts = table.getOpenTypeTables(true);
    require(parts.size() > 1, "Large SinglePos coverage must split");
    bool found = false;
    for (const auto& part : parts) {
      const auto coverage = u16(part, 2), count = u16(part, 6), format = u16(part, 4);
      unsigned fields = 0;
      for (unsigned bits = format; bits; bits >>= 1) fields += bits & 1;
      for (unsigned i = 0; i < count; ++i)
        if (u16(part, coverage + 4 + 2 * i) == code) {
          found = true;
          require(u16(part, 8 + 2 * fields * i) == 12,
              "Split chunks must retain function evaluation and adjustments");
        }
    }
    require(found, "Function glyph lost during splitting");
    const auto bad = temporary / "unknown.fea";
    std::ofstream(bad) << source << "\nlookup test.bad { feature test; pos heh.init <function missing_singlepos_function>; } test.bad;\nfeature test { lookup test.bad; } test;\n";
    bool rejected = false;
    try { layout.loadLookupFile(bad.string()); }
    catch (const std::runtime_error& e) {
      rejected = std::string(e.what()).find("missing_singlepos_function") != std::string::npos;
    }
    require(rejected, "Unknown SinglePos functions must fail with their name");
    fs::remove_all(temporary);
    std::cout << "SinglePos functions: parsing, callbacks, export, splitting and errors passed\n";
  } catch (const std::exception& error) {
    fs::remove_all(temporary);
    std::cerr << error.what() << '\n';
    return 1;
  }
}
