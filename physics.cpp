#include "physics.h"
#include "memory.h"
#include "array.h"
#include "math.h"
#include "mesh.h"
#include "vertex.h"
#include "gjk.h"

//TODO: needs physicsworld??

struct Collider
{
    Vec3* vertices;
    Vec3* transformed_vertices;
    size_t num_vertices;
    Vec3 position;
    Quat rotation;
};

struct ColliderResource
{
    Collider c;
    bool dirty_transform;
    bool used;
};

static ColliderResource* D_colliders = nullptr;

void check_dirty_transform(ColliderHandle h)
{
    if (!D_colliders[h.h].dirty_transform)
        return;

    Collider& c = D_colliders[h.h].c;
    memcpy(c.transformed_vertices, c.vertices, sizeof(Vec3) * c.num_vertices);
    for (size_t i = 0; i < c.num_vertices; ++i)
    {
        c.transformed_vertices[i] = quat_transform_vec3(c.rotation, c.transformed_vertices[i]);
        c.transformed_vertices[i] += c.position;
    }

    D_colliders[h.h].dirty_transform = false;
}

static GJKShape gjk_shape_from_collider(const Collider& c)
{
    GJKShape s = {};
    s.vertices = c.transformed_vertices;
    s.num_vertices = c.num_vertices;
    return s;
}

bool physics_intersect(ColliderHandle h1, ColliderHandle h2)
{
    Assert(D_colliders[h1.h].used && D_colliders[h2.h].used, "Tried to intersect one ore more invalid physics shapes");
    check_dirty_transform(h1);
    check_dirty_transform(h2);
    GJKShape s1 = gjk_shape_from_collider(D_colliders[h1.h].c);
    GJKShape s2 = gjk_shape_from_collider(D_colliders[h2.h].c);
    return gjk_intersect(s1, s2);
}

ColliderHandle physics_create_mesh_collider(const Mesh& m)
{
    Collider c = {};
    size_t vsize = sizeof(Vec3) * m.num_vertices;
    c.vertices = (Vec3*)zalloc(vsize);
    c.transformed_vertices = (Vec3*)zalloc(vsize);

    for (size_t i = 0; i < m.num_vertices; ++i)
        c.vertices[i] = m.vertices[i].position;

    memcpy(c.transformed_vertices, c.vertices, vsize);
    c.num_vertices = m.num_vertices;
    c.rotation = quat_identity();

    for (size_t i = 0; i < c.num_vertices; ++i)
    {
        c.transformed_vertices[i] = quat_transform_vec3(c.rotation, c.transformed_vertices[i]);
        c.transformed_vertices[i] += c.position;
    }

    for (size_t i = 0; i < array_num(D_colliders); ++i)
    {
        if (D_colliders[i].used == false)
        {
            D_colliders[i].c = c;
            D_colliders[i].used = true;
            return {i};
        }
    }

    size_t idx = array_num(D_colliders);
    ColliderResource cr = {};
    cr.used = true;
    cr.c = c;
    array_push(D_colliders, cr);
    return {idx};
}

void physics_set_collider_position(ColliderHandle h, const Vec3& pos)
{
    Assert(D_colliders[h.h].used, "Trying to set position on unused PhyscsShape");
    Collider& c = D_colliders[h.h].c;

    if (pos == c.position)
        return;

    c.position = pos;
    D_colliders[h.h].dirty_transform = true;
}

void physics_set_collider_rotation(ColliderHandle h, const Quat& rot)
{
    Assert(D_colliders[h.h].used, "Trying to set position on unused PhyscsShape");
    Collider& c = D_colliders[h.h].c;

    if (rot == c.rotation)
        return;

    c.rotation = rot;
    D_colliders[h.h].dirty_transform = true;
}

void physics_shutdown()
{
    for (size_t i = 0; i < array_num(D_colliders); ++i)
    {
        if (!D_colliders[i].used)
            continue;

        zfree(D_colliders[i].c.vertices);
        zfree(D_colliders[i].c.transformed_vertices);
    }

    array_destroy(D_colliders);
}
