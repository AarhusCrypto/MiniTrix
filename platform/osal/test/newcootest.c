#include <coov3.h>
#include <stdlib.h>
#include <stdio.h>

/* 
 New COO design:

  Function pointer actually allocated
  +-----------+-----------------------+
  |  * this   | function [asm]        |
  +-----------+-----------------------+

  Assembly code [asm]-block will search back for the this pointer. 

  PATCH: alloc * data, f = data+sizeof(void *), write this to data, return f,

 */



typedef struct _Test_ {
  int j;
  int (*test)(void *, void *);
  void (*test2)(int b);
  int (*test3)();
  void (*test4)(void);
} *Test;


COO_DCL(Test,int,test, void*a,void*b);
COO_DEF_RET_ARGS(Test,int,test,void*a;void*b;,a,b) {
  printf("this=%p\n",this);
  printf("j=%d\n",this->j);
  return 0;
}

COO_DCL(Test,void,test2,int b);
COO_DEF_NORET_ARGS(Test,test2,int b;,b) {
  printf("this=%p\n",this);
  printf("j=%d\n",this->j);
}

COO_DCL(Test,int,test3);
COO_DEF_RET_NOARGS(Test,int,test3) {
  printf("j=%d\n",this->j);
  printf("this=%p\n",this);
  return 0;
}

COO_DCL(Test,void,test4);
COO_DEF_NORET_NOARGS(Test,test4) {
  printf("j=%d\n",this->j);
  printf("this=%p\n",this);
  return;
}

int main(int c, char **a) {
  Memory  mem = LinuxMemoryNew();
  Memory special = LinuxSpecialMemoryNew(mem);
  ull k = 0;
  byte * adr = 0;
  coo_init(special);
  Test t = (Test)malloc(sizeof(*t));

  t->j = 100;

  COO_ATTACH(Test,t,test);
  COO_ATTACH(Test,t,test2);
  COO_ATTACH(Test,t,test3);
  COO_ATTACH(Test,t,test4);

  t->test(0,0);
  t->test2(0);
  t->test3(0);
  t->test4();
}
