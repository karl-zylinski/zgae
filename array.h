#pragma once
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

#define ARRAY_MARKER 0x25A3A22A // ZGAE ARRA
typedef struct array_header_t
{
    uint32_t marker;
    size_t num;
    size_t cap;
} array_header_t;

#define array_num(a) ((a) ? array_internal_header(a)->num : 0)
#define array_cap(a) ((a) ? array_internal_header(a)->cap : 0)

#define array_add(a, item) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    (a)[array_internal_header(a)->num++] = item

#define array_fill_and_set(a, idx, item) \
    idx >= array_num(a) ? a = array_internal_ensure_min_num(a, idx + 1, sizeof(*a)) : 0, \
    (a)[idx] = item

#define array_full(a) (array_cap(a) == array_num(a))

#define array_copy_data(a) (array_internal_copy_data(a, sizeof(*(a))))
#define array_insert(a, item, idx) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    (a)[array_internal_make_insert_room(a, idx, sizeof(*(a)))] = item

#define array_destroy(a) ((a) ? array_internal_destroy(a) : 0)

array_header_t* array_internal_header(void* arr);
void* array_internal_grow(void* arr, size_t item_size);
void* array_internal_copy_data(void *arr, size_t item_size);
size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size);
void* array_internal_ensure_min_num(void* arr, size_t num, size_t item_size);
void array_internal_destroy(void* arr);