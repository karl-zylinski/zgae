#pragma once
#include "world_types.h"

fwd_struct(Vec3);
fwd_struct(Quat);

World* world_create(RenderResourceHandle render_world, PhysicsResourceHandle physics_world);
void world_destroy(World* w);
void world_destroy_entity(World* w, WorldEntityHandle weh);
WorldEntityHandle world_create_entity(World* w, const Vec3& pos, const Quat& rot);
EntityInt* world_lookup_entity(World* w, WorldEntityHandle e);