#include "obj_loader.h"
#include "array.h"
#include "file.h"
#include "memory.h"
#include "math.h"

typedef struct ParserState
{
    char* data;
    char* head;
    char* end;
} ParserState;

typedef struct ParsedFace
{
    u32 v1;
    u32 v2;
    u32 v3;
    u32 n1;
    u32 n2;
    u32 n3;
    u32 u1;
    u32 u2;
    u32 u3;
} ParsedFace;

typedef struct ParsedData
{
    Vec3* vertices;
    Vec3* normals;
    Vec2* uvs;
    ParsedFace* faces;
} ParsedData;

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
    array_add(pd->uvs, uv);
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
    array_add(pd->normals, normal);
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
    array_add(pd->vertices, vertex);
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
    array_add(pd->faces, face);
}

static void skip_line(ParserState* ps)
{
    while (ps->head < ps->end && *ps->head != '\n')
    {
        ++ps->head;
    }

    ++ps->head;
}

typedef enum ParseMode {
    PARSE_MODE_ALL,
    PARSE_MODE_ONLY_VERTICES
} ParseMode;

static ParsedData parse(char* data, unsigned data_size, ParseMode mode)
{
    ParserState ps = {};
    ps.data = data;
    ps.head = data;
    ps.end = (char*)(data + data_size);
    ParsedData pd = {};
    ps.head = ps.data;

    while (ps.head < ps.end)
    {
        char c = *ps.head;
        bool first_on_line = ps.head == ps.data || (ps.head > ps.data && (*(ps.head - 1)) == '\n');

        if (!first_on_line)
            skip_line(&ps);
        else if (c == 'v' && ps.head + 1 < ps.end && (*(ps.head+1)) == 't' && mode == PARSE_MODE_ALL)
            parse_uv(&ps, &pd);
        else if (c == 'v' && ps.head + 1 < ps.end && (*(ps.head+1)) == 'n' && mode == PARSE_MODE_ALL)
            parse_normal(&ps, &pd);
        else if (c == 'v' && ps.head + 1 < ps.end && (*(ps.head+1)) == ' ')
            parse_vertex(&ps, &pd);
        else if (c == 'f' && mode == PARSE_MODE_ALL)
            parse_face(&ps, &pd);
        else
            skip_line(&ps);
    }

    return pd;
}

static int get_existing_vertex(const GeometryVertex* vertices, const GeometryVertex* v1)
{
    for (unsigned i = 0; i < array_num(vertices); ++i)
    {
        const GeometryVertex* v2 = vertices + i;

        if (vec3_almost_eql(&v1->position, &v2->position)
            && vec3_almost_eql(&v1->normal, &v2->normal)
            && vec2_almost_eql(&v1->texcoord, &v2->texcoord)
            && vec4_almost_eql(&v1->color, &v2->color))
        {
            return i;
        }
    }

    return -1;
}

static void add_vertex_to_mesh(GeometryVertex** vertices, GeometryIndex** indices, const Vec3* pos, const Vec3* normal, const Vec2* texcoord, const Vec4* c)
{
    GeometryVertex v = {
        .position = *pos,
        .normal = *normal,
        .texcoord = *texcoord,
        .color = *c
    };
    
    int i = get_existing_vertex(*vertices, &v);

    if (i != -1)
    {
        array_add(*indices, i);
        return;
    }

    array_add(*indices, (unsigned)array_num(*vertices));
    array_add(*vertices, v);
}

ObjLoadResult obj_load(const char* filename)
{
    FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_DEFAULT);

    if (!flr.ok)
    {
        ObjLoadResult r = {.ok = false};
        return r;
    }

    ParsedData pd = parse(flr.data, flr.data_size, PARSE_MODE_ALL);
    memf(flr.data);
    GeometryVertex* vertices = NULL;
    GeometryIndex* indices = NULL;

    for (u32 i = 0; i < array_num(pd.faces); ++i)
    {
        const ParsedFace* f = pd.faces + i;
        Vec4 white = {1,1,1,1};
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f->v1], &pd.normals[f->n1], &pd.uvs[f->u1], &white);
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f->v2], &pd.normals[f->n2], &pd.uvs[f->u2], &white);
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f->v3], &pd.normals[f->n3], &pd.uvs[f->u3], &white);
    }

    array_destroy(pd.vertices);
    array_destroy(pd.normals);
    array_destroy(pd.uvs);
    array_destroy(pd.faces);

    Mesh m = {
        .vertices = array_copy_data(vertices),
        .vertices_num = array_num(vertices),
        .indices =  array_copy_data(indices),
        .indices_num = array_num(indices)
    };

    array_destroy(vertices);
    array_destroy(indices);

    ObjLoadResult olr = {
        .ok = true,
        .mesh = m
    };

    return olr;
}

ObjLoadVerticesResult obj_load_only_vertices(const char* filename)
{
    FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_DEFAULT);

    if (!flr.ok)
    {
        ObjLoadVerticesResult olr = { .ok = false };
        return olr;
    }

    ParsedData pd = parse(flr.data, flr.data_size, PARSE_MODE_ONLY_VERTICES);
    memf(flr.data);

    ObjLoadVerticesResult olr = {
        .ok = true,
        .vertices = array_copy_data(pd.vertices),
        .vertices_num = array_num(pd.vertices)
    };

    return olr;
}