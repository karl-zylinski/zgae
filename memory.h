#pragma once

#include <stdlib.h>
#include <string.h>

static inline void* mema(size_t size)
{
    void* p = malloc(size);
    return p;
}

static inline void* memaz(size_t size)
{
    void* p = mema(size);
    memset(p, 0, size);
    return p;
}

static inline void* memra(void* cur, size_t size)
{
    void* p = realloc(cur, size);
    return p;
}

static inline void* memraz(void* cur, size_t size)
{
    void* p = memra(cur, size);
    memset(p, 0, size);
    return p;
}

static inline void memf(void* p)
{
    free(p);
}

void memory_check_leaks();
