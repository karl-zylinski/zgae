#pragma once

#include <string.h>
#include <stdlib.h>
#include "helpers.h"

// This will eff your performance so only enable when debugging memory leaks in the heap allocator.
#define ENABLE_MEMORY_TRACING

#if defined(ENABLE_MEMORY_TRACING)
    #include "callstack_capturer.h"
    void add_captured_callstack(const CapturedCallstack& cc);
    void remove_captured_callstack(void* p);
    void ensure_captured_callstacks_unused();
#endif

void memory_init();
void memory_shutdown();

static void* zalloc(size_t size)
{
    void* p = malloc(size);
    #if defined(ENABLE_MEMORY_TRACING)
        add_captured_callstack(callstack_capture(1, p));
    #endif
    return p;
}

static void* zalloc_zero(size_t size)
{
    void* p = zalloc(size);
    memzero(p, size);
    return p;
}

static void zfree(void* p)
{
    #if defined(ENABLE_MEMORY_TRACING)
        remove_captured_callstack(p);
    #endif

    free(p);
}

static void heap_check_clean()
{
    #if defined(ENABLE_MEMORY_TRACING)
        ensure_captured_callstacks_unused();
    #endif
}

static unsigned ptr_diff(const void* ptr1, const void* ptr2)
{
    return (unsigned)((unsigned char*)ptr2 - (unsigned char*)ptr1);
}

static void* ptr_add(const void* ptr1, size_t offset)
{
    return (void*)((unsigned char*)ptr1 + offset);
}

static void* ptr_sub(const void* ptr1, size_t offset)
{
    return (void*)((unsigned char*)ptr1 - offset);
}
