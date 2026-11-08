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

#include "font.hpp"

#include <filesystem>
#include <algorithm>
#include <climits>
#include <memory>
#include <vector>
#include <QSaveFile>
#include <QSet>

#include "glyph.hpp"
#include "hb.hh"
#include "metafont.h"
#include "qapplication.h"
#include "qdebug.h"
#include "qdir.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qregularexpression.h"
#include "qtextstream.h"

namespace fs = std::filesystem;

Font::Font(QObject* parent) : QObject(parent) {
}
bool Font::loadFile(const QString& fileName) {
  if (!project.load(QFileInfo(fileName).absolutePath() + "/font-project.json", &projectError))
    return false;
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    return false;
  }

  QFile rsmfplain(":/metafont/mfplain.mp");
  QFile rsmpost(":/metafont/mpost.mp");
  QFile rsvmf(":/metafont/vmf.mp");
  QString initMF = "MPGUI:=1;";
  if (!rsmfplain.open(QIODevice::ReadOnly)) {
    qDebug() << "mfplain.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsmfplain.readAll());
  }

  if (!rsmpost.open(QIODevice::ReadOnly)) {
    qDebug() << "mpost.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsmpost.readAll());
  }

  if (!rsvmf.open(QIODevice::ReadOnly)) {
    qDebug() << "vmf.mp file not opened" << endl;
    return false;
  } else {
    initMF.append(rsvmf.readAll());
  }

  initMF.append(file.readAll());

  fs::path p1 = fileName.toStdString();

  auto parentPath = p1.parent_path();
  m_currentDir = QString::fromStdString(parentPath.string());
  m_mpFont.initialize(initMF.toLocal8Bit().toStdString(), p1);
  synchronizeAnchorVisibility();
  m_fontName = QString::fromStdString(m_mpFont.fontName());

  sourceFiles_.clear();
  glyphOrigins_.clear();
  try {
    const auto sources = digitalkhatt::project::ProjectGlyphSources::load(p1);
    for (const auto& source : sources.files)
      sourceFiles_.append({QString::fromStdString(source.relativePath), GlyphSourceDocument(source.document)});
  } catch (const std::exception& error) {
    projectError = QString::fromUtf8(error.what());
    return false;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  for (int fileIndex = 0; fileIndex < sourceFiles_.size(); ++fileIndex) {
    const auto& sourceFile = sourceFiles_[fileIndex];
    for (int blockIndex = 0; blockIndex < sourceFile.document.blocks.size(); ++blockIndex) {
      const auto& block = sourceFile.document.blocks[blockIndex];
      Glyph* glyph = new Glyph(block.source, this);
      for (const auto& parameter : project.internalParameters()) {
        if (glyph->params.contains(parameter.key)) {
          projectError = "Internal parameter conflicts with glyph parameter: " + glyph->name() + ": " + parameter.key;
          delete glyph;
          QApplication::restoreOverrideCursor();
          return false;
        }
      }
      glyphs.append(glyph);
      glyphOrigins_.insert(glyph, {fileIndex, blockIndex, glyph->source(), sourceFile.path});
      m_mpFont.registerGlyphSource(glyph->name().toStdString(), glyph->source().toStdString(),
                                  glyph->beginMacroName().toStdString(), glyph->unicode());
    }
  }

  QFileInfo fileInfo(fileName);

  m_path = fileInfo.absoluteFilePath();

  file.close();

  return true;
}
double Font::lineHeight() {
  double lineheight = getInternalNumericVariable("lineheight");

  return lineheight;
}
double Font::getNumericVariable(QString name) {
  return m_mpFont.numericVariable(name.toStdString());
}
bool Font::getBoolVariable(QString name) {
  return m_mpFont.boolVariable(name.toStdString());
}
double Font::getInternalNumericVariable(QString name) {
  return m_mpFont.internalNumericVariable(name.toStdString());
}

void Font::synchronizeAnchorVisibility() {
  const bool visible = getInternalNumericVariable("showAnchors") != 0;
  if (visible == anchorsVisible_) return;
  anchorsVisible_ = visible;
  emit anchorsVisibilityChanged(visible);
}

void Font::setAnchorsVisible(bool visible) {
  if (visible == anchorsVisible_) return;
  executeMetaPost(visible ? "showAnchors:=1;" : "showAnchors:=0;");
}

QString Font::familyName() {
  return QString::fromStdString(familyNameStd());
}

std::string Font::familyNameStd() {
  return m_mpFont.familyName();
}
QString Font::copyright() {
  return QString::fromStdString(copyrightStd());
}

std::string Font::copyrightStd() {
  return m_mpFont.copyright();
}
bool Font::getPairVariable(QString name, QPointF& point) {
  if (name[0] == 'z' && (name.size() == 1 || name[1].isDigit() || name[1] == '.')) {
    double x, y = 0;
    name[0] = 'x';
    x = m_mpFont.numericVariable(name.toStdString());
    name[0] = 'y';
    y = m_mpFont.numericVariable(name.toStdString());
    point = QPointF(x, y);
    return true;

  } else {
    double x, y;
    if (!m_mpFont.pairVariable(name.toStdString(), x, y)) {
      return false;
    }
    point = QPointF(x, y);
    return true;
  }
  return false;
}
bool Font::saveUnicodes() {
  if (m_path.isEmpty()) return false;

  QFileInfo fileInfo(m_path);

  QString unicodefileName = fileInfo.absolutePath() + "/output/" + fileInfo.baseName() + "_unicodes.lua";

  QFile unicodesfile(unicodefileName);

  if (!unicodesfile.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  QTextStream outunicode(&unicodesfile);

  outunicode << fileInfo.baseName() << ".unicodes = {\n";

  for (int i = 0; i < glyphs.length(); i++) {
    outunicode << "  [\"" << glyphs[i]->name() << "\"] = " << glyphs[i]->charcode() << ",\n";
  }

  outunicode << "}";

  unicodesfile.close();

  return true;
}
bool Font::glyphHasSourceChanges(Glyph* glyph) const {
  const auto origin=glyphOrigins_.constFind(glyph);
  return origin==glyphOrigins_.cend() || origin->file<0 || origin->block<0 ||
      origin->target!=sourceFiles_[origin->file].path || glyph->source()!=origin->canonical;
}
bool Font::hasSourceChanges() {
  int retainedBlocks = 0;
  for (auto* glyph : glyphs) {
    if (glyphHasSourceChanges(glyph)) return true;
    ++retainedBlocks;
  }
  int originalBlocks = 0;
  for (const auto& file : sourceFiles_) originalBlocks += file.document.blocks.size();
  return retainedBlocks != originalBlocks;
}
bool Font::saveFile() {
  projectError.clear();
  if (m_path.isEmpty()) return false;
  QSet<QString> names;
  for (auto* glyph : glyphs) {
    if (glyph->name().isEmpty() || names.contains(glyph->name()) || !glyphOrigins_.contains(glyph)) {
      projectError = "Unowned or duplicate glyph: " + glyph->name();
      return false;
    }
    names.insert(glyph->name());
  }
  QVector<QByteArray> rendered;
  QVector<GlyphSourceDocument> parsed;
  for (int fileIndex = 0; fileIndex < sourceFiles_.size(); ++fileIndex) {
    QMap<int, QString> replacements;
    QStringList appended;
    QSet<int> retainedBlocks;
    for (auto* glyph : glyphs) {
      const auto& origin = glyphOrigins_[glyph];
      if (origin.file == fileIndex && origin.block >= 0) {
        retainedBlocks.insert(origin.block);
        if (origin.target != sourceFiles_[fileIndex].path) replacements.insert(origin.block, {});
        else if (glyph->source() != origin.canonical) replacements.insert(origin.block, glyph->source().trimmed());
      }
      if (origin.target == sourceFiles_[fileIndex].path && (origin.file != fileIndex || origin.block < 0)) {
        // A pure move retains the original definition's formatting.
        appended.append(origin.file >= 0 && origin.block >= 0 && glyph->source() == origin.canonical
            ? sourceFiles_[origin.file].document.blocks[origin.block].source : glyph->source().trimmed());
      }
    }
    // Deleted glyphs no longer occur in glyphs. Remove their original source
    // blocks as well, while retaining surrounding comments and other glyphs.
    for (int block = 0; block < sourceFiles_[fileIndex].document.blocks.size(); ++block)
      if (!retainedBlocks.contains(block)) replacements.insert(block, {});
    const auto bytes = sourceFiles_[fileIndex].document.render(replacements, appended);
    GlyphSourceDocument document;
    if (!document.parse(bytes, &projectError)) return false;
    rendered.append(bytes); parsed.append(document);
  }
  // Stage all writes before replacing any file. Check for external edits even
  // for untouched files; a previous partially completed save may match rendered.
  std::vector<std::unique_ptr<QSaveFile>> pending;
  for (int i = 0; i < sourceFiles_.size(); ++i) {
    const auto path = QDir(m_currentDir).filePath(sourceFiles_[i].path);
    QFile current(path);
    if (!current.open(QIODevice::ReadOnly)) { projectError = path + ": " + current.errorString(); return false; }
    const auto onDisk = current.readAll();
    if (onDisk != sourceFiles_[i].document.original && onDisk != rendered[i]) {
      projectError = "Source changed on disk; reload before saving: " + sourceFiles_[i].path;
      return false;
    }
    if (onDisk == rendered[i]) { pending.push_back(nullptr); continue; }
    auto output = std::make_unique<QSaveFile>(path);
    if (!output->open(QIODevice::WriteOnly) || output->write(rendered[i]) != rendered[i].size()) {
      projectError = path + ": " + output->errorString(); return false;
    }
    pending.push_back(std::move(output));
  }
  for (std::size_t i = 0; i < pending.size(); ++i) {
    if (pending[i] && !pending[i]->commit()) {
      projectError = "Cannot save " + sourceFiles_[int(i)].path + ": " + pending[i]->errorString() +
          ". Earlier files may have been saved; retry Save after resolving the error.";
      return false;
    }
  }
  QHash<QString, Glyph*> currentGlyphs;
  for (auto* glyph : glyphs) currentGlyphs.insert(glyph->name(), glyph);
  glyphOrigins_.clear();
  for (int i = 0; i < sourceFiles_.size(); ++i) {
    sourceFiles_[i].document = parsed[i];
    for (int block = 0; block < parsed[i].blocks.size(); ++block) {
      auto* glyph = currentGlyphs.value(parsed[i].blocks[block].name);
      if (glyph) glyphOrigins_.insert(glyph, {i, block, glyph->source(), sourceFiles_[i].path});
    }
  }
  emit sourcesSaved();
  return true;
}

QStringList Font::glyphSourceFiles() const {
  QStringList result;
  for (const auto& file : sourceFiles_) result.append(file.path);
  return result;
}
QString Font::glyphSourceFile(const Glyph* glyph) const {
  return glyphOrigins_.value(glyph).target;
}
bool Font::createGlyphFile(const QString& relativePath) {
  auto files = glyphSourceFiles();
  files.append(relativePath);
  ProjectMetadata candidate = project;
  if (!candidate.setGlyphFiles(files, &projectError)) return false;
  const auto cleanPath = QDir::cleanPath(relativePath);
  const auto path = QDir(m_currentDir).filePath(cleanPath);
  if (!QDir().mkpath(QFileInfo(path).absolutePath())) { projectError = "Cannot create source directory"; return false; }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly)) { projectError = path + ": " + file.errorString(); return false; }
  file.close();
  const auto previous = project;
  project = candidate;
  if (!saveProjectMetadata()) { project = previous; QFile::remove(path); return false; }
  SourceFile source{cleanPath, {}};
  source.document.parse({}); sourceFiles_.append(source);
  return true;
}
void Font::orderGlyphsBySource() {
  const auto files = glyphSourceFiles();
  std::stable_sort(glyphs.begin(), glyphs.end(), [&](const Glyph* a, const Glyph* b) {
    const auto& x = glyphOrigins_[a]; const auto& y = glyphOrigins_[b];
    const int xf = files.indexOf(x.target), yf = files.indexOf(y.target);
    if (xf != yf) return xf < yf;
    const int xb = x.file == xf && x.block >= 0 ? x.block : INT_MAX;
    const int yb = y.file == yf && y.block >= 0 ? y.block : INT_MAX;
    return xb < yb;
  });
}
bool Font::moveGlyphs(const QVector<Glyph*>& selected, const QString& sourceFile) {
  if (!glyphSourceFiles().contains(sourceFile)) { projectError = "Unknown source file: " + sourceFile; return false; }
  for (auto* glyph : selected) if (!glyphOrigins_.contains(glyph)) { projectError = "Glyph is not part of this project"; return false; }
  for (auto* glyph : selected) {
    glyphs.removeOne(glyph); glyphs.append(glyph);
    glyphOrigins_[glyph].target = sourceFile;
  }
  orderGlyphsBySource();
  for (auto* glyph : selected) emit glyph->valueChanged("sourceFile");
  return true;
}
Glyph* Font::createGlyph(const QString& name, int unicode, const QString& sourceFile) {
  if (!QRegularExpression("^[A-Za-z][A-Za-z0-9_.]*$").match(name).hasMatch() || !glyphSourceFiles().contains(sourceFile) ||
      unicode < -1 || unicode > 0xffff || (unicode >= 0xd800 && unicode <= 0xdfff)) {
    projectError = "Choose a valid MetaPost glyph name, source file and BMP Unicode value (or leave Unicode empty)";
    return nullptr;
  }
  for (auto* glyph : glyphs) if (glyph->name() == name || (unicode >= 0 && glyph->unicode() == unicode)) {
    projectError = "Glyph name or Unicode value already exists"; return nullptr;
  }
  auto* glyph = new Glyph(QString("defchar(%1,%2,500,700,0);\n%%beginpaths\n%%endpaths\nenddefchar;\n").arg(name).arg(unicode), this);
  glyphs.append(glyph); glyphOrigins_.insert(glyph, {-1, -1, {}, sourceFile});
  orderGlyphsBySource(); synchronizeGlyphSources();
  return glyph;
}

bool Font::removeGlyph(Glyph* glyph) {
  if (!glyph || !glyphs.contains(glyph)) {
    projectError = "Glyph is not part of this project";
    return false;
  }
  glyphs.removeOne(glyph);
  glyphperName.remove(glyph->name());
  glyphOrigins_.remove(glyph);
  synchronizeGlyphSources();
  delete glyph;
  return true;
}

Font::~Font() {
}

QString Font::filePath() {
  return m_path;
}
std::string Font::filePathStd() const {
  return m_path.toStdString();
}
QString Font::fontName() {
  return m_fontName;
}
Glyph* Font::getGlyph(uint charcode) {
  for (QVector<Glyph*>::const_iterator it = glyphs.begin(); it != glyphs.end(); ++it) {
    Glyph* cur = *it;
    if (cur->charcode() == (int)charcode) {
      return cur;
    }
  }

  return NULL;
}

void Font::synchronizeGlyphSources() {
  m_mpFont.clearGlyphSources();
  for (auto* glyph : glyphs) {
    m_mpFont.registerGlyphSource(glyph->name().toStdString(),
                                 glyph->source().toStdString(),
                                 glyph->beginMacroName().toStdString(),
                                 glyph->unicode());
  }
}

MPFont& Font::mpFont() {
  synchronizeGlyphSources();
  return m_mpFont;
}

std::string Font::executeMetaPost(std::string command) {
  // Managed pictures belong to this render request, including requests from
  // specialized editors. Reset them before execution to avoid stale captures
  // and controlledImage accumulation across successive previews.
  if (!pictureNames.isEmpty()) {
    QString reset="picture vmfFinalSourceCapture; vmfFinalSourceCapture:=nullpicture;";
    for (const auto& name:pictureNames)
      reset+=QString("picture %1; %1:=nullpicture; picture %1.vmfSourceCapture; %1.vmfSourceCapture:=nullpicture;").arg(name);
    command.insert(0,reset.toStdString());
  }
  std::vector<std::string> names;
  names.reserve(pictureNames.size());
  for (const auto& name : pictureNames) names.push_back(name.toStdString());
  m_mpFont.setControlledPictureNames(std::move(names));
  auto result = m_mpFont.execute(command);
  synchronizeAnchorVisibility();
  return result;
}
std::vector<mp_edge_object*> Font::getEdges() const {
  return m_mpFont.edges();
};

mp_edge_object* Font::getEdge(int charCode) {
  return m_mpFont.edge(charCode);
}

MPGlyphInfo Font::getMPGlyphInfo(int charCode) {
  MPGlyphInfo info;

  auto mpInfo = m_mpFont.glyphInfo(charCode);
  info.currentPicture = mpInfo.currentPicture;
  for (const auto& [name, picture] : mpInfo.controlledPictures)
    info.controlledPictures.insert(QString::fromStdString(name), picture);
  for (const auto& [name, picture] : mpInfo.controlledPictureSources)
    info.controlledPictureSources.insert(QString::fromStdString(name),picture);

  return info;
}

bool Font::hasGlyph(std::string_view glyphName) const {
  return m_mpFont.hasGlyph(glyphName);
}
mp_graphic_object* Font::copyEdgeBody(mp_graphic_object* body) {
  return m_mpFont.copyBody(body);
/* Legacy implementation retained temporarily for reference.
#if 0
  mp_graphic_object* result = nullptr;

  auto copypath = [this](mp_gr_knot knot) {
    mp_gr_knot p, current, ret;

    ret = nullptr;

    if (knot == nullptr) return ret;

    ret = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data));  // new mp_gr_knot_data();

    ret->x_coord = knot->x_coord;
    ret->y_coord = knot->y_coord;
    ret->left_x = knot->left_x;
    ret->left_y = knot->left_y;
    ret->right_x = knot->right_x;
    ret->right_y = knot->right_y;
    ret->data.types.left_type = knot->data.types.left_type;
    ret->next = nullptr;

    current = ret;

    p = knot->next;
    while (p != knot) {
      mp_gr_knot tmp = (mp_gr_knot)mp_xmalloc(mp, 1, sizeof(struct mp_gr_knot_data));  // new mp_gr_knot_data();

      tmp->left_x = p->left_x;
      tmp->left_y = p->left_y;
      tmp->x_coord = p->x_coord;
      tmp->y_coord = p->y_coord;
      tmp->right_x = p->right_x;
      tmp->right_y = p->right_y;
      tmp->data.types.left_type = p->data.types.left_type;

      current->next = tmp;
      current = tmp;

      p = p->next;
    }

    current->next = ret;

    return ret;
  };

  mp_graphic_object* currObject = nullptr;

  if (body) {
    do {
      switch (body->type) {
        case mp_fill_code: {
          mp_fill_object* fillobject = (mp_fill_object*)body;
          mp_gr_knot newpath = copypath(fillobject->path_p);

          mp_fill_object* nextObject = (mp_fill_object*)mp_new_graphic_object(mp, mp_fill_code);
          nextObject->type = mp_fill_code;
          nextObject->path_p = newpath;
          nextObject->next = nullptr;
          nextObject->pre_script = fillobject->pre_script != nullptr
                                       ? xstrdup(fillobject->pre_script)
                                       : nullptr;
          nextObject->post_script = fillobject->post_script != nullptr
                                        ? xstrdup(fillobject->post_script)
                                        : nullptr;
          nextObject->pen_p = nullptr;
          nextObject->htap_p = nullptr;

          if (fillobject->color_model == mp_rgb_model) {
            nextObject->color_model = mp_rgb_model;
            nextObject->color = fillobject->color;
          }

          if (currObject == nullptr) {
            currObject = (mp_graphic_object*)nextObject;
            result = currObject;
          } else {
            currObject->next = (mp_graphic_object*)nextObject;
            currObject = currObject->next;
          }

          break;
        }
        case mp_stroked_code: {
          mp_stroked_object* fillobject = (mp_stroked_object*)body;
          mp_gr_knot newpath = copypath(fillobject->path_p);

          mp_stroked_object* nextObject = (mp_stroked_object*)mp_new_graphic_object(mp, mp_stroked_code);  // new mp_fill_object;
          nextObject->type = mp_stroked_code;
          nextObject->path_p = newpath;
          nextObject->next = nullptr;
          nextObject->pre_script = fillobject->pre_script != nullptr
                                       ? xstrdup(fillobject->pre_script)
                                       : nullptr;
          nextObject->post_script = fillobject->post_script != nullptr
                                        ? xstrdup(fillobject->post_script)
                                        : nullptr;
          nextObject->pen_p = nullptr;
          // nextObject->htap_p = nullptr;

          if (fillobject->color_model == mp_rgb_model) {
            nextObject->color_model = mp_rgb_model;
            nextObject->color = fillobject->color;
          }

          if (currObject == nullptr) {
            currObject = (mp_graphic_object*)nextObject;
            result = currObject;
          } else {
            currObject->next = (mp_graphic_object*)nextObject;
            currObject = currObject->next;
          }

          break;
        }
        default:
          break;
      }

    } while (body = body->next);
  }

  return result;
#endif */
}

QString Font::getLog() {
  return QString::fromStdString(m_mpFont.log()).trimmed();
}

/* Axes are owned and parsed by MPFont.
void Font::readAxes() {
  axes.clear();

  auto numAxes = getInternalNumericVariable("number_of_axes");

  for (int i = 0; i < numAxes; i++) {
    VarAxis axis;
    char* value;
    auto varname = std::format("axes {} name", i);
    auto found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.name = value;
    }
    varname = std::format("axes {} tag", i);
    found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.axisTag = hb_tag_from_string(value, 4);
    }

    varname = std::format("axes {} equivExpr", i);
    found = getMPStringVariable(mp, varname.c_str(), &value);
    if (found) {
      axis.equivExpr = value;
    }
    double dbValue = 0.0;

    varname = std::format("axes {} minValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.minValue = dbValue;
    }

    varname = std::format("axes {} defaultValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.defaultValue = dbValue;
    }

    varname = std::format("axes {} maxValue", i);
    found = getMPNumVariable(mp, varname.c_str(), &dbValue);
    if (found) {
      axis.maxValue = dbValue;
    }

    axes.append(axis);
  }
} */

bool Font::saveProjectMetadata() {
  return project.save(QFileInfo(m_path).absolutePath() + "/font-project.json", &projectError);
}
