#include <osal.h>
#include <stdio.h>
#define C(fn) {					\
    if (!fn) printf("%s\tfailed" , #fn ); else	\
      printf("%s\tok", #fn);printf("\n");	\
  }

int main(int c, char **a) {

  OE oe = OperatingEnvironment_LinuxNew();
  C(oe->yieldthread);
  C(oe->accept);
  C(oe->getmem);
  C(oe->putmem);
  C(oe->read);
  C(oe->write);
  C(oe->open);
  C(oe->close);
  C(oe->newthread);
  C(oe->jointhread);
  C(oe->newmutex);
  C(oe->destroymutex);
  C(oe->lock);
  C(oe->unlock);
  C(oe->newsemaphore);
  C(oe->destroysemaphore);
  C(oe->down);
  C(oe->up);
  C(oe->syslog);
  C(oe->p);

}
