#include <assert.h>
#include "handle_pool.h"
#include "debug.h"
#include "memory.h"
#include "array.h"
#include <execinfo.h>

static Backtrace get_backtrace(u32 backtrace_size)
{
    if (backtrace_size > 32)
        backtrace_size = 32;

    static void* backtraces[32];
    u32 bt_size = backtrace(backtraces, backtrace_size);
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
        u32 p = 5;
        u32 t = 12;
        u32 g = 513;

        u64 h = i;
        h <<= 4;
        h |= p;
        h <<= 12;
        h |= t;
        h <<= 16;
        h |= g;

        assert(handle_index(h) == i);
        assert(handle_pool(h) == p);
        assert(handle_type(h) == t);
        assert(handle_generation(h) == g);
    }

    {
        HandlePool* hp = handle_pool_create(0, "hp");
        handle_pool_set_type(hp, 0, "some type");
        handle_pool_set_type(hp, 1, "other type");
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
        assert(handle_pool(h1) == 0);
        assert(handle_pool(h2) == 0);
        assert(handle_pool(h3) == 0);
        assert(handle_type(h1) == 0);
        assert(handle_type(h2) == 0);
        assert(handle_type(h3) == 1);
    }

    {
        Array<u32> a = {};
        array_push(&a, 5u);
        array_push(&a, 2u);

        assert(a[0] == 5u);
        assert(a[1] == 2u);

        array_insert(&a, 10u, 1);

        assert(a[0] == 5u);
        assert(a[1] == 10u);
        assert(a[2] == 2u);

        u32 p = array_pop(&a);
        assert(p == 2u);
        assert(a.num == 2);
        assert(a[0] == 5u);
        assert(a[1] == 10u);

        array_fill_and_set(&a, 137u, 10);
        assert(a[10] == 137u);
    }
}