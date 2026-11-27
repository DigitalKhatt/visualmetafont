/*
 * Copyright (c) 2015-2020 DigitalKhatt contributors
 * SPDX-License-Identifier: MIT
 * See LICENSE in the repository root for the full license text.
 */

%option noyywrap nounput noinput batch c++
%option prefix="Glyph"
%option never-interactive
/*%option debug*/
%{
#include "glyphscanner.h"
#include "glyphdriver.h"
using token = glyphparser::Parser::token;
#define YY_NO_UNISTD_H
%}

%x INSIDE_CHAR_CONTENTS
%x INSIDE_VERBATIM
%x INSIDE_DIR

%%

[ \r\n\t]*													; // ignore all whitespace
[-+]?([0-9]*\.[0-9]+|[0-9]+)								{yylval->build<double>(atof(yytext)); return token::T_NUMBER;}
[(),{};=+-/*%]													{return yytext[0];}
":="														{return token::T_AFFECT;}
"%%glyphname:"												{return token::T_GLYPHSECTION;}
"%%backgroundimage:"										{return token::T_BIMAGESECTION;}
"%%begincomponents"											{return token::T_BEGINCOMP_SECTION;}
"%%endcomponents"											{return token::T_ENDCOMP_SECTION;}
"%%beginparams"												{return token::T_BEGINPARA_SECTION;}
"%%beginpaths"												{return token::T_BEGINPATHS_SECTION;}
"%%endpaths"												{return token::T_ENDPATHS_SECTION;}
"controlledPath"											{return token::T_CONTROLLEDPATH;}
"controlledImage"											{return token::T_CONTROLLEDIMAGE;}
"fill"														{return token::T_FILL;}
"fillc"														{return token::T_FILLC;}
"draw"														{return token::T_DRAW;}
"dir"														{return token::T_DIR;}
"controls"													{return token::T_CONTROLS;}
"tension"													{return token::T_TENSION;}
"and"														{return token::T_AND;}
".."														{return token::T_PP;}
"--"|"..."|"---"								{yylval->build<QString>(yytext); return token::T_TT;}
"atleast"													{return token::T_ATLEAST;}
"%%beginbody"[\r\n]											{BEGIN(INSIDE_CHAR_CONTENTS);return token::T_BEGINBODY_SECTION;}
"%"[ \t]*"numeric"											{return token::T_PARAM_NUMERIC;}
"%"[ \t]*"ltens"											{return token::T_PARAM_LTENS;}
"%"[ \t]*"rtens"											{return token::T_PARAM_RTENS;}
"%"[ \t]*"ldir"												{return token::T_PARAM_LDIR;}
"%"[ \t]*"rdir"												{return token::T_PARAM_RDIR;}
"drawcomponent"												{return token::T_DRAWCOMPONENT;}
"exit"														{return token::T_QUIT;}
"quit"														{return token::T_QUIT;}
"beginchar"|"defchar"										{yylval->build<QString>(yytext); return token::T_BEGINCHAR;}
<INITIAL,INSIDE_CHAR_CONTENTS>"endchar"|"enddefchar"		{BEGIN(INITIAL);return token::T_ENDCHAR;}
<INSIDE_CHAR_CONTENTS>(.|\n)/"endchar"|"enddefchar"			{yylval->build<QString>(yytext); return token::T_BODYCONTENT;}
<INSIDE_CHAR_CONTENTS>.|\n									{yymore();} //<INSIDECHAR>[^e]+				{yymore();}
"%%beginverbatim"[\r\n]										{BEGIN(INSIDE_VERBATIM);return token::T_BEGINVERBATIM_SECTION;}
<INSIDE_VERBATIM>(.|\n)/"%%endverbatim"						{yylval->build<QString>(yytext); return token::T_VERBATIM;}
<INSIDE_VERBATIM>.|\n										{yymore();}
<INSIDE_VERBATIM>"%%endverbatim"							{BEGIN(INITIAL);return token::T_ENDVERBATIM_SECTION;}
[a-zA-Z_][a-zA-Z\.0-9\/:_\[\]]*									{yylval->build<QString>(yytext); return token::T_EALPHA;}
<INSIDE_DIR>{
[{}]													{return yytext[0];}
[^{}]													{yylval->build<QString>(yytext);/*auto tt = driver.glyph.params[yylval->as<QString>()];*/ return token::T_EALPHA;}
}

%% /*** Additional Code ***/

namespace glyphparser {

Scanner::Scanner(Driver& driver,std::istream* in,
		 std::ostream* out)
    : GlyphFlexLexer(in, out),driver{driver}
{
}

Scanner::~Scanner()
{
}

void Scanner::set_debug(bool b)
{
    yy_flex_debug = b;
}

void Scanner::begin_dir_state(void) {
    BEGIN(INSIDE_DIR);
}

void Scanner::end_dir_state(void) {
    BEGIN(INITIAL);
}


}

#ifdef yylex
#undef yylex
#endif

int yyFlexLexer::yylex()
{
    std::cerr << "in yyFlexLexer::yylex() !" << std::endl;
    return 0;
}




