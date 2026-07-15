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

#include "feaast.h"

#include <QDebug>

#include "Subtable.h"

using namespace std;

namespace feayy {

std::unordered_set<std::uint16_t> GlyphName::getCodes(OtLayout* otlayout) {
  const auto charcode = getCode(otlayout);
  const auto substitutions = otlayout->getSubsts(charcode);
  std::unordered_set<std::uint16_t> codes{charcode};
  codes.insert(substitutions.begin(), substitutions.end());
  return codes;
}

std::uint16_t GlyphName::getCode(OtLayout* otlayout) {
  const auto found = otlayout->glyphCodePerName.find(name);
  if (found == otlayout->glyphCodePerName.end()) {
    throw std::runtime_error("Glyph Name " + name + " not found");
  }
  return found->second;
}

std::unordered_set<std::uint16_t> ClassName::getCodes(OtLayout* otlayout) {
  const auto codes = otlayout->classtoUnicode(name);
  if (codes.empty()) {
    throw std::runtime_error("Class " + name + " is empty");
  }
  return codes;
}

std::unordered_set<std::uint16_t> RegExpClass::getCodes(OtLayout* otlayout) {
  return otlayout->regexptoUnicode(_regexpr);
}

std::unordered_set<std::uint16_t> GlyphCID::getCodes(OtLayout* otlayout) {
  const auto code = getCode(otlayout);
  const auto substitutions = otlayout->getSubsts(code);
  std::unordered_set<std::uint16_t> codes{code};
  codes.insert(substitutions.begin(), substitutions.end());
  return codes;
}

std::uint16_t GlyphCID::getCode(OtLayout* otlayout) {
  if (!otlayout->glyphNamePerCode.contains(cid)) {
    throw std::runtime_error("GlyphID " + std::to_string(cid) + " does not exist");
  }
  return static_cast<std::uint16_t>(cid);
}

void LookupFlag::accept(Visitor& v) { v.accept(*this); }
void LookupDefinition::accept(Visitor& v) { v.accept(*this); }
void FeatureReference::accept(Visitor& v) { v.accept(*this); }
void LookupReference::accept(Visitor& v) { v.accept(*this); }
void ChainingContextualRule::accept(Visitor& v) { v.accept(*this); }
void SingleAdjustmentRule::accept(Visitor& v) { v.accept(*this); }
void PairAdjustmentRule::accept(Visitor& v) { v.accept(*this); }
void CursiveRule::accept(Visitor& v) { v.accept(*this); }
void Mark2BaseRule::accept(Visitor& v) { v.accept(*this); }
void SingleSubstituionRule::accept(Visitor& v) { v.accept(*this); }
void LookupStatement::accept(Visitor& v) { v.accept(*this); }
void FeatureDefenition::accept(Visitor& v) { v.accept(*this); }
void ClassDefinition::accept(Visitor& v) { v.accept(*this); }
void MarkedGlyphSetRegExp::accept(Visitor& v) { v.accept(*this); }
void LigatureSubstitutionRule::accept(Visitor& v) { v.accept(*this); }
void MultipleSubstitutionRule::accept(Visitor& v) { v.accept(*this); }
void TableDefinition::accept(Visitor& v) { v.accept(*this); }
void JustTable::accept(Visitor& v) { v.accept(*this); }
void IncludeStatment::accept(Visitor& v) { v.accept(*this); }
void ConditionalStatement::accept(Visitor& v) { v.accept(*this); }

void FeaContext::populateFeatures() {
  LookupDefinitionVisitor feaVisitor{otlayout, *this};

  // TODO optimize and support else
  for (auto stmt : *root->stmts) {
    auto conditionalStatement = dynamic_cast<ConditionalStatement*>(stmt);
    if (conditionalStatement != nullptr) {
      bool conditionIsTrue = false;
      auto condition = QString::fromStdString(conditionalStatement->getCondition());
      if (condition == "") {
        conditionIsTrue = true;
      } else {
        conditionIsTrue = otlayout->font->getBoolVariable(condition);
      }
      if (!conditionIsTrue) {
        for (auto stmt : conditionalStatement->getIfStmts()) {
          auto featureDef = dynamic_cast<FeatureDefenition*>(stmt);
          if (featureDef) {
            notincludedfeatures.insert(featureDef);
          }
        }
      }
    }
  }

  for (auto featureDefinition : features) {
    if (notincludedfeatures.find(featureDefinition) == notincludedfeatures.end())
      featureDefinition->accept(feaVisitor);
  }

  for (auto table : tables) {
    TableDefinition* tableDefinition = table.second;

    tableDefinition->accept(feaVisitor);
  }

  jusTable.accept(feaVisitor);
}

LookupDefinitionVisitor::LookupDefinitionVisitor(OtLayout* otlayout, FeaContext& context) : lookup{nullptr}, otlayout{otlayout}, refLookups{nullptr}, context{context} {
}

void LookupDefinitionVisitor::accept(LookupFlag& flag) {
  lookup->flags = flag.getFlag();

  auto glyphSet = flag.getMarkFilteringSet();

  if (glyphSet != nullptr) {
    const auto codes = glyphSet->getCodes(otlayout);
    lookup->markGlyphSetIndex = otlayout->addMarkSet(QList<quint16>(codes.begin(), codes.end()));
  }
}
void LookupDefinitionVisitor::accept(LookupDefinition& lookupDefinition) {
  std::unordered_set<std::string>* old_refLookups = this->refLookups;
  Lookup* old_lookup = this->lookup;
  int old_nextautolookup = this->nextautolookup;

  std::unordered_set<std::string> localrefLookups;

  this->refLookups = &localrefLookups;
  this->lookup = new Lookup(otlayout);

  lookup->name = QString::fromStdString(lookupDefinition.getName());

  for (auto stmt : lookupDefinition.getStmts()) {
    stmt->accept(*this);
  }

  if (lookup->subtables.count() != 0) {
    otlayout->addLookup(lookup);
  }

  for (auto refLookupName : localrefLookups) {
    const auto qRefLookupName = QString::fromStdString(refLookupName);
    auto lookupsIndex = otlayout->lookupsIndexByName.value(qRefLookupName, -1);
    if (lookupsIndex == -1) {
      auto liter = context.lookups.find(refLookupName);

      if (liter == context.lookups.end())
        continue;

      LookupDefinition* lookupDefinition = liter->second;

      lookupDefinition->accept(*this);

      lookupsIndex = otlayout->lookupsIndexByName.value(qRefLookupName, -1);
    }
    if (!currentFeature.empty() && lookupsIndex != -1) {
      Lookup* ll = otlayout->lookups[lookupsIndex];
      if (ll->feature == "inherited") {
        otlayout->allFeatures[QString::fromStdString(currentFeature)].insert(ll);
      }
    }
  }

  this->refLookups = old_refLookups;
  this->lookup = old_lookup;
  this->nextautolookup = old_nextautolookup;
}

void LookupDefinitionVisitor::accept(FeatureReference& featureReference) {
  lookup->feature = QString::fromStdString(featureReference.featureName);
}

void LookupDefinitionVisitor::accept(SingleAdjustmentRule& singleRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::singleadjustment;
  } else if (lookup->type != Lookup::singleadjustment) {
    throw "Lookup with different subtable type";
  }

  int format = 2;

  if (singleRule.color) {
    format = 3;
  }

  int subtableName = lookup->subtables.length() + 1;
  SingleAdjustmentSubtable* newsubtable = nullptr;
  if (lookup->subtables.length() > 0) {
    newsubtable = dynamic_cast<SingleAdjustmentSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr || newsubtable->format != format) {
    newsubtable = new SingleAdjustmentSubtable(lookup, format);
    newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
    lookup->subtables.append(newsubtable);
  }

  auto unicodes = singleRule.glyphset->getCodes(otlayout);

  for (auto unicode : unicodes) {
    newsubtable->singlePos[unicode] = singleRule.valueRecord;
  }
}

void LookupDefinitionVisitor::accept(PairAdjustmentRule& pairRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::pairadjustment;
  } else if (lookup->type != Lookup::pairadjustment) {
    throw "Lookup with different subtable type";
  }

  int format = 1;

  int subtableName = lookup->subtables.length() + 1;
  PairAdjustmentSubtable* newsubtable = nullptr;
  if (lookup->subtables.length() > 0) {
    newsubtable = dynamic_cast<PairAdjustmentSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr || newsubtable->format != format) {
    newsubtable = new PairAdjustmentSubtable(lookup, format);
    newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
    lookup->subtables.append(newsubtable);
  }

  auto codes1 = pairRule.glyphSet1->getCodes(otlayout);

  for (auto code1 : codes1) {
    auto& pairPos = newsubtable->pairPos[code1];
    std::variant<ValueRecord, PairAdjustFunc> valueRecord1;
    std::variant<ValueRecord, PairAdjustFunc> valueRecord2;
    if (std::holds_alternative<ValueRecord>(pairRule.valueRecord1)) {
      valueRecord1 = std::get<ValueRecord>(pairRule.valueRecord1);
    } else {
      valueRecord1 = otlayout->getPairAdjustFunction(std::get<std::string>(pairRule.valueRecord1), newsubtable);
    }
    if (std::holds_alternative<ValueRecord>(pairRule.valueRecord2)) {
      valueRecord2 = std::get<ValueRecord>(pairRule.valueRecord2);
    } else {
      auto& funcName = std::get<std::string>(pairRule.valueRecord2);
      auto func = otlayout->getPairAdjustFunction(funcName, newsubtable);
      if (func) {
        valueRecord2 = func;
      } else {
        std::cerr << "Pair adjustment function " << funcName << " does not exist for lookup " << lookup->name.toStdString() << std::endl;
        valueRecord2 = ValueRecord{};
      }
    }
    auto codes2 = pairRule.glyphSet2->getCodes(otlayout);
    for (auto code2 : codes2) {
      pairPos.insert_or_assign(code2, PairAdjustmentSubtable::PairValue{valueRecord1, valueRecord2});
    }
  }
}

void LookupDefinitionVisitor::accept(CursiveRule& cursiveRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::cursive;
  } else if (lookup->type != Lookup::cursive) {
    throw "Lookup with different subtable type";
  }

  int subtableName = lookup->subtables.length() + 1;
  CursiveSubtable* newsubtable = nullptr;
  if (lookup->subtables.length() > 0) {
    newsubtable = dynamic_cast<CursiveSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr) {
    newsubtable = new CursiveSubtable(lookup);
    newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
    lookup->subtables.append(newsubtable);
  }

  CursiveSubtable::EntryExit value;

  if (cursiveRule.entryAnchor->anchortype() == AnchorType::FormatA) {
    auto anchor = static_cast<AnchorFormatA*>(cursiveRule.entryAnchor);
    value.entry = QPoint(anchor->x, anchor->y);
  } else if (cursiveRule.entryAnchor->anchortype() == AnchorType::Name) {
    auto anchor = static_cast<AnchorName*>(cursiveRule.entryAnchor);
    value.entryName = anchor->name;
  } else if (cursiveRule.entryAnchor->anchortype() == AnchorType::Function) {
    auto functionAnchor = static_cast<AnchorFunction*>(cursiveRule.entryAnchor);
    value.entryFunction = otlayout->getCursiveFunctions(QString::fromStdString(functionAnchor->name), newsubtable);
  }

  if (cursiveRule.exitAnchor->anchortype() == AnchorType::FormatA) {
    auto anchor = static_cast<AnchorFormatA*>(cursiveRule.exitAnchor);
    value.exit = QPoint(anchor->x, anchor->y);
  } else if (cursiveRule.exitAnchor->anchortype() == AnchorType::Name) {
    auto anchor = static_cast<AnchorName*>(cursiveRule.exitAnchor);
    value.exitName = anchor->name;
  } else if (cursiveRule.exitAnchor->anchortype() == AnchorType::Function) {
    auto functionAnchor = static_cast<AnchorFunction*>(cursiveRule.exitAnchor);
    value.exitFunction = otlayout->getCursiveFunctions(QString::fromStdString(functionAnchor->name), newsubtable);
  }

  auto codes = cursiveRule.glyphset->getCodes(otlayout);

  for (auto code : codes) {
    newsubtable->anchors[code] = value;
  }
}

void LookupDefinitionVisitor::accept(Mark2BaseRule& mark2BaseRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = mark2BaseRule.type;
  } else if (lookup->type != mark2BaseRule.type) {
    throw "Lookup with different subtable type";
  }

  int subtableName = lookup->subtables.length() + 1;
  MarkBaseSubtable* newsubtable = new MarkBaseSubtable(lookup);
  newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
  lookup->subtables.append(newsubtable);

  const auto sortedBaseCodes = mark2BaseRule.baseGlyphSet->getSortedCodes(otlayout);
  newsubtable->sortedBaseCodes.assign(sortedBaseCodes.begin(), sortedBaseCodes.end());

  for (auto mark2baseclass : *mark2BaseRule.mark2baseclasses) {
    QString className = QString::fromStdString(mark2baseclass->className);

    // className = lookup->name + "." + className;

    MarkBaseSubtable::MarkClass newclass;

    const auto markCodes = mark2baseclass->glyphset->getCodes(otlayout);
    newclass.markCodes.insert(markCodes.begin(), markCodes.end());

    if (mark2baseclass->baseAnchor->anchortype() == AnchorType::Function) {
      auto functionAnchor = static_cast<AnchorFunction*>(mark2baseclass->baseAnchor);
      newclass.basefunction = otlayout->getanchorCalcFunctions(QString::fromStdString(functionAnchor->name), newsubtable);
    } else if (mark2baseclass->baseAnchor->anchortype() == AnchorType::FormatA) {
      auto formaAAnchor = static_cast<AnchorFormatA*>(mark2baseclass->baseAnchor);
      auto anchor = QPoint{formaAAnchor->x, formaAAnchor->y};
      for (auto code : newsubtable->sortedBaseCodes) {
        auto glyphName = otlayout->glyphNamePerCode[code];
        newclass.baseanchors[glyphName] = anchor;
      }
    }

    if (mark2baseclass->markAnchor->anchortype() == AnchorType::Function) {
      auto functionAnchor = static_cast<AnchorFunction*>(mark2baseclass->markAnchor);
      newclass.markfunction = otlayout->getanchorCalcFunctions(QString::fromStdString(functionAnchor->name), newsubtable);
    } else if (mark2baseclass->markAnchor->anchortype() == AnchorType::FormatA) {
      auto formaAAnchor = static_cast<AnchorFormatA*>(mark2baseclass->markAnchor);
      auto anchor = QPoint{formaAAnchor->x, formaAAnchor->y};
      for (auto code : newclass.markCodes) {
        auto markName = otlayout->glyphNamePerCode[code];
        newclass.markanchors[markName] = anchor;
      }
    }

    newsubtable->classes[className.toStdString()] = newclass;
  }
}

void LookupDefinitionVisitor::accept(SingleSubstituionRule& singleRule) {
  bool multiple = false;

  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::single;
  } else if (lookup->type == Lookup::multiple && singleRule.format != 10 && singleRule.format != 11 && singleRule.firstType != SingleSubstituionRule::FirstType::GLYPHSET) {
    auto newsubtable = static_cast<MultipleSubtable*>(lookup->subtables.last());

    /*auto firstunicodes = singleRule.firstglyph->getCodes(otlayout);

    if (firstunicodes.size() != 1) {
      throw "Single subtitution : first glyph different to 1 matching";
    }

    auto firstunicode = *firstunicodes.begin();*/

    auto firstunicode = singleRule.firstglyph->getCode(otlayout);

    /*auto secondtunicodes = singleRule.secondglyph->getCodes(otlayout);

    if (secondtunicodes.size() != 1) {
      throw "Single subtitution : second glyph different to 1 matching";
    }

    auto secondunicode = *secondtunicodes.begin();*/

    auto secondunicode = singleRule.secondglyph->getCode(otlayout);

    newsubtable->subst.emplace(firstunicode, std::vector<quint16>{secondunicode});

    return;
  } else if (lookup->type != Lookup::single) {
    throw "Lookup with different subtable type";
  }

  int subtableIndex = lookup->subtables.length() + 1;
  SingleSubtable* newsubtable = nullptr;
  if (lookup->subtables.length() > 0) {
    newsubtable = static_cast<SingleSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr || newsubtable->format != singleRule.format) {
    if (singleRule.format == 10) {
      newsubtable = new SingleSubtableWithExpansion(lookup);
    } else if (singleRule.format == 11) {
      newsubtable = new SingleSubtableWithTatweel(lookup);
    } else {
      newsubtable = new SingleSubtable(lookup, singleRule.format);
    }

    newsubtable->name = QString("subtable%1").arg(subtableIndex).toStdString();
    lookup->subtables.append(newsubtable);
  }

  if (singleRule.format != 11) {
    if (singleRule.firstType == SingleSubstituionRule::FirstType::GLYPHSET) {
      auto firstunicodes = singleRule.firstGlyphSet->getCodes(otlayout);

      for (auto code : firstunicodes) {
        newsubtable->subst[code] = code;

        if (singleRule.format == 10) {
          ((SingleSubtableWithExpansion*)newsubtable)->expansion[code] = singleRule.expansion;
          ((SingleSubtableWithExpansion*)newsubtable)->expansion[code].startEndLig = singleRule.startEndLig;
        }
      }
    } else {
      /*auto firstunicodes = singleRule.firstglyph->getCodes(otlayout);

      if (firstunicodes.size() != 1) {
        throw "Single subtitution : first glyph different to 1 matching";
      }

      auto firstunicode = *firstunicodes.begin();*/

      auto firstunicode = singleRule.firstglyph->getCode(otlayout);

      /*auto secondtunicodes = singleRule.secondglyph->getCodes(otlayout);

      if (secondtunicodes.size() != 1) {
        throw "Single subtitution : second glyph different to 1 matching";
      }

      auto secondunicode = *secondtunicodes.begin();*/

      auto secondunicode = singleRule.secondglyph->getCode(otlayout);

      newsubtable->subst[firstunicode] = secondunicode;

      if (singleRule.format == 10) {
        ((SingleSubtableWithExpansion*)newsubtable)->expansion[firstunicode] = singleRule.expansion;
        ((SingleSubtableWithExpansion*)newsubtable)->expansion[firstunicode].startEndLig = singleRule.startEndLig;
      }
    }

  } else if (singleRule.format == 11) {
    if (singleRule.firstType == SingleSubstituionRule::FirstType::GLYPHSET) {
      auto firstunicodes = singleRule.firstGlyphSet->getCodes(otlayout);

      SingleSubtableWithTatweel* subtable = (SingleSubtableWithTatweel*)newsubtable;

      for (auto code : firstunicodes) {
        subtable->expansion[code] = singleRule.expansion;
        subtable->subst[code] = code;
      }
    } else {
      /*auto firstunicodes = singleRule.firstglyph->getCodes(otlayout);
      if (firstunicodes.size() != 1) {
        throw "Single subtitution : first glyph different to 1 matching";
      }

      auto firstunicode = *firstunicodes.begin();*/

      auto firstunicode = singleRule.firstglyph->getCode(otlayout);

      /*auto secondtunicodes = singleRule.secondglyph->getCodes(otlayout);

      if (secondtunicodes.size() != 1) {
        throw "Single subtitution : second glyph different to 1 matching";
      }

      auto secondunicode = *secondtunicodes.begin();*/

      auto secondunicode = singleRule.secondglyph->getCode(otlayout);

      SingleSubtableWithTatweel* subtable = (SingleSubtableWithTatweel*)newsubtable;

      subtable->subst[firstunicode] = secondunicode;
      subtable->expansion[firstunicode] = singleRule.expansion;
    }
  }
}
void LookupDefinitionVisitor::accept(ChainingContextualRule& contextualRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = contextualRule.type;
  } else if (lookup->type != contextualRule.type) {
    throw "Lookup with different subtable type";
  }

  std::vector<std::vector<GlyphSet*>> backtrack;

  if (contextualRule.backtrack) {
    backtrack = contextualRule.backtrack->getSequences();
  } else {
    backtrack = {{}};
  }

  std::vector<std::vector<std::unordered_set<std::uint16_t>>> compiledbacktrack;

  for (auto seq : backtrack) {
    std::vector<std::unordered_set<std::uint16_t>> compiledseq;
    for (auto glyphset : seq) {
      auto set = glyphset->getCodes(otlayout);
      if (!set.empty()) {
        compiledseq.push_back(std::move(set));
      } else {
        throw "Glyphset is empty";
      }
    }
    compiledbacktrack.push_back(std::move(compiledseq));
  }

  std::vector<std::vector<GlyphSet*>> lookahead;

  if (contextualRule.lookahead) {
    lookahead = contextualRule.lookahead->getSequences();
  } else {
    lookahead = {{}};
  }

  std::vector<std::vector<std::unordered_set<std::uint16_t>>> compiledlookahead;

  for (auto seq : lookahead) {
    std::vector<std::unordered_set<std::uint16_t>> compiledseq;
    for (auto glyphset : seq) {
      auto set = glyphset->getCodes(otlayout);
      if (!set.empty()) {
        compiledseq.push_back(std::move(set));
      } else {
        throw "Glyphset is empty";
      }
    }
    compiledlookahead.push_back(std::move(compiledseq));
  }

  using CompiledInput = std::vector<std::pair<GlyphSet*, std::vector<std::string>>>;
  std::vector<CompiledInput> compiledinputs = {{}};

  LookupDefinition* lastautolookup{nullptr};
  InlineType lastInlineType{InlineType::None};

  for (auto value : *contextualRule.input) {
    // value->accept(*this);
    if (value->inlineType != InlineType::None) {
      if (lastInlineType == value->inlineType && lastInlineType == InlineType::CursivePos) {
        lastautolookup->getStmts().push_back(value->stmt);
        std::string named = lastautolookup->getName();
        value->lookupNames = {named};
      } else {
        auto stmts = new vector<Statement*>();
        stmts->push_back(value->stmt);

        QString name = QString("%1_auto%2").arg(lookup->name).arg(nextautolookup++);

        std::string named = name.toStdString();

        value->lookupNames = {named};
        lastautolookup = new LookupDefinition(named, stmts, context.getNbLookup());
        context.lookups[named] = lastautolookup;
      }
    }
    lastInlineType = value->inlineType;
    auto input = value->regexp->getSequences();
    std::vector<std::string> lookupNames;
    if (value->lookupNames.size() > 0) {
      for (auto& lookupName : value->lookupNames) {
        lookupNames.push_back(lookupName);
      }
    }

    std::vector<CompiledInput> result;
    for (auto ii : input) {
      auto temp = compiledinputs;

      for (auto& compiledinput : temp) {
        for (auto jj : ii) {
          compiledinput.emplace_back(jj, lookupNames);
        }
      }

      result.insert(result.end(), temp.begin(), temp.end());
    }

    compiledinputs = result;
  }

  for (auto backtrack : compiledbacktrack) {
    for (auto lookahead : compiledlookahead) {
      for (auto input : compiledinputs) {
        ChainingSubtable* newsubtable = new ChainingSubtable(lookup);
        lookup->subtables.append(newsubtable);

        for (const auto& set : backtrack) {
          newsubtable->compiledRule.backtrack.emplace_back(set.begin(), set.end());
        }

        for (auto pair : input) {
          auto set = pair.first->getCachedCodes(otlayout);
          if (!set.empty()) {
            if (!pair.second.empty()) {
              for (auto& lookupName : pair.second) {
                refLookups->insert(lookupName);
                newsubtable->compiledRule.lookupRecords.push_back({std::uint16_t(newsubtable->compiledRule.input.size()), lookupName});
              }
            }
            newsubtable->compiledRule.input.emplace_back(set.begin(), set.end());
          } else {
            throw "empty set in lookup " + lookup->name;
          }
        }

        for (const auto& set : lookahead) {
          newsubtable->compiledRule.lookahead.emplace_back(set.begin(), set.end());
        }
      }
    }
  }
}

void LookupDefinitionVisitor::accept(ClassDefinition& classDef) {
  QSet<QString> set;

  for (auto glyph : classDef.components->getCodes(otlayout)) {
    set.insert(QString::fromStdString(otlayout->glyphNamePerCode[glyph]));
  }
  otlayout->addClass(QString::fromStdString(classDef.name), set);
}

void LookupDefinitionVisitor::accept(MarkedGlyphSetRegExp& markedGlyphSetRegExp) {
  if (markedGlyphSetRegExp.inlineType != InlineType::None) {
    auto stmts = new vector<Statement*>();
    stmts->push_back(markedGlyphSetRegExp.stmt);

    QString name = QString("%1_auto%2").arg(lookup->name).arg(nextautolookup++);

    auto lookupName = name.toStdString();

    markedGlyphSetRegExp.lookupNames = {lookupName};
    auto lookup = new LookupDefinition(lookupName, stmts, context.getNbLookup());
    context.lookups[lookupName] = lookup;
  }
}

void LookupDefinitionVisitor::accept(LookupStatement&) {
}

void LookupDefinitionVisitor::accept(LookupReference& lookupReference) {
  const auto qLookupName = QString::fromStdString(lookupReference.lookupName);
  auto lookupsIndex = otlayout->lookupsIndexByName.value(qLookupName, -1);

  if (lookupsIndex == -1) {
    auto liter = context.lookups.find(lookupReference.lookupName);

    if (liter == context.lookups.end()) {
      auto ret = otlayout->parseCppLookup(qLookupName);
      if (!ret) {
        qDebug() << "Lookup " << qLookupName << " not found";
      }
      return;
    }

    LookupDefinition* lookupDefinition = liter->second;

    lookupDefinition->accept(*this);

    lookupsIndex = otlayout->lookupsIndexByName.value(qLookupName, -1);
  }
  if (!currentFeature.empty() && lookupsIndex != -1) {
    Lookup* ll = otlayout->lookups[lookupsIndex];
    if (ll->feature == "inherited") {
      otlayout->allFeatures[QString::fromStdString(currentFeature)].insert(ll);
    }
  }
}
void LookupDefinitionVisitor::accept(FeatureDefenition& featureDefenition) {
  this->currentFeature = featureDefenition.getName();

  for (auto stmt : featureDefenition.getStmts()) {
    stmt->accept(*this);
  }

  this->currentFeature.clear();
}

void LookupDefinitionVisitor::accept(MultipleSubstitutionRule& multipleSubstitutionRule) {
  MultipleSubtable* newsubtable = nullptr;

  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::multiple;
  } else if (lookup->type == Lookup::single && lookup->subtables.length() == 1) {
    lookup->type = Lookup::multiple;
    newsubtable = new MultipleSubtable(lookup);
    auto oldsubtable = static_cast<SingleSubtable*>(lookup->subtables.last());
    for (auto it = oldsubtable->subst.begin(); it != oldsubtable->subst.end(); it++) {
      newsubtable->subst.emplace(it->first, std::vector<quint16>{it->second});
    }
    delete oldsubtable;
    lookup->subtables.clear();
    lookup->subtables.append(newsubtable);
  } else if (lookup->type != Lookup::multiple) {
    throw "Lookup with different subtable type";
  }

  int subtableName = lookup->subtables.length() + 1;

  if (lookup->subtables.length() > 0) {
    newsubtable = static_cast<MultipleSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr || newsubtable->format != multipleSubstitutionRule.format) {
    newsubtable = new MultipleSubtable(lookup);

    newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
    lookup->subtables.append(newsubtable);
  }

  /*auto firstunicodes = multipleSubstitutionRule.glyph->getCodes(otlayout);

  if (firstunicodes.size() != 1) {
    throw "multiple subtitution : glyph different to 1 matching";
  }

  auto glyphCode = *firstunicodes.begin();*/

  auto glyphCode = multipleSubstitutionRule.glyph->getCode(otlayout);

  std::vector<quint16> seq;

  for (auto glyph : *multipleSubstitutionRule.sequence) {
    /*auto unicodes = glyph->getCodes(otlayout);
    if (unicodes.size() != 1) {
      throw "multiple subtitution : glyph different to 1 matching";
    }

    seq.append(*unicodes.begin());*/

    auto unicode = glyph->getCode(otlayout);
    seq.push_back(unicode);
  }

  newsubtable->subst.insert_or_assign(glyphCode, std::move(seq));
}

void LookupDefinitionVisitor::accept(LigatureSubstitutionRule& ligatureSubstitutionRule) {
  if (lookup->type == Lookup::none) {
    lookup->type = Lookup::ligature;
  } else if (lookup->type != Lookup::ligature) {
    throw "Lookup with different subtable type";
  }

  int subtableName = lookup->subtables.length() + 1;
  LigatureSubtable* newsubtable = nullptr;
  if (lookup->subtables.length() > 0) {
    newsubtable = static_cast<LigatureSubtable*>(lookup->subtables.last());
  }

  if (newsubtable == nullptr || newsubtable->format != ligatureSubstitutionRule.format) {
    newsubtable = new LigatureSubtable(lookup);

    newsubtable->name = QString("subtable%1").arg(subtableName).toStdString();
    lookup->subtables.append(newsubtable);
  }

  auto firstunicodes = ligatureSubstitutionRule.ligature->getCodes(otlayout);

  if (firstunicodes.size() != 1) {
    throw "ligature subtitution : ligature glyph different to 1 matching";
  }

  LigatureSubtable::Ligature ligStruct;

  ligStruct.ligatureGlyph = *firstunicodes.begin();

  for (auto glyph : *ligatureSubstitutionRule.sequence) {
    auto unicodes = glyph->getCodes(otlayout);
    if (unicodes.size() != 1) {
      throw "ligature subtitution : glyph different to 1 matching";
    }

    ligStruct.componentGlyphIDs.push_back(*unicodes.begin());
  }

  newsubtable->ligatures.push_back(std::move(ligStruct));
}

void LookupDefinitionVisitor::accept(JustTable& jusTable) {
  Just ot_justTable{otlayout};

  for (auto& lname : jusTable.aftergsub) {
    QString name = QString::fromStdString(lname);
    auto lookupsIndex = otlayout->lookupsIndexByName.value(name, -1);
    if (lookupsIndex == -1) {
      auto liter = context.lookups.find(lname);

      if (liter == context.lookups.end()) {
        throw new std::runtime_error("Lookup " + lname + " not found");
      }

      LookupDefinition* lookupDefinition = liter->second;

      lookupDefinition->accept(*this);

      lookupsIndex = otlayout->lookupsIndexByName.value(name, -1);
    }
    auto* lookup = otlayout->lookups[lookupsIndex];
    if (!lookup->isGsubLookup()) {
      throw new std::runtime_error("Cannot have gpos lookup for aftergsub");
    }

    ot_justTable.lastGsubLookups.push_back(lookup);
  }

  for (int i = 0; i < 2; i++) {
    auto& rules = i == 0 ? jusTable.stretchRules : jusTable.shrinkRules;
    auto& ot_rules = i == 0 ? ot_justTable.stretchSteps : ot_justTable.shrinkSteps;

    for (auto& step : rules) {
      Just::JustStep ot_Step;
      for (auto& lname : step.lookupNames) {
        QString name = QString::fromStdString(lname);
        auto lookupsIndex = otlayout->lookupsIndexByName.value(name, -1);
        if (lookupsIndex == -1) {
          auto liter = context.lookups.find(lname);

          if (liter == context.lookups.end()) {
            throw new std::runtime_error("Lookup " + lname + " not found");
          }

          LookupDefinition* lookupDefinition = liter->second;

          lookupDefinition->accept(*this);

          lookupsIndex = otlayout->lookupsIndexByName.value(name, -1);
        }
        auto* lookup = otlayout->lookups[lookupsIndex];
        if (ot_Step.lookups.size() != 0 && ot_Step.gsub != lookup->isGsubLookup()) {
          throw new std::runtime_error("Cannot have step with mix of gsub and gpos");
        }
        ot_Step.gsub = lookup->isGsubLookup();
        ot_Step.lookups.push_back(lookup);
      }
      ot_rules.push_back(ot_Step);
    }
  }

  otlayout->justTable = ot_justTable;
}

void LookupDefinitionVisitor::accept(IncludeStatment& includeStatment) {
}

void LookupDefinitionVisitor::accept(ConditionalStatement& conditionalStatement) {
  bool conditionIsTrue = false;
  auto condition = QString::fromStdString(conditionalStatement.getCondition());
  if (condition == "") {
    conditionIsTrue = true;
  } else {
    conditionIsTrue = otlayout->font->getBoolVariable(condition);
  }
  if (conditionIsTrue) {
    for (auto stmt : conditionalStatement.getIfStmts()) {
      stmt->accept(*this);
    }
  } else {
    auto elseStmt = conditionalStatement.getElseStmt();
    if (elseStmt != nullptr) {
      elseStmt->accept(*this);
    }
  }
}

}  // namespace feayy
