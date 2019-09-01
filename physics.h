#pragma once

#include "physics_types.h"

fwd_handle(PhysicsResourceHandle);
fwd_struct(Vec3);
fwd_struct(Quat);

void physics_init();
PhysicsResourceHandle physics_resource_load(const char* filename);
PhysicsResourceHandle physics_collider_create(PhysicsResourceHandle mesh);
PhysicsResourceHandle physics_world_create(RenderResourceHandle render_handle);
PhysicsWorldObjectHandle physics_world_add(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot);
void physics_world_set_position(PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos, const Quat& rot);
void physics_world_move(PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos);
void physics_update_world(PhysicsResourceHandle world);
void physics_shutdown();