#pragma once
#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class MPFont;

namespace digitalkhatt::project {
struct GlyphSourceBlock {
  std::size_t start = 0, length = 0; // UTF-8 byte offsets, excluding any BOM
  std::string name, source, beginMacro;
  int unicode = -1;
};

// Common, Qt-free parser and lossless source container for editor and CLI.
class GlyphSourceDocument {
 public:
  static GlyphSourceDocument read(const std::filesystem::path& path);
  static GlyphSourceDocument parse(std::string bytes);
  std::string render(const std::map<std::size_t, std::string>& replacements,
                     const std::vector<std::string>& appended = {}) const;
  std::string original;
  std::vector<GlyphSourceBlock> blocks;
 private:
  std::string text_;
  bool bom_ = false, crlf_ = false;
};

// Relative manifest paths are normalized and checked in one place. Explicit CLI
// overrides are resolved by the caller and intentionally replace the manifest.
std::vector<std::string> normalizeGlyphFiles(const std::vector<std::string>& files);
std::vector<std::string> glyphFilesFromMetadata(std::string_view json);

struct GlyphSourceFile {
  std::string relativePath;
  std::filesystem::path path;
  GlyphSourceDocument document;
};
class ProjectGlyphSources {
 public:
  static ProjectGlyphSources load(const std::filesystem::path& project,
      const std::optional<std::filesystem::path>& overrideFile = std::nullopt);
  std::size_t glyphCount() const;
  // All definitions are registered before optional sequential materialization.
  std::size_t registerWith(MPFont& font, bool materialize = true) const;
  std::vector<GlyphSourceFile> files;
  std::optional<std::filesystem::path> metadataPath;
};
} // namespace digitalkhatt::project
