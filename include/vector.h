// vim: noet ts=4 sw=4
#pragma once
#include <stdlib.h>

/* xXx STRUCT=vector xXx
 * xXx DESCRIPTION=A simple vector object. Auto-expands and whatever. xXx
 * xXx item_size=The maximum size of each item. xXx
 * xXx max_size=Used internally to track the current vector's maximum number of elements. xXx
 * xXx count=Used internally to track the current vector's current count of items. xXx
 * xXx *items=The actual memory used for the items stored. xXx
 */
typedef struct vector {
	const size_t item_size;
	size_t max_size;
	size_t count;
	void *items;
} vector;

/* xXx FUNCTION=vector_new xXx
 * xXx DESCRIPTION=Creates a new vector object. xXx
 * xXx RETURNS=A new vector object. xXx
 * xXx item_size=The maximum size of each item. xXx
 * xXx initial_element_count=If you know your amount of objects ahead of time, set this accordingly. Otherwise just guess. The closer you get the fewer mallocs will happen. xXx
 */
vector *vector_new(const size_t item_size, const size_t initial_element_count);

/* xXx FUNCTION=vector_append xXx
 * xXx DESCRIPTION=Adds a new element to a vector. xXx
 * xXx RETURNS=1 on success. xXx
 * xXx *vec=The vector to add the new item to. xXx
 * xXx *item=The item to add to the vector. xXx
 * xXx item_size=The size of the item to be added. xXx
 */
int vector_append(vector *vec, const void *item, const size_t item_size);

/* xXx FUNCTION=vector_append_ptr xXx
 * xXx DESCRIPTION=Similar to vector_append but copies just the pointer value, not what it points to. xXx
 * xXx RETURNS=1 on success. xXx
 * xXx *vec=The vector to add the pointer to. xXx
 * xXx *pointer=The item to add to the vector. xXx
 */
int vector_append_ptr(vector *vec, const void *pointer);

/* xXx FUNCTION=vector_get xXx
 * xXx DESCRIPTION=Gets the nth element of the given vector. xXx
 * xXx RETURNS=A constant pointer to the nth item in the vector. xXx
 * xXx *vec=The vector to get the item from. xXx
 * xXx i=The item you want to retrieve. xXx
 */
const void *vector_get(const vector *vec, const unsigned int i);

/* xXx FUNCTION=vector_free xXx
 * xXx DESCRIPTION=Cleans up and removes a vector's allocated memory. xXx
 * xXx RETURNS=Nothing. xXx
 * xXx *to_free=The vector to free. xXx
 */
void vector_free(vector *to_free);

