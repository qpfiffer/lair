// vim: noet ts=4 sw=4
#pragma once

#include "lair.h"

typedef enum {
	ERR_PARSE,
	ERR_SYNTAX,
	ERR_RUNTIME
} ERROR_TYPE;

#define check(RUNTIME, COND, ERR_TYPE, MSG) if (_check(RUNTIME, COND, ERR_TYPE, MSG)) {\
		return RUNTIME->exception;\
	}

const struct _lair_type *_check(
		struct _lair_runtime *r,
		const int cond,
		const ERROR_TYPE err_type, const char *msg);

const struct _lair_type *throw_exception(
		struct _lair_runtime *r,
		const ERROR_TYPE err_type,
		const char *msg);

void error_and_die(const ERROR_TYPE err_type, const char *msg);
