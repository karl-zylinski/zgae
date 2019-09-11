#include <assert.h>
#include "log.h"
#include "memory.h"
#include "dynamic_array.h"
#include <execinfo.h>
#include "handle.h"
#include "math.h"

static Backtrace get_backtrace(u32 backtrace_size)
{
    if (backtrace_size > 32)
        backtrace_size = 32;

    static void* backtraces[32];
    u32 bt_size = backtrace(backtraces, backtrace_size);
    char** bt_symbols = backtrace_symbols(backtraces, bt_size);
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
        u32* a = NULL;
        da_push(a, 5u);
        da_push(a, 2u);

        assert(a[0] == 5u);
        assert(a[1] == 2u);

        da_insert(a, 10u, 1);

        assert(a[0] == 5u);
        assert(a[1] == 10u);
        assert(a[2] == 2u);

        u32 p = da_pop(a);
        assert(p == 2u);
        assert(da_num(a) == 2);
        assert(a[0] == 5u);
        assert(a[1] == 10u);
        da_free(a);
    }

    {
        Vec3 v1 = {1, 0, 0};
        Vec3 v2 = {0, 1, 0};

        f32 d = dot(v1, v2);
        assert(d == 0.0f);

        Vec3 v3 = cross(v1, v2);
        assert(v3.x == 0 && v3.y == 0 && v3.z == 1);

        Vec3 v4 = cross(v1, v1);
        assert(v4.x == 0 && v4.y == 0 && v4.z == 0);
    }

    info("All tests completed without errors");
}