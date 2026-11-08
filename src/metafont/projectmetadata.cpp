#include "projectmetadata.hpp"
#include "digitalkhatt/project/GlyphSources.h"

#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSet>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <stdexcept>

QVector<ProjectParameter> ProjectMetadata::legacyParameters() {
  QVector<ProjectParameter> result;
  const QStringList keys{"lefttatweel", "righttatweel", "third", "fourth", "fifth"};
  const QStringList labels{"Left extension", "Right extension", "Third axis", "Fourth axis", "Fifth axis"};
  for (int i = 0; i < keys.size(); ++i)
    result.append({keys[i], labels[i], {}, "offset", i, -10, 0, 20, 0.1});
  return result;
}

double ProjectParameter::metaPostValue(double value) const {
  value = std::clamp(value, minimum, maximum);
  if (value == defaultValue) return 0;
  if (metaPostMapping == "absolute") return value;
  if (metaPostMapping == "normalized")
    return (value - defaultValue) / (value < defaultValue ? defaultValue - minimum : maximum - defaultValue);
  return value - defaultValue;
}

bool ProjectMetadata::load(const QString& path, QString* error) {
  auto fail = [&](const QString& text) { if (error) *error = text; return false; };
  if (!QFile::exists(path)) { document_ = {}; parameters_ = legacyParameters(); openTypeAxes_.clear(); return true; }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) return fail(file.errorString());
  QJsonParseError parse;
  auto json = QJsonDocument::fromJson(file.readAll(), &parse);
  if (parse.error != QJsonParseError::NoError || !json.isObject())
    return fail("Invalid project metadata: " + parse.errorString());
  const auto root = json.object();
  const bool legacy = root.value("schemaVersion") == QJsonValue(1);
  if (!legacy && root.value("schemaVersion") != QJsonValue(2)) return fail("Unsupported project metadata schemaVersion");
  if (!legacy && root.contains("axes")) return fail("Use separate internalParameters and openTypeAxes arrays");
  try { digitalkhatt::project::glyphFilesFromMetadata(json.toJson(QJsonDocument::Compact).toStdString()); }
  catch (const std::exception& e) { return fail(QString::fromUtf8(e.what())); }

  QSet<QString> groupIds;
  if (!root.value("groups").isArray()) return fail("groups must be an array");
  for (const auto& value : root.value("groups").toArray()) {
    const auto group = value.toObject();
    const auto id = group.value("id").toString();
    if (!QRegularExpression("^[a-z][a-z0-9-]*$").match(id).hasMatch() || groupIds.contains(id) || group.value("label").toString().isEmpty())
      return fail("Groups need unique IDs and nonempty labels");
    groupIds.insert(id);
  }
  if (!root.value("glyphGroups").isObject()) return fail("glyphGroups must be an object");
  const auto assignments = root.value("glyphGroups").toObject();
  for (auto it = assignments.begin(); it != assignments.end(); ++it) {
    if (!it.value().isArray()) return fail("Glyph groups must be arrays: " + it.key());
    QSet<QString> seen;
    for (const auto& value : it.value().toArray()) {
      const auto id = value.toString();
      if (!groupIds.contains(id) || seen.contains(id)) return fail("Invalid glyph group: " + it.key());
      seen.insert(id);
    }
  }
  QVector<ProjectParameter> parameters;
  QSet<QString> keys;
  const QSet<QString> reservedKeys{"objectName", "sourceFile", "name", "unicode", "width", "height", "depth", "source", "body", "verbatim", "image", "components", "parameters"};
  QSet<int> indices;
  const auto parametersKey = legacy ? "axes" : "internalParameters";
  if (!root.value(parametersKey).isArray()) return fail("internalParameters must be an array");
  for (const auto& value : root.value(parametersKey).toArray()) {
    const auto a = value.toObject();
    ProjectParameter parameter;
    parameter.key = a.value("key").toString(); parameter.label = a.value("label").toString();
    parameter.description = a.value("description").toString();
    parameter.metaPostMapping = a.value(legacy ? "mapping" : "metaPostMapping").toString("offset");
    parameter.visible = a.value("visible").toBool(true);
    const auto indexKey = legacy ? "parameter" : "metaPostIndex";
    parameter.metaPostIndex = a.value(indexKey).toInt(-1);
    parameter.minimum = a.value("min").toDouble(NAN); parameter.defaultValue = a.value("default").toDouble(NAN);
    parameter.maximum = a.value("max").toDouble(NAN); parameter.step = a.value("step").toDouble(1);
    if (!legacy && a.contains("tag")) return fail("OpenType tags belong in openTypeAxes: " + parameter.key);
    if (!QRegularExpression("^[A-Za-z][A-Za-z0-9_]*$").match(parameter.key).hasMatch() ||
        (a.contains("visible") && !a.value("visible").isBool()) ||
        keys.contains(parameter.key) || reservedKeys.contains(parameter.key) || parameter.label.isEmpty() ||
        parameter.metaPostIndex < 0 || parameter.metaPostIndex > 65535 || indices.contains(parameter.metaPostIndex) ||
        a.value(indexKey).toDouble(-1) != parameter.metaPostIndex ||
        !std::isfinite(parameter.minimum) || !std::isfinite(parameter.defaultValue) || !std::isfinite(parameter.maximum) ||
        !(parameter.minimum < parameter.maximum && parameter.minimum <= parameter.defaultValue && parameter.defaultValue <= parameter.maximum) ||
        !std::isfinite(parameter.step) || parameter.step <= 0 ||
        (parameter.metaPostMapping != "offset" && parameter.metaPostMapping != "absolute" && parameter.metaPostMapping != "normalized"))
      return fail("Invalid or duplicate internal parameter: " + parameter.key);
    keys.insert(parameter.key); indices.insert(parameter.metaPostIndex);
    parameters.append(parameter);
  }
  // Fixed legacy slots retain their meanings for existing glyph designs.
  for (const auto& parameter : legacyParameters()) {
    bool found = false;
    for (const auto& p : parameters) if (p.key == parameter.key || p.metaPostIndex == parameter.metaPostIndex) {
      if (p.key != parameter.key || p.metaPostIndex != parameter.metaPostIndex || p.defaultValue != 0 || p.metaPostMapping != "offset")
        return fail("Legacy parameter slots 0–4 cannot be remapped");
      found = true;
    }
    if (!found) parameters.append(parameter);
  }
  QVector<ProjectOpenTypeAxis> openTypeAxes;
  QSet<QString> tags;
  if (root.contains("openTypeAxes") && !root.value("openTypeAxes").isArray()) return fail("openTypeAxes must be an array");
  for (const auto& value : root.value("openTypeAxes").toArray()) {
    const auto a = value.toObject();
    ProjectOpenTypeAxis axis;
    axis.tag = a.value("tag").toString(); axis.label = a.value("label").toString();
    axis.minimum = a.value("min").toDouble(NAN); axis.defaultValue = a.value("default").toDouble(NAN);
    axis.maximum = a.value("max").toDouble(NAN);
    if (!QRegularExpression("^[ -~]{4}$").match(axis.tag).hasMatch() || tags.contains(axis.tag) || axis.label.isEmpty() ||
        !std::isfinite(axis.minimum) || !std::isfinite(axis.defaultValue) || !std::isfinite(axis.maximum) ||
        !(axis.minimum < axis.maximum && axis.minimum <= axis.defaultValue && axis.defaultValue <= axis.maximum) ||
        !a.value("mappings").isArray() || a.value("mappings").toArray().isEmpty())
      return fail("Invalid or duplicate OpenType axis: " + axis.tag);
    QSet<QString> targets;
    for (const auto& binding : a.value("mappings").toArray()) {
      const auto object = binding.toObject();
      OpenTypeParameterMapping mapping;
      mapping.internalParameter = object.value("internalParameter").toString();
      const ProjectParameter* parameter = nullptr;
      for (const auto& p : parameters) if (p.key == mapping.internalParameter) parameter = &p;
      if (!parameter || targets.contains(mapping.internalParameter) || !object.value("points").isArray())
        return fail("Unknown or duplicate internal parameter mapping for " + axis.tag);
      const auto points = object.value("points").toArray();
      if (points.size() < 2) return fail("Axis mappings need at least two points: " + axis.tag);
      for (const auto& item : points) {
        const auto pair = item.toArray();
        const double x = pair.size() == 2 ? pair[0].toDouble(NAN) : NAN;
        const double y = pair.size() == 2 ? pair[1].toDouble(NAN) : NAN;
        if (!std::isfinite(x) || !std::isfinite(y) || x < axis.minimum || x > axis.maximum ||
            y < parameter->minimum || y > parameter->maximum || (!mapping.points.isEmpty() && x <= mapping.points.last().x()))
          return fail("Invalid mapping points for " + axis.tag + " / " + parameter->key);
        mapping.points.append(QPointF(x, y));
      }
      if (mapping.points.first().x() != axis.minimum || mapping.points.last().x() != axis.maximum ||
          std::abs(mapping.internalValue(axis.defaultValue) - parameter->defaultValue) > 1e-9)
        return fail("Mappings must cover the full axis range and preserve the default: " + axis.tag);
      targets.insert(parameter->key); axis.mappings.append(mapping);
    }
    tags.insert(axis.tag); openTypeAxes.append(axis);
  }
  document_ = root; parameters_ = parameters; openTypeAxes_ = openTypeAxes;
  return true;
}

double OpenTypeParameterMapping::internalValue(double axisValue) const {
  if (!std::isfinite(axisValue) || points.size() < 2) throw std::invalid_argument("Invalid axis mapping input");
  if (axisValue <= points.first().x()) return points.first().y();
  for (int i = 1; i < points.size(); ++i) {
    if (axisValue <= points[i].x()) {
      const auto a = points[i - 1], b = points[i];
      const double t = (axisValue - a.x()) / (b.x() - a.x());
      return a.y() + t * (b.y() - a.y());
    }
  }
  return points.last().y();
}
QMap<QString, double> ProjectMetadata::internalValuesForAxis(const QString& tag, double value) const {
  for (const auto& axis : openTypeAxes_) if (axis.tag == tag) {
    QMap<QString, double> result;
    for (const auto& mapping : axis.mappings) {
      double coordinate = mapping.internalValue(value);
      if (value == axis.defaultValue)
        for (const auto& p : parameters_) if (p.key == mapping.internalParameter) coordinate = p.defaultValue;
      result.insert(mapping.internalParameter, coordinate);
    }
    return result;
  }
  throw std::invalid_argument("Unknown OpenType axis: " + tag.toStdString());
}

bool ProjectMetadata::save(const QString& path, QString* error) const {
  if (!isPresent()) return true;
  QSaveFile file(path);
  const auto data = QJsonDocument(document_).toJson(QJsonDocument::Indented);
  if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
    if (error) *error = file.errorString();
    return false;
  }
  return true;
}
QVector<ProjectGroup> ProjectMetadata::groups() const {
  QVector<ProjectGroup> result;
  for (const auto& value : document_.value("groups").toArray()) {
    const auto group = value.toObject();
    result.append({group.value("id").toString(), group.value("label").toString()});
  }
  return result;
}
QStringList ProjectMetadata::glyphGroups(const QString& name) const {
  QStringList result;
  for (const auto& value : document_.value("glyphGroups").toObject().value(name).toArray()) result.append(value.toString());
  return result;
}
bool ProjectMetadata::setGlyphGroups(const QString& name, const QStringList& groups) {
  if (!isPresent() || name.isEmpty()) return false;
  QSet<QString> valid;
  for (const auto& group : this->groups()) valid.insert(group.id);
  QJsonArray values;
  QSet<QString> seen;
  for (const auto& id : groups) {
    if (!valid.contains(id) || seen.contains(id)) return false;
    seen.insert(id); values.append(id);
  }
  auto assignments = document_.value("glyphGroups").toObject();
  assignments.insert(name, values); document_.insert("glyphGroups", assignments);
  return true;
}

QStringList ProjectMetadata::glyphFiles() const {
  if (!document_.contains("glyphFiles")) return {"glyphs.mp"};
  QStringList result;
  for (const auto& value : document_.value("glyphFiles").toArray()) result.append(QDir::cleanPath(value.toString()));
  return result;
}
bool ProjectMetadata::validateGlyphFiles(const QStringList& files, QString* error) {
  std::vector<std::string> paths;
  for (const auto& file : files) paths.push_back(file.toStdString());
  try { digitalkhatt::project::normalizeGlyphFiles(paths); return true; }
  catch (const std::exception& e) { if (error) *error = QString::fromUtf8(e.what()); return false; }
}
bool ProjectMetadata::setGlyphFiles(const QStringList& files, QString* error) {
  if (!validateGlyphFiles(files, error)) return false;
  if (!isPresent()) document_ = QJsonObject{{"schemaVersion", 2}, {"groups", QJsonArray{}},
      {"glyphGroups", QJsonObject{}}, {"internalParameters", QJsonArray{}}};
  QJsonArray values;
  for (const auto& file : files) values.append(QDir::cleanPath(file));
  document_.insert("glyphFiles", values);
  return true;
}
