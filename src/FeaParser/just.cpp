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

using namespace std;

namespace feayy {



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




    int counter = 0;
    for (auto pass : tableDefinition.passes) {
      if (pass.number() == counter++) {
        FSMSubtable* fsm = new FSMSubtable(lookup);
        fsm->name = QString::fromStdString(tableDefinition.name) + QString("%1").arg(pass.number());
        fsm->dfa = pass.computeDFA(*otlayout);
        lookup->subtables.append(fsm);


      }
    }

    if (!insideLookup && lookup->subtables.count() != 0) {
      otlayout->addTable(lookup);
    }
  }



  DFA Pass::computeDFA(OtLayout& otlayout) {

    struct State {
      std::set<RuleRegExpLeaf*> positions;
      bool marked = false;
      std::map<uint16_t, int> transtitions;

      int number = 0;

      bool operator <(const State& pt) const
      {
        return positions < pt.positions;
      }
    };

    DFA dfa{};

    bool firstBackupsequence = true;
    std::vector<std::shared_ptr<RuleRegExp>> regexVec;
    std::vector<int> rulelengths;
    std::map<GraphiteRule*, int> idRules;
    int idRule = 1;
    for (auto& rule : this->rules) {
      idRules[&rule] = idRule++;
      std::shared_ptr<RuleRegExp> backupRegExp;
      int minbackupSize = std::numeric_limits<int>::max();
      int maxbackupSize = 0;
      if (rule.lhs != nullptr) {

        std::vector<BackupSequence> sequences;
        rule.lhs->setBackupSequence(sequences);

        for (auto& seq : sequences) {
          if (seq.glyphSetNumber > maxbackupSize) {
            maxbackupSize = seq.glyphSetNumber;
          }
          if (seq.glyphSetNumber < minbackupSize) {
            minbackupSize = seq.glyphSetNumber;
          }
        }
        if (firstBackupsequence) {
          dfa.minBackup = minbackupSize;
          firstBackupsequence = false;
        }
        else {
          if (minbackupSize < dfa.minBackup) {
            dfa.minBackup = minbackupSize;
          }
        }
        if (maxbackupSize > dfa.maxBackup) {
          dfa.maxBackup = maxbackupSize;
        }

        for (auto& seq : sequences) {
          std::shared_ptr<RuleRegExp> addedAny;
          for (auto& elem : seq.sequence) {
            if (addedAny == nullptr) {
              //addedAny = elem->clone();
              addedAny = elem;
            }
            else {
              //addedAny = make_shared<RuleRegExpConcat>(addedAny, elem->clone());
              addedAny = make_shared<RuleRegExpConcat>(addedAny, elem);
            }
          }

          for (int i = seq.glyphSetNumber; i < maxbackupSize; i++) {
            if (addedAny == nullptr) {
              addedAny = make_shared<RuleRegExpANY>();
            }
            else {
              addedAny = make_shared<RuleRegExpConcat>(make_shared<RuleRegExpANY>(), addedAny);
            }
          }
          if (addedAny != nullptr) {
            if (backupRegExp == nullptr) {
              backupRegExp = addedAny;
            }
            else {
              backupRegExp = make_shared<RuleRegExpOr>(backupRegExp, addedAny);
            }
          }

        }


      }
      if (rule.ctx != nullptr) {
        rule.ctx->setParentRule(&rule);

        std::shared_ptr<RuleRegExp> newRule = rule.ctx;
        if (backupRegExp != nullptr) {
          newRule = make_shared<RuleRegExpConcat>(backupRegExp, rule.ctx);
        }
        newRule->setParentRule(&rule);
        regexVec.push_back(newRule);
        rulelengths.push_back(maxbackupSize);

      }
    }
    std::shared_ptr<RuleRegExp> globalRe;
    for (std::size_t i = 0; i < regexVec.size(); ++i) {
      auto marker = make_shared<RuleRegExpEndMarker>();
      marker->setParentRule(regexVec[i]->parentRule());
      std::shared_ptr<RuleRegExp> addedAny = make_shared<RuleRegExpConcat>(regexVec[i], marker);
      for (int j = rulelengths[i]; j < dfa.maxBackup; j++) {
        addedAny = make_shared<RuleRegExpConcat>(make_shared<RuleRegExpANY>(), addedAny);
      }

      if (globalRe == nullptr) {
        globalRe = addedAny;
      }
      else {
        globalRe = make_shared<RuleRegExpOr>(globalRe, addedAny);
      }
    }


    if (globalRe == nullptr) return dfa;

    globalRe->computeFollowpos();

    int stateNumber = 0;


    State initalState{ globalRe->firstpos(),false,{},stateNumber++ };
    dfa.states.push_back({});

    std::set<State> dstates;

    dstates.insert(initalState);

    auto computeNextSate = [&dfa = dfa, &stateNumber = stateNumber, &dstates = dstates](State& currentState, State& nexSate, uint16_t glyph) {

      if (nexSate.positions.empty()) return;

      // Test if exists multiple actions with the same rule. If yes disable those actions and generate warning (Should stop)			
      // exampe not yet supported :  (a {Ac_a_1} | b {Ac_b_1} )* a {Ac_a_2} b {Ac_b_2} 			
      // for input sequence ab the actions are a{Ac_a_2} b{Ac_b_2} 
      // for input sequence abab the actions are a{Ac_a_1}b{Ac_b_1} a{Ac_a_2} b{Ac_b_2} 
      // thus for the first characters a  we have to wait for the third character to
      // know if we should execute Ac_a_1 or Ac_a_2.
      // To deal with such situation we have to simulate an NFA automaton where the actions are separated in 
      // different states (probability with empty transation to support such re :  (a {action1} | b)* {action2} c

      std::map<GraphiteRule*, std::set<RuleRegExpAction*>> map;

      for (auto leaf : nexSate.positions) {
        if (auto dd = dynamic_cast<RuleRegExpAction*>(leaf)) {
          map[leaf->parentRule()].insert(dd);
        }
      }

      for (auto& m : map) {
        if (m.second.size() > 1) {
          cout << "many actions in the same state: " << '\n';
          throw new std::runtime_error("many actions in the same state");
          for (auto action : m.second) {
            cout << "action '" << action->name() << "' disabled" << '\n';
            action->disable();
          }
        }
      }

      const auto& tt = dstates.find(nexSate);
      if (tt == dstates.end()) {
        nexSate.number = stateNumber++;
        dstates.insert(nexSate);
        dfa.states.push_back({});

        currentState.transtitions.insert({ glyph, nexSate.number });
        dfa.states[currentState.number].transtitions.insert({ glyph, nexSate.number });
      }
      else {
        currentState.transtitions.insert({ glyph, (*tt).number });
        dfa.states[currentState.number].transtitions.insert({ glyph, (*tt).number });

      }
    };

    while (true) {
      State* currentState = nullptr;
      for (auto& state : dstates) {
        if (!state.marked) {
          currentState = (State*)&state;
          break;
        }
      }

      //All states are marked Terminate
      if (currentState == nullptr) break;

      currentState->marked = true;


      State nextStateAnyPos;
      for (auto pos : currentState->positions) {
        if (dynamic_cast<RuleRegExpANY*>(pos)) {
          auto followpos = pos->followpos();
          nextStateAnyPos.positions.insert(followpos.begin(), followpos.end());
        }
      }

      computeNextSate(*currentState, nextStateAnyPos, 0xFFFF);

      for (auto& glyph : otlayout.glyphs) {
        State nextState;
        for (auto pos : currentState->positions) {
          auto glyphSet = pos->getGlyphSet();
          if (glyphSet != nullptr) {
            auto codes = glyphSet->getCodes(&otlayout);
            if (codes.contains(glyph.charcode)) {
              auto followpos = pos->followpos();
              nextState.positions.insert(followpos.begin(), followpos.end());
              nextState.positions.insert(nextStateAnyPos.positions.begin(), nextStateAnyPos.positions.end());
            }
          }
        }
        computeNextSate(*currentState, nextState, glyph.charcode);
      }
    }
    int stateIndex = 0;
    int totalresetPosition = 0;
    for (auto& state : dstates) {
      for (auto pos : state.positions) {
        if (auto d = dynamic_cast<RuleRegExpAction*>(pos)) {
          std::vector<RuleRegExpAction*> actions;

          dfa.states[state.number].actions[idRules[d->parentRule()]].push_back({ d->name() });



        }
        else if (auto d = dynamic_cast<RuleRegExpEndMarker*>(pos)) {
          if (dfa.states[state.number].final != 0) {
            cout << "Cannot have two end markers on the same state" << '\n';
            throw new std::runtime_error("Cannot have two end markers on the same state");

          }
          dfa.states[state.number].final = idRules[d->parentRule()];
        }
        else if (auto d = dynamic_cast<RuleRegExpPosition*>(pos)) {
          dfa.states[state.number].resetPosition = true;
          totalresetPosition++;
        }
      }
    }

    if (totalresetPosition > 1) {
      throw new std::runtime_error("Cannot have more than one reset position");
    }

    for (int i = dfa.minBackup; i <= dfa.maxBackup; i++) {
      dfa.backupStates.push_back(dfa.maxBackup - i);
    }

    std::cout << "*************************************************************************" << '\n';
    for (auto& state : dstates) {
      std::cout << state.number << '\n';
      std::cout << " " << "transitions:" << '\n';
      for (auto& trans : state.transtitions) {
        if (trans.first != 0xFFFF) {
          auto glyphName = otlayout.glyphNamePerCode.value(trans.first);
          std::cout << "   " << glyphName.toStdString() << " > " << trans.second << '\n';
        }
        else {
          std::cout << "   " << "ANY" << " > " << trans.second << '\n';
        }

      }
      std::cout << " " << "positions:" << '\n';
      for (auto& pos : state.positions) {
        if (auto d = dynamic_cast<RuleRegExpEndMarker*>(pos)) {
          std::cout << "    Final state" << '\n';
        }
        else if (auto d = dynamic_cast<RuleRegExpAction*>(pos)) {
          std::cout << "    Action : " << d->name() << '\n';
        }
        else if (auto d = dynamic_cast<RuleRegExpPosition*>(pos)) {
          std::cout << "    setPosition  " << '\n';
        }
        else if (auto d = dynamic_cast<RuleRegExpGlyphSet*>(pos)) {
          auto glyphSet = d->getGlyphSet();
          auto codes = glyphSet->getCodes(&otlayout);
          std::cout << "    Glyphset : ";
          for (auto code : codes) {
            auto glyphName = otlayout.glyphNamePerCode.value(code);
            std::cout << "   " << glyphName.toStdString();
          }
          std::cout << '\n';
        }

      }
    }

    return dfa;
  }
}
