#include "stats.h"
#include <coo.h>
#include <singlelinkedlist.h>
#include <hashmap.h>

#ifdef STATS_ON
typedef struct _measurement_impl_ {
  char * name;
  ull start;
  ull stop;
  Map sub;
} * MeasurementImpl;


static unsigned long long get_nano_time() {
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return tspec.tv_nsec;
  } else {

  }
}

static OE _oe_;

static Measurement top;

void init_stats(OE oe) {
  if (!_oe_) {
    _oe_ = oe;
    top = Measurement_New("CMiniMacs Statistics");
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

COO_DCL(Measurement, Map, get_sub);
COO_DEF_RET_NOARGS(Measurement, Map, get_sub) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->sub;
}}


COO_DCL(Measurement, ull, get_duration_ns);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_ns) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    _oe_->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return impl->stop - impl->start;
}}

#define NANO_PER_MILIS 1000000
COO_DCL(Measurement, ull, get_duration_ms);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_ms) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    _oe_->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  
  return (impl->stop - impl->start)/NANO_PER_MILIS;
}}

#define NANO_PER_SECOND 1000000000L
COO_DCL(Measurement, ull, get_duration_s);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_s) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    _oe_->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return (impl->stop - impl->start)/NANO_PER_SECOND;
}}

#define NANO_PER_MINUTE (60*NANO_PER_SECOND)
COO_DCL(Measurement, ull, get_duration_m);
COO_DEF_RET_NOARGS(Measurement, ull, get_duration_m) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  if (impl->stop < impl->start) {
    _oe_->p("Incomplete measurement: It stopped before it started.");
    return 0;
  }
  return (impl->stop - impl->start)/NANO_PER_MINUTE;
}}

COO_DCL(Measurement, void, measure);
COO_DEF_NORET_NOARGS(Measurement, measure) {
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  MeasurementImpl topi = (MeasurementImpl)top->impl;
  List l = 0;
  /*
  if (topi->sub->contains(this->get_name())) {
    l = topi->sub->get(this->get_name());
  } else {
    l = SingleLinkedList_new(_oe_);
    topi->sub->put(this->get_name(), l);
  }
  impl->stop = get_nano_time();
  l->add_element(this);
  */
}}

void Measurements_print(OE oe) {
  /*
  if (_oe_ && top) {
    uint i = 0;
    MeasurementImpl topi = (MeasurementImpl)top->impl;
    List keys = topi->sub->get_keys();
    oe->p("Measurements Statistics");
    oe->p("-----------------------");
    
    for(i = 0; i < keys->size();++i) {
      List m4k_i = topi->sub->get(keys->get_element(i));
      uint min = 0;
      uint max = 0;
      uint avg = 0;
      uint j = 0;
      if (m4k_i) {
	Measurement first = 0;
	for(j = 0;j < m4k_i->size();++j) {
	  Measurement m = m4k_i->get_element(j);
	  if (j == 0) first = m;
	  ull duration = m->get_duration_ns();
	  if (duration > max) max = duration;
	  if (duration < min) min = duration;
	  avg += duration;
	}
	if (first) {
	  char str[1024] = {0};
	  avg = avg / m4k_i->size();
	  osal_sprintf(str,"%s\t%u\t%u\t%u", first->get_name(), min, max, avg);
	  oe->p(str);
	}
      }
    }
  } else {
    if (oe) oe->p("Measurements not initialized, invoke init_stats(oe);");
  }
  */
}

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

  impl->start = get_nano_time();

  COO_ATTACH(Measurement, res, get_name);
  COO_ATTACH(Measurement, res, get_start);
  COO_ATTACH(Measurement, res, get_stop);
  COO_ATTACH(Measurement, res, get_sub);
  COO_ATTACH(Measurement, res, get_duration_ms);
  COO_ATTACH(Measurement, res, get_duration_ns);
  COO_ATTACH(Measurement, res, get_duration_s);
  COO_ATTACH(Measurement, res, get_duration_m);
  COO_ATTACH(Measurement, res, measure);


  impl->sub = (Map)HashMap_new(_oe_, str_hash, str_compare, 1024);
  res->impl = impl;

  return res;
 failure:  
  _oe_->p("TODO(rwl): Need to clean up :(");
  return 0;
}
#else


#endif
