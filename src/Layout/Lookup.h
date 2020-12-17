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

#ifndef H_LOOKUP
#define H_LOOKUP

#include "qstring.h"
#include "qvector.h"

struct OtLayout;
struct Subtable;
class QJsonObject;



struct Lookup {
public:
  enum None {
    none
  };
  enum SubType {
    single = 1,
    multiple = 2,
    alternate = 3,
    ligature = 4,
    contextsub = 5,
    chainingsub = 6,
    extensiongsub = 7,
    reversesub = 8,
    fsmgsub = 20,
  };

  enum PosType {
    singleadjustment = 1,
    pairadjustment = 2,
    cursive = 3,
    mark2base = 4,
    mark2ligature = 5,
    mark2mark = 6,
    contextpos = 7,
    chainingpos = 8,
    extensiongpos = 9,
    fsmgpos = 20
  };

  class Type {
  public:

    Type() : type{ Table::NONE } { u.none = 0; };

    Type(None none) : type{ Table::NONE } { u.none = 0; };

    Type(SubType subType) {
      this->type = Table::GSUB;
      this->u.subType = subType;
    }
    
    Type(PosType posType) {
      this->type = Table::GPOS;
      this->u.posType = posType;
    }

    bool operator==(Type a) const {
      return (a.type == Table::NONE && type == Table::NONE)
        || (a.type == Table::GSUB && type == Table::GSUB && a.u.subType == u.subType)
        || (a.type == Table::GPOS && type == Table::GPOS && a.u.posType == u.posType);
    }
    bool operator!=(Type a) const { return !(*this == a); }

    bool isGsub() {
      return type == Table::GSUB;
    }

    bool isGpos() {
      return type == Table::GPOS;
    }

    explicit operator quint16() const {
      switch (type) {
      case Table::NONE:
        return 0;
      case Table::GSUB:
        return u.subType;
      case Table::GPOS:
        return u.posType;
      }
    }


    explicit operator bool() = delete;

  private:

    enum Table
    {
      NONE,
      GSUB,
      GPOS
    };

    union {
      int none;
      SubType subType;
      PosType posType;
    } u;

    Table type;

  };

  /*
  enum Type {
    none = 0,
    //GSUB
    single = 1,
    multiple = 2,
    alternate = 3,
    ligature = 4,
    contextsub = 5,
    chainingsub = 6,
    extensiongsub = 7,
    reversesub = 8,
    regexpsub = 9,
    //GPOS
    singleadjustment = 11,
    pairadjustment = 12,
    cursive = 13,
    mark2base = 14,
    mark2ligature = 15,
    mark2mark = 16,
    contextpos = 17,
    chainingpos = 18,
    extensiongpos = 19
  };*/

  enum Flags {
    RightToLeft = 0x0001u,
    IgnoreBaseGlyphs = 0x0002u,
    IgnoreLigatures = 0x0004u,
    IgnoreMarks = 0x0008u,
    IgnoreFlags = 0x000Eu,
    UseMarkFilteringSet = 0x0010u,
    Reserved = 0x00E0u,
    MarkAttachmentType = 0xFF00u
  };

  Lookup(OtLayout* layout);
  ~Lookup();

  virtual QByteArray getOpenTypeTable(bool extended);
  virtual QByteArray getOpenTypeExtenionTable(bool extended);
  virtual QByteArray getSubtableDatas(bool extended);
  void readJson(const QJsonObject& json);
  bool isGsubLookup() {
    //return type < 9;
    return type.isGsub();
  }
  void saveParameters(QJsonObject& json) const;
  void readParameters(const QJsonObject& json);

  QString name;
  QString feature;
  OtLayout* layout;
  quint16 flags;
  QVector<Subtable*> subtables;
  QVector<QString> markGlyphSet;
  quint16 markGlyphSetIndex;
  void setGlyphSet(QVector<QString>);

  Type type;

  QVector<Subtable*> getSubtables(bool extended);

};

#endif // H_LOOKUP
