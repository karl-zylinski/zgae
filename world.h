#pragma once
#include "world_types.h"

fwd_struct(Vec3);
fwd_struct(Quat);

World* world_create(RenderResourceHandle render_world, PhysicsResourceHandle physics_world);
WorldEntityHandle world_create_entity(World* w, const Vec3& pos, const Quat& rot);
Entity* world_lookup_entity(World* w, WorldEntityHandle e);