// vim: noet ts=4 sw=4
#pragma once
#include <unistd.h>

/**
 * @file
 * Lair standard functions, like all of the operators and whatever
 * else.
 */

/* Forward declarations. */
struct _lair_type;

struct _lair_type *_lair_operator_plus(const int argc, const struct _lair_type *argv[]);
