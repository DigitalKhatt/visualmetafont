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

%option noyywrap nounput noinput debug batch c++ 
%option prefix="Fea"
%option never-interactive

%{
#include "scanner.h"
#include "driver.h"
using token = feayy::Parser::token;
#define YY_USER_ACTION  yylloc->columns(yyleng);
#define YY_NO_UNISTD_H
%}

%%

%{
    // reset location
    yylloc->step();
%}


[ \t\r]+													{yylloc->step();} /* gobble up white-spaces */
("#"|"//").*												{yylloc->step();} /* consume //-comment */ 
[\n]+														{yylloc->lines(yyleng); yylloc->step();} /* gobble up end-of-lines */
[-+]?[0-9]+													{/*yylval->INT_LITERAL = atoi(yytext); */ yylval->emplace<int>(atoi(yytext)); return token::INT_LITERAL;}
[-+]?[0-9]+"."[0-9]*										{/*yylval->DOUBLE_LITERAL = atof(yytext); */  yylval->emplace<double>(atof(yytext));return token::DOUBLE_LITERAL;}
"lookup"													{return(token::LOOKUP);}
"action"													{return(token::ACTION);}
"feature"													{return(token::FEATURE);}
"pos"														{return(token::POSITION);}
"just"														{return(token::JUST);}
"stretch"													{return(token::STRETCH);}
"shrink"													{return(token::SHRINK);}
"aftergsub"													{return(token::AFTERGSUB);}
"end"													    {return(token::END);}
"step"													    {return(token::STEP);}
"position"													{return(token::POSITION);}
"substitute"												{return(token::SUBSTITUTE);}
"sub"														{return(token::SUBSTITUTE);}
"callback"													{return(token::CALLBACK);}
"expfactors"												{return(token::EXPANSION);}
"startlig"												    {return(token::STARTLIG);}
"endlig"												    {return(token::ENDLIG);}
"endkashida"												{return(token::ENDKASHIDA);}
"lookupflag"												{return(token::LOOKUPFLAG);}
"RightToLeft"												{return(token::RightToLeft);} 
"IgnoreBaseGlyphs"											{return(token::IgnoreBaseGlyphs);} 
"IgnoreLigatures"											{return(token::IgnoreLigatures);}  
"IgnoreMarks"												{return(token::IgnoreMarks);}  
"MarkAttachmentType"										{return(token::MarkAttachmentType);}  
"UseMarkFilteringSet"										{return(token::UseMarkFilteringSet);} 
"cursive"													{return(token::CURSIVE);} 
"color"														{return(token::COLOR);}
"anchor"													{return(token::ANCHOR);} 
"NULL"														{return(token::T_NULL);} 
".notdef"													{return(token::NOTDEF);} 
"function"													{return(token::FUNCTION);} 
"base"														{return(token::BASE);} 
"mark"														{return(token::MARK);} 
"markClass"													{return(token::MARKCLASS);} 
"by"														{return(token::BY);} 
"add"														{return(token::ADD);} 
"from"														{return(token::FROM);} 
"table"														{return(token::TABLE);} 
"endtable"													{return(token::ENDTABLE);} 
"pass"														{return(token::PASS);} 
"endpass"													{return(token::ENDPASS);} 
"any"													    {return(token::ANY);} 
[a-zA-Z_][a-zA-Z\.0-9_]*									{/*yylval->IDENTIFIANT = new std::string(yytext, yyleng);*/  yylval->emplace<std::string>(std::string(yytext, yyleng)); return token::IDENTIFIANT;}
\\[a-zA-Z_][a-zA-Z\.0-9_]*									{/*yylval->GLYPHNAME = new std::string(&yytext[1], yyleng - 1);*/ yylval->emplace<std::string>(std::string(&yytext[1], yyleng - 1));return token::GLYPHNAME;}
\\[0-9]+													{/*yylval->CID = atoi(&yytext[1]);*/yylval->emplace<int>(atoi(&yytext[1])); return token::CID;}
\\"0"[xX][0-9A-F]+											{/*yylval->CID = strtol(&yytext[3], NULL, 16);*/ yylval->emplace<int>(strtol(&yytext[3], NULL, 16));return token::CID;}
"/"[^/]*"/"													{/*yylval->REGEXP = new std::string(&yytext[1], yyleng-2);*/  yylval->emplace<std::string>(std::string(&yytext[1], yyleng-2)); return token::REGEXP;}
[|(),{};=<>@'#[\]\\/]										{return yytext[0];}
.															{return yytext[0];}


%% /*** Additional Code ***/

namespace feayy {

Scanner::Scanner(std::istream* in,
		 std::ostream* out)
    : FeaFlexLexer(in, out)
{
}

Scanner::~Scanner()
{
}

void Scanner::set_debug(bool b)
{
    yy_flex_debug = b;
}

}

#ifdef yylex
#undef yylex
#endif

int yyFlexLexer::yylex()
{
    std::cerr << "in FeaFlexLexer::yylex() !" << std::endl;
    return 0;
}


