#include "osal.h"

Cmaphore Cmaphore_new(OE oe, uint count) {
  Cmaphore res = (Cmaphore)oe->getmem(sizeof(*res));
  if (!res) return 0;
  res->lock = oe->newmutex();
  res->count = count;
  res->oe = oe;
  return res;
};


void Cmaphore_up(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;
  c->oe->lock(c->lock);
  ++(c->count);
  c->oe->unlock(c->lock);
}
void Cmaphore_down(Cmaphore c) {
  OE oe = 0;
  if (!c) return;
  if (!c->lock) return;

  oe = c->oe;

  oe->lock(c->lock);
  while(c->count <= 0) {
    oe->unlock( c->lock );
    usleep(0);
    oe->lock( c->lock );
    if (!c->oe)  { 
      oe->unlock(c->lock);
      return; // semaphore destroyed, leave !
    }
  }
  --(c->count);
  oe->unlock( c->lock );
}

void Cmaphore_destroy(Cmaphore * c) {
  OE oe = 0;
  Cmaphore s = 0;
  if (!c) return;
  if (!*c) return;

  s = *c;
  oe = s->oe;
  
  // inform all down's that we are destroying this semaphore
  oe->lock(s->lock);
  s->oe = 0;
  oe->unlock(s->lock);

  // give all other threads a chance to run.
  usleep(0); 

  // Really now we destroy it !
  oe->lock(s->lock);
  s->count = 0;
  oe->destroymutex(&s->lock);
  zeromem(s, sizeof(*s));
  *c = 0;
  oe->putmem(s);
}

