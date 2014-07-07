/*
  Copyright (c) 2013, Rasmus Lauritsen, Aarhus University
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  3. All advertising materials mentioning features or use of this software

  must display the following acknowledgement:
  This product includes software developed by the Aarhus University.
  4. Neither the name of the Aarhus University nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY Rasmus Lauritsen at Aarhus University ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL Rasmus Lauritsen at Aarhus University BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  Created: 2014-02-11

  Author: Rasmus Winther Lauritsen, rwl@cs.au.dk

  Changes: 
  20-02-2014 Look in the Subversion log for older comments
  25-02-2014 Updated with NewOpen and Open astNode.
*/

#ifndef AST_H
#define AST_H

#include <osal.h>
#include <singlelinkedlist.h>

/************************************************************
 Concrete Ast node data elements
 ************************************************************/
typedef struct _print_ {
  uint addr;
} * Print;

typedef struct _open_ {
  uint addr;
} * Open;

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
  bool secret;
  uint id;
  Name name;
} * Const;

typedef struct _add_ {
  uint dst,op1,op2;
} * Add;

typedef struct _sadd_ {
  uint dst,op1;
  Name name;
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

struct _ast_node_;

typedef struct _operator_ {
  struct _ast_node_ * left;
  struct _ast_node_ * right;
  uint op;
} * Oper;

typedef struct _load_ {
  Name n;
  uint dst;
} * Load;

typedef struct _sload_ {
  Name n;
  uint dst;
} * Sload;
/************************************************************/

struct _visitor_;

/*
 * AstNode 
 */
typedef struct _ast_node_ {
  OE oe;
  uint line;
  uint pos;
  uint offset;
  void (*visit)(struct _visitor_ * v);
  void * impl;
} *AstNode;



/*!
 * A Visitor instance can be given to the {visit} method on any
 * AstNode. Which in turn will invoke the function on the visitor
 * corresponding to its concrete type.
 *
 * E.g. an AstNode from invoking NewOpen on the factory below will
 * invoke the {Open} function on a visitor given to it's {visit}
 * method.
 */
typedef struct _visitor_ {
  void(*Name)(AstNode n);
  void(*Sload)(AstNode l);
  void(*Load)(AstNode l);
  void(*Mov)(AstNode m);
  void(*Mul)(AstNode m);
  void(*Smul)(AstNode m);
  void(*MulPar)(AstNode mp);
  void(*Sadd)(AstNode sa);
  void(*Add)(AstNode a);
  void(*Const)(AstNode c);
  void(*init_heap)(AstNode ih);
  void(*Number)(AstNode n);
  void(*List)(AstNode l);
  void(*Open)(AstNode o);
  void(*Print)(AstNode p);
  void * impl;
} * Visitor;



/*
 * Ast Node Factory. Use an instance of this factory to create ast
 * nodes.
 */
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
                      AstNode name, AstNode numlist, AstNode id, bool secret);
  AstNode (*NewInitHeap)(uint  pos, uint line, uint offset,
                      AstNode size);

  AstNode (*NewNumber)(uint  pos, uint line, uint offset,
                    uint number);

  AstNode (*NewList)(AstNode first);
  AstNode (*AppList)(AstNode list, AstNode next);
  AstNode (*NewToken)(uint pos, uint line, uint offset, char * str);
  AstNode (*NewOpen)(uint pos, uint line, uint offset, AstNode addr);
  AstNode (*NewPrint)(uint pos, uint line, uint offset, AstNode addr);
  AstNode (*NewAssignment)(uint pos, uint line, uint offset, AstNode lval, AstNode op);
  AstNode (*NewPlusOp)(uint pos, uint line, uint offset, AstNode left);
  AstNode (*NewStarOp)(uint pos, uint line, uint offset, AstNode left);
  AstNode (*NewHashOp)(uint pos, uint line, uint offset, AstNode left);
} * AstNodeFactory;

/*
 * Create a default factory.
 */
AstNodeFactory AstNodeFactory_New(OE oe);

// TODO(rwl): Write a free AST function.

#endif
