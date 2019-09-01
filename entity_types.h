#pragma once
#include "renderer_types.h"
#include "physics_types.h"
#include "math_types.h"

fwd_handle(PhysicsResourceHandle);

struct Entity
{
    Vec3 position;
    Quat rotation;
    PhysicsResourceHandle physics_world;
    RenderWorldObjectHandle render_object;
    PhysicsWorldObjectHandle physics_object;
};