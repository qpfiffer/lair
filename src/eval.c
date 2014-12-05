// vim: noet ts=4 sw=4
#include <assert.h>
#include <string.h>
#include "eval.h"
#include "lair_std.h"
#include "map.h"
#include "parse.h"

_lair_env *_lair_standard_env() {
	_lair_env *std_env = calloc(1, sizeof(_lair_env));

	assert(_lair_add_function(std_env, "+", 2, &_lair_builtin_operator_plus) != NULL);
	assert(_lair_add_function(std_env, "print", 1, &_lair_builtin_print) != NULL);

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

static _lair_type *_lair_env_eval(const struct _lair_ast *ast, _lair_env *env) {
	const _lair_ast *cur_ast_node = ast;
	switch (ast->atom.type) {
		case LR_FUNCTION:
			/* TODO: Call functions, somehow. */
			break;
		case LR_CALL:
			return _lair_env_eval(cur_ast_node->next, env);
		default:
			break;
	}
	/* TODO: Expire anything in this scope. */

	return NULL;
}

int _lair_eval(const struct _lair_ast *root) {
	_lair_env *std_env = _lair_standard_env();
	const _lair_ast *cur_ast_node = root->children;

	while (cur_ast_node != NULL) {
		if (cur_ast_node->atom.type == LR_CALL) {
			_lair_env_eval(cur_ast_node, std_env);
		} else if (cur_ast_node->atom.type == LR_FUNCTION) {
			/* TODO: Add functions defined in code to the env. */
		}

		cur_ast_node = cur_ast_node->sibling;
	}

	_lair_free_env(std_env);
	return 0;
}

void _lair_free_env(_lair_env *env) {
	free(env);
}
