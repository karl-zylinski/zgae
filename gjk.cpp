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
    Vec3 vertices[32];
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
