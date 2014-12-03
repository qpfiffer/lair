// vim: noet ts=4 sw=4
#include <assert.h>
#include <string.h>
#include "map.h"

static const int _tst_insert(_tst_map_node **cur_node, const char *key, size_t klen, const void *value, const size_t vsiz) {
	/* This function is just scary looking because we're passing around pointers
	 * to pointers. It's not that bad.
	 */
	const char current_char = key[0];

	if (*cur_node == NULL) {
		*cur_node = calloc(1, sizeof(_tst_map_node));
		(*cur_node)->node_char = current_char;
	}

	if (current_char < (*cur_node)->node_char) {
		return _tst_insert(&((*cur_node)->lokid), key, klen, value, vsiz);
	} else if (current_char == (*cur_node)->node_char) {
		if (klen > 1) {
			return _tst_insert(&(*cur_node)->eqkid, key + 1, klen - 1, value, vsiz);
		} else {
			if ((*cur_node)->value != NULL) // Duplicate?
				return 1;

			(*cur_node)->value = calloc(1, vsiz);
			memcpy((*cur_node)->value, value, vsiz);
			return 0;
		}
	} else {
		return _tst_insert(&((*cur_node)->hikid), key, klen, value, vsiz);
	}
}

const int _tst_map_insert(_tst_map_node *root, const char *key, const size_t klen, const void *value, const size_t vsiz) {
	assert(root != NULL);
	assert(key != NULL);
	assert(klen > 0);

	size_t _klen = klen;
	return _tst_insert(&root, key, _klen, value, vsiz);
}

