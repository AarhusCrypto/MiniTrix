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
[a-zA-Z_0-9]+ { ch();
  yylval = anf->NewName(pos, line, offset, yytext);
  return NAME; 
}
<<EOF>>     { return -1; }
.           { ch();printf("Error at line %u:%u\n",line+1,pos+1); }
%%

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  extern MiniMacs mm;  
  extern AstNode root;

  if (oe == 0) {
    return -1;
  }
  init_polynomial();
  anf = AstNodeFactory_New(oe);

  
  mm = GenericMiniMacs_DefaultLoadNew(oe,a[2]);
  if (!mm) return -2;
  
  
  yyin = fopen(a[1],"rb");
  if (yyin) {
    Visitor interp = 0;

    // parse
    yyparse();
    if (error) {   printf("\n");return -1; }

    // connect peers now that we read a circuit
    if (mm->get_id() == 0) {
      oe->p("One party invited, waiting ... ");
      mm->invite(1,2020);
    } else {
      mm->connect(a[3],2020);
    }

    // create interpreter
    interp = mpc_circuit_interpreter( oe,  root, mm);
    if (root) 
      root->visit(interp);

    printf("Done %p\n",root);
  } else {
    printf("Error: Unable to open file\n");
  }
  
  return 0 ;
}

#ifndef yywrap
   yywrap() { return 1; }
#endif
