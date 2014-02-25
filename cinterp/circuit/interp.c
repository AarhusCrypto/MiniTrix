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
} * InterpImpl;

COO_DCL(Visitor, void, v_name, Name n);
COO_DEF_NORET_ARGS(Visitor, v_name, Name n;,n) {
  
}}

COO_DCL(Visitor, void, v_Sload, Sload n);
COO_DEF_NORET_ARGS(Visitor, v_Sload, Sload n;,n) {
  printf("Sload\n");
}}
COO_DCL(Visitor, void, v_Load, Load l);
COO_DEF_NORET_ARGS(Visitor, v_Load, Load n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  
  if (ii->env->contains( (void*)(ull)n->n->data ) ) {
    uint addr = (uint)(ull)ii->env->get(n->n->data);
    MiniMacsRep rep = ii->mm->heap_get(addr);
    ii->mm->heap_set(n->dst+ii->cp,rep);
  } else {
    printf("Named Constant %s is undefined.\n", n->n->data );
  }
  
  
  printf("Load\n");
}}
COO_DCL(Visitor, void, v_Mov, Mov m);
COO_DEF_NORET_ARGS(Visitor, v_Mov, Mov n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  MiniMacsRep rep = ii->mm->heap_get(n->src);
  ii->mm->heap_set(n->dst, rep);
  printf("Mov\n");
}}
COO_DCL(Visitor, void, v_Mul, Mul m);
COO_DEF_NORET_ARGS(Visitor, v_Mul, Mul m;,m) {
  InterpImpl ii = (InterpImpl)this->impl;
  ii->mm->mul(m->dst,m->op1,m->op2);
  printf("Mul\n");
}}
COO_DCL(Visitor, void, v_Smul, Smul m);
COO_DEF_NORET_ARGS(Visitor, v_Smul, Smul n;,n) {
  printf("Smul\n");
}}
COO_DCL(Visitor, void, v_MulPar, MulPar mp);
COO_DEF_NORET_ARGS(Visitor, v_MulPar, MulPar n;,n) {
  printf("MulPar\n");
}}
COO_DCL(Visitor, void, v_Sadd, Sadd a);
COO_DEF_NORET_ARGS(Visitor, v_Sadd, Sadd n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  Name op2 = n->name;
  if (ii->env->contains(op2->data)) {
    ull v = (ull)(void*)ii->env->get(op2->data);
    printf("cp=%u\n", ii->cp);
    ii->mm->add(n->dst+ii->cp, n->op1+ii->cp, (hptr)v);
  } else {
    printf("Unknown identifier \"%s\"\n",op2->data);
  }
  printf("Sadd\n");
}}

COO_DCL(Visitor, void, v_Add, Add a);
COO_DEF_NORET_ARGS(Visitor, v_Add, Add n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  ii->mm->add(n->dst+ii->cp,n->op1+ii->cp,n->op2+ii->cp);
  printf("Add\n");
}}

static 
int required_bits(uint n) {
  uint answer = 1;
  while(n >>= 1) ++answer;
  return answer;
}

COO_DCL(Visitor, void, v_Const, Const c);
COO_DEF_NORET_ARGS(Visitor, v_Const, Const n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  Name id = n->name;
  List vals = n->vals;
  uint i  = 0;
  uint val = 0;
  printf("Const\n");

  for(i = 0;i < vals->size();++i) {
    AstNode node = vals->get_element(i);
    Number n = (Number)node->impl;
    uint bits = required_bits(n->val);
    val = val << bits;
    val += n->val;
  }
  id->addr = ii->cp++;
  ii->mm->public_input(id->addr,Data_shallow((byte*)&val,1));
  ii->env->put(id->data, (void*)(ull)id->addr);
  i = ii->env->contains(id->data);
  printf("%u ]] \n",
         i);
}}


COO_DCL(Visitor, void, v_open, Open o);
COO_DEF_NORET_ARGS(Visitor, v_open, Open o;,o) {
  InterpImpl ii = (InterpImpl)this->impl;
  MiniMacs mm = ii->mm;
  
  mm->open(o->addr);
  printf("open\n");
}}



COO_DCL(Visitor, void, v_init_heap, InitHeap ih);
COO_DEF_NORET_ARGS(Visitor, v_init_heap, InitHeap n;,n) {
  InterpImpl ii = (InterpImpl)this->impl;
  ii->mm->init_heap(n->val);
}}
COO_DCL(Visitor, void, v_Number, Number n);
COO_DEF_NORET_ARGS(Visitor, v_Number, Number n;,n) {
  printf("Number\n");
}}

COO_DCL(Visitor, void, v_List, List l);
COO_DEF_NORET_ARGS(Visitor, v_List, List n;,n) {
  uint siz = 0, i = 0;
  printf("List\n");
  siz = n->size();
  for(i = 0; i < siz;++i) {
    AstNode cur = n->get_element(i);
    if (cur) cur->visit(this);
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
