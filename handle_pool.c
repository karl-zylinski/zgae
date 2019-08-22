#include "handle_pool.h"
#include "debug.h"
#include "array.h"
#include "str.h"
#include "memory.h"

typedef struct HandlePool {
    Handle* arr_handles;
    char* types[HANDLE_MAX_TYPE_INDEX];
} HandlePool;

static Handle construct_handle(sizet handle_index, u8 type_index, bool used)
{
    check(handle_index < HANDLE_MAX_INDEX, "Trying to construct handle with index >= %d", HANDLE_MAX_INDEX);
    check(type_index < HANDLE_MAX_TYPE_INDEX, "Trying to construct handle with type >= %d", HANDLE_MAX_TYPE_INDEX);
    Handle h = handle_index; // 00000000 iiiiiiii iiiiiiii iiiiiiii
    h = h << 8; // iiiiiiii iiiiiiii iiiiiiii 00000000
    h = h | (type_index << 1); // iiiiiiii iiiiiiii iiiiiiii ttttttt0
    h = h | (used); // iiiiiiii iiiiiiii iiiiiiii tttttttu
    return h;
}

HandlePool* handle_pool_create()
{
    return mema_zero(sizeof(HandlePool));
}

void handle_pool_destroy(HandlePool* hp)
{
    for (sizet i = 0; i < array_num(hp->arr_handles); ++i)
        check(!handle_used(hp->arr_handles[i]), "Found handles in use while destroying handle pool");

    for (u32 i = 0; i < HANDLE_MAX_TYPE_INDEX; ++i)
        memf(hp->types[i]);

    array_destroy(hp->arr_handles);
    memf(hp);
}

void handle_pool_set_type(HandlePool* hp, u8 type_index, const char* name)
{
    check(type_index < HANDLE_MAX_TYPE_INDEX, "Trying to set type for type index");
    check(!hp->types[type_index], "Handle type already in use!");
    hp->types[type_index] = str_copy(name);
}

Handle handle_pool_reserve(HandlePool* hp, u8 type_index)
{
    check(hp->types[type_index], "Trying to reserve handle with invalid type index!");

    for (sizet i = 0; i < array_num(hp->arr_handles); ++i)
    {
        if (!handle_used(hp->arr_handles[i]))
        {
            Handle h = construct_handle(i, type_index, true);
            hp->arr_handles[i] = h;
            return h;
        }
    }

    Handle h = construct_handle(array_num(hp->arr_handles), type_index, true);
    array_add(hp->arr_handles, h);
    return h;
}

void handle_pool_return(HandlePool* hp, Handle h)
{
    check(handle_pool_is_valid(hp, h), "Trying to return invalid handle!");
    hp->arr_handles[handle_index(h)] ^= 0x1; // First bit is used flag, XOR it out.
}

bool handle_pool_is_valid(const HandlePool* hp, Handle h)
{
    return    handle_index(h) < HANDLE_MAX_INDEX
           && handle_used(h)
           && hp->types[handle_type(h)]
           && handle_index(h) < array_num(hp->arr_handles)
           && hp->arr_handles[handle_index(h)] == h;
}
