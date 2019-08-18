#include "handle_pool.h"
#include "debug.h"
#include "array.h"
#include "str.h"
#include <string.h>

static handle_t construct_handle(size_t handle_index, uint8_t type_index, bool used)
{
    check(handle_index < HANDLE_MAX_INDEX, "Trying to construct handle with index >= %d", HANDLE_MAX_INDEX);
    check(type_index < HANDLE_MAX_TYPE_INDEX, "Trying to construct handle with type >= %d", HANDLE_MAX_TYPE_INDEX);
    handle_t h = handle_index; // 00000000 iiiiiiii iiiiiiii iiiiiiii
    h = h << 8; // iiiiiiii iiiiiiii iiiiiiii 00000000
    h = h | (type_index << 1); // iiiiiiii iiiiiiii iiiiiiii ttttttt0
    h = h | (used); // iiiiiiii iiiiiiii iiiiiiii tttttttu
    return h;
}

void handle_pool_init(handle_pool_t* hp)
{
    memset(hp, 0, sizeof(handle_pool_t));
}

void handle_pool_set_type(handle_pool_t* hp, uint8_t type_index, const char* name)
{
    check(type_index < HANDLE_MAX_TYPE_INDEX, "Trying to set type for type index");
    check(!hp->types[type_index], "Handle type already in use!");
    hp->types[type_index] = str_copy(name);
}

void handle_pool_destroy(handle_pool_t* hp)
{
    for (size_t i = 0; i < array_num(hp->arr_handles); ++i)
        check(!handle_used(hp->arr_handles[i]), "Found handles in use while destroying handle pool");

    array_destroy(hp->arr_handles);
}

handle_t handle_pool_reserve(handle_pool_t* hp, uint8_t type_index)
{
    check(hp->types[type_index], "Trying to reserve handle with invalid index!");

    for (size_t i = 0; i < array_num(hp->arr_handles); ++i)
    {
        if (!handle_used(hp->arr_handles[i]))
        {
            handle_t h = construct_handle(i, type_index, true);
            hp->arr_handles[i] = h;
            return h;
        }
    }

    handle_t h = construct_handle(array_num(hp->arr_handles), type_index, true);
    array_add(hp->arr_handles, h);
    return h;
}

void handle_pool_return(handle_pool_t* hp, handle_t h)
{
    check(handle_pool_is_valid(hp, h), "Trying to return invalid handle!");
    hp->arr_handles[handle_index(h)] ^= 0x1; // First bit is used flag, XOR it out.
}

bool handle_pool_is_valid(handle_pool_t* hp, handle_t h)
{
    return    handle_index(h) < HANDLE_MAX_INDEX
           && handle_used(h)
           && hp->types[handle_type(h)]
           && handle_index(h) < array_num(hp->arr_handles)
           && hp->arr_handles[handle_index(h)] == h;
}
