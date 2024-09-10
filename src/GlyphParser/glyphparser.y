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

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%define parse.trace true
%defines
%define api.namespace {glyphparser}
%define api.value.type variant
%define api.parser.class {Parser}
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};
%parse-param { class Driver& driver }
%parse-param { Glyph* parsingglyph }
%code top {

#include "glyphdriver.h"
#include "glyphscanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex
}

%code requires {
#include <glyph.hpp>
}


%token<int> T_INT
%token<double> T_NUMBER
%token<QString> T_EALPHA T_TT T_NUMVAR T_PAIRVAR
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_COMMA 
%token T_NEWLINE T_QUIT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%token T_ENDCHAR T_SEMICOLON T_GLYPHSECTION T_BIMAGESECTION T_BEGINCOMP_SECTION T_ENDCOMP_SECTION T_DRAWCOMPONENT T_BEGINPARA_SECTION T_AFFECT
%token T_PARAM_NUMERIC T_PARAM_LTENS T_PARAM_RTENS T_PARAM_LDIR T_PARAM_RDIR T_BEGINBODY_SECTION T_BEGINPATHS_SECTION T_ENDPATHS_SECTION
%token T_BEGINVERBATIM_SECTION T_ENDVERBATIM_SECTION
%token T_FILL "fill" T_CYCLE "cycle" T_CONTROLS "controls" T_AND "and" T_TENSION "tension" T_FILLC "fillc"
%token T_PP ".." T_DIR "dir" T_ATLEAST "atleast" T_CONTROLLEDPATH "controlledPath"

%token <QString> T_BEGINCHAR T_BODYCONTENT T_VERBATIM

%type	<Glyph::KnotEntryExit>				direction	controlpoint tensionamount
%type	<Glyph::Knot*>						link pathjoin point 
%type	<QMap<int, Glyph::Knot*> >			subpath	fillpath controlledpath
%type	<bool>								affect
%type <MFExprOperator> plus_minus times_over
%type <MFExpr*> constnum constpoint functionexpr varexpr primarymfexpr    secondarymfexpr tertiarymfexpr mfexpr 
%type <std::vector<MFExpr*>> mffuncparams

%start glyphs


%%

glyphs : 
 |  glyphs glyph 
;

glyph: beginchar backgroundimage componentsection paramsection verbatimsection pathssection bodysection T_ENDCHAR ';'   { /*printf("contents:%s\n", $T_GLYPHCONTENT);*/ }
;

beginchar: T_BEGINCHAR '(' T_EALPHA[name] ',' T_NUMBER[unicode] ',' T_NUMBER[width] ',' T_NUMBER[height] ',' T_NUMBER[depth]  ')' ';' 
{ 
	parsingglyph->setBeginMacroName($T_BEGINCHAR);
	parsingglyph->setName($name.trimmed());
	parsingglyph->setUnicode((int)$unicode);
	parsingglyph->setWidth((int)$width);
	parsingglyph->setHeight((int)$height);
	parsingglyph->setDepth((int)$depth); 	
}
;
;

backgroundimage: {Glyph::ImageInfo imageinfo; parsingglyph->setImage(imageinfo);}
| T_BIMAGESECTION T_EALPHA[path]  ',' T_NUMBER[x] ',' T_NUMBER[y] ',' T_NUMBER[t1] ',' T_NUMBER[t2] ',' T_NUMBER[t3] ',' T_NUMBER[t4]
{
	Glyph::ImageInfo imageinfo;
	imageinfo.path = $path;
	imageinfo.pos = QPointF($x, $y);
	QTransform transform($t1, $t2,$t3, $t4,0,0);		
	imageinfo.transform = transform;
	parsingglyph->setImage(imageinfo);
}
;

componentsection :  | T_BEGINCOMP_SECTION drawcomponents T_ENDCOMP_SECTION
;

drawcomponents : 
	| drawcomponents drawcomponent
;
	
drawcomponent: T_DRAWCOMPONENT '(' T_EALPHA[name] ',' T_NUMBER[x] ',' T_NUMBER[y] ',' T_NUMBER[t1] ',' T_NUMBER[t2] ',' T_NUMBER[t3] ',' T_NUMBER[t4] ')' ';'
{	
	parsingglyph->setComponent($name,$x,$y,$t1,$t2,$t3,$t4);
}
;

paramsection : | T_BEGINPARA_SECTION params

params : 
	| params param
;

param : T_EALPHA[name] affect mfexpr ';'	{parsingglyph->setParameter($name, $mfexpr,$affect);}
  | T_EALPHA[name] affect mfexpr ';' '%' T_EALPHA[dep] {parsingglyph->setParameter($name, $mfexpr,$affect,false,$dep);}
;



affect : T_AFFECT {$$ = false;}
| '=' {$$=true;}

pathssection : | T_BEGINPATHS_SECTION controlledpaths T_ENDPATHS_SECTION

controlledpaths : 
	| controlledpaths controlledpath
;

controlledpath : "controlledPath" '(' T_NUMBER[numpath] ',' T_NUMBER[numpoint] {driver.numpoint = $numpoint;} ')' '(' T_EALPHA[name] ')' '(' fillpath ')'  ';' 
{
	parsingglyph->controlledPaths.insert($numpath,$fillpath);
	parsingglyph->controlledPathNames.insert($numpath,$name);
}
| T_FILL {driver.numpoint = 0;} fillpath ';' 
{
	int count = parsingglyph->controlledPaths.count();
	parsingglyph->controlledPaths.insert(count,$fillpath);
	parsingglyph->controlledPathNames.insert(count,"fill");
}
| T_FILLC {driver.numpoint = 0;} fillpath ';' 
{
	int count = parsingglyph->controlledPaths.count();
	parsingglyph->controlledPaths.insert(count,$fillpath);
	parsingglyph->controlledPathNames.insert(count,"fillc");
}
;

fillpath : subpath 
{
	//$subpath.last()->rightValue = $pathjoin->leftValue; 
	//$subpath.first()->leftValue = $pathjoin->rightValue;
	//$subpath.insert($subpath.lastKey() + 1, $subpath.first());
	//delete $pathjoin;
	$$ = $subpath;
}
;

subpath : point {$$.insert(driver.numpoint,$point);}
| subpath pathjoin point {	
	$1.last()->rightValue = $pathjoin->leftValue; 
	if($point->expr->toString() == "cycle" && $1.firstKey() == 0){
		delete $point;
		$point = $1.first();
	}
	$point->leftValue = $pathjoin->rightValue; 
	delete $pathjoin;
	if($point->leftValue.macrovalue == "leftjoin"){
		$1.insert($1.lastKey() + 4,$point);
	}else if($point->leftValue.macrovalue == "rightjoin"){
		$1.insert($1.lastKey() + 4,$point);
	}else if($point->leftValue.macrovalue == "setranchor"){
		$1.insert($1.lastKey() + 3,$point);
	}else if($point->leftValue.macrovalue == "endlink"){
		$1.insert($1.lastKey() + 2,$point);
	}else if($point->leftValue.macrovalue == "link"){
		$1.insert($1.lastKey() + 3,$point);
	}
	else{
		$1.insert($1.lastKey() + 1,$point);
	}
	
	$$ = $1;
}
;

pathjoin : direction[d1] link direction[d2] {
	$$ = $link;
	if($link->leftValue.type != Glyph::mpgui_explicit){
		$link->leftValue.type = $d1.type;		
		if($d1.dirExpr){
			$link->leftValue.dirExpr = $d1.dirExpr->clone();
		}

		$link->rightValue.type = $d2.type;		
		if($d2.dirExpr){
			$link->rightValue.dirExpr = $d2.dirExpr->clone();
		}
		
	}
	
}
;

direction : 
	{$$ = {}; $$.type = Glyph::mpgui_open;}	
	| '{' mfexpr '}' {
		$$ = {};
		$$.type = Glyph::mpgui_given;		
		$$.dirExpr = std::unique_ptr<MFExpr>($mfexpr);
	}
;



link : ".." "controls" controlpoint[c] ".." {
		$$ = new Glyph::Knot();
		$$->leftValue = $c;
		$$->rightValue = $c;		
		$$->leftValue.isEqualAfter = true;
		$$->rightValue.isEqualBefore = true;
	}
| ".." "controls"  controlpoint[c1] "and" controlpoint[c2] ".." {$$ = new Glyph::Knot(); $$->leftValue = $c1; $$->rightValue = $c2;}
| ".." "tension" tensionamount[c] ".." {$$ = new Glyph::Knot();$$->leftValue = $c; $$->rightValue = $c; $$->leftValue.isEqualAfter = true; $$->rightValue.isEqualBefore = true;}
| ".." "tension"  tensionamount[c1] "and" tensionamount[c2] ".." {$$ = new Glyph::Knot(); $$->leftValue = $c1; $$->rightValue = $c2;}
| ".." {$$ = new Glyph::Knot(); $$->leftValue.jointtype = $$->rightValue.jointtype = Glyph::path_join_tension;}
| T_TT[value] {
	$$ = new Glyph::Knot(); 
	$$->leftValue.type = Glyph::mpgui_curl;
	$$->leftValue.macrovalue = $value; 
	$$->rightValue.type = Glyph::mpgui_curl;
	$$->rightValue.macrovalue = $value;
	$$->leftValue.jointtype = $$->rightValue.jointtype = Glyph::path_join_macro;
}
| T_EALPHA[value] {
	$$ = new Glyph::Knot(); 
	$$->leftValue.type = Glyph::mpgui_curl;
	$$->leftValue.macrovalue = $value; 
	$$->rightValue.type = Glyph::mpgui_curl;
	$$->rightValue.macrovalue = $value;
	$$->leftValue.jointtype = $$->rightValue.jointtype = Glyph::path_join_macro;
}
;

point : mfexpr {$$ = new Glyph::Knot(); $$->expr = std::unique_ptr<MFExpr>($mfexpr);}
;

mfexpr : tertiarymfexpr[expr] {$$ = $expr;}
;

primarymfexpr :	varexpr {$$ = $varexpr;}
	| constpoint {$$ = $constpoint;}
	| functionexpr {$$ = $functionexpr;}
	| plus_minus primarymfexpr[expr] {$$ = new ScalarMultiMFExp($expr,$plus_minus);}
	| constnum {$$ = $constnum;}
	| T_DIR primarymfexpr[expr] {$$ = new DirPathPointExp($expr);}
	| '(' mfexpr ')' {$$ = new ParenthesesMFExp($mfexpr);}
;

secondarymfexpr : primarymfexpr {$$ = $primarymfexpr;}
	| secondarymfexpr[left] times_over primarymfexpr[right] {$$ =  new BinOpMFExp($left,$times_over,$right);}
	;


tertiarymfexpr : secondarymfexpr[expr] {$$ = $expr;}
	| tertiarymfexpr[left] plus_minus secondarymfexpr[right] {$$ =  new BinOpMFExp($left,$plus_minus,$right);}
	;


varexpr : T_EALPHA[var] {
	auto isParam = false;
	if(parsingglyph->params.find($var) != parsingglyph->params.end()){
		isParam = true;
		parsingglyph->params[$var].isInControllePath = true;
	}else if(parsingglyph->dependents.contains($var)){
		auto param = parsingglyph->dependents[$var];
		isParam = true;
		param->isInControllePath = true;
	}
	$$ =  new VarMFExpr($var,isParam);
	
};

constpoint : '(' mfexpr[x] ',' mfexpr[y] ')' {	
	$$ = new PairPathPointExp($x,$y);
};

functionexpr : T_EALPHA[functionName]'(' mffuncparams ')' {		
	$$ =  new FunctionMFExp($functionName,$mffuncparams);	
};

mffuncparams : mfexpr { $$.push_back($1);}
| mffuncparams ',' mfexpr {$$ = std::move($1);$$.push_back($3);}
;

constnum : T_NUMBER[x] {	
	$$ = new LitPathNumericExp($x);
};

times_over : '*' {$$ = MFExprOperator::TIMES;}
| '/' {$$ = MFExprOperator::OVER;}
;

plus_minus : '+' {$$ = MFExprOperator::PLUS;}
| '-' {$$ = MFExprOperator::MINUS;}
;


controlpoint : primarymfexpr {$$ = {};  $$.dirExpr = std::unique_ptr<MFExpr>($primarymfexpr) ;$$.type = Glyph::mpgui_explicit;$$.jointtype = Glyph::path_join_control;}
;

tensionamount : primarymfexpr {$$ = {}; $$.tensionExpr = std::unique_ptr<MFExpr>($primarymfexpr); $$.jointtype = Glyph::path_join_tension;}
| T_ATLEAST primarymfexpr {$$ = {}; $$.tensionExpr = std::unique_ptr<MFExpr>($primarymfexpr);$$.isAtleast = true;$$.jointtype = Glyph::path_join_tension;}
;

verbatimsection : | T_BEGINVERBATIM_SECTION T_VERBATIM T_ENDVERBATIM_SECTION
{
	parsingglyph->setVerbatim($T_VERBATIM);
}
;

bodysection : | T_BEGINBODY_SECTION T_BODYCONTENT
{
	parsingglyph->setBody($T_BODYCONTENT,false);
}
;
%%


void glyphparser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}
