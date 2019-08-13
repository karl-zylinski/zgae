#include "array.h"
#include "memory.h"
#include <string.h>

void* array_internal_grow(void* old_a, uint32 item_size)
{
    uint32 old_capacity = array_capacity(old_a);
    uint32 new_capacity = old_capacity == 0 ? 1 : old_capacity*2;
    struct array_header* new_h = memra(old_a ? array_header(old_a) : 0, sizeof(struct array_header) + new_capacity*item_size);

    if (!old_a)
        new_h->size = 0;
    
    new_h->capacity = new_capacity;

    return (void*)((uint8*)new_h + sizeof(struct array_header));
}

void* array_internal_copy_data(void *a, uint32 item_size)
{
   if (!a)
      return NULL;

   uint64 s = array_size(a) * item_size;
   void* d = mema(s);
   memcpy(d, a, s);
   return d;
}

uint32 array_internal_make_insert_room(void* a, uint32 idx, uint32 item_size)
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
