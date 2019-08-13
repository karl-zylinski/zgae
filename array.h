#pragma once
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

struct array_header
{
    uint32 size;
    uint32 capacity;
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

void* _array_grow(void* old_a, uint32 item_size);
void * _array_copy_data(void *a, uint32 item_size);
uint32 _array_make_insert_room(void* a, uint32 idx, uint32 item_size);