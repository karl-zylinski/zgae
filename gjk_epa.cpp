#include "gjk_epa.h"
#include "math.h"
#include "color.h"
#include "debug.h"
#include "array.h"
#include <math.h>

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
            
            Vec3 f1[] = {A, B, C};
            Vec3 f2[] = {A, C, D};
            Vec3 f3[] = {A, D, B};
            Vec3 f4[] = {B, D, C};

            debug_force_pump_window();
            debug_clear_frame();
            debug_draw_mesh(f1, 3, debug_get_random_color(1));
            debug_draw_mesh(f2, 3, debug_get_random_color(2));
            debug_draw_mesh(f3, 3, debug_get_random_color(3));
            debug_draw_mesh(f4, 3, debug_get_random_color(4));
            debug_present();

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
        if (dot(simplex_candidate, search_dir) < 0)
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

static const unsigned _num_random_colors = 128;
static Color* _random_colors = nullptr;

struct EPAFace {
    float distance;
    Vec3 normal;
    Vec3 vertices[3];
    Color dbg_color;
};

static EPAFace& find_closest_face(EPAFace* faces)
{
    EPAFace* closest = faces;
    closest->distance = (float)fabs(dot(closest->normal, closest->vertices[0]));

    for (unsigned i = 1; i < array_size(faces); ++i)
    {
        EPAFace* f = faces + i;
        float d = (float)fabs(dot(f->normal, f->vertices[0]));
        if (d < closest->distance)
        {
            f->distance = d;
            closest = f;
        }
    }

    return *closest;
}

static void add_face(EPAFace*& faces, const Vec3& A, const Vec3& B, const Vec3& C)
{
    Vec3 AB = B - A;
    Vec3 AC = C - A;
    Vec3 ABC = cross(AC, AB);
    EPAFace f = {};
    f.normal = vec3_normalize(ABC);

    if (dot(f.normal, A) < 0)
    {
        f.vertices[0] = A;
        f.vertices[1] = B;
        f.vertices[2] = C;
        f.normal = -f.normal;
    }
    else
    {
        f.vertices[0] = B;
        f.vertices[1] = A;
        f.vertices[2] = C;
    }

    array_push(faces, f);
}

static EPAFace* convert_simplex_to_epa_faces(Simplex& s)
{
    EPAFace* faces = nullptr;
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
    for (unsigned i = 0; i < array_size(edges); ++i)
    {
        if ((almost_equal(edges[i].start, e.start) && almost_equal(edges[i].end, e.end))
            || (almost_equal(edges[i].start, e.end) && almost_equal(edges[i].end, e.start)))
        {
            array_remove(edges, i);
            return true;
        }
    }

    return false;
}

static void extend_polytope(EPAFace*& faces, const Vec3& extend_to)
{
    Edge* edges = nullptr;

    for (unsigned i = 0; i < array_size(faces);)
    {
        EPAFace& f = faces[i];

        if (dot(f.normal, extend_to-f.vertices[0]) > 0)
        {
            Edge e1 = {f.vertices[0], f.vertices[1]};
            Edge e2 = {f.vertices[1], f.vertices[2]};
            Edge e3 = {f.vertices[2], f.vertices[0]};

            if (!remove_edge_if_present(edges, e1))
                array_push(edges, e1);

            if (!remove_edge_if_present(edges, e2))
                array_push(edges, e2);

            if (!remove_edge_if_present(edges, e3))
                array_push(edges, e3);

            array_remove(faces, i);
        }
        else
            ++i;
    }

    for (unsigned i = 0; i < array_size(edges); ++i)
        add_face(faces, extend_to, edges[i].start, edges[i].end);

    array_destroy(edges);
}

static GJKEPASolution run_epa(const GJKShape& s1, const GJKShape& s2, Simplex* s)
{
    Assert(s->size == 4, "Trying to run EPA with non-tetrahedron simplex.");

    EPAFace* faces = convert_simplex_to_epa_faces(*s);

    while(true)
    {
        /*debug_force_pump_window();
        debug_clear_frame();
        for (unsigned i = 0; i < array_size(faces); ++i)
        {
            static Color nocolor = {};
            if (faces[i].dbg_color == nocolor)
                faces[i].dbg_color = debug_get_random_color(i);
            debug_draw_mesh(faces[i].vertices, 3, faces[i].dbg_color);
        }
        debug_present();*/
        if (array_size(faces) == 0)
        {
            array_destroy(faces);
            return {false}; // hack for corner-cases, shouldn't use EPA for such shallow penetrations anyways
        }

        EPAFace& f = find_closest_face(faces);
        Vec3 d = support_diff(s1, s2, f.normal);
        float depth = dot(d, f.normal);

        if (depth == 0)
        {
            array_destroy(faces);
            return {false}; // hack for corner-cases, shouldn't use EPA for such shallow penetrations anyways
        }

        if (fabs(depth - f.distance) < 0.0001f)
        {
            /*for (unsigned i = 0; i < array_size(faces); ++i)
                debug_draw_mesh(faces[i].vertices, 3, debug_get_random_color(i));*/
            Vec3 sol = -f.normal * depth;
            array_destroy(faces);
            return {true, sol};
        }

        extend_polytope(faces, d);
    }

    array_destroy(faces);
    return {false};
}

GJKEPASolution gjk_epa_intersect_and_solve(const GJKShape& s1, const GJKShape& s2)
{
    GJKResult res = run_gjk(s1, s2);

    if (!res.collision)
        return {false};

    return run_epa(s1, s2, &res.simplex);
}