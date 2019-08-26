#include "array.h"
#include "memory.h"
#include <string.h>

void* arr_mem_realloc(void* d, size_t s)
{
    return memra(d, s);
}

void* arr_mem_copy(void* d, size_t s)
{
    return mema_copy(d, s);
}

void arr_mem_free(void* d)
{
    memf(d);
}

void arr_mem_move(void* dest, void* source, size_t s)
{
    memmove(dest, source, s);
}