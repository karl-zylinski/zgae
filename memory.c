#include "memory.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>

void* mema(size_t s)
{
    void* p = malloc(s);
    return p;
}

void* mema_zero(size_t s)
{
    void* p = mema(s);
    memset(p, 0, s);
    return p;
}

void* memra(void* cur, size_t s)
{
    void* p = realloc(cur, s);
    return p;
}

void* memra_zero(void* cur, size_t s)
{
    void* p = memra(cur, s);
    memset(p, 0, s);
    return p;
}

void memf(void* p)
{
    free(p);
}

void memory_check_leaks()
{
    error("PLZ IMPLEMENT.");
}

void memcpy_alloc(void** dest, void* source, size_t s)
{
    *dest = mema(s);
    memcpy(*dest, source, s);
}

void memzero(void* p, size_t s)
{
    memset(p, 0, s);
}