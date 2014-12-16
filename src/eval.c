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

	rc = _lair_add_builtin_function(std_env, "=", 1, &_lair_builtin_operator_eq);
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
		error_and_die(ERR_RUNTIME, "Cannot override builtin functions.");

	_lair_function _stack_func = {
		.argc = argc,
		.argv = calloc(argc, sizeof(_lair_type)),
		.function_ptr = func_ptr
	};

	return _tst_map_insert(&(env->c_functions), name, strlen(name), &_stack_func, sizeof(_lair_function));
}

static const _lair_type **_get_function_args(const int argc, const _lair_ast *ast_node, _lair_env *env) {
	if (argc == 0)
		return NULL;
	if (ast_node->next->atom.type == LR_CALL) {
		check(argc == 1, ERR_RUNTIME, "Function takes more than one argument, but RHS is only one argument.");
		/* We need to evaluate the RHS before we can pass it to the function
		 * as arguments.
		 */
		const _lair_type **args = calloc(1, sizeof(_lair_type *));
		args[0] = _lair_env_eval(ast_node->next, env);
		return args;
	}

	const _lair_type **args = calloc(argc, sizeof(_lair_type *));
	const _lair_ast *cur_node = ast_node->next;
	int i = 0;
	/* XXX: Bug here. You could have a function like this:
	 * ! print ! add_two x ! add_two 2 2
	 * Which would do something weird. Need to be able to see how far
	 * the eval went.
	 */
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

_lair_env *_lair_env_with_parent(_lair_env *parent) {
	_lair_env *std_env = calloc(1, sizeof(_lair_env));
	std_env->parent = parent;
	return std_env;
}

/* This function creates a simple function that just returns a single value. It is
 * effectively an immuteable variable defined in the scope `env`.
 */
static int _lair_add_simple_function(_lair_env *env, const char *name, const _lair_type *value) {
	/* This is kind of dumb but whatever. */
	_lair_ast val = {
		.atom = {
			.type = value->type,
			/* HOLY SHIT UNIONS ARE AWESOME */
			.value = value->value
		}
	};

	return _tst_map_insert(&(env->not_variables), name, strlen(name), &val, sizeof(_lair_ast));
}

static const _lair_type *_lair_call_runtime_function(const _lair_ast *top_level_ast, const _lair_ast *defined_function_ast, _lair_env *env) {
	/* Figure out how many arguments are require for this function. */
	int argc = 0;
	_lair_ast *_first_function_arg = ((_lair_ast *)defined_function_ast)->next;
	_lair_ast *_func_eval_ast = _first_function_arg;
	while (_func_eval_ast->atom.type == LR_FUNCTION_ARG) {
		argc++;
		_func_eval_ast = _func_eval_ast->next;
	}

	if (argc > 0) {
		const _lair_type **args = _get_function_args(argc, top_level_ast, env);
		check(args != NULL, ERR_RUNTIME, "No arguments.");

		/* So heres how this works. What we do is create a new `_lair_env` object
		 * with the parent set to the current `env`, and then we dynamically
		 * create new 'functions' which return the function arguments. Since there
		 * are no real variables in Den, we just create functions that return the
		 * values that we want, and bind them into the local scope. Or something like
		 * that.
		 */
		_lair_env *scoped_env = _lair_env_with_parent(env);
		int i;
		_lair_ast *function_parameter = _first_function_arg;
		for (i = 0; i < argc; i++) {
			check(function_parameter->atom.type == LR_FUNCTION_ARG, ERR_SYNTAX,
					"Ran out of function argument parameters, buf function expects more.");
			_lair_add_simple_function(scoped_env, function_parameter->atom.value.str, args[i]);
			function_parameter = function_parameter->next;
		}
		const _lair_type *to_return = _lair_env_eval(_func_eval_ast, scoped_env);
		_lair_free_env(scoped_env);
		return to_return;
	} else {
		return _lair_env_eval(_func_eval_ast, env);
	}
}

static const _lair_type *_lair_call_function(const _lair_ast *ast_node, _lair_env *env) {
	check(ast_node->atom.type == LR_FUNCTION || ast_node->atom.type == LR_ATOM, ERR_RUNTIME, "Cannot call a non-function.");
	/* Determine if the thing we're trying to call is a function
	 * or not. It might be an atom, in which case we need to check
	 * or function/c_function maps to see if it's in there.
	 */
	const char *func_name = ast_node->atom.value.str;
	const size_t func_len = strlen(ast_node->atom.value.str);

	_lair_env *cur_env = env;
	while (cur_env != NULL) {
		const _lair_function *builtin_function = _tst_map_get(cur_env->c_functions, func_name, func_len);
		if (builtin_function != NULL)
			return _lair_call_builtin(ast_node, cur_env, builtin_function);

		/* Well if we're at this point this is a program-defined function. */
		const _lair_ast *defined_function_ast = _tst_map_get(cur_env->functions, func_name, func_len);
		if (defined_function_ast != NULL)
			return _lair_call_runtime_function(ast_node, defined_function_ast, cur_env);

		const _lair_ast *not_variable_ast = _tst_map_get(cur_env->not_variables, func_name, func_len);
		if (not_variable_ast != NULL)
			return _lair_call_function(not_variable_ast, (_lair_env *)cur_env);

		cur_env = (_lair_env *)cur_env->parent;
	}

	error_and_die(ERR_RUNTIME, "No such function.");
	return NULL;
}

static const _lair_ast *_infer_atom_at_runtime(const _lair_ast *ast_node, const _lair_env *env) {
	/* This function attempts to modify an LR_ATOM into something more useful. */
	check(ast_node->atom.type == LR_ATOM, ERR_RUNTIME,
			"Can't infer an already inferred atom.");
	const char *func_name = ast_node->atom.value.str;
	const size_t func_len = strlen(ast_node->atom.value.str);
	_lair_ast *to_return = calloc(1, sizeof(_lair_ast));
	memcpy(to_return, ast_node, sizeof(_lair_ast));

	const _lair_function *builtin_function = _tst_map_get(env->c_functions, func_name, func_len);
	if (builtin_function != NULL) {
		to_return->atom.type = LR_FUNCTION;
		return to_return;
	}

	const _lair_ast *defined_function_ast = _tst_map_get(env->functions, func_name, func_len);
	if (defined_function_ast != NULL) {
		to_return->atom.type = LR_FUNCTION;
		return to_return;
	}

	const _lair_ast *not_variable_ast = (_lair_ast *)_tst_map_get(env->not_variables, func_name, func_len);
	if (not_variable_ast != NULL) {
		to_return->atom.type = not_variable_ast->atom.type;
		to_return->atom.value = not_variable_ast->atom.value;
		return to_return;
	}

	free(to_return);
	return NULL;
}

/* Inline to avoid another stack frame. */
const inline _lair_type *_lair_env_eval(const _lair_ast *ast, _lair_env *env) {
	/* We have a goto here to avoid creating a new stack frame, when we really just
	 * want to call this function again.
	 */
	const _lair_ast *possible_new_atom = NULL;
start_eval:
	switch (ast->atom.type) {
		case LR_OPERATOR:
			return _lair_call_function(ast, env);
		case LR_CALL:
			return _lair_call_function(ast->next, env);
		case LR_ATOM:
			possible_new_atom = _infer_atom_at_runtime(ast, env);
			if (possible_new_atom == NULL)
				error_and_die(ERR_RUNTIME, "Atom is undefined.");
			return &possible_new_atom->atom;
		case LR_INDENT:
			ast = ast->next;
			goto start_eval;
		case LR_RETURN:
			return _lair_env_eval(ast->next, env);
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
