%{
#include <osal.h>
#include <minimacs/minimacs.h>
#include <minimacs/generic_minimacs.h>
#include <math/polynomial.h>
#include <stdio.h>
#include "y.tab.h"
#include "ast.h"

  static int line;
  static int offset;
  static int pos;

  void yyerror(int t) {
    printf("Parser error token after line %u:%u",line+1,pos+1);
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

%}
%x COMMENT
%%

#       { ++pos;BEGIN(COMMENT); }
<COMMENT>. { ++pos; }
<COMMENT>\n { nl();BEGIN(INITIAL); }
\n      { nl(); }
[\t ]   { ch(); }
[0-9]*      { ch();return NUMBER; }
"init_heap" { ch();return INIT_HEAP; }
"CONST"     { ch();return CONST; }
"add"       { ch();return ADD; }
"sadd"      { ch();return SADD; }
"mulpar"    { ch();return MULPAR; }
"mul"       { ch();return MUL; }
"smul"      { ch();return SMUL; }
"mov"       { ch();return MOV; }
"load"      { ch();return LOAD; }
"sload"     { ch();return SLOAD; }
"["         { ch();return LSQBRACK; }
"]"         { ch();return RSQBRACK; }
[a-zA-Z_0-9]+ { ch();return NAME; }
<<EOF>>     { return -1; }
.           { ch();printf("Error at line %u:%u\n",line+1,pos+1); }
%%

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  extern MiniMacs mm;  
  extern AstNode root;
  extern AstNodeFactory anf;
  if (oe == 0) {
    return -1;
  }
  init_polynomial();
  anf = AstNodeFactory_New(oe);
  mm = GenericMiniMacs_DefaultLoadNew(oe,a[2]);
  if (!mm) return -2;
  if (mm->get_id() == 0) {
    oe->p("One party invited, waiting ... ");
    mm->invite(1,2020);
  } else {
    mm->connect("127.0.0.1",2020);
  }

  yyin = fopen(a[1],"rb");
  if (yyin) {

    yyparse();
    
    printf("Done %p\n",root);

  } else {
    printf("Error: Unable to open file\n");
  }

  return 0 ;
}

#ifndef yywrap
   yywrap() { return 1; }
#endif
