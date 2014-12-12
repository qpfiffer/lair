// vim: noet ts=4 sw=4
#pragma once

typedef enum {
	ERR_PARSE,
	ERR_SYNTAX,
	ERR_RUNTIME
} ERROR_TYPE;

void check(const int cond, const ERROR_TYPE err_type, const char *msg);
void error_and_die(const ERROR_TYPE err_type, const char *msg);
