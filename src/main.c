// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include "lair.h"

static void _print_usage(const char *name) {
	printf("%s <to_run.den>\n", name);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		_print_usage(argv[0]);
		exit(0);
	}

	/* The file we're going to load in: */
	const char *file_path = argv[1];
	/* Where we're going to store our loaded buffer: */
	char *buf = NULL;
	size_t buf_siz = 0;

	int rc = lair_load_file(file_path, buf, &buf_siz);
	if (rc != 0) {
		fprintf(stderr, "Could not load file.\n");
		return 1;
	}

	lair_unload_file(buf, buf_siz);
	return 0;
}
