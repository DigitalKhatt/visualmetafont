#include "glyphsourcedocument.hpp"
#include <stdexcept>

GlyphSourceDocument::GlyphSourceDocument(const digitalkhatt::project::GlyphSourceDocument& document) { assign(document); }
void GlyphSourceDocument::assign(const digitalkhatt::project::GlyphSourceDocument& document) {
  document_ = document;
  original = QByteArray::fromStdString(document.original);
  blocks.clear();
  const auto bom = document.original.starts_with("\xef\xbb\xbf") ? 3 : 0;
  std::size_t byteCursor = 0;
  int characterCursor = 0;
  for (const auto& block : document.blocks) {
    characterCursor += QString::fromUtf8(document.original.data() + bom + byteCursor, int(block.start - byteCursor)).size();
    const auto source = QString::fromStdString(block.source);
    blocks.append({characterCursor, source.size(), QString::fromStdString(block.name), source});
    characterCursor += source.size(); byteCursor = block.start + block.length;
  }
}
bool GlyphSourceDocument::read(const QString& path, QString* error) {
  try { assign(digitalkhatt::project::GlyphSourceDocument::read(std::filesystem::u8path(path.toStdString()))); return true; }
  catch (const std::exception& e) { if (error) *error = QString::fromUtf8(e.what()); return false; }
}
bool GlyphSourceDocument::parse(const QByteArray& bytes, QString* error) {
  try { assign(digitalkhatt::project::GlyphSourceDocument::parse(bytes.toStdString())); return true; }
  catch (const std::exception& e) { if (error) *error = QString::fromUtf8(e.what()); return false; }
}
QByteArray GlyphSourceDocument::render(const QMap<int, QString>& replacements, const QStringList& appended) const {
  std::map<std::size_t, std::string> changed;
  std::vector<std::string> added;
  for (auto i = replacements.begin(); i != replacements.end(); ++i) changed.emplace(i.key(), i.value().toStdString());
  for (const auto& source : appended) added.push_back(source.toStdString());
  return QByteArray::fromStdString(document_.render(changed, added));
}
