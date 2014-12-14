// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"

static inline const char *_friendly_err(const ERROR_TYPE err) {
	switch (err) {
		case ERR_PARSE:			return "ERR_PARSE";
		case ERR_RUNTIME:		return "ERR_RUNTIME";
		case ERR_SYNTAX:		return "ERR_SYNTAX";
		default:				return "ERROR";
	}
}

inline void check(const int cond, const ERROR_TYPE err_type, const char *msg) {
	if (!cond)
		error_and_die(err_type, msg);
}

void error_and_die(const ERROR_TYPE err_type, const char *msg) {
	const char *friendly_err = _friendly_err(err_type);
	const size_t buf_size = strlen(friendly_err) + strlen(": ") + strlen(msg) + strlen("\n\0");

	char buf[buf_size];
	memset(buf, '\0', buf_size);

	sprintf(buf, "%c[%dm%s%c[%dm", 0x1B, 31, friendly_err, 0x1B, 0x0);
	sprintf(buf + strlen(buf), ": %s\n", msg);
	printf("%s", buf);

	exit(1);
}
