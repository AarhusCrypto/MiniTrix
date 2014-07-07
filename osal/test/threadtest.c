#include <osal.h>
#include <unistd.h>

void * test(void * a) {
  OE oe = (OE)a;
  oe->p("Thread saying hello :D waiting forever");
  usleep(100);
  oe->p("How did we ever get here?");
  return 0;
}

int main(int c, char **a) {
  OE oe = OperatingEnvironment_LinuxNew();
  ThreadID tid = 0;

  oe->p("Thread tests");
  
  tid = oe->newthread(test,oe);
  if (tid) {
    oe->p("Thread successfully created");
  } else {
    oe->p("Damn it thread failed");
  }
  
  oe->jointhread(tid);
  return 0;
}
