#pragma once
#include "math_types.h"
#include "renderer_types.h"
#include "physics_types.h"

fwd_struct(World);
fwd_handle(WorldEntityHandle);
fwd_handle(PhysicsResourceHandle);
fwd_handle(RenderResourceHandle);

struct EntityInt
{
    Vec3 pos;
    Quat rot;
    World* world;
    RenderWorldObjectHandle render_object;
    PhysicsWorldObjectHandle physics_object;
    PhysicsWorldRigidbodyHandle physics_rigidbody;
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
    EntityInt* deref();

    World* world;
    WorldEntityHandle handle;
};