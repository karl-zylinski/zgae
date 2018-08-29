#include "physics.h"
#include "memory.h"
#include "array.h"
#include "math.h"
#include "mesh.h"
#include "vertex.h"
#include "gjk.h"

//TODO: needs physicsworld??

struct PhysicsShape
{
    Vector3* vertices;
    Vector3* transformed_vertices;
    size_t num_vertices;
    Vector3 position;
    Quaternion rotation;
};

struct PhysicsShapeResource
{
    PhysicsShape ps;
    bool dirty_transform;
    bool used;
};

static PhysicsShapeResource* D_shapes = nullptr;

PhysicsShapeHandle physics_add_mesh(const Mesh& m, const Vector3& pos, const Quaternion& rot)
{
    PhysicsShape ps = {};
    size_t vsize = sizeof(Vector3) * m.num_vertices;
    ps.vertices = (Vector3*)zalloc(vsize);
    ps.transformed_vertices = (Vector3*)zalloc(vsize);

    for (size_t i = 0; i < m.num_vertices; ++i)
        ps.vertices[i] = m.vertices[i].position;

    memcpy(ps.transformed_vertices, ps.vertices, vsize);
    ps.num_vertices = m.num_vertices;
    ps.position = pos;
    ps.rotation = rot;

    for (size_t i = 0; i < ps.num_vertices; ++i)
    {
        ps.transformed_vertices[i] = quaternion_transform_vector3(ps.rotation, ps.transformed_vertices[i]);
        ps.transformed_vertices[i] += ps.position;
    }

    for (size_t i = 0; i < array_num(D_shapes); ++i)
    {
        if (D_shapes[i].used == false)
        {
            D_shapes[i].ps = ps;
            D_shapes[i].used = true;
            return {i};
        }
    }

    size_t idx = array_num(D_shapes);
    PhysicsShapeResource psr = {};
    psr.used = true;
    psr.ps = ps;
    array_push(D_shapes, psr);
    return {idx};
}


void check_dirty_transform(PhysicsShapeHandle h)
{
    if (!D_shapes[h.h].dirty_transform)
        return;

    PhysicsShape& ps = D_shapes[h.h].ps;
    memcpy(ps.transformed_vertices, ps.vertices, sizeof(Vector3) * ps.num_vertices);
    for (size_t i = 0; i < ps.num_vertices; ++i)
    {
        ps.transformed_vertices[i] = quaternion_transform_vector3(ps.rotation, ps.transformed_vertices[i]);
        ps.transformed_vertices[i] += ps.position;
    }

    D_shapes[h.h].dirty_transform = false;
}

static GJKShape gjk_shape_from_physics_shape(const PhysicsShape& ps)
{
    GJKShape s = {};
    s.vertices = ps.transformed_vertices;
    s.num_vertices = ps.num_vertices;
    return s;
}

bool physics_intersect(PhysicsShapeHandle h1, PhysicsShapeHandle h2)
{
    Assert(D_shapes[h1.h].used && D_shapes[h2.h].used, "Tried to intersect one ore more invalid physics shapes");
    check_dirty_transform(h1);
    check_dirty_transform(h2);

    GJKShape s1 = gjk_shape_from_physics_shape(D_shapes[h1.h].ps);
    GJKShape s2 = gjk_shape_from_physics_shape(D_shapes[h2.h].ps);
    return gjk_intersect(s1, s2);
}

void physics_set_shape_position(PhysicsShapeHandle h, const Vector3& pos)
{
    Assert(D_shapes[h.h].used, "Trying to set position on unused PhyscsShape");
    PhysicsShape& ps = D_shapes[h.h].ps;

    if (pos == ps.position)
        return;

    ps.position = pos;
    D_shapes[h.h].dirty_transform = true;
}

void physics_set_shape_rotation(PhysicsShapeHandle h, const Quaternion& rot)
{
    Assert(D_shapes[h.h].used, "Trying to set position on unused PhyscsShape");
    PhysicsShape& ps = D_shapes[h.h].ps;

    if (rot == ps.rotation)
        return;

    ps.rotation = rot;
    D_shapes[h.h].dirty_transform = true;
}

void physics_shutdown()
{
    for (size_t i = 0; i < array_num(D_shapes); ++i)
    {
        if (!D_shapes[i].used)
            continue;

        zfree(D_shapes[i].ps.vertices);
        zfree(D_shapes[i].ps.transformed_vertices);
    }

    array_destroy(D_shapes);
}
