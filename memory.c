#include "memory.h"
#include "debug.h"
#include <string.h>

void* mema(size_t size)
{
    void* p = malloc(size);
    return p;
}

void* mema_zero(size_t size)
{
    void* p = mema(size);
    memset(p, 0, size);
    return p;
}

void* memra(void* cur, size_t size)
{
    void* p = realloc(cur, size);
    return p;
}

void* memra_zero(void* cur, size_t size)
{
    void* p = memra(cur, size);
    memset(p, 0, size);
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