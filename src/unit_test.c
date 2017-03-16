// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include "lair.h"

/* Taken from OlegDB */
#define run_test(test) printf("%s: ", #test);\
	test_return_val = test();\
	if (test_return_val != 0) {\
		tests_failed++;\
		printf("%c[%dmFailed.%c[%dm\n", 0x1B, 31, 0x1B, 0);\
	} else {\
		tests_run++;\
		printf("%c[%dmPassed.%c[%dm\n", 0x1B, 32, 0x1B, 0);\
	}

int test_assignment() {
	return 1;
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int test_return_val = 0 ;
	int tests_run = 0;
	int tests_failed = 0;

	printf("Running tests.\n");

	run_test(test_assignment);

	printf("Tests passed: (%i/%i).\n", tests_run, tests_run + tests_failed);

	if (tests_run != tests_run + tests_failed)
		return 1;
	return 0;
}
