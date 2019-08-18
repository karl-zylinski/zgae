#include "memory.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>

void get_backtrace(uint32_t steps, char*** callstack, uint32_t* num)
{
    if (steps > 32)
        steps = 32;

    void* backtraces[32];
    *num = backtrace(backtraces, steps);
    *callstack = backtrace_symbols(backtraces, *num);
}

typedef struct allocation_callstack_t 
{
    char** callstack;
    uint32_t callstack_num;
    void* ptr;
} allocation_callstack_t;


static const uint32_t MAX_ALLOC_CALLSTACKS = 8096;
static allocation_callstack_t alloc_callstacks[MAX_ALLOC_CALLSTACKS];

void memory_init()
{
    memzero(alloc_callstacks, sizeof(allocation_callstack_t) * MAX_ALLOC_CALLSTACKS);
}

void add_allocation_callstack(void* ptr)
{
    for (uint32_t i = 0; i < MAX_ALLOC_CALLSTACKS; ++i)
    {
        if (alloc_callstacks[i].ptr == NULL)
        {
            allocation_callstack_t* ac = alloc_callstacks + i;
            get_backtrace(10, &ac->callstack, &ac->callstack_num);
            ac->ptr = ptr;
            return;
        }
    }

    error("Out of memory debug allocation callstacks!");
}

void remove_allocation_callstack(void* ptr)
{
    for (uint32_t i = 0; i < MAX_ALLOC_CALLSTACKS; ++i)
    {
        if (alloc_callstacks[i].ptr == ptr)
        {
            allocation_callstack_t* ac = alloc_callstacks + i;
            free(ac->callstack);
            memzero(ac, sizeof(allocation_callstack_t));
            return;
        }
    }

    error("Tried to remove non-existing allocation callstack");
}


void memzero(void* p, size_t s)
{
    memset(p, 0, s);
}

void* mema(size_t s)
{
    void* p = malloc(s);
    #ifdef ENABLE_MEMORY_TRACING
        add_allocation_callstack(p);
    #endif
    return p;
}

void* mema_zero(size_t s)
{
    void* p = mema(s);
    memzero(p, s);
    return p;
}

void* memra(void* cur, size_t s)
{
    void* p = realloc(cur, s);

    #ifdef ENABLE_MEMORY_TRACING
        remove_allocation_callstack(cur);
        add_allocation_callstack(p);
    #endif
    
    return p;
}

void* memra_zero(void* cur, size_t s)
{
    void* p = memra(cur, s);
    memzero(p, s);
    return p;
}

void memf(void* p)
{
    free(p);

    #ifdef ENABLE_MEMORY_TRACING
        remove_allocation_callstack(p);
    #endif
}

void memory_check_leaks()
{
    for (uint32_t ac_idx = 0; ac_idx < MAX_ALLOC_CALLSTACKS; ++ac_idx)
    {
        if (alloc_callstacks[ac_idx].ptr != NULL)
        {
            fprintf(stderr, "MEMORY LEAK, backtrace: \n");

            for (uint32_t cidx = 2; cidx < alloc_callstacks[ac_idx].callstack_num; ++cidx)
            {
                fprintf(stderr, "%s", alloc_callstacks[ac_idx].callstack[cidx]);
                fprintf(stderr, "\n");
            }

            error("Exiting.");
        }
    }
}

void memcpy_alloc(void** dest, void* source, size_t s)
{
    *dest = mema(s);
    memcpy(*dest, source, s);
}
