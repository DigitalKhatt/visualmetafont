#include "projectmetadata.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

static void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
static void write(const QString& path, const QJsonObject& value) {
  QFile file(path); require(file.open(QIODevice::WriteOnly), "open fixture");
  file.write(QJsonDocument(value).toJson());
}
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    QTemporaryDir dir;
    const auto path = dir.filePath("font-project.json");
    ProjectMetadata project;
    QString error;
    require(project.load(path, &error), "legacy project");
    require(project.internalParameters().size() == 5 && !project.isPresent(), "legacy axes");
    require(project.save(path) && !QFile::exists(path), "do not create legacy sidecars");
    QJsonObject root{{"schemaVersion", 1}, {"name", "Fixture"}, {"futureField", "retain"},
      {"groups", QJsonArray{QJsonObject{{"id", "arabic"}, {"label", "Arabic"}}, QJsonObject{{"id", "marks"}, {"label", "Marks"}}}},
      {"glyphGroups", QJsonObject{{"beh", QJsonArray{"arabic"}}}},
      {"axes", QJsonArray{QJsonObject{{"key", "weight"}, {"label", "Weight"}, {"tag", "wght"},
        {"parameter", 101}, {"min", 100}, {"default", 400}, {"max", 900}, {"mapping", "normalized"}},
        QJsonObject{{"key", "scale"}, {"label", "Scale"}, {"parameter", 100}, {"min", 50}, {"default", 100}, {"max", 200}, {"mapping", "absolute"}}}}};
    write(path, root);
    require(project.load(path, &error), qPrintable(error));
    const auto weight = project.internalParameters()[0], scale = project.internalParameters()[1];
    require(weight.metaPostValue(400) == 0 && weight.metaPostValue(100) == -1 && weight.metaPostValue(900) == 1 && weight.metaPostValue(650) == .5, "weight coordinates");
    require(weight.metaPostValue(9999) == 1 && scale.metaPostValue(100) == 0 && scale.metaPostValue(150) == 150, "clamping and scale sentinel");
    require(project.internalParameters().size() == 7, "legacy slots retained");
    require(!project.setGlyphGroups("beh", {"missing"}), "reject unknown membership");
    require(project.setGlyphGroups("beh", {"arabic", "marks"}) && project.save(path), "save groups");
    ProjectMetadata reloaded;
    require(reloaded.load(path), "reload groups");
    require(reloaded.glyphGroups("beh") == QStringList({"arabic", "marks"}), "memberships survive reload");
    QFile saved(path); saved.open(QIODevice::ReadOnly);
    require(QJsonDocument::fromJson(saved.readAll()).object().value("futureField") == QJsonValue("retain"), "preserve unknown metadata");
    auto invalid = root;
    auto axes = root.value("axes").toArray(); auto a = axes[0].toObject();
    a["parameter"] = 100; axes[0] = a; invalid["axes"] = axes; write(path, invalid);
    require(!project.load(path, &error) && !error.isEmpty(), "reject duplicate slots");
    require(project.glyphGroups("beh").contains("marks"), "failed load leaves state intact");
    axes = root.value("axes").toArray(); a = axes[0].toObject(); a["key"] = "width"; axes[0] = a;
    invalid = root; invalid["axes"] = axes; write(path, invalid);
    require(!project.load(path), "reserved glyph properties cannot be axes");
    invalid = root; invalid["schemaVersion"] = 3; write(path, invalid);
    require(!project.load(path), "reject unknown schema");
    invalid = root; invalid["glyphGroups"] = QJsonObject{{"beh", QJsonArray{"unknown"}}}; write(path, invalid);
    require(!project.load(path), "reject invalid memberships");
    axes = root.value("axes").toArray(); a = axes[0].toObject(); a["parameter"] = 0; axes[0] = a;
    invalid = root; invalid["axes"] = axes; write(path, invalid);
    require(!project.load(path), "do not reuse legacy slots");
    // Schema 2 separates internal slots from final OpenType axes. One final
    // axis can use different units and non-linear mappings for multiple slots.
    QJsonObject separated = root;
    separated["schemaVersion"] = 2;
    QJsonArray internal;
    for (const auto& value : root.value("axes").toArray()) {
      auto p = value.toObject();
      p.remove("tag"); p["metaPostIndex"] = p.take("parameter"); p["metaPostMapping"] = p.take("mapping");
      internal.append(p);
    }
    internal.append(QJsonObject{{"key", "inkGain"}, {"label", "Ink gain"}, {"metaPostIndex", 102},
      {"min", -20}, {"default", 0}, {"max", 40}});
    separated.remove("axes"); separated["internalParameters"] = internal;
    const QJsonArray bindings{
      QJsonObject{{"internalParameter", "weight"}, {"points", QJsonArray{QJsonArray{100,100}, QJsonArray{400,400}, QJsonArray{900,900}}}},
      QJsonObject{{"internalParameter", "inkGain"}, {"points", QJsonArray{QJsonArray{100,-20}, QJsonArray{400,0}, QJsonArray{650,30}, QJsonArray{900,40}}}}};
    separated["openTypeAxes"] = QJsonArray{QJsonObject{{"tag", "wght"}, {"label", "Weight"}, {"min",100}, {"default",400}, {"max",900}, {"mappings",bindings}}};
    write(path, separated);
    require(project.load(path, &error), qPrintable(error));
    require(project.openTypeAxes().size() == 1 && project.internalParameters().size() == 8, "separate parameter and axis collections");
    const auto neutral = project.internalValuesForAxis("wght", 400);
    require(neutral["weight"] == 400 && neutral["inkGain"] == 0 && neutral.size() == 2, "axis defaults preserve internal defaults and only return driven parameters");
    const auto mid = project.internalValuesForAxis("wght", 525);
    require(mid["weight"] == 525 && mid["inkGain"] == 15, "one axis drives two parameters with different curves and units");
    require(project.internalValuesForAxis("wght", 9999)["inkGain"] == 40, "mapping clamps to endpoint");
    require(project.setGlyphGroups("beh", {"marks"}) && project.save(path), "save separated metadata");
    require(reloaded.load(path) && reloaded.internalValuesForAxis("wght", 525) == mid, "axis mappings survive membership save and reload");
    auto bad = separated;
    auto values = internal; auto p = values[0].toObject(); p["tag"] = "wght"; values[0] = p;
    bad["internalParameters"] = values; write(path,bad);
    require(!project.load(path), "internal parameters must not carry OpenType tags");
    auto invalidBinding = bindings[1].toObject(); invalidBinding["internalParameter"] = "missing";
    auto finalAxis = separated["openTypeAxes"].toArray()[0].toObject();
    bad = separated; finalAxis["mappings"] = QJsonArray{invalidBinding}; bad["openTypeAxes"] = QJsonArray{finalAxis}; write(path,bad);
    require(!project.load(path), "reject unknown mapping target");
    invalidBinding = bindings[1].toObject();
    invalidBinding["points"] = QJsonArray{QJsonArray{100,-20}, QJsonArray{400,5}, QJsonArray{900,40}};
    finalAxis["mappings"] = QJsonArray{invalidBinding}; bad["openTypeAxes"] = QJsonArray{finalAxis}; write(path,bad);
    require(!project.load(path), "reject mapping that moves the neutral design");
    invalidBinding["points"] = QJsonArray{QJsonArray{100,-20}, QJsonArray{400,0}, QJsonArray{300,5}, QJsonArray{900,40}};
    finalAxis["mappings"] = QJsonArray{invalidBinding}; bad["openTypeAxes"] = QJsonArray{finalAxis}; write(path,bad);
    require(!project.load(path), "reject unordered mapping points");
    require(project.glyphFiles() == QStringList{"glyphs.mp"}, "legacy single-file default");
    write(path, separated);
    require(project.load(path), "restore valid metadata");
    require(project.setGlyphFiles({"glyphs/core.mp", "glyphs/marks.mp"}) && project.save(path), "save ordered glyph source manifest");
    require(reloaded.load(path) && reloaded.glyphFiles() == QStringList({"glyphs/core.mp", "glyphs/marks.mp"}), "source file order survives reload");
    require(!project.setGlyphFiles({}), "reject empty manifest");
    require(!project.setGlyphFiles({"glyphs/core.mp", "glyphs/./core.mp"}), "reject duplicate normalized source path");
    require(!project.setGlyphFiles({"../outside.mp"}) && !project.setGlyphFiles({"/absolute.mp"}), "source paths are relative to the project");
    bad = separated; bad["glyphFiles"] = "glyphs.mp"; write(path, bad);
    require(!project.load(path), "reject non-array source manifest");
    std::cout << "Project metadata: legacy defaults, schema, coordinates, round-trip and invalid input passed\n";
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
