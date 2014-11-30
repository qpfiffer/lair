// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <string.h>
#include "parse.h"

static _str read_line(const char *buf) {
	char c = '\0';

	size_t num_read = 0;
	while (1) {
		c = buf[num_read];
		num_read++;
		if (c == '\0' || c == '\n' || c == '\r')
			break;
	}

	char *data = calloc(1, num_read + 1);
	strncpy(data, buf, num_read);

	_str to_return = {
		.size = num_read,
		.data = data
	};

	return to_return;
}


const _lair_token *_tokenize(const char *program, const size_t len) {
	size_t num_read = 0;
	while (num_read < len) {
		const _str line = read_line(program);
		num_read += line.size;

		free((char *)line.data);
		if (line.size == 0)
			break;
	}
	return NULL;
}
