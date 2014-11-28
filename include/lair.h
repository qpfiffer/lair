// vim: noet ts=4 sw=4
#pragma once

typedef struct lair_state {
} lair_state;

int lair_load_file(const char *file_path, char *loaded, size_t *buf_size);
const lair_state lair_parse(const char *buf, const size_t buf_size);
int lair_execute(const lair_state state);
void lair_unload_file(char *loaded, size_t buf_size);
