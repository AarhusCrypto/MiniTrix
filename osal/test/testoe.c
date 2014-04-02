#include <osal.h>
#include <stdio.h>
#include <pthread.h>
#include <coo.h>

#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#define C(fn) {					\
    if (!fn) printf("%s\tfailed" , #fn ); else	\
      printf("%s\tok", #fn);printf("\n");	\
  }


void * ts(void * a) {
  OE oe = (OE)a;
  sleep(1);
  printf("ID from thread %llu\n", (ull)pthread_self());
  //  printf("newthread tid=%u\n",oe->get_thread_id());
  return 0;
}

typedef struct _sobj_ {
  int (*magic)(uint i);
} * SObj;

COO_DCL(SObj, int, magic ,int);
COO_DEF_RET_ARGS(SObj, int, magic, int a;,a) {
  return a+0x42;
}}

SObj SObj_new() {
  SObj r = (SObj)malloc(sizeof(*r));
  COO_ATTACH(SObj,r,magic);
  return r;
}

int main(int c, char **a) {

  OE oe = OperatingEnvironment_LinuxNew();
  C(oe->yieldthread);
  C(oe->accept);
  C(oe->getmem);
  C(oe->putmem);
  C(oe->read);
  C(oe->write);
  C(oe->open);
  C(oe->close);
  C(oe->newthread);
  C(oe->jointhread);
  C(oe->newmutex);
  C(oe->destroymutex);
  C(oe->lock);
  C(oe->unlock);
  C(oe->newsemaphore);
  C(oe->destroysemaphore);
  C(oe->down);
  C(oe->up);
  C(oe->syslog);
  C(oe->p);

  {
    pthread_t * t = (pthread_t*)oe->getmem(sizeof(*t));
    pthread_create(t, 0, ts, oe);
    printf("%llu \n",(ull)*t);
    sleep(2);
  }

  printf("leaving ... ");

}
