#pragma once

fwd_struct(Vec3);
fwd_struct(Quat);
fwd_struct(PhysicsWorld);

struct PhysicsCollider
{
    u32 mesh_idx;
};

struct PhysicsMaterial
{
    f32 friction;
    f32 elasticity;
};

void physics_init();
void physics_shutdown();

PhysicsCollider physics_create_collider(u32 mesh_idx);
PhysicsWorld* physics_create_world();
void physics_destroy_world(PhysicsWorld* w);
u32 physics_load_mesh(const char* filename);
void physics_destroy_mesh(u32 mesh_idx);
u32 physics_create_object(PhysicsWorld* w, const PhysicsCollider& collider, u32 render_object_idx, const Vec3& pos, const Quat& rot, const PhysicsMaterial& = {});
u32 physics_create_rigidbody(PhysicsWorld* w, u32 object_idx, f32 mass, const Vec3& velocity);
void physics_set_velocity(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& vel);
void physics_add_force(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& f);
void physics_add_torque(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& pivot, const Vec3& point, const Vec3& force);
void physics_set_position(PhysicsWorld* w, u32 object_idx, const Vec3& pos, const Quat& rot);
const Vec3& physics_get_position(PhysicsWorld* w, u32 object_idx);
const Quat& physics_get_rotation(PhysicsWorld* w, u32 object_idx);
void physics_update_world(PhysicsWorld* w);