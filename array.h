#pragma once
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

#include <stdint.h>
#include <stdlib.h>

struct array_header
{
    uint32_t size;
    uint32_t capacity;
};

#define array_header(a) \
    ((struct array_header *)((char *)(a) - sizeof(struct array_header)))

#define array_size(a) ((a) ? array_header(a)->size : 0)
#define array_capacity(a) ((a) ? array_header(a)->capacity : 0)

#define array_push(a, item) \
    array_full(a) ? a = _array_grow(a, sizeof(*a)) : 0, \
    a[array_header(a)->size++] = item

#define array_full(a) (array_capacity(a) == array_size(a))

#define array_copy_data(a) (_array_copy_data(a, sizeof(*(a))))
#define array_insert(a, item, idx) \
    array_full(a) ? a = _array_grow(a, sizeof(*a)) : 0, \
    a[_array_make_insert_room(a, idx, sizeof(*(a)))] = item

void* _array_grow(void* old_a, size_t item_size);
void * _array_copy_data(void *a, int item_size);
int _array_make_insert_room(void* a, int idx, int item_size);