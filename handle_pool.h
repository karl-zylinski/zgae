#pragma once


typedef struct handle_pool_t {
    handle_t* handles_da;
    const char* types[128];
} handle_pool_t;

void handle_pool_init(handle_pool_t* hp);

// requires type_index < 128
void handle_pool_set_type(handle_pool_t* hp, uint8_t type_index, const char* name);
void handle_pool_destroy(handle_pool_t* hp);
handle_t handle_pool_reserve(handle_pool_t* hp, uint8_t type_index);
void handle_pool_return(handle_pool_t* hp, handle_t h);
bool handle_pool_is_valid(handle_pool_t* hp, handle_t h);