#include "stats.h"
#include <coov4.h>
#include <singlelinkedlist.h>
#include <hashmap.h>
#include <datetime.h>

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
  ull min, max, avg, count, total;
  Map sub;
  MUTEX l;
} * MeasurementImpl;

static OE _oe_;

static Measurement top;

static MUTEX lock;

void init_stats(OE oe) {
	RC rc = RC_OK;
  if (!_oe_) {
    MeasurementImpl topi = 0;
    _oe_ = oe;
    rc = oe->newmutex(&lock);
	if (rc != RC_OK) {
		oe->p("Could not initialise mutex...");
		return;
	}
    top = Measurement_New("CMiniMacs Statistics");
    topi=(MeasurementImpl)top->impl;
    topi->sub = (Map)HashMap_new(_oe_, str_hash, str_compare, 128);
  }
}


COO_DEF(Measurement, void, set_start)
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  _oe_->lock(impl->l);
  DateTime dt = (DateTime)_oe_->getSystemLibrary(DATE_TIME_LIBRARY);
  impl->start = dt->getNanoTime();
  _oe_->unlock(impl->l);
}

COO_DEF(Measurement, char *, get_name)
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  return impl->name;
}


COO_DEF(Measurement, void, measure)
  MeasurementImpl impl = (MeasurementImpl)this->impl;
  ull duration = 0;
  _oe_->lock(impl->l);
  
  DateTime dt = (DateTime)_oe_->getSystemLibrary(DATE_TIME_LIBRARY);

  duration = dt->getNanoTime() - impl->start;
  if (impl->min == 0) impl->min = duration;
  if (impl->min > duration) impl->min = duration;
  if (impl->max < duration) impl->max = duration;
  impl->total += duration;
  impl->avg = ((impl->avg * impl->count) + duration)/(impl->count+1);
  impl->count += 1;
  impl->start = 0;
  _oe_->unlock(impl->l);
}

Measurement Measurements_get(char * name) {
  MeasurementImpl topi = 0 ;
  Measurement m = 0;
  //  char _name[120] = {0};
  //osal_sprintf(_name,"[%u] %s", _oe_->get_thread_id(),name);
  if (_oe_ && top) {
    _oe_->lock(lock);
    topi = (MeasurementImpl)top->impl;
    if (topi->sub->contains(name)) {
      m = topi->sub->get(name);
    } else {
      m = Measurement_New(name);
      topi->sub->put(name, m);
    }
    _oe_->unlock(lock);
    return m;
  }

  return 0;
}



Measurement Measurements_chk(char * name) {
  MeasurementImpl topi = 0 ;
  Measurement m = 0;

  if (_oe_ && top) {
    _oe_->lock(lock);
    topi = (MeasurementImpl)top->impl;
    if (topi->sub->contains(name)) {
      m = topi->sub->get(name);
      if (m) {
        m->measure();
        m->set_start();
      }
    } else {
      _oe_->p("Adding new measurement to the map");
      m = Measurement_New(name);
      topi->sub->put(name, m);
      m->set_start();
    }
    _oe_->unlock(lock);
    return m;
  }
  _oe_->unlock(lock);
  return 0;
}

Measurement Measurements_start(char * name) {
  MeasurementImpl topi = 0 ;
  Measurement m = 0;

  if (_oe_ && top) {
    _oe_->lock(lock);
    topi = (MeasurementImpl)top->impl;
    if (topi->sub->contains(name)) {
      m = topi->sub->get(name);
    } else {
      _oe_->p("Adding new measurement to the map");
      m = Measurement_New(name);
      topi->sub->put(name, m);
    }
    m->set_start();
    _oe_->unlock(lock);
    return m;
  } else {
  }
  return 0;
}

void Measurements_print(OE oe) {

  if (_oe_ && top) {
    _oe_->lock(lock);
    uint i = 0;
    MeasurementImpl topi = (MeasurementImpl)top->impl;
    List keys = topi->sub->get_keys();
    oe->p("Measurements Statistics");
    oe->p("-----------------------");
    oe->p("NAME                           MIN  \t\t  MAX  \t\t  AVG  \t\t  COUNT \t  TOTAL");
    
    for(i = 0; i < keys->size();++i) {
      Measurement cur = (Measurement)topi->sub->get(keys->get_element(i));
      if (cur) {
        uint lname = 0;
        MeasurementImpl curi = (MeasurementImpl)cur->impl;
        char mmm[512] = {0};
        char nam[32] = {0};
        
        for(lname = 0;lname < 30;++lname) nam[lname] = ' ';
        lname = 0;
        while(curi->name[lname]) ++lname;
        
        if (lname > 30) {
          // typically the last 30 chars are more
          // interesting than the first 30.
          mcpy(nam,curi->name+(lname-30),30);
        } else {
          mcpy(nam,curi->name,lname);
        }



        osal_sprintf(mmm, "%s %u.%06u\t%u.%06u\t%u.%06u\t%06u\t%u.%u", 
                     nam, 
                     curi->min/1000000,curi->min % 1000000, 
                     curi->max/1000000,curi->max % 1000000, 
                     curi->avg/1000000,curi->avg % 1000000, 
                     curi->count,
                     curi->total/1000000,curi->total % 1000000);
      
        _oe_->p(mmm);
      }
    }
    _oe_->unlock(lock);
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

  impl->start = impl->min = impl->max = impl->avg = impl->count = 0;
  _oe_->newmutex(&(impl->l));

  res->get_name = COO_attach(res, Measurement_get_name);
  res->measure = COO_attach(res, Measurement_measure);
  res->set_start = COO_attach(res, Measurement_set_start);

  res->impl = impl;

  return res;
 failure:  
  _oe_->p("TODO(rwl): Need to clean up :(");
  return 0;
}
