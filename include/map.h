// vim: noet ts=4 sw=4
#pragma once
#include <stdlib.h>
/**
 * @file
 * Holds a nice little map implementation. It's not a hash map, it's a ternary
 * search tree. It maps char * -> void *, and can be more efficient (in some
 * random cases) than your good old hash map.
 *
 * IT'S MY PROJECT I'LL USE THE DATA STRUCTURES I WANT TO, DAMN IT!
 */

/**
 * @brief The thing what goes in the tree.
 */
struct _tst_map_node {
	struct _tst_map_node *hikid;
	struct _tst_map_node *eqkid;
	struct _tst_map_node *lokid;

	char node_char; /**	The character for this node. */
	void *value; /** If this node has a value, this will be non-NULL. */
};

/**
 * Function used to insert things into the tree. This function makes a copy of whatever
 * you put into the tree.
 * Returns 0 on success.
 * @param[in]	root	The root node of the tree you want to insert into.
 * @param[in]	key		The string key you want to use to reference the data you're inserting.
 * @param[in]	value	The data you want to insert.
 * @param[in]	vsize	The size of the value you want to insert.
 */
int _tst_map_insert(struct _tst_map_node **root, const char *key, const size_t klen, const void *value, const size_t vsize);

/**
 * Function used to get something out of the tree.
 * Returns NULL on failure.
 */
const void *_tst_map_get(struct _tst_map_node *root, const char *key, const size_t klen);

/**
 * Deletes something out of the tree.
 * Returns 0 on successful deletion.
 */
int _tst_map_delete(struct _tst_map_node *root, const char *key, const size_t klen);

/**
 * Frees the entire tree.
 * @param[in]	root	The root node.
 * @param[in]	per_value_cleanup	Optional callback function that will be called right before the vale is freed.
 */
void _tst_map_destroy(struct _tst_map_node *root, void (*per_value_cleanup)(void *data));
