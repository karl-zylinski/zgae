#include "obj.h"
#include "mesh.h"
#include "array.h"
#include "file.h"
#include "math.h"
#include "vertex.h"

struct ParserState
{
    unsigned char* data;
    unsigned char* head;
    unsigned char* end;
};

struct ParsedFace
{
    unsigned v1;
    unsigned v2;
    unsigned v3;
    unsigned n1;
    unsigned n2;
    unsigned n3;
    unsigned u1;
    unsigned u2;
    unsigned u3;
};

struct ParsedData
{
    Vec3* vertices;
    Vec3* normals;
    Vec2* uvs;
    ParsedFace* faces;
};

static void skip_to_numeric(ParserState* ps)
{
    while (ps->head < ps->end && (*ps->head < '0' || *ps->head > '9') && *ps->head != '-')
    {
        ++ps->head;
    }
}

static void parse_uv(ParserState* ps, ParsedData* pd)
{
    Vec2 uv = {};
    skip_to_numeric(ps);
    uv.x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    uv.y = strtof((const char*)ps->head, (char**)&ps->head);
    array_push(pd->uvs, uv);
}

static void parse_normal(ParserState* ps, ParsedData* pd)
{
    Vec3 normal = {};
    skip_to_numeric(ps);
    normal.x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    normal.y = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    normal.z = strtof((const char*)ps->head, (char**)&ps->head);
    array_push(pd->normals, normal);
}

static void parse_vertex(ParserState* ps, ParsedData* pd)
{
    Vec3 vertex = {};
    skip_to_numeric(ps);
    vertex.x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    vertex.y = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    vertex.z = strtof((const char*)ps->head, (char**)&ps->head);
    array_push(pd->vertices, vertex);
}

static void parse_face(ParserState* ps, ParsedData* pd)
{
    ParsedFace face = {};
    skip_to_numeric(ps);
    face.v1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.v2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.v3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    array_push(pd->faces, face);
}

static void skip_line(ParserState* ps)
{
    while (ps->head < ps->end && *ps->head != '\n')
    {
        ++ps->head;
    }

    ++ps->head;
}

static ParsedData parse(unsigned char* data, unsigned data_size)
{
    ParserState ps = {};
    ps.data = data;
    ps.head = data;
    ps.end = (unsigned char*)ptr_add(data, data_size);
    ParsedData pd = {};
    ps.head = ps.data;

    while (ps.head < ps.end)
    {
        unsigned char c = *ps.head;
        bool first_on_line = ps.head == ps.data || (ps.head > ps.data && (*(ps.head - 1)) == '\n');

        if (!first_on_line)
            skip_line(&ps);
        else if (c == 'v' && ps.head + 1 < ps.end && (*(ps.head+1)) == 't')
            parse_uv(&ps, &pd);
        else if (c == 'v' && ps.head + 1 < ps.end && (*(ps.head+1)) == 'n')
            parse_normal(&ps, &pd);
        else if (c == 'v')
            parse_vertex(&ps, &pd);
        else if (c == 'f')
            parse_face(&ps, &pd);
        else
            skip_line(&ps);
    }

    return pd;
}

static int get_existing_vertex(const Vertex* vertices, const Vertex& v1)
{
    for (unsigned i = 0; i < array_num(vertices); ++i)
    {
        const Vertex& v2 = vertices[i];

        if (almost_equal(v1.position, v2.position)
            && almost_equal(v1.normal, v2.normal)
            && almost_equal(v1.uv, v2.uv)
            && almost_equal(v1.color, v2.color))
        {
            return i;
        }
    }

    return -1;
}

static void add_vertex_to_mesh(Vertex** vertices, unsigned** indices, const Vec3& pos, const Vec3& normal, const Vec2& uv, const Color& c)
{
    Vertex v = {};
    v.position = pos;
    v.normal = normal;
    v.uv = uv;
    v.color = c;
    
    int i = get_existing_vertex(*vertices, v);

    if (i != -1)
    {
        array_push(*indices, i);
        return;
    }

    array_push(*indices, (unsigned)array_num(*vertices));
    array_push(*vertices, v);
}

LoadedMesh obj_load(const char* filename)
{
    LoadedFile lf = file_load(filename);

    if (!lf.valid)
        return {false};

    ParsedData pd = parse(lf.file.data, lf.file.size);
    zfree(lf.file.data);
    Vertex* vertices = nullptr;
    unsigned* indices = nullptr;

    for (unsigned i = 0; i < array_num(pd.faces); ++i)
    {
        const ParsedFace& f = pd.faces[i];
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v1], pd.normals[f.n1], pd.uvs[f.u1], {1.0f, 1.0f,1, 1.0f});
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v2], pd.normals[f.n2], pd.uvs[f.u2], {1, 1.0f, 1, 1.0f});
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v3], pd.normals[f.n3], pd.uvs[f.u3], {1, 1, 1.0f, 1.0f});
    }

    array_destroy(pd.vertices);
    array_destroy(pd.normals);
    array_destroy(pd.uvs);
    array_destroy(pd.faces);

    Mesh m = {};
    m.num_vertices = (unsigned)array_num(vertices);
    m.vertices = (Vertex*)array_grab_data(vertices);
    m.num_indices = (unsigned)array_num(indices);
    m.indices = (unsigned*)array_grab_data(indices);
    return {true, m};
}
