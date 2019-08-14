#pragma once
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

typedef struct array_header_t
{
    size_t size;
    size_t capacity;
} array_header_t;

#define array_header(a) \
    ((array_header_t *)((char *)(a) - sizeof(array_header_t)))

#define array_size(a) ((a) ? array_header(a)->size : 0)
#define array_capacity(a) ((a) ? array_header(a)->capacity : 0)

#define array_push(a, item) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    a[array_header(a)->size++] = item

#define array_set(a, idx, item) \
    idx >= array_size(a) ? a = array_internal_ensure_min_size(a, idx + 1, sizeof(*a)) : 0, \
    a[idx] = item

#define array_full(a) (array_capacity(a) == array_size(a))

#define array_copy_data(a) (array_internal_copy_data(a, sizeof(*(a))))
#define array_insert(a, item, idx) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    a[array_internal_make_insert_room(a, idx, sizeof(*(a)))] = item

#define array_destroy(a) ((a) ? array_internal_destroy(a) : 0)

void* array_internal_grow(void* arr, size_t item_size);
void* array_internal_copy_data(void *arr, size_t item_size);
size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size);
void* array_internal_ensure_min_size(void* arr, size_t size, size_t item_size);
void array_internal_destroy(void* arr);