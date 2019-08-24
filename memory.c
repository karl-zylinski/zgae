#include "memory.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

#ifdef ENABLE_MEMORY_TRACING
    typedef struct AllocationCallstack 
    {
        const char** callstack;
        u32 callstack_num;
        void* ptr;
    } AllocationCallstack;

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

void memzero(void* p, size_t s)
{
    memset(p, 0, s);
}

void* mema(size_t s)
{
    void* p = malloc(s);
    #ifdef ENABLE_MEMORY_TRACING
        if (p)
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

void* memra_zero(void* cur, size_t s)
{
    void* p = memra(cur, s);
    memzero(p, s);
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

void* mema_copy(const void* data, size_t s)
{
    void* p = mema(s);
    memcpy(p, data, s);
    return p;
}
