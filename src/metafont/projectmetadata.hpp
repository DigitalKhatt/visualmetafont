#pragma once

#include <QJsonObject>
#include <QMap>
#include <QPointF>
#include <QStringList>
#include <QVector>

// Internal design coordinates have no OpenType tag. Their units and MetaPost
// slots are independent of any final user-facing variation axis.
struct ProjectParameter {
  QString key, label, description, metaPostMapping;
  int metaPostIndex = 0;
  double minimum = -10, defaultValue = 0, maximum = 20, step = 1;
  bool visible = true;
  double metaPostValue(double value) const;
};
struct OpenTypeParameterMapping {
  QString internalParameter;
  QVector<QPointF> points; // x: OpenType coordinate, y: internal coordinate
  double internalValue(double axisValue) const;
};
struct ProjectOpenTypeAxis {
  QString tag, label;
  double minimum = 0, defaultValue = 0, maximum = 1;
  QVector<OpenTypeParameterMapping> mappings;
};
struct ProjectGroup { QString id, label; };

class ProjectMetadata {
 public:
  bool load(const QString& path, QString* error = nullptr);
  bool save(const QString& path, QString* error = nullptr) const;
  const QVector<ProjectParameter>& internalParameters() const { return parameters_; }
  const QVector<ProjectOpenTypeAxis>& openTypeAxes() const { return openTypeAxes_; }
  // A partial map: only parameters driven by this axis. It never changes a
  // glyph, registers a runtime axis, or changes export behavior by itself.
  QMap<QString, double> internalValuesForAxis(const QString& tag, double value) const;
  QStringList glyphFiles() const;
  bool setGlyphFiles(const QStringList& files, QString* error = nullptr);
  static bool validateGlyphFiles(const QStringList& files, QString* error = nullptr);
  QVector<ProjectGroup> groups() const;
  QStringList glyphGroups(const QString& name) const;
  bool setGlyphGroups(const QString& name, const QStringList& groups);
  QString title() const { return document_.value("name").toString(); }
  bool isPresent() const { return !document_.isEmpty(); }
 private:
  QJsonObject document_;
  QVector<ProjectParameter> parameters_ = legacyParameters();
  QVector<ProjectOpenTypeAxis> openTypeAxes_;
  static QVector<ProjectParameter> legacyParameters();
};
