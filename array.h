#pragma once
#include "memory.h"
// Adaption of Strechy buffers by Sean Barret and Niklas Gray

typedef struct 
{
    uint32_t size;
    uint32_t capacity;
} array_header_t;

#define array_header(a) \
    ((array_header_t *)((char *)(a) - sizeof(array_header_t)))

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

static void* _array_grow(void* old_a, size_t item_size)
{
    size_t old_capacity = array_capacity(old_a);
    size_t new_capacity = old_capacity == 0 ? 1 : old_capacity*2;
    array_header_t* new_h = memra(old_a ? array_header(old_a) : 0, sizeof(array_header_t) + new_capacity*item_size);

    if (!old_a)
        new_h->size = 0;
    
    new_h->capacity = new_capacity;

    return (void*)((uint8_t*)new_h + sizeof(array_header_t));
}

static void * _array_copy_data(void *a, int item_size)
{
   if (!a)
      return NULL;

   int s = array_size(a) * item_size;
   void* d = mema(s);
   memcpy(d, a, s);
   return d;
}

static int _array_make_insert_room(void* a, int idx, int item_size)
{
    if (idx == array_size(a))
    {
        ++(array_header(a)->size);
        return idx;
    }

    memmove(((char*)(a)) + (idx + 1)*item_size, ((char*)(a)) + idx*item_size, (array_size(a) - idx)*item_size);
    ++(array_header(a)->size);
    return idx;
}
