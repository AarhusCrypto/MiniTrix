#include <osal.h>
#include <unistd.h>

volatile MUTEX m = 0;

void * t(void * a) {
  OE oe = (OE)a;
  sleep(1);
  oe->p("unlock m");
  oe->unlock(m);
  return 0;
}

int main(int c, char **a) {
  MUTEX m1 = 0;
  m1 = Mutex_new(0);
  Mutex_lock(m1);
  Mutex_lock(m1);
  Mutex_lock(m1);

  {
    OE oe = OperatingEnvironment_LinuxNew();
    ThreadID tid = 0;
    m = oe->newmutex();
    
    oe->p("locking m");
    oe->lock(m);
    tid = oe->newthread(t, oe);
    
    oe->p("locking m again");
  //  oe->unlock(m);
    oe->lock(m);
    oe->jointhread(tid);
  return 0;
  }
}
