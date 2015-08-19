#include <osal.h>
#include <testcase.h>
#ifndef WINDOWS 
#include <unistd.h>
#endif

static void * test(void * a) {
  OE oe = (OE)a;
  oe->p("Thread saying hello :D waiting forever");
  usleep(1000000);
  oe->p("How did we ever get here?");
  return 0;
}

static int test_thread_create_and_join(OE oe) {
  ThreadID tid = 0;

  oe->p("Thread tests");
  
  oe->newthread(&tid,test,oe);
  if (tid) {
    oe->p("Thread successfully created");
  } else {
    oe->p("Damn it thread failed");
  }
  
  oe->jointhread(tid);
  return tid;
}

static Test tests[] = {
	{ "Create and Join test for thread", test_thread_create_and_join }
};

static TestSuit thread_suit[] = {
	"Thread suit",
	0, 0,
	tests, sizeof(tests) / sizeof(Test)
};

TestSuit * thread_test_suit(OE oe) {
  return (TestSuit*)&thread_suit;
}
