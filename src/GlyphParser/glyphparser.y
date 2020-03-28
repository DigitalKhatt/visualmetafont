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
%token<QString> T_EALPHA T_TT
%token T_PLUS T_MINUS T_MULTIPLY T_DIVIDE T_LEFT T_RIGHT T_COMMA 
%token T_NEWLINE T_QUIT
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE

%token T_ENDCHAR T_SEMICOLON T_GLYPHSECTION T_BIMAGESECTION T_BEGINCOMP_SECTION T_ENDCOMP_SECTION T_DRAWCOMPONENT T_BEGINPARA_SECTION T_AFFECT
%token T_PARAM_NUMERIC T_PARAM_LTENS T_PARAM_RTENS T_PARAM_LDIR T_PARAM_RDIR T_BEGINBODY_SECTION T_BEGINPATHS_SECTION T_ENDPATHS_SECTION
%token T_BEGINVERBATIM_SECTION T_ENDVERBATIM_SECTION
%token T_FILL "fill" T_CYCLE "cycle" T_CONTROLS "controls" T_AND "and" T_TENSION "tension"
%token T_PP ".." T_DIR "dir" T_ATLEAST "atleast" T_CONTROLLEDPATH "controlledPath"

%token <QString> T_BEGINCHAR T_BODYCONTENT T_VERBATIM

%type	<Glyph::KnotEntryExit>				direction	controlpoint tensionpoint
%type	<Glyph::Knot*>						link pathjoin point constpointwitheq
%type	<QMap<int, Glyph::Knot*> >			subpath	fillpath controlledpath
%type	<bool>								affect
%type <QString> opinsidepoint;



%start glyphs


%%

glyphs : 
 |  glyphs glyph 
;

glyph: beginchar backgroundimage componentsection paramsection verbatimsection pathssection bodysection T_ENDCHAR ';'   { /*printf("contents:%s\n", $T_GLYPHCONTENT);*/ }
;

beginchar: T_BEGINCHAR '(' T_EALPHA[name] ',' T_NUMBER[unicode] ',' T_NUMBER[width] ',' T_NUMBER[height] ',' T_NUMBER[depth] ',' T_NUMBER[lefttatweel] ',' T_NUMBER[righttatweel] ')' ';' 
{ 
	parsingglyph->setBeginMacroName($T_BEGINCHAR);
	parsingglyph->setName($name.trimmed());
	parsingglyph->setUnicode((int)$unicode);
	parsingglyph->setWidth((int)$width);
	parsingglyph->setHeight((int)$height);
	parsingglyph->setDepth((int)$depth); 
	parsingglyph->setleftTatweel($lefttatweel); 
	parsingglyph->setrightTatweel($righttatweel); 
}
|T_BEGINCHAR '(' T_EALPHA[name] ',' T_NUMBER[unicode] ',' T_NUMBER[width] ',' T_NUMBER[height] ',' T_NUMBER[depth]  ')' ';' 
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

param : T_EALPHA[name] affect '(' T_NUMBER[x] ',' T_NUMBER[y] ')' ';'	{parsingglyph->setParameter($name, Glyph::point,$x,$y,Glyph::left,0,0,$affect);}
	|	T_EALPHA[name] affect  T_NUMBER[x]  ';'	T_PARAM_NUMERIC	{parsingglyph->setParameter($name, Glyph::numeric,$x,0,Glyph::left,0,0,$affect);}
	|	T_EALPHA[name] affect  T_NUMBER[x]  ';'	T_PARAM_LTENS '(' T_NUMBER[numpath] ',' T_NUMBER[numpoint] ')'	{parsingglyph->setParameter($name, Glyph::tension,$x,0,Glyph::left,$numpath,$numpoint,$affect);}
	|	T_EALPHA[name] affect  T_NUMBER[x]  ';'	T_PARAM_RTENS '(' T_NUMBER[numpath] ',' T_NUMBER[numpoint] ')'	{parsingglyph->setParameter($name, Glyph::tension,$x,0,Glyph::right,$numpath,$numpoint,$affect);}
	|	T_EALPHA[name] affect  T_NUMBER[x]  ';'	T_PARAM_LDIR '(' T_NUMBER[numpath] ',' T_NUMBER[numpoint] ')'	{parsingglyph->setParameter($name, Glyph::direction,$x,0,Glyph::left,$numpath,$numpoint,$affect);}
	|	T_EALPHA[name] affect  T_NUMBER[x]  ';'	T_PARAM_RDIR '(' T_NUMBER[numpath] ',' T_NUMBER[numpoint] ')'	{parsingglyph->setParameter($name, Glyph::direction,$x,0,Glyph::left,$numpath,$numpoint,$affect);}
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
	if($point->value == "cycle" && $1.firstKey() == 0){
		delete $point;
		$point = $1.first();
	}
	$point->leftValue = $pathjoin->rightValue; 
	delete $pathjoin;
	if($point->leftValue.value == "join"){
		$1.insert($1.lastKey() + 4,$point);
	}else if($point->leftValue.value == "endlink"){
		$1.insert($1.lastKey() + 2,$point);
	}
	else{
		$1.insert($1.lastKey() + 1,$point);
	}
	
	$$ = $1;
}
;

pathjoin : direction[d1] link direction[d2] {
	$$ = $link;
	if($link->leftValue.type != Glyph::mpgui_explicit && $link->leftValue.type != Glyph::mpgui_curl){
		$link->leftValue.type = $d1.type;
		$link->leftValue.isDirConstant = $d1.isDirConstant;
		$link->leftValue.value = $d1.value;
		$link->leftValue.x = $d1.x;

		$link->rightValue.type = $d2.type;
		$link->rightValue.isDirConstant = $d2.isDirConstant;
		$link->rightValue.value = $d2.value;
		$link->rightValue.x = $d2.x;
	}
	
}
;

direction : 
		{$$ = {}; $$.type = Glyph::mpgui_open;/*driver.lexer->begin_dir_state();*/}
	| '{' T_DIR T_NUMBER '}' {
		$$ = {};
		$$.type = Glyph::mpgui_given;
		$$.x = $T_NUMBER;
		$$.isDirConstant = true;
		}
	| '{' T_EALPHA '}' {
		$$ = {};
		$$.type = Glyph::mpgui_given;
		$$.value = $T_EALPHA;
		$$.isDirConstant = false;
		}
;



link : ".." "controls" controlpoint[c] ".." {$$ = new Glyph::Knot();$$->leftValue = $c; $$->rightValue = $c; $$->rightValue.isControlConstant = false; $$->leftValue.isEqualAfter = true; $$->rightValue.isEqualBefore = true;}
| ".." "controls"  controlpoint[c1] "and" controlpoint[c2] ".." {$$ = new Glyph::Knot(); $$->leftValue = $c1; $$->rightValue = $c2;}
| ".." "tension" tensionpoint[c] ".." {$$ = new Glyph::Knot();$$->leftValue = $c; $$->rightValue = $c; $$->rightValue.isControlConstant = false; $$->leftValue.isEqualAfter = true; $$->rightValue.isEqualBefore = true;}
| ".." "tension"  tensionpoint[c1] "and" tensionpoint[c2] ".." {$$ = new Glyph::Knot(); $$->leftValue = $c1; $$->rightValue = $c2;}
| ".." {$$ = new Glyph::Knot(); $$->leftValue.y = 1; $$->rightValue.y = 1;}
| T_TT[value] {
	$$ = new Glyph::Knot(); 
	$$->leftValue.type = Glyph::mpgui_curl;
	$$->leftValue.value = $value; 
	$$->rightValue.type = Glyph::mpgui_curl;
	$$->rightValue.value = $value;  
}
| T_EALPHA[value] {
	$$ = new Glyph::Knot(); 
	$$->leftValue.type = Glyph::mpgui_curl;
	$$->leftValue.value = $value; 
	$$->rightValue.type = Glyph::mpgui_curl;
	$$->rightValue.value = $value; 
}
;

point : '(' T_NUMBER[x] ',' T_NUMBER[y] ')' {$$ = new Glyph::Knot(); $$->isConstant = true; $$->x = $x; $$->y = $y;}
| '(' constpointwitheq ')' {$$ = $constpointwitheq;}
| constpointwitheq {$$ = $constpointwitheq;}
| T_EALPHA[left] opinsidepoint T_EALPHA[right] {
	$$ = new Glyph::Knot(); 
	$$->isConstant = false; 
	$$->value = $left + " " + $opinsidepoint + " " + $right;
	if(parsingglyph->params.contains($left)){
		Glyph::Param& val = parsingglyph->params[$left];		
		val.isInControllePath = true;		
		$$->paramName = $left;
	}else if(parsingglyph->params.contains($right)){
		Glyph::Param& val = parsingglyph->params[$right];		
		val.isInControllePath = true;		
		$$->paramName = $right;
	}
}
| T_EALPHA {
	$$ = new Glyph::Knot(); 
	$$->isConstant = false; 
	$$->value = $T_EALPHA;
	if(parsingglyph->params.contains($T_EALPHA)){
		Glyph::Param val = parsingglyph->params.value($T_EALPHA);		
		val.isInControllePath = true;
		parsingglyph->params[$T_EALPHA] = val;
		$$->paramName = $T_EALPHA;
	}
}
;

constpointwitheq : '(' T_NUMBER[x] ',' T_NUMBER[y] ')' opinsidepoint T_EALPHA[right] {$$ = new Glyph::Knot(); $$->isConstant = true; $$->x = $x; $$->y = $y;$$->value = $opinsidepoint + " " + $right;};

opinsidepoint : '+' {$$ = "+";}
| '-' {$$ = "-";}
;

controlpoint : '(' T_NUMBER[x] ',' T_NUMBER[y] ')' {$$ = {}; $$.isControlConstant = true; $$.x = $x; $$.y = $y;$$.type = Glyph::mpgui_explicit;}
| T_EALPHA {$$ = {}; $$.isControlConstant = false; $$.controlValue = $T_EALPHA;$$.type = Glyph::mpgui_explicit;}
;

tensionpoint : T_NUMBER[y] {$$ = {}; $$.isControlConstant = true; $$.y = $y;}
| T_EALPHA {$$ = {}; $$.isControlConstant = false; $$.controlValue = $T_EALPHA;}
| T_ATLEAST T_NUMBER[y] {$$ = {}; $$.isControlConstant = true; $$.y = $y;$$.isAtleast = true;}
| T_ATLEAST T_EALPHA {$$ = {}; $$.isControlConstant = false; $$.controlValue = $T_EALPHA; $$.isAtleast = true;}
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
