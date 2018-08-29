#pragma once

struct Mesh;
struct Vector3;

struct PhysicsShapeHandle {
    size_t h;
};

PhysicsShapeHandle physics_add_mesh(const Mesh& m, const Vector3& pos);
void physics_set_shape_position(PhysicsShapeHandle h, const Vector3& pos);
bool physics_intersect(PhysicsShapeHandle s1, PhysicsShapeHandle s2);
void physics_shutdown();