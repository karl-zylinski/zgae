#pragma once

static u32 marker_value = 0x25AEA22A;

struct DynamicArrayHeader
{
    u32 marker; // set to marker_value in da__grow_func and checked in da__header
    u32 num;
    u32 cap;
};

#define da_num(a) ((a) ? da__num(a) : 0)
#define da_cap(a) ((a) ? da__cap(a) : 0)

#define da_push(a, v) (\
    da__maybe_grow(a, da_num(a) + 1), \
    (a)[da__num(a)++] = (v) \
)

#define da_free(a) (da__destroy(a))
#define da_copy_data(a) ((a) ? da__copy_data(a, da__num(a), sizeof(*(a))) : NULL)
#define da_pop(a) ((a)[--da__num(a)])
#define da_last(a) ((a)[da__num(a) - 1])
#define da_ensure_min_cap(a, n) (da__maybe_grow(a, n))

#define da_insert(a, v, i) (\
    da__maybe_grow(a, da_num(a) + 1), \
    (a)[da__make_insert_room(a, i, sizeof(*(a)))] = (v) \
)

#define da__num(a) (da__header(a)->num)
#define da__cap(a) (da__header(a)->cap)
#define da__need_grow(a, n) ((a == NULL) || (n) > da_cap(a))
#define da__maybe_grow(a, n) (da__need_grow(a, n) ? da__grow(a, n) : 0)
#define da__grow(a, n) (*((void **)&(a)) = da__grow_func((a), (n), sizeof(*(a))))

DynamicArrayHeader* da__header(void* a);
void* da__grow_func(void* a, u32 min_num, u32 item_size);
void da__destroy(void* a);
void* da__copy_data(void* a, u32 num, u32 item_size);
u32 da__make_insert_room(void* a, u32 idx, u32 item_size);
