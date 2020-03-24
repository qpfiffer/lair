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
 * The '-' operator. Subtracts two things, must be of the same type, and
 * probably not functions. Adding functions together sounds weird. Doesn't
 * work on strings.
 */
const struct _lair_type *_lair_builtin_operator_minus(LAIR_FUNCTION_SIG);

/**
 * The '=' operator. Tries to compare two things together.
 */
const struct _lair_type *_lair_builtin_operator_eq(LAIR_FUNCTION_SIG);

/**
 * Prints a _lair_type to STDOUT.
 */
const struct _lair_type *_lair_builtin_print(LAIR_FUNCTION_SIG);

/**
 * Prints a _lair_type to STDOUT and appends a newline character.
 */
const struct _lair_type *_lair_builtin_println(LAIR_FUNCTION_SIG);

/**
 * Converts a type to a string.
 */
const struct _lair_type *_lair_builtin_str(LAIR_FUNCTION_SIG);
