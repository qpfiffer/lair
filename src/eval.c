// vim: noet ts=4 sw=4
#include <string.h>

#include "error.h"
#include "eval.h"
#include "lair_std.h"
#include "map.h"
#include "parse.h"

static const _lair_type _lair_true = {
	.type = LR_BOOL,
	.value = {
		.bool = 1
	}
};

static const _lair_type _lair_false = {
	.type = LR_BOOL,
	.value = {
		.bool = 0
	}
};

const inline _lair_type *_lair_canonical_false() {
	return &_lair_false;
}

const inline _lair_type *_lair_canonical_true() {
	return &_lair_true;
}

_lair_env *_lair_standard_env() {
	_lair_env *std_env = calloc(1, sizeof(_lair_env));

	int rc = _lair_add_builtin_function(std_env, "print", 1, &_lair_builtin_print);
	check(rc == 0, ERR_RUNTIME, "Could not build standard env.");

	rc = _lair_add_builtin_function(std_env, "+", 2, &_lair_builtin_operator_plus);
	check(rc == 0, ERR_RUNTIME, "Could not build standard env.");

	rc = _lair_add_builtin_function(std_env, "eq", 1, &_lair_builtin_operator_eq);
	check(rc == 0, ERR_RUNTIME, "Could not build standard env.");

	return std_env;
}

int _lair_add_builtin_function(_lair_env *env,
		const char *name,
		const int argc,
		const struct _lair_type *(*func_ptr)(LAIR_FUNCTION_SIG)) {
	check(name != NULL, ERR_RUNTIME, "Function name cannot be NULL.");
	check(env != NULL, ERR_RUNTIME, "Environment cannot be NULL.");
	check(strlen(name) > 0, ERR_RUNTIME, "Function name must be more than 0 chars.");

	/* Check to see if that function already exists: */
	const _lair_function *existing_func = _tst_map_get(env->c_functions, name, strlen(name));
	if (existing_func != NULL)
		return 1;

	_lair_function _stack_func = {
		.argc = argc,
		.argv = calloc(argc, sizeof(_lair_type)),
		.function_ptr = func_ptr
	};

	return _tst_map_insert(&(env->c_functions), name, strlen(name), &_stack_func, sizeof(_lair_function));

}

static const _lair_type **_get_function_args(const int argc, const _lair_ast *ast_node, _lair_env *env) {
	if (ast_node->next->atom.type == LR_CALL) {
		check(argc > 1, ERR_RUNTIME, "Function takes more than one argument, but RHS is only one argument.");
		/* We need to evaluate the RHS before we can pass it to the function
		 * as arguments.
		 */
		const _lair_type **args = calloc(1, sizeof(_lair_type *));
		args[0] = _lair_env_eval(ast_node->next, env);
		return args;
	}

	const _lair_type **args = calloc(argc, sizeof(_lair_type *));
	const _lair_ast *cur_node = ast_node;
	int i = 0;
	for (;i < argc; i++) {
		check(cur_node != NULL, ERR_RUNTIME, "Not enough arguments to function.");
		args[i] = _lair_env_eval(cur_node, env);
		cur_node = cur_node->next;
	}
	return args;
}

static const _lair_type *_lair_call_builtin(const _lair_ast *ast_node, _lair_env *env, const _lair_function *builtin_function) {
	int argc = builtin_function->argc;
	const _lair_type **argv = _get_function_args(argc, ast_node, env);
	return builtin_function->function_ptr(builtin_function->argc, argv);
}

static const _lair_type *_lair_call_function(const _lair_ast *ast_node, _lair_env *env) {
	/* Determine if the thing we're trying to call is a function
	 * or not. It might be an atom, in which case we need to check
	 * or function/c_function maps to see if it's in there.
	 */
	const char *func_name = ast_node->atom.value.str;
	const size_t func_len = strlen(ast_node->atom.value.str);

	const _lair_function *builtin_function = _tst_map_get(env->c_functions, func_name, func_len);
	if (builtin_function != NULL)
		return _lair_call_builtin(ast_node, env, builtin_function);

	/* Well if we're at this point this is a program-defined function. */
	const _lair_ast *defined_function_ast = _tst_map_get(env->functions, func_name, func_len);
	check(defined_function_ast != NULL, ERR_RUNTIME, "No such function.");

	/* Figure out how many arguments are require for this function. */
	int argc = 0;
	_lair_ast *_func_eval_ast = ((_lair_ast *)defined_function_ast)->next;
	while (_func_eval_ast->atom.type == LR_FUNCTION_ARG) {
		argc++;
		_func_eval_ast = _func_eval_ast->next;
	}

	const _lair_type **args = _get_function_args(argc, ast_node, env);
	check(args != NULL, ERR_RUNTIME, "No arguments.");
	return NULL;
}

/* Inline to avoid another stack frame. */
const inline _lair_type *_lair_env_eval(const struct _lair_ast *ast, _lair_env *env) {
	/* We have a goto here to avoid creating a new stack frame, when we really just
	 * want to call this function again.
	 */
start_eval:
	switch (ast->atom.type) {
		case LR_CALL:
			return _lair_call_function(ast->next, env);
		case LR_ATOM:
			/* TODO: See if the value of this ATOM is actually a function.
			 * Then eval that.
			 */
		case LR_INDENT:
			ast = ast->next;
			goto start_eval;
		case LR_RETURN:
			/* TODO: We actually want to return everything to the right of the
			 * operator until a DEDENT or an EOF.
			 */
			return &ast->next->atom;
		default:
			return &ast->atom;
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
			const char *func_name = cur_ast_node->atom.value.str;
			const size_t func_name_len = strlen(func_name);
			_tst_map_insert(&(std_env->functions), func_name, func_name_len,
					cur_ast_node, sizeof(_lair_ast));
		}

		cur_ast_node = cur_ast_node->sibling;
	}

	_lair_free_env(std_env);
	return 0;
}

void builtin_cleanup(void *c_function) {
	_lair_function *f = (_lair_function *)c_function;
	free(f->argv);
}

void _lair_free_env(_lair_env *env) {
	_tst_map_destroy(env->c_functions, builtin_cleanup);
	_tst_map_destroy(env->functions, NULL);
	free(env);
}
