#include "digitalkhatt/project/GlyphSources.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace digitalkhatt::project;
namespace fs = std::filesystem;
static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
template<class F> static void reject(F f, const char* message) { bool rejected = false; try { f(); } catch (const std::exception&) { rejected = true; } require(rejected, message); }
static void write(const fs::path& path, std::string_view text) { std::ofstream file(path, std::ios::binary); file << text; require(bool(file), "write fixture"); }
int main() {
  const auto root = fs::temp_directory_path() / ("dk-project-sources-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
  fs::create_directories(root / "glyphs");
  try {
    const std::string first = "defchar(first, -1,500,700,0);\r\nmessage \"enddefchar;\";\r\nenddefchar;";
    const std::string second = "beginchar(second, 1662,500,700,0);\nendchar;";
    const auto raw = std::string("\xef\xbb\xbf") + "% العربية: defchar(commented,-1,0,0,0);enddefchar;\r\n" + first + "\r\n% Footer\r\n";
    const auto doc = GlyphSourceDocument::parse(raw);
    require(doc.blocks.size() == 1 && doc.blocks[0].name == "first" && doc.blocks[0].beginMacro == "defchar" && doc.blocks[0].unicode == -1, "one parser handles comments, strings and headers");
    require(doc.render({}) == raw, "lossless source round-trip");
    require(doc.render({}, {second}).find("beginchar(second, 1662,500,700,0);\r\nendchar;") != std::string::npos, "newlines retained on append");
    reject([] { GlyphSourceDocument::parse("beginchar(bad,-1,0,0,0);enddefchar;"); }, "reject wrong glyph terminator");
    reject([] { GlyphSourceDocument::parse("defchar(bad,garbage,0,0,0);enddefchar;"); }, "reject invalid Unicode header");
    reject([] { GlyphSourceDocument::parse(std::string("\xc0\x80")); }, "reject invalid UTF-8");
    require(normalizeGlyphFiles({"glyphs/./first.mp"})[0] == "glyphs/first.mp", "normalize paths");
    reject([] { normalizeGlyphFiles({"a.mp", "./a.mp"}); }, "reject aliases in manifest");
    reject([] { normalizeGlyphFiles({"../outside.mp"}); }, "reject escaping paths");
    reject([] { normalizeGlyphFiles({std::string("glyphs.mp\0evil.mp", 17)}); }, "reject NUL paths");
    require(glyphFilesFromMetadata(R"({"schemaVersion":2,"groups":[],"glyphFiles":["glyphs/first.mp","glyphs/second.mp"]})").size() == 2, "read ordered manifest");
    require(glyphFilesFromMetadata(std::string("\xef\xbb\xbf") + R"({"schemaVersion":2})")[0] == "glyphs.mp", "UTF-8 BOM in manifest");
    reject([] { glyphFilesFromMetadata(R"({"schemaVersion":2,"glyphFiles":null})"); }, "reject null manifest");
    reject([] { glyphFilesFromMetadata(R"({"schemaVersion":2,"glyphFiles":[3]})"); }, "reject non-string path");
    reject([] { glyphFilesFromMetadata(R"({"schemaVersion":99})"); }, "reject unsupported schema");
    write(root / "font.mp", ""); write(root / "glyphs.mp", first);
    auto sources = ProjectGlyphSources::load(root / "font.mp");
    require(sources.glyphCount() == 1 && !sources.metadataPath, "legacy fallback without metadata");
    write(root / "glyphs/first.mp", raw); write(root / "glyphs/second.mp", second);
    write(root / "font-project.json", R"({"schemaVersion":2,"glyphFiles":["glyphs/first.mp","glyphs/second.mp"]})");
    sources = ProjectGlyphSources::load(root / "font.mp");
    require(sources.files.size() == 2 && sources.glyphCount() == 2 && sources.metadataPath && sources.files[1].document.blocks[0].unicode == 1662, "all sources read in manifest order");
    require(ProjectGlyphSources::load(root / "font.mp", root / "glyphs.mp").glyphCount() == 1, "explicit override replaces manifest");
    write(root / "glyphs/second.mp", first);
    reject([&] { ProjectGlyphSources::load(root / "font.mp"); }, "reject duplicate definitions across files");
    fs::remove_all(root);
    std::cout << "PASS shared Qt-free source loader: parser, manifests, ordering, fallback, overrides and diagnostics\n";
    return 0;
  } catch (const std::exception& e) { fs::remove_all(root); std::cerr << e.what() << '\n'; return 1; }
}
