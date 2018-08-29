#pragma once

struct Mesh;
struct Quat;
struct Vec3;

struct ColliderHandle {
    size_t h;
};

ColliderHandle physics_create_mesh_collider(const Mesh& m);
void physics_set_collider_position(ColliderHandle h, const Vec3& pos);
void physics_set_collider_rotation(ColliderHandle h, const Quat& rot);
bool physics_intersect(ColliderHandle s1, ColliderHandle s2);
void physics_shutdown();