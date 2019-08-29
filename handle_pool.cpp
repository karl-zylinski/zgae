#include "handle_pool.h"
#include "debug.h"
#include "dynamic_array.h"
#include "str.h"
#include "memory.h"

struct HandlePool
{
    u32 index;
    char* name;
    Handle* handles; // dynamic
    Handle* free_handles; // dynamic
    char* types[HANDLE_MAX_TYPE_INDEX];
};

HandlePool* handle_pool_create(u32 index, char* name)
{
    check(index < HANDLE_MAX_POOL_INDEX, "index must be less than %d", TO_STRING(HANDLE_MAX_POOL_INDEX));
    HandlePool* hp = mema_zero_t(HandlePool);
    hp->index = index;
    hp->name = str_copy(name);
    return hp;
}

void handle_pool_destroy(HandlePool* hp)
{
    check(da_num(hp->handles) == da_num(hp->free_handles), "Non-returned handles in pool with index %d", hp->index);

    for (u32 i = 0; i < HANDLE_MAX_TYPE_INDEX; ++i)
        memf(hp->types[i]);

    da_free(hp->handles);
    da_free(hp->free_handles);
    memf(hp->name);
    memf(hp);
}

void handle_pool_set_type(HandlePool* hp, u32 type_index, char* type_name)
{
    check(type_index < HANDLE_MAX_TYPE_INDEX, "type_index must be less than %d", TO_STRING(HANDLE_MAX_TYPE_INDEX));
    check(!hp->types[type_index], "Handle type already in use!");
    hp->types[type_index] = str_copy(type_name);
}

static Handle construct_handle(u32 i, u32 p, u32 t, u32 g)
{
    check(i < HANDLE_MAX_INDEX, "Trying to construct handle with index >= %d", TO_STRING(HANDLE_MAX_INDEX));
    check(p < HANDLE_MAX_POOL_INDEX, "Trying to construct handle with pool index >= %d", TO_STRING(HANDLE_MAX_POOL_INDEX));
    check(t < HANDLE_MAX_TYPE_INDEX, "Trying to construct handle with type index >= %d", TO_STRING(HANDLE_MAX_TYPE_INDEX));
    check(g < HANDLE_MAX_GENERATION, "Trying to construct handle with generation >= %d", TO_STRING(HANDLE_MAX_GENERATION));
    Handle h = i;
    h <<= 4;
    h |= p;
    h <<= 12;
    h |= t;
    h <<= 16;
    h |= g;
    return h;
}

Handle handle_pool_borrow(HandlePool* hp, u32 type_index)
{
    check(hp->types[type_index], "Trying to reserve handle with invalid type_index!");

    if (da_num(hp->free_handles) == 0)
    {
        u32 i = da_num(hp->handles); // access with handle_index(h)
        u32 p = hp->index; // access with handle_type(h)
        u32 t = type_index; // access with handle_subtype(h)
        u32 g = 0; // access with handle_generation(h)
        Handle h = construct_handle(i, p, t, g);
        da_push(hp->handles, h);
        return h;
    }

    Handle old_h = da_pop(hp->free_handles);
    Handle h = construct_handle(handle_index(old_h), hp->index, type_index, handle_generation(old_h));
    hp->handles[handle_index(h)] = h; // so everything is correct in handles
    return h;
}

void handle_pool_return(HandlePool* hp, Handle h)
{
    if (!handle_pool_is_valid(hp, h))
        return;

    Handle new_h = construct_handle(handle_index(h), hp->index, handle_type(h), (handle_generation(h) + 1) % HANDLE_MAX_GENERATION);
    hp->handles[handle_index(h)] = new_h;
    da_push(hp->free_handles, new_h);
}

bool handle_pool_is_valid(HandlePool* hp, Handle h)
{
    check(handle_index(h) < HANDLE_MAX_INDEX, "Handle out of bounds");
    Handle hp_h = hp->handles[handle_index(h)];
    return handle_generation(h) == handle_generation(hp_h);
}