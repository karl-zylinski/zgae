#pragma once

fwd_struct(IdxHashMap);

IdxHashMap* idx_hash_map_create();
void idx_hash_map_destroy(IdxHashMap* ihp);
void idx_hash_map_add(IdxHashMap* ihp, i64 hash, u32 idx);
u32 idx_hash_map_get(const IdxHashMap* ihp, i64 hash);
void idx_hash_map_remove(IdxHashMap* ihp, i64 hash);