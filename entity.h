#pragma once
#include "math.h"
#include "physics.h"

fwd_struct(World);
fwd_handle(WorldEntityHandle);

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
    void rotate(const Vec3& axis, float rad);
    void create_rigidbody(f32 mass);
    void set_render_mesh(RenderResourceHandle mesh);
    void set_physics_collider(PhysicsResourceHandle collider);
    const Vec3& get_position() const;
    void set_position(const Vec3& pos);
    void set_rotation(const Quat& rot);
    void add_force(const Vec3& f);
    RenderWorldObjectHandle get_render_object() const;
    PhysicsObjectHandle get_physics_object() const;

    World* world;
    WorldEntityHandle handle;
};

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);
