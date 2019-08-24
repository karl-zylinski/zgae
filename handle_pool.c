#include "handle_pool.h"
#include "debug.h"
#include "array.h"
#include "str.h"
#include "memory.h"

typedef struct HandlePool {
    u32 type_index;
    Handle* arr_handles;
    Handle* arr_free_handles;
    char* subtypes[HANDLE_MAX_TYPE_INDEX];
} HandlePool;

HandlePool* handle_pool_create(u32 type_index, const char* type_name)
{
    check(type_index < HANDLE_MAX_TYPE_INDEX, "type_index must be less than %d", TO_STRING(HANDLE_MAX_TYPE_INDEX));
    (void)type_name;
    HandlePool* hp = mema_zero(sizeof(HandlePool));
    hp->type_index = type_index;
    return hp;
}

void handle_pool_destroy(HandlePool* hp)
{
    check(array_num(hp->arr_handles) == array_num(hp->arr_free_handles), "Non-returned handles in pool with type index %d", hp->type_index);

    for (u32 i = 0; i < HANDLE_MAX_TYPE_INDEX; ++i)
        memf(hp->subtypes[i]);

    array_destroy(hp->arr_handles);
    array_destroy(hp->arr_free_handles);
    memf(hp);
}

void handle_pool_set_subtype(HandlePool* hp, u32 subtype_index, const char* subtype_name)
{
    check(subtype_index < HANDLE_MAX_SUBTYPE_INDEX, "subtype_index must be less than %d", TO_STRING(HANDLE_MAX_SUBTYPE_INDEX));
    check(!hp->subtypes[subtype_index], "Handle type already in use!");
    hp->subtypes[subtype_index] = str_copy(subtype_name);
}

static Handle construct_handle(u32 i, u32 t, u32 s, u32 g)
{
    check(i < HANDLE_MAX_INDEX, "Trying to construct handle with index >= %d", TO_STRING(HANDLE_MAX_INDEX));
    check(t < HANDLE_MAX_TYPE_INDEX, "Trying to construct handle with type index >= %d", TO_STRING(HANDLE_MAX_TYPE_INDEX));
    check(s < HANDLE_MAX_SUBTYPE_INDEX, "Trying to construct handle with subtype index >= %d", TO_STRING(HANDLE_MAX_SUBTYPE_INDEX));
    check(g < HANDLE_MAX_GENERATION, "Trying to construct handle with generation >= %d", TO_STRING(HANDLE_MAX_GENERATION));
    Handle h = i;
    h <<= 4;
    h |= t;
    h <<= 12;
    h |= s;
    h <<= 16;
    h |= g;
    return h;
}

Handle handle_pool_borrow(HandlePool* hp, u32 subtype_index)
{
    check(hp->subtypes[subtype_index], "Trying to reserve handle with invalid subtype_index!");

    if (array_num(hp->arr_free_handles) == 0)
    {
        u32 i = array_num(hp->arr_handles); // access with handle_index(h)
        u32 t = hp->type_index; // access with handle_type(h)
        u32 s = subtype_index; // access with handle_subtype(h)
        u32 g = 0; // access with handle_generation(h)
        Handle h = construct_handle(i, t, s, g);
        array_add(hp->arr_handles, h);
        return h;
    }

    Handle old_h = array_last(hp->arr_free_handles);
    array_pop(hp->arr_free_handles);
    Handle h = construct_handle(handle_index(old_h), hp->type_index, subtype_index, handle_generation(old_h));
    hp->arr_handles[handle_index(h)] = h; // so everything is correct in arr_handles
    return h;
}

void handle_pool_return(HandlePool* hp, Handle h)
{
    if (!handle_pool_is_valid(hp, h))
        return;

    Handle new_h = construct_handle(handle_index(h), hp->type_index, handle_subtype(h), (handle_generation(h) + 1) % HANDLE_MAX_GENERATION);
    hp->arr_handles[handle_index(h)] = new_h;
    array_add(hp->arr_free_handles, new_h);
}

bool handle_pool_is_valid(const HandlePool* hp, Handle h)
{
    check(handle_index(h) < HANDLE_MAX_INDEX, "Handle out of bounds");
    Handle hp_h = hp->arr_handles[handle_index(h)];
    return handle_generation(h) == handle_generation(hp_h);
}