#include "array.h"
#include "memory.h"
#include <string.h>

void* array_internal_grow(void* arr, size_t item_size)
{
    size_t old_capacity = array_capacity(arr);
    size_t new_capacity = old_capacity == 0 ? 1 : old_capacity*2;
    array_header_t* new_h = memra(arr ? array_header(arr) : 0, sizeof(array_header_t) + new_capacity*item_size);

    if (!arr)
        new_h->size = 0;
    
    new_h->capacity = new_capacity;

    return (void*)((uint8_t*)new_h + sizeof(array_header_t));
}

void* array_internal_copy_data(void *arr, size_t item_size)
{
   if (!arr)
      return NULL;

   uint64_t s = array_size(arr) * item_size;
   void* d = mema(s);
   memcpy(d, arr, s);
   return d;
}

size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size)
{
    if (idx == array_size(arr))
    {
        ++(array_header(arr)->size);
        return idx;
    }

    memmove(((char*)(arr)) + (idx + 1)*item_size, ((char*)(arr)) + idx*item_size, (array_size(arr) - idx)*item_size);
    ++(array_header(arr)->size);
    return idx;
}

void* array_internal_ensure_min_size(void* arr, size_t size, size_t item_size)
{
    if (size <= array_size(arr))
        return arr;

    if (size > array_capacity(arr))
    {
        size_t new_capacity = size;
        array_header_t* new_h = memra(arr ? array_header(arr) : 0, sizeof(array_header_t) + new_capacity*item_size);

        if (!arr)
            new_h->size = 0;
        
        new_h->capacity = new_capacity;
        arr = (void*)((uint8_t*)new_h + sizeof(array_header_t));
    }

    if (size > array_size(arr))
        array_header(arr)->size = size;

    return arr;
}

void array_internal_destroy(void* arr)
{
    memf(array_header(arr));
}