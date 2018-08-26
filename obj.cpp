#include "obj.h"
#include <stdlib.h>
#include "mesh.h"
#include "memory.h"
#include "file.h"

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
    DynamicArray<Vector3> vertices;
    DynamicArray<Vector3> normals;
    DynamicArray<Vector2> uvs;
    DynamicArray<ParsedFace> faces;
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
    Vector2* uv = pd->uvs.push();
    skip_to_numeric(ps);
    uv->x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    uv->y = strtof((const char*)ps->head, (char**)&ps->head);
}

static void parse_normal(ParserState* ps, ParsedData* pd)
{
    Vector3* normal = pd->normals.push();
    skip_to_numeric(ps);
    normal->x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    normal->y = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    normal->z = strtof((const char*)ps->head, (char**)&ps->head);
}

static void parse_vertex(ParserState* ps, ParsedData* pd)
{
    Vector3* vertex = pd->vertices.push();
    skip_to_numeric(ps);
    vertex->x = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    vertex->y = strtof((const char*)ps->head, (char**)&ps->head);
    skip_to_numeric(ps);
    vertex->z = strtof((const char*)ps->head, (char**)&ps->head);
}

static void parse_face(ParserState* ps, ParsedData* pd)
{
    ParsedFace* face = pd->faces.push();
    skip_to_numeric(ps);
    face->v1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->u1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->n1 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->v2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->u2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->n2 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->v3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->u3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
    skip_to_numeric(ps);
    face->n3 = strtol((const char*)ps->head, (char**)&ps->head, 10) - 1;
}

static void skip_line(ParserState* ps)
{
    while (ps->head < ps->end && *ps->head != '\n')
    {
        ++ps->head;
    }

    ++ps->head;
}

static ParsedData parse(Allocator* alloc, unsigned char* data, unsigned data_size)
{
    ParserState ps = {};
    ps.data = data;
    ps.head = data;
    ps.end = (unsigned char*)mem_ptr_add(data, data_size);
    ParsedData pd = {};
    pd.vertices = dynamic_array_create<Vector3>(alloc);
    pd.normals = dynamic_array_create<Vector3>(alloc);
    pd.uvs = dynamic_array_create<Vector2>(alloc);
    pd.faces = dynamic_array_create<ParsedFace>(alloc);
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

static int get_existing_vertex(const DynamicArray<Vertex>& vertices, const Vertex& v1)
{
    for (unsigned i = 0; i < vertices.num; ++i)
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

static void add_vertex_to_mesh(DynamicArray<Vertex>* vertices, DynamicArray<unsigned>* indices, const Vector3& pos, const Vector3& normal, const Vector2& uv, const Color& c)
{
    Vertex v = {};
    v.position = pos;
    v.normal = normal;
    v.uv = uv;
    v.color = c;
    
    int i = get_existing_vertex(*vertices, v);

    if (i != -1)
    {
        indices->add(i);
        return;
    }

    indices->add(vertices->num);
    vertices->add(v);
}

LoadedMesh obj_load(Allocator* alloc, const char* filename)
{
    LoadedFile lf = file_load(alloc, filename);

    if (!lf.valid)
        return {false};

    ParsedData pd = parse(alloc, lf.file.data, lf.file.size);
    DynamicArray<Vertex> vertices = dynamic_array_create<Vertex>(alloc);
    DynamicArray<unsigned> indices = dynamic_array_create<unsigned>(alloc);

    for (unsigned i = 0; i < pd.faces.num; ++i)
    {
        const ParsedFace& f = pd.faces[i];
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v1], pd.normals[f.n1], pd.uvs[f.u1], {1.0f, 0, 0, 1.0f});
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v2], pd.normals[f.n2], pd.uvs[f.u2], {0, 1.0f, 0, 1.0f});
        add_vertex_to_mesh(&vertices, &indices, pd.vertices[f.v3], pd.normals[f.n3], pd.uvs[f.u3], {0, 0, 1.0f, 1.0f});
    }

    Mesh m = {};
    m.vertices = vertices.data;
    m.num_vertices = vertices.num;
    m.indices = indices.data;
    m.num_indices = indices.num;
    return {true, m};
}
