#include "dynamic_array.h"
#include "memory.h"
#include "debug.h"
#include <string.h>

DynamicArrayHeader* da__header(void* a)
{
    DynamicArrayHeader* dah = ((DynamicArrayHeader*)(a)) - 1;
    check(dah->marker == marker_value, "Passed parameter to da_* func probably ins't a dynamic array");
    return dah;
}

void* da__grow_func(void* a, u32 min_num, u32 item_size)
{
    check(min_num > 0, "Trying to grow dynamic array with zero num increment");
    u32 double_cap = da_cap(a) * 2;
    u32 new_cap = double_cap > min_num ? double_cap : min_num;
    DynamicArrayHeader* ah = (DynamicArrayHeader*)memra(a ? da__header(a) : NULL, sizeof(DynamicArrayHeader) + new_cap * item_size);

    if (!a)
    {
        ah->num = 0;
        ah->marker = marker_value;
    }

    ah->cap = new_cap;
    return ah + 1;
}

void da__destroy(void* a)
{
    if (!a)
        return;

    memf(da__header(a));
}

void* da__copy_data(void* a, u32 num, u32 item_size)
{
    return mema_copy(a, num * item_size);
}

u32 da__make_insert_room(void* a, u32 idx, u32 item_size)
{
    u32 n = da__num(a);
    ++da__num(a);

    if (idx == n)
        return idx;

    memmove(((i8*)(a)) + (idx + 1) * item_size, ((i8*)(a)) + idx * item_size, (n - idx) * item_size);
    return idx;
}

void da__remove(void* a, u32 idx, u32 item_size)
{
    u32 n = da__num(a);
    --da__num(a);

    if (idx == n)
        return;

    memmove(((i8*)(a)) + idx * item_size, ((i8*)(a)) + (idx + 1) * item_size, (n - idx) * item_size);
}