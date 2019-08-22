#pragma once

fwd_struct(handle_pool_t);

handle_pool_t* handle_pool_create();
void handle_pool_set_type(handle_pool_t* hp, uint8_t type_index, const char* name);
void handle_pool_destroy(handle_pool_t* hp);
handle_t handle_pool_reserve(handle_pool_t* hp, uint8_t type_index);
void handle_pool_return(handle_pool_t* hp, handle_t h);
bool handle_pool_is_valid(const handle_pool_t* hp, handle_t h);