#include "gjk_epa.h"
#include "dynamic_array.h"
#include <math.h>
#include "log.h"
#include "renderer.h"

static Vec3 support(const GjkShape& s, const Vec3& d)
{
    float max_dot = dot(s.vertices[0], d);
    size_t max_dot_idx = 0;

    for (size_t i = 1; i < s.vertices_num; ++i)
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

static Vec3 support_diff(const GjkShape& s1, const GjkShape& s2, const Vec3& d)
{
    return support(s1, d) - support(s2, -d);
}

struct Simplex
{
    Vec3 vertices[4];
    unsigned char size;
};

enum GjkStatus
{
    GJK_STATUS_CONTINUE,
    GJK_STATUS_ABORT,
    GJK_STATUS_COLLIDING
};

static GjkStatus do_simplex(Simplex* s, Vec3* search_dir)
{
    switch(s->size)
    {
        case 2:
        {
            Vec3& B = s->vertices[0];
            Vec3& A = s->vertices[1];
            Vec3 AB = B - A;
            Vec3 AO = -A;

            if (dot(AB, AO) >= 0)
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

            if (almost_eql(ABC, ACD) || almost_eql(ABC, ADB) || almost_eql(ACD, ADB))
                return GJK_STATUS_ABORT;

            float d1 = dot(ABC, AO);
            if (d1 >= 0)
            {
                if (fabs(dot(AB, AO)) < SMALL_NUMBER || fabs(dot(AC, AO)) < SMALL_NUMBER)
                    return GJK_STATUS_ABORT; // Origin is on an edge, can result in loop.

                s->size = 3;
                s->vertices[0] = C;
                s->vertices[1] = B;
                s->vertices[2] = A;
                *search_dir = ABC;
                return GJK_STATUS_CONTINUE;
            }

            float d2 = dot(ACD, AO);
            if (d2 >= 0)
            {
                if (fabs(dot(AC, AO)) < SMALL_NUMBER || fabs(dot(AC, AO)) < SMALL_NUMBER)
                    return GJK_STATUS_ABORT; 

                s->size = 3;
                s->vertices[0] = D;
                s->vertices[1] = C;
                s->vertices[2] = A;
                *search_dir = ACD;
                return GJK_STATUS_CONTINUE;
            }

            float d3 = dot(ADB, AO);
            if (d3 >= 0)
            {
                if (fabs(dot(AD, AO)) < SMALL_NUMBER || fabs(dot(AB, AO)) < SMALL_NUMBER)
                    return GJK_STATUS_ABORT; 

                s->size = 3;
                s->vertices[0] = B;
                s->vertices[1] = D;
                s->vertices[2] = A;
                *search_dir = ADB;
                return GJK_STATUS_CONTINUE;
            }

            return GJK_STATUS_COLLIDING;
        } break;
    }

    return GJK_STATUS_CONTINUE;
}

struct GjkResult
{
    bool collision;
    Simplex simplex;
};

static GjkResult run_gjk(const GjkShape& s1, const GjkShape& s2)
{
    Simplex s = {};
    GjkStatus status = GJK_STATUS_CONTINUE;
    Vec3 first_point = support_diff(s1, s2, {0, 5, 0});
    s.vertices[s.size++] = first_point;
    Vec3 search_dir = -first_point;

    while (status != GJK_STATUS_ABORT)
    {
        Vec3 simplex_candidate = support_diff(s1, s2, search_dir);
        if (dot(simplex_candidate, search_dir) <= 0)
            return {.collision = false};

        s.vertices[s.size++] = simplex_candidate;
        status = do_simplex(&s, &search_dir);

        switch(status)
        {
            case GJK_STATUS_COLLIDING: {
                return {.collision = true, s};
            }
            case GJK_STATUS_ABORT: return {.collision = false};
            default: break;
        }
    }

    return {.collision = false};
}

bool gjk_intersect(const GjkShape& s1, const GjkShape& s2)
{
    return run_gjk(s1, s2).collision;
}

struct EpaFace
{
    float distance;
    Vec3 normal;
    Vec3 vertices[3];
};

static EpaFace* find_closest_face(EpaFace* faces)
{
    EpaFace* closest = faces;
    closest->distance = dot(closest->normal, closest->vertices[0]);
    check(closest->distance >= 0, "EpaFace in wrong direction.");

    for (unsigned i = 1; i < da_num(faces); ++i)
    {
        EpaFace* f = faces + i;
        float d = dot(f->normal, f->vertices[0]);
        check(d >= 0, "EpaFace in wrong direction.");
        if (d < closest->distance)
        {
            f->distance = d;
            closest = f;
        }
    }

    return closest;
}

static void add_face(EpaFace*& faces, const Vec3& A, const Vec3& B, const Vec3& C)
{
    Vec3 AB = B - A;
    Vec3 AC = C - A;
    Vec3 ABC = cross(AC, AB);

    EpaFace f = {};
    f.vertices[0] = B;
    f.vertices[1] = A;
    f.vertices[2] = C;

    if (almost_eql(ABC, vec3_zero))
    {
        da_push(faces, f);
        return;
    }

    f.normal = normalize(ABC);
    float normal_dir = dot(f.normal, A);

    if (normal_dir < 0)
    {
        f.vertices[0] = A;
        f.vertices[1] = B;
        f.vertices[2] = C;
        f.normal = -f.normal;
    }

    if (fabs(normal_dir) < SMALL_NUMBER)
    {
        /* Normal is zero or almsot zero, another case of zero-in-plane.
           Both this case and the ABC == vec3_zero case will cause algorithm
           to abort next iteration, since depth must be zero. */
        f.normal = vec3_zero;
    }

    da_push(faces, f);
}

static EpaFace* convert_simplex_to_epa_faces(Simplex& s)
{
    EpaFace* faces = nullptr;
    for (unsigned i = 0; i < s.size; ++i)
    {
        int j = ((i + 1) == s.size) ? 0 : i + 1;
        int k = ((j + 1) == s.size) ? 0 : j + 1;
        Vec3& A = s.vertices[i];
        Vec3& B = s.vertices[j];
        Vec3& C = s.vertices[k];
        add_face(faces, A, B, C);
    }
    return faces;
}

struct Edge
{
    Vec3 start;
    Vec3 end;
};

static bool remove_edge_if_present(Edge*& edges, const Edge& e)
{
    for (unsigned i = 0; i < da_num(edges); ++i)
    {
        if ((edges[i].start == e.end && edges[i].end == e.start) ||
            (edges[i].start == e.start && edges[i].end == e.end))
        {
            da_remove(edges, i);
            return true;
        }
    }

    return false;
}

static void extend_polytope(EpaFace*& faces, const Vec3& extend_to)
{
    Edge* edges = nullptr;

    for (unsigned i = 0; i < da_num(faces);)
    {
        EpaFace& f = faces[i];

        if (dot(f.normal, extend_to-f.vertices[0]) > 0)
        {
            Edge e1 = {f.vertices[0], f.vertices[1]};
            Edge e2 = {f.vertices[1], f.vertices[2]};
            Edge e3 = {f.vertices[2], f.vertices[0]};

            if (!remove_edge_if_present(edges, e1))
                da_push(edges, e1);

            if (!remove_edge_if_present(edges, e2))
                da_push(edges, e2);

            if (!remove_edge_if_present(edges, e3))
                da_push(edges, e3);

            da_remove(faces, i);
        }
        else
            ++i;
    }

    for (unsigned i = 0; i < da_num(edges); ++i)
        add_face(faces, extend_to, edges[i].start, edges[i].end);

    da_free(edges);
}

static GjkEpaSolution run_epa(const GjkShape& s1, const GjkShape& s2, Simplex* s)
{
    check(s->size == 4, "Trying to run EPA with non-tetrahedron simplex.");

    EpaFace* faces = convert_simplex_to_epa_faces(*s);

    while(true)
    {
        if (da_num(faces) == 0)
        {
            da_free(faces);
            return {.colliding = false};
        }

        EpaFace* f = find_closest_face(faces);
        Vec3 d = support_diff(s1, s2, f->normal);
        float depth = dot(d, f->normal);

        if (fabs(f->distance) < SMALL_NUMBER)
        {
            da_free(faces);
            // Origin is on face, so depth will be zero. Solution is zero vector.
            return {
                .colliding = true,
                .solution = {0, 0, 0}
            };
        }

        if (fabs(depth - f->distance) < SMALL_NUMBER)
        {
            Vec3 sol = -f->normal * depth;
            da_free(faces);
            return {
                .colliding = true,
                .solution = sol
            };
        }

        extend_polytope(faces, d);
    }

    da_free(faces);
    return {.colliding = false};
}

GjkEpaSolution gjk_epa_intersect_and_solve(const GjkShape& s1, const GjkShape& s2)
{
    GjkResult res = run_gjk(s1, s2);

    if (!res.collision)
        return {.colliding = false};

    return run_epa(s1, s2, &res.simplex);
}