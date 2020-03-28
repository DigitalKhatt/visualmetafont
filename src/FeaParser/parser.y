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
%glr-parser
%require "3.0.4"
%define parse.trace
%defines
%define api.namespace {feayy}
%define api.value.type union
/*%define parser_class_name {Parser}*/
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
}

%code requires {
#include "feaast.h"
using namespace std;
using namespace feayy;
}


%token<int> INT_LITERAL CID
%token<double> DOUBLE_LITERAL
%token<std::string *> IDENTIFIANT REGEXP GLYPHNAME

%token LOOKUP FEATURE POSITION SUBSTITUTE BASE ANCHOR MARK MARKCLASS CURSIVE T_NULL NOTDEF FUNCTION BY FROM COLOR CALLBACK EXPANSION ADD STARTLIG ENDLIG
%token LOOKUPFLAG RightToLeft IgnoreBaseGlyphs IgnoreLigatures IgnoreMarks MarkAttachmentType UseMarkFilteringSet

%type <FeaRoot*> root
%type <vector<Statement *> *> statements
%type <Statement*> statement
%type <Glyph*> glyph
%type <ClassName*> classname
%type <ClassComponent*> classcomponent
%type <GlyphClass*> glyphclass
%type <vector<ClassComponent *>*> classcomponents
%type <GlyphSet*> glyphset
%type <std::string *> lookupreference explicitlookup featurereference identifier featuretag
%type <ValueRecord> valuerecord
%type <LookupFlag*> flag flags lookupflag
%type <LookupStatement*> feature_statement lookup_statement lookup_definition markclassdefinition gsubrule gposrule singlesub multiplesub alternatesub ligaturesub classdefinition
%type <LookupStatement*> singleadjustment cursiverule mark2base mark2mark 
%type <vector<LookupStatement *> *> lookup_statements feature_statements
%type <MarkedGlyphSetRegExp*> markedglyphsetregexp
%type <vector<MarkedGlyphSetRegExp *>*> inputseq
%type <GlyphSetRegExp*> glyphsetregexp 
%type <ChainingContextualRule*> contextualexplicit contextualgpos contextualgsub
%type <Anchor*> anchor anchorformat
%type <Mark2BaseClass*> mark2baseclass
%type <vector<Mark2BaseClass *>*> mark2baseclasses
%type <double> doubleorint
%type <GlyphExpansion> expafactor
%type <FeatureDefenition*> feature_definition;
%type <vector<Glyph *>*> glyphseq;
%type <StartEndLig> startendlig;

%destructor { delete $$; } <std::string *>

%start root

%%

root
	: statements { $$ = new FeaRoot($1); driver.context.root = $$; }
	;

statements
	:  { $$ = new vector<Statement *>(); }  /* empty */
	| statements statement ';'
		{
			$$ = $1;
			$1->push_back($2);
		}
	;

statement
	: lookup_definition {$$ = $1;}
	| markclassdefinition {$$ = $1;}
	| classdefinition {$$ = $1;}
	| feature_definition {$$ = $1;}
	;

feature_definition
	: FEATURE featuretag[bname] '{' feature_statements[lstm] '}' featuretag[lname]
	{
		if(*$bname != *$lname){
			yyparser.error(yylloc, std::string("Feature name '") + *$bname + "' does not match with '" + *$lname + "'");
			delete $lstm;
			delete $bname;
			delete $lname;
			YYERROR;
		}else{
			$$ = new FeatureDefenition($bname,$lstm);
			driver.context.features[*$bname] = $$;
		}
	}
	;

feature_statements
	: feature_statement ';' { $$ = new vector<LookupStatement *>(); $$->push_back($1); }
	| feature_statements feature_statement ';'
		{
			$$ = $1;
			$1->push_back($2);
		}
	;

feature_statement	
	: lookup_definition {$$ = $1;}
	| lookupreference { $$ = new LookupReference($1);}
	| classdefinition {$$ = $1;}
	;

lookup_definition
	: LOOKUP identifier[bname] '{' lookup_statements[lstm] '}' identifier[lname] 
		{ 			
			if(*$bname != *$lname){
				yyparser.error(yylloc, std::string("Lookup name '") + *$bname + "' does not match with '" + *$lname + "'");
				delete $lstm;
				delete $bname;
				delete $lname;
				YYERROR;
			}else{
				auto lookup = new LookupDefinition($bname,$lstm); 
				driver.context.lookups[*$bname] = lookup;
				$$ = lookup;
			}
			
		}
	;

lookup_statements
	: lookup_statement ';' { $$ = new vector<LookupStatement *>(); $$->push_back($1); }
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
	;


lookupflag
	: LOOKUPFLAG INT_LITERAL	{$$ = new LookupFlag($2);}
	| LOOKUPFLAG flags			{$$ = $2;}
	;

flags
	: flag			{$$ = $1;}
	| flags flag	{*$1 | *$2; $$ = $1; /*delete $2;*/}
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
	| ligaturesub {$$ = $1;}	 
	| contextualgsub {$$ = $1;}
	;

singlesub
	: SUBSTITUTE glyph BY glyph	{$$ = new SingleSubstituionRule($2,$4,2);}
	| SUBSTITUTE CALLBACK glyph {$$ = new SingleSubstituionRule($glyph,$glyph,10);}
	| SUBSTITUTE glyphset ADD doubleorint[lefttatweel] doubleorint[righttatweel]  {$$ = new SingleSubstituionRule($glyphset,$lefttatweel,$righttatweel);}	
	| SUBSTITUTE CALLBACK glyph[glyph1] BY glyph[glyph2] {$$ = new SingleSubstituionRule($glyph1,$glyph2,10);}
	| SUBSTITUTE CALLBACK glyph startendlig expafactor[factor] {$$ = new SingleSubstituionRule($glyph,$glyph,10,$factor,$startendlig);}
	| SUBSTITUTE CALLBACK glyph[glyph1] BY glyph[glyph2] startendlig expafactor[factor] {$$ = new SingleSubstituionRule($glyph1,$glyph2,10,$factor,$startendlig);}
	| SUBSTITUTE glyphclass BY glyph
	| SUBSTITUTE glyphclass BY glyphclass
	;

expafactor:
	EXPANSION doubleorint[minleft] doubleorint[maxleft] doubleorint[minright] doubleorint[maxright] {$$ = {(float)$minleft,(float)$maxleft,(float)$minright,(float)$maxright};}
	;

startendlig:
	/* empty */ {$$ = StartEndLig::StartEnd;}
	| STARTLIG {$$ = StartEndLig::Start;}
	| ENDLIG {$$ = StartEndLig::End;}
	;

multiplesub
	: SUBSTITUTE glyph BY glyph glyphseq
	;

alternatesub
	: SUBSTITUTE glyph FROM glyphclass
	;

doubleorint
	: DOUBLE_LITERAL | INT_LITERAL {$$ = $1;}
	;

glyphseq
	: glyph { $$ = new vector<Glyph *>{$1}; }	
	| glyphseq glyph {$$ = $1; $$->push_back($glyph);}
	;



ligaturesub
	: SUBSTITUTE glyphseq glyph[gly]  BY glyph[ligature] { $glyphseq->push_back($gly); $$ = new LigatureSubstitutionRule($glyphseq,$ligature);}
	;

contextualgsub	
	: SUBSTITUTE contextualexplicit {$$ = $2;$$->setType(true);}
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
	| cursiverule {$$ = $1;}
	| mark2base {$$ = $1;}
	| mark2mark {$$ = $1;}
	| contextualgpos {$$ = $1;}
	;

singleadjustment
	: POSITION glyphset valuerecord {$$ = new SingleAdjustmentRule($glyphset,$valuerecord,false);}
	| POSITION glyphset COLOR valuerecord {$$ = new SingleAdjustmentRule($glyphset,$valuerecord,true);}
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



%left '|';
%precedence CID IDENTIFIANT REGEXP GLYPHNAME NOTDEF '[' '@' T_NULL;
%precedence GLYPHSET;
%precedence  '(';



glyphsetregexp
	: glyphset { $$ = new GlyphSetRegExpSingle($1); } 
	| glyphsetregexp glyphset { $$ = new GlyphSetRegExpSeq{$1,$glyphset};}	
	| glyphsetregexp[left] '|' glyphsetregexp[right]  { $$ = new GlyphSetRegExpOr{$left,$right};}	
	| glyphsetregexp '(' glyphsetregexp ')' { $$ = new GlyphSetRegExpSeq{$1,$3};}	
	| '(' glyphsetregexp ')' { $$ = $2;}	
	;

inputseq
	: markedglyphsetregexp  { $$ = new vector<MarkedGlyphSetRegExp *>();$$->push_back($1); } 		
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
	: /* empty */	{$$ = nullptr;}
	| lookupreference {$$ = $1;}	
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
	: '@' identifier '=' '[' classcomponents ']' {$$ = new ClassDefinition($identifier,new GlyphClass($classcomponents));}
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

glyphclass
	: '[' classcomponents ']'	{$$ = new GlyphClass($2);}
	| classname					{$$ = new GlyphClass($1);}		
	| REGEXP					{$$ = new GlyphClass(new RegExpClass($1));}
	;

classcomponents	
	: classcomponent					{ $$ = new std::vector<ClassComponent *>{ $1};}
	| classcomponents  classcomponent	{ $1->push_back($2);$$ = $1;}
	;

classcomponent	
	: glyph			{$$ = $1;}
	| classname		{$$ = $1;}
	| REGEXP		{$$ = new RegExpClass($1);}
	;


glyph	
	: identifier	{$$ = new GlyphName($1);}
	| GLYPHNAME		{$$ = new GlyphName($1);}
	| NOTDEF		{$$ = new GlyphName(new std::string(".notdef"));}
	| CID			{$$ = new GlyphCID($1);}	
	;

classname	
	: '@' identifier {$$ = new ClassName($2);}
	;

valuerecord
	: INT_LITERAL													{$$ = {(qint16)$1,0,0,0};}
	| '<' INT_LITERAL '>'											{$$ = {(qint16)$2,0,0,0};}
	| '<' INT_LITERAL  INT_LITERAL  INT_LITERAL INT_LITERAL '>'		{$$ = {(qint16)$2,(qint16)$3,(qint16)$4,(qint16)$5};}
	;

lookupreference
	: LOOKUP identifier	{$$ = $2;}
	;

featurereference
	: FEATURE featuretag	{$$ = $2;}
	;

featuretag
	: IDENTIFIANT {$$ = $1;}
	| MARK	 {$$ = new std::string("mark");}
	;

identifier
	: IDENTIFIANT	{$$ = $1;}
	;

%%


void feayy::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
