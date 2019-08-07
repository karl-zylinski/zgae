#pragma once

static void* zalloc(size_t size)
{
    void* p = malloc(size);
    return p;
}

static void zfree(void* p)
{
    free(p);
}