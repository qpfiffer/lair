// vim: noet ts=4 sw=4
#include <assert.h>
#include <string.h>
#include "eval.h"
#include "lair_std.h"
#include "map.h"
#include "parse.h"

_lair_env *_lair_standard_env() {
	_lair_env *std_env = calloc(1, sizeof(_lair_env));

	assert(_lair_add_function(std_env, "+", 2, &_lair_operator_plus) != NULL);
	return std_env;
}

_lair_function *
_lair_add_function(_lair_env *env,
		const char *name,
		const int argc,
		struct _lair_type *(*func_ptr)(LAIR_FUNCTION_SIG)) {
	assert(name != NULL);
	assert(env != NULL);
	assert(strlen(name) > 0);

	/* Check to see if that function already exists: */
	const _lair_function *existing_func = _tst_map_get(env->functions, name, strlen(name));
	if (existing_func != NULL)
		return NULL;

	_lair_function _stack_func = {
		.argc = argc,
		.argv = calloc(argc, sizeof(_lair_type)),
		.function_ptr = func_ptr
	};
	_lair_function *new_func = calloc(1, sizeof(_lair_function));
	memcpy(new_func, &_stack_func, sizeof(_lair_function));

	_tst_map_insert(&(env->functions), name, strlen(name), new_func, sizeof(_lair_function));

	return new_func;

}

int _lair_eval(const struct _lair_ast *ast) {
	_lair_env *std_env = _lair_standard_env();
	_lair_free_env(std_env);
	return 0;
}

void _lair_free_env(_lair_env *env) {
	free(env);
}
