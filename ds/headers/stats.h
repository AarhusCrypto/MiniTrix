#ifndef STATS_H
#define STATS_H
#include <osal.h>
#include <common.h>
#include <list.h>
#include <time.h>
#include <map.h>
#include <stdio.h>

typedef struct _measurement_ {
  char * (*get_name)();
  void (*measure)(void);
  void (*set_start)(void);
  void * impl;
} * Measurement;

void Measurements_print(OE oe);
Measurement Measurement_New(char * name);
void init_stats(OE oe);
Measurement Measurements_start(char * name);
Measurement Measurements_chk(char * name);
Measurement Measurements_get(char * name);

// ############################################################
#ifdef STATS_ON
#define InitStats(OE) init_stats((OE));
#define MEASURE_FN(CALL) {                                \
    Measurement _m_ = Measurements_start(__FILE__ #CALL);  \
    CALL;                                                 \
    if (_m_) {                                            \
      _m_->measure();}}

#define CHECK_POINT(N) {                                            \
    Measurement _m_ = 0;                                            \
    char _str_[128] = {0};                                          \
    Measurements_chk((char*)(N));}

#define CHECK_POINT_S(N) {                                          \
    Measurement _m_ = 0;                                            \
    _m_ = Measurements_get((char*)(N));                             \
    if (_m_) _m_->set_start();  }

#define CHECK_POINT_E(N) {                      \
  Measurement _m_ = 0;                          \
  _m_ = Measurements_get((char*)(N));           \
  if (_m_) _m_->measure(); }

#define PrintMeasurements(OE) {                 \
    Measurements_print(OE);}

// ############################################################
#else

#define MEASURE_FN(CALL) CALL;
#define PrintMeasurements(OE) {\
    (OE)->p("Measurements disabled");}
#define InitStats(OE) OE->p("Measurements are disabled");
#define CHECK_POINT(N)
#define CHECK_POINT_S(N) 
#define CHECK_POINT_E(N)

#endif

#endif
