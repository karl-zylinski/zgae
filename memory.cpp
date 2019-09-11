#include "memory.h"
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ENABLE_MEMORY_TRACING
    struct AllocationCallstack 
    {
        char** callstack;
        u32 callstack_num;
        void* ptr;
    };

    #define MAX_ALLOC_CALLSTACKS 8096
    static AllocationCallstack alloc_callstacks[MAX_ALLOC_CALLSTACKS];

    static void add_allocation_callstack(void* ptr)
    {
        for (u32 i = 0; i < MAX_ALLOC_CALLSTACKS; ++i)
        {
            if (alloc_callstacks[i].ptr == NULL)
            {
                Backtrace bt = debug_get_backtrace(10);

                AllocationCallstack ac = {
                    .callstack = bt.function_calls,
                    .callstack_num = bt.function_calls_num,
                    .ptr = ptr
                };

                alloc_callstacks[i] = ac;

                return;
            }
        }

        error("Out of memory debug allocation callstacks!");
    }

    static void remove_allocation_callstack(void* ptr, bool must_be_present)
    {
        for (u32 i = 0; i < MAX_ALLOC_CALLSTACKS; ++i)
        {
            if (alloc_callstacks[i].ptr == ptr)
            {
                AllocationCallstack* ac = alloc_callstacks + i;
                free(ac->callstack);
                memzero(ac, sizeof(AllocationCallstack));
                return;
            }
        }

        if (must_be_present)
            error("Tried to remove non-existing allocation callstack");
    }
#endif

void memory_init()
{
    #ifdef ENABLE_MEMORY_TRACING
        memzero(alloc_callstacks, sizeof(AllocationCallstack) * MAX_ALLOC_CALLSTACKS);
    #endif
}

void memzero(void* p, u64 s)
{
    memset(p, 0, s);
}

void* mema(u64 s)
{
    void* p = malloc(s);
    #ifdef ENABLE_MEMORY_TRACING
        if (p)
            add_allocation_callstack(p);
    #endif
    return p;
}

void* mema_zero(u64 s)
{
    void* p = mema(s);
    memzero(p, s);
    return p;
}

void* memra(void* cur, u64 s)
{
    #ifdef ENABLE_MEMORY_TRACING
        if (cur)
            remove_allocation_callstack(cur, false);
    #endif

    void* p = realloc(cur, s);

    #ifdef ENABLE_MEMORY_TRACING
        if (p)
            add_allocation_callstack(p);
    #endif
    
    return p;
}

void* memra_zero_added(void* cur, u64 new_size, u64 old_size)
{
    void* p = memra(cur, new_size);
    memzero(((i8*)p) + old_size, new_size - old_size);
    return p;
}

void memf(void* p)
{
    #ifdef ENABLE_MEMORY_TRACING
        if (p)
            remove_allocation_callstack(p, true);
    #endif
        
    free(p);
}

void memory_check_leaks()
{
    #ifdef ENABLE_MEMORY_TRACING
        for (u32 ac_idx = 0; ac_idx < MAX_ALLOC_CALLSTACKS; ++ac_idx)
        {
            if (alloc_callstacks[ac_idx].ptr != NULL)
            {
                fprintf(stderr, "MEMORY LEAK, backtrace: \n");

                for (u32 cidx = 2; cidx < alloc_callstacks[ac_idx].callstack_num; ++cidx)
                {
                    fprintf(stderr, "%s", alloc_callstacks[ac_idx].callstack[cidx]);
                    fprintf(stderr, "\n");
                }
            }
        }
    #endif
}

void* mema_copy(void* data, u64 s)
{
    void* p = mema(s);
    memcpy(p, data, s);
    return p;
}

void mema__repl(void** p, u64 s)
{
    void* np = mema(s);
    memcpy(np, *p, s);
    *p = np;
}
