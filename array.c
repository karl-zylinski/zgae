#include "array.h"
#include "memory.h"
#include <string.h>

void* array_internal_grow(void* old_arr, uint32_t item_size)
{
    uint32_t old_capacity = array_capacity(old_arr);
    uint32_t new_capacity = old_capacity == 0 ? 1 : old_capacity*2;
    struct array_header* new_h = memra(old_arr ? array_header(old_arr) : 0, sizeof(struct array_header) + new_capacity*item_size);

    if (!old_arr)
        new_h->size = 0;
    
    new_h->capacity = new_capacity;

    return (void*)((uint8_t*)new_h + sizeof(struct array_header));
}

void* array_internal_copy_data(void *arr, uint32_t item_size)
{
   if (!arr)
      return NULL;

   uint64_t s = array_size(arr) * item_size;
   void* d = mema(s);
   memcpy(d, arr, s);
   return d;
}

uint32_t array_internal_make_insert_room(void* arr, uint32_t idx, uint32_t item_size)
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