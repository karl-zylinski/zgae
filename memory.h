#pragma once

#include <stdlib.h>
#include <string.h>

static void* zalloc(size_t size)
{
    void* p = malloc(size);
    return p;
}

static void* zalloc_zero(size_t size)
{
    void* p = zalloc(size);
    memset(p, 0, size);
    return p;
}

static void zfree(void* p)
{
    free(p);
}