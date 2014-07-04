#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <unistd.h>
#include "mutex.h"
#include "sched.h"
#include "common.h"

MUTEX Mutex_new( unsigned long long initval ) {
  MUTEX m = (MUTEX)malloc(sizeof(*m));
  if (!m) return 0;
  zeromem(m,sizeof(*m));

  (*m) = initval;

  return m;
}


void Mutex_lock( MUTEX m )
{
  register unsigned int v = 0;
  do {
    asm("movq %1, %%rbx\n"
        "movq $0, %%rax\n"
        "movq $1, %%rcx\n"
        "lock cmpxchg %%rcx, (%%rbx)\n"
        "jnz _nzero_\n"
        "movl $0,%0\n"
        "jmp _end_\n"
        "_nzero_:\n"
        "movl $1, %0\n"
        "_end_:\n"
        : "=r"(v) 
        : "r" (m)/* input */
        : /* clobber */
        );
    usleep(0);
  }while(v);
}

void Mutex_unlock ( volatile MUTEX m )
{
 *m = 0; // unlock mutex
}

void Mutex_destroy( MUTEX m )
{
  if (m) {
    *m = 0;
    usleep(0);
    free(m); // free mutex
  }
}

/*
int main(int c, char **arg) {

  unsigned int a = 0;
  unsigned int * pa = &a;
  asm("movq $1, (%0)\n"
      : 
      : "r" (pa)
      :
      );
  printf("%u\n",a);
  return 0;
}

*/

/*
int main(int c, char ** a) {

  MUTEX m = Mutex_new(MUTEX_TAKE);

  printf("We should hang!\n");
  Mutex_lock(m);
  printf("Noooo !\n");
  return 0;
}

*/
