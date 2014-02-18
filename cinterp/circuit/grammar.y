%{
#define YYSTACK_ALLOC_MAXIMUM (1024*1024*10)
#define YYMAXDEPTH (1024*1024)
#include <minimacs/minimacs.h>
#include "ast.h"
#define YYSTYPE AstNode
#include <stdlib.h>
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
  result = anf->AppList(result, consts);
  result = anf->AppList(result, circuit);
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
  AstNode t = $1;
  AstNode name = $2;
  AstNode num_list = $4;
  $$ = anf->NewConst(t->pos, t->line, t->offset,
                     name, num_list);
}

NUMLIST:
NUMBER { $$ = anf->NewList($1); }
|
NUMBER NUMLIST { 
  $$ = anf->AppList($2,$1);
}

circuit:
INSTR  { $$ = anf->NewList($1);}
| 
INSTR circuit { 
  $$ = anf->AppList($2,$1);
}

INSTR:
INIT_HEAP NUMBER { 
  AstNode token = $1;
  AstNode number = $2;
  $$ = anf->NewInitHeap(token->pos, token->line, token->offset, 
                        number);
}
|
ADD NUMBER NUMBER NUMBER {
  AstNode token = $1;
  AstNode dest = $2;
  AstNode op1 = $3;
  AstNode op2 = $4;
  $$ = anf->NewAdd(token->pos, token->line, token->offset,
                   dest,op1,op2);
}
|
SADD NUMBER NUMBER NAME {
  AstNode token = $1;
  AstNode dest = $2;
  AstNode op1 = $3;
  AstNode name = $4;
  $$ = anf->NewSadd(token->pos, token->line, token->offset,
                    dest, op1, name);
}
|
MULPAR NUMBER {
  AstNode token = $1;
  AstNode num = $2;
  $$ = anf->NewMulPar(token->pos, token->line, token->offset,num);
}
|
MUL NUMBER NUMBER NUMBER NUMBER {
  AstNode t = $1;
  AstNode dest = $2;
  AstNode op1 = $3;
  AstNode op2 = $4;
  $$ = anf->NewMul(t->pos,t->line,t->offset,
                   dest,op1,op2);
}
|
SMUL NUMBER NUMBER NAME {
  AstNode t = $1;
  AstNode dst = $2;
  AstNode op1 = $3;
  AstNode op2_name = $4;
  $$ = anf->NewSmul(t->pos, t->line, t->offset,
                    dst, op1, op2_name);
}
|
MOV NUMBER NUMBER {
  AstNode t = $1;
  AstNode dst = $2;
  AstNode src = $3;
  $$ = anf->NewMov(t->pos, t->line, t->offset,
                   dst, src);
}
|
LOAD NUMBER NAME { 
  AstNode t = $1;
  AstNode dst = $2;
  AstNode name = $3;
  $$ = anf->NewLoad(t->pos, t->line, t->offset, dst, name);
}
| 
SLOAD NUMBER NAME { 
  AstNode t = $1;
  AstNode dst = $2;
  AstNode name = $3;
  $$ = anf->NewSload(t->pos, t->line, t->offset,
                     dst,name);
}
