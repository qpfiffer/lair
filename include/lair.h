// vim: noet ts=4 sw=4
#pragma once
#include <sys/queue.h>

typedef enum {
	LAIR_NUM,
	LAIR_FUNC,
	LAIR_STR
} lair_type;

typedef struct lair_object {
	const char *name;
	const lair_type type;
} lair_object;

/**
 * This is an object representing a parsed `den` program.
 */
typedef struct lair_state {
	LIST_ENTRY(lair_object) objects;
} lair_state;

/**
 * Loads a file into memory.
 * @param[in]	file_path	The file to load.
 * @param[out]	loaded	The loaded file in memory.
 * @param[out]	buf_size	The size of the loaded buffer.
 */
int lair_load_file(const char *file_path, char *loaded, size_t *buf_size);

const lair_state lair_parse(const char *buf, const size_t buf_size);
int lair_execute(const lair_state state);

/**
 * Unloads a loaded file.
 * @param[in]	loaded	The loaded buffer.
 * @param[in]	buf_size	The size of the loaded buffer.
 */
void lair_unload_file(char *loaded, size_t buf_size);
