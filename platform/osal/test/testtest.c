/*
 * testtest.c
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */
// TODO(rwz): We need to have a structured way of doing tests.


#include <testcase.h>

static int testtestfn(OE oe) {
  return True;
}

static int testoefn(OE oe) {
  extern bool testoe(OE oe);
  return testoe(oe);
}

static Test tests1[] = {
  {"Test Operating Env", testoefn}
};

static Test tests2[] = {
    {"Test test", testtestfn},
};

static TestSuit subsuits =   
  {"funky test stuff",
      0,
      0,
      tests2,
      1
};

static TestSuit * get_funky_suit(OE oe) {
	return &subsuits;
}

static GetTestSuit subs[] = { get_funky_suit };

static TestSuit testtest = {
		"Operating Environment Abstraction Layer test suit.",
		(__get_suit__ *)subs,
		1,
		tests1,
		1
};


TestSuit * test_test_suit(OE oe) {
	return &testtest;
}
