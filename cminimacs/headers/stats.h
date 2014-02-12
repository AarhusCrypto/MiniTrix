#ifndef STATS_H
#define STATS_H
#include <osal.h>
#include <common.h>
#include <list.h>
#include <time.h>
#include <map.h>


typedef struct _measurement_ {
  char * (*get_name)();
  ull (*get_start)();
  ull (*get_stop)();
  Map (*get_sub)(void);
  ull (*get_duration_ms)(void);
  ull (*get_duration_ns)(void);
  ull (*get_duration_s)(void);
  ull (*get_duration_m)(void);
  void (*measure)(void);
  void * impl;
} * Measurement;

#ifdef STATS_ON
void Measurements_print(OE oe);
Measurement Measurement_New(char * name);
void init_stats(OE oe);
#define InitStats(OE) init_stats((OE));
#define MEASURE_FN(CALL) {					\
    char m[512] = {0};						\
    Measurement _m_ = 0;					\
    osal_sprintf(m,"%s:%u %s",__FILE__, __LINE__, #CALL );	\
    _m_ = Measurement_New( m  );				\
    CALL;							\
    if (_m_)							\
      _m_->measure();}

#define PrintMeasurements(OE) {\
    Measurements_print(OE);}

#else

#define MEASURE_FN(CALL) CALL;

#define PrintMeasurements(OE) {\
    (OE)->p("Measurements disabled");}

#define InitStats(OE)

#endif

#endif
