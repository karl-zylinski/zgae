#pragma once

fwd_struct(HandleHashMap);

HandleHashMap* handle_hash_map_create();
void handle_hash_map_destroy(HandleHashMap* hhp);
void handle_hash_map_add(HandleHashMap* hhp, hash64 hash, Handle handle);
Handle handle_hash_map_get(const HandleHashMap* hhp, hash64 h);
void handle_hash_map_remove(HandleHashMap* hhp, hash64 h);