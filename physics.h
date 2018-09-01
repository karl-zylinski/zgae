#pragma once
#include "math.h"

struct Mesh;

struct ColliderHandle {
    size_t h;
};

struct Collision
{
    bool colliding;
    Vec3 solution;
};

ColliderHandle physics_create_mesh_collider(const Vec3* vertices, size_t num_vertices);
void physics_set_collider_position(ColliderHandle h, const Vec3& pos);
void physics_set_collider_rotation(ColliderHandle h, const Quat& rot);
bool physics_intersect(ColliderHandle h1, ColliderHandle h2);
Collision physics_intersect_and_solve(ColliderHandle h1, ColliderHandle h2);
void physics_shutdown();