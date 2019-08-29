#include "handle_hash_map.h"
#include "memory.h"
#include "dynamic_array.h"
#include "handle.h"

struct HandleHashMap
{
    hash64* hashes; // dynamic
    Handle* handles; // dynamic
};

HandleHashMap* handle_hash_map_create()
{
    return mema_zero_t(HandleHashMap);
}

void handle_hash_map_destroy(HandleHashMap* hhp)
{
    da_free(hhp->hashes);
    da_free(hhp->handles);
    memf(hhp);
}

static u32 find_mapping_insertion_idx(hash64* hashes, hash64 hash)
{
    u32 n = da_num(hashes);

    if (n == 0)
        return 0;

    for (u32 i = 0; i < n; ++i)
    {
        if (hashes[i] > hash)
            return i;
    }

    return n;
}

static u32 mapping_get_idx(hash64* hashes, hash64 hash)
{
    u32 n = da_num(hashes);

    if (n == 0)
        return -1;

    u32 mz = n;
    u32 first = 0;
    u32 last = mz - 1;
    u32 middle = (first + last) / 2;

    while (first <= last)
    {
        if (hashes[middle] < hash)
            first = middle + 1;
        else if (hashes[middle] == hash)
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
    u32 idx = find_mapping_insertion_idx(hhp->hashes, hash);

    if (idx < da_num(hhp->hashes) && hhp->handles[idx] == HANDLE_INVALID) // reuse dead slot
    {
        hhp->handles[idx] = handle;
        hhp->hashes[idx] = hash;
        return;
    }

    da_insert(hhp->hashes, hash, idx);
    da_insert(hhp->handles, handle, idx);
}

Handle handle_hash_map_get(HandleHashMap* hhp, hash64 h)
{
    u32 idx = mapping_get_idx(hhp->hashes, h);

    if (idx == (u32)-1)
        return HANDLE_INVALID;

    return hhp->handles[idx];
}

void handle_hash_map_remove(HandleHashMap* hhp, hash64 h)
{
    u32 idx = mapping_get_idx(hhp->hashes, h);

    if (idx == (u32)-1)
        return;

    hhp->hashes[idx] = 0;
    hhp->handles[idx] = HANDLE_INVALID; // checked in handle_hash_map_add, use to denote dead slot
}