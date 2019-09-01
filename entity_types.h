#pragma once
#include "physics_types.h"
#include "math_types.h"

fwd_handle(PhysicsResourceHandle);

struct Entity
{
    Vec3 pos;
    Quat rot;
    PhysicsResourceHandle physics_world;
    RenderWorldObjectHandle render_object;
    PhysicsWorldObjectHandle physics_object;
    PhysicsWorldRigidbodyHandle physics_rigidbody;
};