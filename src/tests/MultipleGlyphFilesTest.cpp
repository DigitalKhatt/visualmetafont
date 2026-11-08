#include "font.hpp"
#include "glyph.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <QPointer>
#include <iostream>
#include <stdexcept>

static void require(bool ok, const QString& message) { if (!ok) throw std::runtime_error(message.toStdString()); }
static QByteArray read(const QString& path) { QFile f(path); require(f.open(QIODevice::ReadOnly), path); return f.readAll(); }
static void write(const QString& path, const QByteArray& bytes) { QFile f(path); require(f.open(QIODevice::WriteOnly), path); require(f.write(bytes) == bytes.size(), "write fixture"); }
static void load(Font& font, const QString& path) { require(font.loadFile(path), font.projectError); QApplication::restoreOverrideCursor(); }
int main(int argc, char** argv) {
  QApplication app(argc, argv);
  try {
    require(argc == 2, "Expected a Madina source directory");
    const QString source = argv[1];
    const auto original = read(source + "/glyphs.mp");
    ProjectMetadata sourceMetadata;
    require(sourceMetadata.load(source + "/font-project.json"), "source project metadata");
    QByteArray combined;
    for (const auto& path : sourceMetadata.glyphFiles()) combined += read(source + "/" + path) + "\n";
    QTemporaryDir temporary;
    QDir root(temporary.path()); root.mkpath("glyphs");
    // All mutable fixture files are copies, never links to approved source.
    for (const auto& file : QDir(source).entryInfoList(QDir::Files))
      if (file.fileName() != "glyphs.mp") require(QFile::copy(file.absoluteFilePath(), root.filePath(file.fileName())), "copy fixture dependencies");
    GlyphSourceDocument whole;
    require(whole.parse(combined), "parse actual font sources");
    require(whole.blocks.size() > 100, "real-font coverage fixture");
    const auto text = QString::fromUtf8(combined);
    const int cut = whole.blocks[100].start;
    const auto first = QByteArray("% First source file; defchar(commented,1,0,0,0);\n") + text.left(cut).toUtf8();
    const auto second = QByteArray("% Second source file\n") + text.mid(cut).toUtf8();
    write(root.filePath("glyphs/first.mp"), first); write(root.filePath("glyphs/second.mp"), second);
    ProjectMetadata metadata;
    require(metadata.load(root.filePath("font-project.json")), "fixture metadata");
    require(metadata.setGlyphFiles({"glyphs/first.mp", "glyphs/second.mp"}) && metadata.save(root.filePath("font-project.json")), "two-file manifest");
    Font reference, split;
    load(reference, source + "/madina.mp"); load(split, root.filePath("madina.mp"));
    require(split.glyphs.size() == reference.glyphs.size(), "same glyph count");
    int compared = 0;
    for (int i = 0; i < reference.glyphs.size(); ++i) {
      require(reference.glyphs[i]->name() == split.glyphs[i]->name(), "evaluation order preserved");
      auto* a = reference.glyphs[i]->getEdge(); auto* b = split.glyphs[i]->getEdge();
      require(bool(a) == bool(b), "render availability");
      if (a) { require(Glyph::getPath(a) == Glyph::getPath(b) && a->width == b->width && a->charcode == b->charcode, "outlines, advances and glyph IDs unchanged"); ++compared; }
    }
    require(!split.hasSourceChanges(), "rendering does not change source");
    require(split.saveFile(), split.projectError);
    require(read(root.filePath("glyphs/first.mp")) == first && read(root.filePath("glyphs/second.mp")) == second, "no-op Save is byte-identical");
    require(!QFile::exists(root.filePath("glyphs.mp")), "saving never flattens into glyphs.mp");
    auto* glyph = split.glyphs[0];
    const auto glyphName = glyph->name();
    const auto groups = split.project.glyphGroups(glyphName);
    glyph->setVerbatim(glyph->verbatim() + "\n% A targeted edit\n");
    require(split.hasSourceChanges() && split.saveFile(), split.projectError);
    const auto editedFirst = read(root.filePath("glyphs/first.mp"));
    require(editedFirst.contains("% A targeted edit") && editedFirst.startsWith("% First source file") &&
        read(root.filePath("glyphs/second.mp")) == second, "only the edited glyph's owning file changes");
    GlyphSourceDocument edited;
    require(edited.parse(editedFirst), "parse edited document");
    for (int i = 1; i < 100; ++i) require(edited.blocks[i].source == whole.blocks[i].source, "untouched definitions retain formatting");
    require(split.moveGlyphs({glyph}, "glyphs/second.mp") && glyph->sourceFile() == "glyphs/second.mp", "move ownership");
    require(split.project.glyphGroups(glyphName) == groups, "groups independent of source files");
    require(split.saveFile(), split.projectError);
    Font moved;
    load(moved, root.filePath("madina.mp"));
    require(moved.glyphperName.value(glyphName)->sourceFile() == "glyphs/second.mp", "move survives reload");
    for (int i = 0; i < split.glyphs.size(); ++i) require(split.glyphs[i]->name() == moved.glyphs[i]->name(), "move/save/reload order agrees");
    require(split.createGlyphFile("glyphs/new.mp"), split.projectError);
    require(!split.createGlyphFile("glyphs/new.mp"), "do not overwrite an existing source file");
    auto* fresh = split.createGlyph("project.newglyph", 0x067e, "glyphs/new.mp");
    require(fresh && fresh->sourceFile() == "glyphs/new.mp", "new glyph has chosen source ownership");
    fresh->setBody("fill (0,0)--(100,0)--(100,100)--cycle;\n");
    require(fresh->getEdge() && split.hasGlyph("project.newglyph"), "new glyph registered with the editor renderer");
    const auto beforeNewFirst = read(root.filePath("glyphs/first.mp")), beforeNewSecond = read(root.filePath("glyphs/second.mp"));
    require(split.saveFile(), split.projectError);
    require(read(root.filePath("glyphs/new.mp")).contains("project.newglyph") &&
        read(root.filePath("glyphs/first.mp")) == beforeNewFirst && read(root.filePath("glyphs/second.mp")) == beforeNewSecond, "new glyph saved only to selected file");
    require(!split.createGlyph("project.newglyph", -1, "glyphs/new.mp") && !split.createGlyph("different", 0x067e, "glyphs/new.mp"), "reject duplicate new names and Unicode");
    Font reloaded; load(reloaded, root.filePath("madina.mp"));
    require(reloaded.glyphSourceFiles().size() == 3 && reloaded.glyphperName.value("project.newglyph"), "new file and glyph survive reload");
    const auto outsideEdit = beforeNewFirst + "% External editor's change\n";
    write(root.filePath("glyphs/first.mp"), outsideEdit);
    fresh->setDepth(10);
    const auto beforeFailed = read(root.filePath("glyphs/new.mp"));
    require(!split.saveFile() && split.projectError.contains("changed on disk"), "detect external edits");
    require(read(root.filePath("glyphs/first.mp")) == outsideEdit && read(root.filePath("glyphs/new.mp")) == beforeFailed, "failed preflight overwrites nothing");
    // Duplicate definitions and missing files fail explicitly on load.
    write(root.filePath("glyphs/duplicate.mp"), read(root.filePath("glyphs/second.mp")));
    require(metadata.setGlyphFiles({"glyphs/second.mp", "glyphs/duplicate.mp"}) && metadata.save(root.filePath("font-project.json")), "duplicate fixture manifest");
    Font duplicate;
    require(!duplicate.loadFile(root.filePath("madina.mp")) && duplicate.projectError.contains("Duplicate glyph"), "reject duplicate definitions across files");
    require(metadata.setGlyphFiles({"glyphs/missing.mp"}) && metadata.save(root.filePath("font-project.json")), "missing fixture manifest");
    Font missing;
    require(!missing.loadFile(root.filePath("madina.mp")) && missing.projectError.contains("missing.mp"), "report missing source file");
    require(read(source + "/glyphs.mp") == original, "approved source unchanged");

    // Exercise the deletion operation used by the context-menu action on a disposable project.
    write(root.filePath("glyphs/delete-first.mp"), combined);
    const QByteArray onlyHeader = "% Keep this comment when the last glyph is deleted.\n";
    write(root.filePath("glyphs/delete-only.mp"), onlyHeader +
        "defchar(project.deleteFromMenu,-1,500,6,0);\n%%beginpaths\nfill (0,0)--(100,0)--(100,100)--cycle;\n%%endpaths\nenddefchar;\n");
    require(metadata.setGlyphFiles({"glyphs/delete-first.mp", "glyphs/delete-only.mp"}) &&
        metadata.save(root.filePath("font-project.json")), "deletion fixture manifest");
    Font deleting; load(deleting, root.filePath("madina.mp"));
    auto* deletingGlyph = deleting.glyphperName.value("project.deleteFromMenu");
    require(deletingGlyph, "load glyph to delete");
    QPointer<Glyph> deleted = deletingGlyph;
    require(deleting.removeGlyph(deletingGlyph) && deleted.isNull() &&
        !deleting.glyphperName.contains("project.deleteFromMenu"), "Delete removes glyph membership and object");
    require(deleting.hasSourceChanges(), "deletion alone marks project changed");
    require(deleting.saveFile() && !deleting.hasSourceChanges(), deleting.projectError);
    require(read(root.filePath("glyphs/delete-first.mp")) == combined &&
        read(root.filePath("glyphs/delete-only.mp")).trimmed() == onlyHeader.trimmed(), "delete only its block and retain comments/other files");
    const auto savedDeletion = read(root.filePath("glyphs/delete-only.mp"));
    require(deleting.saveFile() && read(root.filePath("glyphs/delete-only.mp")) == savedDeletion, "repeat deletion Save is a no-op");
    Font reopened; load(reopened, root.filePath("madina.mp"));
    require(!reopened.glyphperName.contains("project.deleteFromMenu"), "GUI Delete / Save / Reopen does not resurrect glyph");
    auto* transient = reopened.createGlyph("project.unsavedDeletion", -1, "glyphs/delete-only.mp");
    require(transient && reopened.hasSourceChanges() && reopened.removeGlyph(transient) &&
        !reopened.hasSourceChanges(), "create then delete an unsaved glyph leaves no change");
    // A moved definition must also disappear from its original file when deleted.
    auto* movedDeletion = reopened.glyphs[0]; const auto movedName = movedDeletion->name();
    require(reopened.moveGlyphs({movedDeletion}, "glyphs/delete-only.mp") && reopened.removeGlyph(movedDeletion), "move then delete");
    require(reopened.saveFile(), reopened.projectError);
    Font afterMovedDeletion; load(afterMovedDeletion, root.filePath("madina.mp"));
    require(!afterMovedDeletion.glyphperName.contains(movedName), "moved deletion cannot return from original source");
    std::cout << "PASS multiple glyph files: " << compared << " identical outlines/advances/IDs; order, lossless save, ownership, moves, creation, reload and conflicts\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
