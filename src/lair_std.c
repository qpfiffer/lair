// vim: noet ts=4 sw=4
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "eval.h"
#include "lair_std.h"
#include "parse.h"

_lair_type *_lair_operator_plus(LAIR_FUNCTION_SIG) {
	/* TODO: Fail in a more sensical manner. */
	assert(argc == 2);
	assert(argv[0] != NULL);
	assert(argv[1] != NULL);
	assert(argv[0]->type == argv[1]->type && "Cannot add variables of different types together.");

	/* We already know (probably) that the types are the same. */
	const LAIR_TOKEN first_arg_type = argv[0]->type;
	switch (first_arg_type) {
		/* Integer addition. */
		case LR_NUM: {
			_lair_type _stack = {
				.type = LR_NUM,
				.value = {
					.num = argv[0]->value.num + argv[1]->value.num
				}
			};
			_lair_type *to_return = calloc(1, sizeof(_lair_type));
			memcpy(to_return, &_stack, sizeof(_lair_type));
			return to_return;
		}
		/* Append two strings together. */
		case LR_STRING: {
			const char *str0 = argv[0]->value.str;
			const char *str1 = argv[1]->value.str;
			const size_t str_siz = strlen(str0) + strlen(str1);

			_lair_type _stack = {
				.type = LR_STRING,
				.value = {
					.str = calloc(1, str_siz)
				}
			};

			_lair_type *to_return = calloc(1, sizeof(_lair_type) + 1);
			memcpy(&_stack.value.str, str0, strlen(str0));
			memcpy(&_stack.value.str + strlen(str0), str1, strlen(str1));
			memcpy(to_return, &_stack, sizeof(_lair_type));

			return to_return;
		}
		default:
			assert(1 == 0 && "Don't know how to add these things together.");
	}
	return NULL;
}
