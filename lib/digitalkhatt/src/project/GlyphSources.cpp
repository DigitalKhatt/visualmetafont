#include "digitalkhatt/project/GlyphSources.h"
#include <glaze/glaze.hpp>
#include "MPFont.h"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <set>
#include <stdexcept>

namespace digitalkhatt::project {
namespace fs = std::filesystem;
namespace {
std::string readFile(const fs::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("Cannot read " + path.string());
  return {std::istreambuf_iterator<char>{stream}, {}};
}
bool validUtf8(std::string_view text) {
  for (std::size_t i = 0; i < text.size();) {
    const auto c = static_cast<unsigned char>(text[i++]);
    if (c < 0x80) continue;
    const int extra = c >= 0xc2 && c <= 0xdf ? 1 : c >= 0xe0 && c <= 0xef ? 2 : c >= 0xf0 && c <= 0xf4 ? 3 : -1;
    if (extra < 0 || i + extra > text.size()) return false;
    unsigned value = c & (0x7f >> (extra + 1));
    for (int n = 0; n < extra; ++n) {
      const auto next = static_cast<unsigned char>(text[i++]);
      if ((next & 0xc0) != 0x80) return false;
      value = (value << 6) | (next & 0x3f);
    }
    if (value < (extra == 1 ? 0x80u : extra == 2 ? 0x800u : 0x10000u) ||
        value > 0x10ffff || (value >= 0xd800 && value <= 0xdfff)) return false;
  }
  return true;
}
bool letter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool digit(char c) { return c >= '0' && c <= '9'; }
bool space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v'; }
std::size_t skipTrivia(std::string_view text, std::size_t i) {
  while (i < text.size()) {
    if (space(text[i])) { ++i; continue; }
    if (text[i] == '%') { while (i < text.size() && text[i] != '\n') ++i; continue; }
    break;
  }
  return i;
}
void parseHeader(GlyphSourceBlock& block, std::string_view macro) {
  const auto text = std::string_view(block.source);
  auto i = skipTrivia(text, macro.size());
  if (i >= text.size() || text[i++] != '(') throw std::runtime_error("Invalid glyph header");
  i = skipTrivia(text, i);
  const auto nameStart = i;
  if (i >= text.size() || !letter(text[i])) throw std::runtime_error("Invalid glyph name");
  while (i < text.size() && (letter(text[i]) || digit(text[i]) || text[i] == '.')) ++i;
  block.name = text.substr(nameStart, i - nameStart);
  i = skipTrivia(text, i);
  if (i >= text.size() || text[i++] != ',') throw std::runtime_error("Invalid glyph header after " + block.name);
  i = skipTrivia(text, i);
  const auto numberStart = i;
  if (i < text.size() && (text[i] == '-' || text[i] == '+')) ++i;
  const auto digitStart = i;
  while (i < text.size() && digit(text[i])) ++i;
  if (i == digitStart) throw std::runtime_error("Invalid glyph Unicode value: " + block.name);
  const auto number = text.substr(numberStart, i - numberStart);
  const auto begin = number.data() + (number.front() == '+' ? 1 : 0);
  const auto parsed = std::from_chars(begin, number.data() + number.size(), block.unicode);
  if (parsed.ec != std::errc{} || parsed.ptr != number.data() + number.size()) throw std::runtime_error("Invalid glyph Unicode value: " + block.name);
  i = skipTrivia(text, i);
  if (i >= text.size() || text[i] != ',') throw std::runtime_error("Invalid glyph header after Unicode: " + block.name);
  block.beginMacro = macro;
}
} // namespace

GlyphSourceDocument GlyphSourceDocument::read(const fs::path& path) {
  try { return parse(readFile(path)); }
  catch (const std::exception& error) { throw std::runtime_error(path.string() + ": " + error.what()); }
}
GlyphSourceDocument GlyphSourceDocument::parse(std::string bytes) {
  GlyphSourceDocument result;
  result.bom_ = bytes.starts_with("\xef\xbb\xbf");
  result.text_ = result.bom_ ? bytes.substr(3) : bytes;
  if (!validUtf8(result.text_)) throw std::runtime_error("Glyph sources must be valid UTF-8");
  const auto& text = result.text_;
  std::optional<std::size_t> start;
  std::string ending, beginMacro;
  for (std::size_t i = 0; i < text.size();) {
    if (text[i] == '%') { while (i < text.size() && text[i] != '\n') ++i; continue; }
    if (text[i] == '"') {
      ++i; while (i < text.size() && text[i] != '"') ++i;
      if (i == text.size()) throw std::runtime_error("Unterminated MetaPost string");
      ++i; continue;
    }
    if (!letter(text[i])) { ++i; continue; }
    const auto tokenStart = i++;
    while (i < text.size() && (letter(text[i]) || digit(text[i]) || text[i] == '.')) ++i;
    const auto token = std::string_view(text).substr(tokenStart, i - tokenStart);
    const auto next = skipTrivia(text, i);
    if ((token == "beginchar" || token == "defchar") && next < text.size() && text[next] == '(') {
      if (start) throw std::runtime_error("Nested glyph definition before the previous glyph ended");
      start = tokenStart; beginMacro = token; ending = token == "beginchar" ? "endchar" : "enddefchar";
    } else if (start && (token == "endchar" || token == "enddefchar")) {
      if (token != ending || next >= text.size() || text[next] != ';') throw std::runtime_error("Mismatched glyph terminator");
      GlyphSourceBlock block;
      block.start = *start; block.length = next + 1 - *start; block.source = text.substr(block.start, block.length);
      parseHeader(block, beginMacro);
      result.blocks.push_back(std::move(block)); start.reset(); i = next + 1;
    }
  }
  if (start) throw std::runtime_error("Unterminated glyph definition");
  result.crlf_ = text.find("\r\n") != std::string::npos;
  result.original = std::move(bytes);
  return result;
}
std::string GlyphSourceDocument::render(const std::map<std::size_t, std::string>& replacements,
                                      const std::vector<std::string>& appended) const {
  if (replacements.empty() && appended.empty()) return original;
  for (const auto& [index, value] : replacements)
    if (index >= blocks.size()) throw std::out_of_range("Unknown glyph source block");
  auto newline = [this](std::string_view source) {
    std::string output;
    for (std::size_t i = 0; i < source.size(); ++i) {
      if (source[i] == '\r' && i + 1 < source.size() && source[i + 1] == '\n') continue;
      if (source[i] == '\n' && crlf_) output += '\r';
      output += source[i];
    }
    return output;
  };
  std::string output;
  std::size_t cursor = 0;
  for (std::size_t i = 0; i < blocks.size(); ++i) {
    const auto& block = blocks[i];
    output += text_.substr(cursor, block.start - cursor);
    if (const auto replacement = replacements.find(i); replacement != replacements.end()) output += newline(replacement->second);
    else output += block.source;
    cursor = block.start + block.length;
  }
  output += text_.substr(cursor);
  for (const auto& source : appended) {
    if (!output.empty() && output.back() != '\n') output += crlf_ ? "\r\n" : "\n";
    output += newline(source);
    if (!output.ends_with('\n')) output += crlf_ ? "\r\n" : "\n";
  }
  return (bom_ ? std::string("\xef\xbb\xbf") : std::string{}) + output;
}
std::vector<std::string> normalizeGlyphFiles(const std::vector<std::string>& files) {
  if (files.empty()) throw std::runtime_error("glyphFiles must contain at least one file");
  std::vector<std::string> result;
  std::set<std::string> seen;
  for (const auto& file : files) {
    const auto path = fs::u8path(file).lexically_normal();
    const auto normalized = path.generic_string();
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return c >= 'A' && c <= 'Z' ? c + ('a'-'A') : c; });
    if (file.empty() || file.find('\0') != std::string::npos || path.is_absolute() || file.find('\\') != std::string::npos || file.find(':') != std::string::npos ||
        normalized == ".." || normalized.starts_with("../") || extension != ".mp" || !seen.insert(normalized).second)
      throw std::runtime_error("Invalid or duplicate glyph source path: " + file);
    result.push_back(normalized);
  }
  return result;
}
std::vector<std::string> glyphFilesFromMetadata(std::string_view json) {
  if (json.starts_with("\xef\xbb\xbf")) json.remove_prefix(3);
  glz::generic_i64 document;
  if (const auto error = glz::read_json(document, json)) throw std::runtime_error("Invalid project metadata: " + glz::format_error(error, json));
  if (!document.is_object()) throw std::runtime_error("Project metadata must be an object");
  const auto& fields = document.get_object();
  const auto version = fields.find("schemaVersion");
  if (version == fields.end() || (!version->second.is_double() && !version->second.is_int64()) || (version->second.as_number() != 1 && version->second.as_number() != 2))
    throw std::runtime_error("Unsupported project metadata schemaVersion");
  const auto found = fields.find("glyphFiles");
  if (found == fields.end()) return {"glyphs.mp"};
  if (!found->second.is_array()) throw std::runtime_error("glyphFiles must be an ordered array");
  std::vector<std::string> paths;
  for (const auto& value : found->second.get_array()) {
    if (!value.is_string()) throw std::runtime_error("glyphFiles entries must be paths");
    paths.push_back(value.get_string());
  }
  return normalizeGlyphFiles(paths);
}
ProjectGlyphSources ProjectGlyphSources::load(const fs::path& project, const std::optional<fs::path>& overrideFile) {
  ProjectGlyphSources result;
  const auto root = fs::absolute(project).parent_path();
  std::vector<std::pair<std::string, fs::path>> inputs;
  if (overrideFile) inputs.emplace_back(overrideFile->generic_string(), fs::absolute(*overrideFile));
  else {
    const auto metadata = root / "font-project.json";
    const auto files = fs::exists(metadata) ? glyphFilesFromMetadata(readFile(metadata)) : std::vector<std::string>{"glyphs.mp"};
    if (fs::exists(metadata)) result.metadataPath = metadata;
    for (const auto& file : files) inputs.emplace_back(file, root / fs::u8path(file));
  }
  std::set<fs::path> physicalPaths;
  std::map<std::string, std::string> names;
  for (const auto& [relative, path] : inputs) {
    std::error_code error;
    const auto canonical = fs::weakly_canonical(path, error);
    if (!error && !physicalPaths.insert(canonical).second) throw std::runtime_error("Glyph source listed more than once: " + relative);
    auto document = GlyphSourceDocument::read(path);
    for (const auto& block : document.blocks) {
      const auto [found, inserted] = names.emplace(block.name, relative);
      if (!inserted) throw std::runtime_error("Duplicate glyph " + block.name + " in " + found->second + " and " + relative);
    }
    result.files.push_back({relative, path, std::move(document)});
  }
  return result;
}
std::size_t ProjectGlyphSources::glyphCount() const {
  std::size_t count = 0;
  for (const auto& file : files) count += file.document.blocks.size();
  return count;
}
std::size_t ProjectGlyphSources::registerWith(MPFont& font, bool materialize) const {
  if (!glyphCount()) throw std::runtime_error("No glyph definitions found in project sources");
  for (const auto& file : files) for (const auto& block : file.document.blocks)
    font.registerGlyphSource(block.name, block.source, block.beginMacro, block.unicode);
  if (materialize) for (const auto& file : files) for (const auto& block : file.document.blocks)
    font.execute("params[0]:=0;params[1]:=0;params[2]:=0;params[3]:=0;params[4]:=0;" + block.source);
  return glyphCount();
}
} // namespace digitalkhatt::project
