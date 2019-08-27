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
    Array<Vec3> vertices;
    Array<Vec3> normals;
    Array<Vec2> uvs;
    Array<ParsedFace> faces;
} ParsedData;

static void skip_to_numeric(mut ParserState* ps)
{
    while (ps->head < ps->end && (*ps->head < '0' || *ps->head > '9') && *ps->head != '-')
    {
        ++ps->head;
    }
}

static void parse_uv(mut ParserState* ps, mut ParsedData* pd)
{
    Vec2 uv = {};
    skip_to_numeric(ps);
    uv.x = strtof(ps->head, &ps->head);
    skip_to_numeric(ps);
    uv.y = strtof(ps->head, &ps->head);
    array_push(&pd->uvs, uv);
}

static void parse_normal(mut ParserState* ps, mut ParsedData* pd)
{
    Vec3 normal = {};
    skip_to_numeric(ps);
    normal.x = strtof(ps->head, &ps->head);
    skip_to_numeric(ps);
    normal.y = strtof(ps->head, &ps->head);
    skip_to_numeric(ps);
    normal.z = strtof(ps->head, &ps->head);
    array_push(&pd->normals, normal);
}

static void parse_vertex(mut ParserState* ps, mut ParsedData* pd)
{
    Vec3 vertex = {};
    skip_to_numeric(ps);
    vertex.x = strtof(ps->head, &ps->head);
    skip_to_numeric(ps);
    vertex.y = strtof(ps->head, &ps->head);
    skip_to_numeric(ps);
    vertex.z = strtof(ps->head, &ps->head);
    array_push(&pd->vertices, vertex);
}

static void parse_face(mut ParserState* ps, mut ParsedData* pd)
{
    ParsedFace face = {};
    skip_to_numeric(ps);
    face.v1 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u1 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n1 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.v2 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u2 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n2 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.v3 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.u3 = strtol(ps->head, &ps->head, 10) - 1;
    skip_to_numeric(ps);
    face.n3 = strtol(ps->head, &ps->head, 10) - 1;
    array_push(&pd->faces, face);
}

static void skip_line(mut ParserState* ps)
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
    ps.end = (data + data_size);
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

static int get_existing_vertex(Array<MeshVertex>* vertices, MeshVertex* v1)
{
    for (unsigned i = 0; i < vertices->num; ++i)
    {
        MeshVertex* v2 = vertices->data + i;

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

static void add_vertex_to_mesh(
    mut Array<MeshVertex>* vertices, mut Array<MeshIndex>* indices,
    Vec3* pos, Vec3* normal, Vec2* texcoord, Vec4* c)
{
    MeshVertex v = {
        .position = *pos,
        .normal = *normal,
        .texcoord = *texcoord,
        .color = *c
    };
    
    int i = get_existing_vertex(vertices, &v);

    if (i != -1)
    {
        array_push(indices, (MeshIndex)i);
        return;
    }

    array_push(indices, (MeshIndex)vertices->num);
    array_push(vertices, v);
}

ObjLoadResult obj_load(char* filename)
{
    FileLoadResult flr = file_load(filename, FileLoadMode::Default);

    if (!flr.ok)
    {
        ObjLoadResult r = {.ok = false};
        return r;
    }

    ParsedData pd = parse((char*)flr.data, flr.data_size, PARSE_MODE_ALL);
    memf(flr.data);
    Array<MeshVertex> vertices = {};
    Array<MeshIndex> indices = {};

    for (u32 i = 0; i < pd.faces.num; ++i)
    {
        ParsedFace& f = pd.faces[i];
        Vec4 white = {1,1,1,1};
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f.v1], &pd.normals[f.n1], &pd.uvs[f.u1], &white);
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f.v2], &pd.normals[f.n2], &pd.uvs[f.u2], &white);
        add_vertex_to_mesh(&vertices, &indices, &pd.vertices[f.v3], &pd.normals[f.n3], &pd.uvs[f.u3], &white);
    }

    array_destroy(&pd.vertices);
    array_destroy(&pd.normals);
    array_destroy(&pd.uvs);
    array_destroy(&pd.faces);

    Mesh m = {
        .vertices = array_copy_data(&vertices),
        .vertices_num = (u32)vertices.num,
        .indices =  array_copy_data(&indices),
        .indices_num = (u32)indices.num
    };

    array_destroy(&vertices);
    array_destroy(&indices);

    ObjLoadResult olr = {
        .ok = true,
        .mesh = m
    };

    return olr;
}

ObjLoadVerticesResult obj_load_only_vertices(char* filename)
{
    FileLoadResult flr = file_load(filename, FileLoadMode::Default);

    if (!flr.ok)
    {
        ObjLoadVerticesResult olr = { .ok = false };
        return olr;
    }

    ParsedData pd = parse((char*)flr.data, flr.data_size, PARSE_MODE_ONLY_VERTICES);
    memf(flr.data);

    ObjLoadVerticesResult olr = {
        .ok = true,
        .vertices = array_copy_data(&pd.vertices),
        .vertices_num = (u32)pd.vertices.num
    };

    return olr;
}