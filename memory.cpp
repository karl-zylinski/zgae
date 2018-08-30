#include "memory.h"

#if defined(ENABLE_MEMORY_TRACING)
    static const unsigned G_max_captured_callstacks = 4096;
    static CapturedCallstack G_captured_callstacks[G_max_captured_callstacks];

    void add_captured_callstack(const CapturedCallstack& cc)
    {
        for (unsigned i = 0; i < G_max_captured_callstacks; ++i)
        {
            if (!G_captured_callstacks[i].used)
            {
                G_captured_callstacks[i] = cc;
                return;
            }
        }

        Error("Out of callstacks. Increase max_captured_callstacks in memory.h.");
    }

    void remove_captured_callstack(void* p)
    {
        for (unsigned i = 0; i < G_max_captured_callstacks; ++i) {
            if (G_captured_callstacks[i].ptr == p)
            {
                callstack_destroy(G_captured_callstacks + i);
                return;
            }
        }

        Error("Failed to find callstack in remove_captured_callstack.");
    }

    void ensure_captured_callstacks_unused()
    {
        if (G_captured_callstacks == nullptr)
            return;

        for (unsigned i = 0; i < G_max_captured_callstacks; ++i)
        {
            if (!G_captured_callstacks[i].used)
                continue;

            callstack_print("Memory leak stack trace", G_captured_callstacks[i]);
        }
    }
#endif

void memory_init()
{
    #if defined(ENABLE_MEMORY_TRACING)
        memzero(G_captured_callstacks, G_max_captured_callstacks * sizeof(CapturedCallstack));
    #endif
}

void memory_shutdown()
{
    #if defined(ENABLE_MEMORY_TRACING)
        ensure_captured_callstacks_unused();
    #endif
}