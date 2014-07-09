%{
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <math/polynomial.h>
#include <stdio.h>
#include "ast.h"
#define YYSTYPE AstNode
#include "y.tab.h"
#include "interp.h"
#include <unistd.h>
  extern AstNodeFactory anf;
  static int line;
  static int offset;
  static int pos;

  int error = 0;

  void yyerror(int t) {
    printf("Parser error. Token on line %u:%u-%u was unexpected.",line+1,pos+1-yyleng,pos);
    error = 1;
  }

  static void nl() {
    ++line;
    pos = 0;
    ++offset;
  }

  static void ch() {
    pos += yyleng;
    offset += yyleng;
  }

  static void token() {
    yylval = anf->NewToken(pos,line,offset,yytext);
  }

%}
%x COMMENT
%%

#       { ++pos;BEGIN(COMMENT); }
<COMMENT>. { ++pos; }
<COMMENT>\n { nl();BEGIN(INITIAL); }
\n      { nl(); }
[\t ]   { ch(); }
[0-9]*  { 
  ch(); 
  yylval = anf->NewNumber(pos,line,offset,atoi(yytext));  
  return NUMBER; 
}
"init_heap" { ch();token();return INIT_HEAP; }
"CONST"     { ch();token();return CONST; }
"SECRET"    { ch();token();return SECRET; }
"add"       { ch();token();return ADD; }
"sadd"      { ch();token();return SADD; }
"mulpar"    { ch();token();return MULPAR; }
"mul"       { ch();token();return MUL; }
"smul"      { ch();token();return SMUL; }
"mov"       { ch();token();return MOV; }
"load"      { ch();token();return LOAD; }
"sload"     { ch();token();return SLOAD; }
"open"      { ch();token();return OPEN; }
"print"     { ch();token();return PRINT; }
"["         { ch();token();return LSQBRACK; }
"]"         { ch();token();return RSQBRACK; }
"="         { ch();token();return EQ;}
"+"         { ch();token();return PLUS;}
"%"         { ch();token();return HASH;}
"*"         { ch();token();return STAR;}

[a-zA-Z_0-9]+ { ch();
  yylval = anf->NewName(pos, line, offset, yytext);
  return NAME; 
}
<<EOF>>     { return -1; }
.           { ch();printf("Error at line %u:%u\n",line+1,pos+1); }
%%

#ifndef yywrap
   yywrap() { return 1; }
#endif
