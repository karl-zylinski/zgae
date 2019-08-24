#pragma once

fwd_struct(HandlePool);

HandlePool* handle_pool_create(u32 type_index, const char* type_name);
void handle_pool_set_subtype(HandlePool* hp, u32 subtype_index, const char* subtype_name);
void handle_pool_destroy(HandlePool* hp);
Handle handle_pool_borrow(HandlePool* hp, u32 subtype_index);
void handle_pool_return(HandlePool* hp, Handle h);
bool handle_pool_is_valid(const HandlePool* hp, Handle h);