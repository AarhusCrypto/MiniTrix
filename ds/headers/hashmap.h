#ifndef HASHMAP_H
#define HASHMAP_H
#include "common.h"
#include "map.h"
#include "osal.h"

/*
 * Compute hash of argument.
 */
typedef uint (*HashFN)(void *);

/*
 * return 0 if equal, 1 if a > b and -1 if b > a.
 */
typedef int (*CompareFN)(void * a, void* b);

Map HashMap_new( OE oe, HashFN hfn, CompareFN cfn, uint buckets );
void HashMap_destroy( Map * map );
#endif
