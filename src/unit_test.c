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

int _run_program(const char *filename) {
	char *buf = NULL;
	size_t buf_siz = 0;

	buf = lair_load_file(filename, &buf_siz);
	int rc = lair_execute(buf, buf_siz);
	if (rc != 0) {
		lair_unload_file(buf, buf_siz);
		return 1;
	}

	lair_unload_file(buf, buf_siz);
	return 0;
}

int test_assignment() {
	return _run_program("t/assignment.den");
}

int test_basic() {
	return _run_program("t/basic.den");
}

int test_equality() {
	return _run_program("t/equality.den");
}

int test_functions_all_the_way_down() {
	return _run_program("t/functions_all_the_way_down.den");
}

int test_id_function() {
	return _run_program("t/id_function.den");
}

int test_loop() {
	return _run_program("t/loop.den");
}

int test_multilinefunction() {
	return _run_program("t/multilinefunction.den");
}

int test_objects() {
	return _run_program("t/objects.den");
}

int test_plus() {
	return _run_program("t/plus.den");
}

int test_shadow() {
	return _run_program("t/shadow.den");
}

int test_minus() {
	return _run_program("t/minus.den");
}

int test_string_append() {
	return _run_program("t/string_append.den");
}

int test_thingIThoughtOfThisMorning() {
	return _run_program("t/thingIThoughtOfThisMorning.den");
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	int test_return_val = 0 ;
	int tests_run = 0;
	int tests_failed = 0;

	printf("Running tests.\n");

	run_test(test_assignment);
	run_test(test_basic);
	run_test(test_assignment);
	run_test(test_basic);
	run_test(test_equality);
	run_test(test_functions_all_the_way_down);
	run_test(test_id_function);
	run_test(test_loop);
	run_test(test_multilinefunction);
	run_test(test_objects);
	run_test(test_plus);
	run_test(test_plus);
	run_test(test_shadow);
	run_test(test_minus);
	run_test(test_string_append);
	run_test(test_thingIThoughtOfThisMorning);

	printf("Tests passed: (%i/%i).\n", tests_run, tests_run + tests_failed);

	if (tests_run != tests_run + tests_failed)
		return 1;
	return 0;
}
