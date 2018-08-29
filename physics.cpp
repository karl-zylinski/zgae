#include "physics.h"
#include "memory.h"
#include "array.h"
#include "math.h"
#include "mesh.h"
#include "vertex.h"

//TODO: needs physicsworld??

struct PhysicsShape
{
    Vector3* vertices;
    Vector3* transformed_vertices;
    size_t num_vertices;
    Vector3 position;
};

struct PhysicsShapeResource
{
    PhysicsShape ps;
    bool used;
};

static PhysicsShapeResource* D_shapes = nullptr;

PhysicsShapeHandle physics_add_mesh(const Mesh& m, const Vector3& pos)
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

    for (size_t i = 0; i < ps.num_vertices; ++i)
        ps.transformed_vertices[i] += ps.position;

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

struct GJKShape
{
    Vector3* vertices;
    size_t num_vertices;
};

static Vector3 gjk_support(const GJKShape& s, const Vector3& d)
{
    float max_dot = vector3_dot(s.vertices[0], d);
    size_t max_dot_idx = 0;

    for (size_t i = 1; i < s.num_vertices; ++i)
    {
        float dot = vector3_dot(s.vertices[i], d);
        if (dot > max_dot)
        {
            max_dot = dot;
            max_dot_idx = i;
        }
    }

    return s.vertices[max_dot_idx];
}

static Vector3 gjk_support_diff(const GJKShape& s1, const GJKShape& s2, const Vector3& d)
{
    return gjk_support(s1, d) - gjk_support(s2, -d);
}

static GJKShape gjk_shape_from_physics_shape(const PhysicsShape& ps)
{
    GJKShape s = {};
    s.vertices = ps.transformed_vertices;
    s.num_vertices = ps.num_vertices;
    return s;
}

struct Simplex
{
    Vector3 vertices[4];
    unsigned char size;
};

static void gjk_do_simplex3(Simplex* s, Vector3* search_dir)
{
    Vector3& C = s->vertices[0];
    Vector3& B = s->vertices[1];
    Vector3& A = s->vertices[2];
    Vector3 AB = B - A;
    Vector3 AC = C - A;
    Vector3 ABC = vector3_cross(AB, AC);
    Vector3 AO = -A;

    if (vector3_dot(vector3_cross(ABC, AC), AO) > 0)
    {
        if (vector3_dot(AC, AO) > 0)
            *search_dir = vector3_cross(AC, vector3_cross(AO, AC));
        else
        {
            if (vector3_dot(AB, AO) > 0)
                *search_dir = vector3_cross(AB, vector3_cross(AO, AB));
            else
                *search_dir = AO;
        }
    }
    else
    {
        if (vector3_dot(vector3_cross(AB, ABC), AO) > 0)
        {
            if (vector3_dot(AB, AO) > 0)
                *search_dir = vector3_cross(AB, vector3_cross(AO, AB));
            else
                *search_dir = AO;
        }
        else
        {
            if (vector3_dot(ABC, AO) > 0)
                *search_dir = ABC;
            else
                *search_dir = -ABC;
        }
    }
}

static bool gjk_do_simplex(Simplex* s, Vector3* search_dir)
{
    switch(s->size)
    {
        case 2:
        {
            Vector3& B = s->vertices[0];
            Vector3& A = s->vertices[1];
            Vector3 AB = B - A;
            Vector3 AO = -A;

            if (vector3_dot(AB, AO) > 0)
                *search_dir = vector3_cross(AB, vector3_cross(AO, AB));
            else
                *search_dir = AO;
        } break;
        case 3:
        {
            gjk_do_simplex3(s, search_dir);
        } break;
        case 4:
        {
            Vector3 D = s->vertices[0];
            Vector3 C = s->vertices[1];
            Vector3 B = s->vertices[2];
            Vector3 A = s->vertices[3];
            Vector3 AB = B - A;
            Vector3 AC = C - A;
            Vector3 AD = D - A;
            Vector3 AO = -A;

            Vector3 ABC = vector3_cross(AB, AC);
            Vector3 ACD = vector3_cross(AC, AD);
            Vector3 ADB = vector3_cross(AD, AB);

            if (vector3_dot(ABC, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = C;
                s->vertices[1] = B;
                s->vertices[2] = A;
                *search_dir = ABC;
                gjk_do_simplex3(s, search_dir);
                return false;
            }

            if (vector3_dot(ACD, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = D;
                s->vertices[1] = C;
                s->vertices[2] = A;
                *search_dir = ACD;
                gjk_do_simplex3(s, search_dir);
                return false;
            }

            if (vector3_dot(ADB, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = B;
                s->vertices[1] = D;
                s->vertices[2] = A;
                *search_dir = ADB;
                gjk_do_simplex3(s, search_dir);
                return false;
            }
            
            return true;
        } break;
    }

    return false;
}

static bool gjk_intersect(const PhysicsShape& ps1, const PhysicsShape& ps2)
{
    GJKShape s1 = gjk_shape_from_physics_shape(ps1);
    GJKShape s2 = gjk_shape_from_physics_shape(ps2);
    Simplex s = {};

    Vector3 first_point = gjk_support_diff(s1, s2, {1, 0, 0});
    s.vertices[s.size++] = first_point;
    Vector3 search_dir = -first_point;

    while (true)
    {
        Vector3 simplex_candidate = gjk_support_diff(s1, s2, search_dir);
        float d = vector3_dot(simplex_candidate, search_dir);
        if (d < 0)
            return false;

        s.vertices[s.size++] = simplex_candidate;

        if (gjk_do_simplex(&s, &search_dir))
            return true;
    }

    return false;
}


bool physics_intersect(PhysicsShapeHandle s1, PhysicsShapeHandle s2)
{
    Assert(D_shapes[s1.h].used && D_shapes[s2.h].used, "Tried to intersect one ore more invalid physics shapes");

    const PhysicsShape& ps1 = D_shapes[s1.h].ps;
    const PhysicsShape& ps2 = D_shapes[s2.h].ps;

    return gjk_intersect(ps1, ps2);
}

void physics_set_shape_position(PhysicsShapeHandle h, const Vector3& pos)
{
    Assert(D_shapes[h.h].used, "Trying to set position on unused PhyscsShape");
    PhysicsShape& ps = D_shapes[h.h].ps;
    ps.position = pos;
    memcpy(ps.transformed_vertices, ps.vertices, sizeof(Vector3) * ps.num_vertices);

    for (size_t i = 0; i < ps.num_vertices; ++i)
        ps.transformed_vertices[i] += ps.position;
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
