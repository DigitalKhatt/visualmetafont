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
%x FILENAME
%x DIRECTIVE
%x COMMENT

UTF8_ONE [\x00-\x09\x0B\x0C\x0E-\x21\x23-\x5B\x5D-\x7F]
UTF8_TWO [\xC2-\xDF][\x80-\xBF]
UTF8_THREE (\xE0[\xA0-\xBF][\x80-\xBF]|[\xE1-\xEC\xEE-\xEF][\x80-\xBF]{2}|\xED[\x80-\x9F][\x80-\xBF])
UTF8_FOUR (\xF0[\x90-\xBF][\x80-\xBF]{2}|[\xF1-\xF3][\x80-\xBF]{3}|\xF4[\x80-\x8F][\x80-\xBF]{2})

%%

%{
    // reset location
    yylloc->step();
%}

\"({UTF8_ONE}|{UTF8_TWO}|{UTF8_THREE}|{UTF8_FOUR})*\" {
    // Expand a string to Unicode scalars, not bytes or visual graphemes. The
    // patterns reject overlong encodings, surrogates and values above U+10FFFF.
    const auto* bytes = reinterpret_cast<const unsigned char*>(yytext + 1);
    std::vector<int> characters;
    for (int offset = 0; offset < yyleng - 2;) {
        const auto first = bytes[offset];
        const int length = first < 0x80 ? 1 : first < 0xE0 ? 2 : first < 0xF0 ? 3 : 4;
        int value = first & (length == 1 ? 0x7F : (1 << (7 - length)) - 1);
        for (int i = 1; i < length; ++i) value = (value << 6) | (bytes[offset + i] & 0x3F);
        characters.push_back(value);
        offset += length;
    }
    yylval->emplace<std::vector<int>>(std::move(characters));
    return token::CHAR_STRING;
}
\"[^\"\r\n]*\" { throw feayy::Parser::syntax_error(*yylloc, "Character string must contain valid UTF-8 (no escape sequences)"); }


[ \t\r]+													{yylloc->step();} /* gobble up white-spaces */
"#"                                                         {BEGIN(DIRECTIVE); yyless(0); }
<DIRECTIVE>"#if"											{BEGIN(INITIAL); return(token::IF);} 
<DIRECTIVE>"#elif"											{BEGIN(INITIAL); return(token::ELIF);} 
<DIRECTIVE>"#else"											{BEGIN(INITIAL); return(token::ELSE);} 
<DIRECTIVE>"#endif"											{BEGIN(INITIAL); return(token::ENDIF);}
<DIRECTIVE>.											    {BEGIN(COMMENT);}
<COMMENT>.*											        {yylloc->step(); BEGIN(INITIAL);} /* consume # comment */ 
("//").*												    {yylloc->step();} /* consume //-comment */ 
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
"justdfa"                                                   {return(token::JUSTDFA);}
"linepolicy"                                                {return(token::LINEPOLICY);}
"pagepolicy"                                                {return(token::PAGEPOLICY);}
"facts"                                                     {return(token::FACTS);}
"rules"                                                     {return(token::RULES);}
"actions"                                                   {return(token::ACTIONS);}
"selections"                                                {return(token::SELECTIONS);}
"stretchpolicy"                                             {return(token::STRETCHPOLICY);}
"shrinkpolicy"                                              {return(token::SHRINKPOLICY);}
"stages"                                                    {return(token::STAGES);}
"stage"                                                     {return(token::STAGE);}
"quantize"                                                  {return(token::QUANTIZE);}
"fact"                                                      {return(token::FACT);}
"rule"                                                      {return(token::RULE);}
"bind"                                                      {return(token::BIND);}
"selection"                                                 {return(token::SELECTION);}
"choose"                                                    {return(token::CHOOSE);}
"traversal"                                                 {return(token::TRAVERSAL);}
"phase"                                                     {return(token::PHASE);}
"glyphs"                                                     {return(token::GLYPHS);}
"attachments"                                                     {return(token::ATTACHMENTS);}
"attachment"                                                     {return(token::ATTACHMENT);}
"forbid"                                                     {return(token::FORBID);}
"update"                                                     {return(token::UPDATE);}
"replace"                                                     {return(token::REPLACE);}
"isfact"                                                     {return(token::ISFACT);}
"and"                                                     {return(token::AND);}
"or"                                                     {return(token::OR);}
"not"                                                     {return(token::NOT);}
"when"                                                     {return(token::WHEN);}
"clear"                                                     {return(token::CLEAR);}
"has"                                                     {return(token::HAS);}
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
"vary"                           {return(token::VARY);}
"addparams"                       {return(token::ADDPARAMS);}
"params"                          {return(token::PARAMS);}
"coord"                           {return(token::COORD);}
"from"														{return(token::FROM);} 
"table"														{return(token::TABLE);} 
"endtable"													{return(token::ENDTABLE);} 
"pass"														{return(token::PASS);} 
"endpass"													{return(token::ENDPASS);} 
"any"													    {return(token::ANY);} 
"include"													{return(token::INCLUDE);}  
[a-zA-Z_][a-zA-Z\.0-9_]*									{/*yylval->IDENTIFIANT = new std::string(yytext, yyleng);*/  yylval->emplace<std::string>(std::string(yytext, yyleng)); return token::IDENTIFIANT;}
\\[a-zA-Z_][a-zA-Z\.0-9_]*									{/*yylval->GLYPHNAME = new std::string(&yytext[1], yyleng - 1);*/ yylval->emplace<std::string>(std::string(&yytext[1], yyleng - 1));return token::GLYPHNAME;}
\\[0-9]+													{/*yylval->CID = atoi(&yytext[1]);*/yylval->emplace<int>(atoi(&yytext[1])); return token::CID;}
\\"0"[xX][0-9A-F]+											{/*yylval->CID = strtol(&yytext[3], NULL, 16);*/ yylval->emplace<int>(strtol(&yytext[3], NULL, 16));return token::CID;}
"/"[^/]*"/"													{/*yylval->REGEXP = new std::string(&yytext[1], yyleng-2);*/  yylval->emplace<std::string>(std::string(&yytext[1], yyleng-2)); return token::REGEXP;}
[|(),{};=<>@'#[\]\\/]										{return yytext[0];}
.															{return yytext[0];}
<FILENAME>[^) \t\r\n]+  {
    yylval->emplace<std::string>(std::string(yytext, yyleng));    
    BEGIN(INITIAL);   // exit FILENAME mode after token
    return token::T_FILENAME;
}
<FILENAME>[ \t\r\n]+    ;   // skip spaces

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

void Scanner::pushFilenameMode() { BEGIN(FILENAME); }

}

#ifdef yylex
#undef yylex
#endif

int yyFlexLexer::yylex()
{
    std::cerr << "in FeaFlexLexer::yylex() !" << std::endl;
    return 0;
}
