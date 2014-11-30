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

static inline int _is_newline(const char *token) {
	if (strncmp(token, "\n", strlen("\n")) == 0 ||
		strncmp(token, "\r", strlen("\r")) == 0)
		return 1;
	return 0;
}

/* static inline int _is_all_spaces(const char *token) {
	int i = 0;
	for (; i < strlen(token); i++) {
		if (token[i] == ' ')
			return 1;
	}
	return 0;
} */

static inline void _strip(const char *from, char *to) {
	int i = 0, j = 0;
	for (;i < strlen(from);) {
		while (from[i] == '\n' || from[i] == '\r')
			i++;
		if (i >= strlen(from))
			break;
		to[j] = from[i];
		j++;
		i++;
	}
	to[++j] = '\0';
}

static void _insert_token(_lair_token **head, _lair_token *to_insert) {
	if (*head == NULL) {
		*head = to_insert;
		return;
	}

	_lair_token *current = *head;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = to_insert;
}

const _lair_token *_tokenize(const char *program, const size_t len) {
	_lair_token *tokens = NULL;
	size_t num_read = 0;
	while (num_read < len) {
		const _str line = read_line(program + num_read);
		num_read += line.size;

		/* Read in a token: */
		char *token = strtok((char *)line.data, " ");
		while (token != NULL) {
			/* Is it a comment? Ignore the rest of the line. */
			if (strncmp(token, "#", strlen("#")) == 0)
				break;
			else if (_is_newline(token)) {
				token = strtok(NULL, " ");
				continue;
			}

			_lair_token *new_token = calloc(1, sizeof(_lair_token));
			new_token->token = calloc(1, strlen(token));

			char stripped[strlen(token) + 1];
			memset(stripped, '\0', strlen(token) + 1);
			_strip(token, stripped);

			memcpy(new_token->token, stripped, strlen(stripped));

			_insert_token(&tokens, new_token);

			token = strtok(NULL, " ");
		}

		free((char *)line.data);
	}
	return tokens;
}
