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

struct PhysicsMaterial
{
    f32 friction;
    f32 elasticity;
};

void physics_init();
void physics_shutdown();

PhysicsResourceHandle physics_create_collider(PhysicsResourceHandle mesh);
PhysicsResourceHandle physics_create_world(RenderResourceHandle render_handle);
PhysicsResourceHandle physics_load_resource(const char* filename);
PhysicsObjectHandle physics_create_object(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot, const PhysicsMaterial& = {});
PhysicsRigidbodyHandle physics_create_rigidbody(PhysicsResourceHandle world, PhysicsObjectHandle object_handle,  f32 mass, const Vec3& velocity);
void physics_set_velocity(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& vel);
void physics_add_linear_impulse(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& force, f32 time);
void physics_add_torque(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& pivot, const Vec3& point, const Vec3& force);
void physics_set_position(PhysicsResourceHandle world, PhysicsObjectHandle obj, const Vec3& pos, const Quat& rot);
const Vec3& physics_get_position(PhysicsResourceHandle world, PhysicsObjectHandle obj);
const Quat& physics_get_rotation(PhysicsResourceHandle world, PhysicsObjectHandle obj);
void physics_update_world(PhysicsResourceHandle world);