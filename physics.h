#pragma once
#include "renderer.h"

fwd_handle(PhysicsObjectHandle);
fwd_handle(PhysicsRigidbodyHandle);
fwd_handle(PhysicsResourceHandle);
fwd_struct(Entity);

enum PhysicsResourceType
{
    PHYSICS_RESOURCE_TYPE_INVALID,
    PHYSICS_RESOURCE_TYPE_MESH,
    PHYSICS_RESOURCE_TYPE_COLLIDER,
    PHYSICS_RESOURCE_TYPE_WORLD,
    PHYSICS_RESOURCE_TYPE_NUM
};

void physics_init();
void physics_shutdown();

PhysicsResourceHandle physics_create_collider(PhysicsResourceHandle mesh);
PhysicsResourceHandle physics_create_world(RenderResourceHandle render_handle);
PhysicsResourceHandle physics_load_resource(const char* filename);
PhysicsObjectHandle physics_create_object(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot);
PhysicsRigidbodyHandle physics_create_rigidbody(Entity* e, f32 mass);
void physics_add_force(PhysicsResourceHandle world, PhysicsRigidbodyHandle rb, const Vec3& f);
void physics_set_position(PhysicsResourceHandle world, PhysicsObjectHandle obj, const Vec3& pos, const Quat& rot);
void physics_update_world(PhysicsResourceHandle world);