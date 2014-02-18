#include "interp.h"
#include <coo.h>

COO_DCL(Visitor, void, v_name, Name n);
COO_DEF_NORET_ARGS(Visitor, v_name, Name n;,n) {
  printf("Name\n");
}}

COO_DCL(Visitor, void, v_Sload, Sload n);
COO_DEF_NORET_ARGS(Visitor, v_Sload, Sload n;,n) {
  printf("Sload\n");
}}
COO_DCL(Visitor, void, v_Load, Load l);
COO_DEF_NORET_ARGS(Visitor, v_Load, Load n;,n) {
  printf("Load\n");
}}
COO_DCL(Visitor, void, v_Mov, Mov m);
COO_DEF_NORET_ARGS(Visitor, v_Mov, Mov n;,n) {
  printf("Mov\n");
}}
COO_DCL(Visitor, void, v_Mul, Mul m);
COO_DEF_NORET_ARGS(Visitor, v_Mul, Mul m;,m) {
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
  printf("Sadd\n");
}}

COO_DCL(Visitor, void, v_Add, Add a);
COO_DEF_NORET_ARGS(Visitor, v_Add, Add n;,n) {
  printf("Add\n");
}}

COO_DCL(Visitor, void, v_Const, Const c);
COO_DEF_NORET_ARGS(Visitor, v_Const, Const n;,n) {
  printf("Const\n");
}}
COO_DCL(Visitor, void, v_init_heap, InitHeap ih);
COO_DEF_NORET_ARGS(Visitor, v_init_heap, InitHeap n;,n) {
  printf("init heap\n");
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
  if (!res) return 0;

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

  return res;
}
