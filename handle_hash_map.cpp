#include "handle_hash_map.h"
#include "memory.h"
#include "array.h"

struct HandleHashMap
{
    Array<hash64> hashes;
    Array<Handle> handles;
};

HandleHashMap* handle_hash_map_create()
{
    return mema_zero_t(HandleHashMap);
}

void handle_hash_map_destroy(mut HandleHashMap* hhp)
{
    array_destroy(&hhp->hashes);
    array_destroy(&hhp->handles);
    memf(hhp);
}

static size_t find_mapping_insertion_idx(Array<hash64>& hashes, hash64 hash)
{
    if (hashes.num == 0)
        return 0;

    for (size_t i = 0; i < hashes.num; ++i)
    {
        if (hashes[i] > hash)
            return i;
    }

    return hashes.num;
}

static size_t mapping_get_idx(Array<hash64>& hashes, hash64 hash)
{
    if (hashes.num == 0)
        return -1;

    size_t mz = hashes.num;
    size_t first = 0;
    size_t last = mz - 1;
    size_t middle = (first + last) / 2;

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

void handle_hash_map_add(mut HandleHashMap* hhp, hash64 hash, Handle handle)
{
    u32 idx = find_mapping_insertion_idx(hhp->hashes, hash);

    if (idx < hhp->hashes.num && hhp->handles[idx] == HANDLE_INVALID) // reuse dead slot
    {
        hhp->handles[idx] = handle;
        hhp->hashes[idx] = hash;
        return;
    }

    array_insert(&hhp->hashes, hash, idx);
    array_insert(&hhp->handles, handle, idx);
}

Handle handle_hash_map_get(HandleHashMap* hhp, hash64 h)
{
    size_t idx = mapping_get_idx(hhp->hashes, h);

    if (idx == (size_t)-1)
        return HANDLE_INVALID;

    return hhp->handles[idx];
}

void handle_hash_map_remove(mut HandleHashMap* hhp, hash64 h)
{
    size_t idx = mapping_get_idx(hhp->hashes, h);

    if (idx == (size_t)-1)
        return;

    hhp->hashes[idx] = 0;
    hhp->handles[idx] = HANDLE_INVALID; // checked in handle_hash_map_add, use to denote dead slot
}