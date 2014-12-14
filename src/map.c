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

const int _tst_map_insert(_tst_map_node **root, const char *key, const size_t klen, const void *value, const size_t vsiz) {
	assert(key != NULL);
	assert(klen > 0);

	size_t _klen = klen;
	return _tst_insert(root, key, _klen, value, vsiz);
}

const void *_tst_map_get(_tst_map_node *current_node, const char *key, const size_t klen) {
	const char current_char = key[0];

	if (current_node == NULL)
		return NULL;

	if (current_char < current_node->node_char) {
		return _tst_map_get(current_node->lokid, key, klen);
	} else if (current_char == current_node->node_char) {
		if (klen > 1)
			return _tst_map_get(current_node->eqkid, key + 1, klen - 1);
		return current_node->value;
	} else {
		return _tst_map_get(current_node->hikid, key, klen);
	}
}

/* struct used to teardown the map. */
struct destroy_queue {
	struct destroy_queue *next;
	void *data;
};

static inline void dq_push(struct destroy_queue **top, void *data) {
	struct destroy_queue *to_push = NULL;
	to_push = calloc(1, sizeof(struct destroy_queue));
	to_push->next = *top;
	to_push->data = data;
	*top = to_push;
}

static inline void *dq_pop(struct destroy_queue **stack) {
	struct destroy_queue *top = *stack;
	*stack = top->next;
	void *data = top->data;

	free(top);
	return data;
}

void _tst_map_destroy(_tst_map_node *root, void (*per_value_cleanup)(void *data)) {
	if (root == NULL)
		return;
	/* TODO: This whole function. */
	struct destroy_queue *top = calloc(1, sizeof(struct destroy_queue));
	dq_push(&top, root);

	while (top->next != NULL) {
		_tst_map_node *cur_node = (_tst_map_node *)dq_pop(&top);

		if (cur_node->lokid != NULL)
			dq_push(&top, (void *)cur_node->lokid);

		if (cur_node->eqkid != NULL)
			dq_push(&top, (void *)cur_node->eqkid);

		if (cur_node->hikid != NULL)
			dq_push(&top, (void *)cur_node->hikid);

		if (per_value_cleanup != NULL && cur_node->value != NULL)
			per_value_cleanup(cur_node->value);
		free(cur_node->value);
		free(cur_node);
	}

	free(top);
}
