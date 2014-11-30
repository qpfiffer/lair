// vim: noet ts=4 sw=4
#pragma once

typedef struct _str {
	const size_t size;
	const char *data;
} _str;

typedef struct _lair_token {
	const char *token;
	struct _lair_token *next;
} _lair_token;

const _lair_token *_tokenize(const char *program, const size_t len);
void _parse(const _lair_token *tokens);
