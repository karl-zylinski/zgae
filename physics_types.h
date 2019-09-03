#pragma once
#include "renderer_types.h"

fwd_handle(RenderResourceHandle);
fwd_handle(PhysicsObjectHandle);
fwd_handle(PhysicsRigidbodyHandle);

enum PhysicsResourceType
{
    PHYSICS_RESOURCE_TYPE_INVALID,
    PHYSICS_RESOURCE_TYPE_MESH,
    PHYSICS_RESOURCE_TYPE_COLLIDER,
    PHYSICS_RESOURCE_TYPE_WORLD,
    PHYSICS_RESOURCE_TYPE_NUM
};
