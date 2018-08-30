#include "gjk.h"
#include "math.h"

static Vec3 support(const GJKShape& s, const Vec3& d)
{
    float max_dot = dot(s.vertices[0], d);
    size_t max_dot_idx = 0;

    for (size_t i = 1; i < s.num_vertices; ++i)
    {
        float cur_dot = dot(s.vertices[i], d);
        if (cur_dot > max_dot)
        {
            max_dot = cur_dot;
            max_dot_idx = i;
        }
    }

    return s.vertices[max_dot_idx];
}

static Vec3 support_diff(const GJKShape& s1, const GJKShape& s2, const Vec3& d)
{
    return support(s1, d) - support(s2, -d);
}

struct Simplex
{
    Vec3 vertices[4];
    unsigned char size;
};

static bool do_simplex(Simplex* s, Vec3* search_dir)
{
    switch(s->size)
    {
        case 2:
        {
            Vec3& B = s->vertices[0];
            Vec3& A = s->vertices[1];
            Vec3 AB = B - A;
            Vec3 AO = -A;

            if (dot(AB, AO) > 0)
                *search_dir = cross(AB, cross(AO, AB));
            else
                *search_dir = AO;
        } break;
        case 3:
        {
            Vec3& C = s->vertices[0];
            Vec3& B = s->vertices[1];
            Vec3& A = s->vertices[2];
            Vec3 AB = B - A;
            Vec3 AC = C - A;
            Vec3 ABC = cross(AB, AC);
            Vec3 AO = -A;

            if (dot(cross(ABC, AC), AO) > 0)
            {
                if (dot(AC, AO) > 0)
                    *search_dir = cross(AC, cross(AO, AC));
                else
                {
                    if (dot(AB, AO) > 0)
                        *search_dir = cross(AB, cross(AO, AB));
                    else
                        *search_dir = AO;
                }
            }
            else
            {
                if (dot(cross(AB, ABC), AO) > 0)
                {
                    if (dot(AB, AO) > 0)
                        *search_dir = cross(AB, cross(AO, AB));
                    else
                        *search_dir = AO;
                }
                else
                {
                    if (dot(ABC, AO) > 0)
                        *search_dir = ABC;
                    else
                        *search_dir = -ABC;
                }
            }
        } break;
        case 4:
        {
            Vec3 D = s->vertices[0];
            Vec3 C = s->vertices[1];
            Vec3 B = s->vertices[2];
            Vec3 A = s->vertices[3];
            Vec3 AB = B - A;
            Vec3 AC = C - A;
            Vec3 AD = D - A;
            Vec3 AO = -A;

            Vec3 ABC = cross(AB, AC);
            Vec3 ACD = cross(AC, AD);
            Vec3 ADB = cross(AD, AB);

            if (dot(ABC, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = C;
                s->vertices[1] = B;
                s->vertices[2] = A;
                *search_dir = ABC;
                return false;
            }

            if (dot(ACD, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = D;
                s->vertices[1] = C;
                s->vertices[2] = A;
                *search_dir = ACD;
                return false;
            }

            if (dot(ADB, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = B;
                s->vertices[1] = D;
                s->vertices[2] = A;
                *search_dir = ADB;
                return false;
            }
            
            return true;
        } break;
    }

    return false;
}

struct GJKResult
{
    bool collision;
    Simplex simplex;
};

static GJKResult run_gjk(const GJKShape& s1, const GJKShape& s2)
{
    Simplex s = {};

    Vec3 first_point = support_diff(s1, s2, {1, 0, 0});
    s.vertices[s.size++] = first_point;
    Vec3 search_dir = -first_point;

    while (true)
    {
        Vec3 simplex_candidate = support_diff(s1, s2, search_dir);
        float d = dot(simplex_candidate, search_dir);
        if (d < 0)
            return {false};

        s.vertices[s.size++] = simplex_candidate;

        if (do_simplex(&s, &search_dir))
            return {true, s};
    }

    return {false};
}

bool gjk_intersect(const GJKShape& s1, const GJKShape& s2)
{
    return run_gjk(s1, s2).collision;
}

/*struct EPAFace {
    float distance;
    Vec3 normal;
    unsigned simplex_index;
}

static void simplex_insert(Simplex* s, const Vec3& v, unsigned idx)
{
    if (idx == s->size)
    {
        s[idx] = v;
        ++s->size;
        return;
    }

    memmove(s->vertices + idx + 1, s->vertices + idx, sizeof(Vec3) * (s->size - idx));
    s[idx] = v;
    ++s->size;
}

static EPAFace find_closest_face(const Simplex& s)
{
    EPAFace closest = {};
    closest.distance = FLT_MAX;

    for (unsigned i = 0; i < s.size; ++i)
    {
        int j = ((i + 1) == s.size) ? 0 : i + 1;
        int k = ((k + 1) == s.size) ? 0 : k + 1;
        Vec3 A = s.vertices[i];
        Vec3 B = s.vertices[j];
        Vec3 C = s.vertices[k];

        Vec3 AB = B - A;
        Vec3 AC = C - A;
        Vec3 OA = -A;
        Vec3 ABC = cross(AB, cross(OA, AB))
    }
}

struct Face {
    Vector3 vertices[3];
    Vector3 normal;
}

static Vec3 run_epa(const GJKShape& s1, const GJKShape& s2, Simplex* s)
{
    Assert(s->size == 4, "Trying to run EPA with non-tetrahedron simplex.")
    const max_faces = 32;
    Face faces[max_faces];
    unsigned num_faces = 4;
    faces[0].vertices[0] = s->vertices[0];
    faces[0].vertices[1] = s->vertices[0];
    faces[0].vertices[2] = s->vertices[0];

    while(true)
    {
        Vec3 f = find_closest_face(s);
        Vec3 d = support(s1, s2, f.normal);
        float depth = dot(d, f.normal);

        if (almost_equal(depth, f.distance))
            return f.normal * depth;

        simplex_insert(s, d, f.simplex_index);
    }
}

Vec3 gjk_epa_intersect_and_solve(const GJKShape& s1, const GJKShape& s2)
{
    GJKResult res = run_gjk(s1, s2);

    if (!res.collsion)
        return {0, 0, 0};

    return run_epa(s1, s2, &res.simplex);
}*/