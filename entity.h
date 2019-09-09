#pragma once
#include "math.h"
#include "physics.h"

fwd_handle(WorldEntityHandle);
fwd_struct(PhysicsMaterial);
fwd_struct(World);

struct EntityInt
{
    Vec3 pos;
    Quat rot;
    World* world;
    RenderWorldObjectHandle render_object;
    PhysicsObjectHandle physics_object;
    PhysicsRigidbodyHandle physics_rigidbody;
    WorldEntityHandle handle;
};

struct Entity
{
    void move(const Vec3& d);
    void rotate(const Vec3& axis, f32 rad);
    void rotate(const Quat& q);
    void create_rigidbody(f32 mass, const Vec3& velocity);
    void set_render_mesh(RenderResourceHandle mesh);
    void set_physics_collider(PhysicsResourceHandle collider, const PhysicsMaterial& pm);
    const Vec3& get_position() const;
    void set_position(const Vec3& pos);
    void set_rotation(const Quat& rot);
    void set_velocity(const Vec3& vel);
    void add_linear_impulse(const Vec3& force, f32 time);
    void add_torque(const Vec3& pivot, const Vec3& point, const Vec3& force);
    RenderWorldObjectHandle get_render_object() const;
    PhysicsObjectHandle get_physics_object() const;

    World* world;
    WorldEntityHandle handle;
};

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);
