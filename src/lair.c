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
	const _lair_token *tokens = _tokenize(program, len);
	if (tokens == NULL)
		return 1;

	while (tokens != NULL) {
		_lair_token *to_free = (_lair_token *)tokens;
		tokens = tokens->next;
		free(to_free->token);
		free(to_free);
	}
	return 0;
}

void lair_unload_file(char *loaded, size_t buf_size) {
	munmap(loaded, buf_size);
}
