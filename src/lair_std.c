// vim: noet ts=4 sw=4
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "eval.h"
#include "parse.h"
#include "lair_std.h"

const _lair_type *_lair_builtin_operator_plus(LAIR_FUNCTION_SIG) {
	check(argc == 2, ERR_RUNTIME, "Incorrect number of arguments to `+` function.");
	check(argv[0] != NULL, ERR_RUNTIME, "Argument to `+` function was NULL.");
	check(argv[1] != NULL, ERR_RUNTIME, "Argument to `+` function was NULL.");
	check(argv[0]->type == argv[1]->type, ERR_RUNTIME, "Cannot add variables of different types together.");

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
			memcpy(_stack.value.str, str0, strlen(str0));
			memcpy(_stack.value.str + strlen(str0), str1, strlen(str1));
			memcpy(to_return, &_stack, sizeof(_lair_type));

			return to_return;
		}
		default:
			error_and_die(ERR_RUNTIME, "Don't know how to add these things together.");
	}
	return NULL;
}

const _lair_type *_lair_builtin_operator_eq(LAIR_FUNCTION_SIG) {
	check(argc == 2, ERR_RUNTIME, "Incorrect number of arguments to `=` function.");
	check(argv[0] != NULL, ERR_RUNTIME, "Argument to `=` function was NULL.");
	check(argv[1] != NULL, ERR_RUNTIME, "Argument to `=` function was NULL.");
	check(argv[0]->type == argv[1]->type, ERR_RUNTIME, "Cannot compare disparate types.");

	const LAIR_TOKEN first_arg_type = argv[0]->type;
	switch (first_arg_type) {
		case LR_BOOL:
			if (argv[0]->value.bool == argv[1]->value.bool)
				return _lair_canonical_true();
			return _lair_canonical_false();
		case LR_NUM:
			if (argv[0]->value.num == argv[1]->value.num)
				return _lair_canonical_true();
			return _lair_canonical_false();
		case LR_STRING: {
			const char *str0 = argv[0]->value.str;
			const char *str1 = argv[1]->value.str;
			const size_t str_siz = strlen(str0) + strlen(str1);
			if (strncmp(str0, str1, str_siz) == 0)
				return _lair_canonical_true();
			return _lair_canonical_false();
		}
		default:
			error_and_die(ERR_RUNTIME, "Don't know how to compare these two things.");
	}
	return NULL;
}

const _lair_type *_lair_builtin_print(LAIR_FUNCTION_SIG) {
	check(argc == 1, ERR_RUNTIME, "Incorrect number of arguments to 'print' function.");

	if (argv[0] == NULL) {
		printf("(null)");
		return NULL;
	}

	switch (argv[0]->type) {
	case LR_STRING:
		printf("%s", argv[0]->value.str);
		break;
	case LR_NUM:
		printf("%i", argv[0]->value.num);
		break;
	case LR_FUNCTION:
		printf("<%s: %s>", _friendly_enum(argv[0]->type), argv[0]->value.str);
		break;
	default:
		printf("<%s: %p>", _friendly_enum(argv[0]->type), argv[0]);
		break;
	}

	return NULL;
}

const _lair_type *_lair_builtin_println(LAIR_FUNCTION_SIG) {
	check (argc == 1, ERR_RUNTIME, "Incorrect number of arguments to 'println' function.");
	_lair_builtin_print(argc, argv);
	printf("\n");

	return NULL;
}

const _lair_type *_lair_builtin_str(LAIR_FUNCTION_SIG) {
	printf("ARGC IS %i", argc);
	check (argc == 1, ERR_RUNTIME, "Incorrect number of arguments to 'str' function.");

	_lair_type *new_string = calloc(1, sizeof(_lair_type));
	new_string->type = LR_STRING;

	if (argv[0] == NULL) {
		char buf[] = "(null)";
		strncpy(new_string->value.str, buf, sizeof(buf));
	} else {
		char buf[512] = {0};

		switch (argv[0]->type) {
		case LR_STRING:
			strncpy(new_string->value.str, argv[0]->value.str, sizeof(buf));
			break;
		case LR_NUM:
			snprintf(buf, sizeof(buf), "%i", argv[0]->value.num);
			strncpy(new_string->value.str, buf, sizeof(buf));
			break;
		case LR_FUNCTION:
			snprintf(buf, sizeof(buf), "<%s: %s>", _friendly_enum(argv[0]->type), argv[0]->value.str);
			strncpy(new_string->value.str, buf, sizeof(buf));
			break;
		default:
			snprintf(buf, sizeof(buf), "<%s: %p>", _friendly_enum(argv[0]->type), argv[0]);
			strncpy(new_string->value.str, buf, sizeof(buf));
			break;
		}
	}

	return new_string;
}
