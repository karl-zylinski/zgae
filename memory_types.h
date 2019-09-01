#pragma once

enum AllocatorType
{
    ALLOCATOR_TYPE_HEAP,
    ALLOCATOR_TYPE_TEMP
};

#ifdef ENABLE_MEMORY_TRACING
    struct AllocationCallstack 
    {
        char** callstack;
        u32 callstack_num;
        void* ptr;
    };
#endif

struct Allocator
{
    AllocatorType type;
    void* data;

    #ifdef ENABLE_MEMORY_TRACING
        AllocationCallstack* tracing_callstacks;
        u32 tracing_callstacks_num;
    #endif
};
