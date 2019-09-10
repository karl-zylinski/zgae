#pragma once
#include "renderer.h"

fwd_handle(PhysicsObjectHandle);
fwd_handle(PhysicsRigidbodyHandle);
fwd_handle(PhysicsResourceHandle);
fwd_struct(PhysicsWorld);

enum PhysicsResourceType
{
    PHYSICS_RESOURCE_TYPE_INVALID,
    PHYSICS_RESOURCE_TYPE_MESH,
    PHYSICS_RESOURCE_TYPE_NUM
};

struct PhysicsCollider
{
    PhysicsResourceHandle mesh;
};

struct PhysicsMaterial
{
    f32 friction;
    f32 elasticity;
};

void physics_init();
void physics_shutdown();

PhysicsCollider physics_create_collider(PhysicsResourceHandle mesh);
PhysicsWorld* physics_create_world(RenderResourceHandle render_handle);
void physics_destroy_world(PhysicsWorld* w);
PhysicsResourceHandle physics_load_resource(const char* filename);
PhysicsObjectHandle physics_create_object(PhysicsWorld* w, const PhysicsCollider& collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot, const PhysicsMaterial& = {});
PhysicsRigidbodyHandle physics_create_rigidbody(PhysicsWorld* w, PhysicsObjectHandle object_handle,  f32 mass, const Vec3& velocity);
void physics_set_velocity(PhysicsWorld* w, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& vel);
void physics_add_force(PhysicsWorld* w, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& f);
void physics_add_torque(PhysicsWorld* w, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& pivot, const Vec3& point, const Vec3& force);
void physics_set_position(PhysicsWorld* w, PhysicsObjectHandle obj, const Vec3& pos, const Quat& rot);
const Vec3& physics_get_position(PhysicsWorld* w, PhysicsObjectHandle obj);
const Quat& physics_get_rotation(PhysicsWorld* w, PhysicsObjectHandle obj);
void physics_update_world(PhysicsWorld* w);