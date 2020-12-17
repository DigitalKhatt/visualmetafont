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





namespace feayy {


	class GraphiteRule;
	class RuleRegExp;
	class GlyphSet;
	class RuleRegExpConcat;
	class RuleRegExpLeaf;
	class RuleRegExpGlyphSet;

	

	

	struct BackupSequence {
		std::vector<shared_ptr<RuleRegExpLeaf>> sequence;
		int glyphSetNumber = 0;
	};

	//using UP_RuleRegExp = unique_ptr<RuleRegExp>;
	using PRuleRegExp = shared_ptr<RuleRegExp>;


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

		Pass& getPass(int passNumber)
		{

			if (passNumber < 0)
			{
				throw std::runtime_error("invalid pass number");
			}

			while (passNumber >= passes.size())
				passes.push_back({});

			if (passes[passNumber].number() != passNumber)
			{
				passes[passNumber].setNumber(passNumber);
			}

			return passes[passNumber];
		}

		void updatePass(Pass&& pass) {
			int passNumber = pass.number();

			if (passNumber < 0)
			{
				throw new std::runtime_error("invalid pass number");
			}

			while (passNumber >= passes.size())
				passes.push_back({});

			if (passes[passNumber].number() != passNumber)
			{
				passes[passNumber] = std::move(pass);
			}
			else {
				auto currentPass = passes[passNumber];
				for (size_t i = 0; i < pass.rules.size(); ++i) {
					currentPass.addRule(std::move(pass.rules[i]));
				}
			}
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



	class RuleRegExp  {
		friend class GraphiteRule;
	public:

		virtual ~RuleRegExp()
		{
		}

		virtual std::set<RuleRegExpLeaf*> firstpos() = 0;
		virtual std::set<RuleRegExpLeaf*> lastpos() = 0;
		virtual bool nullable() = 0;
		virtual void computeFollowpos() {};
		virtual void setBackupSequence(std::vector<BackupSequence>& sequences) = 0;

		GraphiteRule* parentRule() {
			return parentRule_;
		}

		virtual void setParentRule(GraphiteRule* rule) = 0;
		virtual std::shared_ptr<RuleRegExp> clone() = 0;

	protected:
		GraphiteRule* parentRule_ = nullptr;
	};



	class RuleRegExpLeaf : public RuleRegExp , public std::enable_shared_from_this<RuleRegExpLeaf> {
	public:
		void addFollowpos(const std::set<RuleRegExpLeaf*>& set) {
			followpos_.insert(set.begin(), set.end());
		}

		const std::set<RuleRegExpLeaf*>& followpos() {
			return followpos_;
		}

		bool nullable() override {
			return false;
		}

		virtual GlyphSet* getGlyphSet() {
			return nullptr;
		}

		std::set<RuleRegExpLeaf*> firstpos() override {
			std::set<RuleRegExpLeaf*> temp;
			temp.insert(this);
			return temp;
		}
		std::set<RuleRegExpLeaf*> lastpos() override {
			std::set<RuleRegExpLeaf*> temp;
			temp.insert(this);
			return temp;
		}

		void setBackupSequence(std::vector<BackupSequence>& sequences) override;

		void setParentRule(GraphiteRule* rule) override {
			parentRule_ = rule;
		}


	protected:
		std::set<RuleRegExpLeaf*> followpos_;
	};

	class RuleRegExpEndMarker : public RuleRegExpLeaf {
		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpEndMarker>();
		}
	};

	class RuleRegExpANY : public RuleRegExpLeaf {
		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpANY>();
		}
		
	};

	class RuleRegExpAction : public RuleRegExpLeaf {
	public:

		explicit RuleRegExpAction(std::string name) : _name{ name } {}

		bool nullable() override {
			return true;
		}

		std::string name() {
			return _name;
		}

		void disable() { disabled_ = true; }

		bool disabled() { return disabled_; }

		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpAction>(_name);
		}

	private:
		std::string _name;
		bool disabled_ = false;
	};

	class RuleRegExpPosition : public RuleRegExpLeaf {
	public:

		bool nullable() override {
			return true;
		}

		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpPosition>();
		}
	};

	class RuleRegExpGlyphSet : public RuleRegExpLeaf{
	public:



		explicit RuleRegExpGlyphSet(GlyphSet* element) : element{ element } {
		}

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

		virtual void setBackupSequence(std::vector<BackupSequence>& sequences) override {
			left->setBackupSequence(sequences);
			right->setBackupSequence(sequences);
		}

		bool nullable() override {
			return this->left->nullable() && this->right->nullable();
		}

		std::set<RuleRegExpLeaf*> firstpos() override {
			auto left = this->left->firstpos();
			if (this->left->nullable()) {
				auto right = this->right->firstpos();
				left.insert(right.begin(), right.end());
			}
			return left;
		}
		std::set<RuleRegExpLeaf*> lastpos() override {
			auto right = this->right->lastpos();
			if (this->right->nullable()) {
				auto left = this->left->lastpos();
				right.insert(left.begin(), left.end());
			}
			return right;
		}

		void computeFollowpos() override {
			left->computeFollowpos();
			right->computeFollowpos();
			for (auto leaf : left->lastpos()) {
				leaf->addFollowpos(right->firstpos());
			}
		}

		void setParentRule(GraphiteRule* rule) override {
			parentRule_ = rule;
			left->setParentRule(rule);
			right->setParentRule(rule);
		}

		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpConcat>(left->clone(),right->clone());
		}

	protected:
		PRuleRegExp left;
		PRuleRegExp right;

	};

	class RuleRegExpOr : public RuleRegExp {
	public:
		explicit RuleRegExpOr(PRuleRegExp left, PRuleRegExp right) : left{ left }, right{ right } {
		}

		bool nullable() override {
			return this->left->nullable() || this->right->nullable();
		}

		std::set<RuleRegExpLeaf*> firstpos() override {
			auto left = this->left->firstpos();
			auto right = this->right->firstpos();
			left.insert(right.begin(), right.end());
			return left;
		}
		virtual std::set<RuleRegExpLeaf*> lastpos() override {
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


			vector<BackupSequence> copy2 = sequences;

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
			return make_shared<RuleRegExpOr>(left->clone(), right->clone());
		}

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

		bool nullable() override {
			return this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::OPT;
		}

		std::set<RuleRegExpLeaf*> firstpos() override {
			return this->regex->firstpos();
		}
		std::set<RuleRegExpLeaf*> lastpos() override {
			return this->regex->lastpos();
		}
		void computeFollowpos() override {
			regex->computeFollowpos();
			if (this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::PLUS) {
				for (auto leaf : this->lastpos()) {
					leaf->addFollowpos(this->firstpos());
				}
			}
		}

		virtual void setBackupSequence(std::vector<BackupSequence>& sequences) override {

			if (this->repeatition == RuleRegExpRepeatType::STAR || this->repeatition == RuleRegExpRepeatType::PLUS) {
				throw std::runtime_error("Backup sequence cannot contain Kleene star");
			}
			else {
				vector<BackupSequence> copy2 = sequences;
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

		void setParentRule(GraphiteRule* rule) override {
			parentRule_ = rule;
			regex->setParentRule(rule);
		}

		std::shared_ptr<RuleRegExp> clone() override {
			return make_shared<RuleRegExpRepeat>(regex->clone(), repeatition);
		}

	protected:
		PRuleRegExp regex;
		RuleRegExpRepeatType repeatition;

	};



}
#endif // H_FEA_JUST