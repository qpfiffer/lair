// vim: noet ts=4 sw=4
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "lair.h"

int lair_load_file(const char *file_path, char *loaded, size_t *buf_size) {
	struct stat st = {0};
	if (stat(file_path, &st) == -1)
		return 1;

	int fd = open(file_path, O_RDONLY);
	if (fd <= 0)
		return 1;

	*buf_size = st.st_size;
	loaded = mmap(NULL, *buf_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (loaded == MAP_FAILED)
		return 1;

	close(fd);

	return 0;
}
