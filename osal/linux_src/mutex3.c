#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <unistd.h>
#include "mutex.h"
#include "sched.h"
#include "common.h"
#include <pthread.h>
#include <errno.h>

extern char * strerror(int);

MUTEX Mutex_new( unsigned long long initval ) {
  MUTEX m = (MUTEX)malloc(sizeof(*m));
  pthread_mutex_t * mutex = malloc(sizeof(*mutex));
  if (!m) return 0;
  zeromem(m,sizeof(*m));

  if (pthread_mutex_init( mutex, 0 ) != 0) {

    printf("FATAL ERROR: Mutex init failed %s.\n",strerror(errno));
    exit(-1);
  }

  (*m) = (unsigned long long)mutex;

  return m;
}


inline
void Mutex_lock( MUTEX m ) {
  int e = 0;
  if (!m) return;
  if ((e = pthread_mutex_lock( (pthread_mutex_t*)*m )) != 0) {
    printf("FATAL ERROR: Mutex lock failed %s. (%d)\n",strerror(e),e);
    exit(-1);
  }
}

inline
void Mutex_unlock (MUTEX m) {
  int e = 0;
  if (!m) return;
  if ((e=pthread_mutex_unlock( (pthread_mutex_t*)*m )) != 0) {
    printf("FATAL ERROR: Mutex unlock failed %s. (%d)\n",strerror(e),e);
    exit(-1);
  }
}

void Mutex_destroy( MUTEX m ) {
  if (m) {
    pthread_mutex_destroy( (pthread_mutex_t*)*m );
    usleep(0);
    free( (void*)*m);
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
