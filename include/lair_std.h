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
const struct _lair_type *_lair_builtin_operator_plus(LAIR_FUNCTION_SIG);

/**
 * The '=' operator. Tries to compare two things together.
 */
const struct _lair_type *_lair_builtin_operator_eq(LAIR_FUNCTION_SIG);

/**
 * Prints a _lair_type to STDOUT.
 */
const struct _lair_type *_lair_builtin_print(LAIR_FUNCTION_SIG);
