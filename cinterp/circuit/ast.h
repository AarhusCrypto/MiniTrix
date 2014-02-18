#ifndef AST_H
#define AST_H

#include <osal.h>
#include <singlelinkedlist.h>

typedef struct _token_ {
  char * cstr;
} * Token;

typedef struct _number_ {
  uint val;
} * Number;

typedef struct _name_ {
  byte * data;
  uint ldata;
  uint addr;
} * Name;

typedef struct _init_heap_ {
  uint val;
} * InitHeap;

typedef struct _const_ {
  List vals;
  Name name;
} * Const;

typedef struct _add_ {
  uint dst,op1,op2;
} * Add;

typedef struct _sadd_ {
  uint dst,op1,op2;
} * Sadd;

typedef struct _mulpar_ {
  uint num;
} * MulPar;

typedef struct _mul_ {
  uint dst,op1,op2;
} * Mul;

typedef struct _smul_ {
  uint dst,op1;
  Name op2_name;
} * Smul;

typedef struct _mov_ {
  uint dst, src;
} * Mov;


typedef struct _load_ {
  Name n;
  uint dst;
} * Load;

typedef struct _sload_ {
  Name n;
  uint dst;
} * Sload;



typedef struct _visitor_ {
  void(*Name)(Name n);
  void(*Sload)(Sload l);
  void(*Load)(Load l);
  void(*Mov)(Mov m);
  void(*Mul)(Mul m);
  void(*Smul)(Smul m);
  void(*MulPar)(MulPar mp);
  void(*Sadd)(Sadd sa);
  void(*Add)(Add a);
  void(*Const)(Const c);
  void(*init_heap)(InitHeap ih);
  void(*Number)(Number n);
  void(*List)(List l);
} * Visitor;

typedef struct _ast_node_ {
  OE oe;
  uint line;
  uint pos;
  uint offset;
  void (*visit)(Visitor v);
  void * impl;
} *AstNode;

typedef struct _ast_node_fac_ {
  OE oe;
  AstNode (*NewName)(uint pos, uint line, uint offset, char * str);
  AstNode (*NewSload)(uint pos, uint line, uint offset, AstNode addr, AstNode name);
  AstNode (*NewLoad)(uint pos, uint line, uint offset, AstNode addr, AstNode name);
  AstNode (*NewMov)(uint pos, uint line, uint offset, AstNode src, AstNode dst);
  AstNode (*NewMul)(uint pos, uint line, uint offset, AstNode dst, AstNode op1,
                    AstNode op2);
  AstNode (*NewSmul)(uint  pos, uint line, uint offset, 
                  AstNode dst, AstNode op1, AstNode op2);
  AstNode (*NewMulPar)(uint  pos, uint line, uint offset, AstNode addr);
  AstNode (*NewSadd)(uint  pos, uint line, uint offset, 
                  AstNode dst, AstNode op1, AstNode op2);
  AstNode (*NewAdd)(uint  pos, uint line, uint offset,
                 AstNode dst, AstNode op1, AstNode op2);
  AstNode (*NewConst)(uint  pos, uint line, uint offset,
                   AstNode name, AstNode numlist);
  AstNode (*NewInitHeap)(uint  pos, uint line, uint offset,
                      AstNode size);

  AstNode (*NewNumber)(uint  pos, uint line, uint offset,
                    uint number);

  AstNode (*NewList)(AstNode first);
  AstNode (*AppList)(AstNode list, AstNode next);
  AstNode (*NewToken)(uint pos, uint line, uint offset, char * str);
} * AstNodeFactory;
AstNodeFactory AstNodeFactory_New(OE oe);
#endif
