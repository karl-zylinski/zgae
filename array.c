#include "array.h"
#include "memory.h"
#include <string.h>
#include "debug.h"

ArrayHeader* array_internal_header(const void* arr)
{
    check(arr, "NULL arr passed");
    ArrayHeader* h = (ArrayHeader *)((char *)(arr) - sizeof(ArrayHeader));
    check_slow(h->marker == ARRAY_MARKER, "Trying to use array_* functions on non-array.");
    return h;
}

void* array_internal_grow(void* arr, size_t item_size)
{
    size_t old_cap = array_cap(arr);
    size_t new_cap = old_cap == 0 ? 1 : old_cap*2;
    ArrayHeader* new_h = memra(arr ? array_internal_header(arr) : 0, sizeof(ArrayHeader) + new_cap*item_size);

    if (!arr)
    {
        new_h->num = 0;
        new_h->marker = ARRAY_MARKER;
    }
    
    new_h->cap = new_cap;

    return (void*)((u8*)new_h + sizeof(ArrayHeader));
}

void* array_internal_copy_data(void *arr, size_t item_size)
{
   if (!arr)
      return NULL;

   size_t s = array_num(arr) * item_size;
   void* d = mema(s);
   memcpy(d, arr, s);
   return d;
}

size_t array_internal_make_insert_room(void* arr, size_t idx, size_t item_size)
{
    check(arr, "NULL arr passed");

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
        ArrayHeader* new_h = memra(arr ? array_internal_header(arr) : 0, sizeof(ArrayHeader) + new_cap*item_size);

        if (!arr)
        {
            new_h->num = 0;
            new_h->marker = ARRAY_MARKER;
        }
        
        new_h->cap = new_cap;
        arr = (void*)((u8*)new_h + sizeof(ArrayHeader));
    }

    if (size > array_num(arr))
        array_internal_header(arr)->num = size;

    return arr;
}

void array_internal_destroy(void* arr)
{
    memf(array_internal_header(arr));
}