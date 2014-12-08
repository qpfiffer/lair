// vim: noet ts=4 sw=4
#pragma once
#define LAIR_FUNCTION_SIG const int argc, const struct _lair_type *argv[]
/**
 * @file
 * Where the magic happens.
 */

/* Forward declarations. */
struct _lair_ast;
struct _lair_type;
struct _tst_map_node;

/**
 * @brief An object representing an environment in Lair.
 */
typedef struct _lair_env {
	struct _tst_map_node *c_functions; /**	C functions defined in the env. map of str -> _lair_function objects. */
	struct _tst_map_node *functions; /**	Functions defined in the program itself. map of str -> _lair_ast objects. */
} _lair_env;

/**
 * @brief Object representinga function.
 */
typedef struct _lair_function {
	const int argc; /**	The number of arguments in the argv array. This is also the number of arguments the function takes. */
	struct _lair_type **argv; /**	The arguments themselves. */
	struct _lair_type *(*function_ptr)(LAIR_FUNCTION_SIG); /**	A C function that will be called when this is evaluated. */
} _lair_function;

/**
 * Runs a lair AST. (Is that right? Am I fooling anyone?)
 * @param[in]	root	The root node of the AST.
 */
int _lair_eval(const struct _lair_ast *root);

/**
 * Generates and returns a map with the standard lib in it.
 */
_lair_env *_lair_standard_env();

/**
 * Adds a function to an environment.
 * On sucess, returns a pointer to a `_lair_arguments` struct with the argc count set to
 * the required number of arguments to the function.
 * On failure, returns NULL.
 * @param[in]	env		The environment to add the function to.
 * @param[in]	name	The name/symbol of the function.
 * @param[in]	argc	The argument require count for the function to be added.
 * @param[in]	func_ptr	The function to evaluate.
 */
_lair_function *_lair_add_builtin_function(_lair_env *env,
		const char *name,
		const int argc,
		struct _lair_type *(*func_ptr)(LAIR_FUNCTION_SIG));

/*
 * Evaluates an ast node inside of the passed environment.
 * @param[in]	ast	The AST node to evaluate.
 * @param[in]	env	The environment to operate under.
 */
const struct _lair_type *_lair_env_eval(const struct _lair_ast *ast, _lair_env *env);

/**
 * Frees the stuff in an environment, and the env itself.
 */
void _lair_free_env(_lair_env *env);
