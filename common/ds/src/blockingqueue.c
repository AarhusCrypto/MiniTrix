#include <blockingqueue.h>
#include <stdarg.h>
#include <coov4.h>

COO_DEF(BlkQueue, void *, get)
  return BlkQueue_take(this);
}

COO_DEF(BlkQueue, void, put, void * elm)
   BlkQueue_push(this, elm);
}

COO_DEF(BlkQueue, uint, size) 

  uint front = this->front;
  uint back = this->back;
  uint cap = this->lelements;
  return  (back - front + cap) % (cap);
}

BlkQueue BlkQueue_new(OE oe, uint size) {
  BlkQueue q = 0;
  RC rc = RC_OK;

  if (!oe) return 0;

  q = (BlkQueue)oe->getmem(sizeof(*q));
  if (!q) return 0;

  q->oe = oe;
  
  if (!size) goto failure;

  rc = oe->newsemaphore(&(q->inserts),size);
  if (!q->inserts) goto failure;
  
  rc = oe->newsemaphore(&(q->takeouts),0);
  if (!q->takeouts) goto failure;
  
  q->elements = oe->getmem( (size+1)*sizeof(void *));
  if (!q->elements) goto failure;
  q->lelements = size+1;

  q->lock = Mutex_new(MUTEX_FREE);
  if (!q->lock) goto failure;

  q->front=q->back=0;

  q->ff = 0;

  q->get = COO_attach(q, BlkQueue_get);
  q->put = COO_attach(q, BlkQueue_put);
  q->size = COO_attach(q, BlkQueue_size);

  return q;
 failure:
  BlkQueue_destroy(&q);
  return 0;
}

BlkQueue BlkQueue_newown(OE oe, uint size, FreeFunction ff) {
  BlkQueue Q = BlkQueue_new(oe,size);
  if (!Q) return Q;
  Q->ff = ff;
  return Q;
}

void * BlkQueue_getcopy(BlkQueue Q, uint * size) {
  uint i = 0, s=0,back=0,front=0,cap=0;
  long ** res = 0; 
  if (!Q) return 0;
  if (!size) return 0;

  back = Q->back;
  front = Q->front;
  cap = Q->lelements;

  s = (back - front + cap) % (cap);

  res = (long **)Q->oe->getmem(sizeof(long*)*s);
  if (!res) return 0;

  for(i = front;i!=back;i = (i+1)%cap) {
    res[i] = ((long**)Q->elements)[i];
  }

  *size = s; 
  return res;
}

int BlkQueue_push(BlkQueue q, void * element) {

  if (!q) return 0;
  
  q->oe->down(q->inserts);
  {
    // TODO(rwl): Notice we have a problem ! Destroy may have freed q
    // before this depending on the scheduler.
    q->oe->lock( q->lock );
    if (!q->elements) {
      Mutex_unlock(q->lock);
      return 0; 
    }
    ((long**)q->elements)[q->back] = (long *)element;
    q->back = (q->back + 1) % (q->lelements);
    q->oe->unlock(q->lock);
  }
  q->oe->up(q->takeouts);
  return 1; // yes I took ownership of that
}


void * BlkQueue_take(BlkQueue q) {
  void * res = 0;
  if (!q) return 0;
  
  q->oe->down(q->takeouts);
  {
    q->oe->lock(q->lock);
    // TODO(rwl): see above same problem as in push when destroying
    // the Q
    if (!q->elements) {
      q->oe->unlock(q->lock);
      return (void*)0;
    }
    
    res = (void*)( ((long **)q->elements)[q->front]);
    ((long **)q->elements)[q->front] = 0;
    q->front = (q->front + 1) % (q->lelements);
    q->oe->unlock(q->lock);
  }
  q->oe->up(q->inserts);
  
  return res;
}

void BlkQueue_destroy( BlkQueue * Q ) {
  void * cur = 0;
  void * elms = 0;
  uint lelms = 0;
  MUTEX lock = 0;
  uint i = 0;
  OE oe = 0;

  if (!Q) return;

  if (!*Q) return;

  oe = (*Q)->oe;

  if ( (*Q)->lock) { oe->lock((*Q)->lock); }
  lock = (*Q)->lock;
  elms = (*Q)->elements;
  lelms = (*Q)->lelements;
  (*Q)->elements = 0;
  (*Q)->lelements = 0;
  oe->yieldthread();
  // TODO(rwl): hopefully this will unlock all takes and pushes.
  if ( (*Q)->lock) { oe->unlock((*Q)->lock);  oe->yieldthread(); }
  if ((*Q)->inserts) { 
    oe->destroysemaphore(&(*Q)->inserts); // unlock all pushes
  }
  if ((*Q)->takeouts) {
    oe->destroysemaphore(&(*Q)->takeouts); // unlock all takes
  }
  
  if ((*Q)->ff) { 
    for(i = 0;i<lelms;++i) {
      cur = (void*)((long**)elms)[i];
      (*Q)->ff(cur);
    }
  } 

  if (elms) {
    oe->putmem(elms);
  }

  
  elms = 0;
  lelms = 0;
  
  if (lock) { oe->lock(lock); }
  oe->putmem(*Q); *Q=0;
  if (lock) { oe->unlock(lock); }
  oe->destroymutex(& lock);
}

