// mutex.c
#include <stdlib.h>
#include "mutex/mutex.h"
/*  mutex == 0 => free/unlocked */
/*  mutex == 1=> taken/locked */

MUTEX Mutex_new( unsigned long long initval )
{ // Create mutex
  MUTEX m = (MUTEX)malloc(sizeof(*m));
  if (!m) return 0;
  memset(m,0,sizeof(*m));

  (*m) = initval;

  return m;
}

void Mutex_lock( MUTEX m )
{
  asm("movq %0, %%rbx;\n"
      "_mutex:\n"
      "movq $0, %%rax;\n"
      "movq $1, %%rcx;\n"
      "lock cmpxchg %%rcx, (%%rbx);\n"
      "jnz _mutex;\n"
      :
      :"r"(m)
      :"rax","rcx","rbx"
      );
}

void Mutex_unlock ( MUTEX m )
{
	*m = 0; // unlock mutex
}

void Mutex_destroy( MUTEX m )
{
	free(m); // free mutex
}
