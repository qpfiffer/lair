// vim: noet ts=4 sw=4
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "eval.h"
#include "lair_std.h"
#include "map.h"
#include "parse.h"

static const struct _lair_type _lair_true = {
	.type = LR_BOOL,
	.value = {
		.bool = 1
	}
};

static const struct _lair_type _lair_false = {
	.type = LR_BOOL,
	.value = {
		.bool = 0
	}
};

inline const struct _lair_type *_lair_canonical_false() {
	return &_lair_false;
}

inline const struct _lair_type *_lair_canonical_true() {
	return &_lair_true;
}

static inline int _is_callable(const struct _lair_ast *n) {
	const LAIR_TOKEN t = n->atom.type;

	switch (t) {
		case LR_ATOM:
		case LR_IF:
		case LR_OPERATOR:
		case LR_FUNCTION_DEF:
		case LR_FUNCTION_CALL:
			return 1;
		default:
			return 0;
	}

	return 0;
}

static inline int _is_callable_runtime(const struct _lair_env *env, const struct _lair_ast *n) {
	const char *func_name = n->atom.value.str;
	const size_t func_name_len = strlen(func_name);

	const void *is_std_func = _tst_map_get(
			env->functions, func_name, func_name_len);
	const void *is_c_func = _tst_map_get(
			env->c_functions, func_name, func_name_len);

	return _is_callable(n) || is_std_func != NULL || is_c_func != NULL;
}


struct _lair_env *_lair_standard_env(struct _lair_runtime *r) {
	struct _lair_env *std_env = calloc(1, sizeof(struct _lair_env));

	int rc = 0;
	ADD_TO_STD_ENV(r, "print", 1, &_lair_builtin_print);
	ADD_TO_STD_ENV(r, "println", 1, &_lair_builtin_println);
	ADD_TO_STD_ENV(r, "+", 2, &_lair_builtin_operator_plus);
	ADD_TO_STD_ENV(r, "-", 2, &_lair_builtin_operator_minus);
	ADD_TO_STD_ENV(r, "=", 2, &_lair_builtin_operator_eq);
	ADD_TO_STD_ENV(r, "str", 2, &_lair_builtin_str);

	return std_env;
}

int _lair_add_builtin_function(
		struct _lair_runtime *r,
		struct _lair_env *env,
		const char *name,
		const int argc,
		const struct _lair_type *(*func_ptr)(LAIR_FUNCTION_SIG)) {
	check(r, name != NULL, ERR_RUNTIME, "Function name cannot be NULL.");
	check(r, env != NULL, ERR_RUNTIME, "Environment cannot be NULL.");
	check(r, strlen(name) > 0, ERR_RUNTIME, "Function name must be more than 0 chars.");

	/* Check to see if that function already exists: */
	const struct _lair_function *existing_func = _tst_map_get(env->c_functions, name, strlen(name));
	if (existing_func != NULL)
		throw_exception(r, ERR_RUNTIME, "Cannot override builtin functions.");

	struct _lair_function _stack_func = {
		.argc = argc,
		.argv = calloc(argc, sizeof(struct _lair_type)),
		.function_ptr = func_ptr
	};

	return _tst_map_insert(&(env->c_functions), name, strlen(name), &_stack_func, sizeof(struct _lair_function));
}

static void _reevaluate_until_break(struct _lair_runtime *r,
		struct _lair_env *env,
		struct _lair_ast *current_node) {
	/* This function will take an AST node and re-evaluate it in it's new context.
	 * We don't know until runtime if we're calling or defining a function,
	 * so the idea here is to turn LR_FUNCTION_ARG nodes into things that
	 * they actually are.
	 */
	struct _lair_ast *n = current_node;
	while (n &&
			n->atom.type != LR_DEDENT &&
			n->atom.type != LR_EOF) {
		struct _lair_ast *prev = n->prev;
		struct _lair_ast *next = n->next;
		if (n->next && n->next->prev == NULL) {
			n->next->prev = n;
		}

		if (prev && (prev->atom.type == LR_CALL || prev->atom.type == LR_INDENT)) {
			if (_is_callable_runtime(env, n)) {
				n->atom.type = LR_FUNCTION_CALL;
			}
		} else if (prev && prev->atom.type == LR_FUNCTION_CALL) {
			/* We need to parse this into something more useful, like a string or a number. */
			struct _lair_token token = {
				.token_str = n->atom.value.str,
				.token_type = LR_ERR,
				.indent_level = n->indent_level,
				.next = NULL,
				.prev = NULL,
			};
			_intuit_token_type(r, &token, n->atom.value.str);
			n->atom = _lair_atomize_token(&token);
		} else if (prev && (prev->atom.type == LR_FUNCTION_ARG || prev->atom.type == LR_FUNCTION_DEF)) {
			n->atom.type = LR_FUNCTION_ARG;
		} else {
			struct _lair_token token = {
				.token_str = n->atom.value.str,
				.token_type = LR_ERR,
				.indent_level = n->indent_level,
				.next = NULL,
				.prev = NULL,
			};
			_intuit_token_type(r, &token, n->atom.value.str);
			n->atom = _lair_atomize_token(&token);
		}

		n = next;
	}
}
static const struct _lair_type **_get_function_args(
		struct _lair_runtime *r,
		const int argc,
		const struct _lair_ast *ast_node,
		struct _lair_env *env) {
	if (argc == 0)
		return NULL;

	const struct _lair_ast *next_node = ast_node->next;
	if (next_node->atom.type == LR_FUNCTION_ARG &&
			strncmp(next_node->atom.value.str, "!", 1) == 0) {
		/* I guess we're building a JIT now. */
		/* Evalue at the RHS: */
		ast_node->next->atom.type = LR_CALL;

		/* This one is a function, the next one is a call, so we evaluate the
		 * one after that:
		 */
		_reevaluate_until_break(r, env, ast_node->next->next);
	}

	if (next_node->atom.type == LR_CALL) {
		check(r, argc == 1, ERR_RUNTIME, "Function takes more than one argument, but RHS is only one argument.");
		/* We need to evaluate the RHS before we can pass it to the function
		 * as arguments.
		 */
		const struct _lair_type **args = calloc(1, sizeof(struct _lair_type *));
		args[0] = _lair_env_eval(r, ast_node->next, env);
		return args;
	}

	const struct _lair_type **args = calloc(argc, sizeof(struct _lair_type *));
	const struct _lair_ast *cur_node = ast_node->next;
	int i = 0;
	/* XXX: Bug here. You could have a function like this:
	 * ! print ! add_two x ! add_two 2 2
	 * Which would do something weird. Need to be able to see how far
	 * the eval went.
	 */
	for (;i < argc; i++) {
		check(r, cur_node != NULL, ERR_RUNTIME, "Not enough arguments to function.");
		args[i] = _lair_env_eval(r, cur_node, env);
		cur_node = cur_node->next;
	}
	return args;
}

static const struct _lair_type *_lair_call_builtin(struct _lair_runtime *r, const struct _lair_ast *ast_node, struct _lair_env *env, const struct _lair_function *builtin_function) {
	int argc = builtin_function->argc;
	const struct _lair_type **argv = _get_function_args(r, argc, ast_node, env);
	return builtin_function->function_ptr(r, builtin_function->argc, argv);
}

struct _lair_env *_lair_env_with_parent(struct _lair_env *parent) {
	struct _lair_env *std_env = calloc(1, sizeof(struct _lair_env));
	std_env->parent = parent;
	return std_env;
}

/* This function creates a simple function that just returns a single value. It is
 * effectively an immuteable variable defined in the scope `env`.
 */
static int _lair_add_simple_function(struct _lair_env *env, const char *name, const struct _lair_type *value) {
	/* This is kind of dumb but whatever. */
	struct _lair_ast val = {
		.atom = {
			.type = value->type,
			/* HOLY SHIT UNIONS ARE AWESOME */
			.value = value->value
		}
	};

	return _tst_map_insert(&(env->not_variables), name, strlen(name), &val, sizeof(struct _lair_ast));
}

static const struct _lair_type *_lair_call_runtime_function(struct _lair_runtime *r, const struct _lair_ast *top_level_ast, const struct _lair_ast *defined_function_ast, struct _lair_env *env) {
	/* Figure out how many arguments are require for this function. */
	int argc = 0;
	struct _lair_ast *_first_function_arg = ((struct _lair_ast *)defined_function_ast)->next;
	struct _lair_ast *_func_eval_ast = _first_function_arg;
	while (_func_eval_ast->atom.type == LR_FUNCTION_ARG) {
		argc++;
		_func_eval_ast = _func_eval_ast->next;
	}

	if (argc > 0) {
		const struct _lair_type **args = _get_function_args(r, argc, top_level_ast, env);
		check(r, args != NULL, ERR_RUNTIME, "No arguments.");

		/* So heres how this works. What we do is create a new `struct _lair_env` object
		 * with the parent set to the current `env`, and then we dynamically
		 * create new 'functions' which return the function arguments. Since there
		 * are no real variables in Den, we just create functions that return the
		 * values that we want, and bind them into the local scope. Or something like
		 * that.
		 */
		struct _lair_env *scoped_env = _lair_env_with_parent(env);
		int i;
		struct _lair_ast *function_parameter = _first_function_arg;
		for (i = 0; i < argc; i++) {
			check(r, function_parameter->atom.type == LR_FUNCTION_ARG, ERR_SYNTAX,
					"Ran out of function argument parameters, buf function expects more.");
			_lair_add_simple_function(scoped_env, function_parameter->atom.value.str, args[i]);
			function_parameter = function_parameter->next;
		}
		const struct _lair_type *to_return = _lair_env_eval(r, _func_eval_ast, scoped_env);
		_lair_free_env(scoped_env);
		return to_return;
	} else {
		return _lair_env_eval(r, _func_eval_ast, env);
	}
}

static const struct _lair_type *_lair_call_function(struct _lair_runtime *r, const struct _lair_ast *ast_node, struct _lair_env *env) {
	if (!_is_callable(ast_node)) {
		char buf[512] = {0};
		snprintf(buf, sizeof(buf), "Cannot call a non-function: %s", _friendly_enum(ast_node->atom.type));
		check(r, _is_callable(ast_node), ERR_RUNTIME, buf);
	}
	/* Determine if the thing we're trying to call is a function
	 * or not. It might be an atom, in which case we need to check
	 * or function/c_function maps to see if it's in there.
	 */
	const char *func_name = ast_node->atom.value.str;
	const size_t func_len = strlen(ast_node->atom.value.str);

	struct _lair_env *cur_env = env;
	while (cur_env != NULL) {
		const struct _lair_function *builtin_function = _tst_map_get(cur_env->c_functions, func_name, func_len);
		if (builtin_function != NULL)
			return _lair_call_builtin(r, ast_node, env, builtin_function);

		/* Well if we're at this point this is a program-defined function. */
		const struct _lair_ast *defined_function_ast = _tst_map_get(cur_env->functions, func_name, func_len);
		if (defined_function_ast != NULL) {
			/* Don't call it a stack frame. */
			const struct _lair_ast *last_func = env->current_function;
			env->current_function = defined_function_ast;
			const struct _lair_type *value = _lair_call_runtime_function(r, ast_node, defined_function_ast, env);
			env->current_function = last_func;
			return value;
		}

		const struct _lair_ast *not_variable_ast = _tst_map_get(cur_env->not_variables, func_name, func_len);
		if (not_variable_ast != NULL) {
			const struct _lair_ast *last_func = env->current_function;
			env->current_function = defined_function_ast;
			const struct _lair_type *value = _lair_call_function(r, not_variable_ast, env);
			env->current_function = last_func;
			return value;
		}

		cur_env = (struct _lair_env *)cur_env->parent;
	}

	char buf[512] = {0};
	snprintf(buf, sizeof(buf), "No such function: %s", func_name);
	throw_exception(r, ERR_RUNTIME, buf);
	return NULL;
}

static const struct _lair_ast *_infer_atom_at_runtime(
		struct _lair_runtime *r,
		const struct _lair_ast *ast_node,
		const struct _lair_env *top_env) {
	/* This function attempts to modify an LR_ATOM into something more useful. */
	check(r, ast_node->atom.type == LR_ATOM, ERR_RUNTIME,
			"Can't infer an already inferred atom.");
	const char *func_name = ast_node->atom.value.str;
	const size_t func_len = strlen(ast_node->atom.value.str);
	struct _lair_ast *to_return = calloc(1, sizeof(struct _lair_ast));
	memcpy(to_return, ast_node, sizeof(struct _lair_ast));

	const struct _lair_env *env = top_env;
	while (env != NULL) {
		const struct _lair_function *builtin_function = _tst_map_get(env->c_functions, func_name, func_len);
		if (builtin_function != NULL) {
			to_return->atom.type = LR_FUNCTION_DEF;
			return to_return;
		}

		const struct _lair_ast *defined_function_ast = _tst_map_get(env->functions, func_name, func_len);
		if (defined_function_ast != NULL) {
			to_return->atom.type = LR_FUNCTION_DEF;
			return to_return;
		}

		const struct _lair_ast *not_variable_ast = (struct _lair_ast *)_tst_map_get(env->not_variables, func_name, func_len);
		if (not_variable_ast != NULL) {
			to_return->atom.type = not_variable_ast->atom.type;
			to_return->atom.value = not_variable_ast->atom.value;
			return to_return;
		}
		env = env->parent;
	}

	free(to_return);
	return NULL;
}

static inline const struct _lair_ast *_evalute_if_statement(struct _lair_runtime *r, const struct _lair_ast *ast, struct _lair_env *env) {
	const struct _lair_type *result = _lair_call_function(r, ast->next, env);
	const unsigned int initial_indent_level = ast->indent_level;
	if (result == _lair_canonical_true()) {
		/* If we're true then we want to jump to the next AST item and make sure
		 * that it's indentation level is *higher* than ours.
		 */
		while (ast->indent_level == initial_indent_level) {
			if (ast->next == NULL)
				error_and_die(ERR_SYNTAX, "No 'true' branch on if statement.");
			ast = ast->next;
		}

		check(r, ast->indent_level > initial_indent_level, ERR_SYNTAX, "No 'True' condition to follow.");
	} else {
		/* If we're false we just continue on to the next ast node LESS THAN OR EQUAL to ours.
		*/
		unsigned int skip_indent_level = ast->indent_level;
		while (ast->indent_level == skip_indent_level) {
			if (ast->next == NULL)
				error_and_die(ERR_SYNTAX, "Unexpected EOF.");
			ast = ast->next;
			if (ast->atom.type == LR_INDENT && ast->indent_level > initial_indent_level)
				skip_indent_level = ast->indent_level;
			if (ast->indent_level < skip_indent_level)
				break;
		}
	}

	return ast;
}

static inline const struct _lair_ast *_continue(const struct _lair_ast *ast) {
	/* Jump to next line here. */
	while (ast->atom.type != LR_INDENT) {
		ast = ast->next;
		if (ast == NULL || ast->atom.type == LR_EOF)
			break;
	}
	return ast;
}

static inline const struct _lair_ast *_call_and_continue(
		struct _lair_runtime *r,
		const struct _lair_ast *ast,
		struct _lair_env *env) {
	_lair_call_function(r, ast, env);
	ast = _continue(ast);
	return ast;
}

/* Inline to avoid another stack frame. */
inline const struct _lair_type *_lair_env_eval(
		struct _lair_runtime *r,
		const struct _lair_ast *ast,
		struct _lair_env *env) {
	/* We have a goto here to avoid creating a new stack frame, when we really just
	 * want to call this function again.
	 */
	/* THIS WHOLE FUCKING THING NEEDS A FINITE STATE MACHINE */
	const struct _lair_ast *possible_new_atom = NULL;
start_eval:
	switch (ast->atom.type) {
		case LR_OPERATOR:
			return _lair_call_function(r, ast, env);
		case LR_CALL:
			return _lair_call_function(r, ast->next, env);
		case LR_IF:
			ast = _evalute_if_statement(r, ast, env);
			goto start_eval;
		case LR_DEDENT:
			throw_exception(r, ERR_RUNTIME, "PANIC DEDENT");
			return NULL;
		case LR_ATOM:
			possible_new_atom = _infer_atom_at_runtime(r, ast, env);
			if (ast->next != NULL && ast->next->atom.type == LR_RETURN) {
				/* Evaluate the RHS, get the value. */
				const struct _lair_type *ret_val = _lair_env_eval(r, ast->next, env);
				/* Now stick that value as a simple function under the name of
				 * whatever the AST's atom is.
				 */
				_lair_add_simple_function(env, ast->atom.value.str, ret_val);
				ast = _continue(ast);
				if (ast == NULL)
					return ret_val;
				goto start_eval;
			} else if (ast->prev != NULL && ast->prev->atom.type == LR_INDENT &&
					_is_callable(ast)) {
				ast = _call_and_continue(r, ast, env);
				if (ast == NULL)
					return &possible_new_atom->atom;
				goto start_eval;
			} else if (possible_new_atom == NULL) {
				char buf[256] = {0};
				snprintf(buf, sizeof(buf), "Atom is undefined: %s", ast->atom.value.str);
				throw_exception(r, ERR_RUNTIME, buf);
			}
			return &possible_new_atom->atom;
		case LR_INDENT:
			env->currently_returning = 0;
			ast = ast->next;
			goto start_eval;
		case LR_RETURN:
			env->currently_returning = 1;
			return _lair_env_eval(r, ast->next, env);
		default:
			return &ast->atom;
	}
	/* TODO: Expire anything in this scope. Probably. */

	return NULL;
}

int _lair_eval(struct _lair_runtime *r, const struct _lair_ast *root) {
	struct _lair_env *std_env = _lair_standard_env(r);
	const struct _lair_ast *cur_ast_node = root->children;

	while (cur_ast_node != NULL) {
		if (cur_ast_node->atom.type == LR_CALL) {
			_lair_env_eval(r, cur_ast_node, std_env);
		} else if (cur_ast_node->atom.type == LR_FUNCTION_DEF) {
			const char *func_name = cur_ast_node->atom.value.str;
			const size_t func_name_len = strlen(func_name);

			const void *is_std_func = _tst_map_get(
					std_env->functions, func_name, func_name_len);
			const void *is_c_func = _tst_map_get(
					std_env->c_functions, func_name, func_name_len);
			if (!is_std_func && !is_c_func) {
				/* We're defining a function so insert it. */
				_tst_map_insert(&std_env->functions,
						func_name,
						func_name_len,
						cur_ast_node,
						sizeof(struct _lair_ast));
			} else {
				/* Call the function instead of defining it. */
				_lair_call_function(r, cur_ast_node, std_env);
			}
		}

		cur_ast_node = cur_ast_node->sibling;
	}

	_lair_free_env(std_env);
	return 0;
}

void builtin_cleanup(void *c_function) {
	struct _lair_function *f = (struct _lair_function *)c_function;
	free(f->argv);
}

void _lair_free_env(struct _lair_env *env) {
	_tst_map_destroy(env->c_functions, builtin_cleanup);
	_tst_map_destroy(env->functions, NULL);
	free(env);
}
