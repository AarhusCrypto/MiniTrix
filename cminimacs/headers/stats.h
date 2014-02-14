#ifndef STATS_H
#define STATS_H
#include <osal.h>
#include <common.h>
#include <list.h>
#include <time.h>
#include <map.h>


typedef struct _measurement_ {
  char * (*get_name)();
  void (*measure)(void);
  void * impl;
} * Measurement;

#ifdef STATS_ON
void Measurements_print(OE oe);
Measurement Measurement_New(char * name);
void init_stats(OE oe);
Measurement Measurements_start(char * name);
#define InitStats(OE) init_stats((OE));
#define MEASURE_FN(CALL) {                                \
    Measurement _m_ = Measurements_start(__FILE__ #CALL);  \
    CALL;                                                 \
    if (_m_)                                              \
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
