#pragma once

fwd_struct(HandlePool);

enum HandlePoolType
{
    HANDLE_POOL_TYPE_INVALID,
    HANDLE_POOL_TYPE_RENDER_RESOURCE,
    HANDLE_POOL_TYPE_PHYSICS_RESOURCE,
    HANDLE_POOL_TYPE_WORLD_ENTITY,
    HANDLE_POOL_TYPE_PHYSICS_OBJECT,
    HANDLE_POOL_TYPE_RIGIDBODY
};

static const char* HANDLE_POOL_TYPE_NAMES[] = {
    "invalid", "render_resource", "physics_resource", "world_entity", "physics_object", "rigidbody"
};

HandlePool* handle_pool_create(HandlePoolType type);
void handle_pool_set_type(HandlePool* hp, u32 subtype_index, const char* subtype_name);
void handle_pool_destroy(HandlePool* hp);
Handle handle_pool_borrow(HandlePool* hp, u32 subtype_index = 0);
void handle_pool_return(HandlePool* hp, Handle h);
bool handle_pool_is_valid(const HandlePool* hp, Handle h);
