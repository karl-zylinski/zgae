#include "array.h"
#include "memory.h"
#include <string.h>
#include "debug.h"

array_header_t* array_internal_header(void* arr)
{
    array_header_t* h = (array_header_t *)((char *)(arr) - sizeof(array_header_t));
    check(h->marker == ARRAY_MARKER, "Trying to use array_* functions on non-array.");
    return h;
}

void* array_internal_grow(void* arr, size_t item_size)
{
    size_t old_cap = array_cap(arr);
    size_t new_cap = old_cap == 0 ? 1 : old_cap*2;
    array_header_t* new_h = memra(arr ? array_internal_header(arr) : 0, sizeof(array_header_t) + new_cap*item_size);

    if (!arr)
    {
        new_h->num = 0;
        new_h->marker = ARRAY_MARKER;
    }
    
    new_h->cap = new_cap;

    return (void*)((uint8_t*)new_h + sizeof(array_header_t));
}

void* array_internal_copy_data(void *arr, size_t item_size)
{
   if (!arr)
      return NULL;

   uint64_t s = array_num(arr) * item_size;
   void* d = mema(s);
   memcpy(d, arr, s);
   return d;
}

size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size)
{
    if (idx == array_num(arr))
    {
        ++(array_internal_header(arr)->num);
        return idx;
    }

    memmove(((char*)(arr)) + (idx + 1)*item_size, ((char*)(arr)) + idx*item_size, (array_num(arr) - idx)*item_size);
    ++(array_internal_header(arr)->num);
    return idx;
}

void* array_internal_ensure_min_num(void* arr, size_t size, size_t item_size)
{
    if (size <= array_num(arr))
        return arr;

    if (size > array_cap(arr))
    {
        size_t new_cap = size;
        array_header_t* new_h = memra(arr ? array_internal_header(arr) : 0, sizeof(array_header_t) + new_cap*item_size);

        if (!arr)
        {
            new_h->num = 0;
            new_h->marker = ARRAY_MARKER;
        }
        
        new_h->cap = new_cap;
        arr = (void*)((uint8_t*)new_h + sizeof(array_header_t));
    }

    if (size > array_num(arr))
        array_internal_header(arr)->num = size;

    return arr;
}

void array_internal_destroy(void* arr)
{
    memf(array_internal_header(arr));
}