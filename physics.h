#pragma once

#include "physics_types.h"

fwd_handle(PhysicsResourceHandle);
fwd_struct(PhysicsState);
fwd_struct(Vec3);
fwd_struct(Quat);

PhysicsState* physics_state_create(PhysicsPositionUpdateCallback position_update);
PhysicsResourceHandle physics_resource_load(PhysicsState* ps, const char* filename);
PhysicsResourceHandle physics_collider_create(PhysicsState* ps, PhysicsResourceHandle mesh);
PhysicsResourceHandle physics_world_create(PhysicsState* ps, RenderResourceHandle render_handle);
PhysicsWorldObjectHandle physics_world_add(PhysicsState* ps, PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot);
void physics_world_set_position(PhysicsState* ps, PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos, const Quat& rot);
void physics_world_move(PhysicsState* ps, PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos);
void physics_update_world(PhysicsState* ps, PhysicsResourceHandle world);
void physics_state_destroy(PhysicsState* ps);