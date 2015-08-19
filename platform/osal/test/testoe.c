#include <osal.h>
#include <stdio.h>
#include <coov4.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <testcase.h>

#define C(fn) {					\
    if (!fn) printf("%s\tfailed" , #fn );	\
  }

int testoe(OE oe) {
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
	  return True;
}



Test tests[] = { { "Check OE has been properly initialized", testoe } };

extern GetTestSuit print_test_suit(OE oe);
extern GetTestSuit thread_test_suit(OE oe);
extern GetTestSuit open_test_suit(OE oe);

GetTestSuit subsuits[] = {(GetTestSuit)print_test_suit, 
			  (GetTestSuit)thread_test_suit, 
			  (GetTestSuit)open_test_suit};


TestSuit toptest = {
	"Operating Environment Abstraction Layer test suit.",
	(__get_suit__ *)subsuits,
	sizeof(subsuits)/sizeof(GetTestSuit),
	tests,
	sizeof(tests) / sizeof(Test)
};

TEST_MAIN(toptest);
