#pragma once
#include "physics_types.h"
#include "math_types.h"

fwd_handle(PhysicsResourceHandle);
fwd_handle(WorldEntityHandle);
fwd_struct(World);

struct Entity
{
    Vec3 pos;
    Quat rot;
    World* world;
    RenderWorldObjectHandle render_object;
    PhysicsWorldObjectHandle physics_object;
    PhysicsWorldRigidbodyHandle physics_rigidbody;
};

struct EntityRef
{
    World* world;
    WorldEntityHandle handle;
};