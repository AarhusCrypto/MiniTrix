/*
 * testcase.h
 *
 *  Created on: Dec 10, 2014
 *      Author: rwl
 */

// TODO(rwz): Testcase needs to be moved out into its own project
// in this way we can use utils for command line arguments differentiating
// which tests to run rather than uncommenting 

#ifndef OSAL_INCLUDE_TESTCASE_H_
#define OSAL_INCLUDE_TESTCASE_H_
#include <osal.h>

#define PRINT(OE,MSG,...) {\
    (OE)->print( MSG "\n",##__VA_ARGS__);}


typedef int (*test_function)(OE oe);
typedef struct _test_ {
	char * name;
	test_function f;
} Test;

struct _test_suit_;
typedef struct _test_suit_ * tsptr;

typedef void*(*__get_suit__)(OE oe);

struct _test_suit_;
typedef struct _test_suit_ {
	char * name;
	__get_suit__ * subs;
	uint lsubs;
	Test * tests;
	uint ltest;
} TestSuit;
typedef TestSuit*(*GetTestSuit)(OE oe);
bool run_test_suit(OE oe, TestSuit suit);


#define TEST_MAIN(TOPSUIT)\
	int main(int c, char **a) { \
		OE oe = OperatingEnvironment_New();\
		oe->set_log_file("test.log");	   \
		if (!oe) { return -1; }\
		PRINT(oe,"TestCase Running Suits: ");\
		run_test_suit(oe,(TOPSUIT)); \
		OperatingEnvironment_Destroy(&oe); \
		return 0;\
    }

#endif /* OSAL_INCLUDE_TESTCASE_H_ */
