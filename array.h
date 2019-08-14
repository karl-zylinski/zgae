#pragma once
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

struct array_header
{
    size_t size;
    size_t capacity;
};

#define array_header(a) \
    ((struct array_header *)((char *)(a) - sizeof(struct array_header)))

#define array_size(a) ((a) ? array_header(a)->size : 0)
#define array_capacity(a) ((a) ? array_header(a)->capacity : 0)

#define array_push(a, item) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    a[array_header(a)->size++] = item

#define array_full(a) (array_capacity(a) == array_size(a))

#define array_copy_data(a) (array_internal_copy_data(a, sizeof(*(a))))
#define array_insert(a, item, idx) \
    array_full(a) ? a = array_internal_grow(a, sizeof(*a)) : 0, \
    a[array_internal_make_insert_room(a, idx, sizeof(*(a)))] = item

void* array_internal_grow(void* old_arr, size_t item_size);
void* array_internal_copy_data(void *arr, size_t item_size);
size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size);