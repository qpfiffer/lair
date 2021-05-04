// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "lair.h"

static void _print_usage(const char *name) {
	printf("%s -- Runs REPL mode.\n", name);
	printf("%s <to_run.den> -- Executes a file.\n", name);
}

int _load_file(const char *argv[]) {
	/* The file we're going to load in: */
	const char *file_path = argv[1];
	/* Where we're going to store our loaded buffer: */
	char *buf = NULL;
	size_t buf_siz = 0;

	buf = lair_load_file(file_path, &buf_siz);
	if (buf == NULL) {
		error_and_die(ERR_RUNTIME, "Could not load file.");
		return 1;
	}

	int rc = lair_execute(buf, buf_siz);
	if (rc != 0) {
		error_and_die(ERR_RUNTIME, "Could not execute.");
		return 1;
	}

	lair_unload_file(buf, buf_siz);
	return 0;
}

int _repl_mode() {
	size_t text_size = 1;
	char *text = calloc(text_size, 1);
	char buf[256] = {0};

	printf(">>> ");
	while (fgets(buf, sizeof(buf), stdin)) {
		text_size += strnlen(buf, sizeof(buf));
		text = realloc(text, text_size);
		if (!text)
			return 1;
		strncat(text, buf, text_size);
	}
	printf("%s\n", text);
	free(text);
	return 0;
}

int main(const int argc, const char *argv[]) {
	if (argc < 1) {
		_print_usage(argv[0]);
		exit(0);
	}

	if (argc >= 2)
		return _load_file(argv);
	return _repl_mode();
}
