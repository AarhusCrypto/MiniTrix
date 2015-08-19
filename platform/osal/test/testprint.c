#include <osal.h>
#include <testcase.h>

static int test_print_no_arguments(OE oe) {
	oe->print("\nno variadic arguments given\n");
	return 1;
}

static int test_print_one_argument(OE oe) {
	oe->print("\n%d argument(s) given\n", 1);
	return 1;
}

static Test tests[] = { {"Check that print with no arguments works", test_print_no_arguments},
{ "Check that printing with one variadic argument works", test_print_one_argument }};

static TestSuit print_suit = {
	"Print test suit",
	0, 0,
	tests, sizeof(tests) / sizeof(Test) }
;

TestSuit * print_test_suit(OE oe) {
	return &print_suit;
}