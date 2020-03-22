// vim: noet ts=4 sw=4
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "eval.h"
#include "error.h"
#include "lair.h"
#include "parse.h"

struct _lair_runtime *_lair_runtime_start() {
	struct _lair_runtime *new_runtime = calloc(1, sizeof(struct _lair_runtime));
	return new_runtime;
}

void _lair_runtime_end(struct _lair_runtime *runtime) {
	free(runtime);
}

char *lair_load_file(const char *file_path, size_t *buf_size) {
	struct stat st = {0};
	if (stat(file_path, &st) == -1)
		return NULL;

	int fd = open(file_path, O_RDONLY);
	if (fd <= 0)
		return NULL;

	*buf_size = st.st_size;
	char *loaded = mmap(NULL, *buf_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (loaded == MAP_FAILED || loaded == NULL)
		return NULL;

	close(fd);

	return loaded;
}

int lair_execute(const char *program, const size_t len) {
	struct _lair_runtime *runtime = _lair_runtime_start();
	if (setjmp(runtime->exception_buffer)) {
		if (runtime->exception_msg) {
			print_error(runtime->exception_type, runtime->exception_msg);
		}
		goto error;
	}

	struct _lair_token *tokens = _lair_tokenize(runtime, program, len);
	if (tokens == NULL)
		goto error;

#ifdef DEBUG
	lair_print_tokens(tokens);
#endif
	const struct _lair_ast *ast = _lair_parse_from_tokens(runtime, &tokens);
	if (ast == NULL)
		return 1;

	_lair_eval(runtime, ast);
	_lair_free_tokens(tokens);
	_lair_runtime_end(runtime);
	return 0;

error:
	_lair_free_tokens(tokens);
	_lair_runtime_end(runtime);
	return 1;
}

void lair_unload_file(char *loaded, size_t buf_size) {
	munmap(loaded, buf_size);
}
