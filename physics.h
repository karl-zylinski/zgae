#pragma once

#include "physics_types.h"

fwd_handle(PhysicsResourceHandle);
fwd_struct(Vec3);
fwd_struct(Quat);
fwd_struct(Entity);

void physics_init();
void physics_shutdown();

PhysicsResourceHandle physics_create_collider(PhysicsResourceHandle mesh);
PhysicsResourceHandle physics_create_world(RenderResourceHandle render_handle);
PhysicsResourceHandle physics_load_resource(const char* filename);
PhysicsWorldObjectHandle physics_create_object(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot);
PhysicsWorldRigidbodyHandle physics_create_rigidbody(Entity* e, f32 mass);
void physics_add_force(PhysicsResourceHandle world, PhysicsWorldRigidbodyHandle rb, const Vec3& f);
void physics_set_position(PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos, const Quat& rot);
void physics_update_world(PhysicsResourceHandle world);