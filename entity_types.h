#pragma once
#include "math_types.h"
#include "renderer_types.h"
#include "physics_types.h"

fwd_struct(World);
fwd_handle(WorldEntityHandle);

struct EntityInt
{
    Vec3 pos;
    Quat rot;
    World* world;
    RenderWorldObjectHandle render_object;
    PhysicsWorldObjectHandle physics_object;
    PhysicsWorldRigidbodyHandle physics_rigidbody;
    WorldEntityHandle handle;
};
