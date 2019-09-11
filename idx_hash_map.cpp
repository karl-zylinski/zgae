#include "idx_hash_map.h"
#include "memory.h"
#include "dynamic_array.h"

struct IdxHashMap
{
    i64* hashes; // dynamic
    u32* idxs; // dynamic
};

IdxHashMap* idx_hash_map_create()
{
    let ihp = mema_zero_t(IdxHashMap);
    da_push(ihp->hashes, 0); // dummy
    da_push(ihp->idxs, 0); // dummy
    return ihp;
}

void idx_hash_map_destroy(IdxHashMap* ihp)
{
    da_free(ihp->hashes);
    da_free(ihp->idxs);
    memf(ihp);
}

static u32 find_mapping_insertion_slot(i64* hashes, i64 hash)
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

static u32 mapping_get_slot(i64* hashes, i64 hash)
{
    u32 n = da_num(hashes);

    if (n == 0)
        return 0;

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

    return 0;
}

void idx_hash_map_add(IdxHashMap* ihp, i64 hash, u32 idx)
{
    u32 slot = find_mapping_insertion_slot(ihp->hashes, hash);

    if (slot < da_num(ihp->hashes) && !ihp->idxs[slot]) // reuse dead slot
    {
        ihp->hashes[slot] = hash;
        ihp->idxs[slot] = idx;
        return;
    }

    da_insert(ihp->hashes, hash, slot);
    da_insert(ihp->idxs, idx, slot);
}

u32 idx_hash_map_get(const IdxHashMap* ihp, i64 hash)
{
    return ihp->idxs[mapping_get_slot(ihp->hashes, hash)];
}

void idx_hash_map_remove(IdxHashMap* ihp, i64 hash)
{
    u32 slot = mapping_get_slot(ihp->hashes, hash);
    ihp->hashes[slot] = 0;
    ihp->idxs[slot] = 0;
}