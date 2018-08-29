#include "gjk.h"
#include "math.h"

static Vector3 support(const GJKShape& s, const Vector3& d)
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

static Vector3 support_diff(const GJKShape& s1, const GJKShape& s2, const Vector3& d)
{
    return support(s1, d) - support(s2, -d);
}

struct Simplex
{
    Vector3 vertices[4];
    unsigned char size;
};

static bool do_simplex(Simplex* s, Vector3* search_dir)
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
                return false;
            }

            if (vector3_dot(ACD, AO) > 0)
            {
                s->size = 3;
                s->vertices[0] = D;
                s->vertices[1] = C;
                s->vertices[2] = A;
                *search_dir = ACD;
                return false;
            }

            if (vector3_dot(ADB, AO) > 0)
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

bool gjk_intersect(const GJKShape& s1, const GJKShape& s2)
{
    Simplex s = {};

    Vector3 first_point = support_diff(s1, s2, {1, 0, 0});
    s.vertices[s.size++] = first_point;
    Vector3 search_dir = -first_point;

    while (true)
    {
        Vector3 simplex_candidate = support_diff(s1, s2, search_dir);
        float d = vector3_dot(simplex_candidate, search_dir);
        if (d < 0)
            return false;

        s.vertices[s.size++] = simplex_candidate;

        if (do_simplex(&s, &search_dir))
            return true;
    }

    return false;
}
