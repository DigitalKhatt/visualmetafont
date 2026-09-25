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

%language "C++"
/*%no-lines*/
/*%glr-parser*/
%require "3.0.4"
%define parse.trace
%defines
%define api.namespace {feayy}
%define api.value.type variant
%define api.parser.class {Parser}
%locations
/*%error-verbose*/
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};
%parse-param { class ::feayy::Driver& driver }
%lex-param { class ::feayy::Driver& driver }
%code top {


#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex
}
%code requires {
#include "driver.h"
#include "feaast.h"
//using namespace std;
//using namespace feayy;
}


%token<int> INT_LITERAL CID
%token<std::vector<int>> CHAR_STRING
%token<double> DOUBLE_LITERAL
%token<std::string> IDENTIFIANT REGEXP GLYPHNAME T_FILENAME

%token LOOKUP FEATURE POSITION SUBSTITUTE BASE ANCHOR MARK MARKCLASS CURSIVE T_NULL NOTDEF FUNCTION BY FROM ACTION
%token COLOR CALLBACK EXPANSION ADD ADDPARAMS STARTLIG ENDLIG ENDKASHIDA JUST COORD PARAMS
%token LOOKUPFLAG RightToLeft IgnoreBaseGlyphs IgnoreLigatures IgnoreMarks MarkAttachmentType UseMarkFilteringSet
%token TABLE ENDTABLE PASS ENDPASS ANY STRETCH SHRINK END STEP AFTERGSUB
%token STRETCHPOLICY SHRINKPOLICY STAGES STAGE QUANTIZE
%token JUSTDFA FACTS RULES ACTIONS SELECTIONS FACT RULE BIND SELECTION CHOOSE TRAVERSAL PHASE GLYPHS
%token ATTACHMENTS ATTACHMENT FORBID UPDATE REPLACE ISFACT AND OR NOT WHEN CLEAR HAS VARY
%token LINEPOLICY PAGEPOLICY
%token INCLUDE IF ELIF ELSE ENDIF

%type <FeaRoot*> root
%type <std::vector<Statement *> *> statements
%type <Statement*> statement
%type <Statement*> justdfa_definition
%type <Glyph*> glyph glyphidentifier
%type <ClassName*> classname
%type <ClassComponent*> classcomponent
%type <GlyphClass*> glyphclass
%type <std::vector<ClassComponent *>> classcomponents
%type <GlyphSet*> glyphset glyphsetwithoutglyph
%type <std::string> lookupreference featurereference identifier featuretag
%type <std::vector<std::string>> explicitlookup
%type <ValueRecord> valuerecord
%type <LookupFlag*> flag flags lookupflag
%type <Statement*> feature_statement lookup_statement lookup_definition markclassdefinition gsubrule gposrule singlesub multiplesub alternatesub classdefinition contextualligagsub
%type <Statement*> singleadjustment cursiverule mark2base mark2mark pairadjustment
%type <std::vector<Statement *> *> lookup_statements feature_statements
%type <MarkedGlyphSetRegExp*> markedglyphsetregexp
%type <std::vector<MarkedGlyphSetRegExp *>*> inputseq
%type <GlyphSetRegExp*> glyphsetregexp  glyphsetregexpwithoutglyph glyphsetregexp_prim /*gsregex gsregex_prim gsregex_sec  gsregex_terc gsregex_q */
%type <ChainingContextualRule*> contextualexplicit contextualgpos
%type <Anchor*> anchor anchorformat
%type <Mark2BaseClass*> mark2baseclass
%type <std::vector<Mark2BaseClass *>*> mark2baseclasses
%type <double> doubleorint
%type <GlyphExpansion> expafactor
%type <FeatureDefenition*> feature_definition;
%type <std::vector<Glyph *>*> glyphseq;
%type <StartEndLig> startendlig;
%type <PRuleRegExp> rule_regex prregexp_terc prregexp_sec  prregexp_prim  rule_lhs rule_rhs rule_context prregexp_action;
%type <GraphiteRule> rule openttype_regexp;
%type <Pass>  rules pass_definition;
%type <TableDefinition*>  table_definition passes;
%type <std::vector<std::string>> lookupreferences aftergsub;
%type <std::vector<std::pair<std::string, double>>> glyphparameteradjustments;
%type <JustTable::JustStep> juststep;
%type <JustTable> justrules;
%type <std::vector<JustTable::JustStep>> juststeps stretch shrink;
%type <IncludeStatment*> include_statement
%type <ConditionalStatement*>  conditional_statement conditional_statement_lookup
%type <ConditionalStatement*> elif_chain elif_chain_lookup
%type <ValueRecordExtended> valuerecordextended;
%type <std::vector<int>> character_list;
%type <std::vector<std::string>> identifier_list;
%type <digitalkhatt::justify::DfaFactSource> justdfa_fact justdfa_fact_body;
%type <digitalkhatt::justify::DfaRuleSource> justdfa_rule;
%type <digitalkhatt::justify::DfaActionSource> justdfa_action;
%type <digitalkhatt::justify::DfaSelectionSource> justdfa_selection justdfa_selection_body;
%type <digitalkhatt::justify::DfaChoiceSource> justdfa_choice_options;
%type <digitalkhatt::justify::DfaPhaseSource> justdfa_phase justdfa_phase_options;
%type <std::vector<digitalkhatt::justify::DfaFactSource>> justdfa_facts;
%type <std::vector<digitalkhatt::justify::DfaRuleSource>> justdfa_rules;
%type <digitalkhatt::justify::DfaAttachmentSource> justdfa_attachment justdfa_attachment_body;
%type <std::vector<digitalkhatt::justify::DfaAttachmentSource>> justdfa_attachments justdfa_attachments_block;
%type <digitalkhatt::justify::DfaActionsBlockSource> justdfa_actions_block;
%type <digitalkhatt::justify::DfaActionDefSource> justdfa_action_def;
%type <std::vector<digitalkhatt::justify::DfaEffectSource>> justdfa_effects;
%type <digitalkhatt::justify::DfaEffectSource> justdfa_effect;
%type <std::vector<digitalkhatt::justify::DfaWriteSource>> justdfa_writes;
%type <digitalkhatt::justify::DfaTargetSource> justdfa_target justdfa_slot;
%type <digitalkhatt::justify::DfaExprSource> justdfa_cond justdfa_comparison justdfa_prim;
%type <std::vector<digitalkhatt::justify::DfaExprSource>> justdfa_args;
%type <std::vector<digitalkhatt::justify::DfaSelectionSource>> justdfa_selections;
%type <std::vector<digitalkhatt::justify::DfaPhaseSource>> justdfa_phases;
%type <digitalkhatt::justify::DfaStageSource> justdfa_stage;
%type <std::vector<digitalkhatt::justify::DfaStageSource>> justdfa_stages justdfa_stage_list;
%type <digitalkhatt::justify::DfaLineRecipeSource> justdfa_stretch_policy justdfa_shrink_policy justdfa_line_recipe_body;
%type <std::vector<digitalkhatt::justify::DfaLineRecipeSource>> justdfa_stretch_policy_list justdfa_shrink_policy_list;
%type <std::optional<digitalkhatt::justify::DfaLinePolicySource>> justdfa_line_policy;
%type <digitalkhatt::justify::DfaLinePolicySource> justdfa_line_branches;
%type <digitalkhatt::justify::DfaLinePolicySelectionSource> justdfa_line_selection;
%type <std::vector<digitalkhatt::justify::DfaLineStepSource>> justdfa_line_steps;
%type <digitalkhatt::justify::DfaLineStepSource> justdfa_line_step;
%type <std::optional<digitalkhatt::justify::DfaPagePolicySource>> justdfa_page_policy;
%type <std::vector<digitalkhatt::justify::DfaPageBranchSource>> justdfa_page_branches;
%type <digitalkhatt::justify::DfaPageBranchSource> justdfa_page_branch;
%type <std::string> justdfa_page_condition;

//%destructor { delete $$; } <std::string>

%start root

%%

root
	: statements { $$ = new FeaRoot($1); driver.context.root = $$; }
	;

statements
	:  { $$ = new std::vector<Statement *>(); }  /* empty */
	| statements statement ';'
		{
			$$ = $1;
			$1->push_back($2);
		}
	;

statement
	: conditional_statement {$$ = $1;}
	| lookup_definition {$$ = $1;}
	| markclassdefinition {$$ = $1;}
	| classdefinition {$$ = $1;}
	| feature_definition {$$ = $1;}
	| table_definition {$$ = $1;}
	| justdfa_definition {$$ = $1;}
	| include_statement {$$ = $1;}
	| lookupreference { $$ = new LookupReference($1);}
	;

conditional_statement:
      IF identifier[condition] statements[stmts] elif_chain[else]
        {
          $$ = new ConditionalStatement($condition, $stmts, $else);
        }
    ;

elif_chain:
      ELIF identifier[condition] statements[stmts] elif_chain[else]
        {
          $$ = new ConditionalStatement($condition, $stmts, $else);
        }
    | ELSE  statements[stmts] ENDIF
        {
          /* else without condition */
          $$ = new ConditionalStatement("", $stmts, nullptr);
        }
    | ENDIF
        {
          /* no else */
          $$ = nullptr;
        }
    ;

include_statement
	: INCLUDE '(' { driver.lexer->pushFilenameMode(); } T_FILENAME[name] ')'
	{
		if(!driver.parse_file($name)){
			error(yyla.location, std::string("File name '") + $name + "' cannot be parsed'");
		};
		$$ = new IncludeStatment($name);
	}
	;
feature_definition
	: FEATURE featuretag[bname] '{' feature_statements[lstm] '}' featuretag[lname]
	{
		if($bname != $lname){
			//yyparser.error(yylloc, std::string("Feature name '") + $bname + "' does not match with '" + $lname + "'");
			error(yyla.location, std::string("Feature name '") + $bname + "' does not match with '" + $lname + "'");
			YYERROR;
		}else{
			$$ = new FeatureDefenition($bname,$lstm);
			driver.context.features.push_back($$);
		}
	}
	;

feature_statements
	: feature_statement ';' { $$ = new std::vector<Statement *>(); $$->push_back($1); }
	| feature_statements feature_statement ';'
		{
			$$ = $1;
			$1->push_back($2);
		}
	;

feature_statement
	: conditional_statement {$$ = $1;}
	| lookup_definition {$$ = $1;}
	| lookupreference { $$ = new LookupReference($1);}
	| classdefinition {$$ = $1;}
	;

lookup_definition
	: LOOKUP identifier[bname] <int>{ $$ = driver.context.getNbLookup(); }[order] '{' lookup_statements[lstm] '}' identifier[lname]
		{
			if($bname != $lname){
//				yyparser.error(yylloc, std::string("Lookup name '") + $bname + "' does not match with '" + $lname + "'");
				error(yyla.location, std::string("Lookup name '") + $bname + "' does not match with '" + $lname + "'");

				YYERROR;
			}else{
			    if(driver.context.lookups.find($bname) != driver.context.lookups.end()){
				//yyparser.error(yylloc, std::string("Lookup name '") + $bname + "' does not match with '" + $lname + "'");
				error(yyla.location, std::string("Lookup name '") + $bname + "' already defined");

				YYERROR;
			    }else{
			      auto lookup = new LookupDefinition($bname,$lstm,$order);
			      driver.context.lookups[$bname] = lookup;
			      $$ = lookup;
			    }

			}

		}
	;

lookup_statements
	: lookup_statement ';' { $$ = new std::vector<Statement *>(); $$->push_back($1); }
	| lookup_statements lookup_statement ';'
		{
			$$ = $1;
			$1->push_back($2);
		}
	;

lookup_statement
	: lookupflag {$$ = $1;}
	| markclassdefinition {$$ = $1;}
	| classdefinition {$$ = $1;}
	| gsubrule {$$ = $1;}
	| gposrule {$$ = $1;}
	| lookup_definition {$$ = $1;}
	| featurereference { $$ = new FeatureReference($1);}
	| conditional_statement_lookup {$$ = $1;}
	;

conditional_statement_lookup:
      IF identifier[condition] lookup_statements[stmts] elif_chain_lookup[else]
        {
          $$ = new ConditionalStatement($condition, $stmts, $else);
        }
    ;

elif_chain_lookup
	: ELIF identifier[condition] lookup_statements[stmts] elif_chain_lookup[else] { $$ = new ConditionalStatement($condition, $stmts, $else);}
	| ELSE  lookup_statements[stmts] ENDIF {/* else without condition */ $$ = new ConditionalStatement("", $stmts, nullptr);}
	| ENDIF { /* no else */ $$ = nullptr; }
	;


lookupflag
	: LOOKUPFLAG INT_LITERAL	{$$ = new LookupFlag($2);}
	| LOOKUPFLAG flags			{$$ = $2;}
	;

flags
	: flag			{$$ = $1;}
	| flags flag	{*$1 = *$1 | *$2; $$ = $1; /*delete $2;*/}
	;

flag
	: RightToLeft					{$$ = new LookupFlag(LookupFlag::RightToLeft);}
	| IgnoreBaseGlyphs				{$$ = new LookupFlag(LookupFlag::IgnoreBaseGlyphs);}
	| IgnoreLigatures				{$$ = new LookupFlag(LookupFlag::IgnoreLigatures);}
	| IgnoreMarks					{$$ = new LookupFlag(LookupFlag::IgnoreMarks);}
	| UseMarkFilteringSet glyphset	{$$ = new LookupFlag($2);}
	;

gsubrule
	: singlesub {$$ = $1;}
	| multiplesub {$$ = $1;}
	| alternatesub {$$ = $1;}
	| contextualligagsub {$$ = $1;}
	| TABLE '(' SUBSTITUTE ')' passes ENDTABLE { $passes->name = "sub" ;$$ = $passes;}
	;

singlesub
	: SUBSTITUTE glyph BY glyph	{$$ = new SingleSubstituionRule($2,$4,2);}
	| SUBSTITUTE CALLBACK glyph {$$ = new SingleSubstituionRule($glyph,$glyph,10);}
	| SUBSTITUTE glyphclass ADD doubleorint[lefttatweel] doubleorint[righttatweel]  {$$ = new SingleSubstituionRule(new GlyphSet($glyphclass),$lefttatweel,$righttatweel);}
	| SUBSTITUTE glyph ADD doubleorint[lefttatweel] doubleorint[righttatweel]  {$$ = new SingleSubstituionRule(new GlyphSet($glyph),$lefttatweel,$righttatweel);}
	| SUBSTITUTE glyph[glyph1] BY glyph[glyph2] ADD doubleorint[lefttatweel] doubleorint[righttatweel]  {$$ = new SingleSubstituionRule($glyph1,$glyph2,$lefttatweel,$righttatweel);}
	| SUBSTITUTE glyph ADDPARAMS '{' glyphparameteradjustments[parameters] '}' {$$ = new SingleSubstituionRule(new GlyphSet($glyph), std::move($parameters));}
	| SUBSTITUTE glyphclass ADDPARAMS '{' glyphparameteradjustments[parameters] '}' {$$ = new SingleSubstituionRule(new GlyphSet($glyphclass), std::move($parameters));}
	| SUBSTITUTE glyph[glyph1] BY glyph[glyph2] ADDPARAMS '{' glyphparameteradjustments[parameters] '}' {$$ = new SingleSubstituionRule($glyph1, $glyph2, std::move($parameters));}
	| SUBSTITUTE CALLBACK glyph[glyph1] BY glyph[glyph2] {$$ = new SingleSubstituionRule($glyph1,$glyph2,10);}
	| SUBSTITUTE CALLBACK glyph startendlig expafactor[factor] {$$ = new SingleSubstituionRule($glyph,$glyph,10,$factor,$startendlig);}
	| SUBSTITUTE CALLBACK glyph[glyph1] BY glyph[glyph2] startendlig expafactor[factor] {$$ = new SingleSubstituionRule($glyph1,$glyph2,10,$factor,$startendlig);}
	| SUBSTITUTE CALLBACK glyphclass startendlig expafactor[factor]  {$$ = new SingleSubstituionRule(new GlyphSet($glyphclass),10,$factor,$startendlig);}
	| SUBSTITUTE glyphclass BY glyph
	| SUBSTITUTE glyphclass BY glyphclass
	;

glyphparameteradjustments:
	%empty {$$ = {};}
	| glyphparameteradjustments IDENTIFIANT[name] doubleorint[value] ';' {$$ = std::move($1); $$.emplace_back(std::move($name), $value);}
	;

expafactor:
	EXPANSION doubleorint[minleft] doubleorint[maxleft] doubleorint[minright] doubleorint[maxright] {$$ = {$minleft,$maxleft,$minright,$maxright};}
	| EXPANSION doubleorint[minleft] doubleorint[maxleft] doubleorint[minright] doubleorint[maxright] INT_LITERAL[absolute] {$$ = {$minleft,$maxleft,$minright,$maxright,1,0,StartEndLig::StartEnd,(bool)$absolute,(bool)$absolute};}

	;

startendlig:
	/* empty */ {$$ = StartEndLig::StartEnd;}
	| STARTLIG {$$ = StartEndLig::Start;}
	| ENDLIG {$$ = StartEndLig::End;}
	| ENDKASHIDA {$$ = StartEndLig::EndKashida;}

	;

multiplesub
	: SUBSTITUTE glyph BY glyphseq { $$ = new MultipleSubstitutionRule($glyph,$glyphseq);}
	;

alternatesub
	: SUBSTITUTE glyph FROM glyphclass {$$ = new AlternateSubstitutionRule($glyph, $glyphclass);}
	;

doubleorint
	: DOUBLE_LITERAL | INT_LITERAL {$$ = $1;}
	;

glyphseq
	: glyph glyph { $$ = new std::vector<Glyph *>{$1,$2}; }
	| glyphseq glyph {$$ = $1; $$->push_back($glyph);}
	;


/*
ligaturesub
	: SUBSTITUTE glyphseq  BY glyph[ligature] { $$ = new LigatureSubstitutionRule($glyphseq,$ligature);}
	;*/

contextualligagsub
	: SUBSTITUTE inputseq {auto lookup = new ChainingContextualRule(nullptr,$2,nullptr);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE inputseq glyphsetregexp {auto lookup = new ChainingContextualRule(nullptr,$2,$3);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE glyphsetregexpwithoutglyph[back] inputseq {auto lookup = new ChainingContextualRule($back,$inputseq,nullptr);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE glyphsetregexpwithoutglyph[back] inputseq glyphsetregexp[look] {auto lookup = new ChainingContextualRule($back,$inputseq,$look);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE glyphseq[back] inputseq {auto lookup = new ChainingContextualRule(new GlyphSetRegExpGlyphSeq($back),$inputseq,nullptr);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE glyphseq[back] inputseq glyphsetregexp[look] {auto lookup = new ChainingContextualRule(new GlyphSetRegExpGlyphSeq($back),$inputseq,$look);lookup->setType(true);$$ = lookup;}
	| SUBSTITUTE glyphseq  BY glyph[ligature] { $$ = new LigatureSubstitutionRule($glyphseq,$ligature);}
	;

contextualgpos
	: POSITION contextualexplicit {$$ = $2;$$->setType(false);}
	;


contextualexplicit
	: inputseq {$$ = new ChainingContextualRule(nullptr,$1,nullptr);}
	| inputseq glyphsetregexp {$$ = new ChainingContextualRule(nullptr,$1,$2);}
	| glyphsetregexp inputseq {$$ = new ChainingContextualRule($1,$2,nullptr);}
	| glyphsetregexp inputseq glyphsetregexp {$$ = new ChainingContextualRule($1,$2,$3);}
	;



gposrule
	: singleadjustment	{$$ = $1;}
	| pairadjustment {$$ = $1;}
	| cursiverule {$$ = $1;}
	| mark2base {$$ = $1;}
	| mark2mark {$$ = $1;}
	| contextualgpos {$$ = $1;}
	;

singleadjustment
	: POSITION glyphset valuerecord {$$ = new SingleAdjustmentRule($glyphset,$valuerecord,false);}
	| POSITION glyphset COLOR valuerecord {$$ = new SingleAdjustmentRule($glyphset,$valuerecord,true);}
	;

pairadjustment
	: POSITION glyphset[glyphset1] valuerecordextended[valuerecord1] glyphset[glyphset2] valuerecordextended[valuerecord2]
		{
			$$ = new PairAdjustmentRule($glyphset1,$glyphset2,$valuerecord1,$valuerecord2);
		}
	;

cursiverule
	: POSITION CURSIVE glyphset anchor[entry] anchor[exit]  {$$ = new CursiveRule($glyphset,$entry,$exit);}
	;

mark2base
	: POSITION BASE glyphset mark2baseclasses	{$$ = new Mark2BaseRule($glyphset,$mark2baseclasses,Lookup::mark2base);}
	;

mark2mark
	: POSITION MARK glyphset mark2baseclasses	{$$ = new Mark2BaseRule($glyphset,$mark2baseclasses,Lookup::mark2mark);}
	;



%left OR;
%left AND;
%precedence NOT;
%left '|';
%precedence CID IDENTIFIANT REGEXP GLYPHNAME NOTDEF '[' '@' T_NULL;
%precedence GLYPHSET;
%precedence  '(';
%precedence  '{';
%precedence  '?';

glyphsetregexpwithoutglyph
	: glyphsetwithoutglyph { $$ = new GlyphSetRegExpSingle($1); }
	| glyph glyphsetwithoutglyph { $$ = new GlyphSetRegExpSeq{new GlyphSetRegExpSingle(new GlyphSet($glyph)),new GlyphSetRegExpSingle($glyphsetwithoutglyph)};}
	/*| glyphseq glyphsetwithoutglyph { $$ = new GlyphSetRegExpSingle($glyphsetwithoutglyph); }*/
	| glyphsetregexpwithoutglyph[left] glyphset { $$ = new GlyphSetRegExpSeq{$left,$glyphset};}
	| glyphsetregexpwithoutglyph[left] '|' glyphsetregexp[right]  { $$ = new GlyphSetRegExpOr{$left,$right};}
	| glyphsetregexpwithoutglyph[left] '(' glyphsetregexp[right] ')' { $$ = new GlyphSetRegExpSeq{$left,$right};}
	| '(' glyphsetregexp ')' { $$ = $glyphsetregexp;}
	;

/*
gsregex
	: gsregex_q	{ $$ = $1;}
	| gsregex gsregex_q  {$$ = new GlyphSetRegExpSeq{$1,$2};}
	;

gsregex_q
	: gsregex_prim	{ $$ = $1;}
	| gsregex_q '\'' { $$ = $1;}
	;


gsregex_terc
	: gsregex_terc[left] '|' gsregex_sec[right]  { $$ = new GlyphSetRegExpOr{$left,$right};}
	| gsregex_sec	{ $$ = $1;}
	;

gsregex_sec
	: gsregex_sec gsregex_prim  {$$ = new GlyphSetRegExpSeq{$1,$2};}
	| gsregex_prim	{ $$ = $1;}
	;


gsregex_prim
	: glyphset { $$ = new GlyphSetRegExpSingle($1); }
	| gsregex_prim[exp] '{' INT_LITERAL[min] ',' INT_LITERAL[max] '}'  { $$ = new GlyphSetRegExpRep{$exp,$min,$max};}
	| '(' gsregex_terc ')' { $$ = $2;}
	;*/

/*
glyphsetregexp_prim
	: glyphsetregexp[left] '|' glyphsetregexp[right]  { $$ = new GlyphSetRegExpOr{$left,$right};}
	;


glyphsetregexp
	: glyphset { $$ = new GlyphSetRegExpSingle($1); }
	| glyphsetregexp glyphset { $$ = new GlyphSetRegExpSeq{$1,$glyphset};}
	| glyphsetregexp '(' glyphsetregexp ')' { $$ = new GlyphSetRegExpSeq{$1,$3};}
	| '(' glyphsetregexp ')' { $$ = $2;}
	| glyphsetregexp[exp] '{' INT_LITERAL[min] ',' INT_LITERAL[max] '}'  { $$ = new GlyphSetRegExpRep{$exp,$min,$max};}
	| glyphsetregexp_prim
	;*/


glyphsetregexp
	: glyphset { $$ = new GlyphSetRegExpSingle($1); }
	| glyphsetregexp glyphset { $$ = new GlyphSetRegExpSeq{$1,$glyphset};}
	| glyphsetregexp[left] '|' glyphsetregexp[right]  { $$ = new GlyphSetRegExpOr{$left,$right};}
	| glyphsetregexp[left] '?'  { $$ = new GlyphSetRegExpOr{$left,new GlyphSetRegExpSingle(new GlyphSet())};}
	| glyphsetregexp '(' glyphsetregexp ')' { $$ = new GlyphSetRegExpSeq{$1,$3};}
	| '(' glyphsetregexp ')' { $$ = $2;}
	| glyphsetregexp[exp] '{' INT_LITERAL[min] ',' INT_LITERAL[max] '}'  { $$ = new GlyphSetRegExpRep{$exp,$min,$max};}
	;





inputseq
	: markedglyphsetregexp  { $$ = new std::vector<MarkedGlyphSetRegExp *>();$$->push_back($1); }
	| inputseq markedglyphsetregexp
		{
			$$ = $1;
			$$->push_back($2);
		}
	;

markedglyphsetregexp
	: glyphset '\'' explicitlookup {$$ = new MarkedGlyphSetRegExp($glyphset,$explicitlookup);}
	| '(' glyphsetregexp ')' '\'' explicitlookup {$$ = new MarkedGlyphSetRegExp($glyphsetregexp,$explicitlookup);}
	| glyphset '\'' valuerecord {
		$$ = new MarkedGlyphSetRegExp($glyphset,$valuerecord);
	}
	| CURSIVE glyphset'\''  anchor[entry] anchor[exit]  {
		$$ = new MarkedGlyphSetRegExp($glyphset,new CursiveRule($glyphset,$entry,$exit));
	}

	;


explicitlookup
	: /* empty */	{$$ = {};}
	| explicitlookup lookupreference {$$ = std::move($1);$$.push_back($2);}
	;

mark2baseclasses
	: mark2baseclass					{ $$ = new std::vector<Mark2BaseClass *>{ $1};}
	| mark2baseclasses mark2baseclass	{ $1->push_back($2);$$ = $1;}
	;

mark2baseclass
	: anchor MARK classname
	| anchor[baseAnchor] MARKCLASS glyphset anchor[markAnchor] '@' identifier  {$$ = new Mark2BaseClass($glyphset,$baseAnchor,$markAnchor, $identifier);}
	;

markclassdefinition
	: MARKCLASS glyphset anchor classname
		{
		}
	;

classdefinition
	: '@' identifier '=' '[' classcomponents ']' {$$ = new ClassDefinition($identifier,new GlyphClass(std::move($classcomponents)));}
	;

anchor
	: '<' ANCHOR anchorformat '>' {$$ = $anchorformat;}
	;

anchorformat
	: INT_LITERAL[x] INT_LITERAL[y]		{ $$ = new AnchorFormatA($x,$y);}
	| identifier						{ $$ = new AnchorName($identifier);}
	| FUNCTION identifier				{ $$ = new AnchorFunction($identifier);}
	| T_NULL							{ $$ = new AnchorNull();}
	;

glyphset
	: glyphclass	{$$ = new GlyphSet($1);}
	| glyph			{$$ = new GlyphSet($1);}
	| T_NULL		{$$ = new GlyphSet();}

glyphsetwithoutglyph
	: glyphclass	{$$ = new GlyphSet($1);}
	| T_NULL		{$$ = new GlyphSet();}

glyphclass
	: '[' classcomponents ']'	{$$ = new GlyphClass(std::move($2));}
	| classname					{$$ = new GlyphClass($1);}
	| REGEXP					{$$ = new GlyphClass(new RegExpClass($1));}
	;

classcomponents
	: classcomponent					{ $$ = std::vector<ClassComponent *>{ $1};}
	| classcomponents  classcomponent	{ $1.push_back($2);$$ = std::move($1);}
	;

classcomponent
	: glyph			{$$ = $1;}
	| classname		{$$ = $1;}
	| REGEXP		{$$ = new RegExpClass($1);}
	;

/*ADD doubleorint[lefttatweel] doubleorint[righttatweel]*/
glyph
	: glyphidentifier	{$$ = $1;}
	| glyphidentifier	COORD doubleorint[lefttatweel] doubleorint[righttatweel]	{
		GlyphParameters parameters;
		parameters.lefttatweel = $lefttatweel;
		parameters.righttatweel = $righttatweel;
		$$ = new GlyphWithParameters($1, parameters);
	}
	| glyphidentifier PARAMS '{' glyphparameteradjustments[parameters] '}' {
		$$ = new GlyphWithParameters($1, std::move($parameters));
	}
	;

glyphidentifier
	: identifier	{$$ = new GlyphName($1);}
	| GLYPHNAME		{$$ = new GlyphName($1);}
	| NOTDEF		{$$ = new GlyphName(std::string(".notdef"));}
	| CID			{$$ = new GlyphCID($1);}
	;

classname
	: '@' identifier {$$ = new ClassName($2);}
	;

valuerecordextended
	: valuerecord { $$ = $valuerecord;}
	| '<' FUNCTION identifier '>'				{ $$ = $identifier;}
	;

valuerecord
	: INT_LITERAL													{$$ = {0,0,(std::int16_t)$1,0};}
	| '<' INT_LITERAL '>'											{$$ = {0,0,(std::int16_t)$2,0};}
	| '<' INT_LITERAL  INT_LITERAL  INT_LITERAL INT_LITERAL '>'		{$$ = {(std::int16_t)$2,(std::int16_t)$3,(std::int16_t)$4,(std::int16_t)$5};}
	;

lookupreference
	: LOOKUP identifier	{$$ = $2;}
	;

featurereference
	: FEATURE featuretag	{$$ = $2;}
	;

featuretag
	: IDENTIFIANT {$$ = $1;}
	| MARK	 {$$ = std::string("mark");}
	;

identifier
	: IDENTIFIANT	{$$ = $1;}
	;

table_definition
	: TABLE '(' identifier ')' passes ENDTABLE { $$ = $passes;driver.context.tables[$identifier] = $$;$$->name = std::move($identifier); }
	| TABLE '(' JUST ')' '{' justrules '}' { driver.context.jusTable = $justrules; }
	;

justdfa_definition
	: TABLE '(' JUSTDFA ')' identifier[name] '{'
	  justdfa_page_policy[pagePolicy]
	  justdfa_line_policy[linePolicy]
	  FACTS '{' justdfa_facts[facts] '}'
	  RULES '{' justdfa_rules[rules] '}'
	  justdfa_attachments_block[attachments]
	  ACTIONS '{' justdfa_actions_block[actions] '}'
	  SELECTIONS '{' justdfa_selections[selections] '}'
	  justdfa_stages[stages]
	  justdfa_stretch_policy_list[stretchPolicies]
	  justdfa_shrink_policy_list[shrinkPolicies]
	  '}'
	{
		digitalkhatt::justify::JustificationDfaSource source;
		source.name = $name;
		source.pagePolicy = std::move($pagePolicy);
		source.linePolicy = std::move($linePolicy);
		source.facts = std::move($facts);
		source.rules = std::move($rules);
		source.attachments = std::move($attachments);
		source.actions = std::move($actions.binds);
		source.actionDefinitions = std::move($actions.definitions);
		source.selections = std::move($selections);
		source.stages = std::move($stages);
		source.stretchPolicies = std::move($stretchPolicies);
		source.shrinkPolicies = std::move($shrinkPolicies);
		driver.context.justificationDfas.insert_or_assign(source.name, std::move(source));
		$$ = new DeclarativeStatement();
	}
	;

justdfa_page_policy
	: { $$ = std::nullopt; }
	| PAGEPOLICY '{' identifier[property] '[' identifier_list[types] ']' ';' justdfa_page_branches[branches] '}'
	  {
		if ($property != "measure") { error(yyla.location, "pagepolicy must start with measure [line types];"); YYERROR; }
		$$ = digitalkhatt::justify::DfaPagePolicySource{std::move($types), std::move($branches)};
	  }
	;

justdfa_page_branches
	: { $$ = {}; }
	| justdfa_page_branches justdfa_page_branch { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_page_branch
	: identifier[section] identifier[selector] identifier[value] justdfa_page_condition[condition] '{' justdfa_line_steps[steps] '}'
	  { $$ = {$section, $selector, $value, $condition, std::move($steps)}; }
	| identifier[section] identifier[selector] '{' justdfa_line_steps[steps] '}'
	  { $$ = {$section, $selector, "", "", std::move($steps)}; }
	;

justdfa_page_condition
	: { $$ = ""; }
	| WHEN identifier[condition] { $$ = $condition; }
	;

justdfa_line_policy
	: { $$ = std::nullopt; }
	| LINEPOLICY '{' justdfa_line_branches[branches] '}' { $$ = std::move($branches); }
	;

justdfa_line_branches
	: { $$ = {}; }
	| justdfa_line_branches justdfa_line_selection[selection]
	  {
		$$ = std::move($1);
		if ($selection.selector == "stretch_policy") {
			if (!$$.stretchPolicy.empty()) { error(yyla.location, "linepolicy has more than one stretch policy"); YYERROR; }
			$$.stretchPolicy = std::move($selection.value);
		}
		else {
			if (!$$.shrinkPolicy.empty()) { error(yyla.location, "linepolicy has more than one shrink policy"); YYERROR; }
			$$.shrinkPolicy = std::move($selection.value);
		}
	  }
	;

justdfa_line_selection
	: STRETCH identifier[policy] ';' { $$ = {"stretch_policy", $policy}; }
	| SHRINK identifier[policy] ';' { $$ = {"shrink_policy", $policy}; }
	;

justdfa_line_steps
	: { $$ = {}; }
	| justdfa_line_steps justdfa_line_step { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_line_step
	: identifier[operation] ';' { $$ = {$operation, {}, {}}; }
	| STAGE identifier[name] ';' { $$ = {.operation = "stage", .stage = $name}; }
	| identifier[operation] doubleorint[a] ';' { $$ = {$operation, {$a}, {}}; }
	| identifier[operation] doubleorint[a] doubleorint[b] ';' { $$ = {$operation, {$a, $b}, {}}; }
	| identifier[operation] doubleorint[a] doubleorint[b] doubleorint[c] doubleorint[d] ';' { $$ = {$operation, {$a, $b, $c, $d}, {}}; }
	| identifier[operation] '[' identifier_list[features] ']' ';' { $$ = {$operation, {}, std::move($features)}; }
	;

justdfa_facts
	: { $$ = {}; }
	| justdfa_facts justdfa_fact { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_fact
	: FACT identifier[name] justdfa_fact_body[body] ';'
	  {
		$$ = std::move($body);
		$$.name = $name;
	  }
	;

/* Keyword-tagged discriminators, each optional and ANDed together.  Spelled
   with plain identifiers, dispatched semantically, so that "form", "chars",
   "glyphs" and "position" stay usable as ordinary names everywhere else in the
   feature file -- the same approach as the selection properties below. */
justdfa_fact_body
	: { $$ = {}; }
	| justdfa_fact_body identifier[property] identifier[value]
	  {
		$$ = std::move($1);
		if ($property == "form") $$.form = $value;
		else {
			error(yyla.location, "Unknown justification fact property '" + $property + "'");
			YYERROR;
		}
	  }
	/* "position" is already token POSITION, from the GPOS rules, so it can
	   never arrive here as an identifier. */
	| justdfa_fact_body POSITION identifier[value]
	  { $$ = std::move($1); $$.position = $value; }
	| justdfa_fact_body identifier[property] '[' character_list[characters] ']'
	  {
		$$ = std::move($1);
		if ($property == "chars") $$.sourceCharacters = std::move($characters);
		else {
			error(yyla.location, "Unknown justification fact character property '" + $property + "'");
			YYERROR;
		}
	  }
	/* "glyphs" is a token rather than an identifier: both it and "chars" would
	   otherwise open with identifier '[', which is a reduce/reduce conflict that
	   silently makes the "chars" rule unreachable. */
	| justdfa_fact_body GLYPHS glyphset[set]
	  {
		$$ = std::move($1);
		driver.context.justificationGlyphSets.emplace_back($set);
		$$.glyphSetRef = static_cast<int>(driver.context.justificationGlyphSets.size()) - 1;
	  }
	;

/* Quoted strings contribute all their Unicode characters, including literal
   spaces and combining marks. No normalization or sequence matching is done.
   Keep these literals limited to character lists, not numeric action values. */
character_list
	: { $$ = {}; }
	| character_list INT_LITERAL { $$ = std::move($1); $$.push_back($2); }
	| character_list CHAR_STRING { $$ = std::move($1); $$.insert($$.end(), $2.begin(), $2.end()); }
	;

justdfa_rules
	: { $$ = {}; }
	| justdfa_rules justdfa_rule { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_rule
	: RULE identifier[name] '[' identifier_list[slots] ']' ';'
	  { $$ = {$name, std::move($slots)}; }
	;

identifier_list
	: identifier { $$ = {$1}; }
	| identifier_list identifier { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_action
	: BIND identifier[rule] identifier[action] ';' { $$ = {$rule, $action}; }
	;

justdfa_attachments_block
	: { $$ = {}; }
	| ATTACHMENTS '{' justdfa_attachments[list] '}' { $$ = std::move($list); }
	;

justdfa_attachments
	: { $$ = {}; }
	| justdfa_attachments justdfa_attachment { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_attachment
	: ATTACHMENT identifier[name] justdfa_attachment_body[body] ';'
	  { $$ = std::move($body); $$.name = $name; }
	;

justdfa_attachment_body
	: { $$ = {}; }
	| justdfa_attachment_body identifier[property] '[' character_list[characters] ']'
	  {
		$$ = std::move($1);
		if ($property == "find") $$.find = std::move($characters);
		else if ($property == "skip") $$.skip = std::move($characters);
		else {
			error(yyla.location, "Unknown justification attachment property '" + $property + "'");
			YYERROR;
		}
	  }
	/* The glyph-set form of find/skip.  GLYPHS keeps it distinct from the
	   character-list form, which opens with identifier '[' just as facts do. */
	| justdfa_attachment_body identifier[property] GLYPHS glyphset[set]
	  {
		$$ = std::move($1);
		driver.context.justificationGlyphSets.emplace_back($set);
		const int reference = static_cast<int>(driver.context.justificationGlyphSets.size()) - 1;
		if ($property == "find") $$.findGlyphSetRef = reference;
		else if ($property == "skip") $$.skipGlyphSetRef = reference;
		else {
			error(yyla.location, "Unknown justification attachment glyph property '" + $property + "'");
			YYERROR;
		}
	  }
	| justdfa_attachment_body identifier[property] INT_LITERAL[count]
	  {
		$$ = std::move($1);
		if ($property == "within") $$.within = $count;
		else {
			error(yyla.location, "Unknown justification attachment count property '" + $property + "'");
			YYERROR;
		}
	  }
	;

justdfa_actions_block
	: { $$ = {}; }
	| justdfa_actions_block justdfa_action { $$ = std::move($1); $$.binds.push_back(std::move($2)); }
	| justdfa_actions_block justdfa_action_def { $$ = std::move($1); $$.definitions.push_back(std::move($2)); }
	;

justdfa_action_def
	: ACTION identifier[name] '{' justdfa_effects[effects] '}'
	  { $$ = {}; $$.name = $name; $$.effects = std::move($effects); }
	;

justdfa_effects
	: { $$ = {}; }
	| justdfa_effects justdfa_effect { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

/* Effect verbs are tokens so that each effect is distinguishable from the
   first token; they would otherwise all begin with identifier '$'. */
justdfa_effect
	: FORBID justdfa_cond[condition] ';'
	  { $$ = {}; $$.kind = "forbid"; $$.condition = std::move($condition); }
	| ADD justdfa_target[target] ':' identifier[attribute] justdfa_prim[value] identifier[keyword] doubleorint[maximum] ';'
	  {
		if ($keyword != "clamp") {
			error(yyla.location, "Expected 'clamp' in a justification add effect, found '" + $keyword + "'");
			YYERROR;
		}
		$$ = {}; $$.kind = "add"; $$.target = std::move($target);
		$$.attribute = $attribute; $$.value = std::move($value); $$.clamp = $maximum;
	  }
	| UPDATE justdfa_target[target] ':' identifier[attribute] '=' justdfa_prim[value] ';'
	  {
		$$ = {}; $$.kind = "update"; $$.target = std::move($target);
		$$.attribute = $attribute; $$.value = std::move($value);
	  }
	| REPLACE justdfa_target[target] '{' justdfa_writes[writes] '}'
	  { $$ = {}; $$.kind = "replace"; $$.target = std::move($target); $$.writes = std::move($writes); }
	| CLEAR justdfa_target[target] ';'
	  { $$ = {}; $$.kind = "clear"; $$.target = std::move($target); }
	| LOOKUP justdfa_target[target] identifier[name] ';'
	  { $$ = {}; $$.kind = "lookup"; $$.target = std::move($target); $$.lookup = std::move($name); }
	| VARY justdfa_target[target] ':' identifier[attribute] identifier[keyword] justdfa_prim[value] ';'
	  {
		if ($keyword != "to") {
			error(yyla.location, "Expected 'to' in a justification vary effect, found '" + $keyword + "'");
			YYERROR;
		}
		$$ = {}; $$.kind = "vary"; $$.target = std::move($target);
		$$.attribute = $attribute; $$.value = std::move($value);
	  }
	| WHEN justdfa_cond[condition] '{' justdfa_effects[nested] '}'
	  { $$ = {}; $$.kind = "when"; $$.condition = std::move($condition); $$.nested = std::move($nested); }
	;

justdfa_writes
	: { $$ = {}; }
	| justdfa_writes identifier[attribute] '=' justdfa_prim[value] ';'
	  { $$ = std::move($1); $$.push_back({$attribute, std::move($value)}); }
	;

/* ':' separates a target from its attribute because the lexer allows '.'
   inside an identifier, so "$self.cv02" would arrive as a single token. */
justdfa_target
	: justdfa_slot { $$ = std::move($1); }
	| justdfa_slot '@' identifier[attachment] { $$ = std::move($1); $$.attachment = $attachment; }
	;

justdfa_slot
	: '$' identifier[name]
	  {
		$$ = {};
		if ($name != "self") {
			error(yyla.location, "Unknown justification slot '$" + $name + "'; use $self or $1..$n");
			YYERROR;
		}
		$$.slot = -1;
	  }
	| '$' INT_LITERAL[position] { $$ = {}; $$.slot = $position; }
	;

justdfa_cond
	: justdfa_comparison { $$ = std::move($1); }
	| justdfa_cond[left] AND justdfa_cond[right]
	  {
		$$ = std::move($left);
		for (auto& node : $right) $$.push_back(std::move(node));
		$$.push_back({"and"});
	  }
	| justdfa_cond[left] OR justdfa_cond[right]
	  {
		$$ = std::move($left);
		for (auto& node : $right) $$.push_back(std::move(node));
		$$.push_back({"or"});
	  }
	| NOT justdfa_cond[operand] { $$ = std::move($operand); $$.push_back({"not"}); }
	| '(' justdfa_cond[inner] ')' { $$ = std::move($inner); }
	;

justdfa_comparison
	: justdfa_prim { $$ = std::move($1); }
	| justdfa_prim[left] '>' justdfa_prim[right]
	  {
		$$ = std::move($left);
		for (auto& node : $right) $$.push_back(std::move(node));
		$$.push_back({"gt"});
	  }
	| justdfa_prim[left] '<' justdfa_prim[right]
	  {
		$$ = std::move($left);
		for (auto& node : $right) $$.push_back(std::move(node));
		$$.push_back({"lt"});
	  }
	;

justdfa_prim
	: INT_LITERAL[value] { $$ = {}; $$.push_back({.op = "literal", .literal = static_cast<double>($value)}); }
	| DOUBLE_LITERAL[value] { $$ = {}; $$.push_back({.op = "literal", .literal = $value}); }
	| justdfa_target[target] ':' identifier[attribute]
	  { $$ = {}; $$.push_back({.op = "attribute", .target = std::move($target), .name = $attribute}); }
	/* A fact test needs a token of its own: its second argument is a fact name,
	   not an expression, so it cannot share the function-call production. */
	| ISFACT '(' justdfa_target[target] ',' identifier[fact] ')'
	  { $$ = {}; $$.push_back({.op = "fact", .target = std::move($target), .name = $fact}); }
	/* Presence, which is not the same as "> 0": an attribute can be staged as
	   zero, and one of the kashida guards tests presence alone. */
	| HAS '(' justdfa_target[target] ':' identifier[attribute] ')'
	  { $$ = {}; $$.push_back({.op = "present", .target = std::move($target), .name = $attribute}); }
	| identifier[function] '(' ')'
	  {
		if ($function != "underfull" && $function != "underfull_percent" && $function != "initial_underfull" && $function != "initial_underfull_percent" && $function != "stretched" && $function != "stretched_percent") {
			error(yyla.location, "Unknown zero-argument justification function '" + $function + "'");
			YYERROR;
		}
		$$ = {}; $$.push_back({$function});
	  }
	| identifier[function] '(' justdfa_args[arguments] ')'
	  {
		if ($function == "underfull" || $function == "underfull_percent" || $function == "initial_underfull" || $function == "initial_underfull_percent" || $function == "stretched" || $function == "stretched_percent") {
			error(yyla.location, "Justification function '" + $function + "' takes no arguments");
			YYERROR;
		}
		$$ = {};
		for (auto& argument : $arguments)
			for (auto& node : argument) $$.push_back(std::move(node));
		$$.push_back({$function});
	  }
	;

justdfa_args
	: justdfa_cond { $$ = {std::move($1)}; }
	| justdfa_args ',' justdfa_cond { $$ = std::move($1); $$.push_back(std::move($3)); }
	;

justdfa_selections
	: { $$ = {}; }
	| justdfa_selections justdfa_selection { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_selection
	: SELECTION identifier[name] '{' justdfa_selection_body[body] '}'
	  { $$ = std::move($body); $$.name = $name; }
	;

justdfa_selection_body
	: { $$ = {}; }
	| justdfa_selection_body CHOOSE identifier[rule] identifier[direction] ';'
	  { $$ = std::move($1); $$.choices.push_back({$rule, $direction, "selected_only"}); }
	| justdfa_selection_body CHOOSE identifier[rule] identifier[direction] identifier[multiplicity] justdfa_choice_options[options] ';'
	  { $$ = std::move($1); $options.rule = $rule; $options.direction = $direction; $options.multiplicity = $multiplicity; $$.choices.push_back(std::move($options)); }
	| justdfa_selection_body TRAVERSAL identifier[direction] ';'
	  { $$ = std::move($1); $$.traversal = $direction; }
	| justdfa_selection_body identifier[property] identifier[value] ';'
	  {
		$$ = std::move($1);
		if ($property == "record") $$.record = $value;
		else {
			error(yyla.location, "Unknown scalar justification selection property '" + $property + "'");
			YYERROR;
		}
	  }
	| justdfa_selection_body identifier[property] '[' identifier_list[records] ']' ';'
	  {
		$$ = std::move($1);
		if ($property == "forbid_recorded") $$.forbidRecorded = std::move($records);
		else if ($property == "require_recorded") $$.requireRecorded = std::move($records);
		else if ($property == "different_subword") $$.differentSubword = std::move($records);
		else if ($property == "different_position") $$.differentPosition = std::move($records);
		else {
			error(yyla.location, "Unknown list justification selection property '" + $property + "'");
			YYERROR;
		}
	  }
	;

justdfa_choice_options
	: { $$ = {}; }
	| justdfa_choice_options identifier[property] doubleorint[value]
	  {
		$$ = std::move($1);
		if ($property == "weight") $$.weight = $value;
		else if ($property == "decay") $$.decay = $value;
		else {
			error(yyla.location, "Unknown justification choice option '" + $property + "'");
			YYERROR;
		}
	  }
	| justdfa_choice_options identifier[property] identifier[value]
	  {
		$$ = std::move($1);
		if ($property == "record") $$.record = $value;
		else {
			error(yyla.location, "Unknown scalar justification choice option '" + $property + "'");
			YYERROR;
		}
	  }
	| justdfa_choice_options identifier[property] '[' identifier_list[records] ']'
	  {
		$$ = std::move($1);
		if ($property == "forbid_recorded") $$.forbidRecorded = std::move($records);
		else if ($property == "require_recorded") $$.requireRecorded = std::move($records);
		else if ($property == "different_subword") $$.differentSubword = std::move($records);
		else if ($property == "different_position") $$.differentPosition = std::move($records);
		else {
			error(yyla.location, "Unknown list justification choice option '" + $property + "'");
			YYERROR;
		}
	  }
	;

justdfa_phases
	: { $$ = {}; }
	| justdfa_phases justdfa_phase { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_phase
	: PHASE identifier[selection] INT_LITERAL[levels] ';' { $$ = {$selection, $levels}; }
	| PHASE identifier[selection] INT_LITERAL[levels] identifier[allocator] justdfa_phase_options[options] ';'
	  { $$ = std::move($options); $$.selection = $selection; $$.levels = $levels; $$.allocator = $allocator; }
	;

justdfa_phase_options
	: { $$ = {}; }
	| justdfa_phase_options identifier[property] doubleorint[value]
	  {
		$$ = std::move($1);
		if ($property == "weight") $$.weight = $value;
		else if ($property == "limit" || $property == "per_word" || $property == "per_subword") {
			const auto integer = static_cast<int>($value);
			if (static_cast<double>(integer) != $value) {
				error(yyla.location, "Justification candidate limits must be integers");
				YYERROR;
			}
			if ($property == "limit") $$.limit = integer;
			else if ($property == "per_word") $$.perWord = integer;
			else $$.perSubword = integer;
		} else if ($property == "longer_subword") $$.longerSubword = $value;
		else if ($property == "central_connection") $$.centralConnection = $value;
		else if ($property == "word_position") $$.wordPosition = $value;
		else {
			error(yyla.location, "Unknown justification phase option '" + $property + "'");
			YYERROR;
		}
	  }
	;

justdfa_stages
	: { $$ = {}; }
	| STAGES '{' justdfa_stage_list[stages] '}' { $$ = std::move($stages); }
	;

justdfa_stage_list
	: { $$ = {}; }
	| justdfa_stage_list justdfa_stage[stage] { $$ = std::move($1); $$.push_back(std::move($stage)); }
	;

justdfa_stage
	: STAGE identifier[name] '{' justdfa_phases[phases] '}' { $$ = {$name, std::move($phases)}; }
	;

justdfa_stretch_policy_list
	: justdfa_stretch_policy { $$ = {std::move($1)}; }
	| justdfa_stretch_policy_list justdfa_stretch_policy { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

/* One selectable justification.  The line and page policies stay on the
   catalog: they are not stretching, and nesting them here would contradict the
   block's name. */
justdfa_stretch_policy
	: STRETCHPOLICY identifier[name] '{' justdfa_line_recipe_body[body] '}'
	  { $$ = std::move($body); $$.name = std::move($name); }
	;

justdfa_shrink_policy_list
	: justdfa_shrink_policy { $$ = {std::move($1)}; }
	| justdfa_shrink_policy_list justdfa_shrink_policy { $$ = std::move($1); $$.push_back(std::move($2)); }
	;

justdfa_shrink_policy
	: SHRINKPOLICY identifier[name] '{' justdfa_line_recipe_body[body] '}'
	  { $$ = std::move($body); $$.name = std::move($name); }
	;

justdfa_line_recipe_body
	: { $$ = {}; }
	| justdfa_line_recipe_body[body] justdfa_line_step[step]
	  { $$ = std::move($body); $$.steps.push_back(std::move($step)); }
	| justdfa_line_recipe_body[body] QUANTIZE INT_LITERAL[value] ';'
	  {
		$$ = std::move($body);
		if ($$.parameterQuantization) { error(yyla.location, "line policy has more than one quantize setting"); YYERROR; }
		if ($value <= 0) { error(yyla.location, "quantize expects a positive subdivision count or off"); YYERROR; }
		$$.parameterQuantization = static_cast<unsigned>($value);
	  }
	| justdfa_line_recipe_body[body] QUANTIZE identifier[value] ';'
	  {
		$$ = std::move($body);
		if ($$.parameterQuantization) { error(yyla.location, "line policy has more than one quantize setting"); YYERROR; }
		if ($value != "off") { error(yyla.location, "quantize expects a positive subdivision count or off"); YYERROR; }
		$$.parameterQuantization = 0;
	  }
	| justdfa_line_recipe_body[body] identifier[property] identifier[value] ';'
	  {
		$$ = std::move($body);
		if ($property != "candidate_width") { error(yyla.location, "Unknown line policy property '" + $property + "'"); YYERROR; }
		if ($$.candidateWidth) { error(yyla.location, "line policy has more than one candidate_width setting"); YYERROR; }
		if ($value != "advance" && $value != "full_shape") { error(yyla.location, "candidate_width expects advance or full_shape"); YYERROR; }
		$$.candidateWidth = std::move($value);
	  }
	;

justrules
	: stretch shrink aftergsub {$$=JustTable{$stretch,$shrink,$aftergsub};}
	;

stretch
	: STRETCH '{' juststeps '}' {$$=$juststeps;}
	;
shrink
	:  SHRINK '{' juststeps '}' {$$=$juststeps;}
	;
aftergsub
	: { $$ = {};}
	| AFTERGSUB '{' lookupreferences '}' { $$ = $lookupreferences;}
	;


juststeps
	: juststeps juststep { $$ = $1; $$.push_back($juststep);}
	| { $$ = {};}
	;

juststep
	: STEP '{'  lookupreferences '}' {$$ = JustTable::JustStep($lookupreferences);}
	| lookupreference ';' {$$ = JustTable::JustStep(std::vector<std::string>{$lookupreference});}
	;
lookupreferences
	: lookupreferences lookupreference ';' { $$ = $1; $$.push_back($lookupreference);}
	| lookupreference ';' { $$ = std::vector<std::string>({$lookupreference});}
	;

passes
	: passes pass_definition ';' {$$ = $1; $$->updatePass(std::move($pass_definition));}
	| pass_definition ';'  { $$ = new TableDefinition("substitution");$$->updatePass(std::move($pass_definition));}
	;

pass_definition
	: PASS '(' INT_LITERAL ')' rules  ENDPASS
		{
			if($INT_LITERAL < 1){
				error(yyla.location, std::string("Pass number cannot be less than 1 "));
				YYERROR;
			}
			$rules.setNumber($INT_LITERAL);
			$$ = std::move($rules);
		}


	;

rules
	: rules rule ';'  {$$ = std::move($1); $$.addRule($rule);}
	| rule ';' {$$ = Pass(); $$.addRule($rule);}
	;
rule
	: rule_lhs rule_rhs rule_context { $$ = GraphiteRule{$rule_lhs,$rule_rhs,$rule_context};}
	| rule_rhs rule_context { $$ = GraphiteRule{nullptr,$rule_rhs,$rule_context};}
	/*| rule_rhs { $$ = GraphiteRule{nullptr,$rule_rhs,nullptr};}*/
	| openttype_regexp  { $$ = $openttype_regexp;}
	;

openttype_regexp
	: rule_regex {$$ = GraphiteRule{nullptr,nullptr,$rule_regex};}
	| rule_regex[backtrack] '$' rule_regex[input] {$$ = GraphiteRule{$backtrack,nullptr,$input};}
	;

rule_lhs
	: rule_regex '>'  { $$ = $rule_regex;}
	;

rule_rhs
	: rule_regex  { $$ = $rule_regex;}
	;

rule_context
	: '/' rule_regex { $$ = $rule_regex;}
	;

rule_regex
	: rule_regex '|' prregexp_terc { $$ = std::make_shared<RuleRegExpOr>($1,$3);}
	| prregexp_terc	{ $$ = $prregexp_terc;}
	;

prregexp_terc
	: prregexp_terc prregexp_sec  { $$ = std::make_shared<RuleRegExpConcat>($1,$prregexp_sec);}
	| prregexp_sec	{ $$ = $prregexp_sec;}
	;

prregexp_sec
	: prregexp_sec '*'					{ $$ = std::make_shared<RuleRegExpRepeat>($1,RuleRegExpRepeatType::STAR);}
	| prregexp_sec '?'					{ $$ = std::make_shared<RuleRegExpRepeat>($1,RuleRegExpRepeatType::OPT);}
	| prregexp_sec '+'					{ $$ = std::make_shared<RuleRegExpRepeat>($1,RuleRegExpRepeatType::PLUS);}
	/*| prregexp_sec prregexp_action	{ $$ = std::make_shared<RuleRegExpConcat>($1,$prregexp_action);}	*/
	| prregexp_sec '^'					{ $$ = std::make_shared<RuleRegExpConcat>($1, std::make_shared<RuleRegExpAction>("StartMatch", RuleRegExpActionType::STARTNEWMATCH));}
	| prregexp_sec '{' INT_LITERAL[min] ',' INT_LITERAL[max] '}' {$$ = $1->makeRepetition($min,$max);}
	| prregexp_sec '{' INT_LITERAL[min] ',' '}' {$$ = $1->makeRepetition($min,-1);}
	| prregexp_sec '{' INT_LITERAL[val] '}' {$$ = $1->makeRepetition($val,$val);}
	| prregexp_prim						{ $$ = $prregexp_prim;}
	;

prregexp_prim
	: '(' rule_regex ')' { $$ = $rule_regex;}
	| glyphset { $$ = std::make_shared<RuleRegExpGlyphSet>($1);}
	| prregexp_action { $$ = $prregexp_action;}
	/*| ANY {$$ = std::make_shared<RuleRegExpANY>();}	*/
	;

prregexp_action
	: LOOKUP identifier {$$ = std::make_shared<RuleRegExpAction>($identifier, RuleRegExpActionType::LOOKUP);}
	| ACTION identifier {$$ = std::make_shared<RuleRegExpAction>($identifier, RuleRegExpActionType::ACTION);}
	;

%%


void feayy::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
