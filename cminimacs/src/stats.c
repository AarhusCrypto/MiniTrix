#include "stats.h"
#include <coo.h>
#include <singlelinkedlist.h>
#include <hashmap.h>

#ifdef STATS_ON

static uint str_hash(void * s) {
  char *ss = (char*)s;
  uint lss = 1;
  uint res = 0;
  if (!s) return 0;

  while(ss[lss]) {
    res += 101*ss[lss]+65537;
    ++lss;
  }
  
  return res;
}

static int str_compare(void * a, void * b) {
  char * as = (char*)a;
  char * bs = (char*)b;
  uint las = 0;
  uint lbs = 0;
  uint i = 0, l = 0;
  // as==bs
  if (!as && !bs) return 0;
  // bs > as
  if (!as && bs) return -1;
  // as > bs
  if (as && !bs) return 1;

  while(as[las]) ++las;
  while(bs[lbs]) ++lbs;

  l = las > lbs ? lbs : las;
  for(i = 0;i < l;++i) {   
    if (as[i] > bs[i]) return 1;
    if (as[i] < bs[i]) return -1;
  }
  if (las > lbs) return 1;
  if (las < lbs) return -1;

  return 0;
}

typedef struct _measurement_impl_ {
  char * name;
  ull start;
  ull min, max, avg, count;
  Map sub;
} * MeasurementImpl;

static
unsigned long long _nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
}

static OE _oe_;

static Measurement top;

void init_stats(OE oe) {
  if (!_oe_) {
    MeasurementImpl topi = 0;
    _oe_ = oe;
    top = Measurement_New("CMiniMacs Statistics");
    topi=(MeasurementImpl)top->impl;
    topi->sub = (Map)HashMap_new(_oe_, str_hash, str_compare, 128);
  }
}


COO_DCL(Measurement, char *, get_name)
COO_DEF_RET_NOARGS(Measurement, char *, get_name) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->name;
}}


COO_DCL(Measurement, void, measure);
COO_DEF_NORET_NOARGS(Measurement, measure) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  MeasurementImpl topi = (MeasurementImpl)top->impl;
  MeasurementImpl m = 0;
  ull duration = 0;

  if (topi->sub->contains(this->get_name())) {
    m = topi->sub->get(this->get_name());
  } else {
    topi->sub->put(this->get_name(), this);
  }
  
  duration = _nano_time() - impl->start;
  if (impl->min == 0) impl->min = duration;
  if (impl->min > duration) impl->min = duration;
  if (impl->max < duration) impl->max = duration;
  impl->avg = ((impl->avg * impl->count) + duration)/(impl->count+1);
  impl->count += 1;

}}

Measurement Measurements_start(char * name) {
  MeasurementImpl topi = (MeasurementImpl)top->impl;
  Measurement m = 0;
  MeasurementImpl mi = 0;
  ull start = _nano_time();

  if (topi->sub->contains(name)) {
    m = topi->sub->get(name);
  } else {
    m = Measurement_New(name);
    topi->sub->put(name, m);
  }
  mi = (MeasurementImpl)m->impl;
  mi->start = start;
  return m;
}

void Measurements_print(OE oe) {

  if (_oe_ && top) {
    uint i = 0;
    MeasurementImpl topi = (MeasurementImpl)top->impl;
    List keys = topi->sub->get_keys();
    oe->p("Measurements Statistics");
    oe->p("-----------------------");
    oe->p("           NAME          \t  MIN  \t  MAX  \t  AVG  \t  COUNT");
    
    for(i = 0; i < keys->size();++i) {
      Measurement cur = (Measurement)topi->sub->get(keys->get_element(i));
      if (cur) {
        MeasurementImpl curi = (MeasurementImpl)cur->impl;
        char mmm[512] = {0};
        osal_sprintf(mmm, "%s\t%u\%u\%u\%u", 
                     curi->name, curi->min, curi->max, curi->avg, curi->count);
      }
    }
  } else {
    if (oe) oe->p("Measurements not initialized, invoke init_stats(oe);");
  }

}


Measurement Measurement_New(char * name) {
  Measurement res = 0; 
  MeasurementImpl impl= 0;
  uint lname = 0;

  if (!_oe_) {
    return 0;
  }
 
  res =  _oe_->getmem(sizeof(*res));
  if (!res) return 0;

  impl = (MeasurementImpl)_oe_->getmem(sizeof(*impl));
  if (!impl) goto failure;
  

  while(name[lname]) ++lname;
  ++lname;

  impl->name = _oe_->getmem(lname);
  if (impl->name) {
    mcpy(impl->name, name, lname);
  }

  impl->start = _nano_time();

  COO_ATTACH(Measurement, res, get_name);
  COO_ATTACH(Measurement, res, measure);

  res->impl = impl;

  return res;
 failure:  
  _oe_->p("TODO(rwl): Need to clean up :(");
  return 0;
}
#else


#endif
