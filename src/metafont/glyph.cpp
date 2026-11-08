/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#include "glyph.hpp"

#include <cmath>
#include <iostream>
#include <QRegularExpression>

#include "GlyphParser/glyphdriver.h"
#include "commands.h"
#include "font.hpp"
#include "metafont.h"
#include "qcoreevent.h"
#include "qpainter.h"
#include "qstringbuilder.h"

Glyph::Param::Param(const Glyph::Param& a) {
  name = a.name;
  position = a.position;
  applytosubpath = a.applytosubpath;
  applytopoint = a.applytopoint;
  isEquation = a.isEquation;
  isInControllePath = a.isInControllePath;
  affects = a.affects;
  value = a.value;
  expr = a.expr->clone();
}
Glyph::Param::Param(Glyph::Param&& a) {
  name = a.name;
  position = a.position;
  applytosubpath = a.applytosubpath;
  applytopoint = a.applytopoint;
  isEquation = a.isEquation;
  isInControllePath = a.isInControllePath;
  affects = a.affects;
  value = a.value;
  expr = std::move(a.expr);
}
Glyph::Param& Glyph::Param::operator=(Glyph::Param const& a) {
  name = a.name;
  position = a.position;
  applytosubpath = a.applytosubpath;
  applytopoint = a.applytopoint;
  isEquation = a.isEquation;
  isInControllePath = a.isInControllePath;
  affects = a.affects;
  value = a.value;
  expr = a.expr->clone();
  return *this;
}
Glyph::Param& Glyph::Param::operator=(Glyph::Param&& a) {
  name = a.name;
  position = a.position;
  applytosubpath = a.applytosubpath;
  applytopoint = a.applytopoint;
  isEquation = a.isEquation;
  isInControllePath = a.isInControllePath;
  affects = a.affects;
  value = a.value;
  expr = std::move(a.expr);
  return *this;
}

Glyph::Glyph(QString code, Font* parent) : QObject((QObject*)parent) {
  edge = NULL;
  this->font = parent;
  m_unicode = -1;
  m_charcode = -1;

  for (const auto& axis : font->project.internalParameters()) axisNames.push_back(axis.key);
  this->setSource(code);

  m_undoStack = new QUndoStack(this);

  for (const auto& axis : font->project.internalParameters()) {
    QObject::setProperty(axis.key.toUtf8(), QVariant::fromValue(AxisType{axis.defaultValue}));
  }
}

Glyph::~Glyph() {
}
void Glyph::setBeginMacroName(QString macro) {
  m_beginmacroname = macro;
}
QString Glyph::beginMacroName() {
  return m_beginmacroname;
}

void Glyph::setSource(QString source, bool structureChanged) {
  isSetSource = true;
  blockSignals(true);
  isDirty = true;

  params.clear();
  dependents.clear();
  pictureNames.clear();

  QList<QByteArray>
      dynamicProperties = dynamicPropertyNames();
  for (int i = 0; i < dynamicProperties.length(); i++) {
    QByteArray propname = dynamicProperties[i];

    if (std::find(axisNames.begin(), axisNames.end(), propname) == axisNames.end()) {
      QVariant variant;
      setProperty(propname, variant);
    }
  }

  QHash<Glyph*, ComponentInfo>::iterator comp;
  for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
    Glyph* glyph = comp.key();
    disconnect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
  }

  m_components.clear();
  controlledPaths.clear();

  m_body = "";
  m_verbatim = "";

  glyphparser::Driver driver(*this);

  driver.parse_string(source.toStdString(), name().toStdString());

  edge = NULL;
  isDirty = true;

  m_source = this->source();

  isSetSource = false;

  blockSignals(false);

  // auto gg = receivers(SIGNAL(valueChanged(QString)));

  emit valueChanged("source", structureChanged);
}
QString Glyph::source() {
  if (isDirty) {
    edge = NULL;
    QString source;

    source = QString("%1(%2,%3,%4,%5,%6);\n").arg(beginMacroName()).arg(name()).arg(unicode()).arg(width()).arg(height()).arg(depth());
    // source = source % QString("%%glyphname:%1\n").arg(name());
    if (!image().path.isEmpty()) {
      Glyph::ImageInfo imageInfo = image();
      QTransform transform = imageInfo.transform;
      source = source % QString("%%backgroundimage:%1,%2,%3,%4,%5,%6,%7\n")
                            .arg(imageInfo.path)
                            .arg(imageInfo.pos.x())
                            .arg(imageInfo.pos.y())
                            .arg(transform.m11())
                            .arg(transform.m12())
                            .arg(transform.m21())
                            .arg(transform.m22());
    }

    if (!m_components.isEmpty()) {
      source = source % "%%begincomponents\n";
      QHash<Glyph*, ComponentInfo>::iterator i;
      for (i = m_components.begin(); i != m_components.end(); ++i) {
        ComponentInfo component = i.value();
        QTransform transform = component.transform;
        source = source % QString("drawcomponent(%1,%2,%3,%4,%5,%6,%7);\n").arg(i.key()->name()).arg(component.pos.x()).arg(component.pos.y()).arg(transform.m11()).arg(transform.m12()).arg(transform.m21()).arg(transform.m22());
      }
      source = source % "%%endcomponents\n";
    }

    if (params.size() > 0) {
      source = source % "%%beginparams\n";
      QPointF valpoint;
      QString typepoint;
      double valdouble;
      for (const auto& [key, param] : params) {
        QString propname = param.name;
        QString affect = param.isEquation ? "=" : ":=";

        auto expString = param.expr->toString();

        QString comment;

        if (!param.affects.isEmpty()) {
          comment = " % " + param.affects;
        }

        source = source % QString("%1 %2 %3;%4\n").arg(QString(propname)).arg(affect).arg(expString).arg(comment);
      }
      /*
      QMapIterator<QString, Param> i(params);
      while (i.hasNext()) {
        i.next();
        Param param = i.value();
        QString propname = param.name;
        QString affect = param.isEquation ? "=" : ":=";
        switch (param.type) {
        case expression:

          auto value = property(propname.toLatin1());

          auto exp = expressions.value(propname);

          exp->setConstantValue(value);

          auto expString = exp->toString();

          QString comment;

          if (!param.affects.isEmpty()) {
            comment = " % " + param.affects;
          }



          source = source % QString("%1 %2 %3;%4\n").arg(QString(propname)).arg(affect).arg(expString).arg(comment);
          break;
        }
      }*/
    }

    if (verbatim() != "") {
      source = source % "\n%%beginverbatim\n";
      source = source % verbatim();
      source = source % "%%endverbatim\n";
    }

    if (!controlledPaths.isEmpty()) {
      source = source % "\n%%beginpaths\n\n";
      QMapIterator<int, QMap<int, Glyph::Knot*> > j(controlledPaths);
      while (j.hasNext()) {
        j.next();
        QMap<int, Knot*> path = j.value();
        const Knot* firstpoint = path.first();
        const Knot* currentpoint = firstpoint;
        auto imageInfo = controlledPathNames[j.key()];
        if (imageInfo.pictureName != "currentpicture") {
          source = source % QString("controlledImage(%1)(\n").arg(imageInfo.pictureName);
          source = source % imageInfo.commandName % "\n";
        } else {
          if (imageInfo.commandName == "fill") {
            source = source % QString("fill\n");
          } else if (imageInfo.commandName == "fillc") {
            source = source % QString("fillc\n");
          } else {
            source = source % QString("controlledPath (%1,%2)(%3)(\n").arg(j.key()).arg(path.firstKey()).arg(imageInfo.commandName);
          }
        }

        QMapIterator<int, Glyph::Knot*> h(path);
        h.next();
        while (h.hasNext()) {
          h.next();
          const Knot* nextpoint;
          nextpoint = h.value();

          source = source % currentpoint->expr->toString();

          if (currentpoint->rightValue.jointtype == path_join_control) {
            source = source % QString(" .. controls %1").arg(currentpoint->rightValue.dirExpr->toString());
            if (!currentpoint->rightValue.isEqualAfter) {
              source = source % QString(" and %1").arg(nextpoint->leftValue.dirExpr->toString());
            }
            source = source % QString(" ..\n");
          } else {
            if (currentpoint->rightValue.type == mpgui_given) {
              source = source % QString(" {%1}").arg(currentpoint->rightValue.dirExpr->toString());
            }
            if (currentpoint->rightValue.jointtype == path_join_macro) {
              source = source % QString(" %1\n").arg(currentpoint->rightValue.macrovalue);
            } else {
              if (currentpoint->rightValue.tensionExpr) {
                source = source % QString(" .. tension %1%2").arg(currentpoint->rightValue.isAtleast ? "atleast " : "").arg(currentpoint->rightValue.tensionExpr->toString());
                if (!currentpoint->rightValue.isEqualAfter) {
                  if (nextpoint->leftValue.tensionExpr) {
                    source = source % QString(" and %1%2").arg(nextpoint->leftValue.isAtleast ? "atleast " : "").arg(nextpoint->leftValue.tensionExpr->toString());
                  }
                }
              }

              source = source % QString(" ..\n");
            }
            if (nextpoint->leftValue.type == mpgui_given) {
              source = source % QString(" {%1}").arg(nextpoint->leftValue.dirExpr->toString());
            }
          }
          currentpoint = h.value();
        }
        if (currentpoint == firstpoint || imageInfo.commandName != "draw") {
          source = source % QString(" cycle\n");
        } else {
          source = source % currentpoint->expr->toString() % "\n";
        }

        if (imageInfo.pictureName != "currentpicture" || (imageInfo.commandName != "fill" && imageInfo.commandName != "fillc")) {
          source = source % QString(");\n");
        } else {
          source = source % QString(";\n");
        }
      }
      source = source % "\n%%endpaths\n";
    }

    if (body() != "") {
      source = source % "%%beginbody\n";
      source = source % body();
    }

    if (m_beginmacroname == "beginchar") {
      source = source % "endchar;\n";
    } else {
      source = source % "enddefchar;\n";
    }

    m_source = source;
    isDirty = false;
  }
  return m_source;
}
void Glyph::setName(QString name) {
  if (name == m_name)
    return;

  font->glyphperName.remove(name);
  m_name = name;
  isDirty = true;
  font->glyphperName.insert(m_name, this);
  if (m_unicode == -1) {
    m_charcode = -1;
  }
  emit valueChanged("name");
}
QString Glyph::sourceFile() const { return font->glyphSourceFile(this); }
QString Glyph::name() const {
  return m_name;
}
void Glyph::setUnicode(int unicode) {
  if (unicode != m_unicode) {
    m_unicode = unicode;
    m_charcode = unicode;
    isDirty = true;
    if (edge) {
      edge->charcode = charcode();
    }
    emit valueChanged("unicode");
  }
}
int Glyph::unicode() const {
  return m_unicode;
}
int Glyph::charcode() {
  // if (m_charcode == -1) {
  m_charcode = font->getNumericVariable(name());
  //}
  return m_charcode;
}

void Glyph::setWidth(double width) {
  m_width = width;
  isDirty = true;
  emit valueChanged("width");
}
double Glyph::width() const {
  return m_width;
}
void Glyph::setHeight(double height) {
  m_height = height;
  isDirty = true;
  emit valueChanged("height");
}
double Glyph::height() const {
  return m_height;
}
void Glyph::setDepth(double depth) {
  m_depth = depth;
  isDirty = true;
  emit valueChanged("depth");
}
double Glyph::depth() const {
  return m_depth;
}
double Glyph::axis(QString name) {
  QVariant var = property(name.toLocal8Bit());
  Glyph::AxisType oldValue = var.value<Glyph::AxisType>();
  return oldValue.value;
}
void Glyph::setImage(Glyph::ImageInfo image) {
  ImageInfo old = m_image;
  m_image = image;
  isDirty = true;
  if (old.path != m_image.path) {
    emit valueChanged("image");
  } else if (old.pos != m_image.pos | old.transform != m_image.transform) {
    emit valueChanged("imagetransform");
  }
}
Glyph::ImageInfo Glyph::image() const {
  return m_image;
}
void Glyph::setComponents(QHashGlyphComponentInfo components) {
  isDirty = true;

  QHash<Glyph*, ComponentInfo>::iterator comp;
  for (comp = m_components.begin(); comp != m_components.end(); ++comp) {
    Glyph* glyph = comp.key();
    disconnect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
  }

  m_components.clear();

  for (comp = components.begin(); comp != components.end(); ++comp) {
    Glyph* glyph = comp.key();
    connect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);
  }

  m_components = components;

  emit valueChanged("components");
}
Glyph::QHashGlyphComponentInfo Glyph::components() const {
  return m_components;
}
void Glyph::setBody(QString body, bool autoParam) {
  m_body = body;
  isDirty = true;
  emit valueChanged("body");
}
QString Glyph::body() const {
  return m_body;
}
void Glyph::setVerbatim(QString verbatim) {
  m_verbatim = verbatim;
  isDirty = true;
  emit valueChanged("verbatim");
}
QString Glyph::verbatim() const {
  return m_verbatim;
}
void Glyph::setComponent(QString name, double x, double y, double t1, double t2, double t3, double t4) {
  isDirty = true;

  ComponentInfo component;
  Glyph* glyph = font->glyphperName[name];
  component.pos = QPointF(x, y);
  QTransform transform(t1, t2, t3, t4, 0, 0);
  component.transform = transform;
  m_components[glyph] = component;
  connect(glyph, &Glyph::valueChanged, this, &Glyph::componentChanged);

  emit valueChanged("components");
}
void Glyph::parseComponents(QString componentSource) {
}
void Glyph::parsePaths(QString pathsSource) {
}
void Glyph::componentChanged(QString name, bool structureChanged) {
  emit valueChanged("component", false);
}

void Glyph::setParameter(QString name, MFExpr* exp, bool isEquation) {
  setParameter(name, exp, isEquation, false, QString());
}
void Glyph::setParameter(QString name, MFExpr* exp, bool isEquation, bool isInControllePath) {
  setParameter(name, exp, isEquation, isInControllePath, QString());
}
void Glyph::setParameter(QString name, MFExpr* exp, bool isEquation, bool isInControllePath, QString affects) {
  Param param = {};

  param.name = name;
  param.isEquation = isEquation;
  param.affects = affects;
  param.expr = std::unique_ptr<MFExpr>{exp};

  param.isInControllePath = isInControllePath;

  if (name.contains("deltas")) {
    param.isInControllePath = true;
  }

  params.insert({param.name, std::move(param)});

  if (!affects.isEmpty()) {
    dependents.insert(affects, &params[name]);
  }

  // TODO : check this
  if (exp->isLiteral()) {
    setProperty(name.toLatin1(), exp->constantValue(0));
  }
}

QString Glyph::getLog() {
  return font->getLog();
}

QVector<int> Glyph::authoredPointIds(int path) const {
  QVector<int> result;
  const auto points=controlledPaths.value(path);
  if (points.isEmpty()) return result;
  const auto command=controlledPathNames.value(path).commandName;
  for (auto i=points.cbegin();i!=points.cend();++i) {
    const bool last=i.key()==points.lastKey() && i.key()!=points.firstKey();
    // Closed paths have a closing sentinel, not another authored point. Value
    // snapshots in structural undo may no longer share the first knot pointer.
    const bool closure=last && (i.value()==points.first() || i.value()->expr->toString()=="cycle" ||
        (command!="draw" && i.value()->expr->toString()==points.first()->expr->toString()));
    if (!closure) result.append(i.key());
  }
  return result;
}

static QSet<QString> anchorParametersForSource(Glyph& glyph, const QSet<QString>* onlyAnchors) {
  const auto& params = glyph.params;
  // Follow source references rather than relying only on the designer's names.
  // Legacy mark controls also use names such as z.mark1 and z.marktop.
  const auto normalize = [](QString name) {
    if (name.startsWith("x.") || name.startsWith("y.") ||
        (name.size() > 1 && (name[0] == 'x' || name[0] == 'y') && name[1].isDigit()))
      name[0] = 'z';
    return name;
  };
  static const QRegularExpression token("[A-Za-z][A-Za-z0-9_.]*");
  const auto references = [&](const QString& text) {
    QSet<QString> result;
    auto matches = token.globalMatch(text);
    while (matches.hasNext()) result.insert(normalize(matches.next().captured()));
    return result;
  };
  QString text = glyph.source();
  text.replace(QRegularExpression("\"[^\"]*\"|%[^\\n]*"), " ");
  QSet<QString> needed;
  if (!onlyAnchors || !onlyAnchors->isEmpty()) needed = {"leftanchor", "rightanchor"};
  static const QRegularExpression call("\\b(?:setMarkAnchor|setCursiveAnchor|setAnchor|defAnchor)\\s*\\(");
  auto calls = call.globalMatch(text);
  while (calls.hasNext()) {
    const auto match = calls.next();
    int depth = 1, start = match.capturedEnd(), argument = 0;
    bool wanted = !onlyAnchors;
    for (int i = start; i < text.size(); ++i) {
      const auto ch = text[i];
      if (ch == '(') ++depth;
      if (ch == ')') --depth;
      if ((ch == ',' && depth == 1) || depth == 0) {
        if (argument == 0 && onlyAnchors)
          wanted = onlyAnchors->contains(text.mid(start, i - start).trimmed());
        if (argument == 1 && wanted) needed.unite(references(text.mid(start, i - start)));
        ++argument;
        start = i + 1;
      }
      if (depth == 0) {
        // MetaPost also accepts setMarkAnchor(name)(value).
        if (argument == 1) {
          int next = i + 1;
          while (next < text.size() && text[next].isSpace()) ++next;
          if (next < text.size() && text[next] == '(') {
            start = ++next;
            depth = 1;
            while (next < text.size() && depth) {
              if (text[next] == '(') ++depth;
              if (text[next] == ')') --depth;
              ++next;
            }
            if (wanted) needed.unite(references(text.mid(start, next - start - 1)));
          }
        }
        break;
      }
    }
  }
  QMap<QString, QSet<QString>> dependencies;
  static const QRegularExpression assignment("\\b([A-Za-z][A-Za-z0-9_.]*)\\s*(?::=|=)\\s*([^;]+);");
  auto assignments = assignment.globalMatch(text);
  while (assignments.hasNext()) {
    const auto match = assignments.next();
    dependencies[normalize(match.captured(1))].unite(references(match.captured(2)));
  }
  bool changed;
  do {
    const auto previous = needed;
    for (const auto& name : previous) needed.unite(dependencies.value(name));
    changed = needed != previous;
  } while (changed);
  QSet<QString> result;
  for (const auto& [name, param] : params) {
    QString shortName = name.toLower();
    if (shortName.startsWith("z.")) shortName.remove(0, 2);
    if ((!onlyAnchors && (shortName.startsWith("mark") || shortName.startsWith("from.") ||
                          shortName.startsWith("anchor"))) ||
        needed.contains(normalize(name)) ||
        (!param.affects.isEmpty() && needed.contains(normalize(param.affects))))
      result.insert(name);
  }
  return result;
}

QSet<QString> Glyph::anchorParameterNames() { return anchorParametersForSource(*this, nullptr); }
QSet<QString> Glyph::anchorParameterNamesFor(const QSet<QString>& anchors) {
  return anchorParametersForSource(*this, &anchors);
}

QString Glyph::getInfo() {
  QString log;

  log = QString("charcode=%1\n").arg(charcode());
  auto edge = getEdge();
  if (edge) {
    log = log % QString("width=%1").arg(edge->width);
  }

  return log;
}

mp_edge_object* Glyph::getEdge() {
  if (edge && !isDirty)
    return edge;

  QString paramsString;

  // Localize preview parameters, including sparse named internal parameters. Rendering another
  // glyph must never inherit the current glyph's design coordinates.
  paramsString = "save params;";
  for (const auto& descriptor : font->project.internalParameters()) {
    paramsString += QString("params%1:=%2;").arg(descriptor.metaPostIndex)
        .arg(descriptor.metaPostValue(axis(descriptor.key)), 0, 'g', 15);
  }

  QString data = paramsString % source();
  if (!pictureNames.isEmpty()) {
    const int section=data.indexOf("%%endpaths");
    if (section>=0) data.insert(section,"vmfFinalSourceCapture:=currentpicture;\n");
  }

  try {
    font->pictureNames = pictureNames;
    font->executeMetaPost(data.toLatin1().toStdString());
    font->pictureNames.clear();
  } catch (const std::exception&) {
    font->pictureNames.clear();
    return nullptr;
  }

  const bool scaled = std::any_of(font->project.internalParameters().begin(), font->project.internalParameters().end(),
      [&](const auto& a) { return a.metaPostIndex == 100 && a.metaPostValue(axis(a.key)) != 0; });
  edge = font->getEdge(scaled ? 983040 : charcode());

  if (edge == nullptr) {
    return edge;
  }

  QPointF translate;
  if (edge != nullptr) {
    translate = QPointF(edge->xpart, edge->ypart);
  }

  for (auto& [name, param] : params) {
    if (!param.isInControllePath) {
      auto expr = param.expr.get();
      if (expr->type() != QVariant::Double) {
        QString varName = !param.affects.isEmpty() ? param.affects : name;
        QPointF point;
        if (font->getPairVariable(varName, point)) {
          param.value = point + translate;
        }
      }
    }
  }

  return edge;
}

Glyph::ComputedValues Glyph::getComputedValues() {
  mp_edge_object* mp_edge = getEdge();
  ComputedValues values;

  if (mp_edge) {
    values.charcode = mp_edge->charcode;
    values.width = mp_edge->width;
    values.height = mp_edge->height;
    values.depth = mp_edge->depth;
    values.bbox.llx = mp_edge->minx;
    values.bbox.urx = mp_edge->maxx;
    values.bbox.lly = mp_edge->miny;
    values.bbox.ury = mp_edge->maxy;

    if (!std::isnan(mp_edge->xleftanchor)) {
      values.leftAnchor = QPoint(mp_edge->xleftanchor, mp_edge->yleftanchor);
    }
    if (!std::isnan(mp_edge->xrightanchor)) {
      values.rightAnchor = QPoint(mp_edge->xrightanchor, mp_edge->yrightanchor);
    }
  }

  return values;
}

bool Glyph::event(QEvent* e) {
  if (e->type() == QEvent::DynamicPropertyChange) {
    QDynamicPropertyChangeEvent* pe = static_cast<QDynamicPropertyChangeEvent*>(e);
    if (!isSetSource) {
      isDirty = true;
      emit valueChanged(pe->propertyName());
    }
  }
  return QObject::event(e);  // don't forget this
}
QUndoStack* Glyph::undoStack() const {
  return m_undoStack;
}
void Glyph::setPropertyWithUndo(const QString& name, const QVariant& value) {
  ChangeGlyphPropertyCommand* command = new ChangeGlyphPropertyCommand(this, name, value);
  m_undoStack->push(command);
}
QPainterPath Glyph::getPathFromEdge(mp_edge_object* h) {
  QPainterPath localpath;

  if (h) {
    mp_graphic_object* body = h->body;

    if (body) {
      do {
        switch (body->type) {
          case mp_fill_code: {
            QPainterPath subpath = mp_dump_solved_path(((mp_fill_object*)body)->path_p);
            localpath.addPath(subpath);

            break;
          }
          default:
            break;
        }

      } while (body = body->next);
    }
  }

  return localpath;
}
bool Glyph::setProperty(const char* name, const QVariant& value, bool updateParam) {
  if (updateParam) {
    auto param = this->params.find(name);
    if (param != this->params.end()) {
      auto expr = param->second.expr.get();
      if (expr->isConstant(0)) {
        expr->setConstantValue(0, value);
      }
    }
  }
  return QObject::setProperty(name, value);
}
QPainterPath Glyph::getPath() {
  return getPath(getEdge());
}
QPicture Glyph::getPicture() {
  return getPicture(getEdge());
}
QPainterPath Glyph::getPath(mp_edge_object* h) {
  QPainterPath localpath;

  localpath.setFillRule(Qt::WindingFill);

  if (h) {
    mp_graphic_object* body = h->body;

    if (body) {
      do {
        switch (body->type) {
          case mp_stroked_code:
          case mp_fill_code: {
            QPainterPath subpath = mp_dump_solved_path(((mp_fill_object*)body)->path_p);
            localpath.addPath(subpath);

            break;
          }
          default:
            break;
        }

      } while (body = body->next);
    }
  }

  return localpath;
}
static QColor adaptColorForDarkTheme(const QColor& c) {
  if (!c.isValid())
    return c;

  qreal h, s, l, a;
  c.getHslF(&h, &s, &l, &a);

  // Special handling for grayscale colors
  if (s < 0.05) {
    // Invert grayscale perceptually
    qreal newL = 1.0 - l;

    // Avoid pure white glowing too much
    if (newL > 0.92)
      newL = 0.92;

    QColor out;
    out.setHslF(0.0, 0.0, newL, a);

    return out;
  }

  // --- Colored colors ---

  qreal newL;

  if (l < 0.5) {
    // Dark colors become bright
    newL = 0.72 + l * 0.18;
  } else {
    // Bright colors remain controlled
    newL = 0.82 + (l - 0.5) * 0.10;
  }

  // Slightly reduce saturation
  qreal newS = s * 0.85;

  // Clamp
  newL = std::clamp(newL, 0.0, 0.92);
  newS = std::clamp(newS, 0.18, 1.0);

  QColor out;
  out.setHslF(h, newS, newL, a);

  return out;
}
QPicture Glyph::getPicture(mp_edge_object* h) {
  QPicture pic;
  QPainter painter;

  painter.begin(&pic);

  if (h) {
    mp_graphic_object* body = h->body;

    if (body) {
      QPainterPath foregroudpath;
      foregroudpath.setFillRule(Qt::FillRule::WindingFill);

      do {
        switch (body->type) {
          case mp_stroked_code:
          case mp_fill_code: {
            auto fillobject = (mp_fill_object*)body;
            /*auto done = false;
            if (fillobject->pre_script && strcmp(fillobject->pre_script, "begincomponent") == 0) {
              auto comp = QString(fillobject->post_script).split(",");
              QString name = comp[0];
              GlyphVis& compGlyph = m_otLayout->glyphs[name];
              if (!compGlyph.isColored()) {
                auto initPosX = comp[1].toDouble() + matrix.xpart;
                auto initPosY = comp[2].toDouble() + matrix.ypart;
                painter.save();
                painter.translate(initPosX, initPosY);
                painter.fillPath(compGlyph.path, QColor(Qt::black));
                painter.restore();
                body = body->next;
                while (body->type != mp_stroked_code || ((mp_stroked_object*)body)->pre_script == nullptr || strcmp(((mp_stroked_object*)body)->pre_script, "endcomponent")) body = body->next;
                done = true;
              }

            }
            if (!done) {*/
            QPainterPath subpath = mp_dump_solved_path(fillobject->path_p);
            if (fillobject->color_model == mp_rgb_model) {
              auto color = QColor(fillobject->color.a_val * 255, fillobject->color.b_val * 255, fillobject->color.c_val * 255);
              // color= adaptColorForDarkTheme(color);
              painter.fillPath(subpath, color);
            } else if (fillobject->color_model == mp_no_model) {
              foregroudpath.addPath(subpath);
            }
            //}
          }
        }
      } while (body = body->next);

      painter.fillPath(foregroudpath, QColor(Qt::black));
    }
  }

  painter.end();

  return pic;
}

QPainterPath Glyph::mp_dump_solved_path(mp_gr_knot h) {
  mp_gr_knot p, q;
  QPainterPath path;
  // path.setFillRule(Qt::OddEvenFill);
  if (h == NULL) return path;

  path.moveTo(h->x_coord, h->y_coord);
  p = h;
  do {
    q = p->next;
    path.cubicTo(p->right_x, p->right_y, q->left_x, q->left_y, q->x_coord, q->y_coord);

    p = q;
  } while (p != h);
  if (h->data.types.left_type != mp_endpoint)
    path.closeSubpath();

  return path;
}
