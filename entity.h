#pragma once
#include "math.h"

fwd_struct(World);
fwd_struct(PhysicsCollider);
fwd_struct(PhysicsMaterial);

struct EntityInt
{
    u32 idx;
    Vec3 pos;
    Quat rot;
    World* world;
    u32 render_object_idx;
    u32 physics_object_idx;
    u32 physics_rigidbody_idx;
};

struct Entity
{
    void move(const Vec3& d);
    void rotate(const Vec3& axis, f32 rad);
    void rotate(const Quat& q);
    void create_rigidbody(f32 mass, const Vec3& velocity);
    void set_render_mesh(u32 mesh_idx);
    void set_physics_collider(const PhysicsCollider& collider, const PhysicsMaterial& pm);
    const Vec3& get_position() const;
    void set_position(const Vec3& pos);
    void set_rotation(const Quat& rot);
    void set_velocity(const Vec3& vel);
    void add_force(const Vec3& f);
    void add_torque(const Vec3& pivot, const Vec3& point, const Vec3& force);
    void update_from_rigidbody();
    u32 get_render_object_idx() const;
    u32 get_physics_object_idx() const;

    World* world;
    u32 idx;
};

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);
