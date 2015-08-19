#include <osal.h>
#include <testcase.h>
#include <carena.h>

extern TestSuit * get_testsendraw_testsuit(OE oe);
extern TestSuit * get_test_and_send_suit(OE oe);

static GetTestSuit subsuits[] = { get_testsendraw_testsuit,  get_test_and_send_suit };

static int test_carena_create(OE oe) {
	CArena arena = CArena_new(oe);
	bool result = 0;

	result = arena == 0 ? False : True;

	CArena_destroy(&arena);
	return result;
}

static Test tests[] = { 
	{"Testing we can create an arena", test_carena_create},
	 
};


static TestSuit carena_toptest = { 
	"CArena Test Suit ",
	subsuits,sizeof(subsuits)/sizeof(GetTestSuit),
	tests, sizeof(tests) / sizeof(Test)
};

TEST_MAIN(carena_toptest);