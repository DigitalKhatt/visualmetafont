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

  int RuleRegExp::LastPosition;

  struct StateConfiguration {
    std::vector<RuleRegExpTag*> currentTags;
    std::vector<RuleRegExpTag*> lookaheadTags;
    RuleRegExpSymbol* origin;
  };

  struct BackTrackInfo {
    int index;
    std::vector<RuleRegExpTag*> tags;
  };

  struct Transition {
    int nextState;
    std::vector<BackTrackInfo> back;

  };

  struct OriginIndex {
    std::vector<RuleRegExpTag*> tags;
    int index;
  };

  using StatePositions = std::map < RuleRegExpSymbol*, std::pair<std::vector<RuleRegExpTag*>, RuleRegExpSymbol*>>;

  struct TDFAConstState {
    StatePositions positions;
    std::map<RuleRegExpSymbol*, StateConfiguration> posStates;
    bool marked = false;
    std::map<uint16_t, Transition> transtitions;
    std::map<RuleRegExpSymbol*, OriginIndex> origins;
    std::vector<RuleRegExpSymbol*> originIndexes;

    int number = 0;

    BackTrackInfo finalBackTrack;
    RuleRegExpSymbol* final = nullptr;

    bool operator <(const TDFAConstState& pt) const
    {
      return positions < pt.positions;
    }

    void addPositions(const ASTPositions& positions, std::vector<RuleRegExpTag*> currentTags, RuleRegExpSymbol* origin) {
      for (auto& pos : positions) {
        auto old = this->posStates.find(pos.first);
        if (old != this->posStates.end()) {
          if (old->second.origin->positionNumber > origin->positionNumber) {
            this->positions.erase(old->first);
            this->posStates.erase(old);
            //TODO : Origin always unique ?
            this->positions.insert({ pos.first,{pos.second,nullptr} });
            this->posStates.insert({ pos.first,{currentTags,pos.second,origin } });
          }
        }
        else {
          //this->positions.insert(pos);
          this->positions.insert({ pos.first,{pos.second,nullptr} });
          this->posStates.insert({ pos.first,{currentTags,pos.second,origin } });
        }
      }
    }

    void computeUniques() {
      origins.clear();
      originIndexes.clear();

      for (auto& pos : this->posStates) {
        if (origins.find(pos.second.origin) == origins.end()) {
          origins[pos.second.origin] = { pos.second.currentTags,(int)originIndexes.size() };
          originIndexes.push_back(pos.second.origin);

        }
      }
    }

    void populateUniqueTags() {

      for (auto& pos : this->posStates) {
        origins[pos.second.origin].tags = pos.second.currentTags;
      }
    }
  };

  static void printDFA(const DFA& dfa, OtLayout& otlayout, const std::map<GraphiteRule*, int>& idRules, const std::set<TDFAConstState>& dstates) {

    std::cout << "***********************************START**************************************" << '\n';
    std::cout << "classes:";

    for (int classIndice = 0; classIndice < dfa.eqClasses.size(); classIndice++) {
      std::cout << '\n' << " " << classIndice << " : ";
      for (auto& state : dfa.eqClasses[classIndice]) {
        auto glyphName = otlayout.glyphNamePerCode.value(state);
        std::cout << glyphName.toStdString() << ", ";
      }
    }
    std::cout << '\n';

    std::map<int, TDFAConstState> sortedstates;

    std::map<int, RuleRegExpSymbol*> allPositions;

    for (auto& state : dstates) {
      sortedstates.insert({ state.number,state });
    }

    for (auto& pair : sortedstates) {
      auto& state = pair.second;
      std::cout << "State " << state.number;
      for (int i = 0; i < dfa.backupStates.size(); i++) {
        if (dfa.backupStates[i] == state.number) {
          std::cout << " ( backup Indice = " << i << ")";
          break;
        }
      }
      if (state.final) {
        std::cout << " ( final from rule " << idRules.at(state.final->parentRule()) << ")\n";
        std::cout << " " << "back:";
        std::cout << "(";
        //if (state.finalBackTrack.prevPos) {
          //std::cout << state.finalBackTrack.prevPos->positionNumber;
        std::cout << state.finalBackTrack.index;
        //}
        std::cout << ",";
        for (auto& tag : state.finalBackTrack.tags) {
          if (auto d = dynamic_cast<RuleRegExpAction*>(tag)) {
            std::cout << d->name() << " ";
          }
        }
        std::cout << ")";
      }
      std::cout << '\n';

      std::cout << " " << "transitions:" << '\n';
      for (auto& trans : state.transtitions) {
        if (trans.first == 0xFFFF) {
          std::cout << "   " << "ANY" << " > State " << trans.second.nextState << '\n';
        }
        else if (trans.first == 0xFFFE) {
          std::cout << "   " << "ANYACTION" << " > State " << trans.second.nextState << '\n';
        }
        else {
          //auto glyphName = otlayout.glyphNamePerCode.value(trans.first);
          //std::cout << "   " << glyphName.toStdString() << " > " << trans.second << '\n';
          std::cout << "   Class " << trans.first << " > State " << trans.second.nextState << '\n';

        }
        if (trans.second.back.size() > 0) {
          std::cout << "   {";
          for (auto& back : trans.second.back) {
            std::cout << "(";
            //if (back.second.prevPos) {
            //  std::cout << back.second.prevPos->positionNumber;
            //}
            std::cout << back.index;
            std::cout << ",";
            for (auto& tag : back.tags) {
              if (auto d = dynamic_cast<RuleRegExpAction*>(tag)) {
                std::cout << d->name() << " ";
              }
            }
            std::cout << ")";
          }
          std::cout << "}\n";
        }


      }
      std::cout << " " << "positions:" << '\n';

      std::vector<std::pair < RuleRegExpSymbol*, std::pair<std::vector<RuleRegExpTag*>, RuleRegExpSymbol*>>> sortedPositions;

      for (auto& posr : state.positions) {
        sortedPositions.push_back(posr);
        allPositions.insert({ posr.first->positionNumber,posr.first });
      }
      std::sort(sortedPositions.begin(),
        sortedPositions.end(),
        [](const auto& left, const auto& right) {return left.first->positionNumber < right.first->positionNumber; });

      for (auto& posr : sortedPositions) {
        auto pos = posr.first;
        std::cout << "    Position No " << pos->positionNumber;
        std::cout << ",tags={";
        for (auto it = posr.second.first.begin(); it != posr.second.first.end(); ++it) {
          if (auto lookup = dynamic_cast<RuleRegExpAction*>(*it)) {
            std::cout << lookup->name();
          }
          if (std::next(it) != posr.second.first.end()) {
            std::cout << ", ";
          }
        }
        std::cout << "}";
        if (state.posStates[pos].origin) {
          std::cout << ",origin:{posNumber:" << state.posStates[pos].origin->positionNumber << ",index:" << state.origins[state.posStates[pos].origin].index << "}";
        }
        if (posr.second.second) {
          std::cout << ",originnullptr:{" << posr.second.second << "}";
        }
        /*
        if (state.origins[state.posStates[pos].origin].tags.size() > 0) {
          std::cout << ",origintags={";
          for (auto it = state.origins[state.posStates[pos].origin].tags.begin(); it != state.origins[state.posStates[pos].origin].tags.end(); ++it) {
            if (auto lookup = dynamic_cast<RuleRegExpAction*>(*it)) {
              std::cout << lookup->name();
            }
            if (std::next(it) != state.origins[state.posStates[pos].origin].tags.end()) {
              std::cout << ", ";
            }
          }

          std::cout << "}";
        }*/

        if (auto d = dynamic_cast<RuleRegExpEndMarker*>(pos)) {
          std::cout << ", Final state(" << idRules.at(pos->parentRule()) << ")";
        }
        else if (auto d = dynamic_cast<RuleRegExpGlyphSet*>(pos)) {
          auto glyphSet = d->getGlyphSet();
          auto codes = glyphSet->getCachedCodes(&otlayout);
          /*
          std::cout << ", Glyphset : ";;
          for (auto code : codes) {
            auto glyphName = otlayout.glyphNamePerCode.value(code);
            std::cout << "   " << glyphName.toStdString();
          }*/

        }

        std::cout << '\n';

      }
    }
    for (auto pos : allPositions) {
      std::cout << "Followers of position No " << pos.first << std::endl;
      for (auto follow : pos.second->followpos()) {
        std::cout << "  Position No " << follow.first->positionNumber;
        if (follow.second.size() > 0) {
          std::cout << " {";
          for (auto it = follow.second.begin(); it != follow.second.end(); ++it) {
            if (auto lookup = dynamic_cast<RuleRegExpAction*>(*it)) {
              std::cout << lookup->name();
            }
            if (std::next(it) != follow.second.end()) {
              std::cout << ", ";
            }
          }

          std::cout << "}";
        }
        std::wcout << std::endl;
      }
    }
    std::cout << "*****************************END********************************************" << '\n';
  }

  DFA Pass::computeDFA(OtLayout& otlayout) {

    if (this->rules.size() == 0) return {};

    auto concatReg = [](std::shared_ptr<RuleRegExp> first, std::shared_ptr<RuleRegExp> last) {
      if (first == nullptr) {
        return last;
      }
      else if (last == nullptr) {
        return first;

      }
      else {
        std::shared_ptr<RuleRegExp> ret = make_shared<RuleRegExpConcat>(first, last);
        return ret;
      }
    };

    auto orReg = [](std::shared_ptr<RuleRegExp> first, std::shared_ptr<RuleRegExp> last) {
      if (first == nullptr) {
        return last;
      }
      else if (last == nullptr) {
        return first;

      }
      else {
        std::shared_ptr<RuleRegExp> ret = make_shared<RuleRegExpOr>(first, last);
        return ret;
      }
    };

    DFA dfa{};

    bool firstBackupsequence = true;
    std::vector<std::shared_ptr<RuleRegExp>> regexVec;
    std::vector<int> rulelengths;
    std::map<GraphiteRule*, int> idRules;
    int idRule = 1;

    std::map<GraphiteRule*, std::map<int, std::vector<std::vector<std::shared_ptr<RuleRegExpLeaf>>>>> backupRegExprs;
    int minbackupSize = std::numeric_limits<int>::max();
    int maxbackupSize = 0;

    for (auto& rule : this->rules) {

      if (rule.lhs == nullptr && rule.ctx == nullptr) {
        throw new std::runtime_error("Invalid empty rule");
      }

      idRules[&rule] = idRule++;
      auto& backupRules = backupRegExprs[&rule];
      if (rule.lhs != nullptr) {

        std::vector<BackupSequence> sequences;
        rule.lhs->setBackupSequence(sequences);
        for (auto& seq : sequences) {
          minbackupSize = std::min(minbackupSize, seq.glyphSetNumber);
          maxbackupSize = std::max(maxbackupSize, seq.glyphSetNumber);
          auto tt = backupRules.find(seq.glyphSetNumber);
          if (tt != backupRules.end()) {
            tt->second.push_back(std::move(seq.sequence));
          }
          else {
            backupRules.insert({ seq.glyphSetNumber,{std::move(seq.sequence)} });
          }
        }
      }
      else if (rule.ctx != nullptr) {
        minbackupSize = 0;
        backupRules.insert({ 0,{{}} });
      }

      rule.ctx = concatReg(rule.ctx, make_shared<RuleRegExpEndMarker>());

    }

    std::map < GraphiteRule*, std::shared_ptr<RuleRegExp>> prevRegExps;

    std::map <int, std::shared_ptr<RuleRegExp>> globalRes;

    for (int backupIndex = minbackupSize; backupIndex <= maxbackupSize; backupIndex++) {
      for (auto& rule : this->rules) {
        auto& backupRules = backupRegExprs[&rule];
        std::shared_ptr<RuleRegExp> backTrackSeq;
        auto prevRegExp = prevRegExps[&rule];
        if (backupRules.find(backupIndex) != backupRules.end()) {
          auto& sequences = backupRules[backupIndex];
          for (auto& seq : sequences) {
            std::shared_ptr<RuleRegExp> currentSeq;
            for (auto& elem : seq) {
              currentSeq = concatReg(currentSeq, elem);
            }
            backTrackSeq = orReg(backTrackSeq, currentSeq);
          }
          backTrackSeq = concatReg(backTrackSeq, rule.ctx);
        }

        if (prevRegExp != nullptr) {
          prevRegExp = concatReg(make_shared<RuleRegExpANY>(), prevRegExp);
        }

        prevRegExp = orReg(prevRegExp, backTrackSeq);

        if (prevRegExp != nullptr) {
          prevRegExp->setParentRule(&rule);
          prevRegExps[&rule] = prevRegExp;
          globalRes[backupIndex] = orReg(globalRes[backupIndex], prevRegExp);
        }
      }
    }

    dfa.minBackup = minbackupSize;
    dfa.maxBackup = maxbackupSize;

    EqClassesVisitor eqClassesVisitor{ &otlayout };
    globalRes[dfa.maxBackup]->accept(eqClassesVisitor);
    eqClassesVisitor.generateClasses();
    dfa.eqClasses = eqClassesVisitor.eqClasses;
    dfa.glyphToClass = eqClassesVisitor.glyphToClass;

    int stateNumber = 0;
    std::set<TDFAConstState> dstates;
    std::stack<std::reference_wrapper<TDFAConstState>> notmarked;
    std::vector<std::vector<int>> uniqueOrigins;

    auto addState = [&dfa = dfa, &stateNumber = stateNumber, &dstates = dstates, &notmarked = notmarked, &idRules = idRules,
      &uniqueOrigins = uniqueOrigins](TDFAConstState& nextState) -> TDFAConstState& {
      auto tt = dstates.find(nextState);

      if (tt == dstates.end()) {
        nextState.number = stateNumber++;

        auto result = dstates.insert(nextState);
        tt = result.first;
        TDFAConstState& res = const_cast<TDFAConstState&>(*tt);
        res.number = nextState.number;
        res.computeUniques();
        notmarked.push(std::ref(res));
        dfa.states.push_back({});

        //test final
        RuleRegExpEndMarker* final = nullptr;

        for (auto pos : res.posStates) {
          auto symbol = pos.first;
          if (auto finalMarker = dynamic_cast<RuleRegExpEndMarker*>(symbol)) {
            if (final) {
              auto idRule = idRules[finalMarker->parentRule()];
              if (idRules[final->parentRule()] > idRule) {
                final = finalMarker;
              }
            }
            else {
              final = finalMarker;
            }
          }
        }

        if (final) {
          dfa.states[res.number].final = idRules[final->parentRule()];
          dfa.states[res.number].backtrackfinal.prevTransIndex = res.origins[res.posStates[final].origin].index;
          for (auto tag : res.posStates[final].lookaheadTags) {
            if (auto action = dynamic_cast<RuleRegExpAction*>(tag)) {
              dfa.states[res.number].backtrackfinal.actions.push_back({});
              DFAAction& dfaAction = dfa.states[res.number].backtrackfinal.actions.back();
              dfaAction.idRule = idRules.at(action->parentRule());
              dfaAction.name = action->name();
              dfaAction.type = static_cast<DFAActionType>(action->getType());

            }
          }

          res.finalBackTrack = { res.origins[res.posStates[final].origin].index,res.posStates[final].lookaheadTags };
          res.final = final;
        }
      }
      else {
        auto& res = const_cast<TDFAConstState&>(*tt);
        res.posStates = nextState.posStates;
        res.populateUniqueTags();
      }

      //TODO
      return const_cast<TDFAConstState&>(*tt);

    };

    auto computeNextSate = [&dfa = dfa, &stateNumber = stateNumber, &dstates = dstates,
      &addState = addState, &idRules = idRules,
      &notmarked = notmarked, &uniqueOrigins = uniqueOrigins]
      (TDFAConstState& currentState, TDFAConstState& newState, int eqClass) {

      if (newState.positions.empty()) return;

      auto& nextState = addState(newState);

      Transition tran{ nextState.number ,{} };
      DFATransOut dfatrans{ nextState.number ,{} };

      for (auto origin : nextState.originIndexes) {
        BackTrackInfo info{ currentState.origins[currentState.posStates[origin].origin].index,nextState.origins[origin].tags };
        tran.back.push_back(info);

        dfatrans.backtracks.push_back({});
        auto& dfaInfo = dfatrans.backtracks.back();

        dfaInfo.prevTransIndex = info.index;

        for (auto tag : info.tags) {
          if (auto action = dynamic_cast<RuleRegExpAction*>(tag)) {
            dfaInfo.actions.push_back({});
            DFAAction& dfaAction = dfaInfo.actions.back();
            dfaAction.idRule = idRules.at(action->parentRule());
            dfaAction.name = action->name();
            dfaAction.type = static_cast<DFAActionType>(action->getType());

          }
        }

      }

      currentState.transtitions.insert({ eqClass, tran });
      dfa.states[currentState.number].transtitions.insert({ eqClass, dfatrans });

    };



    for (int backupLen = dfa.maxBackup; backupLen >= dfa.minBackup; backupLen--) {
      auto& globalRe = globalRes[backupLen];
      RuleRegExp::LastPosition = 0;
      globalRe->computeFollowpos();

      auto initalPositions = globalRe->firstpos();

      TDFAConstState initalState;

      initalState.addPositions(initalPositions, {}, nullptr);

      addState(initalState);

      dfa.backupStates.push_back(initalState.number);


      while (true) {

        //All states are marked Terminate
        if (notmarked.empty()) break;

        TDFAConstState& currentState = notmarked.top().get();
        notmarked.pop();


        TDFAConstState nextStateAnyPos;

        for (auto pos : currentState.positions) {
          if (dynamic_cast<RuleRegExpANY*>(pos.first)) {
            auto followPositions = pos.first->followpos();
            nextStateAnyPos.addPositions(followPositions, pos.second.first, pos.first);
          }
        }

        computeNextSate(currentState, nextStateAnyPos, 0xFFFF);

        std::set<int> totalClasses;
        std::map<int, std::vector <StatePositions::value_type >> posByClass;
        for (auto pos : currentState.positions) {
          auto& classes = eqClassesVisitor.eqClassesByGlyphSet[pos.first];
          //totalClasses.unite(classes);
          totalClasses.insert(classes.begin(), classes.end());
          for (auto it = classes.constBegin(); it != classes.constEnd(); it++) {
            posByClass[*it].push_back(pos);
          }
        }

        for (auto eqClass : totalClasses) {
          TDFAConstState nextState;
          auto& positions = posByClass[eqClass];
          for (auto pos : positions) {
            auto followPositions = pos.first->followpos();
            nextState.addPositions(followPositions, pos.second.first, pos.first);
          }
          for (auto pos : currentState.positions) {
            if (dynamic_cast<RuleRegExpANY*>(pos.first)) {
              auto followPositions = pos.first->followpos();
              nextState.addPositions(followPositions, pos.second.first, pos.first);
            }
          }
          computeNextSate(currentState, nextState, eqClass);
        }
      }
    }

    std::reverse(dfa.backupStates.begin(), dfa.backupStates.end());

    //printDFA(dfa, otlayout, idRules, dstates);


    return dfa;
  }
}
