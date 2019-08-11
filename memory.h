#pragma once

#include <stdlib.h>
#include <string.h>

static void* mema(size_t size)
{
    void* p = malloc(size);
    return p;
}

static void* memaz(size_t size)
{
    void* p = mema(size);
    memset(p, 0, size);
    return p;
}

static void memf(void* p)
{
    free(p);
}