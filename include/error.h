// vim: noet ts=4 sw=4
#pragma once

struct _lair_type;

typedef enum {
	ERR_PARSE,
	ERR_SYNTAX,
	ERR_RUNTIME
} ERROR_TYPE;

void check(const int cond, const ERROR_TYPE err_type, const char *msg);
const struct _lair_type *throw_exception(const ERROR_TYPE err_type, const char *msg);
void error_and_die(const ERROR_TYPE err_type, const char *msg);
