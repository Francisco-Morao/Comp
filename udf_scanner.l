%option c++ prefix="udf_scanner_" outfile="udf_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "ast/tensor_node.h"
#include "udf_parser.tab.h"

// output stream for building string literals
static std::ostringstream strlit;
// don't change this
#define yyerror LexerError

#define CHECKED_STOD { \
  try { \
    yylval.d = std::stod(yytext); \
    return tREAL; \
  } catch (const std::out_of_range&) { \
    yyerror("double overflow"); \
  } \
}

%}

%x X_COMMENT X_STRING X_BACKSLASH X_STRING_SKIP
%%
  yydebug=1; set_debug(1);

  /* ====================================================================== */
  /* ====[                      3.2 - COMMENTS                        ]==== */
  /* ====================================================================== */

"//".*                 /* match the whole line (it is ignored) */

"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"/*"      yy_push_state(X_COMMENT);
<X_COMMENT>"*/"      yy_pop_state();
<X_COMMENT>.|"\n"     ;  /* ignore inside comments */

  /* ====================================================================== */
  /* ====[                  1   - Tipos de dados                    ]==== */
  /* ====[                  1   - Null pointers                     ]==== */
  /* ====[                  1   - Objects                        ]==== */
  /* ====================================================================== */

"int"                  return tTYPE_INT;
"real"                 return tTYPE_REAL;
"ptr"                  return tTYPE_PTR;
"string"               return tTYPE_STRING;
"void"                 return tTYPE_VOID;
"tensor"               return tTYPE_TENSOR;
"auto"                 return tTYPE_AUTO;

"nullptr"              return tNULLPTR;
"objects"              return tOBJECTS;
"sizeof"               return tSIZEOF;

  /* ====================================================================== */
  /* ====[                 3.5 - Operadores de expressões               ]==== */
  /* ====================================================================== */

[-+*/%~<>?=@]       return *yytext;
">="               return tGE; 
"<="               return tLE; 
"=="               return tEQ; 
"!="               return tNE; 
"&&"              return tAND;
"||"                return tOR;
"input"            return tINPUT;

  /* ====================================================================== */
  /* ====[                  4.5 - Símbolos globais                  ]==== */
  /* ====================================================================== */

"forward"              return tFORWARD;
"public"               return tPUBLIC;

  /* ====================================================================== */
  /* ====[                 6.2 - Instrução condicional                ]==== */
  /* ====================================================================== */

"if"                  return tIF; 
"elif"                return tELIF;
"else"                return tELSE;

  /* ====================================================================== */
  /* ====[                 6.3 - Instrução de iteração                ]==== */
  /* ====[                 6.4 - Instrução de terminação              ]==== */
  /* ====[                 6.5 - Instrução de continuação             ]==== */
  /* ====================================================================== */

"for"                 return tFOR;
"break"               return tBREAK;
"continue"            return tCONTINUE;

 /* ====================================================================== */
  /* ====[              3   - Delimitadores e separadores             ]==== */
  /* ====[              4.3 - Corpo (de função)                       ]==== */
  /* ====[                 5.3 - Instrução de retorno                 ]==== */
  /* ====================================================================== */

[\[\](){};,.]              return *yytext;
"return"               return tRETURN; 
"write"                return tWRITE;
"writeln"              return tWRITELN;

  /* ====================================================================== */
  /* ====[            7.3 Expressões dependentes de tensores             ]==== */
  /* ====================================================================== */

"capacity"            return tCAPACITY;
"rank"                return tRANK;
"dims"                return tDIMS;
"dim"                 return tDIM;
"reshape"             return tRESHAPE;
"**"                return tCONTRACTION;

  /* ====================================================================== */
  /* ====[                3.7 - Identificadores (nomes)               ]==== */
  /* ====================================================================== */

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

 /* ====================================================================== */
  /* ====[                   3.8.1 - inteiros                         ]==== */
  /* ====================================================================== */

[0-9]+                  { /* decimal */
                          try {
                            yylval.i = std::stoi(yytext);
                            return tINTEGER;
                          } catch (const std::out_of_range &e) {
                            yyerror("out of range integer");
                          }
                      }
0x[0-9a-fA-F]+          { /* hexadecimal */
                          try {
                            yylval.i = std::stoi(yytext, NULL, 16);
                            return tINTEGER;
                          } catch (const std::out_of_range &e) {
                            yyerror("out of range integer");
                          }
                      }

  /* ====================================================================== */
  /* ====[                   3.8.2 - reais                            ]==== */
  /* ====================================================================== */

([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([eE][-+]?[0-9]+)? |
[0-9]+([eE][-+]?[0-9]+)                   {
                                      try {
                                        yylval.d = std::stod(yytext);
                                        return tREAL;
                                      } catch (const std::out_of_range &e) {
                                        yyerror("out of range real");
                                      }
                                    }

  /* ====================================================================== */
  /* ====[              3.8.3 - Cadeias de caracteres                 ]==== */
  /* ====================================================================== */

\"                         yy_push_state(X_STRING);
<X_STRING>\\               yy_push_state(X_BACKSLASH);
<X_STRING>\"               {
                              yylval.s = new std::string(strlit.str());
                              strlit.str("");
                              yy_pop_state();
                              return tSTRING;
                            }
<X_STRING>\0              yyerror("NULL byte in string");
<X_STRING>.                strlit << *yytext;

<X_BACKSLASH>n           strlit << '\n'; yy_pop_state();
<X_BACKSLASH>r           strlit << '\r'; yy_pop_state();
<X_BACKSLASH>t           strlit << '\t'; yy_pop_state();
<X_BACKSLASH>\"          strlit << '\"'; yy_pop_state();
<X_BACKSLASH>\\          strlit << '\\'; yy_pop_state();
<X_BACKSLASH>0           yy_push_state(X_STRING_SKIP);

<X_BACKSLASH>[0-9a-fA-F]{1,2} {
                            try {
                              const int c = std::stoi(yytext, NULL, 16);
                              if (c > 255) {
                                yyerror("hexadecimal escape sequence out of range");
                              }
                              strlit << (char) c;
                              yy_pop_state(); 
                            } catch (const std::out_of_range &e) {
                              yyerror("hexadecimal escape sequence out of range");
                            }
                          }
<X_BACKSLASH>.           strlit << *yytext; yy_pop_state();

<X_STRING_SKIP>\\\"|\\\\   ;
<X_STRING_SKIP>\0          yyerror("NULL byte in string");
<X_STRING_SKIP>\"          {
                              yylval.s = new std::string(strlit.str());
                              strlit.str("");
                              yy_pop_state(); yy_pop_state(); yy_pop_state(); /* X_STRING_SKIP, X_BACKSLASH, X_STRING */
                              return tSTRING;
                            }
<X_STRING_SKIP>.           ;

  /* ====================================================================== */
  /* ====[                    3.1 - Caracteres brancos                ]==== */
  /* ====================================================================== */

[ \t\n\r]+             ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
