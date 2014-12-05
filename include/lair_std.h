// vim: noet ts=4 sw=4
#pragma once
#include <unistd.h>
#include "eval.h"

/**
 * @file
 * Lair standard functions, like all of the operators and whatever
 * else.
 */

/* Forward declarations. */
struct _lair_type;

/**
 * The '+' operator. Adds two things together, must be of the same type, and
 * probably not functions. Adding functions together sounds weird.
 */
struct _lair_type *_lair_operator_plus(LAIR_FUNCTION_SIG);
