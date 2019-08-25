#include "handle_hash_map.h"
#include "memory.h"
#include "array.h"

typedef struct HandleHashMap
{
    hash64* arr_hashes;
    Handle* arr_handles;
} HandleHashMap;

HandleHashMap* handle_hash_map_create()
{
    HandleHashMap* hhp = mema_type(HandleHashMap);
    return hhp;
}

void handle_hash_map_destroy(HandleHashMap* hhp)
{
    array_destroy(hhp->arr_hashes);
    array_destroy(hhp->arr_handles);
    memf(hhp);
}

static size_t find_mapping_insertion_idx(hash64* arr_hashes, hash64 name_hash)
{
    if (array_num(arr_hashes) == 0)
        return 0;

    for (size_t i = 0; i < array_num(arr_hashes); ++i)
    {
        if (arr_hashes[i] > name_hash)
            return i;
    }

    return array_num(arr_hashes);
}

static size_t mapping_get_idx(hash64* arr_hashes, hash64 name_hash)
{
    if (array_num(arr_hashes) == 0)
        return -1;

    size_t mz = array_num(arr_hashes);
    size_t first = 0;
    size_t last = mz - 1;
    size_t middle = (first + last) / 2;

    while (first <= last)
    {
        if (arr_hashes[middle] < name_hash)
            first = middle + 1;
        else if (arr_hashes[middle] == name_hash)
            return middle;
        else
        {
            if (middle == 0)
                break;

            last = middle - 1;
        }

        middle = (first + last) / 2;
    }

    return -1;
}

void handle_hash_map_add(HandleHashMap* hhp, hash64 hash, Handle handle)
{
    u32 idx = find_mapping_insertion_idx(hhp->arr_hashes, hash);

    if (idx < array_num(hhp->arr_hashes) && hhp->arr_handles[idx] == HANDLE_INVALID) // reuse dead slot
    {
        hhp->arr_handles[idx] = handle;
        hhp->arr_hashes[idx] = hash;
        return;
    }

    array_insert(hhp->arr_hashes, hash, idx);
    array_insert(hhp->arr_handles, handle, idx);
}

Handle handle_hash_map_get(HandleHashMap* hhp, hash64 h)
{
    size_t idx = mapping_get_idx(hhp->arr_hashes, h);

    if (idx == (size_t)-1)
        return HANDLE_INVALID;

    return hhp->arr_handles[idx];
}

void handle_hash_map_remove(HandleHashMap* hhp, hash64 h)
{
    size_t idx = mapping_get_idx(hhp->arr_hashes, h);

    if (idx == (size_t)-1)
        return;

    hhp->arr_hashes[idx] = 0;
    hhp->arr_handles[idx] = HANDLE_INVALID; // checked in handle_hash_map_add, use to denote dead slot
}