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

#include "Lookup.h"
#include "Subtable.h"
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include "digitalkhatt/core/ByteBuffer.h"
#include "qiodevice.h"

Lookup::Lookup(OtLayout* layout) {
  this->layout = layout;
  flags = 0;
}

Lookup::~Lookup()
{
  for (auto subtable : subtables) {
    delete subtable;
  }
}
void Lookup::setGlyphSet(QVector<QString> list) {
  QSet<quint16> set;
  for (auto className : list) {
    markGlyphSet.append(className);
    const auto codes = layout->classtoUnicode(className.toStdString());
    for (const auto code : codes) set.insert(code);
  }

  markGlyphSetIndex = -1;
  if (set.size() > 0) {
    auto list = set.values();
    std::sort(list.begin(), list.end());
    markGlyphSetIndex = layout->addMarkSet(list);
    flags = flags | Flags::UseMarkFilteringSet;
  }
}
void Lookup::readJson(const QJsonObject& jsonsubtable) {
  QString type = jsonsubtable["type"].toString();

  QJsonArray flagsArray = jsonsubtable["flags"].toArray();
  flags = 0;
  if (flagsArray.size() == 4) {
    if (flagsArray[0].toBool()) {
      flags = flags | Flags::RightToLeft;
    }
    if (flagsArray[1].toBool()) {
      flags = flags | Flags::IgnoreBaseGlyphs;
    }
    if (flagsArray[2].toBool()) {
      flags = flags | Flags::IgnoreLigatures;
    }
    if (flagsArray[3].toBool()) {
      flags = flags | Flags::IgnoreMarks;
    }
  }

  QJsonArray markGlyphSetArray = jsonsubtable["markSet"].toArray();

  QSet<quint16> set;
  for (int index = 0; index < markGlyphSetArray.size(); ++index) {
    QString className = markGlyphSetArray[index].toString();
    markGlyphSet.append(className);
    const auto codes = layout->classtoUnicode(className.toStdString());
    for (const auto code : codes) set.insert(code);
  }

  markGlyphSetIndex = -1;
  if (set.size() > 0) {
    auto list = set.values();
    std::sort(list.begin(), list.end());
    markGlyphSetIndex = layout->addMarkSet(list);
    flags = flags | Flags::UseMarkFilteringSet;
  }

  if (type == "color") {
    SingleAdjustmentSubtable* newsubtable = new SingleAdjustmentSubtable(this, 3);
    this->type = Lookup::singleadjustment;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable["data"].toObject());
    subtables.append(newsubtable);

  }
  else if (type == "singleadj") {
    SingleAdjustmentSubtable* newsubtable = new SingleAdjustmentSubtable(this);
    this->type = Lookup::singleadjustment;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable["data"].toObject());
    subtables.append(newsubtable);

  }
  else if (type == "cursive") {
    CursiveSubtable* newsubtable = new CursiveSubtable(this);
    this->type = Lookup::cursive;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable);
    subtables.append(newsubtable);

  }
  else if (type == "mark2base") {
    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(this);
    this->type = Lookup::mark2base;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable);
    subtables.append(newsubtable);

  }
  else if (type == "mark2mark") {
    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(this);
    this->type = Lookup::mark2mark;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable);
    this->subtables.append(newsubtable);

  }
  else if (type == "single") {
    SingleSubtable* newsubtable = new SingleSubtable(this);
    this->type = Lookup::single;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable["data"].toObject());
    this->subtables.append(newsubtable);
  }
  else if (type == "multiple") {
    MultipleSubtable* newsubtable = new MultipleSubtable(this);
    this->type = Lookup::multiple;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable["data"].toObject());
    this->subtables.append(newsubtable);
  }
  else if (type == "ligature") {
    LigatureSubtable* newsubtable = new LigatureSubtable(this);
    this->type = Lookup::ligature;
    newsubtable->name = name.toStdString();
    newsubtable->readJson(jsonsubtable["data"].toObject());
    this->subtables.append(newsubtable);
  }
  else if (type == "chainingsub") {

    QJsonArray subtablesArray = jsonsubtable["subtables"].toArray();
    for (int index = 0; index < subtablesArray.size(); ++index) {

      QJsonObject ruleObject = subtablesArray[index].toObject();
      ChainingSubtable* newsubtable = new ChainingSubtable(this);
      this->type = Lookup::chainingsub;
      newsubtable->name = name.toStdString() + std::to_string(index);
      this->subtables.append(newsubtable);
      newsubtable->readJson(ruleObject);
    }

  }
  else if (type == "chainingpos") {

    QJsonArray subtablesArray = jsonsubtable["subtables"].toArray();
    for (int index = 0; index < subtablesArray.size(); ++index) {

      QJsonObject ruleObject = subtablesArray[index].toObject();
      ChainingSubtable* newsubtable = new ChainingSubtable(this);
      this->type = Lookup::chainingpos;
      newsubtable->name = name.toStdString() + std::to_string(index);
      this->subtables.append(newsubtable);
      newsubtable->readJson(ruleObject);

    }

  }

}
void Lookup::saveParameters(QJsonObject& json) const {
  for (auto subtable : subtables) {
    QJsonObject  subtableObject;
    subtable->saveParameters(subtableObject);
    if (!subtableObject.isEmpty()) {
      json[QString::fromStdString(subtable->name)] = subtableObject;
    }

  }
}
void Lookup::readParameters(const QJsonObject& json) {
  for (int index = 0; index < json.size(); ++index) {
    QString subtableName = json.keys()[index];
    for (auto subtable : subtables) {
      if (subtable->name == subtableName.toStdString()) {
        subtable->readParameters(json[subtableName].toObject());
        break;
      }
    }

  }

}
digitalkhatt::ByteBuffer Lookup::getSubtableDatas(bool extended) {
  digitalkhatt::ByteBuffer result;
  for (auto* subtable : subtables)
    result.append(subtable->getOptOpenTypeTable(extended));
  return result;
}
QVector<Subtable*> Lookup::getSubtables(bool extended) {

  if (extended) return subtables;

  QVector<Subtable*> subs;

  for (auto sub : subtables) {
    if (!sub->isExtended() || sub->isConvertible()) {
      subs.append(sub);
    }
  }

  return subs;

}
digitalkhatt::ByteBuffer Lookup::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer subtableData;
  const uint16_t subtableCount = subtables.size();
  root.writeU16(static_cast<quint16>(type));
  root.writeU16(flags);
  root.writeU16(subtableCount);
  uint16_t subtableOffset = 8 + 2 * subtableCount;
  for (auto* subtable : subtables) {
    const auto bytes = subtable->getOptOpenTypeTable(extended);
    root.writeU16(subtableOffset);
    subtableData.append(bytes);
    subtableOffset += bytes.size();
  }
  root.writeU16(markGlyphSetIndex);
  root.append(subtableData);
  return root;
};
digitalkhatt::ByteBuffer Lookup::getOpenTypeExtenionTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer subtableData;
  const uint16_t subtableCount = subtables.size();
  root.writeU16(isGsubLookup() ? static_cast<quint16>(extensiongsub)
                                  : static_cast<quint16>(extensiongpos));
  root.writeU16(flags);
  root.writeU16(subtableCount);
  uint16_t subtableOffset = 6 + 2 * subtableCount;
  if (markGlyphSetIndex != -1) subtableOffset += 2;
  for (auto* subtable : subtables) {
    const auto bytes = subtable->getOptOpenTypeTable(extended);
    root.writeU16(subtableOffset);
    subtableData.append(bytes);
    subtableOffset += bytes.size();
  }
  if (markGlyphSetIndex != -1) root.writeU16(markGlyphSetIndex);
  root.append(subtableData);
  return root;
};
