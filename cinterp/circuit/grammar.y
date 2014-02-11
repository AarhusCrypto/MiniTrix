%{
#define YYSTACK_ALLOC_MAXIMUM (1024*1024*10)
#define YYMAXDEPTH (1024*1024)
#include <minimacs/minimacs.h>
#include "ast.h"
#define YYSTYPE AstNode
  MiniMacs mm;
  AstNodeFactory anf;  
  AstNode root;
  %}
%start ROOT
%token CONST
%token NUMBER
%token NAME
%token ADD
%token MUL
%token MULPAR
%token SMUL
%token SADD
%token MOV
%token LSQBRACK
%token RSQBRACK
%token INIT_HEAP
%token SLOAD
%token LOAD

%%

ROOT:
intro consts circuit { 
  AstNode numbers = $1;
  AstNode consts = $2;
  AstNode circuit = $3;
  AstNode result = anf->NewList(numbers);
  root = result;
 }

intro:
NUMBER NUMBER NUMBER NUMBER {
  AstNode res = anf->NewList($1);
  anf->AppList(res, $2);
  anf->AppList(res, $3);
  anf->AppList(res, $4);
  $$ = res;
}


consts:
aconst {
  $$ = anf->NewList($1);
}
|
aconst consts {
  AstNode list = $2;
  $$ = anf->AppList(list,$1);
}

aconst:
CONST NAME LSQBRACK NUMLIST RSQBRACK { 

  $$ = anf->NewConst
}

NUMLIST:
NUMBER { }
|
NUMBER NUMLIST { }

circuit:
INSTR  { }
| 
INSTR circuit { }

INSTR:
INIT_HEAP NUMBER { mm->init_heap(1024); }
|
ADD NUMBER NUMBER NUMBER {}
|
SADD NUMBER NUMBER NAME {}
|
MULPAR NUMBER {}
|
MUL NUMBER NUMBER NUMBER NUMBER {}
|
SMUL NUMBER NUMBER NAME {}
|
MOV NUMBER NUMBER {}
|
LOAD NUMBER NAME { mm->public_input(0,Data_shallow("rasmus",6)); }
| 
SLOAD NUMBER NAME { mm->secret_input(0,0,Data_shallow("rasmus",6));}
