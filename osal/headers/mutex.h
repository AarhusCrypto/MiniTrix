// mutex.h
/*  mutex == 0 => free/unlocked */
/*  mutex == 1=> taken/locked */

#ifndef MUTEX_H
#define MUTEX_H

#define MUTEX_FREE 0
#define MUTEX_TAKE 1

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long * MUTEX;

MUTEX Mutex_new( unsigned long long init );

void Mutex_lock( volatile MUTEX m );

void Mutex_unlock ( volatile MUTEX m );

void Mutex_destroy( volatile MUTEX m );

#ifdef __cplusplus
}
#endif


#endif
