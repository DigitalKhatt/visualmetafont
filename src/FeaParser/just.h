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

#ifndef H_FEA_JUST
#define H_FEA_JUST


#include "statement.h"
#include <set>
#include<memory>

#include <vector>
#include <map>
#include <string>





namespace feayy {


  class GraphiteRule;
  class RuleRegExp;
  class GlyphSet;
  class RuleRegExpConcat;
  class RuleRegExpLeaf;
  class RuleRegExpTag;
  class RuleRegExpGlyphSet;
  class RuleRegExpSymbol;


  class RuleRegExpVisitor;


  struct BackupSequence {
    std::vector<std::shared_ptr<RuleRegExpLeaf>> sequence;
    int glyphSetNumber = 0;
  };


  using ASTPositions = std::map< RuleRegExpSymbol*, std::vector<RuleRegExpTag*>>;

  //using UP_RuleRegExp = unique_ptr<RuleRegExp>;
  using PRuleRegExp = std::shared_ptr<RuleRegExp>;


  class Pass {
    friend class TableDefinition;
    friend class LookupDefinitionVisitor;
  public:

    Pass()
      : number_{ 0 } {}

    int number() const {
      return number_;
    }

    void setNumber(int number) {
      number_ = number;
    }

    void addRule(const GraphiteRule& rule) {
      rules.push_back(rule);
    }

    void addRule(GraphiteRule&& rule) {
      rules.push_back(std::move(rule));
    }

    Pass(int number)
      : number_{ number } {}

    DFA computeDFA(OtLayout& layout);


  protected:
    int number_;
    std::vector<GraphiteRule>	rules;
  };

  class TableDefinition : public LookupStatement {
    friend class LookupDefinitionVisitor;
  public:
    TableDefinition(std::string name)
      : name{ name }
    {}



    void updatePass(Pass&& pass) {
      int passNumber = pass.number();

      if (passNumber < 0)
      {
        throw new std::runtime_error("invalid pass number");
      }

      passes.push_back(pass);



    }


    void accept(Visitor&) override;

    std::string name;

  protected:

    std::vector<Pass> passes;
  };



  class GraphiteRule {
    friend class Pass;
  public:
    GraphiteRule() {};
    GraphiteRule(PRuleRegExp lhs, PRuleRegExp rhs, PRuleRegExp ctx);



  protected:
    PRuleRegExp lhs;
    PRuleRegExp rhs;
    PRuleRegExp ctx;
  };

  class RuleRegExpLeaf;

  class RuleRegExp {
    friend class GraphiteRule;
  public:

    static int LastPosition;

    virtual ~RuleRegExp()
    {
    }

    virtual ASTPositions firstpos() = 0;
    virtual ASTPositions lastpos() = 0;
    virtual std::vector<RuleRegExpTag*> emptymatch() = 0;

    virtual bool nullable() = 0;
    virtual void computeFollowpos() {};
    virtual void setBackupSequence(std::vector<BackupSequence>& sequences) = 0;

    GraphiteRule* parentRule() {
      return parentRule_;
    }

    virtual void setParentRule(GraphiteRule* rule) = 0;
    virtual std::shared_ptr<RuleRegExp> clone() = 0;

    virtual void accept(RuleRegExpVisitor& v) = 0;

    std::shared_ptr<RuleRegExp>  makeRepetition(int min, int max);


  protected:
    GraphiteRule* parentRule_ = nullptr;

  };



  class RuleRegExpLeaf : public RuleRegExp, public std::enable_shared_from_this<RuleRegExpLeaf> {
  public:

    void setBackupSequence(std::vector<BackupSequence>& sequences) override;

    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
    }


  };

  class RuleRegExpSymbol : public RuleRegExpLeaf, public std::enable_shared_from_this<RuleRegExpSymbol> {
  public:
    void addFollowpos(const ASTPositions& positions) {
      followpos_.insert(positions.begin(), positions.end());
    }

    const ASTPositions& followpos() {
      return followpos_;
    }

    bool nullable() override {
      return false;
    }

    virtual GlyphSet* getGlyphSet() {
      return nullptr;
    }

    ASTPositions firstpos() override {
      return { {this,{}} };
    }
    ASTPositions lastpos() override {
      return { {this,{}} };
    }

    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
    }

    std::vector<RuleRegExpTag*> emptymatch() {
      return {};
    }

    int positionNumber = 0;

    void computeFollowpos() override {
      LastPosition++;
      positionNumber = LastPosition;
    }


  protected:
    ASTPositions followpos_;
  };

  class RuleRegExpTag : public RuleRegExpLeaf {
  public:

    bool nullable() override {
      return true;
    }

    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
    }

    std::vector<RuleRegExpTag*> emptymatch() {
      return { this };
    }

    ASTPositions firstpos() override {
      return {};
    }
    ASTPositions lastpos() override {
      return {};
    }
  };

  class RuleRegExpEndMarker : public RuleRegExpSymbol {
  public:
    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpEndMarker>();
    }
    virtual void accept(RuleRegExpVisitor&);
  };

  class RuleRegExpANY : public RuleRegExpSymbol {
  public:
    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpANY>();
    }
    virtual void accept(RuleRegExpVisitor&);

  };

  enum class RuleRegExpActionType {
    LOOKUP,
    ACTION,
    STARTNEWMATCH
  };

  class RuleRegExpAction : public RuleRegExpTag {
  public:

    explicit RuleRegExpAction(std::string name, RuleRegExpActionType type) : _name{ name }, type{ type }, position{ 0 } {}

    explicit RuleRegExpAction(std::string name, RuleRegExpActionType type, int position) : _name{ name }, type{ type }, position{ position } {}

    virtual void accept(RuleRegExpVisitor&);


    std::string name() {
      return _name;
    }

    RuleRegExpActionType getType() {
      return type;
    }

    void disable() { disabled_ = true; }

    bool disabled() { return disabled_; }

    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpAction>(_name, type);
    }

  private:
    std::string _name;
    bool disabled_ = false;
    RuleRegExpActionType type;
    int position = 0;
  };

  class RuleRegExpGlyphSet : public RuleRegExpSymbol {
  public:

    explicit RuleRegExpGlyphSet(GlyphSet* element) : element{ element } {
    }

    virtual void accept(RuleRegExpVisitor&);

    virtual ~RuleRegExpGlyphSet()
    {
      delete element;
    }

    GlyphSet* getGlyphSet() override {
      return element;
    }
    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
    }

    std::shared_ptr<RuleRegExp> clone() override;

  protected:
    GlyphSet* element;


  };

  class RuleRegExpConcat : public RuleRegExp {
  public:

    RuleRegExpConcat(PRuleRegExp left, PRuleRegExp right) : left{ left }, right{ right } {
    }


    virtual void accept(RuleRegExpVisitor&);

    virtual void setBackupSequence(std::vector<BackupSequence>& sequences) override {
      left->setBackupSequence(sequences);
      right->setBackupSequence(sequences);
    }

    bool nullable() override {
      return this->left->nullable() && this->right->nullable();
    }

    ASTPositions firstpos() override {
      auto ret = this->left->firstpos();
      if (this->left->nullable()) {
        auto emptymatch = this->left->emptymatch();
        auto right = this->right->firstpos();
        for (auto& t : right) {
          t.second.insert(t.second.begin(), emptymatch.begin(), emptymatch.end());
          ret.insert(t);
        }
      }
      return ret;
    }
    ASTPositions lastpos() override {
      auto ret = this->right->lastpos();
      if (this->right->nullable()) {
        auto emptymatch = this->right->emptymatch();
        auto left = this->left->lastpos();
        for (auto& t : left) {
          t.second.insert(t.second.end(), emptymatch.begin(), emptymatch.end());
          ret.insert(t);
        }
      }
      return ret;
    }

    void computeFollowpos() override {
      left->computeFollowpos();
      right->computeFollowpos();
      auto rightPosition = right->firstpos();
      for (auto leaf : left->lastpos()) {

        if (leaf.second.size() != 0) {
          ASTPositions newPos;
          for (auto pos : rightPosition) {
            std::vector<RuleRegExpTag*> newTags;
            newTags.insert(newTags.begin(), leaf.second.begin(), leaf.second.end());
            newTags.insert(newTags.end(), pos.second.begin(), pos.second.end());
            newPos.insert({ pos.first,newTags });
          }
          leaf.first->addFollowpos(std::move(newPos));
        }
        else {
          leaf.first->addFollowpos(rightPosition);
        }
      }
    }

    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
      left->setParentRule(rule);
      right->setParentRule(rule);
    }

    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpConcat>(left->clone(), right->clone());
    }

    RuleRegExp& lhs() { return *left; }
    RuleRegExp& rhs() { return *right; }

    std::vector<RuleRegExpTag*> emptymatch() override {
      std::set<RuleRegExpTag*> ret;
      auto left = this->left->emptymatch();
      auto right = this->right->emptymatch();

      left.insert(left.end(), right.begin(), right.end());
      return left;
    }

  protected:
    PRuleRegExp left;
    PRuleRegExp right;

  };

  class RuleRegExpOr : public RuleRegExp {
  public:
    explicit RuleRegExpOr(PRuleRegExp left, PRuleRegExp right) : left{ left }, right{ right } {
    }

    virtual void accept(RuleRegExpVisitor&);

    bool nullable() override {
      return this->left->nullable() || this->right->nullable();
    }

    ASTPositions firstpos() override {
      auto left = this->left->firstpos();
      auto right = this->right->firstpos();
      left.insert(right.begin(), right.end());
      return left;
    }
    ASTPositions lastpos() override {
      auto left = this->left->lastpos();
      auto right = this->right->lastpos();
      left.insert(right.begin(), right.end());
      return left;
    }
    void computeFollowpos() override {
      left->computeFollowpos();
      right->computeFollowpos();
    }

    virtual void setBackupSequence(std::vector<BackupSequence>& sequences) override {


      std::vector<BackupSequence> copy2 = sequences;

      right->setBackupSequence(copy2);

      left->setBackupSequence(sequences);

      if (!copy2.empty()) {
        for (auto& seq : copy2) {
          sequences.push_back(seq);
        }
      }
      else {
        sequences.push_back({ {},0 });
      }
    }
    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
      left->setParentRule(rule);
      right->setParentRule(rule);
    }

    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpOr>(left->clone(), right->clone());
    }

    std::vector<RuleRegExpTag*> emptymatch() {
      if (left->nullable()) {
        return left->emptymatch();
      }
      else {
        return right->emptymatch();
      }
    }

    RuleRegExp& lhs() { return *left; }
    RuleRegExp& rhs() { return *right; }

  protected:
    PRuleRegExp left;
    PRuleRegExp right;
  };

  enum class RuleRegExpRepeatType {
    STAR,
    PLUS,
    OPT,
  };

  class RuleRegExpRepeat : public RuleRegExp {

  public:
    explicit RuleRegExpRepeat(PRuleRegExp regex, RuleRegExpRepeatType repeatition) : regex{ regex }, repeatition{ repeatition } {
    }

    virtual void accept(RuleRegExpVisitor&);

    bool nullable() override {
      return this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::OPT;
    }

    ASTPositions firstpos() override {
      return this->regex->firstpos();
    }
    ASTPositions lastpos() override {
      return this->regex->lastpos();
    }
    void computeFollowpos() override {
      regex->computeFollowpos();
      if (this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::PLUS) {
        
        auto rightPosition = this->firstpos();
        for (auto leaf : this->lastpos()) {
          if (leaf.second.size() != 0) {
            ASTPositions newPos;
            for (auto pos : rightPosition) {
              std::vector<RuleRegExpTag*> newTags;
              newTags.insert(newTags.begin(), leaf.second.begin(), leaf.second.end());
              newTags.insert(newTags.end(), pos.second.begin(), pos.second.end());
              newPos.insert({ pos.first,newTags });
            }
            leaf.first->addFollowpos(std::move(newPos));
          }
          else {
            leaf.first->addFollowpos(rightPosition);
          }          
        }
      }
    }

    virtual void setBackupSequence(std::vector<BackupSequence>& sequences) override {

      if (this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::PLUS) {
        throw std::runtime_error("Backup sequence cannot contain Kleene star");
      }
      else {
        std::vector<BackupSequence> copy2 = sequences;
        regex->setBackupSequence(sequences);
        if (!copy2.empty()) {
          for (auto& seq : copy2) {
            sequences.push_back(seq);
          }
        }
        else {
          sequences.push_back({ {},0 });
        }


      }
    }

    std::vector<RuleRegExpTag*> emptymatch() {
      if (regex->nullable()) {
        return regex->emptymatch();
      }
      else {
        return {};
      }
    }

    void setParentRule(GraphiteRule* rule) override {
      parentRule_ = rule;
      regex->setParentRule(rule);
    }

    std::shared_ptr<RuleRegExp> clone() override {
      return std::make_shared<RuleRegExpRepeat>(regex->clone(), repeatition);
    }

    RuleRegExp& innerExpr() { return *regex; }


  protected:
    PRuleRegExp regex;
    RuleRegExpRepeatType repeatition;

  };

  struct JustTable : Statement {
    struct JustStep {
      std::vector<std::string> lookupNames;
      JustStep() = default;
      JustStep(std::vector<std::string> lookupNames) : lookupNames{ lookupNames } {};
    };

    JustTable() = default;
    JustTable(std::vector<JustStep> stretchRules, std::vector<JustStep> shrinkRules, std::vector<std::string> aftergsub) : stretchRules{ stretchRules }, shrinkRules{ shrinkRules }, aftergsub{ aftergsub } {}

    std::vector<JustStep> stretchRules;
    std::vector<JustStep> shrinkRules;
    std::vector<std::string> aftergsub;

    void accept(Visitor&) override;
  };

  class RuleRegExpVisitor {
  public:

    virtual void accept(RuleRegExpConcat&) = 0;
    virtual void accept(RuleRegExpRepeat&) = 0;
    virtual void accept(RuleRegExpOr&) = 0;
    virtual void accept(RuleRegExpANY&) = 0;
    virtual void accept(RuleRegExpEndMarker&) = 0;
    virtual void accept(RuleRegExpAction&) = 0;
    virtual void accept(RuleRegExpGlyphSet&) = 0;
  };

  class EqClassesVisitor : public RuleRegExpVisitor {
  public:
    friend class Pass;
    EqClassesVisitor(OtLayout* otlayout) : otlayout{ otlayout } {};
    void accept(RuleRegExpConcat&) override;
    void accept(RuleRegExpRepeat&) override;
    void accept(RuleRegExpOr&) override;
    void accept(RuleRegExpANY&) override;
    void accept(RuleRegExpEndMarker&) override;
    void accept(RuleRegExpAction&) override;
    void accept(RuleRegExpGlyphSet&) override;

    void generateClasses();

  private:

    OtLayout* otlayout;
    QVector<QSet<quint16>> eqClasses;
    QMap<RuleRegExpSymbol*, QSet<int>> eqClassesByGlyphSet;
    QMap<quint16, quint16> glyphToClass;

  };

}
#endif // H_FEA_JUST
