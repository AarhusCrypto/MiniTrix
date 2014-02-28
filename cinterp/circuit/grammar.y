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
%token OPEN
%token PRINT
%token SECRET
%%

ROOT:
intro heapinit consts circuit {
  AstNode numbers = $1;
  AstNode heapi = $2;
  AstNode consts = $3;
  AstNode circuit = $4;
  AstNode result = anf->NewList(numbers);
  result = anf->AppList(result, heapi);
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
{ $$ = 0; }
|
consts aconst {
  if ($1 == 0) {
    $$ = anf->NewList($2);
  } else {
    $$ = anf->AppList($1,$2);
  }
}

aconst:
CONST NAME LSQBRACK NUMLIST RSQBRACK  { 
  AstNode t = $1;
  AstNode name = $2;
  AstNode num_list = $4;
  $$ = anf->NewConst(t->pos, t->line, t->offset,
                     name, num_list, 0, False);
}
|
SECRET NUMBER RSQBRACK CONST NAME LSQBRACK NUMLIST RSQBRACK {
  AstNode t = $1;
  AstNode name = $5;
  AstNode num_list = $7;
  AstNode id = $2;
  $$ = anf->NewConst(t->pos, t->line, t->offset,
                     name, num_list, id, True);
}
|
SECRET NUMBER RSQBRACK CONST NAME {
  AstNode t = $1;
  AstNode name = $5;
  AstNode id = $2;
  $$ = anf->NewConst(t->pos, t->line, t->offset,
                     name, 0, id, True);
}

NUMLIST:
{ $$ = 0; }
|
NUMLIST NUMBER { 
  if ($1 == 0) {
    $$ = anf->NewList($2);
  } else {
    $$ = anf->AppList($1,$2);
  }
}

circuit:
{ $$ = 0; }
| 
circuit INSTR  { 
  if ($1 == 0) {
    $$ = anf->NewList($2);
  }  else {
    $$ = anf->AppList($1,$2);
  }
}

heapinit:
INIT_HEAP NUMBER { 
  AstNode token = $1;
  AstNode number = $2;
  $$ = anf->NewInitHeap(token->pos, token->line, token->offset, 
                        number);
}


INSTR:
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
  AstNode src = $2;
  AstNode dst = $3;
  $$ = anf->NewMov(t->pos, t->line, t->offset,
                   src, dst);
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
  $$ = anf->NewSload(t->pos, t->line, t->offset, dst,name);
}
|
OPEN NUMBER {
  AstNode t = $1;
  AstNode addr = $2;
  $$ = anf->NewOpen(t->pos, t->line, t->offset, addr);
}
|
PRINT NUMBER {
  AstNode t = $1;
  AstNode addr = $2;
  $$ = anf->NewPrint(t->pos, t->line, t->offset, addr);
}
