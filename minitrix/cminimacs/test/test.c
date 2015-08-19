#include <minimacs/generic_minimacs.h>
#include <testcase.h>


static void* ping_me(void * a) {
	OE oe = (OE)a;
	CArena arena = CArena_new(oe);
	MpcPeer h = 0;

	usleep(1000);

	arena->connect("127.0.0.1", 2020);
	h = arena->get_peer(0);

	if (h) {
		CAR rc = { 0 };
		rc = h->send(Data_shallow("ping", 5));
		
	}
	CArena_destroy(&arena);
	return 0;
}

static int test_listening(OE oe) {
	CArena arena = CArena_new(oe);
	ThreadID tid = 0;

	oe->newthread(&tid,ping_me, oe);

	arena->listen_wait(1,2020);

	return 0;
}

Test tests[] = { {"Test CArena listning", test_listening} };

TestSuit cminimacs_toptest = { "TestCase Cminimacs",
0,0,
tests,sizeof(tests)/sizeof(Test)};

TEST_MAIN(cminimacs_toptest);

/*
int main(int c, char **a) {
  
  OE oe = OperatingEnvironment_New();
  CArena arena = CArena_new(oe);

  arena->listen(2020);
  
  while(!arena->get_no_peers()) {
    char m[32] = {0};
    sprintf(m,"peer %d\n",arena->get_no_peers());
    oe->p(m);
    usleep(1000);
  }

  CArena_destroy( & arena );
  OperatingEnvironment_Destroy(&oe);
  return 0;
}

*/
