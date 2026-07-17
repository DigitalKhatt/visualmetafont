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
#include <set>
#include "GlazeJson.h"
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
void Lookup::setGlyphSet(std::vector<std::string> list) {
  std::set<std::uint16_t> set;
  for (const auto& className : list) {
    markGlyphSet.push_back(className);
    const auto codes = layout->classtoUnicode(className);
    for (const auto code : codes) set.insert(code);
  }

  markGlyphSetIndex = NoMarkGlyphSet;
  if (!set.empty()) {
    markGlyphSetIndex = layout->addMarkSet(QList<quint16>(set.begin(), set.end()));
    flags = flags | Flags::UseMarkFilteringSet;
  }
}
void Lookup::readJson(const ParameterJsonObject& jsonsubtable) {
  const auto type = jsonValueAs<std::string>(jsonsubtable, "type").value_or("");

  const auto flagsArray =
      jsonValueAs<std::vector<bool>>(jsonsubtable, "flags").value_or(std::vector<bool>{});
  flags = 0;
  if (flagsArray.size() == 4) {
    if (flagsArray[0]) {
      flags = flags | Flags::RightToLeft;
    }
    if (flagsArray[1]) {
      flags = flags | Flags::IgnoreBaseGlyphs;
    }
    if (flagsArray[2]) {
      flags = flags | Flags::IgnoreLigatures;
    }
    if (flagsArray[3]) {
      flags = flags | Flags::IgnoreMarks;
    }
  }

  const auto markGlyphSetArray =
      jsonValueAs<std::vector<std::string>>(jsonsubtable, "markSet")
          .value_or(std::vector<std::string>{});

  std::set<std::uint16_t> set;
  for (const auto& className : markGlyphSetArray) {
    markGlyphSet.push_back(className);
    const auto codes = layout->classtoUnicode(className);
    for (const auto code : codes) set.insert(code);
  }

  markGlyphSetIndex = NoMarkGlyphSet;
  if (!set.empty()) {
    markGlyphSetIndex = layout->addMarkSet(QList<quint16>(set.begin(), set.end()));
    flags = flags | Flags::UseMarkFilteringSet;
  }

  const auto* dataValue = findJsonValue(jsonsubtable, "data");
  const ParameterJsonObject* data =
      dataValue && dataValue->is_object() ? &dataValue->get_object() : nullptr;

  if (type == "color" && data) {
    SingleAdjustmentSubtable* newsubtable = new SingleAdjustmentSubtable(this, 3);
    this->type = Lookup::singleadjustment;
    newsubtable->name = name;
    newsubtable->readJson(*data);
    subtables.push_back(newsubtable);

  }
  else if (type == "singleadj" && data) {
    SingleAdjustmentSubtable* newsubtable = new SingleAdjustmentSubtable(this);
    this->type = Lookup::singleadjustment;
    newsubtable->name = name;
    newsubtable->readJson(*data);
    subtables.push_back(newsubtable);

  }
  else if (type == "cursive") {
    CursiveSubtable* newsubtable = new CursiveSubtable(this);
    this->type = Lookup::cursive;
    newsubtable->name = name;
    newsubtable->readJson(jsonsubtable);
    subtables.push_back(newsubtable);

  }
  else if (type == "mark2base") {
    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(this);
    this->type = Lookup::mark2base;
    newsubtable->name = name;
    newsubtable->readJson(jsonsubtable);
    subtables.push_back(newsubtable);

  }
  else if (type == "mark2mark") {
    MarkBaseSubtable* newsubtable = new MarkBaseSubtable(this);
    this->type = Lookup::mark2mark;
    newsubtable->name = name;
    newsubtable->readJson(jsonsubtable);
    this->subtables.push_back(newsubtable);

  }
  else if (type == "single" && data) {
    SingleSubtable* newsubtable = new SingleSubtable(this);
    this->type = Lookup::single;
    newsubtable->name = name;
    newsubtable->readJson(*data);
    this->subtables.push_back(newsubtable);
  }
  else if (type == "multiple" && data) {
    MultipleSubtable* newsubtable = new MultipleSubtable(this);
    this->type = Lookup::multiple;
    newsubtable->name = name;
    newsubtable->readJson(*data);
    this->subtables.push_back(newsubtable);
  }
  else if (type == "ligature" && data) {
    LigatureSubtable* newsubtable = new LigatureSubtable(this);
    this->type = Lookup::ligature;
    newsubtable->name = name;
    newsubtable->readJson(*data);
    this->subtables.push_back(newsubtable);
  }
  else if (type == "chainingsub") {

    const auto* subtablesValue = findJsonValue(jsonsubtable, "subtables");
    if (!subtablesValue || !subtablesValue->is_array()) return;
    const auto& subtablesArray = subtablesValue->get_array();
    for (std::size_t index = 0; index < subtablesArray.size(); ++index) {
      if (!subtablesArray[index].is_object()) continue;
      ChainingSubtable* newsubtable = new ChainingSubtable(this);
      this->type = Lookup::chainingsub;
      newsubtable->name = name + std::to_string(index);
      this->subtables.push_back(newsubtable);
      newsubtable->readJson(subtablesArray[index].get_object());
    }

  }
  else if (type == "chainingpos") {

    const auto* subtablesValue = findJsonValue(jsonsubtable, "subtables");
    if (!subtablesValue || !subtablesValue->is_array()) return;
    const auto& subtablesArray = subtablesValue->get_array();
    for (std::size_t index = 0; index < subtablesArray.size(); ++index) {
      if (!subtablesArray[index].is_object()) continue;
      ChainingSubtable* newsubtable = new ChainingSubtable(this);
      this->type = Lookup::chainingpos;
      newsubtable->name = name + std::to_string(index);
      this->subtables.push_back(newsubtable);
      newsubtable->readJson(subtablesArray[index].get_object());

    }

  }

}
void Lookup::saveParameters(ParameterJsonObject& json) const {
  for (auto subtable : subtables) {
    ParameterJsonObject subtableObject;
    subtable->saveParameters(subtableObject);
    if (!subtableObject.empty()) {
      json[subtable->name] = std::move(subtableObject);
    }

  }
}
void Lookup::readParameters(const ParameterJsonObject& json) {
  for (const auto& [subtableName, subtableValue] : json) {
    for (auto subtable : subtables) {
      if (subtable->name == subtableName) {
        if (!subtableValue.is_object()) break;
        subtable->readParameters(subtableValue.get_object());
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
std::vector<Subtable*> Lookup::getSubtables(bool extended) {

  if (extended) return subtables;

  std::vector<Subtable*> subs;

  for (auto sub : subtables) {
    if (!sub->isExtended() || sub->isConvertible()) {
      subs.push_back(sub);
    }
  }

  return subs;

}
digitalkhatt::ByteBuffer Lookup::getOpenTypeTable(bool extended) {
  digitalkhatt::ByteBuffer root;
  digitalkhatt::ByteBuffer subtableData;
  const uint16_t subtableCount = subtables.size();
  root.writeU16(static_cast<std::uint16_t>(type));
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
  root.writeU16(isGsubLookup() ? static_cast<std::uint16_t>(extensiongsub)
                               : static_cast<std::uint16_t>(extensiongpos));
  root.writeU16(flags);
  root.writeU16(subtableCount);
  uint16_t subtableOffset = 6 + 2 * subtableCount;
  if (markGlyphSetIndex != NoMarkGlyphSet) subtableOffset += 2;
  for (auto* subtable : subtables) {
    const auto bytes = subtable->getOptOpenTypeTable(extended);
    root.writeU16(subtableOffset);
    subtableData.append(bytes);
    subtableOffset += bytes.size();
  }
  if (markGlyphSetIndex != NoMarkGlyphSet) root.writeU16(markGlyphSetIndex);
  root.append(subtableData);
  return root;
};
