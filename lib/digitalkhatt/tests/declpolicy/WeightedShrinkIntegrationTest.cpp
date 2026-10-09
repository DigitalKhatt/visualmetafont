// Read-only integration test using the real OldMadina policy and shaper.
// Run: python3 visualmetafont/tests/glyph-editor/run-runtime-parameters.py
//   build/vscode-nmc oldmadinafont/oldmadina.mp this-file.cpp
// Build visualmetafont and oldmadina_font (Release) first.
#include "Layout/OtLayout.h"
#include "Layout/GlyphVis.h"
#include "metafont/font.hpp"
#include "../../src/justify/JustificationShaping.h"
#include "../../src/justify/declpolicy/DeclPolicyAdapter.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string_view>

using namespace digitalkhatt;
using namespace digitalkhatt::justify;
using namespace digitalkhatt::justify::runtime;
static void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }

class Provider : public FeatureJustificationLayout {
 public:
  OtLayout& layout;
  Provider(OtLayout& value) : layout(value) {}
  int scaleBy() const override { return 0; }
  int topSpace() const override { return 0; }
  int interLineSpacing() const override { return 0; }
  hb_font_t* createFont(double scale, bool fresh) override { return layout.createFont(scale, fresh); }
  std::string glyphName(hb_font_t*, hb_codepoint_t code) const override { return layout.glyphNamePerCode.at(code); }
  const CompiledJustificationCatalog* justificationCatalog() const override { return &*layout.compiledJustificationCatalog; }
  bool supportsGlyphParameters() const override { return true; }
  GlyphParameters glyphParameters(const hb_glyph_info_t& info, double, double) const override { return layout.glyphParameters(info); }
  bool setGlyphParameter(hb_glyph_info_t& info, GlyphAxisId axis, double value) const override {
    auto params = layout.glyphParameters(info);
    params.set(axis, value);
    layout.setGlyphParameters(info, params);
    return true;
  }
  std::optional<double> glyphParameterMaximum(hb_codepoint_t code, GlyphAxisId axis) const override {
    const auto glyph = layout.resolveGlyphInstance(code).sourceGlyph;
    const auto found = layout.expandableGlyphs.find(glyph->name);
    if (found == layout.expandableGlyphs.end()) return std::nullopt;
    return axis == LeftTatweelAxis ? found->second.maxLeft : found->second.maxRight;
  }
  std::optional<double> glyphAdvance(hb_font_t* font, hb_codepoint_t code, const GlyphParameters& params) const override { return layout.gethHorizontalAdvance(font, code, params, nullptr); }
  std::optional<hb_codepoint_t> resolveJustificationLookup(hb_codepoint_t code, std::string_view name) const override { return layout.resolveJustificationLookup(code, name); }
};

// Accepted native sites replay the corresponding parameter-only lookups,
// including ignored contexts, compact forms and later additive GSUB.
static void testFeatureDeltas(Provider& provider) {
  const auto& catalog = *provider.justificationCatalog();
  auto* font = provider.createFont(1, false);
  for (const auto value : {u"بب بب", u"عص فص حس حع عح حح مف لك كف لن قف", u"بسم الله الرحمن الرحيم"}) {
    auto text = analyzeLineForJust(value);
    for (const bool compact : {false, true}) {
      std::vector<TextFontFeatures> globals;
      if (compact) for (const auto* tag : {"sk01", "sk02", "sk03"}) globals.push_back({tag, 1});
      std::vector<hb_feature_t> features;
      for (const auto& feature : globals) features.push_back({hb_tag_from_string(feature.name.c_str(), 4), 1, 0, static_cast<unsigned>(-1)});
      ShapingBuffer recognition(shapeForMeasurement(text.lineText, font, features));
      const double natural = getBufferWidth(recognition.get());
      JustInfo info{.globalFeatures = globals, .desiredWidth = natural - 10000, .textLineWidth = natural, .initialLineWidth = natural, .font = font, .layout = &provider, .catalog = &catalog, .recognitionBuffer = recognition.get()};
      for (const auto& word : text.wordInfos) info.layoutResult.push_back({getWordWidth(word, {}, font, nullptr, &provider, globals), {}});
      for (const auto* name : {"NativeShrinkOnce", "NativeShrinkTwice"}) {
        const auto selection = std::find_if(catalog.selections.begin(), catalog.selections.end(), [&](const auto& selected) { return selected.name == name; }) - catalog.selections.begin();
        check(selection < catalog.selections.size(), "Missing native selection");
        PolicyPhase phase{.selection = static_cast<JustificationSelectionId>(selection), .levels = 1, .allocator = PolicyPhase::Allocator::CandidatePool};
        decl::applyDeclPolicyStage(text, info, std::span(&phase, 1), 16, CandidateWidthMode::Advance);

        // Repeating the stage must not apply the same level a second time.
        const auto accepted = info.fontFeatures;
        decl::applyDeclPolicyStage(text, info, std::span(&phase, 1), 16, CandidateWidthMode::Advance);
        check(info.fontFeatures == accepted, "Native shrink level applied more than once");
      }
      std::map<int, std::vector<TextFontFeatures>> featureValues;
      for (const auto& [site, values] : info.fontFeatures) {
        const auto first = info.featureParameterDeltas.at("sk04").at(site);
        bool twice = false;
        for (const auto& value : values) {
          check(value.additive && value.axis != NoGlyphAxis, "Native policy wrote an OpenType feature");
          twice = twice || std::abs(value.value - first.value(value.axis)) > 1e-12;
        }
        featureValues[site].push_back({"sk04", 1});
        if (twice) featureValues[site].push_back({"sk05", 1});
      }
      // Include a later parameter fallback: deltas must not overwrite it.
      for (const bool fallback : {false, true}) {
        auto replayGlobals = globals;
        if (fallback) replayGlobals.push_back({"sk06", 1});
        WordInfo whole{.text = text.lineText, .startIndex = 0, .endIndex = static_cast<int>(text.lineText.size()) - 1};
        ShapingBuffer replay, reference;
        const auto referenceWidth = getWordWidth(whole, featureValues, font, &reference, &provider, replayGlobals);
        const auto width = getWordWidth(whole, info.fontFeatures, font, &replay, &provider, replayGlobals);
        check(std::abs(width - referenceWidth) < 0.01, "Native shrinking changed contextual advances");
        unsigned count = 0, referenceCount = 0;
        const auto* glyphs = hb_buffer_get_glyph_infos(replay.get(), &count);
        const auto* expected = hb_buffer_get_glyph_infos(reference.get(), &referenceCount);
        check(count == referenceCount, "Native shrinking changed glyph count");
        for (unsigned index = 0; index < count; ++index) {
          check(glyphs[index].codepoint == expected[index].codepoint, "Native shrinking changed a glyph substitution");
          const auto actualParams = provider.glyphParameters(glyphs[index], 0, 0);
          const auto expectedParams = provider.glyphParameters(expected[index], 0, 0);
          for (GlyphAxisId axis = 0; axis < std::max(actualParams.size(), expectedParams.size()); ++axis)
            check(std::abs(actualParams.value(axis) - expectedParams.value(axis)) <= 2.0 / 65536,
                  "Native shrinking lost a contextual parameter delta");
        }
      }
    }
  }
  hb_font_destroy(font);
  std::cout << "PASS native feature deltas, contextual exclusions, compact forms and additive fallback\n";
}

static void test(Provider& provider) {
  auto catalog = *provider.justificationCatalog();
  const auto selection = std::find_if(catalog.selections.begin(), catalog.selections.end(), [](const auto& value) { return value.name == "ShrinkOnce"; }) - catalog.selections.begin();
  check(selection < catalog.selections.size(), "Missing weighted shrink selection");
  catalog.selections[selection].rules.resize(1);
  const auto rule = catalog.selections[selection].rules.front().rule;
  const auto binding = std::find_if(catalog.actions.begin(), catalog.actions.end(), [&](const auto& action) { return action.rule == rule; });
  check(binding != catalog.actions.end(), "Missing shrink binding");
  const auto nativeAttribute = std::find(catalog.attributes.begin(), catalog.attributes.end(), "lefttatweel") - catalog.attributes.begin();
  const auto rightAttribute = std::find(catalog.attributes.begin(), catalog.attributes.end(), "righttatweel") - catalog.attributes.begin();
  check(nativeAttribute < catalog.attributes.size() && rightAttribute < catalog.attributes.size(), "Missing native axes");
  catalog.actionDefinitions[binding->definition].effects = {
      {.kind = JustEffectKind::Vary, .attribute = static_cast<JustAttributeId>(nativeAttribute), .value = {{.literal = -0.2}}},
      {.kind = JustEffectKind::Vary, .attribute = static_cast<JustAttributeId>(rightAttribute), .value = {{.literal = -0.2}}}};
  catalog.selections[selection].rules.front().decay = 0.5;
  PolicyPhase phase{.selection = static_cast<JustificationSelectionId>(selection), .levels = 1, .allocator = PolicyPhase::Allocator::CandidatePool};
  auto text = analyzeLineForJust(u"بب بب");
  auto* font = provider.createFont(1, false);
  ShapingBuffer recognition(shapeForMeasurement(text.lineText, font, {}));
  const double natural = getBufferWidth(recognition.get());
  const auto fresh = [&](double gap) {
    JustInfo info{.desiredWidth = natural - gap, .textLineWidth = natural, .initialLineWidth = natural, .font = font, .layout = &provider, .catalog = &catalog, .recognitionBuffer = recognition.get()};
    for (const auto& word : text.wordInfos) info.layoutResult.push_back({getWidth(word.text, font, {}), {}});
    return info;
  };
  std::vector<JustificationDecisionTrace> traces;
  provider.setJustificationTraceCallback([&](const auto& trace) { traces.push_back(trace); });
  for (const auto gap : {0.001, 30.0, 10000.0}) {
    for (const auto subdivisions : {0u, 100u}) {
      auto info = fresh(gap);
      traces.clear();
      decl::applyDeclPolicyStage(text, info, std::span(&phase, 1), subdivisions, CandidateWidthMode::FullShape);
      check(info.textLineWidth <= natural && info.textLineWidth >= info.desiredWidth, "Shrinking crossed target or grew the line");
      double replay = getWidth(u" ", font, {});
      for (const auto& word : text.wordInfos) replay += getWordWidth(word, info.fontFeatures, font, nullptr, &provider);
      check(std::abs(replay - info.textLineWidth) < 0.01, "Accepted shrink state does not replay at measured width");
      if (gap < 1) check(info.fontFeatures.empty(), "A tiny remaining budget mutated glyphs");
      else check(info.textLineWidth < natural, "Native shrinking pool did not reduce width");
      std::vector<double> ratios;
      for (const auto& trace : traces) if (trace.decision == "applied") {
        check(trace.maximumWidthDelta && *trace.maximumWidthDelta < 0, "Accepted shrinking candidate had a positive delta");
        ratios.push_back(trace.appliedRatio);
      }
      if (gap == 30 && subdivisions == 0) {
        check(ratios.size() >= 2 && ratios[0] > ratios[1], "Occurrence weights did not distribute decreasing native ranges");
      }
      std::cout << "PASS weighted native shrink gap=" << gap << " quantization=" << subdivisions << '\n';
    }
  }
  // Discrete lookup opportunities also respect both word and subword limits.
  catalog = *provider.justificationCatalog();
  const auto& recipe = catalog.shrinkPolicies[catalog.shrinkPolicyIndex("weighted")];
  const auto& spread = *std::find_if(recipe.steps.begin(), recipe.steps.end(), [](const auto& step) { return step.stage == "ShrinkSpread"; });
  auto info = fresh(10000);
  decl::applyDeclPolicyStage(text, info, spread.phases, 16, CandidateWidthMode::FullShape);
  check(info.textLineWidth < natural, "Discrete shrink rules did not apply");
  std::map<int, int> words;
  for (const auto& [site, values] : info.fontFeatures) {
    for (const auto& word : text.wordInfos) if (site >= word.startIndex && site <= word.endIndex) ++words[word.startIndex];
  }
  for (const auto& [word, count] : words) check(count <= 1, "First pass exceeded one opportunity per subword");
  hb_font_destroy(font);
  std::cout << "PASS discrete shrink limits\n";
}

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  try {
    Font editor;
    check(argc == 2 && editor.loadFile(QString::fromLocal8Bit(argv[1])), "Font load failed");
    const auto dir = QFileInfo(QString::fromLocal8Bit(argv[1])).dir();
    QFile source(dir.filePath("glyphs.mp"));
    check(source.open(QFile::ReadOnly), "Cannot read glyphs");
    editor.executeMetaPost("params0:=0;params1:=0;params2:=0;params3:=0;params4:=0;" + source.readAll().toStdString());
    OtLayout layout(&editor.mpFont(), true, true);
    layout.useNormAxisValues = false;
    layout.loadLookupFile(dir.filePath("features.fea").toStdString());
    Provider provider(layout);
    testFeatureDeltas(provider);
    test(provider);
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
