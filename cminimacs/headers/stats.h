#ifndef STATS_H
#define STATS_H
#include <osal.h>
#include <common.h>
#include <list.h>

typedef struct _measurement_ {
  char * (*get_name)();
  ull (*get_start)();
  ull (*get_stop)();
  List (*get_sub)(void);
  ull (*get_duration_ms)(void);
  ull (*get_duration_ns)(void);
  ull (*get_duration_s)(void);
  ull (*get_duration_m)(void);
  void * impl;
} * Measurement;

#ifdef STATS_ON

#define MEASURE_FN(CALL) 

#else

#define MEASURE_FN(CALL) 

#endif

#endif
