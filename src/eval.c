// vim: noet ts=4 sw=4
#include "eval.h"
#include "map.h"
#include "parse.h"

_lair_env *_lair_standard_env() {
	_lair_env *std_env = calloc(1, sizeof(_lair_env));
	return std_env;
}

int _lair_eval(const struct _lair_ast *ast) {
	_lair_env *std_env = _lair_standard_env();
	_lair_free_env(std_env);
	return 0;
}

void _lair_free_env(_lair_env *env) {
	free(env);
}
