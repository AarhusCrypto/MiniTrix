#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include "mutex.h"
#include "common.h"

MUTEX Mutex_new( unsigned long long initval ) {
  MUTEX m = (MUTEX)malloc(sizeof(*m));
  if (!m) return 0;
  zeromem(m,sizeof(*m));

  (*m) = initval;

  return m;
}

extern void Mutex_lock(MUTEX m);

void Mutex_unlock ( volatile MUTEX m ) {
 *m = 0; // unlock mutex
}

extern void usleep(long long usec);
void Mutex_destroy(volatile MUTEX m) {
  if (m) {
    *m = 0;
	// ask scheduler to releave this thread
    usleep(0);
    free(m); // free mutex
  }
}
