// vim: noet ts=4 sw=4
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "lair.h"
#include "parse.h"

static inline const char *_friendly_err(const ERROR_TYPE err) {
	switch (err) {
		case ERR_PARSE:			return "ERR_PARSE";
		case ERR_RUNTIME:		return "ERR_RUNTIME";
		case ERR_SYNTAX:		return "ERR_SYNTAX";
		default:				return "ERROR";
	}
}

const struct _lair_type *check(
		struct _lair_runtime *r,
		const int cond,
		const ERROR_TYPE err_type, const char *msg) {
	if (!cond)
		return throw_exception(r, err_type, msg);
	return NULL;
}

const struct _lair_type *throw_exception(
		struct _lair_runtime *r,
		const ERROR_TYPE err_type,
		const char *msg) {
	const char *friendly_err = _friendly_err(err_type);
	struct _lair_type *new_type = calloc(1, sizeof(struct _lair_type));
	new_type->type = LR_ERR;

	char buf[256] = {0};
	const size_t len = snprintf(buf, sizeof(buf), "%s: %s", friendly_err, msg);
	new_type->value.str = calloc(1, len);
	strncpy(new_type->value.str, buf, len);

	r->exception = new_type;

	longjmp(r->exception_buffer, 1);
	return new_type;
}

void error_and_die(const ERROR_TYPE err_type, const char *msg) {
	const char *friendly_err = _friendly_err(err_type);
	char buf[256] = {0};

	snprintf(buf, sizeof(buf), "%c[%dm%s%c[%dm", 0x1B, 31, friendly_err, 0x1B, 0x0);
	snprintf(buf + strlen(buf), sizeof(buf), ": %s\n", msg);
	printf("%s", buf);

	exit(1);
}
