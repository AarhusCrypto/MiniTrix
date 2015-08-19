/*
 * testcase.c
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */
#include <testcase.h>
#include <stdio.h>
// TODO(rwz): we need to implement this properly
static uint indent_level;
bool run_test_suit(OE oe, TestSuit suit) {
	bool success = True;
	uint i = 0;
	uint j = 0;
	
	// indent suit name print out
	for (j = 0; j < indent_level; ++j) {
		oe->print("  ");
	}
	PRINT(oe,"[SUIT] %s",suit.name);

	// for each inferior test suit run it
	for(i = 0;i < suit.lsubs;++i) {
		__get_suit__ fn = suit.subs[i];
		TestSuit * subsuit = (TestSuit *)fn(oe);
		if (subsuit) {
			indent_level++;
			run_test_suit(oe, *subsuit);
			indent_level--;
		}
	}

	// for each test in this suit run it.
	for(i = 0; i < suit.ltest;++i) {
		uint j = 0;
		Test t = suit.tests[i];
		bool r = t.f(oe);
		for (j = 0; j < indent_level; ++j) {
			oe->print("  ");
		}
		if (r == True) {
			PRINT(oe, "[TEST] %s [OK]",suit.tests[i].name);
		} else {
			PRINT(oe, "[TEST] %s [FAILED]",suit.tests[i].name);
			success = False;
		}
	}
	return success;
}

