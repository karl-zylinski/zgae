#pragma once
#include "math.h"
#include "entity_handle.h"
#include "collider_handle.h"
#include "rigid_body_handle.h"

struct Mesh;

struct Collision
{
    bool colliding;
    Vec3 solution;
};

ColliderHandle physics_create_mesh_collider(const Vec3* vertices, size_t num_vertices);
RigidBodyHandle physics_create_rigid_body(EntityHandle e);
void physics_set_collider_position(ColliderHandle h, const Vec3& pos);
void physics_set_collider_rotation(ColliderHandle h, const Quat& rot);
bool physics_intersects(ColliderHandle h1, ColliderHandle h2);
Collision physics_intersect_and_solve(ColliderHandle h1, ColliderHandle h2);
void physics_shutdown();
void physics_simulate();