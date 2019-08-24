#include <assert.h>
#include "handle_pool.h"
#include "debug.h"
#include "memory.h"
#include <execinfo.h>

static Backtrace get_backtrace(u32 backtrace_size)
{
    if (backtrace_size > 32)
        backtrace_size = 32;

    static void* backtraces[32];
    size_t bt_size = backtrace(backtraces, backtrace_size);
    const char** bt_symbols = (const char**)backtrace_symbols(backtraces, bt_size);
    Backtrace bt = {
        .function_calls = bt_symbols,
        .function_calls_num = bt_size
    };
    return bt;
}

int main()
{
    debug_init(get_backtrace);
    memory_init();

    {
        u32 i = 777;
        u32 t = 5;
        u32 s = 12;
        u32 g = 513;

        u64 h = i;
        h <<= 4;
        h |= t;
        h <<= 12;
        h |= s;
        h <<= 16;
        h |= g;

        assert(handle_index(h) == i);
        assert(handle_type(h) == t);
        assert(handle_subtype(h) == s);
        assert(handle_generation(h) == g);
    }

    {
        HandlePool* hp = handle_pool_create(0, "hp");
        handle_pool_set_subtype(hp, 0, "some type");
        handle_pool_set_subtype(hp, 1, "other type");
        Handle h1 = handle_pool_borrow(hp, 0);
        Handle h2 = handle_pool_borrow(hp, 0);
        handle_pool_return(hp, h2);
        Handle h3 = handle_pool_borrow(hp, 1);
        assert(handle_generation(h1) == 0);
        assert(handle_generation(h2) == 0);
        assert(handle_generation(h3) == 1);
        assert(handle_index(h1) == 0);
        assert(handle_index(h2) == 1);
        assert(handle_index(h3) == 1);
        assert(handle_type(h1) == 0);
        assert(handle_type(h2) == 0);
        assert(handle_type(h3) == 0);
        assert(handle_subtype(h1) == 0);
        assert(handle_subtype(h2) == 0);
        assert(handle_subtype(h3) == 1);
    }
}