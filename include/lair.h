// vim: noet ts=4 sw=4
#pragma once

#include <setjmp.h>

/** @file
 * @brief Main functions intended for outside usage.
 */

/**
 * Loads a file into memory.
 * @param[in]	file_path	The file to load.
 * @param[out]	buf_size	The size of the loaded buffer.
 */
char *lair_load_file(const char *file_path, size_t *buf_size);

/**
 * Executes a program.
 * @param[in]	program	The program to be executed.
 * @param[in]	len	The length of the program, in bytes.
 */
int lair_execute(const char *program, const size_t len);

/**
 * Unloads a loaded file.
 * @param[in]	loaded	The loaded buffer.
 * @param[in]	buf_size	The size of the loaded buffer.
 */
void lair_unload_file(char *loaded, size_t buf_size);

/**
 * Lair runtime, holds various state and error constructs.
 */
struct _lair_runtime {
	struct _lair_type *exception;
	jmp_buf exception_buffer;
};
