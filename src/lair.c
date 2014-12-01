// vim: noet ts=4 sw=4
#include <sys/mman.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "lair.h"
#include "parse.h"

char *lair_load_file(const char *file_path, size_t *buf_size) {
	struct stat st = {0};
	if (stat(file_path, &st) == -1)
		return NULL;

	int fd = open(file_path, O_RDONLY);
	if (fd <= 0)
		return NULL;

	*buf_size = st.st_size;
	char *loaded = mmap(NULL, *buf_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (loaded == MAP_FAILED)
		return NULL;

	close(fd);

	return loaded;
}

int lair_execute(const char *program, const size_t len) {
	_lair_token *tokens = _lair_tokenize(program, len);
	if (tokens == NULL)
		goto error;

	lair_print_tokens(tokens);
	const _lair_ast *ast = _lair_parse_from_tokens(&tokens);
	if (ast == NULL)
		return 1;

	return 0;

error:
	_lair_free_tokens(tokens);
	return 1;
}

void lair_unload_file(char *loaded, size_t buf_size) {
	munmap(loaded, buf_size);
}
