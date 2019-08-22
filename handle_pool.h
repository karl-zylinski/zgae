#pragma once

fwd_struct(HandlePool);

HandlePool* handle_pool_create();
void handle_pool_set_type(HandlePool* hp, u8 type_index, const char* name);
void handle_pool_destroy(HandlePool* hp);
Handle handle_pool_reserve(HandlePool* hp, u8 type_index);
void handle_pool_return(HandlePool* hp, Handle h);
bool handle_pool_is_valid(const HandlePool* hp, Handle h);