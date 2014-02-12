#include "stats.h"
#include <coo.h>
#include <singlelinkedlist.h>


#ifdef STATS_ON
typedef struct _measurement_impl_ {
  char * name;
  ull start;
  ull stop;
  List sub;
} * MeasurementImpl;


static unsigned long long get_nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return tspec.tv_nsec;
  } else {
    printf("Failed to get time\n");
    exit(-1);
  }
}

static OE _oe_;

static Measurement top;

static init_stats(OE oe) {
  if (!_oe_) {
    _oe_ = oe;
    top = Measurement_New(oe, "CMiniMacs Statistics", get_nano_time());
  }
}


COO_DCL(Measurement, char *, get_name)
COO_DEF_RET_NOARGS(Measurement, char *, get_name) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->name;
}}


COO_DCL(Measurement, ull, get_start)
COO_DEF_RET_NOARGS(Measurement, ull, get_start) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->start;
}}

COO_DCL(Measurement, ull, get_stop);
COO_DEF_RET_NOARGS(Measurement, ull, get_stop) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->stop;
}}

COO_DCL(Measurement, List, get_sub);
COO_DEF_RET_NOARGS(Measurement, List, get_sub) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->sub;
}}


COO_DCL(Measurement, ull, get_duration_ns);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_ns) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    oe->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return impl->stop - impl->start;
}}

#define NANO_PER_MILIS 1000000
COO_DCL(Measurement, ull, get_duration_ms);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_ms) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    oe->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  
  return (impl->stop - impl->start)/NANO_PER_MILIS;
}}

#define NANO_PER_SECOND 1000000000L
COO_DCL(Measurement, ull, get_duration_s);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_s) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    oe->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return (impl->stop - impl->start)/NANO_PER_SECOND;
}}

#define NANO_PER_MINUTE (60*NANO_PER_SECOND)
COO_DCL(Measurement, ull, get_duration_m);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_m) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    oe->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return (impl->stop - impl->start)/NANO_PER_MINUTE;
}}

Measurement Measurement_New(OE oe, char * name, ull start) {
  Measurement res = oe->getmem(sizeof(*res));
  MeasurementImpl impl= 0;
  uint lname = 0;
  if (!res) return 0;
  
  impl = (MeasurementImpl)oe->getmen(sizeof(*impl));
  if (!impl) goto failure;

  

  while(name[lname]) ++lname;

  impl->name = oe->getmem(lname);
  if (impl->name) {
    mcpy(impl->name, name, lname);
  }

  impl->start = start;

  COO_ATTACH(Measurement, res, get_name);
  COO_ATTACH(Measurement, res, get_start);
  COO_ATTACH(Measurement, res, get_stop);
  COO_ATTACH(Measurement, res, get_sub);
  COO_ATTACH(Measurement, res, get_duration_ms);
  COO_ATTACH(Measurement, res, get_duration_ns);
  COO_ATTACH(Measurement, res, get_duration_s);
  COO_ATTACH(Measurement, res, get_duration_m);

  impl->sub = SingleLinkedList_New(oe);
  
  return res;
}


#else


#endif
