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
#include "just.h"
#include "GlyphVis.h"
#include <iostream>
#include "Subtable.h"
#include <stack>

using namespace std;

namespace feayy {

  std::shared_ptr<RuleRegExp>  RuleRegExp::makeRepetition(int min, int max) {


    if (min < 0 || (max != -1 && min > max) || (min == 0 && max <= 0)) {
      throw new std::runtime_error("bad repetition values");
    }

    PRuleRegExp ret;

    if (min != 0) {
      ret = this->clone();
      for (int i = 1; i < min; i++) {
        ret = std::make_shared<RuleRegExpConcat>(ret, this->clone());
      }
      if (max == -1) {
        ret = std::make_shared<RuleRegExpConcat>(ret, std::make_shared<RuleRegExpRepeat>(this->clone(), RuleRegExpRepeatType::STAR));
      }
      else {
        for (int i = min; i < max; i++) {
          ret = std::make_shared<RuleRegExpConcat>(ret, std::make_shared<RuleRegExpRepeat>(this->clone(), RuleRegExpRepeatType::OPT));
        }
      }
    }
    else {
      ret = std::make_shared<RuleRegExpRepeat>(this->clone(), RuleRegExpRepeatType::OPT);
      for (int i = min + 1; i < max; i++) {
        ret = std::make_shared<RuleRegExpConcat>(ret, std::make_shared<RuleRegExpRepeat>(this->clone(), RuleRegExpRepeatType::OPT));
      }
    }

    return ret;

  }

  std::shared_ptr<RuleRegExp> RuleRegExpGlyphSet::clone() {
    return make_shared<RuleRegExpGlyphSet>(element->clone());
  }

  void RuleRegExpLeaf::setBackupSequence(std::vector<BackupSequence>& sequences) {

    int add = dynamic_cast<RuleRegExpGlyphSet*>(this) || dynamic_cast<RuleRegExpANY*>(this) ? 1 : 0;
    if (!sequences.empty()) {
      for (auto& seq : sequences) {
        seq.sequence.push_back(shared_from_this());
        seq.glyphSetNumber += add;
      }
    }
    else {
      sequences.push_back({ {shared_from_this()},add });
    }
  }


  /*
  void RuleRegExpANY::setBackupSequence(std::vector<BackupSequence>& sequences) {
    int add = dynamic_cast<RuleRegExpGlyphSet*>(this) || dynamic_cast<RuleRegExpANY*>(this) ? 1 : 0;
    if (!sequences.empty()) {
      for (auto& seq : sequences) {
        seq.sequence.push_back(shared_from_this());
        seq.glyphSetNumber += add;
      }
    }
    else {
      sequences.push_back({ {shared_from_this()},add });
    }
  }*/

  GraphiteRule::GraphiteRule(PRuleRegExp lhs, PRuleRegExp rhs, PRuleRegExp ctx) : lhs{ lhs }, rhs{ rhs }, ctx{ ctx } {
    if (lhs != nullptr)
      lhs->parentRule_ = this;
    if (rhs != nullptr)
      rhs->parentRule_ = this;
    if (ctx != nullptr)
      ctx->parentRule_ = this;
  }


  void LookupDefinitionVisitor::accept(TableDefinition& tableDefinition) {

    bool insideLookup = this->lookup != nullptr;

    Lookup* lookup = nullptr;

    if (insideLookup) {
      Lookup::Type type;

      lookup = this->lookup;

      if (lookup->type == Lookup::none) {
        if (tableDefinition.name == "sub") {
          lookup->type = Lookup::fsmgsub;
        }
        else if (tableDefinition.name == "pos") {
          lookup->type = Lookup::fsmgpos;
        }
        else {
          throw "non valid table name. an be only 'sub' or 'pos'";
        }
      }
      else if (lookup->type != type) {
        throw "Lookup cannot have more than one table";
      }
    }
    else {
      lookup = new Lookup(otlayout);
      lookup->type = Lookup::fsmgsub;
      lookup->name = QString::fromStdString(tableDefinition.name);
      otlayout->setDisabled(lookup);
    }





    for (auto pass : tableDefinition.passes) {
      FSMSubtable* fsm = new FSMSubtable(lookup);
      fsm->name = QString::fromStdString(tableDefinition.name) + QString("%1").arg(pass.number());
      fsm->dfa = pass.computeDFA(*otlayout);
      lookup->subtables.append(fsm);
    }

    if (!insideLookup && lookup->subtables.count() != 0) {
      otlayout->addTable(lookup);
    }
  }



  

  void RuleRegExpConcat::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpRepeat::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpOr::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpANY::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpEndMarker::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpAction::accept(RuleRegExpVisitor& v) { v.accept(*this); }
  void RuleRegExpGlyphSet::accept(RuleRegExpVisitor& v) { v.accept(*this); }

  void EqClassesVisitor::accept(RuleRegExpConcat& regExp) {
    regExp.lhs().accept(*this);
    regExp.rhs().accept(*this);
  };
  void EqClassesVisitor::accept(RuleRegExpRepeat& regExp) {
    regExp.innerExpr().accept(*this);
  };
  void EqClassesVisitor::accept(RuleRegExpOr& regExp) {
    regExp.lhs().accept(*this);
    regExp.rhs().accept(*this);
  };
  void EqClassesVisitor::accept(RuleRegExpANY&) {};
  void EqClassesVisitor::accept(RuleRegExpEndMarker&) {};
  void EqClassesVisitor::accept(RuleRegExpAction&) {};
  void EqClassesVisitor::accept(RuleRegExpGlyphSet& regExp) {

    eqClassesByGlyphSet.insert(&regExp, {});

    auto glyphSet = regExp.getGlyphSet();

    if (glyphSet == nullptr) return;

    auto codes = glyphSet->getCachedCodes(otlayout);

    if (codes.isEmpty()) return;

    QMutableVectorIterator<QSet<quint16>> i(eqClasses);

    QVector<QSet<quint16>> newSets;

    while (i.hasNext()) {
      auto set = i.next();

      auto intersect = set & codes;

      if (!intersect.isEmpty()) {

        auto newset = set - intersect;

        if (!newset.isEmpty()) {
          i.setValue(newset);
          newSets.append(intersect);
        }

        codes = codes - intersect;

        if (codes.isEmpty()) break;

      }
    }

    if (!newSets.isEmpty()) {
      eqClasses.append(newSets);
    }

    if (!codes.isEmpty()) {
      eqClasses.append(codes);
    }



  };

  void EqClassesVisitor::generateClasses() {

    for (int i = 0; i < eqClasses.size(); i++) {
      auto eqClass = eqClasses[i];
      for (auto glyphId : eqClass) {
        glyphToClass.insert(glyphId, i);
        for (auto iter = eqClassesByGlyphSet.begin(); iter != eqClassesByGlyphSet.end(); iter++) {
          auto glyphSet = iter.key()->getGlyphSet()->getCachedCodes(otlayout);
          if (glyphSet.contains(glyphId)) {
            auto& classes = iter.value();
            classes.insert(i);
          }
        }
      }
    }
  }
}
