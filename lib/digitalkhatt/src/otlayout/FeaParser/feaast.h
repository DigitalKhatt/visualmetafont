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

#ifndef H_FEAAST
#define H_FEAAST

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <map>
#include <ostream>
#include <set>
#include <stack>
#include <unordered_set>
#include <stdexcept>
#include <variant>
#include <vector>

#include "GlyphVis.h"
#include "Lookup.h"
#include "OtLayout.h"
#include "Subtable.h"
#include "just.h"
#include "statement.h"

namespace feayy {

enum class GlyphType {
  CID,
  Name
};

enum class AnchorType {
  Null,
  Name,
  FormatA,
  Function
};

enum class InlineType {
  None,
  SinglePos,
  CursivePos
};

enum class ClassComponentType {
  Glyph,
  ClassName,
  RegExp
};

using ValueRecordExtended = std::variant<ValueRecord, std::string>;

class ClassComponent {
  ClassComponentType _componenttype;

 public:
  explicit ClassComponent(ClassComponentType type) : _componenttype{type} {};

  ClassComponentType componentType() {
    return _componenttype;
  }

  virtual ~ClassComponent() {
  }

  virtual std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) = 0;

  virtual ClassComponent* clone() = 0;
};

class Glyph : public ClassComponent {
  GlyphType _glyphtype;

 public:
  explicit Glyph(GlyphType type) : ClassComponent{ClassComponentType::Glyph}, _glyphtype{type} {};

  GlyphType glyphType() {
    return _glyphtype;
  }

  virtual std::uint16_t getCode(OtLayout* otlayout) = 0;

  virtual Glyph* clone() = 0;
};

class Anchor {
  AnchorType _anchortype;

 public:
  explicit Anchor(AnchorType type) : _anchortype{type} {};

  AnchorType anchortype() {
    return _anchortype;
  }
};

class AnchorNull : public Anchor {
 public:
  explicit AnchorNull() : Anchor(AnchorType::Null) {}

  ~AnchorNull() {
  }
};

class AnchorName : public Anchor {
 public:
  std::string name;

  explicit AnchorName(std::string _name) : Anchor(AnchorType::Name), name{_name} {}

  operator const std::string&() const { return name; }

  ~AnchorName() {
    // delete name;
  }
};

class AnchorFunction : public Anchor {
 public:
  std::string name;

  explicit AnchorFunction(std::string _name) : Anchor(AnchorType::Function), name{_name} {}

  operator const std::string&() const { return name; }

  ~AnchorFunction() {
    // delete name;
  }
};

class AnchorFormatA : public Anchor {
 public:
  int x;
  int y;

  explicit AnchorFormatA(int x, int y) : Anchor{AnchorType::FormatA}, x{x}, y{y} {}

  ~AnchorFormatA() {
  }
};

class GlyphName : public Glyph {
  std::string name;

 public:
  explicit GlyphName(std::string _name) : Glyph(GlyphType::Name), name{_name} {}

  operator const std::string&() const { return name; }

  GlyphName* clone() override {
    return new GlyphName(name);
  }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) override;

  std::uint16_t getCode(OtLayout* otlayout) override;
};

class ClassName : public ClassComponent {
  const std::string name;

 public:
  explicit ClassName(std::string _name) : ClassComponent(ClassComponentType::ClassName), name{_name} {}

  operator const std::string&() const { return name; }

  ClassName* clone() override {
    return new ClassName(name);
  }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) override;
};

class RegExpClass : public ClassComponent {
  std::string _regexpr;

 public:
  explicit RegExpClass(std::string _reg) : ClassComponent(ClassComponentType::RegExp), _regexpr{_reg} {}

  operator const std::string&() const { return _regexpr; }

  RegExpClass* clone() override {
    return new RegExpClass(_regexpr);
  }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) override;
};

class GlyphCID : public Glyph {
 public:
  explicit GlyphCID(int _cid) : Glyph(GlyphType::CID), cid{_cid} {}

  operator int() const { return cid; }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) override;

  GlyphCID* clone() override {
    return new GlyphCID(cid);
  }

  std::uint16_t getCode(OtLayout* otlayout) override;

 private:
  int cid;
};

class GlyphWithParameters : public Glyph {
 public:
  explicit GlyphWithParameters(Glyph* glyph, GlyphParameters parameters)
      : Glyph(glyph->glyphType()), glyph{glyph}, parameters{parameters} {}

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) override {
    auto glyphCode = glyph->getCode(otlayout);
    auto alternate = otlayout->getAlternate(glyphCode, parameters, true, true);
    return {static_cast<std::uint16_t>(alternate->charcode)};
  }

  std::uint16_t getCode(OtLayout* otlayout) override {
    return glyph->getCode(otlayout);
  }
  GlyphWithParameters* clone() override {
    return new GlyphWithParameters(glyph->clone(), parameters);
  }

  ~GlyphWithParameters() override {
    delete glyph;
  }

 private:
  Glyph* glyph;
  GlyphParameters parameters;
};

class GlyphClass {
  std::vector<ClassComponent*> _components;

 public:
  explicit GlyphClass(const std::vector<ClassComponent*>& components) : _components{components} {
  }

  explicit GlyphClass(std::vector<ClassComponent*>&& components) : _components{std::move(components)} {
  }

  explicit GlyphClass(ClassName* component) {
    _components = std::vector<ClassComponent*>{component};
  }

  explicit GlyphClass(RegExpClass* component) {
    _components = std::vector<ClassComponent*>{component};
  }

  virtual ~GlyphClass() {
    for (auto component : _components) {
      delete component;
    }
  }

  GlyphClass* clone() {
    std::vector<ClassComponent*> componentsclone;
    for (auto com : _components) {
      componentsclone.push_back(com->clone());
    }
    return new GlyphClass(std::move(componentsclone));
  }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) {
    std::unordered_set<std::uint16_t> unicodes;
    for (auto comp : _components) {
      auto codes = comp->getCodes(otlayout);
      unicodes.insert(codes.begin(), codes.end());
      /*if (auto glyph = dynamic_cast<Glyph*>(comp)) {
        for (auto code : codes) {
          auto set = otlayout->getSubsts(code);
          unicodes.unite(set);
        }
      }*/
    }

    return unicodes;
  }
};

class GlyphSet {
 public:
  struct Bad_entry {};  // used for exceptions
  explicit GlyphSet(GlyphClass* _glyphClass) : _glyphClass(_glyphClass), type{Tag::glyphclass} {}
  explicit GlyphSet(Glyph* _glyph) : _glyph(_glyph), type{Tag::glyph} {}
  explicit GlyphSet() : _glyph(nullptr), type{Tag::empty} {}

  GlyphClass* glyphClass() const {
    if (type != Tag::glyphclass) throw Bad_entry{};
    return _glyphClass;
  }

  Glyph* glyph() const {
    if (type != Tag::glyph) throw Bad_entry{};
    return _glyph;
  }

  ~GlyphSet() {
    if (type == Tag::glyphclass) {
      delete _glyphClass;
    } else if (type == Tag::glyph) {
      delete _glyph;
    }
  }

  GlyphSet* clone() {
    if (type == Tag::glyphclass) {
      return new GlyphSet(_glyphClass->clone());
    } else if (type == Tag::glyph) {
      return new GlyphSet(_glyph->clone());
    }
    return new GlyphSet();
  }

  std::unordered_set<std::uint16_t> getCodes(OtLayout* otlayout) {
    if (type == Tag::glyphclass) {
      return _glyphClass->getCodes(otlayout);
    } else if (type == Tag::glyph) {
      return _glyph->getCodes(otlayout);
    } else {
      return {};
    }
  }

  const std::unordered_set<std::uint16_t>& getCachedCodes(OtLayout* otlayout) {
    if (!isCached) {
      cached = getCodes(otlayout);
      isCached = true;
    }

    return cached;
  }

  std::vector<std::uint16_t> getSortedCodes(OtLayout* otlayout) {
    const auto codes = getCodes(otlayout);
    std::vector<std::uint16_t> sorted(codes.begin(), codes.end());
    std::sort(sorted.begin(), sorted.end());
    return sorted;
  }

  bool isEmpty() {
    return type == Tag::empty;
  }

 private:
  enum class Tag { glyphclass,
                   glyph,
                   empty };

  union {
    GlyphClass* _glyphClass;
    Glyph* _glyph;
  };
  Tag type;

  bool isCached = false;
  std::unordered_set<std::uint16_t> cached;
};

class Mark2BaseClass {
 public:
  GlyphSet* glyphset;
  Anchor* baseAnchor;
  Anchor* markAnchor;
  std::string className;

  explicit Mark2BaseClass(GlyphSet* glyphset, Anchor* baseAnchor, Anchor* markAnchor, std::string className)
      : glyphset{glyphset}, baseAnchor{baseAnchor}, markAnchor{markAnchor}, className{className} {}

  ~Mark2BaseClass() {
    delete glyphset;
    delete baseAnchor;
    delete markAnchor;
    // delete className;
  }
};

class GlyphSetRegExp {
 public:
  virtual ~GlyphSetRegExp() {
  }

  virtual std::vector<std::vector<GlyphSet*>> getSequences() = 0;
};

class GlyphSetRegExpSingle : public GlyphSetRegExp {
 public:
  GlyphSet* element;

  explicit GlyphSetRegExpSingle(GlyphSet* element) : element{element} {}

  virtual ~GlyphSetRegExpSingle() {
    delete element;
  }

  std::vector<std::vector<GlyphSet*>> getSequences() override {
    if (element->isEmpty()) {
      return {{}};
    } else {
      return {{element}};
    }
  }
};

class GlyphSetRegExpGlyphSeq : public GlyphSetRegExp {
 public:
  std::vector<GlyphSet*> seq;

  explicit GlyphSetRegExpGlyphSeq(std::vector<Glyph*>* glyphSeq) {
    for (auto glyph : *glyphSeq) {
      this->seq.push_back(new GlyphSet(glyph));
    }
    delete glyphSeq;
  }

  virtual ~GlyphSetRegExpGlyphSeq() {
    for (auto glyph : seq) {
      delete glyph;
    }
  }

  std::vector<std::vector<GlyphSet*>> getSequences() override {
    if (seq.empty()) {
      return {{}};
    } else {
      return {seq};
    }
  }
};

class GlyphSetRegExpSeq : public GlyphSetRegExp {
 public:
  GlyphSetRegExp* left;
  GlyphSetRegExp* right;

  explicit GlyphSetRegExpSeq(GlyphSetRegExp* left, GlyphSetRegExp* right) : left{left}, right{right} {}
  explicit GlyphSetRegExpSeq(GlyphSetRegExp* left, GlyphSet* right) : left{left}, right{new GlyphSetRegExpSingle(right)} {}
  explicit GlyphSetRegExpSeq(GlyphSet* left, GlyphSetRegExp* right) : left{new GlyphSetRegExpSingle(left)}, right{right} {}
  explicit GlyphSetRegExpSeq(GlyphSet* left, GlyphSet* right) : left{new GlyphSetRegExpSingle(left)}, right{new GlyphSetRegExpSingle(right)} {}

  virtual ~GlyphSetRegExpSeq() {
    delete left;
    delete right;
  }

  std::vector<std::vector<GlyphSet*>> getSequences() override {
    auto lefseqs = left->getSequences();
    auto rightseqs = right->getSequences();
    std::vector<std::vector<GlyphSet*>> ret;
    for (auto leftseq : lefseqs) {
      for (auto rightseq : rightseqs) {
        std::vector<GlyphSet*> newseq{leftseq};
        newseq.insert(newseq.end(), rightseq.begin(), rightseq.end());
        ret.push_back(std::move(newseq));
      }
    }

    return ret;
  }
};

class GlyphSetRegExpRep : public GlyphSetRegExp {
 public:
  GlyphSetRegExp* expr;
  int minRep;
  int maxRep;

  explicit GlyphSetRegExpRep(GlyphSetRegExp* expr, int min, int max) : expr{expr}, minRep{min}, maxRep{max} {
    if (minRep <= 0) minRep = 0;
  }
  explicit GlyphSetRegExpRep(GlyphSetRegExp* expr, int rep) : GlyphSetRegExpRep{expr, rep, rep} {}

  virtual ~GlyphSetRegExpRep() {
    delete expr;
  }

  std::vector<std::vector<GlyphSet*>> getSequences() override {
    std::vector<std::vector<GlyphSet*>> minSeqs;

    auto rightseqs = expr->getSequences();

    if (minRep == 0) {
      minSeqs.push_back({});
    } else {
      for (int i = 0; i < minRep; i++) {
        auto lefseqs = minSeqs;
        for (auto leftseq : lefseqs) {
          for (auto rightseq : rightseqs) {
            std::vector<GlyphSet*> newseq{leftseq};
            newseq.insert(newseq.end(), rightseq.begin(), rightseq.end());
            minSeqs.push_back(std::move(newseq));
          }
        }
      }
    }

    std::vector<std::vector<GlyphSet*>> ret = minSeqs;
    std::vector<std::vector<GlyphSet*>> latSeqs = minSeqs;

    for (int i = minRep; i < maxRep; i++) {
      auto lefseqs = latSeqs;
      latSeqs = {};
      for (auto leftseq : lefseqs) {
        for (auto rightseq : rightseqs) {
          std::vector<GlyphSet*> newseq{leftseq};
          newseq.insert(newseq.end(), rightseq.begin(), rightseq.end());
          latSeqs.push_back(std::move(newseq));
        }
      }
      ret.insert(ret.end(), latSeqs.begin(), latSeqs.end());
    }

    std::sort(ret.begin(), ret.end(),
              [](const std::vector<GlyphSet*>& a, const std::vector<GlyphSet*>& b) {
                return a.size() > b.size();
              });

    return ret;
  }
};

class GlyphSetRegExpOr : public GlyphSetRegExp {
 public:
  GlyphSetRegExp* left;
  GlyphSetRegExp* right;

  explicit GlyphSetRegExpOr(GlyphSetRegExp* left, GlyphSetRegExp* right) : left{left}, right{right} {}

  virtual ~GlyphSetRegExpOr() {
    delete left;
    delete right;
  }

  std::vector<std::vector<GlyphSet*>> getSequences() override {
    std::vector<std::vector<GlyphSet*>> ret;

    auto lefseqs = left->getSequences();
    auto rightseqs = right->getSequences();

    for (auto leftseq : lefseqs) {
      ret.push_back(leftseq);
    }

    for (auto rightseq : rightseqs) {
      ret.push_back(rightseq);
    }

    return ret;
  }
};

class SingleAdjustmentRule : public LookupStatement {
 public:
  GlyphSet* glyphset;
  ValueRecord valueRecord;
  bool color;

  explicit SingleAdjustmentRule(GlyphSet* glyphset, ValueRecord valueRecord, bool color = false)
      : glyphset{glyphset}, valueRecord{valueRecord}, color{color} {}

  void accept(Visitor&) override;
};

class PairAdjustmentRule : public LookupStatement {
 public:
  GlyphSet* glyphSet1;
  GlyphSet* glyphSet2;
  ValueRecordExtended valueRecord1;
  ValueRecordExtended valueRecord2;

  explicit PairAdjustmentRule(GlyphSet* glyphSet1, GlyphSet* glyphSet2, ValueRecordExtended valueRecord1, ValueRecordExtended valueRecord2)
      : glyphSet1{glyphSet1}, glyphSet2{glyphSet2}, valueRecord1{valueRecord1}, valueRecord2{valueRecord2} {}

  void accept(Visitor&) override;
};

class CursiveRule : public LookupStatement {
 public:
  GlyphSet* glyphset;
  Anchor* entryAnchor;
  Anchor* exitAnchor;

  explicit CursiveRule(GlyphSet* glyphset, Anchor* entryAnchor, Anchor* exitAnchor)
      : glyphset{glyphset}, entryAnchor{entryAnchor}, exitAnchor{exitAnchor} {}

  void accept(Visitor&) override;

  ~CursiveRule() {
    delete glyphset;
    delete entryAnchor;
    delete exitAnchor;
  }
};

class Mark2BaseRule : public LookupStatement {
 public:
  GlyphSet* baseGlyphSet;
  std::vector<Mark2BaseClass*>* mark2baseclasses;
  Lookup::Type type;

  explicit Mark2BaseRule(GlyphSet* baseGlyphSet, std::vector<Mark2BaseClass*>* mark2baseclasses, Lookup::Type type)
      : baseGlyphSet{baseGlyphSet}, mark2baseclasses{mark2baseclasses}, type{type} {}

  void accept(Visitor&) override;

  virtual ~Mark2BaseRule() {
    delete baseGlyphSet;
    for (auto mark2baseclass : *mark2baseclasses) {
      delete mark2baseclass;
    }
    delete mark2baseclasses;
  }
};

class MarkedGlyphSetRegExp {
 public:
  GlyphSetRegExp* regexp = nullptr;
  std::vector<std::string> lookupNames;
  InlineType inlineType = InlineType::None;
  LookupStatement* stmt = nullptr;

  virtual void accept(Visitor&);

  explicit MarkedGlyphSetRegExp(GlyphSet* glyphset, std::vector<std::string> lookupNames) : regexp{new GlyphSetRegExpSingle{glyphset}}, lookupNames{lookupNames} {}
  explicit MarkedGlyphSetRegExp(GlyphSet* glyphset, SingleAdjustmentRule* rule) : regexp{new GlyphSetRegExpSingle{glyphset}}, inlineType{InlineType::SinglePos}, stmt{rule} {};
  explicit MarkedGlyphSetRegExp(GlyphSet* glyphset, CursiveRule* rule) : regexp{new GlyphSetRegExpSingle{glyphset}}, inlineType{InlineType::CursivePos}, stmt{rule} {};
  explicit MarkedGlyphSetRegExp(GlyphSet* glyphset, ValueRecord valuerecord) : MarkedGlyphSetRegExp{glyphset, new SingleAdjustmentRule(glyphset, valuerecord, false)} {};

  explicit MarkedGlyphSetRegExp(GlyphSetRegExp* regexp, std::vector<std::string> lookupNames)
      : regexp{regexp}, lookupNames{lookupNames} {}

  ~MarkedGlyphSetRegExp() {
    delete regexp;
    // delete lookupName;
    // delete stmt;
  }
};

class ClassDefinition : public LookupStatement {
 public:
  std::string name;
  GlyphClass* components;

  explicit ClassDefinition(std::string name, GlyphClass* components)
      : name{std::move(name)}, components{components} {}

  void accept(Visitor&) override;

  ~ClassDefinition() {
    delete components;
  }
};

class FeatureReference : public LookupStatement {
 public:
  std::string featureName;

  explicit FeatureReference(std::string name)
      : featureName{std::move(name)} {}

  void accept(Visitor&) override;
};

class LookupReference : public LookupStatement {
 public:
  std::string lookupName;

  explicit LookupReference(std::string name)
      : lookupName{std::move(name)} {}

  void accept(Visitor&) override;
};

class SingleSubstituionRule : public LookupStatement {
 public:
  enum class FirstType {
    GLYPH,
    GLYPHSET
  };

  union {
    Glyph* firstglyph;
    GlyphSet* firstGlyphSet;
  };

  FirstType firstType = FirstType::GLYPH;

  Glyph* secondglyph;
  int format;
  GlyphExpansion expansion;
  StartEndLig startEndLig = StartEndLig::StartEnd;

  explicit SingleSubstituionRule(Glyph* firstglyph, Glyph* secondglyph, int format)
      : SingleSubstituionRule{firstglyph, secondglyph, format, {}, StartEndLig::StartEnd} {}

  explicit SingleSubstituionRule(Glyph* firstglyph, Glyph* secondglyph, int format, GlyphExpansion expansion, StartEndLig startEndLig)
      : firstglyph{firstglyph}, secondglyph{secondglyph}, format{format}, expansion{expansion}, startEndLig{startEndLig} {}

  explicit SingleSubstituionRule(Glyph* firstglyph, double lefttatweel, double righttatweel)
      : firstglyph{firstglyph}, secondglyph{firstglyph}, format{11}, expansion{lefttatweel, lefttatweel, righttatweel, righttatweel}, startEndLig{StartEndLig::StartEnd} {}

  explicit SingleSubstituionRule(Glyph* firstglyph, Glyph* secondglyph, double lefttatweel, double righttatweel)
      : firstglyph{firstglyph}, secondglyph{secondglyph}, format{11}, expansion{lefttatweel, lefttatweel, righttatweel, righttatweel}, startEndLig{StartEndLig::StartEnd} {}

  explicit SingleSubstituionRule(GlyphSet* firstGlyphSet, double lefttatweel, double righttatweel)
      : firstGlyphSet{firstGlyphSet}, firstType{FirstType::GLYPHSET}, secondglyph{nullptr}, format{11}, expansion{lefttatweel, lefttatweel, righttatweel, righttatweel}, startEndLig{StartEndLig::StartEnd} {}

  explicit SingleSubstituionRule(GlyphSet* firstGlyphSet, int format, GlyphExpansion expansion, StartEndLig startEndLig)
      : firstGlyphSet{firstGlyphSet}, firstType{FirstType::GLYPHSET}, secondglyph{nullptr}, format{format}, expansion{expansion}, startEndLig{startEndLig} {}

  void accept(Visitor&) override;

  ~SingleSubstituionRule() {
    bool firtequalsecond = firstglyph == secondglyph;

    if (firstType == FirstType::GLYPH) {
      delete firstglyph;
    } else {
      delete firstGlyphSet;
    }

    if (!firtequalsecond) {
      delete secondglyph;
    }
  }
};

class MultipleSubstitutionRule : public LookupStatement {
 public:
  Glyph* glyph;
  std::vector<Glyph*>* sequence;

  int format;

  explicit MultipleSubstitutionRule(Glyph* glyph, std::vector<Glyph*>* sequence)
      : glyph{glyph}, sequence{sequence}, format{1} {}

  ~MultipleSubstitutionRule() {
    for (auto glyph : *sequence) {
      delete glyph;
    }
    delete glyph;
    delete sequence;
  }

  void accept(Visitor&) override;
};

class LigatureSubstitutionRule : public LookupStatement {
 public:
  std::vector<Glyph*>* sequence;
  Glyph* ligature;
  int format;

  explicit LigatureSubstitutionRule(std::vector<Glyph*>* sequence, Glyph* ligature)
      : sequence{sequence}, ligature{ligature}, format{1} {}

  ~LigatureSubstitutionRule() {
    for (auto glyph : *sequence) {
      delete glyph;
    }
    delete ligature;
    delete sequence;
  }

  void accept(Visitor&) override;
};

class ChainingContextualRule : public LookupStatement {
 public:
  std::vector<MarkedGlyphSetRegExp*>* input;
  GlyphSetRegExp* backtrack;
  GlyphSetRegExp* lookahead;
  Lookup::Type type;

 public:
  explicit ChainingContextualRule(GlyphSetRegExp* backtrack, std::vector<MarkedGlyphSetRegExp*>* input, GlyphSetRegExp* lookahead)
      : input{input}, backtrack{backtrack}, lookahead{lookahead}, type{Lookup::none} {}

  ~ChainingContextualRule() {
    for (auto glypset : *input) {
      delete glypset;
    }
    delete input;
    delete backtrack;
    delete lookahead;
  }

  void setType(bool isgsub) {
    if (isgsub)
      type = Lookup::chainingsub;
    else
      type = Lookup::chainingpos;
  }

  void accept(Visitor&) override;
};

class FSMRule : public LookupStatement {
};

class LookupFlag : public LookupStatement {
 public:
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
  explicit LookupFlag(short flag) : markFilteringSet{nullptr}, flag{flag} {
  }

  explicit LookupFlag() : markFilteringSet{nullptr}, flag(0) {
  }

  explicit LookupFlag(GlyphSet* set) : markFilteringSet(set), flag(UseMarkFilteringSet) {
  }

  void set_Flag(Flags pflag) {
    flag |= pflag;
  }

  void set_markFilteringSet(GlyphSet* pmarkFilteringSet) {
    if (markFilteringSet != nullptr) {
      delete markFilteringSet;
    }

    flag |= Flags::UseMarkFilteringSet;

    markFilteringSet = pmarkFilteringSet;
  }

  LookupFlag operator|(const LookupFlag& b) {
    LookupFlag obj{flag};
    obj.markFilteringSet = this->markFilteringSet;
    obj.flag = obj.flag | b.flag;

    if (obj.markFilteringSet == nullptr) {
      obj.markFilteringSet = b.markFilteringSet;
    }

    // std::cout << "|=" << obj.flag << ";markFilteringSet=" << obj.markFilteringSet << std::endl;
    return obj;
  }

  ~LookupFlag() {
    if (markFilteringSet != nullptr) {
      delete markFilteringSet;
    }
  }

  short getFlag() {
    return flag;
  }

  GlyphSet* getMarkFilteringSet() {
    return markFilteringSet;
  }

  void accept(Visitor&) override;

 private:
  GlyphSet* markFilteringSet;
  short flag;
};

class LookupDefinition : public LookupStatement {
 public:
  LookupDefinition(std::string name, std::vector<Statement*>* stmtlist, int order)
      : name{name}, stmts(stmtlist), order{order} {}

  ~LookupDefinition() {
    if (stmts) {
      for (auto stm : *stmts) {
        delete stm;
      }
    }
    delete stmts;
    // delete name;
  }

  const std::string& getName() {
    return name;
  }

  int getOrder() {
    return order;
  }

  std::vector<Statement*>& getStmts() {
    return *stmts;
  }

  void accept(Visitor&) override;

 private:
  std::string name;
  std::vector<Statement*>* stmts;
  int order;
};

class FeaContext;

class FeaRoot {
  friend FeaContext;

 protected:
  std::vector<Statement*>* stmts;

 public:
  FeaRoot(std::vector<Statement*>* stmtlist)
      : stmts(stmtlist) {}
  ~FeaRoot() {
    for (auto stmt : *stmts) {
      delete stmt;
    }
    delete stmts;
  }
};

class ConditionalStatement : public LookupStatement {
 public:
  ConditionalStatement(std::string condition, std::vector<Statement*>* ifstmts, ConditionalStatement* elseStmt)
      : condition{condition}, ifstmts(ifstmts), elseStmt(elseStmt) {}

  ~ConditionalStatement() {
    if (ifstmts) {
      for (auto stm : *ifstmts) {
        delete stm;
      }
    }
    delete ifstmts;
    delete elseStmt;
  }

  const std::string& getCondition() {
    return condition;
  }

  std::vector<Statement*>& getIfStmts() {
    return *ifstmts;
  }

  ConditionalStatement* getElseStmt() {
    return elseStmt;
  }

  void accept(Visitor&) override;

 private:
  std::string condition;
  std::vector<Statement*>* ifstmts;
  ConditionalStatement* elseStmt;
};

class FeatureDefenition : public Statement {
 public:
  FeatureDefenition(std::string name, std::vector<Statement*>* stmtlist)
      : name{name}, stmts(stmtlist) {}

  ~FeatureDefenition() {
    if (stmts) {
      for (auto stm : *stmts) {
        delete stm;
      }
    }
    delete stmts;
    // delete name;
  }

  const std::string& getName() {
    return name;
  }

  std::vector<Statement*>& getStmts() {
    return *stmts;
  }

  void accept(Visitor&) override;

 private:
  std::string name;
  std::vector<Statement*>* stmts;
};

class IncludeStatment : public Statement {
 public:
  IncludeStatment(std::string fileName)
      : fileName{fileName} {}

  ~IncludeStatment() {
  }

  const std::string& getFileName() {
    return fileName;
  }

  void accept(Visitor&) override;

 private:
  std::string fileName;
};

class FeaContext {
 public:
  typedef std::map<std::string, LookupDefinition*> lookupmap_type;
  typedef std::map<std::string, FeatureDefenition*> featuremap_type;

  lookupmap_type lookups;
  std::vector<FeatureDefenition*> features;
  std::set<FeatureDefenition*> notincludedfeatures;
  std::map<std::string, TableDefinition*> tables;
  JustTable jusTable;

  FeaRoot* root;

  FeaContext(OtLayout* otlayout) : otlayout{otlayout} {
    root = nullptr;
  }

  /// free the saved expression trees
  ~FeaContext() {
    delete root;
  }

  void populateFeatures();

  int getNbLookup() {
    nbLookups++;
    return nbLookups;
  }

 private:
  OtLayout* otlayout;
  int nbLookups = 0;
};

class Visitor {
 public:
  virtual void accept(LookupFlag&) = 0;
  virtual void accept(LookupDefinition&) = 0;
  virtual void accept(ChainingContextualRule&) = 0;
  virtual void accept(SingleAdjustmentRule&) = 0;
  virtual void accept(PairAdjustmentRule&) = 0;
  virtual void accept(CursiveRule&) = 0;
  virtual void accept(Mark2BaseRule&) = 0;
  virtual void accept(SingleSubstituionRule&) = 0;
  virtual void accept(MultipleSubstitutionRule&) = 0;
  virtual void accept(LookupStatement&) = 0;
  virtual void accept(FeatureReference&) = 0;
  virtual void accept(FeatureDefenition&) = 0;
  virtual void accept(ClassDefinition&) = 0;
  virtual void accept(MarkedGlyphSetRegExp&) = 0;
  virtual void accept(LookupReference&) = 0;
  virtual void accept(LigatureSubstitutionRule&) = 0;
  virtual void accept(TableDefinition&) = 0;
  virtual void accept(JustTable&) = 0;
  virtual void accept(IncludeStatment& includeStatment) = 0;
  virtual void accept(ConditionalStatement& conditionalStatement) = 0;
};

class LookupDefinitionVisitor : public Visitor {
 public:
  LookupDefinitionVisitor(OtLayout* otlayout, FeaContext& context);
  void accept(LookupFlag&) override;
  void accept(LookupDefinition&) override;
  void accept(ChainingContextualRule&) override;
  void accept(SingleAdjustmentRule&) override;
  void accept(PairAdjustmentRule&) override;
  void accept(CursiveRule&) override;
  void accept(Mark2BaseRule&) override;
  void accept(SingleSubstituionRule&) override;
  void accept(MultipleSubstitutionRule&) override;
  void accept(LookupStatement&) override;
  void accept(FeatureDefenition&) override;
  void accept(FeatureReference&) override;
  void accept(ClassDefinition&) override;
  void accept(MarkedGlyphSetRegExp&) override;
  void accept(LookupReference&) override;
  void accept(LigatureSubstitutionRule&) override;
  void accept(TableDefinition&) override;
  void accept(JustTable&) override;
  void accept(IncludeStatment& includeStatment) override;
  void accept(ConditionalStatement& conditionalStatement) override;

  Lookup* lookup;
  std::string currentFeature;

 private:
  OtLayout* otlayout;
  std::unordered_set<std::string>* refLookups;
  FeaContext& context;

  std::stack<std::tuple<Lookup*, std::unordered_set<std::string>*, int>> lookupStack;
  int nextautolookup = 1;
};

}  // namespace feayy

#endif  // H_FEAAST
