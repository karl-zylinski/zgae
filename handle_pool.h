#pragma once

fwd_struct(HandlePool);

HandlePool* handle_pool_create(u32 type_index, char* type_name);
void handle_pool_set_type(mut HandlePool* hp, u32 subtype_index, char* subtype_name);
void handle_pool_destroy(mut HandlePool* hp);
Handle handle_pool_borrow(mut HandlePool* hp, u32 subtype_index);
void handle_pool_return(mut HandlePool* hp, Handle h);
bool handle_pool_is_valid(HandlePool* hp, Handle h);