#include "interp.h"
#include <osal.h>
#include <coo.h>
#include <map.h>
#include <hashmap.h>
#include <unistd.h>

static uint str_hash(void * s) {
  char *ss = (char*)s;
  uint lss = 1;
  uint res = 0;
  if (!s) return 0;

  while(ss[lss]) {
    res += 101*ss[lss]+65537;
    ++lss;
  }
  
  return res;
}

static int str_compare(void * a, void * b) {
  char * as = (char*)a;
  char * bs = (char*)b;
  uint las = 0;
  uint lbs = 0;
  uint i = 0, l = 0;
  // as==bs
  if (!as && !bs) return 0;
  // bs > as
  if (!as && bs) return -1;
  // as > bs
  if (as && !bs) return 1;

  while(as[las]) ++las;
  while(bs[lbs]) ++lbs;

  l = las > lbs ? lbs : las;
  for(i = 0;i < l;++i) {   
    if (as[i] > bs[i]) return 1;
    if (as[i] < bs[i]) return -1;
  }
  if (las > lbs) return 1;
  if (las < lbs) return -1;

  return 0;
}


typedef struct _interp_impl_ {
  OE oe;
  MiniMacs mm;
  Map env;
  uint cp;
  bool error;
} * InterpImpl;

COO_DCL(Visitor, void, v_name, AstNode n);
COO_DEF_NORET_ARGS(Visitor, v_name, AstNode n;,n) {
  
}}

COO_DCL(Visitor, void, v_Sload, AstNode n);
COO_DEF_NORET_ARGS(Visitor, v_Sload, AstNode n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  ii->oe->p("Sload\n");
}}

COO_DCL(Visitor, void, v_Load, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Load, AstNode node;,node) {
  Load n = (Load)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  
  if (ii->env->contains( (void*)(ull)n->n->data ) ) {
    uint addr = (uint)(ull)ii->env->get(n->n->data);
    MiniMacsRep rep = ii->mm->heap_get(addr);
    ii->mm->heap_set(n->dst+ii->cp,rep);
  } else {
    char m[128] = {0};
    osal_sprintf(m,"Named Constant %s is undefined.\n", n->n->data );
    ii->oe->p(m);
  }
  ii->oe->p("Load\n");
}}


COO_DCL(Visitor, void, v_Mov, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Mov, AstNode node;,node) {
  Mov n = (Mov)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  MiniMacsRep rep = ii->mm->heap_get(n->src+ii->cp);
  if (rep == 0) {
    char m[128] = {0};
    osal_sprintf(m,"ADDR(%u) Mov(src=%u,dst=%u)\n",n->src+ii->cp,n->src, n->dst);
    ii->oe->p(m);
    ii->error = True;
    return;
  }
  ii->mm->heap_set(n->dst+ii->cp, rep);
}}


COO_DCL(Visitor, void, v_Mul, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Mul, AstNode node;,node) {
  Mul m = (Mul)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  MR res = ii->mm->mul(m->dst+ii->cp,m->op1+ii->cp,m->op2+ii->cp);
  char mmm[32] = {0};
  osal_sprintf(mmm,"%u: mul(%u,%u,%u)\n",node->line,m->dst,m->op1,m->op2);
  ii->oe->p(mmm);
  if (res != 0) {
    char ms[128] = {0};
    osal_sprintf(ms,"Failure at line: %u mul(%u,%u,%u)\n", node->line, m->dst,m->op1,m->op2);
    ii->oe->p(ms);
    ii->error = True;
  }
}}


COO_DCL(Visitor, void, v_Smul, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Smul, AstNode node;,node) {
  Smul n = (Smul)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  Name op2 = n->op2_name;
  char mmm[32] = {0};
  osal_sprintf(mmm,"%u: smul(%u,%u,%s)\n",node->line,n->dst,n->op1,op2->data);
  ii->oe->p(mmm);

  if (ii->env->contains(op2->data)) {
    ull addr = (ull)(void*)ii->env->get(op2->data);
    MR res = ii->mm->mul((n->dst)+(ii->cp), n->op1+ii->cp, (hptr)addr);
    
    if (res != 0) {
      char m[128] = {0};
      osal_sprintf(m,"Failure at line: %u smul(%u,%u,%s)\n",node->line,n->dst, n->op1, op2->data);
      ii->oe->p(m);
      ii->error = True;
    }
  }  else {
    char m[32] = {0};
    osal_sprintf(m,"Unknown identifier \"%s\" \n",op2->data);
    ii->oe->p(m);
    ii->error = True;
  }
}}


COO_DCL(Visitor, void, v_MulPar, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_MulPar, AstNode node;,node) {
  MulPar mp = (MulPar)node->impl;
}}


COO_DCL(Visitor, void, v_Sadd, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Sadd, AstNode node;,node) {
  Sadd n = (Sadd)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  Name op2 = n->name;
  char mmm[32] = {0};
  osal_sprintf(mmm,"%u: sadd(%u,%u,%s)\n",node->line,n->dst,n->op1,op2->data);
  ii->oe->p(mmm);

  if (ii->env->contains(op2->data)) {
    ull v = (ull)(void*)ii->env->get(op2->data);
    MR res = ii->mm->add(n->dst+ii->cp, n->op1+ii->cp, (hptr)v);
    if (res != 0) {
      char m[128] = {0};
      osal_sprintf(m,"Failure at line: %u sadd(%u,%u,%s)\n",node->line,n->dst, n->op1, op2->data);
      ii->oe->p(m);
      ii->error = True;
    }
  } else {
    printf("Unknown identifier \"%s\"\n",op2->data);
    ii->error = True;
  }
  printf("Sadd\n");
}}

COO_DCL(Visitor, void, v_Add, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Add, AstNode node;,node) {
  Add n = (Add)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  MR res = ii->mm->add(n->dst+ii->cp,n->op1+ii->cp,n->op2+ii->cp);
  char mmm[32] = {0};
  osal_sprintf(mmm,"%u: add(%u,%u,%u)\n",node->line,n->dst,n->op1,n->op2);
  ii->oe->p(mmm);

  if (res!= 0) {
      char m[128] = {0};
      osal_sprintf(m,"Failure at line: %u add(%u,%u,%u)\n",node->line,n->dst, n->op1, n->op2);
      ii->oe->p(m);
      ii->error = True;
  }
  printf("Add\n");
}}

static 
int required_bits(uint n) {
  uint answer = 1;
  while(n >>= 1) ++answer;
  return answer;
}

COO_DCL(Visitor, void, v_Const, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_Const, AstNode node;,node) {
  Const n = (Const)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  Name id = n->name;
  List vals = n->vals;
  uint i  = 0;
  uint val = 0;
  printf("Const\n");

  if (vals) {
    for(i = 0;i < vals->size();++i) {
      AstNode node = vals->get_element(i);
      Number n = (Number)node->impl;
      uint bits = required_bits(n->val);
      val = val << bits;
      val += n->val;
    }
  }
    id->addr = ii->cp++;

  if (n->secret == True) {
    ii->mm->secret_input(n->id,id->addr, Data_shallow((byte*)&val,1));
  } else {
    ii->mm->public_input(id->addr,Data_shallow((byte*)&val,1));
  }
  ii->env->put(id->data, (void*)(ull)id->addr);
  i = ii->env->contains(id->data);
}}


COO_DCL(Visitor, void, v_open, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_open, AstNode node;,node) {
  Open o = (Open)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  MiniMacs mm = ii->mm;
  char mmm[32] = {0};
  osal_sprintf(mmm,"%u: open(%u)\n",node->line,o->addr);
  ii->oe->p(mmm);
  mm->open(o->addr+ii->cp);
}}


COO_DCL(Visitor, void, v_print, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_print, AstNode node;,node) {
  Print p = (Print)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  MiniMacs mm = ii->mm;
  MiniMacsRep rep = mm->heap_get(p->addr+ii->cp);
  if (!rep) {
    printf("Address %u not set\n",p->addr);
  } else {
    printf("[%4x] %2x\n",p->addr,rep->codeword[0]);
  }
  printf("print\n");
}}

COO_DCL(Visitor, void, v_init_heap, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_init_heap, AstNode node;,node) {
  InitHeap n = (InitHeap)node->impl;
  InterpImpl ii = (InterpImpl)this->impl;
  ii->mm->init_heap(n->val);
}}


COO_DCL(Visitor, void, v_Number, AstNode n);
COO_DEF_NORET_ARGS(Visitor, v_Number, AstNode n;,n) {
  printf("Number\n");
}}

COO_DCL(Visitor, void, v_List, AstNode node);
COO_DEF_NORET_ARGS(Visitor, v_List, AstNode node;,node) {
  InterpImpl ii = (InterpImpl)this->impl;
  List n = (List)node->impl;
  uint siz = 0, i = 0;
  printf("List\n");
  siz = n->size();
  for(i = 0; i < siz;++i) {
    AstNode cur = n->get_element(i);
    if (cur) cur->visit(this);
    if (ii->error == True) {
      return;
    }
  }
}}

Visitor mpc_circuit_interpreter(OE oe, AstNode root,MiniMacs mm) {
  Visitor res = (Visitor)oe->getmem(sizeof(*res));
  InterpImpl ii = (InterpImpl)oe->getmem(sizeof(*ii));
  if (!res) goto failure;
  if (!ii) goto failure;
  
  res->impl = ii;
  ii->mm = mm;
  ii->cp = 1;
  ii->oe = oe;
  ii->env = HashMap_new(oe, str_hash, str_compare, 32);

  COO_ATTACH_FN(Visitor, res, Name, v_name);
  COO_ATTACH_FN(Visitor, res, Sload, v_Sload);
  COO_ATTACH_FN(Visitor, res, Load, v_Load);
  COO_ATTACH_FN(Visitor, res, Mov, v_Mov);
  COO_ATTACH_FN(Visitor, res, Mul, v_Mul);
  COO_ATTACH_FN(Visitor, res, Smul, v_Smul);
  COO_ATTACH_FN(Visitor, res, MulPar, v_MulPar);
  COO_ATTACH_FN(Visitor, res, Sadd, v_Sadd);
  COO_ATTACH_FN(Visitor, res, Add, v_Add);
  COO_ATTACH_FN(Visitor, res, Const, v_Const);
  COO_ATTACH_FN(Visitor, res, init_heap, v_init_heap);
  COO_ATTACH_FN(Visitor, res, Number, v_Number);
  COO_ATTACH_FN(Visitor, res, List, v_List);
  COO_ATTACH_FN(Visitor, res, Open, v_open );
  COO_ATTACH_FN(Visitor, res, Print, v_print);

  return res;
 failure:
  if (res) {
    oe->putmem(res);res=0;
  }
  if (ii) {
    oe->putmem(ii);ii = 0;
  }
  return res;
}
