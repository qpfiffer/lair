// vim: noet ts=4 sw=4
#pragma once
/**
 * @file
 * Where the magic happens.
 */

/* Forward declaration for _lair_env so we don't have to include
 * map.h here. */
struct _tst_map_node;
struct _lair_ast;

/**
 * @brief An object representing an environment in Lair.
 */
typedef struct _lair_env {
	struct _tst_map_node *env_map;
} _lair_env;

/**
 * Runs a lair AST. (Is that right? Am I fooling anyone?)
 */
int _lair_eval(const struct _lair_ast *ast);

/**
 * Generates and returns a map with the standard lib in it.
 */
_lair_env *_lair_standard_env();

/**
 * Frees the stuff in an environment, and the env itself.
 */
void _lair_free_env(_lair_env *env);
