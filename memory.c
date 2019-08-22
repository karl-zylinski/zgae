#include "memory.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

#ifdef ENABLE_MEMORY_TRACING
    typedef struct allocation_callstack_t 
    {
        const char** callstack;
        uint32_t callstack_num;
        void* ptr;
    } allocation_callstack_t;

    static const uint32_t MAX_ALLOC_CALLSTACKS = 8096;
    static allocation_callstack_t alloc_callstacks[MAX_ALLOC_CALLSTACKS];

    static void add_allocation_callstack(void* ptr)
    {
        for (uint32_t i = 0; i < MAX_ALLOC_CALLSTACKS; ++i)
        {
            if (alloc_callstacks[i].ptr == NULL)
            {
                backtrace_t bt = debug_get_backtrace(10);

                allocation_callstack_t ac = {
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

        if (must_be_present)
            error("Tried to remove non-existing allocation callstack");
    }
#endif

void memory_init()
{
    #ifdef ENABLE_MEMORY_TRACING
        memzero(alloc_callstacks, sizeof(allocation_callstack_t) * MAX_ALLOC_CALLSTACKS);
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
    void* p = realloc(cur, s);

    #ifdef ENABLE_MEMORY_TRACING
        if (cur)
            remove_allocation_callstack(cur, false);

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
    free(p);

    #ifdef ENABLE_MEMORY_TRACING
        if (p)
            remove_allocation_callstack(p, true);
    #endif
}

void memory_check_leaks()
{
    #ifdef ENABLE_MEMORY_TRACING
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
