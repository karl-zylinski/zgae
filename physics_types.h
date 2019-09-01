#pragma once
#include "renderer_types.h"

fwd_handle(RenderResourceHandle);

typedef u32 PhysicsWorldObjectHandle;
typedef u32 PhysicsWorldRigidbodyHandle;

enum PhysicsResourceType
{
    PHYSICS_RESOURCE_TYPE_INVALID,
    PHYSICS_RESOURCE_TYPE_MESH,
    PHYSICS_RESOURCE_TYPE_COLLIDER,
    PHYSICS_RESOURCE_TYPE_WORLD,
    PHYSICS_RESOURCE_TYPE_NUM
};
