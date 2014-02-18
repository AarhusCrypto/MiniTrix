#include "ast.h"
#include <coo.h>
#include <singlelinkedlist.h>

static
AstNode AstNode_New(OE oe, uint pos, uint line, uint offset, void * impl) {
  AstNode node = (AstNode)oe->getmem(sizeof(*node));
  if (!node) return 0;

  node->oe = oe;
  node->pos = pos;
  node->line = line;
  node->offset =offset;
  node->impl = impl;
}

COO_DCL(AstNode, void, visit_name, Visitor v);
COO_DEF_NORET_ARGS(AstNode, visit_name, Visitor v;,v) {
  v->Name( (Name)this->impl );
  return;
}}

COO_DCL(AstNodeFactory, AstNode, NewName, 
        uint pos, uint line, uint offset, char * str);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewName, 
                 uint pos; uint line; 
                 uint offset; char * str;, 
                 pos,line,offset,str) {
  uint lstr = 0, i =0;
  Name impl = (Name)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe,pos,line,offset,impl);

  while(str[lstr] != 0) ++lstr;

  COO_ATTACH_FN(AstNode, result, visit, visit_name);

  impl->data =(byte*)this->oe->getmem(lstr+1);
  mcpy(impl->data,str,lstr);
  impl->ldata = lstr;
  return result;
}}


COO_DCL(AstNode, void, visit_token, Visitor v);
COO_DEF_NORET_ARGS(AstNode, visit_token, Visitor v;,v) {
  return;
}}

COO_DCL(AstNodeFactory, AstNode, NewToken, 
        uint pos, uint line, uint offset, char * str);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewToken, 
                 uint pos; uint line; 
                 uint offset; char * str;, 
                 pos,line,offset,str) {
  uint lstr = 0, i =0;
  Token impl = (Token)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe,pos,line,offset,impl);

  while(str[lstr] != 0) ++lstr;

  COO_ATTACH_FN(AstNode, result, visit, visit_token);

  impl->cstr =(byte*)this->oe->getmem(lstr+1);
  mcpy(impl->cstr,str,lstr);
  return result;
}}


COO_DCL(AstNode, void, visit_sload, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_sload, Visitor v;,v) {
  v->Sload((Sload)this->impl);
  return;
}}

COO_DCL(AstNodeFactory, AstNode, NewSload,
        uint pos, uint line, uint offset, AstNode addr, AstNode name);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewSload, 
                 uint pos; uint line; uint offset;
                 AstNode addr; AstNode name;,pos,line,offset,addr,name) {
  Sload impl = (Sload)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->n = (Name)name->impl;

  impl->dst = ((Number)addr->impl)->val;

  COO_ATTACH_FN(AstNode, result, visit, visit_sload);
  return result;
}}


COO_DCL(AstNode, void, visit_load, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_load, Visitor v;,v) {
  v->Load((Load)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewLoad, 
        uint pos, uint line, uint offset, AstNode addr, AstNode name);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewLoad, 
                 uint pos; uint line; uint offset; AstNode addr; AstNode name;,
                 pos,line,offset,addr,name) {

  Load impl = (Load)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe,pos,line,offset,impl);
  
  impl->n = (Name)name->impl;
  impl->dst = ((Number)addr->impl)->val;
  
  COO_ATTACH_FN(AstNode,result, visit, visit_load);
  
  return result;
}}

COO_DCL(AstNode, void, visit_mov, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_mov, Visitor v;,v) {
  v->Mov((Mov)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewMov,
        uint pos, uint line, uint offset, AstNode src, AstNode dst);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewMov,
                 uint pos; uint line; uint offset; AstNode src; AstNode dst;,
                 pos,line,offset,src,dst) {
  Mov impl = (Mov)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->dst = ((Number)dst->impl)->val;
  impl->src = ((Number)src->impl)->val;
  
  COO_ATTACH_FN(AstNode, result, visit, visit_mov);

  return result;
}}


COO_DCL(AstNode, void, visit_smul, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_smul, Visitor v;,v) {
  v->Smul((Smul)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewSmul, 
        uint  pos, uint line, uint offset, 
        AstNode dst, AstNode op1, AstNode op2);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewSmul,
                 uint  pos; uint line; uint offset; 
                 AstNode dst; AstNode op1; AstNode op2;,
                 pos,line,offset,dst,op1,op2) {
  Smul impl = (Smul)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);
  
  impl->dst = ((Number)dst->impl)->val;
  impl->op1 = ((Number)op1->impl)->val;
  impl->op2_name = (Name)op2->impl;

  COO_ATTACH_FN(AstNode, result, visit, visit_smul);
  return result;
}}
 
COO_DCL(AstNode, void, visit_mul, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_mul, Visitor v;,v) {
  v->Mul((Mul)this->impl);
  return;
}}



COO_DCL(AstNodeFactory, AstNode, NewMul, 
        uint  pos, uint line, uint offset, 
        AstNode dst, AstNode op1, AstNode op2);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewMul,
                 uint  pos; uint line; uint offset; 
                 AstNode dst; AstNode op1; AstNode op2;,
                 pos,line,offset,dst,op1,op2) {
  Mul impl = (Mul)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);
  
  impl->dst = ((Number)dst->impl)->val;
  impl->op1 = ((Number)op1->impl)->val;
  impl->op2 = ((Number)op2->impl)->val;

  COO_ATTACH_FN(AstNode, result, visit, visit_mul);
  return result;
}}



COO_DCL(AstNode, void, visit_mulpar, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_mulpar, Visitor v;,v) {
  v->MulPar((MulPar)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewMulPar, 
        uint  pos, uint line, uint offset, AstNode addr);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewMulPar, 
        uint  pos; uint line; uint offset; AstNode addr;,
        pos, line, offset, addr) {
  MulPar impl = (MulPar)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);
  
  impl->num = ((Number)addr->impl)->val;
 
  COO_ATTACH_FN(AstNode, result, visit, visit_mulpar);
  
  return result;
}}


COO_DCL(AstNode, void, visit_sadd, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_sadd, Visitor v;,v) {
  v->Sadd((Sadd)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewSadd, 
        uint  pos, uint line, uint offset, 
        AstNode dst, AstNode op1, AstNode op2);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewSadd,
                 uint  pos; uint line; uint offset; 
                 AstNode dst; AstNode op1; AstNode op2;,
                 pos, line, offset, dst, op1, op2) {
  Sadd impl = (Sadd)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->dst = ((Number)dst->impl)->val;
  impl->op1 = ((Number)op1->impl)->val;
  impl->op2 = ((Number)op2->impl)->val;

  COO_ATTACH_FN(AstNode, result, visit, visit_sadd);
  
  return result;
}}

COO_DCL(AstNode, void, visit_add, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_add, Visitor v;,v) {
  v->Add((Add)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewAdd,
        uint  pos, uint line, uint offset,
        AstNode dst, AstNode op1, AstNode op2);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewAdd,
                 uint  pos; uint line; uint offset;
                 AstNode dst; AstNode op1; AstNode op2;,
                 pos, line, offset, dst, op1, op2) {

  Add impl = (Add)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->dst = ((Number)dst->impl)->val;
  impl->op1 = ((Number)op1->impl)->val;
  impl->op2 = ((Number)op2->impl)->val;

  COO_ATTACH_FN(AstNode, result, visit, visit_add);
  
  return result;
  
}}


COO_DCL(AstNode, void, visit_const, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_const, Visitor v;,v) {
  v->Const((Const)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewConst,
        uint  pos, uint line, uint offset,
        AstNode name, AstNode numlist);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewConst,
                 uint  pos; uint line; uint offset;
                 AstNode name; AstNode numlist;,
                 pos, line, offset, name, numlist) {
  
  Const impl = (Const)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->vals = (List)numlist->impl;
  impl->name = (Name)name->impl;
  
  COO_ATTACH_FN(AstNode, result, visit, visit_const);

  return result;
}}


COO_DCL(AstNode, void, visit_initheap, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_initheap, Visitor v;,v) {
  v->init_heap((InitHeap)this->impl);
  return;
}}

COO_DCL(AstNodeFactory, AstNode, NewInitHeap, 
        uint  pos, uint line, uint offset,
        AstNode size);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewInitHeap, 
                 uint  pos; uint line; uint offset;
                 AstNode size;, pos,line,offset,size) {
  InitHeap impl = (InitHeap)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->val = ((Number)size->impl)->val;
  COO_ATTACH_FN(AstNode, result, visit, visit_initheap);
  return result;
}}


COO_DCL(AstNode, void, visit_number, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_number, Visitor v;,v) {
  v->Number((Number)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewNumber, 
        uint  pos, uint line, uint offset,
        uint number);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewNumber,
                 uint  pos; uint line; uint offset;
                 uint number;, pos, line, offset, number) {
  Number impl = (Number)this->oe->getmem(sizeof(*impl));
  AstNode result = AstNode_New(this->oe, pos, line, offset, impl);

  impl->val = number;

  COO_ATTACH_FN(AstNode, result, visit, visit_number);

  return result;
}}


COO_DCL(AstNode, void, visit_list, Visitor v);
COO_DEF_NORET_ARGS( AstNode, visit_list, Visitor v;,v) {
  v->List((List)this->impl);
  return;
}}


COO_DCL(AstNodeFactory, AstNode, NewList, AstNode first);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, NewList, AstNode first;,
                 first) {
  if (first) {
    List impl = SingleLinkedList_new(this->oe);
    AstNode result = AstNode_New(this->oe, first->pos, first->line, first->offset, impl);
  
    impl->add_element(first);
    
    COO_ATTACH_FN(AstNode, result, visit, visit_list);
    return result;
  }
  
  return 0;
}}

COO_DCL(AstNodeFactory, AstNode, AppList, AstNode list, AstNode next);
COO_DEF_RET_ARGS(AstNodeFactory, AstNode, AppList, AstNode list; AstNode next;,list, next) {
  List l = (List)list->impl;
  l->add_element(next);
  return list;
}}


AstNodeFactory AstNodeFactory_New(OE oe) {
  AstNodeFactory anf = (AstNodeFactory)oe->getmem(sizeof(*anf));
  if (!anf) return 0;

  COO_ATTACH(AstNodeFactory, anf, NewName);
  COO_ATTACH(AstNodeFactory, anf, NewSload);
  COO_ATTACH(AstNodeFactory, anf, NewLoad);
  COO_ATTACH(AstNodeFactory, anf, NewMov);
  COO_ATTACH(AstNodeFactory, anf, NewMul);
  COO_ATTACH(AstNodeFactory, anf, NewSmul);
  COO_ATTACH(AstNodeFactory, anf, NewMulPar);
  COO_ATTACH(AstNodeFactory, anf, NewSadd);
  COO_ATTACH(AstNodeFactory, anf, NewAdd);
  COO_ATTACH(AstNodeFactory, anf, NewConst);
  COO_ATTACH(AstNodeFactory, anf, NewInitHeap);
  COO_ATTACH(AstNodeFactory, anf, NewNumber);
  COO_ATTACH(AstNodeFactory, anf, NewList);
  COO_ATTACH(AstNodeFactory, anf, AppList);
  COO_ATTACH(AstNodeFactory, anf, NewToken);
  anf->oe = oe;

  return anf;
}
