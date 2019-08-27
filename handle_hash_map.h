#pragma once

fwd_struct(HandleHashMap);
HandleHashMap* handle_hash_map_create();
void handle_hash_map_destroy(mut HandleHashMap* hhp);
void handle_hash_map_add(mut HandleHashMap* hhp, hash64 hash, Handle handle);
Handle handle_hash_map_get(HandleHashMap* hhp, hash64 h);
void handle_hash_map_remove(mut HandleHashMap* hhp, hash64 h);